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

# ##############################################
# Material Properties on the material objects
class MaterialProperties(bpy.types.PropertyGroup):
	materialExportMode_override = BoolProperty(
		name = "Material Export Mode Override",
		description = "Override global setting.",
		default = True
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
	template_name = StringProperty(
		name = "Template Name",
		description = "Name of material template."
	)
