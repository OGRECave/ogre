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

from ogre_mesh_exporter.global_properties import loadStaticConfig, saveStaticConfig
from ogre_mesh_exporter.mesh_exporter import exportMesh
from ogre_mesh_exporter.log_manager import LogManager, ObjectLog, Message

import bpy, os, time, multiprocessing
from bpy.app.handlers import persistent

class MainExporterPanel(bpy.types.Panel):
	bl_idname = "ogre3d_exporter_panel"
	bl_label = "Ogre Mesh Exporter"
	bl_space_type = "VIEW_3D"
	bl_region_type = "TOOLS"

	# flag to tell which view we are in.
	VS_MAIN = 0
	VS_PREFERENCE = 1
	VS_LOG = 2
	sViewState = VS_MAIN

	@persistent
	def refreshSelection(scene):
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		collection = globalSettings.selectedObjectList.collection

		# get valid selection and store in dictionary.
		# note that we filter of objects sharing the same datablock.
		# Hence only the first object pointing to the same datablock will be used.
		# Also note that we observe the mesh datablock level export setting here.
		selected_objects = bpy.context.selected_objects
		selectedDataObjects = dict()
		for object in bpy.context.selected_objects:
			dataName = object.data.name
			if (object.type == 'MESH' and (not dataName in selectedDataObjects)):
				meshSettings = object.data.ogre_mesh_exporter
				if (meshSettings.exportEnabled):
					selectedDataObjects[dataName] = object

		# To avoid recreating the list stupidly,
		# we play smart and reuse as nessesary.
		# resize collection as necessary.
		selectedCount = len(selectedDataObjects)
		changed = (selectedCount != len(collection))
		while (len(collection) < selectedCount): collection.add() # add more items if needed.
		while (len(collection) > selectedCount): collection.remove(0) # remove items if needed.

		# update list if changed.
		i = 0
		for dataName, object in selectedDataObjects.items():
			collectionItem = collection[i]
			if (collectionItem.name != dataName or collectionItem.objectName != object.name):
				collectionItem.name = dataName
				collectionItem.objectName = object.name
				changed = True
			i += 1

		# skip refreshing if not changed.
		if (not changed): return

		# tell blender to refresh.
		for screen in bpy.data.screens:
			for area in screen.areas:
				if area.type == 'VIEW_3D':
					for region in area.regions:
						if region.type == 'TOOLS':
							region.tag_redraw()

	def draw(self, context):
		layout = self.layout
		globalSettings = bpy.context.scene.ogre_mesh_exporter

		# ####################################################
		# display preference view.
		# ####################################################
		if (MainExporterPanel.sViewState == MainExporterPanel.VS_PREFERENCE):
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

		# ####################################################
		# display log view.
		# ####################################################
		elif (MainExporterPanel.sViewState == MainExporterPanel.VS_LOG):
			LogManager.drawLog(layout)
			return

		# ####################################################
		# display main view.
		# ####################################################

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
		subrow = row.row()
		subrow.scale_y = 1.5
		exportRow = subrow.row(True)
		exportRow.scale_y = 1.5
		if (not exportPathValid or len(selectedObjectList.collection) == 0 or
			(not globalSettings.exportMeshes and not globalSettings.exportMaterials)):
			exportRow.enabled = False
		exportRow.operator("ogre3d.export", icon = 'EXPORT')
		subrow.operator("ogre3d.preferences", icon = 'SETTINGS')
		subrow.operator("ogre3d.help", icon = 'HELP')
		row = row.row()
		row.scale_y = 1.5
		row.alignment = 'RIGHT'
		row.operator("ogre3d.log", "", icon = 'CONSOLE')

class OperatorExport(bpy.types.Operator):
	bl_idname = "ogre3d.export"
	bl_label = "Export"
	bl_description = "Export selected meshes"

	MAX_PENDING_PROCESSES = multiprocessing.cpu_count()

	def refresh(self, context):
		# tell blender to refresh.
		for area in context.screen.areas:
			if area.type == 'VIEW_3D':
				for region in area.regions:
					if region.type == 'TOOLS':
						region.tag_redraw()

	def modal(self, context, event):
		if (event.type == 'ESC'):  # Cancel
			for objectLog, processList in self.pendingProcesses.items():
				for process in processList: process.kill() 
				objectLog.mStatus = "(Canceling...)"
			self.canceling = True
			self.refresh(context)
			return {'RUNNING_MODAL'}

		if (LogManager.getLogCount() == 0):
			# add first item to log so we can see the progress.
			# TODO: Fix this for when no mesh data are exported but materials are.
			LogManager.addObjectLog(self.collection[0].name, ObjectLog.TYPE_MESH)
			LogManager.setProgress(0)
			self.refresh(context)
			return {'RUNNING_MODAL'}

		# poll subprocesses to make sure they are done and log accordingly.
		pendingDelete = list()
		for objectLog, processList in self.pendingProcesses.items():
			result = 0
			for process in processList:
				pResult = process.poll()
				if (pResult == None): continue
				result += pResult
			if (result == 0):
				objectLog.mStatus = ""
				objectLog.logMessage("OgreXMLConverter Success!")
				objectLog.mState = ObjectLog.ST_SUCCEED
				self.completedCount += 1
			elif (self.canceling):
				objectLog.mStatus = "(Canceled)"
				objectLog.logMessage("OgreXMLConverter Canceled.", Message.LVL_INFO)
				objectLog.mState = ObjectLog.ST_CANCELED
			else:
				objectLog.mStatus = ""
				objectLog.logMessage("OgreXMLConverter Failed! Check log file for more detail.", Message.LVL_ERROR)
				objectLog.mState = ObjectLog.ST_FAILED
			pendingDelete.append(objectLog) # cache it for delete.
		# delete from dictionary what needs to be deleted.
		# (Wish python has smarter way to do this -_-" Why can't they have erase iterator system?)
		for objectLog in pendingDelete: del self.pendingProcesses[objectLog]
		# check that we do not have too many process running. If so, skip until done.
		if (len(self.pendingProcesses) == OperatorExport.MAX_PENDING_PROCESSES):
			self.refresh(context)
			return {'RUNNING_MODAL'}

		# Check exit strategy.
		if (self.itemIndex == self.collectionCount or self.canceling):
			if (len(self.pendingProcesses)): return {'RUNNING_MODAL'}
			context.window_manager.event_timer_remove(self.timer)
			LogManager.setProgress(100)
			self.refresh(context)
			return {'FINISHED'}

		# TODO: Handle exportMeshes == FALSE state.
		item = self.collection[self.itemIndex]
		object = bpy.data.objects[item.objectName]
		#~ time.sleep(0.1)
		result = exportMesh(object, self.globalSettings.exportPath + item.name)
		objectLog = LogManager.getObjectLog(-1)
		if (False in result): objectLog.mState = ObjectLog.ST_FAILED
		elif (len(result)):
			self.pendingProcesses[objectLog] = result
			objectLog.mStatus = "(Converting...)"
			objectLog.mState = ObjectLog.ST_CONVERTING
		else:
			objectLog.mState = ObjectLog.ST_SUCCEED
			self.completedCount += 1

		# update progress bar.
		LogManager.setProgress((100 * self.completedCount) / self.collectionCount)

		self.itemIndex += 1
		if (self.itemIndex < self.collectionCount):
			LogManager.addObjectLog(self.collection[self.itemIndex].name, ObjectLog.TYPE_MESH)

		# tell blender to refresh.
		self.refresh(context)
		return {'RUNNING_MODAL'}

	def invoke(self, context, event):
		# change our view to log panel.
		MainExporterPanel.sViewState = MainExporterPanel.VS_LOG

		self.globalSettings = bpy.context.scene.ogre_mesh_exporter
		self.collection = self.globalSettings.selectedObjectList.collection
		self.collectionCount = len(self.collection)
		self.pendingProcesses = dict()
		self.completedCount = 0
		self.canceling = False
		self.itemIndex = 0

		# clear log.
		LogManager.reset()

		# run modal operator mode.
		context.window_manager.modal_handler_add(self)
		self.timer = context.window_manager.event_timer_add(0.1, context.window)
		return {'RUNNING_MODAL'}

class OperatorPreferences(bpy.types.Operator):
	bl_idname = "ogre3d.preferences"
	bl_label = "Preferences"
	bl_description = "Exporter preferences that is static across all blend files."

	def invoke(self, context, event):
		MainExporterPanel.sViewState = MainExporterPanel.VS_PREFERENCE
		return {'FINISHED'}

class OperatorHelp(bpy.types.Operator):
	bl_idname = "ogre3d.help"
	bl_label = "Help"
	bl_description = "Open help document."

	def invoke(self, context, event):
		return {'FINISHED'}

class OperatorShowLog(bpy.types.Operator):
	bl_idname = "ogre3d.log"
	bl_label = "Log History"
	bl_description = "Show log history."

	def invoke(self, context, event):
		MainExporterPanel.sViewState = MainExporterPanel.VS_LOG
		return {'FINISHED'}

class OperatorPrefApplyStaticConfig(bpy.types.Operator):
	bl_idname = "ogre3d.preferences_apply_static_config"
	bl_label = "Apply"
	bl_description = "Apply static configs."

	def invoke(self, context, event):
		saveStaticConfig()
		return {'FINISHED'}

class OperatorPrefBack(bpy.types.Operator):
	bl_idname = "ogre3d.preferences_back"
	bl_label = "Back"
	bl_description = "Back to main panel."

	def invoke(self, context, event):
		MainExporterPanel.sViewState = MainExporterPanel.VS_MAIN
		loadStaticConfig()
		return {'FINISHED'}

class OperatorLogBack(bpy.types.Operator):
	bl_idname = "ogre3d.log_back"
	bl_label = "Back"
	bl_description = "Back to main panel."

	def invoke(self, context, event):
		MainExporterPanel.sViewState = MainExporterPanel.VS_MAIN
		return {'FINISHED'}
