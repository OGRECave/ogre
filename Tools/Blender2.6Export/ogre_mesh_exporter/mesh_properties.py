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

class SubMeshProperties(bpy.types.PropertyGroup):
	# Flag to tell if this submesh should use shared vertices.
	useSharedVertices = BoolProperty(
		name = "Use Shared Vertices",
		description = "Use shared vertices with other submeshes.",
		default = True,
		options = set()
	)

	# Custom name of submesh.
	name = StringProperty(
		name = "Custom name",
		description = "Custom name of submesh.",
		default = "",
		options = set()
	)

# ##############################################
# Mesh Properties on the mesh objects
class MeshProperties(bpy.types.PropertyGroup):
	# Enable/Disable export of this mesh.
	exportEnabled = BoolProperty(
		name = "Export",
		description = "Export this mesh.",
		default = True,
		options = set()
	)

	exportTab = EnumProperty(
		items = (
			("mesh", "Mesh", "Mesh tab"),
			("animation", "Animation", "Animation tab"),
			("settings", "Override Settings", "Override global settings tab")),
		default = "mesh",
		options = {'SKIP_SAVE'}
	)

	subMeshProperties = CollectionProperty(type = SubMeshProperties)

	animationTab = EnumProperty(
		items = (
			("skel", "Skeleton", "Skeleton animation tab"),
			("pose", "Pose", "Vertex Pose animation tab"),
			("morph", "Morph", "Vertex Morph animation tab")),
		default = "skel",
		options = {'SKIP_SAVE'}
	)


	# ##############################################
	# Export override specific Properties
	requireMaterials_override = BoolProperty(
		name = "Require Materials Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	requireMaterials = BoolProperty(
		name = "Require Materials",
		description = "Generate Error message when part of this mesh is not assigned with a material.",
		default = True,
		options = set()
	)

	applyModifiers_override = BoolProperty(
		name = "Apply Modifiers Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	applyModifiers = BoolProperty(
		name = "Apply Modifiers",
		description = "Apply mesh modifiers before export. (Slow and may break vertex order for morph targets!)",
		default = False,
		options = set()
	)

	skeletonNameFollowMesh_override = BoolProperty(
		name = "Skeleton Name Follow Mesh Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	skeletonNameFollowMesh = BoolProperty(
		name = "Skeleton Name Follow Mesh",
		description = "Use mesh name for exported skeleton name instead of the armature name.",
		default = True,
		options = set()
	)

	# ##############################################
	# XML Converter specific Properties
	extremityPoints_override = BoolProperty(
		name = "Extremity Points Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	extremityPoints = IntProperty(
		name = "Extremity Points",
		description = "Generate no more than num eXtremes for every submesh. (For submesh render sorting when using alpha materials on submesh)",
		soft_min = 0,
		soft_max = 65536,
		options = set()
	)

	edgeLists_override = BoolProperty(
		name = "Edge Lists Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	edgeLists = BoolProperty(
		name = "Edge Lists",
		description = "Generate edge lists. (Useful for outlining or doing stencil shadows)",
		default = False,
		options = set()
	)

	tangent_override = BoolProperty(
		name = "Tangent Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	tangent = BoolProperty(
		name = "Tangent",
		description = "Generate tangent.",
		default = False,
		options = set()
	)

	tangentSemantic_override = BoolProperty(
		name = "Tangent Semantic Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	tangentSemantic = EnumProperty(
		name = "Tangent Semantic",
		description = "Tangent Semantic to use.",
		items=(("uvw", "uvw", "Use UV semantic."),
				("tangent", "tangent", "Use tangent semantic."),
				),
		default= "tangent",
		options = set()
	)

	tangentSize_override = BoolProperty(
		name = "Tangent Size Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	tangentSize = EnumProperty(
		name = "Tangent Size",
		description = "Size of tangent.",
		items=(("4", "4 component (parity)", "Use 4 component tangent where 4th component is parity."),
				("3", "3 component", "Use 3 component tangent."),
				),
		default= "3",
		options = set()
	)

	splitMirrored_override = BoolProperty(
		name = "Split Mirrored Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	splitMirrored = BoolProperty(
		name = "Split Mirrored",
		description = "Split tangent vertices at UV mirror points.",
		default = False,
		options = set()
	)

	splitRotated_override = BoolProperty(
		name = "Split Rotated Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	splitRotated = BoolProperty(
		name = "Split Rotated",
		description = "Split tangent vertices where basis is rotated > 90 degrees.",
		default = False,
		options = set()
	)

	reorganiseVertBuff_override = BoolProperty(
		name = "Reorganise Vertex Buffers Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	reorganiseVertBuff = BoolProperty(
		name = "Reorganise Vertex Buffers",
		description = "Reorganise vertex buffer to make it GPU vertex cache friendly.",
		default = True,
		options = set()
	)

	optimiseAnimation_override = BoolProperty(
		name = "Optimise Animation Override",
		description = "Override global setting.",
		default = False,
		options = set()
	)
	optimiseAnimation = BoolProperty(
		name = "Optimise Animation",
		description = "Optimise out redundant tracks & keyframes.",
		default = True,
		options = set()
	)
