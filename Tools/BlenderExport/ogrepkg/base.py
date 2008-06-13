"""Helper functions and classes.

   @author Michael Reimpell
"""
# Copyright (C) 2005  Michael Reimpell
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# epydoc doc format
__docformat__ = "javadoc en"

import sys
import os
import string
import pickle
import Blender

#matrixOne = [[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
#matrixFlip = [[1,0,0,0],[0,0,-1,0],[0,1,0,0],[0,0,0,1]]

def indent(indent):
	"""Indentation.
	
	   @param indent Level of indentation.
	   @return String.
	"""
	return "	"*indent
	
class Singleton:
	"""Ensure that a class has at most one instance.
	
	@cvar instances Existing instances.
	"""
	instances = {}
	def __init__(self):
		"""Constructor.
		"""
		assert self.__class__ not in Singleton.instances.keys(), \
			"Class \"%s\" is a singleton." % self.__class__
		Singleton.instances[self.__class__] = self
		return
	# @classmethod
	def getSingleton(singletonClass):
		"""Returns singleton instance.
		"""
		instance = None
		if Singleton.instances.has_key(singletonClass):
			instance = Singleton.instances[singletonClass]
		else:
			instance = singletonClass()
		return instance
	getSingleton = classmethod(getSingleton)

class Model:
	"""Model interface.
	"""
	def __init__(self):
		self.viewList = []
		return
	def addView(self, view):
		self.viewList.append(view)
		return
	def removeView(self, view):
		if view in self.viewList:
			self.viewList.remove(view)
		return
	# protected
	def _notify(self):
		"""Notify views.
		"""
		for view in self.viewList:
			view.update()
		return

class View:
	"""View interface.
	"""
	def __init__(self, model):
		self.attachModel(model)
		return
	def __del__(self):
		self.detachModel()
		return
	def attachModel(self, model):
		self.model = model
		self.model.addView(self)
		return
	def detachModel(self):
		self.model.removeView(self)
		return
	def update(self):
		return

class Log(Singleton, Model):
	"""Logs messages and status.
	
	   Logs messages as a list of strings and keeps track of the status.
	   Possible status values are info, warning and error.
	   
	   @cvar INFO info status
	   @cvar WARNING warning status
	   @cvar ERROR error status
	"""
	INFO, WARNING, ERROR = range(3)
	def __init__(self):
		"""Constructor.
		"""
		Singleton.__init__(self)
		Model.__init__(self)
		self.clear()
		return
	def clear(self):
		"""Clears the log.
		"""
		self.messageList = []
		self.status = Log.INFO
		return
	def logInfo(self, message):
		"""Logs an info message.
		
		   @param message message string
		"""
		self.messageList.append((Log.INFO, message))
		self._notify()
		return		
	def logWarning(self, message):
		"""Logs a warning message.
		
		   The status is set to <code>Log.WARNING</code> if it is not already <code>Log.ERROR</code>.
		   
		   @param message message string
		"""
		self.messageList.append((Log.WARNING, "Warning: "+message))
		if not self.status == Log.ERROR:
			self.status = Log.WARNING
		self._notify()
		return
	def logError(self, message):
		"""Logs an error message.
		
		   The status is set to <code>Log.ERROR</code>.
		   
		   @param message message string
		"""
		self.messageList.append((Log.ERROR, "Error: "+message))
		self.status = Log.ERROR
		self._notify()
		return
	def getStatus(self):
		"""Gets the current status.
		
		   The status can be
		   <ul>
		   <li><code>Log.INFO</code>
		   <li><code>Log.WARNING</code>
		   <li><code>Log.ERROR</code>
		   </ul>
		   
		   @return status
		"""
		return self.status
	def getMessageList(self):
		"""Returns the list of log messages.
		
		   @return list of tuples (status, message)
		"""
		return self.messageList

class PackageSettings(Singleton):
	"""Manages package configuration.
	"""
	def __init__(self):
		Singleton.__init__(self)
		self.dict = {'objectDict':{}}
		self.textName = 'OgrePackage.cfg'
		return
	def save(self):
		"""Save to Blender text 'OgrePackage.cfg'.
		"""
		# remove old configuration text
		if self.textName in [text.getName() for text in Blender.Text.Get()]:
			oldSettingsText = Blender.Text.Get(self.textName)
			oldSettingsText.setName('OgrePackage.old')
			Blender.Text.unlink(oldSettingsText)
		# write new configuration text
		settingsText = Blender.Text.New(self.textName)
		settingsText.write('OGRE Package settings file.\n\nThis file is automatically created. Please don\'t edit this file directly.\n\n')
		try:
			# pickle
			settingsText.write(pickle.dumps(self.dict))
		except (pickle.PickleError):
			Log.getSingleton().logError('Couldn\'t pickle package settings!')
		return
	def clearNoneExistingObjects(self):
		"""Maintainance method.
		
		   As settigns are shared among the package, they get never deleted automatically.
		   All settings that are loaded are saved again. Therefore it is advisable to clear
		   the object settings for deleted objects from time to time. Applications that use
		   the package are responsible to maintain their settings.
		"""
		existingNameList = [object.getName() for object in Blender.Object.Get()]
		for name in self.dict['objectDict'].keys():
			if name not in existingNameList:
				del self.dict['objectDict'][name]
		return
	def setSetting(self, key, value):
		"""Saves a setting to the package dict.
		
		   Note: 'objectDict' is a reserved key.
		"""
		self.dict[key] = value
		return
	def getSetting(self, key):
		value = None
		if self.dict.has_key(key):
			value = self.dict[key]
		return value
	def setObjectSetting(self, objectName, key, value):
		if self.dict['objectDict'].has_key(objectName):
			self.dict['objectDict'][objectName][key] = value
		else:
			self.dict['objectDict'][objectName] = {key:value}
		return
	def getObjectSetting(self, objectName, key):
		value = None
		if self.dict['objectDict'].has_key(objectName):
			if self.dict['objectDict'][objectName].has_key(key):
				value = self.dict['objectDict'][objectName][key]
		return value	
	def load(self):
		"""Load from Blender text 'OgrePackage.cfg'.
		"""
		if self.textName in [text.getName() for text in Blender.Text.Get()]:
			settingsText = Blender.Text.Get(self.textName)
			# compose string from text and unpickle
			try:
				# unpickle
				self.dict = pickle.loads(string.join(settingsText.asLines()[4:],'\n'))
			except (pickle.PickleError):
				Log.getSingleton().logError('Couldn\'t unpickle package settings!')
				self.dict = {'objectDict':{}}
		return

class OgreXMLConverter(Singleton):
	def __init__(self):
		Singleton.__init__(self)
		self.converter = None
		self.converterArgs = ''
		# get binary location from Blender's registry
		registryDict = Blender.Registry.GetKey('OgrePackage', True)
		if registryDict:
			if registryDict.has_key('OgreXMLConverter'):
				self.converter = registryDict['OgreXMLConverter']
		# basic converter arguements
		nuextremityPoints = PackageSettings.getSingleton().getSetting('OgreXMLConverterNuExtremityPoints')
		if nuextremityPoints:
			self.nuextremityPoints = nuextremityPoints
		else:
			self.nuextremityPoints = 0
		generateEdgeLists = PackageSettings.getSingleton().getSetting('OgreXMLConverterGenerateEdgeLists')
		if generateEdgeLists:
			self.generateEdgeLists = generateEdgeLists
		else:
			self.generateEdgeLists = True
		generateTangents = PackageSettings.getSingleton().getSetting('OgreXMLConverterGenerateTangents')
		if generateTangents:
			self.generateTangents = generateTangents
		else:
			self.generateTangents = False
		tangentSemantic = PackageSettings.getSingleton().getSetting('OgreXMLConverterTangentSemantic')
		if tangentSemantic:
			self.tangentSemantic = tangentSemantic
		else:
			self.tangentSemantic = 'tangent'
		tangentUseParity = PackageSettings.getSingleton().getSetting('OgreXMLConverterTangentUseParity')
		if tangentUseParity:
			self.tangentUseParity = tangentUseParity
		else:
			self.tangentUseParity = '3'
		tangentSplitMirrored = PackageSettings.getSingleton().getSetting('OgreXMLConverterTangentSplitMirrored')
		if tangentSplitMirrored:
			self.tangentSplitMirrored = tangentSplitMirrored
		else:
			self.tangentSplitMirrored = False
		tangentSplitRotated = PackageSettings.getSingleton().getSetting('OgreXMLConverterTangentSplitRotated')
		if tangentSplitRotated:
			self.tangentSplitRotated = tangentSplitRotated
		else:
			self.tangentSplitRotated = False
		reorganiseBuffers = PackageSettings.getSingleton().getSetting('OgreXMLConverterReorganiseBuffers')
		if reorganiseBuffers:
			self.reorganiseBuffers = reorganiseBuffers
		else:
			self.reorganiseBuffers = True
		optimiseAnimations = PackageSettings.getSingleton().getSetting('OgreXMLConverterOptimiseAnimations')
		if optimiseAnimations:
			self.optimiseAnimations = optimiseAnimations
		else:
			self.optimiseAnimations = True

		# additional converter arguments
		converterArgs = PackageSettings.getSingleton().getSetting('OgreXMLConverterArgs')
		if converterArgs:
			self.converterArgs = converterArgs
		else:
			self.converterArgs = ''
		return
	def setNuExtremityPoints(self, nuextremityPoints):
		self.nuextremityPoints = nuextremityPoints
		PackageSettings.getSingleton().setSetting('OgreXMLConverterNuExtremityPoints', self.nuextremityPoints)
		return
	def getNuExtremityPoints(self):
		return self.nuextremityPoints
	def setGenerateEdgeLists(self, generateEdgeLists):
		self.generateEdgeLists = generateEdgeLists
		PackageSettings.getSingleton().setSetting('OgreXMLConverterGenerateEdgeLists', self.generateEdgeLists)
		return
	def getGenerateEdgeLists(self):
		return self.generateEdgeLists
	def setGenerateTangents(self, generateTangents):
		self.generateTangents = generateTangents
		PackageSettings.getSingleton().setSetting('OgreXMLConverterGenerateTangents', self.generateTangents)
		return
	def getGenerateTangents(self):
		return self.generateTangents
	def setTangentSemantic(self, tangentSemantic):
		self.tangentSemantic = tangentSemantic
		PackageSettings.getSingleton().setSetting('OgreXMLConverterTangentSemantic', self.tangentSemantic)
		return
	def getTangentSemantic(self):
		return self.tangentSemantic
	def setTangentUseParity(self, tangentUseParity):
		self.tangentUseParity = tangentUseParity
		PackageSettings.getSingleton().setSetting('OgreXMLConverterTangentUseParity', self.tangentUseParity)
		return
	def getTangentUseParity(self):
		return self.tangentUseParity
	def setTangentSplitMirrored(self, tangentSplitMirrored):
		self.tangentSplitMirrored = tangentSplitMirrored
		PackageSettings.getSingleton().setSetting('OgreXMLConverterTangentSplitMirrored', self.tangentSplitMirrored)
		return
	def getTangentSplitMirrored(self):
		return self.tangentSplitMirrored
	def setTangentSplitRotated(self, tangentSplitRotated):
		self.tangentSplitRotated = tangentSplitRotated
		PackageSettings.getSingleton().setSetting('OgreXMLConverterTangentSplitRotated', self.tangentSplitRotated)
		return
	def getTangentSplitRotated(self):
		return self.tangentSplitRotated
	def setReorganiseBuffers(self, reorganiseBuffers):
		self.reorganiseBuffers = reorganiseBuffers
		PackageSettings.getSingleton().setSetting('OgreXMLConverterReorganiseBuffers', self.reorganiseBuffers)
		return
	def getReorganiseBuffers(self):
		return self.reorganiseBuffers
	def setOptimiseAnimations(self, optimiseAnimations):
		self.optimiseAnimations = optimiseAnimations
		PackageSettings.getSingleton().setSetting('OgreXMLConverterOptimiseAnimations', self.optimiseAnimations)
		return
	def getOptimiseAnimations(self):
		return self.optimiseAnimations

	def setAdditionalArguments(self, arguments):
		self.converterArgs = arguments
		PackageSettings.getSingleton().setSetting('OgreXMLConverterArgs', self.converterArgs)
		return
	def getAdditionalArguments(self):
		return self.converterArgs
	def getConverter(self):
		return self.converter
	def setConverter(self, converter):
		"""Sets converter executable.
		   
		   Also saves converter location in Blender's registry. If <code>None</code>
		   is passed, the converter is searched in $PATH.
		   
		   @param converter Location of OgreXMLConverter.
		"""
		self.converter = converter
		# update registry
		registryDict = Blender.Registry.GetKey('OgrePackage', True)
		if converter is None:
			# remove registry entry
			if registryDict and registryDict.has_key('OgreXMLConverter'):
				del registryDict['OgreXMLConverter']
				Blender.Registry.SetKey('OgrePackage', registryDict, True)
		else:
			# set registry entry
			if registryDict is None:
				registryDict = {}
			registryDict['OgreXMLConverter'] = self.converter
			Blender.Registry.SetKey('OgrePackage', registryDict, True)
		return
	def findConverter(self):
		"""Find converter in path.
		
		   @return converter location or <code>None</code> if converter is not found.
		"""
		converter = None
		path = os.environ.get('PATH')
		if path:
			pathList = string.split(path, ':')
			found = False
			i = 0
			while not found and (i < len(pathList)):
				if os.path.exists(os.path.join(pathList[i], 'OgreXMLConverter')):
					converter = os.path.join(pathList[i], 'OgreXMLConverter')
					found = True
				i += 1
		return converter
	def convert(self, filename, arguments=''):
		"""Converts given file with the OgreXMLConverter.
		
		   @param filename The xml filename to pass to the converter.
		   @param arguments Additional arguments to pass to the converter.
		"""
		converter = None
		if self.converter:
			converter = self.converter
		elif self.findConverter():
			converter = 'OgreXMLConverter'
		if converter:
			dir = os.path.dirname(filename)
			# ensure proper encoding of filenames
			encodedDir = os.path.normpath(dir)
			encodedFilename = os.path.normpath(filename)
			encodedConverter = os.path.normpath(converter)
			# check if converter exists
			# if os.access(encodedConverter, os.X_OK):
			# Does only work for full path!
			# Testcase: Preferences->Manual->"OgreXMLConverter"
			if True:
				# build basic arguments
				basicArguments = ''
				if self.nuextremityPoints > 0:
					basicArguments += ' -x ' + self.nuextremityPoints
				if not self.generateEdgeLists:
					basicArguments += ' -e'
				if self.generateTangents:
					basicArguments += ' -t'
				if self.tangentSemantic == 'uvw':
					basicArguments += ' -td uvw'
				if self.tangentUseParity == '4':
					basicArguments += ' -ts 4'
				if self.tangentSplitMirrored:
					basicArguments += ' -tm'
				if self.tangentSplitRotated:
					basicArguments += ' -tr'
				if not self.reorganiseBuffers:
					basicArguments += ' -r'
				if not self.optimiseAnimations:
					basicArguments += ' -o'
				# call the converter
				commandLine = '"' + encodedConverter + '" -log "' \
					 + os.path.join(encodedDir, 'OgreXMLConverter.log') \
					 + '" ' + self.converterArgs + basicArguments + ' ' + arguments \
					 + ' "' + encodedFilename + '"'
				if os.name == "nt":
					# workaround for popen windows bug
					commandLine = '"' + commandLine + '"'
				Log.getSingleton().logInfo("Running OgreXMLConverter: " + commandLine)
				xmlConverter = os.popen(commandLine, 'r')
				for line in xmlConverter:
					Log.getSingleton().logInfo("OgreXMLConverter: " + line)
				xmlConverter.close()
			else:
				Log.getSingleton().logError('Can not execute OgreXMLConverter "' + encodedConverter + '"')
		else:
			Log.getSingleton().logError("OgreXMLConverter not found! Please specify the path to the OgreXMLConverter in the preferences.")
		return

class ConvertibleXMLFile:
	"""
	"""
	def __init__(self, filename):
		self.filename = filename
		self.converterOptionDict = {}
		self.fileObject = open(self.filename, "w")
		return
	def write(self, str):
		self.fileObject.write(str)
		return
	def close(self):
		self.fileObject.close()
		return
	def addConverterOption(self, option, value=''):
		self.converterOptionDict[option] = value
		return
	def convert(self):
		arguments = ''
		for (key, value) in self.converterOptionDict.iteritems():
			arguments += ' ' + key + ' ' + value
		OgreXMLConverter.getSingleton().convert(self.filename, arguments)
		return

class PathName:
	"""Splits a pathname independent of the underlying os.
	
	   Blender saves pathnames in the os specific manner. Using os.path may result in problems
	   when the export is done on a different os than the creation of the .blend file.	   
	"""
	def __init__(self, pathName):
		self.pathName = pathName
		return
	def dirname(self):
		return os.path.dirname(self.pathName) 
	def basename(self):
		baseName = os.path.basename(self.pathName)
		# split from non-os directories
		# \\
		baseName = baseName.split('\\').pop()
		# /
		baseName = baseName.split('/').pop()
		if (baseName != baseName.replace(' ','_')):
			# replace whitespace with underscore
			Log.getSingleton().logWarning("Whitespaces in filename \"%s\" replaced with underscores." % baseName)
			baseName = baseName.replace(' ','_')
		return baseName
	def path(self):
		return self.pathName
