
# OGRE COMMAND-LINE UTILITIES

This archive contains a few prebuilt command-line tools for manipulating your media.
For further info, visit http://www.ogre3d.org

## Index
 - [AssimpConverter](#assimpconverter)
 - [BitmapFontBuilderTool](#bitmapfontbuildertool)
 - [gsplat_to_mesh](#gsplat_to_mesh)
 - [LightwaveConverter](#lightwaveconverter)
 - [MayaExport](#mayaexport)
 - [OgreXMLConverter](#ogrexmlconverter)
 - [OgreMeshUpgrader](#ogremeshupgrader)
 - [MilkshapeExport](#milkshapeexport)
 - [VRMLConverter](#vrmlconverter)
 - [Wings3DExporter](#wings3dexporter)
 - [XSIExport](#xsiexport)


## AssimpConverter
Converts 3D-formats supported by assimp to native OGRE formats

```
Usage: OgreAssimpConverter [options] sourcefile [destination]

Available options:
-q                  = Quiet mode, less output
-log filename       = name of the log file (default: 'OgreAssimp.log')
-aniSpeedMod [0..1] = Factor to scale the animation speed (default: 1.0)
-3ds_ani_fix        = Fix for 3ds max, which exports the animation over a
                      longer time frame than the animation actually plays
-max_edge_angle deg = When normals are generated, max angle between
                      two faces to smooth over
sourcefile          = name of file to convert
destination         = optional name of directory to write to. If you don't
                      specify this the converter will use the same
                      directory as the sourcefile.

```

## BitmapFontBuilderTool
Tool designed to take the binary width files from BitmapFontBuilder http://www.lmnopc.com/bitmapfontbuilder/ and convert them into Ogre .fontdef 'glyph' statements.

## gsplat_to_mesh

Converts 3D gaussian splatting .ply files to OGRE .mesh files while reducing the size.
See https://www.rojtberg.net/2801/incorporating-3d-gaussian-splats-into-the-graphics-pipeline/

## LightwaveConverter
Lwo2Mesh v0.89 by Dennis Verbeek ( dverbeek@hotmail.com )
Linux port by Magnus Møller Petersen ( magnus@moaner.dk )

Lwo2Mesh is a commandline tool to convert lightwave objects into ogre-meshes.
Use -? to get help. Use *.lwo to convert multiple objects in one run.

```
Usage: lwo2mesh [options] source [dest]
options:
  -g do not use shared geometry
  -d generate level of detail information
     method (f)ixed or (p)roportional
     reduction (fixed) or reductionfactor (proportional)
     number of LOD levels
     distances
     example: -dp 0.5 4 1000.0 2000.0 4000.0 8000.0
  -l save layers separately
  -m do not export materials
  -r rename materials
     method (i)nteractive, (o)bjectname or (p)refix
     example: -rp prefix_
  -s do not export skeleton
  -i info on .lwo only, no conversion to mesh
  -v dump vertex maps
```

The lightwave-object loading code is based on Lightwave SDK code by Ernie Wright.

This program is distributed as-is and WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## MayaExport
Ogre exporter for Maya

[MayaExport README](MayaExport/Readme.md)

## OgreXMLConverter
Converts between the binary and XML formats for .mesh and .skeleton.
This tool is necessary to convert from the XML to OGRE's native runtime format if your exporter produces XML.
You can find the XML Schema for the .mesh and .skeleton formats in the Ogre source under Tools/XMLConverter/docs.

```
Usage: OgreXMLConverter [options] sourcefile [destfile]

Available options:
-o             = DON'T optimise out redundant tracks & keyframes
-d3d           = Prefer D3D packed colour formats (default on Windows)
-gl            = Prefer GL packed colour formats (default on non-Windows)
-E endian      = Set endian mode 'big' 'little' or 'native' (default)
-q             = Quiet mode, less output
-log filename  = name of the log file (default: 'OgreXMLConverter.log')
sourcefile     = name of file to convert
destfile       = optional name of file to write to. If you don't
                 specify this OGRE works it out through the extension
                 and the XML contents if the source is XML. For example
                 test.mesh becomes test.xml, test.xml becomes test.mesh
                 if the XML document root is <mesh> etc.
```

Because the default behaviour is to convert binary to XML and vice versa, you can simply drag files onto this converter and it will convert between the 2 formats.

## OgreMeshUpgrader
This tool upgrades a .mesh file from any previous version of OGRE to the latest version.
You will be advised in Ogre.log if your meshes are of an old version;
OGRE can still load old versions but performance may not be as good as it would be with the latest version.
You are advised to upgrade your meshes whenever you update to another major version of OGRE.

```
Usage: OgreMeshUpgrader [options] sourcefile [destfile]
-i             = Interactive mode, prompt for options
-l lodlevels   = number of LOD levels
-d loddist     = distance increment to reduce LOD
-p lodpercent  = Percentage triangle reduction amount per LOD
-f lodnumtris  = Fixed vertex reduction per LOD
-e         = DON'T generate edge lists (for stencil shadows)
-t         = Generate tangents (for normal mapping)
-r         = DON'T reorganise buffers to recommended format
-d3d       = Convert to D3D colour formats
-gl        = Convert to GL colour formats
-srcd3d    = Interpret ambiguous colours as D3D style
-srcgl     = Interpret ambiguous colours as GL style
-E endian  = Set endian mode 'big' 'little' or 'native' (default)
sourcefile = name of file to convert
destfile   = optional name of file to write to. If you don't
             specify this OGRE overwrites the existing file.
```

Again you can drag files onto this tool, so long as you don't mind it overwriting the file in place.
If you'd prefer to keep a backup, make a copy or use the command line to upgrade to a different file.

Reorganising vertex buffers:
This tool now allows you to restructure the vertex buffers in your mesh.
If you are upgrading from a version prior to 0.15.0, then you should answer 'y' when asked if you want to reorganise the buffers, since 0.15.0 and later allows more efficient structures in the binary mesh.
You will then be shown the buffer structures for each of the geometry sections; you can either reorganise the buffers yourself, or use 'automatic' mode, which is recommended unless you know what you're doing.

## MilkshapeExport
Allows you to export OGRE .mesh files from the shareware modelling tool Milkshape3d.

[Milkshape Exporter README](https://htmlpreview.github.io/?https://github.com/OGRECave/ogre/blob/master/Tools/MilkshapeExport/ReadMe.html)

## VRMLConverter
VRML - OGRE .mesh converter

```
Usage:
	VRML2mesh <input vrml file> [output mesh file]
	Or just drag-and-drop a VRML file on the progam.

	Well... before doing that, you have to export a VRML file from your
	modeler. 3dsmax, Maya and Milkshape can all do this, and their
	exporters have been confirmed to create trouble-free files.
	Other programs should work as well, but there are no guarantees.

Features:
	Converts VRML97 files to .mesh files containing static meshes and materials.
	That's all.

Notes:
	* Make sure your objects are tesselated. They don't have to be
	  triangulated - the converter takes care of that, but primitive
	  shapes (like spheres, boxes, etc) are not recognised.

	* If you want the mesh to contain normals (which is usually the case),
	  then make sure that normals are exported. There is usually an option
	  for this in the modeler, but the default is 'off' in most programs.
```

### Wings3DExporter
Wings3D to OGRE converter written in Python.

[Wings3D to OGRE converter README](Wings3DExporter/Readme.md)

### XSIExport
This distribution contains the files required to export OGRE .mesh, .skeleton and .material files from SoftImage|XSI.
Currently supported XSI versions are 6.5 up to Softimage 2011

[XSI Exporter README](https://htmlpreview.github.io/?https://github.com/OGRECave/ogre/blob/master/Tools/XSIExport/OGREXSI_Readme.html)
