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

# Helper class to get linked skeleton settings from mesh object
def getSkeletonSettings(context):
	# get linked armature
	meshObject = context.object
	parentObject = meshObject.parent
	armatureObject = None
	if (parentObject and meshObject.parent_type == 'ARMATURE'):
		armatureObject = parentObject
	else:
		# check modifier stack, use first valid armature modifier.
		for modifier in meshObject.modifiers:
			if (modifier.type == 'ARMATURE' and
				(modifier.use_vertex_groups or
				modifier.use_bone_envelopes)):
				armatureObject = modifier.object

	return armatureObject.data.ogre_mesh_exporter \
		if (armatureObject is not None) else None

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
		globalSettings = bpy.context.scene.ogre_mesh_exporter
		meshSettings = context.mesh.ogre_mesh_exporter
		skeletonSettings = getSkeletonSettings(context)

		row = layout.row(True)
		row.prop(meshSettings, "exportEnabled", icon = 'EXPORT', toggle = True)

		row = row.row()
		row.alignment = 'RIGHT'
		row.scale_x = 0.3
		row.enabled = meshSettings.exportEnabled
		row.prop_enum(meshSettings, "exportTab", 'mesh', text = "", icon = 'MESH_MONKEY')
		row.prop_enum(meshSettings, "exportTab", 'animation', text = "", icon = 'ANIM')
		row.prop_enum(meshSettings, "exportTab", 'settings', text = "", icon = 'SETTINGS')

		if (not meshSettings.exportEnabled): return

		# prepare material slot shared vertex info.
		subMeshProperties = meshSettings.subMeshProperties
		materialList = context.mesh.materials
		materialCount = len(materialList)
		while (len(subMeshProperties) < materialCount): subMeshProperties.add() # add more items if needed.
		while (len(subMeshProperties) > materialCount): subMeshProperties.remove(0) # remove items if needed.

		if (meshSettings.exportTab == 'mesh'):
			layout.label("Submesh Properties:")
			submeshNames = list()
			box = layout.box()
			if (len(subMeshProperties) == 0): box.label("No Materials Defined", icon = 'INFO')
			for index, subMeshProperty in enumerate(subMeshProperties):
				row = box.row(True)

				# Material index & name.
				material = materialList[index]
				row.label("[%d]%s" % (index, "NONE" if (material == None) else materialList[index].name), icon = 'ERROR' if (material == None) else 'MATERIAL')

				# Submesh name.
				subrow = row.row()
				if (subMeshProperty.name in submeshNames): subrow.alert = True
				else: submeshNames.append(subMeshProperty.name)
				subrow.prop(subMeshProperty, "name", "")

				# Use shared vertices.
				row.prop(subMeshProperty, "useSharedVertices", "", icon = 'GROUP_VERTEX')

				# Select vertices under this submesh.
				if (context.mode == 'EDIT_MESH'):
					prop = row.operator("ogre3d.select_submesh_vertices", "", icon='MESH_DATA')
					prop.index = index

		elif (meshSettings.exportTab == 'animation'):
			# Animations.
			layout.separator()
			row = layout.row(True)
			row.label("Animations:")
			row.prop_enum(meshSettings, "animationTab", 'skel', icon = 'POSE_DATA')
			row.prop_enum(meshSettings, "animationTab", 'pose', icon = 'OUTLINER_DATA_MESH')
			row.prop_enum(meshSettings, "animationTab", 'morph', icon = 'OUTLINER_DATA_MESH')

			box = layout.box()
			table = box.column(True)
			row = table.row()
			col = row.column()
			delCol = row.column()

			# draw grid header.
			row = col.row()
			row.prop(globalSettings, "dummyTrue", toggle = True, text = "Action")
			row.prop(globalSettings, "dummyTrue", toggle = True, text = "Name")
			frameRow = row.row()
			frameRow.scale_x = 0.5
			frameRow.prop(globalSettings, "dummyTrue", toggle = True, text = "Start")
			frameRow.prop(globalSettings, "dummyTrue", toggle = True, text = "End")
			delCol.prop(globalSettings, "dummyTrue", text = "", icon = 'SCRIPTWIN')

			# Populate skeletal animation action list.
			if (meshSettings.animationTab == 'skel'):
				if (skeletonSettings is not None):
					if (len(skeletonSettings.exportSkeletonActions) == 0):
						table.prop(globalSettings, "dummyFalse", toggle = True, text = "No Animations")
					for index, item in enumerate(skeletonSettings.exportSkeletonActions):
						row = col.row()
						row.prop(item, "action", text = "", icon = 'ACTION')
						row.prop(item, "name", text = "")
						frameRow = row.row()
						frameRow.scale_x = 0.5
						frameRow.prop(item, "startFrame", text = "")
						frameRow.prop(item, "endFrame", text = "")
						prop = delCol.operator("ogre3d.skeleton_delete_animation", text = "", icon = 'ZOOMOUT')
						prop.index = index
					box.operator("ogre3d.skeleton_add_animation", icon = 'ZOOMIN')
				else:
					table.prop(globalSettings, "dummyFalse", toggle = True, text = "No Armature Link")
			elif (meshSettings.animationTab == 'pose'):
				table.prop(globalSettings, "dummyFalse", toggle = True, text = "Not Implemented Yet")
			else:
				table.prop(globalSettings, "dummyFalse", toggle = True, text = "Not Implemented Yet")

		else:
			# Mesh override settings:
			layout.label("Mesh Override Settings:")
			col = layout.column(True)
			row = col.row()
			overrideSetting = meshSettings.requireMaterials_override
			row.prop(meshSettings, "requireMaterials_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "requireMaterials", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.skeletonNameFollowMesh_override
			row.prop(meshSettings, "skeletonNameFollowMesh_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "skeletonNameFollowMesh", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.applyModifiers_override
			row.prop(meshSettings, "applyModifiers_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "applyModifiers", toggle = True)

			# XML Converter override settings:
			layout.label("XML Converter Override Settings:")
			col = layout.column(True)
			row = col.row()
			overrideSetting = meshSettings.extremityPoints_override
			row.prop(meshSettings, "extremityPoints_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "extremityPoints")

			row = col.row()
			overrideSetting = meshSettings.edgeLists_override
			row.prop(meshSettings, "edgeLists_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "edgeLists", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.tangent_override
			row.prop(meshSettings, "tangent_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "tangent", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.tangentSemantic_override
			row.prop(meshSettings, "tangentSemantic_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "tangentSemantic", "")

			row = col.row()
			overrideSetting = meshSettings.tangentSize_override
			row.prop(meshSettings, "tangentSize_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "tangentSize", "")

			row = col.row()
			overrideSetting = meshSettings.splitMirrored_override
			row.prop(meshSettings, "splitMirrored_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "splitMirrored", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.splitRotated_override
			row.prop(meshSettings, "splitRotated_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "splitRotated", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.reorganiseVertBuff_override
			row.prop(meshSettings, "reorganiseVertBuff_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "reorganiseVertBuff", toggle = True)

			row = col.row()
			overrideSetting = meshSettings.optimiseAnimation_override
			row.prop(meshSettings, "optimiseAnimation_override", "",
				icon = 'PINNED' if overrideSetting else 'UNPINNED', toggle = True)
			if (not overrideSetting):
				row = row.row()
				row.enabled = False
			row.prop(meshSettings, "optimiseAnimation", toggle = True)

class OperatorSelectSubmeshVertices(bpy.types.Operator):
	bl_idname = "ogre3d.select_submesh_vertices"
	bl_label = "Select"
	bl_description = "Select submesh vertices."

	index = bpy.props.IntProperty()

	def invoke(self, context, event):
		context.object.active_material_index = self.index
		bpy.ops.mesh.select_all(action = 'DESELECT')
		bpy.ops.object.material_slot_select()
		return {'FINISHED'}

class OperatorSkeletonAddAnimation(bpy.types.Operator):
	bl_idname = "ogre3d.skeleton_add_animation"
	bl_label = "Add"
	bl_description = "Add new skeleton animation."

	def invoke(self, context, event):
		skeletonSettings = getSkeletonSettings(context)
		item = skeletonSettings.exportSkeletonActions.add()
		item.onActionChanged(context)
		return {'FINISHED'}

class OperatorSkeletonDeleteAnimation(bpy.types.Operator):
	bl_idname = "ogre3d.skeleton_delete_animation"
	bl_label = "Delete"
	bl_description = "Delete skeleton animation."

	index = bpy.props.IntProperty()

	def invoke(self, context, event):
		skeletonSettings = getSkeletonSettings(context)
		skeletonSettings.exportSkeletonActions.remove(self.index)
		return {'FINISHED'}
