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

bl_info = {
	"name": "Ogre Mesh Exporter",
	"description": "Exports mesh and (skeletal/morph/pose) animations to Ogre3D format.",
	"author": "Lih-Hern Pang",
	"version": (2, 0, 3),
	"blender": (2, 6, 4),
	"api": 44136,
	"location": "3D View Side Panel",
	"warning": '', # used for warning icon and text in addons panel
	"wiki_url": "http://www.ogre3d.org/tikiwiki/Blender+2.5+Exporter",
	"tracker_url": "http://sourceforge.net/tracker/?group_id=2997&atid=302997",
	"category": "Import-Export"}

if "bpy" in locals():
	import imp
	imp.reload(global_properties)
	imp.reload(material_properties)
	imp.reload(skeleton_properties)
	imp.reload(mesh_properties)
	imp.reload(main_exporter_panel)
	imp.reload(log_manager)
	#~ imp.reload(material_panel)
	imp.reload(skeleton_panel)
	imp.reload(skeleton_impl)
	imp.reload(mesh_panel)
	imp.reload(mesh_exporter)
	imp.reload(mesh_impl)
else:
	from . import global_properties
	from . import material_properties
	from . import skeleton_properties
	from . import mesh_properties
	from . import main_exporter_panel
	from . import log_manager
	#~ from . import material_panel
	from . import skeleton_panel
	from . import skeleton_impl
	from . import mesh_panel
	from . import mesh_exporter
	from . import mesh_impl

import bpy, mathutils
from bpy.props import PointerProperty
from bpy.app.handlers import persistent
from ogre_mesh_exporter.global_properties import GlobalProperties, loadStaticConfig
from ogre_mesh_exporter.material_properties import MaterialProperties
from ogre_mesh_exporter.skeleton_properties import SkeletonProperties
from ogre_mesh_exporter.mesh_properties import MeshProperties
from ogre_mesh_exporter.main_exporter_panel import MainExporterPanel

@persistent
def onBlendLoadHandler(dummy):
	loadStaticConfig()

# registering and menu integration
def register():
	bpy.utils.register_module(__name__)

	# initialize the global properties group.
	bpy.types.Scene.ogre_mesh_exporter = PointerProperty(
		name = "Ogre Mesh Exporter properties",
		type = GlobalProperties,
		description = "Ogre Mesh Exporter properties"
	)

	bpy.types.Material.ogre_mesh_exporter = PointerProperty(
		name = "Ogre Mesh Exporter properties",
		type = MaterialProperties,
		description = "Ogre Mesh Exporter properties"
	)

	bpy.types.Armature.ogre_mesh_exporter = PointerProperty(
		name = "Ogre Mesh Exporter properties",
		type = SkeletonProperties,
		description = "Ogre Mesh Exporter properties"
	)

	bpy.types.Mesh.ogre_mesh_exporter = PointerProperty(
		name = "Ogre Mesh Exporter properties",
		type = MeshProperties,
		description = "Ogre Mesh Exporter properties"
	)

	# register scene update callback.
	# NOTE: This is for a hack to allow us to list selected objects on the fly.
	# SEE: MainExporterPanel.refreshSelection in mesh_exporter_panel.py
	bpy.app.handlers.scene_update_pre.append(MainExporterPanel.refreshSelection)
	bpy.app.handlers.load_post.append(onBlendLoadHandler)

# unregistering and removing menus
def unregister():
	bpy.utils.unregister_module(__name__)

	# unregister scene update callback.
	# NOTE: This is for a hack to allow us to list selected objects on the fly.
	# SEE: MainExporterPanel.refreshSelection in mesh_exporter_panel.py
	bpy.app.handlers.scene_update_pre.remove(MainExporterPanel.refreshSelection)
	bpy.app.handlers.load_post.remove(onBlendLoadHandler)

if __name__ == "__main__":
	register()
