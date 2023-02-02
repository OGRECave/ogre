
# VRML - OGRE .mesh converter

## Usage
```
VRML2mesh <input vrml file> [output mesh file]
```
Or just drag-and-drop a VRML file on the progam.

Well... before doing that, you have to export a VRML file from your modeler.
3dsmax, Maya and Milkshape can all do this, and their exporters have been confirmed to create trouble-free files.
Other programs should work as well, but there are no guarantees.

## Features
Converts VRML97 files to .mesh files containing static meshes and materials.
That's all.

## Notes
 - Make sure your objects are tesselated.
   They don't have to be triangulated - the converter takes care of that, but primitive shapes (like spheres, boxes, etc) are not recognised.

 - If you want the mesh to contain normals (which is usually the case), then make sure that normals are exported.
   There is usually an option for this in the modeler, but the default is 'off' in most programs.
