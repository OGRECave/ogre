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

class Vertex():
	def __init__(self, pos, norm, uvs):
		self.mPosition = pos
		self.mNormal = norm
		self.mUVs = uvs

class VertexData():
	

class SubMesh():
	# True if submesh is sharing vertex data.
	mShareVertexData = True

	# Index data.
	mIndexData = list()

	def __init__(self, vertexData = None):
		if (vertexData is None):
			self.mShareVertexData = False
		else
			self.mVertexData = vertexData

class Mesh():
	# shared vertex data.
	mSharedVertexData = VertexData()

	# collection of submeshes.
	mSubMeshDict = dict()
