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

import bpy

class MeshExporterPanel(bpy.types.Panel):
	bl_idname = "ogre3d_mesh_panel"
	bl_label = "Ogre Mesh Exporter"
	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "data"

	@classmethod
	def poll(cls, context):
		return context.mesh

	def draw(self, context):
		layout = self.layout
		meshSettings = context.mesh.ogre_mesh_exporter

		layout.prop(meshSettings, "exportEnabled", icon = 'MESH_MONKEY', toggle = True)
		if (not meshSettings.exportEnabled): return

		# Mesh override settings:
		layout.label("Mesh Override Settings:")
		col = layout.column(True)

		row = col.row(True)
		overrideSetting = meshSettings.requireMaterials_override
		row.prop(meshSettings, "requireMaterials_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "requireMaterials", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.skeletonNameFollowMesh_override
		row.prop(meshSettings, "skeletonNameFollowMesh_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "skeletonNameFollowMesh", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.applyModifiers_override
		row.prop(meshSettings, "applyModifiers_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "applyModifiers", toggle = True)

		# XML Converter override settings:
		layout.label("XML Converter Override Settings:")
		col = layout.column(True)

		row = col.row(True)
		overrideSetting = meshSettings.extremityPoints_override
		row.prop(meshSettings, "extremityPoints_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "extremityPoints")

		row = col.row(True)
		overrideSetting = meshSettings.edgeLists_override
		row.prop(meshSettings, "edgeLists_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "edgeLists", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.tangent_override
		row.prop(meshSettings, "tangent_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "tangent", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.tangentSemantic_override
		row.prop(meshSettings, "tangentSemantic_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "tangentSemantic", "")

		row = col.row(True)
		overrideSetting = meshSettings.tangentSize_override
		row.prop(meshSettings, "tangentSize_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "tangentSize", "")

		row = col.row(True)
		overrideSetting = meshSettings.splitMirrored_override
		row.prop(meshSettings, "splitMirrored_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "splitMirrored", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.splitRotated_override
		row.prop(meshSettings, "splitRotated_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "splitRotated", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.reorganiseVertBuff_override
		row.prop(meshSettings, "reorganiseVertBuff_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "reorganiseVertBuff", toggle = True)

		row = col.row(True)
		overrideSetting = meshSettings.optimiseAnimation_override
		row.prop(meshSettings, "optimiseAnimation_override", "",
			icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
		if (not overrideSetting):
			row = row.row(True)
			row.enabled = False
		row.prop(meshSettings, "optimiseAnimation", toggle = True)

# registering and menu integration
def register():
	bpy.utils.register_module(__name__)

# unregistering and removing menus
def unregister():
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
