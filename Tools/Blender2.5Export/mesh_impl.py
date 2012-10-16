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
# See mesh_exporter.py for explanation.
# ########################################################################

import bpy, mathutils

from ogre_mesh_exporter.log_manager import LogManager, Message

# Mesh export settings class to define how we are going to export the mesh.
class MeshExportSettings():
	def __init__(self, fixUpAxisToY = True, requireMaterials = True, applyModifiers = False, skeletonNameFollowMesh = True, runOgreXMLConverter = True):
		self.fixUpAxisToY = fixUpAxisToY
		self.requireMaterials = requireMaterials
		self.applyModifiers = applyModifiers
		self.skeletonNameFollowMesh = skeletonNameFollowMesh
		self.runOgreXMLConverter = runOgreXMLConverter

	@classmethod
	def fromRNA(cls, meshObject):
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		meshSettings = meshObject.data.ogre_mesh_exporter

		return MeshExportSettings(
			fixUpAxisToY = globalSettings.fixUpAxisToY,
			requireMaterials = meshSettings.requireMaterials if (meshSettings.requireMaterials_override) else globalSettings.requireMaterials,
			applyModifiers = meshSettings.applyModifiers if (meshSettings.applyModifiers_override) else globalSettings.applyModifiers,
			skeletonNameFollowMesh = meshSettings.skeletonNameFollowMesh if (meshSettings.skeletonNameFollowMesh_override) else globalSettings.skeletonNameFollowMesh,
			runOgreXMLConverter = globalSettings.runOgreXMLConverter)

class Vertex():
	def __init__(self, pos, norm, uvs):
		self.mPosition = pos
		self.mNormal = norm
		self.mUVs = uvs

	def match(self, norm, uvs, colors):
		# Test normal.
		if (self.mNormal != norm): return False;

		# Test UVs.
		if (len(self.mUVs) is not len(uvs)): return False
		for uv1, uv2 in zip(self.mUVs, uvs):
			if (uv1 != uv2): return False

		# Test Colors.
		if (len(self.mColors) is not len(colors)): return False
		for color1, color2 in zip(self.mColors, colors):
			if (color1 != color2): return False

		return True

class VertexBuffer():
	def __init__(self, uvLayers = 0, colorLayers = 0):
		# Vertex data.
		self.mVertexData = list()
		self.mUVLayers = uvLayers
		self.mColorLayers = colorLayers

		# Blender mesh -> vertex index link.
		# Only useful when exporting.
		self.mMeshVertexIndexLink = dict()

	def reset(self, uvLayers, colorLayers):
		self.mVertexData = list()
		self.mUVLayers = uvLayers
		self.mColorLayers = colorLayers

	def vertexCount(self):
		return len(self.mVertexData)

	# This method adds a vertex from the given blend mesh index into the buffer.
	# If the uv information does not match the recorded vertex, it will automatically
	# clone a new vertex for use.
	def addVertex(self, index, pos, norm, uvs, colors, fixUpAxisToY):
		# Fix Up axis to Y (swap Y and Z and negate Z)
		if (fixUpAxisToY):
			pos = [pos[0], pos[2], -pos[1]]
			norm = [norm[0], norm[2], -norm[1]]

		# make sure uv layers and color layers matches as defined.
		if (len(uvs) != self.mUVLayers or len(colors) != self.mColorLayers):
			raise Exception("Invalid UV layer or Color layer count! Expecting uv(%d), color(%d). Got uv(%d), color(%d)" %
				(self.mUVLayers, self.mColorLayers, len(uvs), len(colors)))

		# try to find pre added vertex that matches criteria.
		if (index in self.mMeshVertexIndexLink):
			localIndexList = self.mMeshVertexIndexLink[index]
			for localIndex in localIndexList:
				if (self.mVertexData[localIndex].match(norm, uvs, colors)):
					return localIndex

		# nothing found. so we add a new vertex.
		localIndex = len(self.mVertexData)
		if (index not in self.mMeshVertexIndexLink): self.mMeshVertexIndexLink[index] = list()
		self.mMeshVertexIndexLink[index].append(localIndex)
		self.mVertexData.append(Vertex(pos, norm, uvs, colors))
		return localIndex

	def serialize(self, file, indent = ''):
		extraAttributes = ''
		uvLayerCount = 8 if (self.mUVLayers > 8) else self.mUVLayers
		if (uvLayerCount > 0):
			extraAttributes = ' texture_coords="%d"' % uvLayerCount
			for i in range(uvLayerCount):
				extraAttributes += ' texture_coord_dimensions_%d="float2"' % i

		colorLayerCount = self.mColorLayers
		if (colorLayerCount > 0): extraAttributes += ' colours_diffuse="true"'
		if (colorLayerCount > 1): extraAttributes += ' colours_specular="true"'

		file.write('%s<vertexbuffer positions="true" normals="true"%s>\n' % (indent, extraAttributes))

		for vertex in self.mVertexData:
			file.write('%s\t<vertex>\n' % indent)

			# write position and normal.
			file.write('%s\t\t<position x="%.6f" y="%.6f" z="%.6f" />\n' % (indent, vertex.mPosition[0], vertex.mPosition[1], vertex.mPosition[2]))
			file.write('%s\t\t<normal x="%.6f" y="%.6f" z="%.6f" />\n' % (indent, vertex.mNormal[0], vertex.mNormal[1], vertex.mNormal[2]))

			# write UV layers. (NOTE: Blender uses bottom left coord! Ogre uses top left! So we have to flip Y.)
			for i in range(uvLayerCount):
				uv = vertex.mUVs[i]
				file.write('%s\t\t<texcoord u="%.6f" v="%.6f" />\n' % (indent, uv[0], (1.0 - uv[1])))

			# write diffuse.
			if (colorLayerCount > 0):
				color = vertex.mColors[0]
				file.write('%s\t\t<colour_diffuse value="%.6f %.6f %.6f" />\n' % (indent, color[0], color[1], color[2]))

			# write specular.
			if (colorLayerCount > 1):
				color = vertex.mColors[1]
				file.write('%s\t\t<colour_diffuse value="%.6f %.6f %.6f" />\n' % (indent, color[0], color[1], color[2]))

class SubMesh():
	# True if submesh is sharing vertex data.
	mShareVertexData = True

class SubMesh():
	def __init__(self, vertexBuffer = None, meshVertexIndexLink = None):
		# True if submesh is sharing vertex buffer.
		self.mShareVertexBuffer = False
		# Vertex buffer.
		self.mVertexBuffer = VertexBuffer()
		# Blender mesh -> local/shared vertex index link.
		self.mMeshVertexIndexLink = dict()
		# Face data.
		self.mFaceData = list()
		# Blender material.
		self.mMaterial = None

		if ((vertexBuffer is not None) and (meshVertexIndexLink is not None)):
			self.mShareVertexBuffer = True
			self.mVertexBuffer = vertexBuffer
			self.mMeshVertexIndexLink = meshVertexIndexLink

	def insertPolygon(self, blendMesh, polygon, fixUpAxisToY):
		polygonVertices = polygon.vertices
		polygonVertexCount = polygon.loop_total

		# extract uv information.
		# Here we convert blender uv data into our own
		# uv information that lists uvs by vertices.
		blendUVLoopLayers = blendMesh.uv_layers
		# construct empty polygon vertex uv list.
		polygonVertUVs = list()
		for i in range(polygonVertexCount): polygonVertUVs.append(list())
		for uvLoopLayer in blendUVLoopLayers:
			for i, loopIndex in enumerate(polygon.loop_indices):
				polygonVertUVs[i].append(uvLoopLayer.data[loopIndex].uv)

		# extract color information.
		# Here we convert blender color data into our own
		# color information that lists colors by vertices.
		blendColorLoopLayers = blendMesh.vertex_colors
		# construct empty polygon vertex color list.
		polygonVertColors = list()
		for i in range(polygonVertexCount): polygonVertColors.append(list())
		for colorLoopLayer in blendColorLoopLayers:
			for i, loopIndex in enumerate(polygon.loop_indices):
				polygonVertColors[i].append(colorLoopLayer.data[loopIndex].color)

		# loop through the vertices and add to this submesh.
		localIndices = list()
		useSmooth = polygon.use_smooth
		for index, uvs, colors in zip(polygonVertices, polygonVertUVs, polygonVertColors):
			vertex = blendMesh.vertices[index]
			norm = vertex.normal if (useSmooth) else polygon.normal
			localIndices.append(self.mVertexBuffer.addVertex(index, vertex.co, norm, uvs, colors, fixUpAxisToY))

		# construct triangle index data.
		if (polygonVertexCount is 3):
			self.mFaceData.append(localIndices)
		else:
			# split quad into triangles.
			self.mFaceData.append(localIndices[:3])
			self.mFaceData.append([localIndices[0], localIndices[2], localIndices[3]])

	def serialize(self, file):
		vertexCount = self.mVertexBuffer.vertexCount()
		materialAttribute = '' if (self.mMaterial is None) else ' material="%s"' % self.mMaterial.name
		file.write('\t\t<submesh%s usesharedvertices="%s" use32bitindexes="%s">\n' %
			(materialAttribute, 'true' if self.mShareVertexBuffer else 'false',
			'true' if (vertexCount > 65536) else 'false'))

		# write submesh vertex buffer if not shared.
		if (not self.mShareVertexBuffer):
			file.write('\t\t\t<geometry vertexcount="%d">\n' % vertexCount)
			self.mVertexBuffer.serialize(file, '\t\t\t\t')
			file.write('\t\t\t</geometry>\n')

		# write face data.
		file.write('\t\t\t<faces count="%d">\n' % len(self.mFaceData))
		for face in self.mFaceData:
			file.write('\t\t\t\t<face v1="%d" v2="%d" v3="%d" />\n' % tuple(face))
		file.write('\t\t\t</faces>\n')

	def __init__(self, vertexData = None):
		if (vertexData is None):
			self.mShareVertexData = False
		else
			self.mVertexData = vertexData

class Mesh():
	def __init__(self, blendMesh = None, exportSettings = MeshExportSettings()):
		# shared vertex buffer.
		self.mSharedVertexBuffer = VertexBuffer()
		# Blender mesh -> shared vertex index link.
		self.mSharedMeshVertexIndexLink = dict()
		# collection of submeshes.
		self.mSubMeshDict = dict()

		# skip blend mesh conversion if no blend mesh passed in.
		if (blendMesh is None): return

		# Lets do some pre checking to show warnings if needed.
		uvLayerCount = len(blendMesh.uv_layers)
		colorLayerCount = len(blendMesh.vertex_colors)
		if (uvLayerCount > 8): LogManager.logMessage("More than 8 UV layers in this mesh. Only 8 will be exported.", Message.LVL_WARNING)
		if (colorLayerCount > 2): LogManager.logMessage("More than 2 color layers in this mesh. Only 2 will be exported.", Message.LVL_WARNING)

		# setup shared vertex buffer.
		self.mSharedVertexBuffer.reset(uvLayerCount, colorLayerCount)

		# split up the mesh into submeshes by materials.
		materialList = blendMesh.materials
		LogManager.logMessage("Material Count: %d" % len(materialList), Message.LVL_INFO)
		for polygon in blendMesh.polygons:
			# get or create submesh.
			if (polygon.material_index in self.mSubMeshDict):
				subMesh = self.mSubMeshDict[polygon.material_index]
			else:
				subMesh = SubMesh(self.mSharedVertexBuffer, self.mSharedMeshVertexIndexLink)
				subMesh.mMaterial = None if (len(materialList) == 0) else materialList[polygon.material_index]
				if (exportSettings.requireMaterials and subMesh.mMaterial == None):
					LogManager.logMessage("Some faces are not assigned with a material!", Message.LVL_WARNING)
					LogManager.logMessage("To hide this warning, please uncheck the 'Require Materials' option.", Message.LVL_WARNING)
				self.mSubMeshDict[polygon.material_index] = subMesh

			# insert polygon.
			subMesh.insertPolygon(blendMesh, polygon, exportSettings.fixUpAxisToY)

	def serialize(self, file):
		file.write('<mesh>\n')

		# write shared vertex buffer if available.
		sharedVertexCount = self.mSharedVertexBuffer.vertexCount()
		if (sharedVertexCount > 0):
			file.write('\t<sharedgeometry vertexcount="%d">\n' % sharedVertexCount)
			self.mSharedVertexBuffer.serialize(file, '\t\t')
			file.write('\t</sharedgeometry>\n')

		# write submeshes.
		file.write('\t<submeshes>\n')
		for subMesh in self.mSubMeshDict.values():
			subMesh.serialize(file)
		file.write('\t</submeshes>\n')

	# collection of submeshes.
	mSubMeshDict = dict()
