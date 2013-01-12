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

from ogre_mesh_exporter.global_properties import loadStaticConfig
from ogre_mesh_exporter.global_properties import saveStaticConfig
from ogre_mesh_exporter.mesh_exporter import exportMesh

import bpy, os
from bpy.app.handlers import persistent

class MainExporterPanel(bpy.types.Panel):
	bl_idname = "ogre3d_exporter_panel"
	bl_label = "Ogre Mesh Exporter"
	bl_space_type = "VIEW_3D"
	bl_region_type = "TOOLS"

	# flag to tell if we are in preference view or not.
	mPreference_view = False
	mFirstLoad = True
	mSelectionRefreshState = 0

	@persistent
	def refreshSelection(context):
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		collection = globalSettings.selectedObjectList.collection

		# CollectionProperty stupidly has no clear function. So we have to remove one by one.
		while (len(collection) > 0):
			collection.remove(0)

		for object in bpy.context.selected_objects:
			if (object.type == 'MESH'):
				item = collection.add()
				item.name = object.name

		# NOTE1: we need to remove the callback first before setting the frame.
		# this is because calling frame_set() refreshes the whole scene again which will end up
		# randomly(?) calling this callback function again if we don't stop listening first.
		# NOTE2: we call frame_set() for a stupid hackish reason to refresh the UI to reflect the selection change.
		bpy.app.handlers.scene_update_pre.remove(MainExporterPanel.refreshSelection)
		context.frame_set(context.frame_current)
		panel = bpy.types.ogre3d_exporter_panel
		# flag selection refresh state to 2.
		# this is because the poll() event will be triggered again due to scene refresh.
		# this way, the poll event will correctly ignore registering callback for refreshing of selection.
		panel.mSelectionRefreshState = 2

	# take advantage of the poll event to refresh our selection.
	@classmethod
	def poll(self, context):
		# due to context issue, we cannot modify RNA data during drawing stage which is what this poll stage is.
		# hence we take advantage of the scene update handler callback to do the actual refresh.
		# however, we do not want to keep refreshing every frame which is rediculous.
		# this is why we have the mRefreshingSelection flag to make sure we only refresh when nessesary.
		# once the selection is refreshed, we reset this flag and remove the callback so this polling path can continue again.
		if (self.mSelectionRefreshState == 0) :
			self.mSelectionRefreshState = 1
			bpy.app.handlers.scene_update_pre.append(MainExporterPanel.refreshSelection)
		elif (self.mSelectionRefreshState == 2): self.mSelectionRefreshState = 0
		return True

	def draw(self, context):
		layout = self.layout
		globalSettings = bpy.context.scene.ogre_mesh_exporter

		if (self.mPreference_view):
			col = layout.column(True)
			col.label("Global Static Config (shared across blend files):")
			staticConfigBox = col.box()
			col = staticConfigBox.column()
			col.prop(globalSettings, "ogreXMLConverterPath")
			col.prop(globalSettings, "ogreXMLConverterAdditionalArg")
			row = col.row()
			row.split()
			row.operator("ogre3d.preferences_apply_static_config", icon = 'LIBRARY_DATA_DIRECT')

			xmlConverterOptionsBox = layout.column(True)
			xmlConverterOptionsBox.label("Global XML Converter Options:")

			disableConverterOptions = not globalSettings.useXMLConverterOptions
			row = xmlConverterOptionsBox.row(True)
			row.prop(globalSettings, "useXMLConverterOptions", "", 'MODIFIER', toggle = True)
			row = row.row(True)
			if (disableConverterOptions): row.enabled = False
			row.prop(globalSettings, "extremityPoints")
			row.prop(globalSettings, "edgeLists", toggle = True)
			row.prop(globalSettings, "tangent", toggle = True)
			row = xmlConverterOptionsBox.row(True)
			if (disableConverterOptions): row.enabled = False
			row.prop(globalSettings, "tangentSemantic", "")
			row.prop(globalSettings, "tangentSize", "")
			row.prop(globalSettings, "splitMirrored", toggle = True)
			row.prop(globalSettings, "splitRotated", toggle = True)
			row = xmlConverterOptionsBox.row(True)
			if (disableConverterOptions): row.enabled = False
			row.prop(globalSettings, "reorganiseVertBuff", toggle = True)
			row.prop(globalSettings, "optimiseAnimation", toggle = True)

			row = layout.row()
			row.scale_y = 1.5
			row.operator("ogre3d.preferences_back", icon = 'BACK')
			return

		# display selection list.
		col = layout.column()
		col.label("Selected:")
		selectedObjectList = globalSettings.selectedObjectList
		col.template_list(selectedObjectList, "collection", selectedObjectList, "collectionIndex")

		# ####################################################
		# Material settings
		disableMaterialSettings = not globalSettings.exportMaterials
		materialExportBox = layout.column(True)
		materialExportBox.label("Material Settings:")
		row = materialExportBox.row(True)
		row.prop(globalSettings, "exportMaterials", "", 'MATERIAL', toggle = True)
		row = row.row(True)
		if (disableMaterialSettings): row.enabled = False
		row.prop(globalSettings, "materialFile", "")
		row.prop(globalSettings, "copyTextures", "", 'TEXTURE', toggle = True)
		row = materialExportBox.row(True)
		if (disableMaterialSettings): row.enabled = False
		row.prop_enum(globalSettings, "materialExportMode", 'rend')
		row.prop_enum(globalSettings, "materialExportMode", 'game')
		row.prop_enum(globalSettings, "materialExportMode", 'custom')
		row = materialExportBox.row(True)
		if (disableMaterialSettings or globalSettings.materialExportMode != 'custom'): row.enabled = False
		row.prop(globalSettings, "templatePath", "")

		# ####################################################
		# Mesh settings
		disableMeshSettings = not globalSettings.exportMeshes
		meshExportBox = layout.column(True)
		meshExportBox.label("Mesh Settings:")
		row = meshExportBox.row(True)
		row.prop(globalSettings, "exportMeshes", "", 'MESH_MONKEY', toggle = True)
		row = row.row(True)
		if (disableMeshSettings): row.enabled = False
		row.prop(globalSettings, "fixUpAxisToY", icon = 'NONE', toggle = True)
		row.prop(globalSettings, "requireMaterials", icon = 'NONE', toggle = True)
		row.prop(globalSettings, "applyModifiers", icon = 'NONE', toggle = True)
		row = meshExportBox.row(True)
		if (disableMeshSettings): row.enabled = False
		row.prop(globalSettings, "skeletonNameFollowMesh", icon = 'NONE', toggle = True)
		row.prop(globalSettings, "runOgreXMLConverter", icon = 'NONE', toggle = True)

		exportPathValid = os.path.isdir(globalSettings.exportPath)

		exportPathBox = layout.column(True)
		exportPathBox.label("Export Path:")
		exportPathBox.prop(globalSettings, "exportPath", "",
			icon = ('NONE' if (exportPathValid) else 'ERROR'))

		row = layout.row(True)
		row.scale_y = 1.5
		exportRow = row.row(True)
		exportRow.scale_y = 1.5
		if (not exportPathValid): exportRow.enabled = False
		exportRow.operator("ogre3d.export", icon = 'SCRIPTWIN')
		row.operator("ogre3d.preferences", icon = 'SETTINGS')
		row.operator("ogre3d.help", icon = 'HELP')

class OperatorExport(bpy.types.Operator):
	bl_idname = "ogre3d.export"
	bl_label = "Export"
	bl_description = "Export selected meshes"

	def invoke(self, context, event):
		# make sure the global data is loaded first if it hasn't already.
		panel = bpy.types.ogre3d_exporter_panel
		if (panel.mFirstLoad):
			loadStaticConfig()
			panel.mFirstLoad = False

		globalSettings = bpy.context.scene.ogre_mesh_exporter
		collection = globalSettings.selectedObjectList.collection
		for item in collection:
			object = bpy.data.objects[item.name]
			exportMesh(object, "%s%s.mesh.xml" % (globalSettings.exportPath, item.name))

		return {'FINISHED'}

class OperatorPreferences(bpy.types.Operator):
	bl_idname = "ogre3d.preferences"
	bl_label = "Preferences"
	bl_description = "Exporter preferences that is static across all blend files."

	def invoke(self, context, event):
		panel = bpy.types.ogre3d_exporter_panel
		panel.mPreference_view = True
		if (panel.mFirstLoad):
			loadStaticConfig()
			panel.mFirstLoad = False
		return {'FINISHED'}

class OperatorHelp(bpy.types.Operator):
	bl_idname = "ogre3d.help"
	bl_label = "Help"
	bl_description = "Open help document."

	def invoke(self, context, event):
		return {'FINISHED'}

class OperatorPrefApplyStaticConfig(bpy.types.Operator):
	bl_idname = "ogre3d.preferences_apply_static_config"
	bl_label = "Apply"
	bl_description = "Apply static configs."

	def invoke(self, context, event):
		bpy.types.ogre3d_exporter_panel.mPreference_view = False
		saveStaticConfig()
		return {'FINISHED'}

class OperatorPrefBack(bpy.types.Operator):
	bl_idname = "ogre3d.preferences_back"
	bl_label = "Back"
	bl_description = "Back to main panel."

	def invoke(self, context, event):
		bpy.types.ogre3d_exporter_panel.mPreference_view = False
		loadStaticConfig()
		return {'FINISHED'}
