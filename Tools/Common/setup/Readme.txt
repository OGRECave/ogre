OGRE COMMAND-LINE UTILITIES
===========================

This archive contains a few prebuilt command-line tools for manipulating your
media. For further info, visit http://www.ogre3d.org

OgreXMLConverter
----------------
Converts between the binary and XML formats for .mesh and .skeleton. Will also 
allow you to generate LOD information if you are converting to the binary 
format. This tool is necessary to convert from the XML to OGRE's native runtime
format if your exporter produces XML. You can find the XML Schema for the .mesh
and .skeleton formats in the Ogre source under Tools/XMLConverter/docs.

Usage: OgreXMLConverter [options] sourcefile [destfile]

Available options:
-i             = interactive mode - prompt for options
(The next 4 options are only applicable when converting XML to Mesh)
-l lodlevels   = number of LOD levels
-d loddist     = distance increment to reduce LOD
-p lodpercent  = Percentage triangle reduction amount per LOD
-f lodnumtris  = Fixed vertex reduction per LOD
-e             = DON'T generate edge lists (for stencil shadows)
-r             = DON'T reorganise vertex buffers to OGRE recommended format.
-t             = Generate tangents (for normal mapping)
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

Because the default behaviour is to convert binary to XML and vice versa, you 
can simply drag files onto this converter and it will convert between the 2 
formats, although you will not be able to use it to generate LOD levels this
way.

OgreMeshUpgrade
---------------

This tool upgrades a .mesh file from any previous version of OGRE to the latest
version. You will be advised in Ogre.log if your meshes are of an old version;
OGRE can still load old versions but performance may not be as good as it would
be with the latest version. You are advised to upgrade your meshes whenever you
update to another major version of OGRE.

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

Again you can drag files onto this tool, so long as you don't mind it 
overwriting the file in place. If you'd prefer to keep a backup, make a copy or
use the command line to upgrade to a different file.

Reorganising vertex buffers: this tool now allows you to restructure the vertex
buffers in your mesh. If you are upgrading from a version prior to 0.15.0, then 
you should answer 'y' when asked if you want to reorganise the buffers, since
0.15.0 and later allows more efficient structures in the binary mesh. You will 
then be shown the buffer structures for each of the geometry sections; you can
either reorganise the buffers yourself, or use 'automatic' mode, which is
recommended unless you know what you're doing.

OgreMaterialUpgrade
-------------------
Upgrades a .material script from any previous version of OGRE to the new 
.material format. Note that upgraded scripts do not use any new features of the
material, and you may find that some attributes are re-written as their 
'complex' variants rather than their simplified versions (e.g. "scene_blend add"
will be written as "scene_blend one one" because this is what 'add' maps down 
to.

You only need to run this tool if you have .material scripts from a version of
OGRE older than 0.13.0. Material scripts written for 0.13.0 onwards do not 
need upgrading.

Usage:

OgreMaterialUpgrade sourcefile [destfile]
sourcefile = name of file to convert
destfile   = optional name of file to write to. If you don't
             specify this OGRE overwrites the existing file.

Again you can drag files onto this tool, so long as you don't mind it 
overwriting the file in place. If you'd prefer to keep a backup, make a copy or
use the command line to upgrade to a different file.

MeshMagick
----------
Copyright 2007 by Daniel Wickert

Command-line tool for performing operations on meshes and skeletons.

Usage: 

MeshMagick [global_options] toolname [tool_options] infile(s) -- [outfile(s)]

Available Tools:

info - print information about the mesh.
meshmerge - Merge multiple submeshes into a single mesh.
optimise - Optimise meshes and skeletons.
rename - Rename different elements of meshes and skeletons.
transform - Scale, rotate or otherwise transform a mesh.

See MeshMagick -help=toolname for more detailed information.



Copyright 2004-7 The OGRE Team
