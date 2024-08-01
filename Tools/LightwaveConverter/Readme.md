
# LightwaveConverter
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
