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

import bpy, pprint, mathutils

class Vertex():
	def __init__(self, pos, norm, uvs = list(), colors = list()):
		self.mPosition = pos
		self.mNormal = norm
		self.mUVs = uvs
		self.mColors = colors

	def matchUVs(self, uvs):
		if (len(self.mUVs) is not len(uvs)): return False
		for uv1, uv2 in zip(self.mUVs, uvs):
			if (uv1 != uv2): return False
		return True

class VertexBuffer():
	def __init__(self, uvLayers = 0, colorLayers = 0):
		# TODO: Post warning about ignoring more than 8 UV layers.
		# TODO: Post warning about ignoring more than 2 color layers.
		#~ if (uvLayers > 8): print("Warning")

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
	def addVertex(self, blendMesh, index, uvs, colors):
		# make sure uv layers and color layers matches as defined.
		if (len(uvs) != self.mUVLayers or len(colors) != self.mColorLayers):
			raise Exception("Invalid UV layer or Color layer count! Expecting uv(%d), color(%d). Got uv(%d), color(%d)" %
				(self.mUVLayers, self.mColorLayers, len(uvs), len(colors)))

		# try to find pre added vertex that matches criteria.
		if (index in self.mMeshVertexIndexLink):
			localIndexList = self.mMeshVertexIndexLink[index]
			for localIndex in localIndexList:
				if (self.mVertexData[localIndex].matchUVs(uvs)):
					return localIndex

		# nothing found. so we add a new vertex.
		vertex = blendMesh.vertices[index]
		localIndex = len(self.mVertexData)
		if (index not in self.mMeshVertexIndexLink): self.mMeshVertexIndexLink[index] = list()
		self.mMeshVertexIndexLink[index].append(localIndex)
		self.mVertexData.append(Vertex(vertex.co, vertex.normal, uvs, colors))
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

			# write UV layers.
			for i in range(uvLayerCount):
				uv = mUVs[i]
				file.write('%s\t\t<texcoord u="%.6f" v="%.6f" />\n' % (indent, uv[0], uv[1]))

			# write diffuse.
			if (colorLayerCount > 0):
				color = self.mColors[0]
				file.write('%s\t\t<colour_diffuse value="%.6f %.6f %.6f" />\n' % (indent, color[0], color[1], color[2]))

			# write specular.
			if (colorLayerCount > 1):
				color = self.mColors[1]
				file.write('%s\t\t<colour_diffuse value="%.6f %.6f %.6f" />\n' % (indent, color[0], color[1], color[2]))

			file.write('%s\t</vertex>\n' % indent)

		file.write('%s</vertexbuffer>\n' % indent)

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

	def insertFace(self, blendMesh, face):
		faceVertices = face.vertices
		faceVertexCount = len(faceVertices)

		# extract uv information.
		# Here we convert blender uv data into our own
		# uv information that lists uvs by vertices.
		blendFaceUVLayers = blendMesh.uv_textures
		# construct empty face vertex uv list.
		faceVertUVs = list()
		for i in range(faceVertexCount): faceVertUVs.append(list())
		for faceUVLayer in blendFaceUVLayers:
			# get and merge face uv data into a list of
			# uvs where list is following vertex order.
			faceUV = faceUVLayer.data[face.index]
			if (faceVertexCount is 3):
				uvs = faceUV.uv1[:], faceUV.uv2[:], faceUV.uv3[:]
			else:
				uvs = faceUV.uv1[:], faceUV.uv2[:], faceUV.uv3[:], faceUV.uv4[:]

			# loop through colors and insert them to our color list by vertices
			for i, uv in enumerate(uvs): faceVertUVs[i].append(uv)

		# extract color information.
		# Here we convert blender color data into our own
		# color information that lists colors by vertices.
		blendFaceColorLayers = blendMesh.vertex_colors
		# construct empty face vertex color list.
		faceVertColors = list()
		for i in range(faceVertexCount): faceVertColors.append(list())
		for faceColorLayer in blendFaceColorLayers:
			# get and merge face color data into a list of
			# colors where list is following vertex order.
			faceColor = faceColorLayer.data[face.index]
			if (faceVertexCount is 3):
				colors = faceColor.color1[:], faceColor.color2[:], faceColor.color3[:]
			else:
				colors = faceColor.color1[:], faceColor.color2[:], faceColor.color3[:], faceColor.color4[:]

			# loop through colors and insert them to our color list by vertices
			for i, color in enumerate(colors):
				faceVertColors[i].append(color)

		# loop through the vertices and add to this submesh.
		localIndices = list()
		for index, uvs, colors in zip(faceVertices, faceVertUVs, faceVertColors):
			localIndices.append(self.mVertexBuffer.addVertex(blendMesh, index, uvs, colors))

		# construct triangle index data.
		if (faceVertexCount is 3):
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

		file.write('\t\t</submesh>\n')

class Mesh():
	def __init__(self, blendMesh = None):
		# shared vertex buffer.
		self.mSharedVertexBuffer = VertexBuffer()
		# Blender mesh -> shared vertex index link.
		self.mSharedMeshVertexIndexLink = dict()
		# collection of submeshes.
		self.mSubMeshDict = dict()

		# skip blend mesh conversion if no blend mesh passed in.
		if (blendMesh is None): return

		# setup shared vertex buffer.
		self.mSharedVertexBuffer.reset(
			len(blendMesh.uv_textures),
			len(blendMesh.vertex_colors))

		# split up the mesh into submeshes by materials.
		materialList = blendMesh.materials
		for face in blendMesh.faces:
			# get or create submesh.
			if (face.material_index in self.mSubMeshDict):
				subMesh = self.mSubMeshDict[face.material_index]
			else:
				subMesh = SubMesh(self.mSharedVertexBuffer, self.mSharedMeshVertexIndexLink)
				subMesh.mMaterial = None if (len(materialList) == 0) else materialList[face.material_index]
				self.mSubMeshDict[face.material_index] = subMesh

			# insert face.
			subMesh.insertFace(blendMesh, face)

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

		file.write('</mesh>\n')
