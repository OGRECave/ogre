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

from ogre_mesh_exporter.mesh_properties import MeshProperties

STATIC_CONFIG_FILENAME = "ogre_mesh_exporter.cfg"

class SelectedObject(bpy.types.PropertyGroup):
	name = StringProperty(name="Name", default="Unknown")

class SelectedObjectList(bpy.types.PropertyGroup):
	def onSelectionChanged(self, context):
		# Set the selected object as active.
		bpy.context.scene.objects.active = bpy.data.objects[self.collection[self.collectionIndex].name]

	collection = CollectionProperty(type = SelectedObject)
	collectionIndex = IntProperty(min = -1, default = -1, update=onSelectionChanged)

class GlobalProperties(bpy.types.PropertyGroup):
	# ##############################################
	# Material Properties
	exportMaterials = BoolProperty(
		name = "Export Materials",
		description = "Enable/Disable material file exporting.",
		default = True
	)
	materialFile = StringProperty(
		name = "Material File",
		description = "File name of material.",
		default = "Scene.material"
	)
	copyTextures = BoolProperty(
		name = "Copy Textures",
		description = "Copy textures to export path.",
		default = False
	)
	materialExportMode = EnumProperty(
		name= "Material Export Mode",
		description= "Diffrent Material Export Modes.",
		items=(("rend", "Rendering Materials", "Export using rendering materials."),
				("game", "Game Engine Materials", "Export using game engine materials."),
				("custom",  "Custom Materials", "Export using custom template based materials."),
				),
		default= "rend"
	)
	templatePath = StringProperty(
		name = "Template Path",
		description = "Path to material templates for generating custom materials.",
		subtype = "DIR_PATH"
	)

	# ##############################################
	# Mesh Properties
	exportMeshes = BoolProperty(
		name = "Export Meshes",
		description = "Enable/Disable mesh & skeleton file exporting.",
		default = True
	)
	exportPath = StringProperty(
		name = "Export Path",
		description = "Path to export files.",
		subtype = "DIR_PATH"
	)
	fixUpAxisToY = BoolProperty(
		name = "Fix Up Axis to Y",
		description = "Fix up axis as Y instead of Z.",
		default = True
	)
	requireMaterials = BoolProperty(
		name = "Require Materials",
		description = "Generate Error message when part of a mesh is not assigned with a material.",
		default = True
	)
	skeletonNameFollowMesh = BoolProperty(
		name = "Skeleton Name Follow Mesh",
		description = "Use mesh name for exported skeleton name instead of the armature name.",
		default = True
	)
	applyModifiers = BoolProperty(
		name = "Apply Modifiers",
		description = "Apply mesh modifiers before export. (Slow and may break vertex order for morph targets!)",
		default = False
	)
	runOgreXMLConverter = BoolProperty(
		name = "OgreXMLConverter",
		description = "Run OgreXMLConverter on exported XML files.",
		default = True
	)

	# ##############################################
	# XML Converter Properties

	# This is only a temporary property for editing due to blender's limitation of it's dynamic properties.
	# The true value is stored in the globally shared config file.
	# This means that this value will be the same for all blend file opened.
	ogreXMLConverterPath = StringProperty(
		name = "Ogre XML Converter Path",
		description = "Path to OgreXMLConverter.",
		subtype = "FILE_PATH"
	)
	# Saved to the shared config file as above.
	ogreXMLConverterAdditionalArg = StringProperty(
		name = "Additional Arguments",
		description = "Additional Arguments outside of the provided options below. Note that this is shared across all blend files."
	)

	useXMLConverterOptions = BoolProperty(
		name = "Use XML Converter Options",
		description = "Use the settings set by this XML converter option. These options are saved in blend file. If you want a globally shared option, please uncheck this and use the 'Additional Arguments' option.",
		default = True
	)
	extremityPoints = IntProperty(
		name = "Extremity Points",
		description = "Generate no more than num eXtremes for every submesh. (For submesh render sorting when using alpha materials on submesh)",
		soft_min = 0,
		soft_max = 65536
	)
	edgeLists = BoolProperty(
		name = "Edge Lists",
		description = "Generate edge lists. (Useful for outlining or doing stencil shadows)",
		default = False
	)
	tangent = BoolProperty(
		name = "Tangent",
		description = "Generate tangent.",
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
	tangentSize = EnumProperty(
		name = "Tangent Size",
		description = "Size of tangent.",
		items=(("4", "4 component (parity)", "Use 4 component tangent where 4th component is parity."),
				("3", "3 component", "Use 3 component tangent."),
				),
		default= "3"
	)
	splitMirrored = BoolProperty(
		name = "Split Mirrored",
		description = "Split tangent vertices at UV mirror points.",
		default = False
	)
	splitRotated = BoolProperty(
		name = "Split Rotated",
		description = "Split tangent vertices where basis is rotated > 90 degrees.",
		default = False
	)
	reorganiseVertBuff = BoolProperty(
		name = "Reorganise Vertex Buffers",
		description = "Reorganise vertex buffer to make it GPU vertex cache friendly.",
		default = True
	)
	optimiseAnimation = BoolProperty(
		name = "Optimise Animation",
		description = "Optimise out redundant tracks & keyframes.",
		default = True
	)

	# ##############################################
	# temporary collection for listing selected meshes.
	selectedObjectList = bpy.props.PointerProperty(type = SelectedObjectList)

# Load static data from config file.
def loadStaticConfig():
	global OGRE_XML_CONVERTERPATH
	global_settings = bpy.context.scene.ogre_mesh_exporter

	# load static data from config file.
	config_path = bpy.utils.user_resource('CONFIG')
	config_filepath = os.path.join(config_path, STATIC_CONFIG_FILENAME)
	config = configparser.ConfigParser()
	config.read(config_filepath)

	if sys.platform.startswith('win'):
		global_settings.ogreXMLConverterPath = _parseConfig(config, "PATHS", "OgreXMLConverter", "C:\\OgreCommandLineTools\\OgreXmlConverter.exe")
	elif sys.platform.startswith('linux'):
		global_settings.ogreXMLConverterPath = _parseConfig(config, "PATHS", "OgreXMLConverter", "/usr/bin/OgreXMLConverter")

# Parse static config data.
def _parseConfig(config, section, key, default):
	try:
		return config.get(section, key)
	except configparser.Error:
		return default

# Save static data to config file.
def saveStaticConfig():
	global_settings = bpy.context.scene.ogre_mesh_exporter

	config_path = bpy.utils.user_resource('CONFIG')
	config_filepath = os.path.join(config_path, STATIC_CONFIG_FILENAME)
	config = configparser.ConfigParser()

	config.add_section("PATHS")
	config.set("PATHS", "OgreXMLConverter", global_settings.ogreXMLConverterPath)
	config.read(config_filepath)

	with open(config_filepath, 'w') as configfile:
		config.write(configfile)

# registering and menu integration
def register():
	bpy.utils.register_module(__name__)

# unregistering and removing menus
def unregister():
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
