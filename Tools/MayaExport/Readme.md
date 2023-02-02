
# Ogre exporter for Maya
```
/*******************************************************************************
 *                        Ogre exporter for Maya                               *
 *                                                                             *
 * Author: Francesco Giordana ( fra.giordana@tiscali.it )                      *
 *                                                                             *
 * Sponsored by: Anygma N.V. ( http://www.nazooka.com )                        *
 *                                                                             *
 *******************************************************************************
```

## Installation
1) Copy "ogreExporter.mll" to your Maya plug-ins directory (e.g.: `C:\Program Files\Alias\Maya7.0\bin\plug-ins`)

2) Copy Ogre DLLs (you can find them in the "dlls" directory of this zip) to your Maya bin directory
   (e.g.: `C:\Program Files\Alias\Maya7.0\bin`)

3) Copy "ogreExporter.mel" (from the "mel" directory of this zip) to your Maya scripts directory 
   (e.g.: `C:\Documents and Settings\user\My Documents\maya\6.5\scripts`)

4) If you already have a userSetup.mel in your scripts folder, then append the line 
      source ogreExporter.mel;
   to the existing "userSetup.mel".
   If you don't have a "userSetup.mel" file in your scripts directory, then copy there the one you find in
   the "scripts" directory of this zip.

5) Launch `vcredist_x86.exe` to install the latest Visual Studio DLLs

## Usage

1) Via GUI from the menu Ogre->Export

2) Via script:
```
ogreExport 	generalOptions 

		["-mesh" meshFilename meshOptions]
			export mesh to .mesh binary file

		["-mat" matFilename matOptions]
			export materials to .material script file

		["-skel" skelFilename]
			export skeleton to .skeleton binary file
	
		["-skeletonAnims" skelAnimsOptions ["-skeletonClip" clipName clipOptions] ["-clip" ...] [...] ]
			export skeleton animations to the .skeleton file 
			[requires -skel]

		["-vertexAnims" vertexAnimOptions ["-vertexClip" clipName clipOptions] ["-clip" ...] [...] ]
			export vertex animations as morph animations to the .mesh file 
			[requires -mesh]

		["-blendShapes"] bsOptions
			export blend shapes as mesh poses to the .mesh file 
			[requires -mesh]

		["-BSAnims" ["-BSClip" clipName clipOptions] ["-clip" ...] [...] ]
			export blend shape animations as pose animations to the .mesh file 
			[requires -mesh]

		["-particles" particlesFilename]
			export particles to .particle file

generalOptions:
	"-sel" | "-all"		export whole scene or only selected objects
	"-world" | "-obj"	export in world or object coordinates
	"-lu " "pref | mm | cm | m | in | ft | yd"	select length unit for export
							("-lu pref" means to get unit from scene
								preferences)
	"-scale" s		scale the whole mesh by s
	
meshOptions:
	["-shared"]				export using shared geometry
	["-v"]					export vertex bone assignements
	["-n"]					export vertex normals
	["-c"]					export vertex colours
	["-t"]					export texture coordinates
	["-edges"]				generate mesh edge list
	["-tangents" "TEXCOORD | TANGENT"]	generate tangents
	["-tangentsplitmirrored"]	split tangents mirrored
	["-tangentsplitrotated"]	split tangents rotated
	["-tangentuseparity"]		use parity for tangents

matOptions:
	["-matPrefix" prefix]	add prefix to all exported materials names [optional]
	["-copyTex" outDir]	copy textures used in the exported materials to outDir [optional]
	["-lightOff"]		export materials with lighting off [optional]

skelAnimsOptions:
	"-skelBB"		include skeleton animations in bounding box calculation
	"-np" ( "curFrame" | "bindPose" | "frame" n )	specify neutral pose, can be current frame or bind pose or specified frame

bsOptions:
	["-bsBB"]		include blend shapes in bounding box calculation

vertexAnimOptions:
	["-vertBB"]		include vertex animations in bounding box calculation

clipOptions:
	"startEnd" s e ("frames" | "seconds") | "timeSlider"	specify clip range with start/end time or use time slider range
```

# Troubleshooting
- Some users reported that they need to add an additional "-v" parameter after "-q" into the file "ogreExporter.mel",
  in the lines 1924 to 1926 (details: [Maya Ogre Exporter Save Settings problem - Download the fix](http://www.ogre3d.org/forums/viewtopic.php?f=8&t=46563))
