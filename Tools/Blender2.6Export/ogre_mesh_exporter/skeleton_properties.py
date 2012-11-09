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

import bpy
from bpy.types import PoseBone
from bpy.props import *

def boneGroupList(self, context):
	boneGroups = context.object.pose.bone_groups
	return {(group.name, group.name, '') for group in boneGroups}

# helper function to get all bone names related to given action.
# Copied from export_fbx.py
def getActionBoneNames(obj, action):
	names = set()
	path_resolve = obj.path_resolve

	for fcu in action.fcurves:
		try:
			prop = path_resolve(fcu.data_path, False)
		except:
			prop = None

		if prop is not None:
			data = prop.data
			if isinstance(data, PoseBone):
				names.add(data.name)
	return names

# Helper class to get linked skeleton object from mesh object if context is on mesh object
def getSkeletonObject(context):
	object = context.object
	if (object.type == 'ARMATURE'): return object

	# get linked armature
	parentObject = object.parent
	if (parentObject and object.parent_type == 'ARMATURE'):
		armatureObject = parentObject
	else:
		# check modifier stack, use first valid armature modifier.
		for modifier in object.modifiers:
			if (modifier.type == 'ARMATURE' and
				(modifier.use_vertex_groups or
				modifier.use_bone_envelopes)):
				armatureObject = modifier.object
	return armatureObject

# Callback to list available actions for armature.
def actionByArmatureList(self, context):
	actions = bpy.data.actions
	object = getSkeletonObject(context)
	armature = object.data
	actionNames = set()

	armatureBoneNames = set([bBone.name for bBone in armature.bones])
	for action in actions:
		actionBoneNames = getActionBoneNames(object, action)
		if (armatureBoneNames.intersection(actionBoneNames)):  # at least one channel matches.
			actionNames.add((action.name, action.name, ''))
	return actionNames

# Single skeleton animation action sequence.
class SkeletonActionSequence(bpy.types.PropertyGroup):
	def onActionChanged(self, context):
		actionIndex = bpy.data.actions.find(self.action)
		if (actionIndex == -1): return
		action = bpy.data.actions[actionIndex]
		self.name = action.name
		self.startFrame = action.frame_range[0]
		self.endFrame = action.frame_range[1]

	action = EnumProperty(
		name = "Action",
		description = "Action to export from",
		items = actionByArmatureList,
		update = onActionChanged,
		options = set())

	name = StringProperty(name = "Name", default = "Unknown", options = set())

	startFrame = IntProperty(
		name = "Start",
		soft_min = 0,
		soft_max = 300000,
		options = set())

	endFrame = IntProperty(
		name = "End",
		soft_min = 0,
		soft_max = 300000,
		options = set())

# ##############################################
# Skeleton Properties on the armature objects
class SkeletonProperties(bpy.types.PropertyGroup):
	# ##############################################
	# Export Filter Settings

	exportFilter = EnumProperty(
		name = "Export Bones Filter",
		description = "Bone export filtering.",
		items = (("all", "All", "Export all bones."),
				("layers", "Layers", "Export bones in layers."),
				("groups", "Bone Groups", "Export bones in bone groups."),
				),
		default = 'all',
		options = set())

	# Export layer mask.
	# Took me a while to figure this one out.
	# Blender guys at irc are not helpful too :-(
	# I wonder how the layer UI in Armature shows the dots on the layer buttons.
	exportBoneLayerMask = BoolVectorProperty(
		size = 32, subtype = 'LAYER', options = set())

	exportBoneGroupMask = EnumProperty(
		items = boneGroupList,
		options = {'ENUM_FLAG'})

	# ##############################################
	# Animation Settings

	exportSkeletonActions = CollectionProperty(
		name = "Export Actions Set.",
		description = "List of Actions to export for this bone.",
		type = SkeletonActionSequence,
		options = set())
