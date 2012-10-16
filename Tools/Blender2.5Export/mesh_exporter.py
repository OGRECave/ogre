# ##### BEGIN MIT LICENSE BLOCK #####
# Copyright (C) 2011 by Lih-Hern Pang

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ##### END MIT LICENSE BLOCK #####

# ########################################################################
# NOTE:
# The exporter will try to share vertex data by default.
# Sharing of vertex data is important to keep render state update small.
# This means that even if we are using different material submeshes in a mesh,
# the mesh's vertex data only needs to be assigned to the GPU once as it's shared.
# However on some special occasions we would not want to share vertex data.
#
# In the case of animated meshes, we sometimes want to split the meshes
# depending on different animation type within a mesh.
#
# Imagine an animated character with skeletal and pose animation for facial expressions.
# To be optimal, the facial vertex pose animation should be keyed in it's own submesh
# so as to avoid updating a huge data set of vertex points in software mode.
# In hardware mode, this is also essential to split parts that has more complex vertex
# animations from the skeletal skinning only parts.
#
# In the case of a complex character with a lot of bones, hardware skinning may hit the
# limit of vertex shader registers for each bone matrix. To fix this, the mesh should be
# split into it's own individual submeshes in a way where the total number of bones
# affecting each submesh does not exceed the bone limit.
#
# In preparation of these cases, this exporter will take in to consideration of the
# vertex group settings. In the exporter settings, there will be a setting to specify
# the vertex groups that will be used to split meshes into it's own non shared vertex
# data sub meshes.
#
# For info on implementation see mesh_impl.py
#
# ########################################################################

import bpy, os, traceback, shlex, subprocess

from ogre_mesh_exporter.mesh_impl import Mesh
from ogre_mesh_exporter.mesh_impl import MeshExportSettings
from ogre_mesh_exporter.log_manager import LogManager, Message

# Mesh XML Converter settings class to define how we are going to convert the mesh.
class MeshXMLConverterSettings():
	def __init__(self, extremityPoints = 0, edgeLists = False, tangent = False, tangentSemantic = 'tangent', tangentSize = '3', splitMirrored = False, splitRotated = False, reorganiseVertBuff = True, optimiseAnimation = True):
		self.extremityPoints = extremityPoints
		self.edgeLists = edgeLists
		self.tangent = tangent
		self.tangentSemantic = tangentSemantic
		self.tangentSize = tangentSize
		self.splitMirrored = splitMirrored
		self.splitRotated = splitRotated
		self.reorganiseVertBuff = reorganiseVertBuff
		self.optimiseAnimation = optimiseAnimation

	@classmethod
	def fromRNA(cls, meshObject):
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		meshSettings = meshObject.data.ogre_mesh_exporter

		return MeshXMLConverterSettings(
			extremityPoints = meshSettings.extremityPoints if (meshSettings.extremityPoints_override) else globalSettings.extremityPoints,
			edgeLists = meshSettings.edgeLists if (meshSettings.edgeLists_override) else globalSettings.edgeLists,
			tangent = meshSettings.tangent if (meshSettings.tangent_override) else globalSettings.tangent,
			tangentSemantic = meshSettings.tangentSemantic if (meshSettings.tangentSemantic_override) else globalSettings.tangentSemantic,
			tangentSize = meshSettings.tangentSize if (meshSettings.tangentSize_override) else globalSettings.tangentSize,
			splitMirrored = meshSettings.splitMirrored if (meshSettings.splitMirrored_override) else globalSettings.splitMirrored,
			splitRotated = meshSettings.splitRotated if (meshSettings.splitRotated_override) else globalSettings.splitRotated,
			reorganiseVertBuff = meshSettings.reorganiseVertBuff if (meshSettings.reorganiseVertBuff_override) else globalSettings.reorganiseVertBuff,
			optimiseAnimation = meshSettings.optimiseAnimation if (meshSettings.optimiseAnimation_override) else globalSettings.optimiseAnimation)

def exportMesh(meshObject, filepath):
	result = True
	try:
		LogManager.logMessage("Output: %s" % filepath, Message.LVL_INFO)

		# get combined mesh override & global settings.
		meshExportSettings = MeshExportSettings.fromRNA(meshObject)

		# If modifiers need to be applied, we will need to create a new mesh with flattened modifiers.
		LogManager.logMessage("Apply Modifier: %s" % meshExportSettings.applyModifiers, Message.LVL_INFO)
		if (meshExportSettings.applyModifiers):
			mesh = meshObject.to_mesh(bpy.context.scene, True, 'PREVIEW')
			cleanUpMesh = True
		else:
			mesh = meshObject.data
			cleanUpMesh = False

		# prepare mesh.
		ogreMesh = Mesh(mesh, meshExportSettings)
		LogManager.logMessage("Shared Vertices: %d" % len(ogreMesh.mSharedVertexBuffer.mVertexData), Message.LVL_INFO);
		LogManager.logMessage("Submeshes: %d" % len(ogreMesh.mSubMeshDict), Message.LVL_INFO);

		# write mesh.
		file = open(filepath, "w", encoding="utf8", newline="\n")
		ogreMesh.serialize(file)
		file.close()

		# remove mesh if we created a new one that has modifiers applied.
		if (cleanUpMesh): bpy.data.meshes.remove(mesh)
		ogreMesh = None

		LogManager.logMessage("Done exporting XML.")

		# check if we need to convert to ogre mesh.
		if (meshExportSettings.runOgreXMLConverter):
			globalSettings = bpy.context.scene.ogre_mesh_exporter
			LogManager.logMessage("Converting mesh to Ogre binary format...")
			result = convertToOgreMesh(globalSettings.ogreXMLConverterPath, filepath, MeshXMLConverterSettings.fromRNA(meshObject))

	except IOError as err:
		LogManager.logMessage("I/O error(%d): %s" % (err.errno, err.strerror), Message.LVL_ERROR)
		result = False
	except Exception as err:
		LogManager.logMessage(str(err), Message.LVL_ERROR)
		result = False
	except:
		traceback.print_exc()

	return result

def convertToOgreMesh(converterpath, filepath, settings = MeshXMLConverterSettings()):
	try:
		if os.path.exists(converterpath):
			filepath = os.path.normpath(filepath)
			converterpath = os.path.normpath(converterpath)

			# converter path
			command = [converterpath]

			# options
			if settings.extremityPoints > 0:
				command.extend(['-x', str(settings.extremityPoints)])
			if not settings.edgeLists:
				command.append('-e')
			if settings.tangent:
				command.append('-t')
			if settings.tangentSemantic == 'uvw':
				command.extend(['-td', 'uvw'])
			if settings.tangentSize == '4':
				command.extend(['-ts', '4'])
			if settings.splitMirrored:
				command.append('-tm')
			if settings.splitRotated:
				command.append('-tr')
			if settings.reorganiseVertBuff:
				command.append('-r')
			if not settings.optimiseAnimation:
				command.append('-o')

			# additional arguments
			globalSettings = bpy.context.scene.ogre_mesh_exporter
			additional = shlex.split(globalSettings.ogreXMLConverterAdditionalArg)
			if len(additional): command.extend(additional)
			command.extend(['-log', '%s.log' % filepath])

			# file path
			command.append(filepath)

			LogManager.logMessage("Executing OgrXMLConverter: " + " ".join(command))
			p = subprocess.Popen(command)
			p.wait() # wait for process to finish.
			# TODO! Allow multiple XML Converter process to happen at once? This will speed up exporting many meshes.
			LogManager.logMessage("Success!")
			return (p.returncode == 0)
		else:
			LogManager.logMessage("No converter found at %s" % converterpath, Message.LVL_ERROR)
	except:
		traceback.print_exc()

	return False
