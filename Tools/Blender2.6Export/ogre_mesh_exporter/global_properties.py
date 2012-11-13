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

STATIC_CONFIG_FILENAME = "ogre_mesh_exporter.cfg"

class SelectedObject(bpy.types.PropertyGroup):
	name = StringProperty(name = "Name", default = "Unknown", options = set())
	objectName = StringProperty(name = "Object", default = "Unknown", options = set())

class SelectedObjectList(bpy.types.PropertyGroup):
	def onSelectionChanged(self, context):
		# Set the selected object as active.
		bpy.context.scene.objects.active = bpy.data.objects[self.collection[self.collectionIndex].objectName]

	collection = CollectionProperty(type = SelectedObject, options = set())
	collectionIndex = IntProperty(min = -1, default = -1, options = set(), update=onSelectionChanged)

class GlobalProperties(bpy.types.PropertyGroup):
	# ##############################################
	# Material Properties
	exportMaterials = BoolProperty(
		name = "Export Materials",
		description = "Enable/Disable material file exporting.",
		default = True,
		options = set()
	)
	materialFile = StringProperty(
		name = "Material File",
		description = "File name of material.",
		default = "Scene.material",
		options = set()
	)
	copyTextures = BoolProperty(
		name = "Copy Textures",
		description = "Copy textures to export path.",
		default = False,
		options = set()
	)
	materialExportMode = EnumProperty(
		name = "Material Export Mode",
		description = "Diffrent Material Export Modes.",
		items = (("rend", "Rendering Materials", "Export using rendering materials."),
				("game", "Game Engine Materials", "Export using game engine materials."),
				("custom",  "Custom Materials", "Export using custom template based materials."),
				),
		default = "rend",
		options = set()
	)
	templatePath = StringProperty(
		name = "Template Path",
		description = "Path to material templates for generating custom materials.",
		subtype = "DIR_PATH",
		options = set()
	)

	# ##############################################
	# Mesh Properties
	exportMeshes = BoolProperty(
		name = "Export Meshes",
		description = "Enable/Disable mesh & skeleton file exporting.",
		default = True,
		options = set()
	)
	exportPath = StringProperty(
		name = "Export Path",
		description = "Path to export files.",
		subtype = "DIR_PATH",
		options = set()
	)
	fixUpAxisToY = BoolProperty(
		name = "Fix Up Axis to Y",
		description = "Fix up axis as Y instead of Z.",
		default = True,
		options = set()
	)
	requireMaterials = BoolProperty(
		name = "Require Materials",
		description = "Generate Error message when part of a mesh is not assigned with a material.",
		default = True,
		options = set()
	)
	applyModifiers = BoolProperty(
		name = "Apply Modifiers",
		description = "Apply mesh modifiers before export. (Slow and may break vertex order for morph targets!)",
		default = False,
		options = set()
	)
	skeletonNameFollowMesh = BoolProperty(
		name = "Skeleton Name Follow Mesh",
		description = "Use mesh name for exported skeleton name instead of the armature name.",
		default = True,
		options = set()
	)
	runOgreXMLConverter = BoolProperty(
		name = "OgreXMLConverter",
		description = "Run OgreXMLConverter on exported XML files.",
		default = True,
		options = set()
	)

	# ##############################################
	# XML Converter Properties

	# This is only a temporary property for editing due to blender's limitation of it's dynamic properties.
	# The true value is stored in the globally shared config file.
	# This means that this value will be the same for all blend file opened.
	ogreXMLConverterPath = StringProperty(
		name = "Ogre XML Converter Path",
		description = "Path to OgreXMLConverter.",
		subtype = "FILE_PATH",
		options = {'SKIP_SAVE'}
	)
	# Saved to the shared config file as above.
	ogreXMLConverterAdditionalArg = StringProperty(
		name = "Additional Arguments",
		description = "Additional Arguments outside of the provided options below. Note that this is shared across all blend files.",
		options = {'SKIP_SAVE'}
	)

	useXMLConverterOptions = BoolProperty(
		name = "Use XML Converter Options",
		description = "Use the settings set by this XML converter option. These options are saved in blend file. If you want a globally shared option, please uncheck this and use the 'Additional Arguments' option.",
		default = True,
		options = {'SKIP_SAVE'}
	)
	extremityPoints = IntProperty(
		name = "Extremity Points",
		description = "Generate no more than num eXtremes for every submesh. (For submesh render sorting when using alpha materials on submesh)",
		soft_min = 0,
		soft_max = 65536,
		options = {'SKIP_SAVE'}
	)
	edgeLists = BoolProperty(
		name = "Edge Lists",
		description = "Generate edge lists. (Useful for outlining or doing stencil shadows)",
		default = False,
		options = {'SKIP_SAVE'}
	)
	tangent = BoolProperty(
		name = "Tangent",
		description = "Generate tangent.",
		default = False,
		options = {'SKIP_SAVE'}
	)
	tangentSemantic = EnumProperty(
		name = "Tangent Semantic",
		description = "Tangent Semantic to use.",
		items=(("uvw", "uvw", "Use UV semantic."),
				("tangent", "tangent", "Use tangent semantic."),
				),
		default= "tangent",
		options = {'SKIP_SAVE'}
	)
	tangentSize = EnumProperty(
		name = "Tangent Size",
		description = "Size of tangent.",
		items=(("4", "4 component (parity)", "Use 4 component tangent where 4th component is parity."),
				("3", "3 component", "Use 3 component tangent."),
				),
		default= "3",
		options = {'SKIP_SAVE'}
	)
	splitMirrored = BoolProperty(
		name = "Split Mirrored",
		description = "Split tangent vertices at UV mirror points.",
		default = False,
		options = {'SKIP_SAVE'}
	)
	splitRotated = BoolProperty(
		name = "Split Rotated",
		description = "Split tangent vertices where basis is rotated > 90 degrees.",
		default = False,
		options = {'SKIP_SAVE'}
	)
	reorganiseVertBuff = BoolProperty(
		name = "Reorganise Vertex Buffers",
		description = "Reorganise vertex buffer to make it GPU vertex cache friendly.",
		default = True,
		options = {'SKIP_SAVE'}
	)
	optimiseAnimation = BoolProperty(
		name = "Optimise Animation",
		description = "Optimise out redundant tracks & keyframes.",
		default = True,
		options = {'SKIP_SAVE'}
	)

	# ##############################################
	# Log properties.
	logPageSize = IntProperty(
		name = "Log Page Size",
		description = "Size of a visible log page",
		default = 10,
		options = {'SKIP_SAVE'}
	)
	logPercentage = IntProperty(
		name = "Log Percentage",
		description = "Log progress",
		default = 100, min = 0, max = 100,
		subtype = 'PERCENTAGE',
		options = {'SKIP_SAVE'}
	)

	# ##############################################
	# temporary collection for listing selected meshes.
	selectedObjectList = PointerProperty(type = SelectedObjectList)

	def onDummyTrueChanged(self, context):
		# Never let Dummy change.
		self.dummyTrue = True
	def onDummyFalseChanged(self, context):
		# Never let Dummy change.
		self.dummyFalse = False

	# Dummy property for tab use. (NEVER SET)
	dummyTrue = BoolProperty(
		default = True,
		update = onDummyTrueChanged,
		options = {'SKIP_SAVE'})

	# Dummy property for label box use. (NEVER SET)
	dummyFalse = BoolProperty(
		default = False,
		update = onDummyFalseChanged,
		options = {'SKIP_SAVE'})

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
