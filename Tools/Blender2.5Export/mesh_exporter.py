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

import bpy

def exportMesh(meshObject, filepath):
	globalSettings = bpy.context.scene.ogre_mesh_exporter
	meshSettings = meshObject.mesh.ogre_mesh_exporter

	# If modifiers need to be applied, we will need to create a new mesh with flattened modifiers.
	if ((meshSettings.applyModifiers_override and meshSettings.applyModifiers) or globalSettings.applyModifiers):
		mesh = meshObject.to_mesh(meshObject.users_scene, True, 'PREVIEW')
		cleanUpMesh = True
	else:
		mesh = meshObject.mesh
		cleanUpMesh = False

	file = open(filepath, "w", encoding="utf8", newline="\n")
	file.write('<mesh>')
	file.write('	<submeshes>')

	#TODO: Prepare submeshes (grouped by materials and vertex group settings).
	#TODO: Write to mesh.xml format.

	file.write('</mesh>')
	file.close()

	# remove mesh if we created a new one that has modifiers applied.
	if (cleanUpMesh):
		bpy.data.meshes.remove(mesh)
