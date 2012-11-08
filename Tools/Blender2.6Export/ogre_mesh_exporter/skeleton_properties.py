# ##### BEGIN MIT LICENSE BLOCK #####
# Copyright (C) 2012 by Lih-Hern Pang

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

def boneGroupList(self, context):
	pose = context.object.pose
	return [(group.name, group.name, '') for group in pose.bone_groups]

# ##############################################
# Skeleton Properties on the armature objects
class SkeletonProperties(bpy.types.PropertyGroup):
	exportFilter = EnumProperty(
		name = "Export Bones Filter",
		description = "Bone export filtering.",
		items=(("all", "All", "Export all bones."),
				("layers", "Layers", "Export bones in layers."),
				),
		default = 'all',
		options = set())

	# Export layer mask.
	# Took me a while to figure this one out.
	# Blender guys at irc are not helpful too :-(
	# I wonder how the layer UI in Armature shows the dots on the layer buttons.
	exportLayerMaskVector = BoolVectorProperty(
		size = 32, subtype = 'LAYER', options = set())
