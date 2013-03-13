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

class SkeletonExporterPanel(bpy.types.Panel):
	bl_idname = "ogre3d_skeleton_panel"
	bl_label = "Ogre Mesh Exporter"
	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "data"

	@classmethod
	def poll(cls, context):
		return context.armature

	def draw(self, context):
		layout = self.layout
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		skeletonSettings = context.armature.ogre_mesh_exporter

		# bone filter type.
		layout.label("Export bone filter:", icon = 'FILTER')
		row = layout.row(True)
		row.prop_enum(skeletonSettings, "exportFilter", 'all', icon = 'ARMATURE_DATA')
		row.prop_enum(skeletonSettings, "exportFilter", 'layers', icon = 'MESH_GRID')
		row.prop_enum(skeletonSettings, "exportFilter", 'groups', icon = 'GROUP_BONE')

		# bone filter by layer UI.
		box = layout.box()
		table = box.column(True)
		if (skeletonSettings.exportFilter == 'layers'):
			table.label("Export Bones In Selected Layers:")
			table.prop(skeletonSettings, "exportBoneLayerMask", text = "")
		elif (skeletonSettings.exportFilter == 'groups'):
			table.label("Export Bones In Selected Bone Groups:")
			row = table.row()
			checkCol = row.column()
			checkCol.alignment = 'RIGHT'
			checkCol.scale_x = 0.185
			col = row.column()

			boneGroups = context.object.pose.bone_groups
			for group in boneGroups:
				exportingGroup = (group.name in skeletonSettings.exportBoneGroupMask)
				checkCol.prop_enum(skeletonSettings, "exportBoneGroupMask", group.name, text = "",
					icon = 'CHECKBOX_HLT' if (exportingGroup) else 'CHECKBOX_DEHLT')
				#~ col.prop_enum(skeletonSettings, "exportBoneGroupMask", group.name, icon = 'GROUP_BONE')
				col.prop(globalSettings, "dummyTrue", toggle = True, text = group.name, icon = 'GROUP_BONE')
		else: table.label("Export All Bones", icon = 'INFO')
