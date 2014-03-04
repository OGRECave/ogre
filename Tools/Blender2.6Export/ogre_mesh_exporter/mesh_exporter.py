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
#
# In hardware mode, it is also essential to split parts that has more complex vertex
# animations from the skeletal skinning only parts. However as they will be using different
# shaders and hence different materials, this feature is not of any use here.
#
# Even so, in the case of a complex character with a lot of bones, hardware skinning may
# hit the limit of vertex shader registers for each bone matrix. To fix this, the mesh
# should be split into it's own individual submeshes in a way where the total number of
# bones affecting each submesh does not exceed the bone limit.
#
# One interesting case is the submesh alpha sorting case. When modelling things like cars
# or any models that have multiple transparent pieces, we may want to split the transparent
# parts into submeshes that can be depth sorted. Though in this case, we do not want to
# apply non shared vertex data on each submesh.
#
# In preparation of these cases, this exporter will take into consideration of the
# material settings. In the exporter settings under mesh tab, there will be a setting to
# flag materials to split meshes into it's own shared/non-shared vertex data submeshes.
# It will also have an option to allow custom naming of submeshes.
#
# For info on implementation see mesh_impl.py
#
# ########################################################################

import bpy, os, traceback, shlex, subprocess

from ogre_mesh_exporter.mesh_impl import Mesh
from ogre_mesh_exporter.mesh_impl import MeshExportSettings
from ogre_mesh_exporter.skeleton_impl import Skeleton
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

# NOTE: filepath must NOT contain extension part. Mesh will append mesh.xml and Skeleton will append skeleton.xml.
def exportMesh(meshObject, filepath):
	result = list()
	try:
		LogManager.logMessage("Output: %s.mesh.xml" % filepath, Message.LVL_INFO)

		# get combined mesh override & global settings.
		meshExportSettings = MeshExportSettings.fromRNA(meshObject)
		if (meshExportSettings.runOgreXMLConverter):
			meshXMLConverterSettings = MeshXMLConverterSettings.fromRNA(meshObject)

		# get linked armature
		armatureObject = None
		ogreSkeleton = None
		parentObject = meshObject.parent
		if (parentObject and meshObject.parent_type == 'ARMATURE'):
			armatureObject = parentObject
		else:
			# check modifier stack, use first valid armature modifier.
			for modifier in meshObject.modifiers:
				if (modifier.type == 'ARMATURE' and
					(modifier.use_vertex_groups or
					modifier.use_bone_envelopes)):
					armatureObject = modifier.object

		# Do skeleton export first if armature exist.
		if (armatureObject):
			# get skeleton file path and name.
			if (meshExportSettings.skeletonNameFollowMesh):
				skeletonFilePath = filepath + '.skeleton.xml';
				skeletonName = os.path.basename(filepath)
			else:
				dirname = os.path.dirname(filepath)
				skeletonFilePath = dirname + armatureObject.data.name + '.skeleton.xml';
				skeletonName = armatureObject.data.name

			LogManager.logMessage("Skeleton: " + skeletonName, Message.LVL_INFO);

			# prepare skeleton.
			meshInverseMatrix = meshObject.matrix_world.inverted()
			ogreSkeleton = Skeleton(skeletonName, armatureObject, meshInverseMatrix, meshExportSettings.fixUpAxisToY)
			LogManager.logMessage("Bones: %d" % len(ogreSkeleton.mBones), Message.LVL_INFO);

			# write skeleton.
			file = open(skeletonFilePath, "w", encoding="utf8", newline="\n")
			ogreSkeleton.serialize(file)
			file.close()

			LogManager.logMessage("Done exporting skeleton XML.")

			# Run XML Converter if needed.
			if (meshExportSettings.runOgreXMLConverter):
				globalSettings = bpy.context.scene.ogre_mesh_exporter
				LogManager.logMessage("Converting skeleton to Ogre binary format...")
				result.append(executeOgreXMLConverter(globalSettings.ogreXMLConverterPath, skeletonFilePath, meshXMLConverterSettings))

		# If modifiers need to be applied, we will need to create a new mesh with flattened modifiers.
		LogManager.logMessage("Apply Modifier: %s" % meshExportSettings.applyModifiers, Message.LVL_INFO)
		if (meshExportSettings.applyModifiers):
			mesh = meshObject.to_mesh(bpy.context.scene, True, 'PREVIEW')
			cleanUpMesh = True
		else:
			mesh = meshObject.data
			cleanUpMesh = False

		# prepare mesh.
		ogreMesh = Mesh(mesh, meshObject.vertex_groups, ogreSkeleton, meshExportSettings)
		LogManager.logMessage("Shared Vertices: %d" % len(ogreMesh.mSharedVertexBuffer.mVertexData), Message.LVL_INFO);
		LogManager.logMessage("Submeshes: %d" % len(ogreMesh.mSubMeshDict), Message.LVL_INFO);
		for index, submesh in enumerate(ogreMesh.mSubMeshDict.values()):
			if (submesh.mShareVertexBuffer): continue
			LogManager.logMessage(" [%d]%s: vertices: %d" % (index, submesh.mName if (submesh.mName) else '' , len(submesh.mVertexBuffer.mVertexData)), Message.LVL_INFO);

		# write mesh.
		meshFilePath = filepath + ".mesh.xml"
		file = open(meshFilePath, "w", encoding="utf8", newline="\n")
		ogreMesh.serialize(file)
		file.close()

		# remove mesh if we created a new one that has modifiers applied.
		if (cleanUpMesh): bpy.data.meshes.remove(mesh)
		ogreMesh = None

		LogManager.logMessage("Done exporting mesh XML.")

		# Run XML Converter if needed.
		if (meshExportSettings.runOgreXMLConverter):
			globalSettings = bpy.context.scene.ogre_mesh_exporter
			LogManager.logMessage("Converting mesh to Ogre binary format...")
			result.append(executeOgreXMLConverter(globalSettings.ogreXMLConverterPath, meshFilePath, meshXMLConverterSettings))
		else:
			LogManager.logMessage("Success!")

	except IOError as err:
		LogManager.logMessage("I/O error(%d): %s" % (err.errno, err.strerror), Message.LVL_ERROR)
		result.append(False)
	#~ except Exception as err:
		#~ LogManager.logMessage(str(err), Message.LVL_ERROR)
		#~ result.append(False)
	#~ except:
		#~ traceback.print_exc()
		#~ result.append(False)

	return result

gDevNull = open(os.devnull, "w")

def executeOgreXMLConverter(converterpath, filepath, settings = MeshXMLConverterSettings()):
	global gDevNull
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
			p = subprocess.Popen(command, stdout = gDevNull, stderr = gDevNull)
			return p
		else:
			LogManager.logMessage("No converter found at %s" % converterpath, Message.LVL_ERROR)
	except:
		traceback.print_exc()

	return False
