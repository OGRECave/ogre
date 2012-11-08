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

import bpy, pprint

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
		skeletonSettings = context.armature.ogre_mesh_exporter

		layout.label("Export bone filter:", icon = 'FILTER')
		row = layout.row(True)
		row.prop_enum(skeletonSettings, "exportFilter", 'all', icon = 'ARMATURE_DATA')
		row.prop_enum(skeletonSettings, "exportFilter", 'layers', icon = 'MESH_GRID')

		col = layout.column()
		col.enabled = skeletonSettings.exportFilter == 'layers'
		col.prop(skeletonSettings, "exportLayerMaskVector", text = "")
