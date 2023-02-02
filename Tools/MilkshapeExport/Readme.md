
# Milkshape Exporter README file

## How to Install

If you are using the prebuilt archive, just extract it into the folder you installed Milkshape3D into (e.g. `C:\Program Files\Milkshape3d`).
If you are building the exporter yourself, copy the msOGREExporter.dll into the Milkshape folder, and either copy OgreMain.dll from the 'Release' build of OgreMain into the Milkshape folder too, or make sure it is somewhere on your path.

## How to Use
Select an object and click File|Export|OGRE Mesh / Skeleton...

It's possible to export Material definitions from Milkshape into the .mesh file, but because Milkshape only supports very basic material types, I recommend you don't do this.
Instead, create materials in Milkshape and use them for skinning purposes, but don't check the 'Export Materials' box in the OGRE export dialog.
Make sure you note down the names of the materials you defined in Milkshape, then write definitions for the materials in OGRE .material scripts instead, using the same names.
The meshes exported will still have a reference to the material name defined in Milkshape, but the superior OGRE material will be used at runtime.

Skeletons can be exported too: if you choose not to export a skeleton and there are bones in the model, you must link the mesh to it's target skeleton.

You can optionally choose to split the single animation sequence in Milkshape into multiple separate animations.
To do this you have to supply a simple comma-separated text file to record which frames are for which animation, e.g:
1,30,Walk
31,40,Wave
41,80,Run

These 3 animations would then show up under `Skeleton::getAnimation("Walk")`, `Skeleton::getAnimation("Run")` etc as well as `Entity::getAnimationState("Walk")` etc.
