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
	"version": (2,0,1),
	"blender": (2, 5, 7),
	"api": 31236,
	"location": "3D View Side Panel",
	"warning": '', # used for warning icon and text in addons panel
	"wiki_url": "http://www.ogre3d.org/tikiwiki/Blender+2.5+Exporter",
	"tracker_url": "http://sourceforge.net/tracker/?group_id=2997&atid=302997",
	"category": "Import-Export"}

if "bpy" in locals():
	import imp
	imp.reload(global_properties)
	imp.reload(material_properties)
	imp.reload(mesh_properties)
	imp.reload(main_exporter_panel)
	imp.reload(mesh_panel)
	imp.reload(mesh_exporter)
else:
	from . import global_properties
	from . import material_properties
	from . import mesh_properties
	from . import main_exporter_panel
	from . import mesh_panel
	from . import mesh_exporter

import bpy, pprint
from bpy.props import PointerProperty
from ogre_mesh_exporter.global_properties import GlobalProperties
from ogre_mesh_exporter.material_properties import MaterialProperties
from ogre_mesh_exporter.mesh_properties import MeshProperties

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

	bpy.types.Mesh.ogre_mesh_exporter = PointerProperty(
		name = "Ogre Mesh Exporter properties",
		type = MeshProperties,
		description = "Ogre Mesh Exporter properties"
	)

# unregistering and removing menus
def unregister():
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
