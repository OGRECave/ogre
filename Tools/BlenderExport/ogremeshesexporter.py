#!BPY

# """
# Name: 'OGRE Meshes'
# Blender: 244
# Group: 'Export'
# Tooltip: 'Export meshes and animations to OGRE'
# """

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

__author__ = 'Michael Reimpell (Maintainer: Lih-Hern Pang)'
__version__ = '1.1'
__url__ = ["Help, http://www.ogre3d.org/phpBB2/search.php", "Ogre3D, http://www.ogre3d.org"]
__bpydoc__ = "Please see the external documentation that comes with the script."

# epydoc doc format
__docformat__ = "javadoc en"

import sys

try:
	import Blender
except ImportError:
	sys.exit("Please run this script from within Blender!\nSee the script's manual for more information.")

class OgrePackageImporter:
		"""Imports ogrepkg.
		
		   If ogrepkg fails to load, the user can be requested to
		   point to the parent directory of the 'ogrepkg' directory.
		   The PYTHONPATH is then set accordingly and the package
		   location is stored in Blender's registry.
		   
		   Requires modules sys and Blender.
		"""
		def __init__(self, packagePath=None):
			"""Constructor.
			
			   @param packagePath Optional packagePath to include into PYTHONPATH.
			"""
			# import sys
			self.packagePath = packagePath
			return
		def importPackage(self):
			"""Tries to import ogrepkg.
			
			   Tries to import ogrepkg with the PYTHONPATH based
			   on the optional constructor argument and the contents
			   of Blender's registry entry for "OgrePackage".
			   Raises an ImportError on failure.
			"""
			if not self.packagePath:
				# load registry if packagePath is not given to constructor.
				self._loadRegistry()
			if self.packagePath:
				# append patckagePath to PYTHONPATH
				sys.path.append(self.packagePath)
			# try importing ogrepkg
			import ogrepkg #raises ImportError on failure
			return
		def requestPackagePath(self):
			"""Requests path to the ogrepkg package from the user.
			"""
			# gui
			path = Blender.Get('uscriptsdir')
			if not path:
				path = Blender.Get('scriptsdir')
			if not path:
				path = Blender.Get('filename')
			if not path:
				path = ''
			self.pathButton = Blender.Draw.Create(path)
			self.importSuccess = False
			Blender.Draw.Register(self._draw, self._eventHandler, self._buttonHandler)
			return
		def _loadRegistry(self):
			registryDict = Blender.Registry.GetKey('OgrePackage', True)
			if registryDict:
				if registryDict.has_key('packagePath'):
					if registryDict['packagePath']:
						self.packagePath = registryDict['packagePath']
			return
		def _saveRegistry(self):
			if self.packagePath:
				registryDict = Blender.Registry.GetKey('OgrePackage', True)
				if registryDict is None:
					registryDict =  {}
				registryDict['packagePath'] = self.packagePath
				Blender.Registry.SetKey('OgrePackage', registryDict, True)
			return
		def _checkPath(self, path):
			# Sets self.pathButton, self.importSuccess
			dirName = Blender.sys.dirname(path)
			if Blender.sys.exists(dirName):
				self.pathButton = Blender.Draw.Create(dirName)
				sys.path.append(dirName)
				try:
					import ogrepkg
				except ImportError:
					self.importSuccess = False
				else:
					self.importSuccess = True
				sys.path.remove(dirName)
				Blender.Draw.Redraw(1)
			return
		def _pathSelector(self, filename):
			self._checkPath(filename)
			return
		def _draw(self):
			# clear background
			theme = Blender.Window.Theme.Get()[0]
			bgColor = [color/255.0 for color in theme.get('buts').back]
			Blender.BGL.glClearColor(*bgColor)
			Blender.BGL.glClear(Blender.BGL.GL_COLOR_BUFFER_BIT)
			size = list(Blender.Window.GetAreaSize())
			if size[0] < 350:
				size[0] = 350
			if size[1] < 165:
				size[1] = 165
			# 10 px border
			rect = [10, 10, size[0]-11, size[1]-11]
			# text color
			Blender.BGL.glColor4ub(*theme.get('text').text)
			# title "Settings", large, bold, underlined
			Blender.BGL.glRasterPos2i(rect[0], rect[3] - 9)
			Blender.Draw.Text("Settings", 'large')
			Blender.BGL.glRasterPos2i(rect[0] + 1, rect[3] - 9)
			Blender.Draw.Text("Settings", 'large')
			# 14 px font size, 2 px line spacing, 5 px shift to baseline
			rect[3] -= 16
			# 2 px underline
			Blender.BGL.glRecti(rect[0], \
				rect[3]-2, \
				rect[0] + Blender.Draw.GetStringWidth("Settings", 'large') + 1, \
				rect[3])
			# 10 px skip
			rect[3] -= 12
			# message 12 px font size, 2 px line spacing, 4 px shift to baseline
			if not self.importSuccess:
				Blender.BGL.glRasterPos2i(rect[0], rect[3] - 8)
				Blender.Draw.Text("The script can't find the 'ogrepkg' package!")
				Blender.BGL.glRasterPos2i(rect[0], rect[3] - 22)
				Blender.Draw.Text("In order to run the script, please provide the path to the")
				Blender.BGL.glRasterPos2i(rect[0], rect[3] - 36)
				Blender.Draw.Text("directory that contains the 'ogrepkg' directory.")
			else:
				Blender.BGL.glRasterPos2i(rect[0], rect[3] - 8)
				Blender.Draw.Text("Package 'ogrepkg' found!")
				Blender.BGL.glRasterPos2i(rect[0], rect[3] - 22)
				Blender.Draw.Text("Click 'Ok' to store the package location permanently")
				Blender.BGL.glRasterPos2i(rect[0], rect[3] - 36)
				Blender.Draw.Text("and run the script again.")
				# You can cange the location any time using Blender's script registry editor.
			# 10 px skip
			rect[3] -= 52
			# path string
			self.pathButton = Blender.Draw.String("Path: ", 101, rect[0], rect[3] - 21, rect[2] - rect[1] - 71, 20, self.pathButton.val, 255, "Package location")
			# select button
			Blender.Draw.Button("Select", 102, rect[2] - 70, rect[3] - 21, 70, 20, "Select package location")
			rect[3] -= 31
			# Ok button
			if self.importSuccess:
				Blender.Draw.Button("Ok", 103, rect[0], rect[1], 100, 30, "Quit, accepting package location")
			# Abort button
			Blender.Draw.Button("Abort", 104, rect[2] - 101, rect[1], 100, 30, "Quit, discarding package location")
			return
		def _eventHandler(self, event, value):
			return
		def _buttonHandler(self, event):
			# 101: PathString
			# 102: Select
			# 103: Ok
			# 104: Abort
			if (event == 101): # PathString
				self._checkPath(self.pathButton.val)
			elif (event == 102): # Select
				Blender.Window.FileSelector(self._pathSelector, "Select Package Location", self.pathButton.val)
			elif (event ==  103): # Ok
				# assign packagePath
				self.packagePath = self.pathButton.val
				self._saveRegistry()
				Blender.Draw.Exit()
			elif(event == 104): # Abort
				Blender.Draw.Exit()
			return

try:
	import pickle
except ImportError:
	choice = Blender.Draw.PupMenu("Error: Python installation not found!%t" \
		+ "|This script needs a full Python %d.%d installation." % tuple(sys.version_info[0:2]) \
		+ "|Please refer to the Blender website how to install Python for Blender.|%l" \
		+ "|www.blender.org" \
		+ "|www.python.org")
	try:
		import webbrowser
	except ImportError:
		pass
	else:
		if (choice == 4):
			webbrowser.open("www.blender.org", 1, 1)
		elif (choice == 5):
			webbrowser.open("www.python.org", 1 , 1)
else:
	# force reload of modules
	for name in sys.modules.keys()[:]:
		if name[0:7] == 'ogrepkg':
			del sys.modules[name]
	opi = OgrePackageImporter()
	try:
		opi.importPackage() # import ogrepkg
	except ImportError:
		opi.requestPackagePath()
	else:
		import webbrowser
		# ogrepkg successfully imported
		from ogrepkg import *
		from ogrepkg.base import *
		from ogrepkg.gui import *
		from ogrepkg.meshexport import *
		from ogrepkg.materialexport import *
		
		PackageSettings.getSingleton().load()
		# modules stay in memory, therefore update settings and clear log manually
		##Log.getSingleton().clear()
		
		class PreferencesScreen(Screen):
			def __init__(self):
				Screen.__init__(self)
				# none or converter
				self.converter = ValueModel(OgreXMLConverter.getSingleton().getConverter())
				# always valid string '' if no additional arguments
				self.converterArgs = ValueModel(OgreXMLConverter.getSingleton().getAdditionalArguments())
				frame = OgreFrame(self, "Preferences")
				layout = VerticalLayout(frame)
				cbox = Box(layout, L("OgreXMLConverter"), 0 , 10)
				cbLayout = VerticalLayout(cbox)
				locationLayout = VerticalLayout(cbLayout, True)
				Spacer(cbLayout, Size([0, 20]))
				optionLayout = VerticalLayout(cbLayout, True)
				# converter location
				self.locationView = PreferencesScreen.ConverterLocationView(locationLayout, self.converter)
				# additional arguments
				self.nuextremityPoints = BoundedValueModel(0, 65536, OgreXMLConverter.getSingleton().getNuExtremityPoints())
				self.generateEdgeLists = ToggleModel(OgreXMLConverter.getSingleton().getGenerateEdgeLists())
				self.generateTangents = ToggleModel(OgreXMLConverter.getSingleton().getGenerateTangents())
				self.tangentSemantic = ValueModel(OgreXMLConverter.getSingleton().getTangentSemantic())
				self.tangentUseParity = ValueModel(OgreXMLConverter.getSingleton().getTangentUseParity())
				self.tangentSplitMirrored = ToggleModel(OgreXMLConverter.getSingleton().getTangentSplitMirrored())
				self.tangentSplitRotated = ToggleModel(OgreXMLConverter.getSingleton().getTangentSplitRotated())
				self.reorganiseBuffers = ToggleModel(OgreXMLConverter.getSingleton().getReorganiseBuffers())
				self.optimiseAnimations = ToggleModel(OgreXMLConverter.getSingleton().getOptimiseAnimations())
				LabelView(optionLayout, L("Export options:"))
				arg1Layout = HorizontalLayout(optionLayout)
				NumberView(arg1Layout, Size([Size.INFINITY, 20], [100, 20]), self.nuextremityPoints, T("Extremity Points: "))
				ToggleView(arg1Layout, Size([Size.INFINITY, 20], [100, 20]), self.generateEdgeLists, T("Edge Lists"))
				ToggleView(arg1Layout, Size([Size.INFINITY, 20], [101, 20]), self.generateTangents, T("Tangent"))
				arg2Layout = HorizontalLayout(optionLayout)
				tangentSemanticMenu = Menu(arg2Layout, Size([Size.INFINITY, 20], [75, 20]), T("Selects the tangent semantic destination"))
				tangentSemanticMenu.appendItem(MenuTitle("Tangent semantic"))
				tangentSemanticMenu.appendItem(MenuItem("tangent", PreferencesScreen.OptionMenuSelectAction(self.tangentSemantic, 'tangent')), \
					self.tangentSemantic.getValue() == 'tangent')
				tangentSemanticMenu.appendItem(MenuItem("uvw", PreferencesScreen.OptionMenuSelectAction(self.tangentSemantic, 'uvw')), \
					self.tangentSemantic.getValue() != 'tangent')
				tangentParityMenu = Menu(arg2Layout, Size([Size.INFINITY, 20], [75, 20]), T("Selects the tangent size"))
				tangentParityMenu.appendItem(MenuTitle("Tangent size"))
				tangentParityMenu.appendItem(MenuItem("3 component", PreferencesScreen.OptionMenuSelectAction(self.tangentUseParity, '3')), \
					self.tangentUseParity.getValue() == '3')
				tangentParityMenu.appendItem(MenuItem("4 component (parity)", PreferencesScreen.OptionMenuSelectAction(self.tangentUseParity, '4')), \
					self.tangentUseParity.getValue() != '3')
				ToggleView(arg2Layout, Size([Size.INFINITY, 20], [75, 20]), self.tangentSplitMirrored, T("Split Mirrored"))
				ToggleView(arg2Layout, Size([Size.INFINITY, 20], [75, 20]), self.tangentSplitRotated, T("Split Rotated"))
				arg3Layout = HorizontalLayout(optionLayout)
				ToggleView(arg3Layout, Size([Size.INFINITY, 20], [150, 20]), self.reorganiseBuffers, T("Reorganise vertex buffers"))
				ToggleView(arg3Layout, Size([Size.INFINITY, 20], [150, 20]), self.optimiseAnimations, T("Optimise animations"))

				Spacer(cbLayout, Size([0, 20]))
				LabelView(cbLayout, L("Additional arguments:"))
				StringView(cbLayout, Size([Size.INFINITY, 20], [200, 20]), self.converterArgs, T("Arguments: "), \
						T("Additional arguments in call to OgreXMLConverter."))
				Spacer(layout, Size([0, Size.INFINITY], [0, 0]))
				# button row
				Spacer(layout, Size([0, 10]))
				bhLayout = HorizontalLayout(layout)
				bSize = Size([Size.INFINITY, 30], [Blender.Draw.GetStringWidth('Preferences')+10, 30])
				Button(bhLayout, bSize, PreferencesScreen.OkAction(self), T("Ok"), \
					T("Apply and save settings."))
				Spacer(bhLayout, bSize)
				Spacer(bhLayout, bSize)
				Button(bhLayout, bSize, PreferencesScreen.CancelAction(self), T("Cancel"), \
					T("Restore old settings."))
				return
			def activate(self):
				self.converter.setValue(OgreXMLConverter.getSingleton().getConverter())
				self.converterArgs.setValue(OgreXMLConverter.getSingleton().getAdditionalArguments())
				self.locationView.activate()
				Screen.activate(self)
				return
			def applySettings(self):
				"""Apply settings.
				"""
				OgreXMLConverter.getSingleton().setConverter(self.converter.getValue())
				OgreXMLConverter.getSingleton().setNuExtremityPoints(self.nuextremityPoints.getValue())
				OgreXMLConverter.getSingleton().setGenerateEdgeLists(self.generateEdgeLists.getValue())
				OgreXMLConverter.getSingleton().setGenerateTangents(self.generateTangents.getValue())
				OgreXMLConverter.getSingleton().setTangentSemantic(self.tangentSemantic.getValue())
				OgreXMLConverter.getSingleton().setTangentUseParity(self.tangentUseParity.getValue())
				OgreXMLConverter.getSingleton().setTangentSplitMirrored(self.tangentSplitMirrored.getValue())
				OgreXMLConverter.getSingleton().setTangentSplitRotated(self.tangentSplitRotated.getValue())
				OgreXMLConverter.getSingleton().setReorganiseBuffers(self.reorganiseBuffers.getValue())
				OgreXMLConverter.getSingleton().setOptimiseAnimations(self.optimiseAnimations.getValue())
				OgreXMLConverter.getSingleton().setAdditionalArguments(self.converterArgs.getValue())
				return
			def discardSettings(self):
				self.locationView.discardSettings()
				return
			class OptionMenuSelectAction(Action):
				def __init__(self, valueModel, value):
					self.valueModel = valueModel
					self.value = value
					return
				def execute(self):
					self.valueModel.setValue(self.value)
					return
			class ConverterLocationView(View):
				def __init__(self, layout, converterModel):
					self.converter = converterModel
					# "Location:"
					LabelView(layout, L("Location:"))
					# [  Auto  ] [ Manual ]
					ctg = ToggleGroup()
					self.cAutoToggle = ToggleModel(True)
					self.cManualToggle = ToggleModel(False)
					ctg.addToggle(self.cAutoToggle)
					ctg.addToggle(self.cManualToggle)
					cthLayout = HorizontalLayout(layout)
					ToggleView(cthLayout, Size([Size.INFINITY, 20], [100, 20]), self.cAutoToggle, T("Auto"), \
						T("Use OgreXMLConverter in Path."))
					ToggleView(cthLayout, Size([Size.INFINITY, 20], [101, 20]), self.cManualToggle, T("Manual"), \
						T("Specifiy OgreXMLConverter location manually."))
					# save choosen path temporarily,
					# so clicking on Auto and back on Manual does not clear it
					self.lastPath = ValueModel('')
					# Nothing or [Converter: ............ ][select] 
					self.alternatives = AlternativesLayout(layout)
					self.altAuto = Spacer(self.alternatives, Size([0, 20]))
					self.altManual = HorizontalLayout(self.alternatives)
					StringView(self.altManual, Size([Size.INFINITY, 20], [200, 20]), \
						PreferencesScreen.ConverterLocationView.ConverterAdapter(self.converter), T("Converter: "), \
						T("OgreXMLConverter executable."))
					Button(self.altManual, Size([70, 20]), PreferencesScreen.ConverterLocationView.SelectAction(self.converter), \
						T("Select"), T("Select the converter location."))
					self.cAutoToggle.addView(self)
					self.activate()					
					return
				def activate(self):
					"""Reset to saved configuration.
					"""
					if (self.converter.getValue() == None):
						# restore last path
						self.converter.setValue(self.lastPath.getValue())
						# select auto (sets converter back to "None")
						self.cAutoToggle.setValue(True)
						self.alternatives.setCurrent(self.altAuto)
					else:
						self.cManualToggle.setValue(True)
						self.alternatives.setCurrent(self.altManual)						
					return
				def update(self):
					"""Called by cAutoToggle.
					"""
					if self.cAutoToggle.getValue():
						self.alternatives.setCurrent(self.altAuto)
						self.lastPath.setValue(self.converter.getValue())
						self.converter.setValue(None)
					else:
						self.converter.setValue(self.lastPath.getValue())
						self.alternatives.setCurrent(self.altManual)
					return
				def discardSettings(self):
					self.lastPath.setValue(self.converter.getValue())
					return
				class ConverterAdapter(ValueModel):
					def __init__(self, model):
						Model.__init__(self)
						self.model = model
						return
					def setValue(self, value):
						self.model.setValue(value)
						self._notify()
						return
					def getValue(self):
						value = self.model.getValue()
						if value is None:
							value = ''
						return value
				class SelectAction(Action):
					def __init__(self, converterModel):
						self.converter = converterModel
						return
					def execute(self):
						if( self.converter.getValue() == None):
							self.converter.setValue('')
						Blender.Window.FileSelector(self.converter.setValue, "Select Converter", self.converter.getValue())
						return
			class OkAction(Action):
				def __init__(self, screen):
					self.screen = screen
					return
				def execute(self):
					self.screen.applySettings()
					self.screen.deactivate()
					return
			class CancelAction(Action):
				def __init__(self, screen):
					self.screen = screen
					return
				def execute(self):
					self.screen.discardSettings()
					self.screen.deactivate()
					return
		
		class ArmatureAction:
			"""Wrapper for Blender actions.
			"""
			def __init__(self, bAction):
				self.bAction = bAction
				self.firstFrame = None
				self.lastFrame = None
				self.animationProxyList = []
				self.update()
			def invalidate(self):
				"""Invalidate all ArmatureAnimationProxys as Blender actions does no longer exist or affect the mesh.
				
				   Called by ActionManager. Calls ArmatureAnimationProxy.invalidate() on all attached
				   ArmatureAnimationProxies.
				"""
				# copy list as proxy.invalidate() calls detachAnimationProxy(proxy)
				for proxy in self.animationProxyList[:]:
					proxy.invalidate()
				return
			def attachAnimationProxy(self, proxy):
				"""Attaches ArmatureAnimationProxy to this action.
				
				   All attached actions are notified to invalidate themselves if this
				   ArmatureAction gets invalidated.
				   
				   This method is called by the ArmatureAnimationProxy constructor.
				   @see ArmatureAnimationProxy
				"""
				self.animationProxyList.append(proxy)
				return
			def detachAnimationProxy(self, proxy):
				"""Detaches ArmatureAnimationProxy from this action.
				
				   The proxy is removed from the list of attached animations and thus no
				   longer invalidated if this ArmatureAction gets invalidated.
				   
				   This method is called by the ArmatureAnimationProxyManager.removeProxy() method.
				
				   @see attachAnimationProxy()
				   @see ArmatureAnimationProxyManager
				"""
				if proxy in self.animationProxyList:
					self.animationProxyList.remove(proxy)
				return
			def getName(self):
				return self.bAction.getName()
			def getFirstFrame(self):
				return self.firstFrame
			def getLastFrame(self):
				return self.lastFrame
			def getBlenderAction(self):
				return self.bAction
			def hasEffectOn(self, bArmature):
				hasEffect = False
				actionIpoDict = self.bAction.getAllChannelIpos()
				armatureBoneIter = iter(bArmature.bones.keys())
				try:
					while not hasEffect:
						boneName = armatureBoneIter.next()
						if actionIpoDict.has_key(boneName):
							# TODO: check for curves
							hasEffect = True
				except StopIteration:
					pass
				return hasEffect
			def update(self):
				"""Updates firstKeyFrame and lastKeyFrame considering the current IpoCurves.
				"""
				firstKeyFrame = None
				lastKeyFrame = None
				ipoDict = self.bAction.getAllChannelIpos()
				if ipoDict is not None:
					# check all bone Ipos
					# ipoDict[boneName] = Blender.Ipo
					for ipo in ipoDict.values():
						if ipo is not None:
							# check all IpoCurves
							for ipoCurve in ipo.getCurves():
								# check first and last keyframe
								for bezTriple in ipoCurve.getPoints():
									iFrame = bezTriple.getPoints()[0]
									if ((iFrame < firstKeyFrame) or (firstKeyFrame is None)):
										firstKeyFrame = iFrame
									if ((iFrame > lastKeyFrame) or (lastKeyFrame is None)):
										lastKeyFrame = iFrame
				if firstKeyFrame == None:
					firstKeyFrame = 1
				if lastKeyFrame == None:
					lastKeyFrame = 1
				self.firstFrame = firstKeyFrame
				self.lastFrame = lastKeyFrame
				return
		
		class ActionManager:
			"""Manages Blender actions.
			"""
			def __init__(self):
				# key: name, value: ArmatureAction
				self.armatureActionDict = {}
				self.update()
				return
			def update(self):
				"""Synchronizes with Blender.
				
				   Updates available actions and default keyframe ranges.
				   Invalidates actions that are no longer available.
				   
				   update() calls Armature.Action.invalidate(), which in turn calls
				   ArmatureAnimationProxy.invalidate(), which in turn calls
				   ArmatureAnimationProxyMangaer.removeProxy().
				"""
				# popultate armatureActionDict
				# get dictionary (name, Blender Action) of valid blender actions
				bActionDict = Blender.Armature.NLA.GetActions()
				#TODO: ?Check for curve and beztriple?
				# invalidate and remove deleted actions
				for actionName in [name for name in self.armatureActionDict.keys() if name not in bActionDict.keys()]:
					self.armatureActionDict[actionName].invalidate()
					del self.armatureActionDict[actionName]
				# update remaining actions
				for action in self.armatureActionDict.values():
					action.update()
				# create new actions
				for bAction in [action for (name, action) in bActionDict.iteritems() if name not in self.armatureActionDict.keys()]:
					self.armatureActionDict[bAction.getName()] = ArmatureAction(bAction)
				return
			def getAction(self, name):
				"""Returns ArmatureAction for Blender action with given name.
				"""
				action = None
				if name in self.armatureActionDict.keys():
					action = self.armatureActionDict[name]
				return action
			def getActions(self, bObject):
				"""Returns list of actions that have effect on the given Blender mesh object.
				
				   If the mesh is linked to an action that has an effect on the meshes armature,
				   this action is the first element of the returned action list.
				   
				   @param bObject Blender mesh object.
				   @return list of ArmatureActions that have an effect on the given mesh object.
				"""
				actionList = []
				# get available actions
				parent = GetArmatureObject(bObject)
				if (parent is not None):
					actionList = [action for action in self.armatureActionDict.values() if action.hasEffectOn(parent.getData(mesh=True))]
				# move linked action to first position in returned list
				bAction = bObject.getAction()
				if bAction is not None:
					# linked
					if self.armatureActionDict.has_key(bAction.getName()):
						# valid
						linkedAction = self.armatureActionDict[bAction.getName()]
						if linkedAction in actionList:
							# has effect on mesh, move to first position in list
							actionList.remove(linkedAction)
							actionList.insert(0, linkedAction)
				return actionList
		
		class AnimationProxy(Model, View):
			"""Base class that represents an animation.
			"""
			def __init__(self, name='', startFrame=1, endFrame=1):
				Model.__init__(self)
				# Model
				self.nameModel = ValueModel(name)
				self.nameModel.addView(self)
				self.startFrameModel = BoundedValueModel(1, 30000, startFrame)
				self.startFrameModel.addView(self)
				self.endFrameModel = BoundedValueModel(1, 30000, endFrame)
				self.endFrameModel.addView(self)
				return	
			def getName(self):
				return self.nameModel.getValue()
			def getStartFrame(self):
				return self.startFrameModel.getValue()
			def getEndFrame(self):
				return self.endFrameModel.getValue()
			def update(self):
				self._notify()
				return
			def toAnimation(self):
				"""Returns a corresponding animation.
				"""
				raise NotImplementedError
				return		
		
		class PoseAnimationProxy(AnimationProxy):
			def toAnimation(self):
				return PoseAnimation(self.nameModel.getValue(), self.startFrameModel.getValue(), self.endFrameModel.getValue())
			
		class MorphAnimationProxy(AnimationProxy):
			def toAnimation(self):
				return MorphAnimation(self.nameModel.getValue(), self.startFrameModel.getValue(), self.endFrameModel.getValue())
		
		class AnimationProxyView(HorizontalLayout, View):
			def __init__(self, parent, model, deleteAction):
				HorizontalLayout.__init__(self, parent, True)
				View.__init__(self, model)
				NumberView(self, Size([100, 20]), self.model.startFrameModel, T("Start: "), T("Start frame"))
				NumberView(self, Size([100, 20]), self.model.endFrameModel, T("End: "), T("End frame"))
				StringView(self, Size([Size.INFINITY, 20], [80, 20]), self.model.nameModel, tooltip=T("Animation name in Ogre3D"))
				Button(self, Size([60, 20]), deleteAction, T("Delete"), T("Delete animation from export"))
				return
		
		class ArmatureAnimationProxy(AnimationProxy):
			"""Represents an ArmatureAnimation.
			"""
			def __init__(self, manager, action=None, name='', startFrame=-1, endFrame=-1):
				"""Constructor.
				
				   @param manager ArmatureAnimationProxyManager
				   @param action Armature action. If <code>None</code> the default action is used.
				                 In this case, the manager must provide at least one action.
				   @param name Animation name. If an empty string is passed, the name is set to 
				               the name of the action.
				   @param startFrame Start frame of animation. If set to -1, it is set to the 
				                     first keyframe for the action .
				   @param endFrame End frame of animation. If set to -1, it is set to the last
				                   keyframe for the action.
				"""
				self.manager = manager
				# action
				if action is None:
					self.actionModel = ArmatureAnimationProxy.ActionValueModel(self, self.manager.getActions()[0])
				else:
					self.actionModel = ArmatureAnimationProxy.ActionValueModel(self, action)
				self.actionModel.addView(self)
				if (name == ''):
					name = self.actionModel.getValue().getName()
				if (startFrame == -1):
					startFrame = self.actionModel.getValue().getFirstFrame()
				if (endFrame == -1):
					endFrame = self.actionModel.getValue().getLastFrame()
				AnimationProxy.__init__(self, name, startFrame, endFrame)
				return
			def invalidate(self):
				"""Invalidate animation as action does no longer exist or affect the mesh.
				
				   Called by ArmatureAction. It invalidates itself by calling
				   ArmatureAnimationProxyManager.removeProxy().
				"""
				self.manager.removeProxy(self.manager.getKey(proxy))
				return
			def getAction(self):
				return self.actionModel.getValue()
			def setAction(self, action):
				self.actionModel.setValue(action)
				return
			def getActionChoices(self):
				return self.manager.getActions()
			def toAnimation(self):
				"""Returns the corresponding armature animation.
				"""
				return ArmatureAnimation(self.actionModel.getValue().getBlenderAction(), \
					self.nameModel.getValue(), self.startFrameModel.getValue(), self.endFrameModel.getValue())
			class ActionValueModel(ValueModel):
				def __init__(self, armatureAnimationProxy, armatureAction):
					"""Constructor.
					
					   Attaches ArmatureAnimationProxy to ArmatureAction.
					"""
					Model.__init__(self)
					self.proxy = armatureAnimationProxy
					# do not call setValue() in constructor as it relies on other proxy models
					# and it doesn't have a view yet.
					self.value = armatureAction
					self.value.attachAnimationProxy(self.proxy)
					return
				def setValue(self, value):
					"""Set to new action.
					
					   Detaches proxy on old action and attaches proxy on new action.
					"""
					if (value != self.value):
						self.value.detachAnimationProxy(self.proxy)
						value.attachAnimationProxy(self.proxy)
						ValueModel.setValue(self, value)
						# reset animation properties to action's default.
						self.proxy.nameModel.setValue(value.getName())
						self.proxy.startFrameModel.setValue(value.getFirstFrame())
						self.proxy.endFrameModel.setValue(value.getLastFrame())
					return
		
		class ArmatureAnimationProxyView(HorizontalLayout, View):
			def __init__(self, parent, model, deleteAction):
				HorizontalLayout.__init__(self, parent, True)
				View.__init__(self, model)
				# action menu
				self.menu = Menu(self, Size([100, 20]), T("Blender Action"))
				NumberView(self, Size([100, 20]), self.model.startFrameModel, T("Start: "), T("Start frame"))
				NumberView(self, Size([100, 20]), self.model.endFrameModel, T("End: "), T("End frame"))
				StringView(self, Size([Size.INFINITY, 20], [80, 20]), self.model.nameModel, tooltip=T("Animation name in Ogre3D"))
				Button(self, Size([60, 20]), deleteAction, T("Delete"), T("Delete animation from export"))
				self.update()
				return
			def update(self):
				# rebuild action menu
				self.menu.removeAll()
				self.menu.appendItem(MenuTitle("Action"))
				# current available actions
				choices = self.model.getActionChoices()
				current = self.model.getAction()
				for action in choices:
					if action == current:
						isSelected = True
					else:
						isSelected = False
					self.menu.appendItem(MenuItem(action.getName(), \
						ArmatureAnimationProxyView.SelectAction(self, action)), isSelected)
				return
			class SelectAction(Action):
				def __init__(self, view, action):
					self.view = view
					self.action = action
					return
				def execute(self):
					self.view.model.setAction(self.action)
					return
			
		class AnimationProxyManager(Model):
			"""Base class for managing animations of a given Blender mesh object.
			"""
			def __init__(self, bObject):
				Model.__init__(self)
				self.bObject = bObject
				# list of keys determines ordering
				self.animationProxyKeyList = []
				# all keys < maxKey
				self.maxKey = 0
				# key: key, value: xAnimationProxy
				self.animationProxyDict = {}
				return
			def addProxy(self, proxy):
				"""Adds an AnimationProxy to the manager.
				
				   @param proxy AnimationProxy.
				   @return Key.
				"""
				if (len(self.animationProxyKeyList) == self.maxKey):
					self.maxKey += 1
					key = self.maxKey
				else:
					# first unused
					key = [(x + 1) for x in range(self.maxKey) if (x + 1) not in self.animationProxyDict.keys()][0]
				self.animationProxyDict[key] = proxy
				self.animationProxyKeyList.append(key)
				for view in self.viewList:
					view.notifyAdded(key)
				return key
			def removeProxy(self, key):
				if self.animationProxyDict.has_key(key):
					del self.animationProxyDict[key]
					self.animationProxyKeyList.remove(key)
					for view in self.viewList:
						view.notifyRemoved(key)
				return
			def getBlenderObject(self):
				return self.bObject
			def getKey(self, proxy):
				
				return
			def getKeyList(self):
				return self.animationProxyKeyList
			def getProxy(self, key):
				proxy = None
				if key in self.animationProxyDict.keys():
					proxy = self.animationProxyDict[key]
				return proxy
			def toAnimations(self, animationExporter):
				for key in self.animationProxyKeyList:
					animationExporter.addAnimation(self.animationProxyDict[key].toAnimation())
				return
			def update(self):
				"""Synchronize with Blender.
				
				   @param <code>True</code> if animations can still be exported, else <code>False</code>.
				"""
				return
			def loadPackageSettings(self):
				return
			def savePackageSettings(self):
				return
			# @staticmethod
			# def create( ... ):
			# 	"""Factory method.
			#	"""
		
		class PoseAnimationProxyManager(AnimationProxyManager):
			"""Manages pose animations of a given Blender mesh object.
			"""
			def __init__(self, bMeshObject):
				AnimationProxyManager.__init__(self, bMeshObject)
				# load package settings if available
				self.loadPackageSettings()
				return
			def toAnimations(self, animationExporter):
				for key in self.animationProxyKeyList:
					animationExporter.addPoseAnimation(self.animationProxyDict[key].toAnimation())
				return
			def loadPackageSettings(self):
				# clear old
				self.animationProxyKeyList = []
				self.maxKey = 0
				self.animationProxyDict = {}
				# get package settings
				poseAnimationList = PackageSettings.getSingleton().getObjectSetting(self.bObject.getName(), 'PoseAnimationList')
				# old naming convention
				if not poseAnimationList:
					poseAnimationList = PackageSettings.getSingleton().getObjectSetting(self.bObject.getName(), 'poseAnimationList')
				if poseAnimationList:
					for poseAnimation in poseAnimationList:
						# skip old single frame pose animations
						if (len(poseAnimation) == 3):
							self.addProxy(PoseAnimationProxy(poseAnimation[0], poseAnimation[1], poseAnimation[2]))
				self._notify()
				return
			def savePackageSettings(self):
				poseAnimationList = []
				for key in self.animationProxyKeyList:
					proxy = self.animationProxyDict[key]
					poseAnimationList.append([proxy.getName(), proxy.getStartFrame(), proxy.getEndFrame()])
				PackageSettings.getSingleton().setObjectSetting(self.bObject.getName(), 'PoseAnimationList', poseAnimationList)
				return
			def update(self):
				"""Checks if Blender object has still shape keys.
				
				   @return <code>True</code> if shape keys still present, else <code>False</code>.
				"""
				isValid = False
				bKey = self.bObject.getData(mesh=True).key
				if bKey is not None:
					if (len(bKey.blocks) > 0):
						isValid = True
				return isValid
			# @staticmethod
			def create(bMeshObject):
			 	"""Factory method:
			 		
			 	   @return instance or <code>None</code> if object has no shape key.
				"""
				manager = None
				bKey =  bMeshObject.getData(mesh=True).key
				if bKey is not None:
					if (len(bKey.blocks) > 0):
						manager = PoseAnimationProxyManager(bMeshObject)
				return manager
			create = staticmethod(create)
				
		class MorphAnimationProxyManager(AnimationProxyManager):
			"""Manages morph animations of a given Blender mesh object.
			"""
			def __init__(self, bMeshObject):
				AnimationProxyManager.__init__(self, bMeshObject)
				# load package settings if available
				self.loadPackageSettings()
				return
			def toAnimations(self, animationExporter):
				for key in self.animationProxyKeyList:
					animationExporter.addMorphAnimation(self.animationProxyDict[key].toAnimation())
				return
			def loadPackageSettings(self):
				# clear old
				self.animationProxyKeyList = []
				self.maxKey = 0
				self.animationProxyDict = {}
				# get package settings
				morphAnimationList = PackageSettings.getSingleton().getObjectSetting(self.bObject.getName(), 'MorphAnimationList')
				# old naming convention
				if not morphAnimationList:
					morphAnimationList = PackageSettings.getSingleton().getObjectSetting(self.bObject.getName(), 'morphAnimationList')
				if morphAnimationList:
					for morphAnimation in morphAnimationList:
						self.addProxy(MorphAnimationProxy(morphAnimation[0], morphAnimation[1], morphAnimation[2]))
				self._notify()
				return
			def savePackageSettings(self):
				morphAnimationList = []
				for key in self.animationProxyKeyList:
					proxy = self.animationProxyDict[key]
					morphAnimationList.append([proxy.getName(), proxy.getStartFrame(), proxy.getEndFrame()])
				PackageSettings.getSingleton().setObjectSetting(self.bObject.getName(), 'MorphAnimationList', morphAnimationList)
				return
			def update(self):
				"""Checks if Blender object has still shape keys.
				
				   @return <code>True</code> if shape keys still present, else <code>False</code>.
				"""
				isValid = False
				bKey = self.bObject.getData(mesh=True).key
				if bKey is not None:
					if (len(bKey.blocks) > 0):
						isValid = True
				return isValid
			# @staticmethod
			def create(bMeshObject):
			 	"""Factory method:
			 		
			 	   @return instance or <code>None</code> if object has no shape key.
				"""
				manager = None
				bKey =  bMeshObject.getData(mesh=True).key
				if bKey is not None:
					if (len(bKey.blocks) > 0):
						manager = MorphAnimationProxyManager(bMeshObject)
				return manager
			create = staticmethod(create)
		
		class AnimationProxyManagerView(AddWidgetListLayout, View):
			"""Base class for view for AnimationProxyMangager.
			"""
			def __init__(self, parent, size, model, addAction, addTooltip):
				# Model
				View.__init__(self, model)
				# View
				AddWidgetListLayout.__init__(self, parent, size)
				Button(self, Size([60, 20]), addAction, T("Add"), addTooltip)
				# list of displayed keys
				self.displayedKeyList = []
				# key: key, value: AnimationProxyView
				self.proxyViewDict = {}
				self.update()
				self.setAutoScroll(True)
				return
			def notifyAdded(self, key):
				proxyView = AnimationProxyView(self, self.model.getProxy(key), AnimationProxyManagerView.DeleteAction(self.model, key))
				self.proxyViewDict[key] = proxyView
				self.displayedKeyList.append(key)
				Blender.Draw.Redraw(1)
				return
			def update(self):
				self.removeAll()
				self.displayedKeyList = []
				for key in self.model.getKeyList():
					self.notifyAdded(key)
				Blender.Draw.Redraw(1)
				return
			def notifyRemoved(self, key):
				self.displayedKeyList.remove(key)
				self.proxyViewDict[key].removeFromParent()
				del self.proxyViewDict[key]
				Blender.Draw.Redraw(1)
				return
			class DeleteAction(Action):
				def __init__(self, model, key):
					self.model = model
					self.key = key
					return
				def execute(self):
					self.model.removeProxy(self.key)
					return
		
		class PoseAnimationProxyManagerView(AnimationProxyManagerView):
			"""View for PoseAnimationProxyManager.
			
			   Set size greater than <code>Size([350,100])</code>.
			"""
			def __init__(self, parent, size, model):
				AnimationProxyManagerView.__init__(self, parent, size, model, \
					PoseAnimationProxyManagerView.AddAction(model), \
					T("Add new pose animation."))
				return
			class AddAction(Action):
				def __init__(self, model):
					self.model = model
					return
				def execute(self):
					key = self.model.addProxy(PoseAnimationProxy('name', 1, 1))
					return
			
		class MorphAnimationProxyManagerView(AnimationProxyManagerView):
			"""View for MorphAnimationProxyManager.
			
			   Set size greater than <code>Size([350,100])</code>.
			"""
			def __init__(self, parent, size, model):
				AnimationProxyManagerView.__init__(self, parent, size, model, \
					MorphAnimationProxyManagerView.AddAction(model), \
					T("Add new morph animation."))
				return
			class AddAction(Action):
				def __init__(self, model):
					self.model = model
					return
				def execute(self):
					key = self.model.addProxy(MorphAnimationProxy('name', 1, 1))
					return
		
		class ArmatureAnimationProxyManager(AnimationProxyManager):
			"""Manages armature animations of a given Blender mesh object.
			"""
			def __init__(self, bMeshObject, actionManager):
				AnimationProxyManager.__init__(self, bMeshObject)
				# actions
				self.actionManager = actionManager
				self.actionList = []
				self.update()
				# load package settings if available
				self.loadPackageSettings()
				return
			def removeProxy(self, key):
				"""Removes proxy from ProxyManager as well as detaches it from ActionManager.
				"""
				if self.animationProxyDict.has_key(key):
					proxy = self.animationProxyDict[key]
					proxy.getAction().detachAnimationProxy(proxy)
					del self.animationProxyDict[key]
					self.animationProxyKeyList.remove(key)
					for view in self.viewList:
						view.notifyRemoved(key)
				return
			def removeProxies(self):
				"""Removes all animation proxies.
				"""
				for proxy in self.animationProxyDict.values():
					proxy.getAction().detachAnimationProxy(proxy)
				self.animationProxyDict = {}
				return
			def update(self):
				"""Updates available actions and default action keyframe ranges.
				
				   Only notifies views if it is still a valid manager.
				   @return <code>True</code> if armature and actions are still present,
				           else <code>False</code>.
				"""
				isValid = False
				self.actionList = []
				# check if mesh object has (still) an armature is done by getActions
				self.actionList = self.actionManager.getActions(self.bObject)
				if (len(self.actionList) > 0):
					isValid = True
				return isValid
			def loadPackageSettings(self):
				# clear old
				for proxy in self.animationProxyDict.values():
					proxy.invalidate()		
				self.animationProxyKeyList = []
				self.maxKey = 0
				self.animationProxyDict = {}
				# get package settings
				animationList = PackageSettings.getSingleton().getObjectSetting(self.bObject.getName(), 'ArmatureAnimationList')
				if not animationList:
					# load old configuration text
					animationList = self._loadOldSettings()
				if animationList and len(animationList):
					validActionNames = [action.getName() for action in self.actionList]
					for animation in animationList:
						# animation = [action name, animation name, start frame, end frame]
						# check if action is still available
						if animation[0] in validActionNames:
							# add animiation
							action = self.actionManager.getAction(animation[0])
							if action:
						 		self.addProxy(ArmatureAnimationProxy(self, action, animation[1], animation[2], animation[3]))
				self._notify()
				return
			def savePackageSettings(self):
				animationList = []
				for key in self.animationProxyKeyList:
					proxy = self.animationProxyDict[key]
					animationList.append([proxy.getAction().getName(), proxy.getName(), proxy.getStartFrame(), proxy.getEndFrame()])
				PackageSettings.getSingleton().setObjectSetting(self.bObject.getName(), 'ArmatureAnimationList', animationList)
				return
			def toAnimations(self, animationExporter):
				for key in self.animationProxyKeyList:
					animationExporter.addAnimation(self.animationProxyDict[key].toAnimation())
				return
			def getActions(self):
				return self.actionList
			def _loadOldSettings(self):
				"""Load animation settings from the old exporter.
				
				   @return list of animations in the new format or <code>None</code>.
				"""
				animationList = None
				# try open 'ogreexport.cfg' text
				configTextName = 'ogreexport.cfg'
				if configTextName in [text.getName() for text in Blender.Text.Get()]:
					configText = Blender.Text.Get(configTextName)
					# compose string from text and unpickle
					try:
						# unpickle
						settingsDict = pickle.loads(string.join(configText.asLines()[4:],'\n'))
					except (PickleError):
						pass
					else:
						# read configuration
						if settingsDict.has_key('armatureAnimationDictListDict'):
							armatureAnimationDictListDict = settingsDict['armatureAnimationDictListDict']
							parent = GetArmatureObject(self.bObject)
							if (parent is not None):
								if armatureAnimationDictListDict.has_key(parent.getName()):
									armatureAnimationDictList = armatureAnimationDictListDict[parent.getName()]
									animationList = []
									for animationDict in armatureAnimationDictList:
										actionName = animationDict['actionKey']
										name = animationDict['name']
										startFrame = animationDict['startFrame']
										endFrame = animationDict['endFrame']
										animationList.append([actionName, name, startFrame, endFrame])
				return animationList
			# @staticmethod
			def create(bMeshObject, actionManager):
			 	"""Factory method:
			 		
			 	   @return instance or <code>None</code> if object has no shape key.
				"""
				manager = None
				if (len(actionManager.getActions(bMeshObject)) > 0):
					manager = ArmatureAnimationProxyManager(bMeshObject, actionManager)
				return manager
			create = staticmethod(create)
			
		class ArmatureAnimationProxyManagerView(AnimationProxyManagerView):
			"""View for MorphAnimationProxyManager.
			
			   Set size greater than <code>Size([350,100])</code>.
			"""
			def __init__(self, parent, size, model):
				AnimationProxyManagerView.__init__(self, parent, size, model, \
					ArmatureAnimationProxyManagerView.AddAction(model), \
					T("Add new skeleton animation."))
				return
			def notifyAdded(self, key):
				proxyView = ArmatureAnimationProxyView(self, self.model.getProxy(key), AnimationProxyManagerView.DeleteAction(self.model, key))
				self.proxyViewDict[key] = proxyView
				self.displayedKeyList.append(key)
				Blender.Draw.Redraw(1)
				return
			class AddAction(Action):
				def __init__(self, model):
					self.model = model
					return
				def execute(self):
					key = self.model.addProxy(ArmatureAnimationProxy(self.model))
					return
		
		class SelectedObjectManager(Model):
			"""Manages mesh objects selected for export.
			
			   Views have to provide an updateSelection() method.
			"""
			def __init__(self):
				Model.__init__(self)
				# ActionManager takes care of actions for all objects
				self.actionManager = ActionManager()
				# object names
				self.selectedList = []
				# key: name, value: Blender Object of type Mesh
				self.selectedDict = {}
				# name of current selected object
				self.current = None
				# key: name; value: MorphAnimationProxyManager
				self.morphAnimationProxyManagerDict = {}
				# key: name; value: PoseAnimationProxyMangager
				self.poseAnimationProxyManagerDict = {}
				# key: name; value: ArmatureAnimationProxyManager
				self.armatureAnimationProxyManagerDict = {}
				self.updateSelection()
				return
			def updateSelection(self):
				"""Update the list of objects selected for export and the available actions for these objects.
				"""
				# update available actions
				self.actionManager.update()
				self.selectedList = []
				self.selectedDict = {}
				# for every currently selected mesh object
				for bMeshObject in [bObject for bObject in Blender.Object.GetSelected() if (bObject.getType() == 'Mesh')]:
					name = bMeshObject.getData(True)
					if self.selectedDict.has_key(name):
						continue
					self.selectedList.append(name)
					self.selectedDict[name] = bMeshObject
					# morph animation
					if self.morphAnimationProxyManagerDict.has_key(name):
						# update
						if not self.morphAnimationProxyManagerDict[name].update():
							# manager no longer valid
							del self.morphAnimationProxyManagerDict[name]
					else:
						# create new manager
						manager = MorphAnimationProxyManager.create(bMeshObject)
						if manager:
							self.morphAnimationProxyManagerDict[name] = manager
					# pose animation
					if self.poseAnimationProxyManagerDict.has_key(name):
						# update
						if not self.poseAnimationProxyManagerDict[name].update():
							# manager no longer valid
							del self.poseAnimationProxyManagerDict[name]
					else:
						# create new manager
						manager = PoseAnimationProxyManager.create(bMeshObject)
						if manager:
							self.poseAnimationProxyManagerDict[name] = manager
					# armature animation
					if self.armatureAnimationProxyManagerDict.has_key(name):
						# update
						if not self.armatureAnimationProxyManagerDict[name].update():
							# manager no longer valid
							self.armatureAnimationProxyManagerDict[name].removeProxies()
							del self.armatureAnimationProxyManagerDict[name]
					else:
						# create new manager
						manager = ArmatureAnimationProxyManager.create(bMeshObject, self.actionManager)
						if manager:
							self.armatureAnimationProxyManagerDict[name] = manager
				# == remove AnimationProxyManagers of unselected objects ==
				# save animations of unselected objects
				# remove MorphAnimationProxyManagers of unselected objects
				for (bObjectName, proxyManager) in [(name, self.morphAnimationProxyManagerDict[name]) \
					for name in self.morphAnimationProxyManagerDict.keys()[:] if name not in self.selectedList]:
					proxyManager.savePackageSettings()
					del self.morphAnimationProxyManagerDict[bObjectName]
				# remove PoseAnimationProxyManagers of unselected objects
				for (bObjectName, proxyManager) in [(name, self.poseAnimationProxyManagerDict[name]) \
					for name in self.poseAnimationProxyManagerDict.keys()[:] if name not in self.selectedList]:
					proxyManager.savePackageSettings()
					del self.poseAnimationProxyManagerDict[bObjectName]
				# remove ArmatureAnimationProxyMangers of unselected objects
				for (bObjectName, proxyManager) in [(name, self.armatureAnimationProxyManagerDict[name]) \
					for name in self.armatureAnimationProxyManagerDict.keys()[:] if name not in self.selectedList]:
					proxyManager.savePackageSettings()
					# detach animations from actions
					proxyManager.removeProxies()
					del self.armatureAnimationProxyManagerDict[bObjectName]
				# update current selected
				if len(self.selectedList):
					self.current = self.selectedList[0]
				else:
					self.current = None
				# notify views
				for view in self.viewList:
					view.updateSelection()
				return
			def setCurrentObjectName(self, name):
				if name in self.selectedDict.keys():
					self.current = name
					self._notify()
				return
			def getCurrentObjectName(self):
				return self.current
			def getObject(self, name):
				bObject = None
				if name in self.selectedDict.keys():
					bObject = self.selectedDict[name]
				return bObject
			def getMorphAnimationProxyManager(self, name):
				proxyManager = None
				if name in self.morphAnimationProxyManagerDict.keys():
					proxyManager = self.morphAnimationProxyManagerDict[name]
				return proxyManager
			def getPoseAnimationProxyManager(self, name):
				proxyManager = None
				if name in self.poseAnimationProxyManagerDict.keys():
					proxyManager = self.poseAnimationProxyManagerDict[name]
				return proxyManager
			def getArmatureAnimationProxyManager(self, name):
				proxyManger = None
				if name in self.armatureAnimationProxyManagerDict.keys():
					proxyManger = self.armatureAnimationProxyManagerDict[name]
				return proxyManger
			def getObjectNameList(self):
				return self.selectedList
			def savePackageSettings(self):
				for proxyManager in self.morphAnimationProxyManagerDict.values():
					proxyManager.savePackageSettings()
				for proxyManager in self.poseAnimationProxyManagerDict.values():
					proxyManager.savePackageSettings()
				for proxyManager in self.armatureAnimationProxyManagerDict.values():
					proxyManager.savePackageSettings()
				return
			def export(self, exportPath, exportMaterial, materialScriptName, customMaterial, customMaterialTplPath, colouredAmbient, gameEngineMaterials, exportMesh, fixUpAxis, skeletonUseMeshName, convertXML, copyTextures, requireFaceMats):
				# create MaterialManager
				if len(self.selectedList):
					materialManager = MaterialManager(exportPath, materialScriptName, gameEngineMaterials, customMaterial, customMaterialTplPath, requireFaceMats)
					Log.getSingleton().logInfo("Output to directory \"%s\"" % exportPath)
					for name in self.selectedList:
						Log.getSingleton().logInfo("Processing Object \"%s\"" % name)
						# create MeshExporter
						meshExporter = MeshExporter(self.selectedDict[name], skeletonUseMeshName)
						if self.morphAnimationProxyManagerDict.has_key(name):
							self.morphAnimationProxyManagerDict[name].toAnimations(meshExporter.getVertexAnimationExporter())
						if self.poseAnimationProxyManagerDict.has_key(name):
							self.poseAnimationProxyManagerDict[name].toAnimations(meshExporter.getVertexAnimationExporter())
						if self.armatureAnimationProxyManagerDict.has_key(name):
							self.armatureAnimationProxyManagerDict[name].toAnimations(meshExporter.getArmatureExporter())
						# export
						meshExporter.export(exportPath, materialManager, fixUpAxis, exportMesh, colouredAmbient, convertXML)
					# export materials
					if (exportMaterial):
						materialManager.export(exportPath, materialScriptName, copyTextures)
				else:
					Log.getSingleton().logWarning("No mesh object selected for export!")
				return

		class SelectedObjectManagerView(VerticalLayout, View):
			def __init__(self, parent, model):
				VerticalLayout.__init__(self, parent)
				View.__init__(self, model)
				hLayout = HorizontalLayout(self, True)
				LabelView(hLayout, L("Selected: "))
				self.menu = Menu(hLayout, Size([Size.INFINITY, 20], [150, 20]), T("Objects selected for export."))
				Button(hLayout, Size([70, 20]), SelectedObjectManagerView.UpdateAction(self.model), T("Update"), \
					T("Update list of objects selected for export."))
				Spacer(self, Size([0, 10]))
				self.alternatives = AlternativesLayout(self)
				self.noneSelectedWidget = Spacer(self.alternatives, Size([0, Size.INFINITY], [0, 0]))
				self.alternatives.setCurrent(self.noneSelectedWidget)
				# key: object name, value: AnimationProxyManagersView 
				self.animationProxyManagersViewDict = {}
				self.updateSelection()
				return
			def updateSelection(self):
				"""Called from model if list of selected mesh objects changes.
				"""
				self.menu.removeAll()
				self.alternatives.setCurrent(self.noneSelectedWidget)
				for name in self.model.getObjectNameList():
					# set current object to first in list
					setCurrent = False
					if (name == self.model.getObjectNameList()[0]):
						setCurrent = True
					self.menu.appendItem(MenuItem(name, SelectedObjectManagerView.SelectAction(self.model, name)), setCurrent)
					if not self.animationProxyManagersViewDict.has_key(name):
						poseManager = self.model.getPoseAnimationProxyManager(name)
						morphManager = self.model.getMorphAnimationProxyManager(name)
						armatureManager = self.model.getArmatureAnimationProxyManager(name)
						self.animationProxyManagersViewDict[name] = SelectedObjectManagerView.AnimationProxyManagersView( \
							self.alternatives, self.model, name)
					else:
						# udpate existing
						self.animationProxyManagersViewDict[name].updateSelection()
					# set current object to first in list
					if (name == self.model.getObjectNameList()[0]):
						self.alternatives.setCurrent(self.animationProxyManagersViewDict[name])
				Blender.Draw.Redraw(1)
				return
			def update(self):
				"""Update current selected.
				"""
				self.alternatives.setCurrent(self.animationProxyManagersViewDict[self.model.getCurrentObjectName()])
				Blender.Draw.Redraw(1)
				return
			class SelectAction(Action):
				def __init__(self, model, name):
					self.model = model
					self.name = name
					return
				def execute(self):
					self.model.setCurrentObjectName(self.name)
					return
			class UpdateAction(Action):
				def __init__(self, model):
					self.model = model
					return
				def execute(self):
					self.model.updateSelection()
					return
			class AnimationProxyManagersView(Box, View):
				"""View for the different animation proxy managers.
				
				   Provides toggles to switch between morph, pose and armature animation proxy managers.
				"""
				def __init__(self, parent, manager, name):
					"""Constructor.
					
					   @manager SelectedObjectManager.
					   @name Blender mesh object name.
					"""
					Box.__init__(self, parent, L("Animation Settings of \"%s\"" % name), 0, 10)
					# mesh object name
					self.name = name
					# SelectedObjectManager
					self.manager = manager
					# AnimationProxyManagers
					# [Armature, Pose, Morph]
					self.managerList = [None, None, None]
					# Toggle Bar [Skeleton] [  Pose  ] [  Morph ]
					# If corresponding manager does not exist
					# the toggle is replaced with a Spacer that has size [0, 0].
					# Therefore ToggleView and Spacer are alternatives
					#
					# Only active toggles, i.e. toggles that correspond to an existing
					# AnimationProxyManager are currently in the group
					self.toggleList = [None, None, None] # models
					self.toggleList[0] = ToggleModel(False)
					self.toggleList[1] = ToggleModel(False)
					self.toggleList[2] = ToggleModel(False)
					# self.model = ToggleGroup()
					View.__init__(self, ToggleGroup())	
					vLayout = VerticalLayout(self)
					hLayout = HorizontalLayout(vLayout, True)
					self.alternativesList = [None, None, None]
					self.alternativesList[0] = AlternativesLayout(hLayout)
					self.alternativesList[1] = AlternativesLayout(hLayout)
					self.alternativesList[2] = AlternativesLayout(hLayout)
					self.toggleViewList = [None, None, None]
					self.toggleViewList[0] = ToggleView(self.alternativesList[0], \
						Size([Size.INFINITY, 20], [150, 20]), \
						self.toggleList[0], \
						T("Skeleton"), T("Armature animation to Ogre skeleton animation."))
					self.toggleViewList[1] = ToggleView(self.alternativesList[1], \
						Size([Size.INFINITY, 20], [150, 20]), \
						self.toggleList[1], \
						T("Pose"),T("Shape animation to Ogre pose animation."))
					self.toggleViewList[2] = ToggleView(self.alternativesList[2], \
						Size([Size.INFINITY, 20], [150, 20]), \
						self.toggleList[2], \
						T("Morph"),T("Shape animation to Ogre morph animation."))
					self.spacerList = [None, None, None]
					self.spacerList[0] = Spacer(self.alternativesList[0], Size([0,0]))
					self.alternativesList[0].setCurrent(self.spacerList[0])
					self.spacerList[1] = Spacer(self.alternativesList[1], Size([0,0]))
					self.alternativesList[1].setCurrent(self.spacerList[1])
					self.spacerList[2] = Spacer(self.alternativesList[2], Size([0,0]))
					self.alternativesList[2].setCurrent(self.spacerList[2])
					# lower part of screen shows either the selected AnimationProxyManagerView
					# or a spacer
					self.alternatives = AlternativesLayout(vLayout)
					# AnimationProxyManagerViews
					self.managerViewList = [None, None, None]
					self.spacer = Spacer(self.alternatives, Size([Size.INFINITY, Size.INFINITY], [350, 120]))
					self.alternatives.setCurrent(self.spacer)
					# update managers
					self.updateSelection()
					return
				def update(self):
					"""Updates togglegroup status.
					"""
					if (self.managerList.count(None) != 3):
						toggleModel = self.model.getValue()
						# TODO: test if toggle is enabled should be unnecessary
						if toggleModel and toggleModel.getValue():
							# raises ValueError if model is not in list
							index = self.toggleList.index(toggleModel)
							if self.managerViewList[index] is not None:
								# display current selected AnimationProxyMana
								self.alternatives.setCurrent(self.managerViewList[index])
							else:
								# forgot to remove toggle from ToggleGroup
								# as manager is no longer present
								raise RuntimeError
					return
				def updateSelection(self):
					"""Checks if action managers are still or newly available.
					"""
					newManager = self.manager.getArmatureAnimationProxyManager(self.name)
					if not (self.managerList[0] == newManager):
						# armature manager changed
						if self.managerList[0] is None:
							# create new
							# add manager
							self.managerList[0] = newManager
							# add manager view
							self.managerViewList[0] = ArmatureAnimationProxyManagerView( \
								self.alternatives,  Size([Size.INFINITY, Size.INFINITY], [350, 120]), \
								self.managerList[0])
							# add toggle to toggleGroup, calls update() if selected
							self.model.addToggle(self.toggleList[0])
							# show ToggleView
							self.alternativesList[0].setCurrent(self.toggleViewList[0])
						elif newManager is None:
							# remove old
							# hide ToggleView
							self.alternativesList[0].setCurrent(self.spacerList[0])
							# remove toggle from ToggleGroup
							self.model.removeToggle(self.toggleList[0])
							# set spacer in lower part of the window if no manager left
							if (self.managerList.count(None) >= 2):
								self.alternatives.setCurrent(self.spacer)
							# remove old manager in list
							self.managerList[0] = None
							# remove old managerView
							self.managerViewList[0].removeFromParent()
							self.managerViewList[0] = None
						else:
							# swap
							# manager
							self.managerList[0] = newManager
							# view
							oldView = self.managerViewList[0]
							# add new to alternatives layout
							self.managerViewList[0] = ArmatureAnimationProxyManagerView( \
								self.alternatives,  Size([Size.INFINITY, Size.INFINITY], [350, 120]), \
								self.managerList[0])
							# remove old from alternatives layout
							if (self.alternatives.getCurrent() == oldView):
								# show view
								self.alternatives.setCurrent(self.managerViewList[0])
							oldView.removeFromParent()
					newManager = self.manager.getPoseAnimationProxyManager(self.name)
					if not (self.managerList[1] == newManager):
						# pose manager changed
						if self.managerList[1] is None:
							# create new
							# add manager
							self.managerList[1] = newManager
							# add manager view
							self.managerViewList[1] = PoseAnimationProxyManagerView( \
								self.alternatives,  Size([Size.INFINITY, Size.INFINITY], [350, 120]), \
								self.managerList[1])
							# add toggle to toggleGroup, calls update() if selected
							self.model.addToggle(self.toggleList[1])
							# show ToggleView
							self.alternativesList[1].setCurrent(self.toggleViewList[1])
						elif newManager is None:
							# remove old
							# hide ToggleView
							self.alternativesList[1].setCurrent(self.spacerList[1])
							# remove toggle from ToggleGroup
							self.model.removeToggle(self.toggleList[1])
							# set spacer in lower part of the window if no manager left
							if (self.managerList.count(None) >= 2):
								self.alternatives.setCurrent(self.spacer)
							# remove old manager in list
							self.managerList[1] = None
							# remove old managerView
							self.managerViewList[1].removeFromParent()
							self.managerViewList[1] = None
						else:
							# swap
							# manager
							self.managerList[0] = newManager
							# view
							oldView = self.managerViewList[1]
							# add new to alternatives layout
							self.managerViewList[1] = PoseAnimationProxyManagerView( \
								self.alternatives,  Size([Size.INFINITY, Size.INFINITY], [350, 120]), \
								self.managerList[1])
							# remove old from alternatives layout
							if (self.alternatives.getCurrent() == oldView):
								# show view
								self.alternatives.setCurrent(self.managerViewList[1])
							oldView.removeFromParent()
					newManager = self.manager.getMorphAnimationProxyManager(self.name)
					if not (self.managerList[2] == newManager):
						# morph manager changed
						if self.managerList[2] is None:
							# create new
							# add manager
							self.managerList[2] = newManager
							# add manager view
							self.managerViewList[2] = MorphAnimationProxyManagerView( \
								self.alternatives,  Size([Size.INFINITY, Size.INFINITY], [350, 120]), \
								self.managerList[2])
							# add toggle to toggleGroup, calls update() if selected
							self.model.addToggle(self.toggleList[2])
							# show ToggleView
							self.alternativesList[2].setCurrent(self.toggleViewList[2])
						elif newManager is None:
							# remove old
							# hide ToggleView
							self.alternativesList[2].setCurrent(self.spacerList[2])
							# remove toggle from ToggleGroup
							self.model.removeToggle(self.toggleList[2])
							# set spacer in lower part of the window if no manager left
							if (self.managerList.count(None) >= 2):
								self.alternatives.setCurrent(self.spacer)
							# remove old manager in list
							self.managerList[2] = None
							# remove old managerView
							self.managerViewList[2].removeFromParent()
							self.managerViewList[2] = None
						else:
							# swap
							# manager
							self.managerList[2] = newManager
							# view
							oldView = self.managerViewList[2]
							# add new to alternatives layout
							self.managerViewList[2] = PoseAnimationProxyManagerView( \
								self.alternatives,  Size([Size.INFINITY, Size.INFINITY], [350, 120]), \
								self.managerList[2])
							# remove old from alternatives layout
							if (self.alternatives.getCurrent() == oldView):
								# show view
								self.alternatives.setCurrent(self.managerViewList[2])
							oldView.removeFromParent()
					return
		
		class MeshExporterApplication:
			def __init__(self):
				# initialize global settings to default value
				self.exportMaterial = ToggleModel(True)
				self.materalScriptName = BasenameModel(Blender.Scene.GetCurrent().getName() + '.material')
				self.exportPath = DirnameModel(Blender.Get('filename'))
				self.renderingMaterial = ToggleModel(True)
				self.gameEngineMaterials = ToggleModel(False)
				self.customMaterial = ToggleModel(False)
				self.customMaterialTplPath = DirnameModel(Blender.Get('filename'))
				self.colouredAmbient = ToggleModel(0)
				self.exportMesh = ToggleModel(True)
				self.fixUpAxis = ToggleModel(True)
				self.convertXML = ToggleModel(0)
				self.copyTextures = ToggleModel(0)
				self.requireFaceMats = ToggleModel(True)
				self.skeletonUseMeshName = ToggleModel(1)

				matTglGrp = ToggleGroup()
				matTglGrp.addToggle(self.renderingMaterial)
				matTglGrp.addToggle(self.gameEngineMaterials)
				matTglGrp.addToggle(self.customMaterial)
				# load package settings if applicable
				self._loadPackageSettings()
				# manager for selected objects
				self.selectedObjectManager = SelectedObjectManager()
				self.preferencesScreen = PreferencesScreen()
				# create main screen
				#  material settings
				self.mainScreen = Screen()
				frame = OgreFrame(self.mainScreen, "Meshes Exporter")
				vLayout = VerticalLayout(frame)
				# SelectedObjectManagerView
				SelectedObjectManagerView(vLayout, self.selectedObjectManager)
				## material settings
				Spacer(vLayout, Size([0, 10]))
				mbox = Box(vLayout, L("Material Settings"), 0 , 10)
				mvLayout = VerticalLayout(mbox, True)
				mvhLayout1 = HorizontalLayout(mvLayout)
				ToggleView(mvhLayout1, Size([100, 20]), self.exportMaterial, \
					T("Export Materials"), \
					T("Uncheck this to not export materials when exporting mesh."))
				StringView(mvhLayout1, Size([Size.INFINITY, 20], [150, 20]), self.materalScriptName, T("Material File: "), \
					T("All material definitions go in this file (relative to the export path)."))
				mvhLayout2 = HorizontalLayout(mvLayout)
				ToggleView(mvhLayout2, Size([Size.INFINITY, 20], [150, 20]), self.colouredAmbient, \
					T("Coloured Ambient"), \
					T("Use Amb factor times diffuse colour as ambient instead of Amb factor times white."))
				ToggleView(mvhLayout2, Size([Size.INFINITY, 20], [100, 20]), self.copyTextures, \
					T("Copy Textures"), \
					T("Copy texture files into export path."))

				mvhLayout3 = HorizontalLayout(mvLayout)
				ToggleView(mvhLayout3, Size([Size.INFINITY, 20], [100, 20]), self.renderingMaterial, \
					T("Rendering Materials"), \
					T("Export rendering materials."))
				ToggleView(mvhLayout3, Size([Size.INFINITY, 20], [100, 20]), self.gameEngineMaterials, \
					T("Game Engine Materials"), \
					T("Export game engine materials instead of rendering materials."))
				ToggleView(mvhLayout3, Size([Size.INFINITY, 20], [100, 20]), self.customMaterial, \
					T("Custom Materials"), \
					T("Export using custom material templates."))

				self.matAlternatives = AlternativesLayout(mvLayout)
				self.matAltDefault = Spacer(self.matAlternatives, Size([0, 20]))
				self.matAltCustom = HorizontalLayout(self.matAlternatives)
				self.update()
				StringView(self.matAltCustom, Size([Size.INFINITY, 20], [230, 20]), self.customMaterialTplPath, T("Template Path: "), \
					T("Path to the material template files for generating custom materials."))
				Button(self.matAltCustom, Size([70, 20]), MeshExporterApplication.SelectAction(self.customMaterialTplPath, "Template Directory"), T("Select"), \
					T("Select the material template directory."))
				## global settings
				Spacer(vLayout, Size([0, 10]))
				globalSettingLayout = VerticalLayout(vLayout, True)
				# path panel
				phLayout = HorizontalLayout(globalSettingLayout)
				ToggleView(phLayout, Size([100, 20]), self.exportMesh, T("Export Meshes"), \
					T("Export the selected meshes. Uncheck this if you only want to export the materials."))
				StringView(phLayout, Size([Size.INFINITY, 20], [200, 20]), self.exportPath, T("Path: "), \
					T("The directory where the exported files are saved."))
				Button(phLayout, Size([70, 20]), MeshExporterApplication.SelectAction(self.exportPath, "Export Directory"), T("Select"), \
					T("Select the export directory."))				
				# mesh settings
				globalSettingLayout1 = HorizontalLayout(globalSettingLayout)
				ToggleView(globalSettingLayout1, Size([Size.INFINITY, 20], [100, 20]), self.fixUpAxis, T("Fix Up Axis to Y"), \
					T("Fix up axis as Y instead of Z."))
				ToggleView(globalSettingLayout1, Size([Size.INFINITY, 20], [100, 20]), self.requireFaceMats, T("Require Materials"), \
					T("Generate Error message when part of a mesh is not assigned with a material."))
				ToggleView(globalSettingLayout1, Size([Size.INFINITY, 20], [150, 20]), self.skeletonUseMeshName, T("Skeleton name follow mesh"), \
					T("Use mesh name for the exported skeleton name instead of the armature name."))
				globalSettingLayout2 = HorizontalLayout(globalSettingLayout)
				ToggleView(globalSettingLayout2, Size([Size.INFINITY, 20], [100, 20]), self.convertXML, T("OgreXMLConverter"), \
					T("Run OgreXMLConverter on the exported XML files."))
				## buttons
				Spacer(vLayout, Size([0, 10]))
				bhLayout = HorizontalLayout(vLayout, True)
				bSize = Size([Size.INFINITY, 30], [Blender.Draw.GetStringWidth('Preferences')+10, 30])
				Button(bhLayout, bSize, MeshExporterApplication.ExportAction(self), T("Export"), \
					T("Export selected mesh objects."))
				Button(bhLayout, bSize, MeshExporterApplication.PreferencesAction(self), T("Preferences"), \
					T("Exporter preferences."))
				Button(bhLayout, bSize, MeshExporterApplication.HelpAction(), T("Help"), \
					T("Get help."))
				Button(bhLayout, bSize, MeshExporterApplication.QuitAction(self), T("Quit"), \
					T("Quit without exporting."))

				# hook up events.
				self.customMaterial.addView(self)
				self.materalScriptName.addView(self)

				return
			def update(self):
				"""Called by customMaterial.
				"""
				if self.customMaterial.getValue():
					self.matAlternatives.setCurrent(self.matAltCustom)
				else:
					self.matAlternatives.setCurrent(self.matAltDefault)

				# Material script name must have a value. if it's empty, force it to scene name.
				if len(self.materalScriptName.getValue()) == 0:
					self.materalScriptName.setValue(Blender.Scene.GetCurrent().getName() + '.material')
				return
			def go(self):
				self.mainScreen.activate()
				return
			def _loadPackageSettings(self):
				exportMaterial = PackageSettings.getSingleton().getSetting('exportMaterial')
				if exportMaterial is not None:
					self.exportMaterial.setValue(exportMaterial)
				materalScriptName = PackageSettings.getSingleton().getSetting('materalScriptName')
				if materalScriptName is not None:
					self.materalScriptName.setValue(materalScriptName)
				customMaterial = PackageSettings.getSingleton().getSetting('customMaterial')
				if customMaterial is not None:
					self.customMaterial.setValue(customMaterial)
				customMaterialTplPath = PackageSettings.getSingleton().getSetting('customMaterialTplPath')
				if customMaterialTplPath is not None:
					self.customMaterialTplPath.setValue(customMaterialTplPath)
				colouredAmbient = PackageSettings.getSingleton().getSetting('colouredAmbient')
				if colouredAmbient is not None:
					self.colouredAmbient.setValue(colouredAmbient)
				gameEngineMaterials = PackageSettings.getSingleton().getSetting('gameEngineMaterials')
				if gameEngineMaterials is not None:
					self.gameEngineMaterials.setValue(gameEngineMaterials)
				copyTextures = PackageSettings.getSingleton().getSetting('copyTextures')
				if copyTextures is not None:
					self.copyTextures.setValue(copyTextures)
				exportMesh = PackageSettings.getSingleton().getSetting('exportMesh')
				if exportMesh is not None:
					self.exportMesh.setValue(exportMesh)
				fixUpAxis = PackageSettings.getSingleton().getSetting('fixUpAxis')
				if fixUpAxis is not None:
					self.fixUpAxis.setValue(fixUpAxis)
				requireFaceMats = PackageSettings.getSingleton().getSetting('requireFaceMats')
				if requireFaceMats is not None:
					self.requireFaceMats.setValue(requireFaceMats)
				skeletonUseMeshName = PackageSettings.getSingleton().getSetting('skeletonUseMeshName')
				if skeletonUseMeshName is not None:
					self.skeletonUseMeshName.setValue(skeletonUseMeshName)
				convertXML = PackageSettings.getSingleton().getSetting('convertXML')
				if convertXML is not None:
					self.convertXML.setValue(convertXML)
				exportPath = PackageSettings.getSingleton().getSetting('exportPath')
				if exportPath is not None:
					self.exportPath.setValue(exportPath)
				return
			def _savePackageSettings(self):
				self.selectedObjectManager.savePackageSettings()
				PackageSettings.getSingleton().setSetting('exportMaterial', self.exportMaterial.getValue())
				PackageSettings.getSingleton().setSetting('materalScriptName', self.materalScriptName.getValue())
				PackageSettings.getSingleton().setSetting('customMaterial', self.customMaterial.getValue())
				PackageSettings.getSingleton().setSetting('customMaterialTplPath', self.customMaterialTplPath.getValue())
				PackageSettings.getSingleton().setSetting('colouredAmbient', self.colouredAmbient.getValue())
				PackageSettings.getSingleton().setSetting('gameEngineMaterials', self.gameEngineMaterials.getValue())
				PackageSettings.getSingleton().setSetting('copyTextures', self.copyTextures.getValue())
				PackageSettings.getSingleton().setSetting('requireFaceMats', self.requireFaceMats.getValue())
				PackageSettings.getSingleton().setSetting('exportMesh', self.exportMesh.getValue())
				PackageSettings.getSingleton().setSetting('fixUpAxis', self.fixUpAxis.getValue())
				PackageSettings.getSingleton().setSetting('skeletonUseMeshName', self.skeletonUseMeshName.getValue())
				PackageSettings.getSingleton().setSetting('convertXML', self.convertXML.getValue())
				PackageSettings.getSingleton().setSetting('exportPath', self.exportPath.getValue())
				PackageSettings.getSingleton().save()
				return
			class PreferencesAction(Action):
				def __init__(self, app):
					self.app = app
					return
				def execute(self):
					self.app.preferencesScreen.activate()
					return
			class HelpAction(Action):
				def execute(self):
					dirList = [Blender.Get('datadir'), Blender.Get('udatadir'), Blender.Get('scriptsdir'), Blender.Get('uscriptsdir')]
					stack = [dir for dir in dirList if dir is not None]
					found = False
					helpFile = ''
					while not(found) and (len(stack) > 0):
						dir = stack.pop(0)
						helpFile = Blender.sys.join(dir, Blender.sys.join('ogrehelp','ogremeshesexporter.html'))
						if Blender.sys.exists(helpFile):
							found = True
					if found:
						webbrowser.open(helpFile, 1, 1)
					else:
						webbrowser.open("http://www.ogre3d.org/phpBB2/search.php", 1, 1)
					return
			class UpdateAction(Action):
				def __init__(self, app):
					self.app = app
					return
			class SelectAction(Action):
				def __init__(self, pathObject, title):
					self.pathObject = pathObject
					self.title = title
					return
				def execute(self):
					Blender.Window.FileSelector(self.pathObject.setValue, self.title, self.pathObject.getValue())
					return
			class ExportAction(Action):
				def __init__(self, app):
					self.app = app
					self.exportScreen = None
					self.logView = None
					return
				def execute(self):
					# create export screen
					self.exportScreen = Screen()
					frame = OgreFrame(self.exportScreen, "Meshes Exporter")
					vLayout = VerticalLayout(frame)
					LabelView(vLayout, L('Export Log:', 'large'))
					self.logView = LogView(vLayout, Size([Size.INFINITY, Size.INFINITY], [300, 200]), 20, False)
					Spacer(vLayout, Size([0, 10]))
					activator = Activator(vLayout, False)
					bhLayout = HorizontalLayout(activator)
					Button(bhLayout, Size([Size.INFINITY, 30], [120, 30]), MeshExporterApplication.ExportAction.OkAction(self), \
						T("Ok"), T("Close export log."))
					Spacer(bhLayout, Size([Size.INFINITY, 30], [120, 30]))
					Spacer(bhLayout, Size([Size.INFINITY, 30], [120, 30]))
					Button(bhLayout, Size([Size.INFINITY, 30], [120, 30]), MeshExporterApplication.QuitAction(self.app), \
						T("Quit"), T("Quit exporter."))
					self.exportScreen.activate()
					# save package settings on every export
					self.app._savePackageSettings()
					Log.getSingleton().logInfo("Exporting...")
					self.app.selectedObjectManager.export(self.app.exportPath.getValue(), \
						self.app.exportMaterial.getValue(), \
						self.app.materalScriptName.getValue(), \
						self.app.customMaterial.getValue(), \
						self.app.customMaterialTplPath.getValue(), \
						self.app.colouredAmbient.getValue(), \
						self.app.gameEngineMaterials.getValue(), \
						self.app.exportMesh.getValue(), \
						self.app.fixUpAxis.getValue(), \
						self.app.skeletonUseMeshName.getValue(), \
						self.app.convertXML.getValue(), \
						self.app.copyTextures.getValue(), \
						self.app.requireFaceMats.getValue(), \
						)
					Log.getSingleton().logInfo("Done.")
					activator.setEnabled(True)
					Blender.Draw.Redraw(1)
					return
				class OkAction(Action):
					def __init__(self, exportAction):
						self.action = exportAction
						return
					def execute(self):
						self.action.exportScreen.deactivate()
						# remove view from model
						self.action.logView.detachModel()
						return
			class QuitAction(QuitAction):
				def __init__(self, app):
					self.app = app
					return
				def execute(self):
					self.app._savePackageSettings()
					QuitAction.execute(self)
					return

		if (__name__ == '__main__'):
			application = MeshExporterApplication()
			application.go()
