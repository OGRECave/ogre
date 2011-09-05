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

import bpy, os, sys, configparser
from bpy.props import *

# ##############################################
# Mesh Properties on the mesh objects
class MeshProperties(bpy.types.PropertyGroup):
	# Enable/Disable export of this mesh.
	exportEnabled = BoolProperty(
		name = "Export",
		description = "Export this mesh.",
		default = True
	)

	requireMaterials_override = BoolProperty(
		name = "Require Materials Override",
		description = "Override global setting.",
		default = False
	)
	requireMaterials = BoolProperty(
		name = "Require Materials",
		description = "Generate Error message when part of this mesh is not assigned with a material.",
		default = True
	)

	skeletonNameFollowMesh_override = BoolProperty(
		name = "Skeleton Name Follow Mesh Override",
		description = "Override global setting.",
		default = False
	)
	skeletonNameFollowMesh = BoolProperty(
		name = "Skeleton Name Follow Mesh",
		description = "Use mesh name for exported skeleton name instead of the armature name.",
		default = True
	)

	applyModifiers_override = BoolProperty(
		name = "Apply Modifiers Override",
		description = "Override global setting.",
		default = False
	)
	applyModifiers = BoolProperty(
		name = "Apply Modifiers",
		description = "Apply mesh modifiers before export. (Slow and may break vertex order for morph targets!)",
		default = False
	)

	# ##############################################
	# XML Converter specific Properties
	extremityPoints_override = BoolProperty(
		name = "Extremity Points Override",
		description = "Override global setting.",
		default = False
	)
	extremityPoints = IntProperty(
		name = "Extremity Points",
		description = "Generate no more than num eXtremes for every submesh. (For submesh render sorting when using alpha materials on submesh)",
		soft_min = 0,
		soft_max = 65536
	)

	edgeLists_override = BoolProperty(
		name = "Edge Lists Override",
		description = "Override global setting.",
		default = False
	)
	edgeLists = BoolProperty(
		name = "Edge Lists",
		description = "Generate edge lists. (Useful for outlining or doing stencil shadows)",
		default = False
	)

	tangent_override = BoolProperty(
		name = "Tangent Override",
		description = "Override global setting.",
		default = False
	)
	tangent = BoolProperty(
		name = "Tangent",
		description = "Generate tangent.",
		default = False
	)

	tangentSemantic_override = BoolProperty(
		name = "Tangent Semantic Override",
		description = "Override global setting.",
		default = False
	)
	tangentSemantic = EnumProperty(
		name = "Tangent Semantic",
		description = "Tangent Semantic to use.",
		items=(("uvw", "uvw", "Use UV semantic."),
				("tangent", "tangent", "Use tangent semantic."),
				),
		default= "tangent"
	)

	tangentSize_override = BoolProperty(
		name = "Tangent Size Override",
		description = "Override global setting.",
		default = False
	)
	tangentSize = EnumProperty(
		name = "Tangent Size",
		description = "Size of tangent.",
		items=(("4", "4 component (parity)", "Use 4 component tangent where 4th component is parity."),
				("3", "3 component", "Use 3 component tangent."),
				),
		default= "3"
	)

	splitMirrored_override = BoolProperty(
		name = "Split Mirrored Override",
		description = "Override global setting.",
		default = False
	)
	splitMirrored = BoolProperty(
		name = "Split Mirrored",
		description = "Split tangent vertices at UV mirror points.",
		default = False
	)

	splitRotated_override = BoolProperty(
		name = "Split Rotated Override",
		description = "Override global setting.",
		default = False
	)
	splitRotated = BoolProperty(
		name = "Split Rotated",
		description = "Split tangent vertices where basis is rotated > 90 degrees.",
		default = False
	)

	reorganiseVertBuff_override = BoolProperty(
		name = "Reorganise Vertex Buffers Override",
		description = "Override global setting.",
		default = False
	)
	reorganiseVertBuff = BoolProperty(
		name = "Reorganise Vertex Buffers",
		description = "Reorganise vertex buffer to make it GPU vertex cache friendly.",
		default = True
	)

	optimiseAnimation_override = BoolProperty(
		name = "Optimise Animation Override",
		description = "Override global setting.",
		default = False
	)
	optimiseAnimation = BoolProperty(
		name = "Optimise Animation",
		description = "Optimise out redundant tracks & keyframes.",
		default = True
	)

# registering and menu integration
def register():
	bpy.utils.register_module(__name__)

# unregistering and removing menus
def unregister():
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
