# Automatic Mesh LOD Generator {#meshlod-generator}

@tableofcontents

Mesh LOD allows to swap the models to Low-poly models in far distance, which makes your game faster.
%Ogre will automatically use Mesh LOD when you load a mesh file, which has LOD information in it. So basically you don't need to do any coding to use this feature, but you need to prepare your meshes!

The Mesh LOD generator can generate Low-poly models automatically from High-poly models.
It implements an improved version of "A simple, fast, and effective polygon reduction algorithm" by Stan Melax @cite melax1998simple. The improvements are described in detail [in the GSoC2012 report](http://sajty.elementfx.com/progressivemesh/GSoC2012.pdf).

# How to use it for non-programmers {#meshlod-nonprog}
The __fastest way__ to generate mesh LOD is to use OgreMeshUpgrader on the console:
* Generate autoconfigured LOD: no questions asked, just get instant performance.
  ```
  OgreMeshUpgrader -autogen athene.mesh athene_lod.mesh
  ```
* Interactive mode: It will ask you for every LOD level detail and much more!
  ```
  OgreMeshUpgrader -i athene.mesh athene_lod.mesh
  ```
* If you don't use stencil shadows, I would recommend disabling Edge lists, because they increase the mesh file a lot! If you load the mesh, edge lists will also stay in memory and consuming resources.
* If you use older %Ogre version, but you want to use latest LOD generator algorithm for best quality, you can get it with -V option!
  ```
  OgreMeshUpgrader -autogen -V 1.8 athene.mesh athene_lod.mesh
  ```

The __best way__ to generate mesh LOD with visual feedback is to use the Mesh Lod sample in the Sample Browser.
* Put your meshes and materials into `Samples/Media/models`
* Start up SampleBrowser and load MeshLod sample.
* Select your mesh in the "model" combobox.
* Change the "Reduced vertices" slider to the reduction where you want to create a LOD level.
* Change the camera distance to the desired distance
* Click on Add level to add the reduction amount with given distance
* Repeat previous 3 steps to create all levels.
* You can check the final mesh by clicking on "Show all levels". If you made the LOD perfectly, you see the triangle count dropping, when zooming out, while not seeing graphical errors.
* If you are ready, click on "save mesh" to save the LOD into the mesh file.

# How to use it for programmers {#meshlod-prog}
You can find lot of usage samples in Ogre. For example MeshLod sample, MeshLodTests, MeshSerializerTests, PlayPen.

```cpp
#include <OgreMeshLodGenerator.h>

...
new Ogre::MeshLodGenerator();
```

@note You can create it on the stack if you don’t enable lodConfig.advanced.useBackgroundQueue. Otherwise, when you free up the stack it will abort the LOD generation and you will not get any LOD for your mesh!

As a first step, I would recommend to read through OgreLodConfig.h to know all available options!
LodConfig inheritance (having a base/default LodConfig) can be done with copy constructor!
```cpp
Ogre::LodConfig config(mesh);
config.createGeneratedLodLevel(5, 0.5); // At 5 ogre worldspace units use 50% reduced mesh
config.createGeneratedLodLevel(10, 0.75); // By default, it is using DistanceLodStrategy and proportional reduction.
config.createManualLodLevel(15, "mesh_LOD3.mesh"); // Manual level created in blender, maya or 3ds max.
config.advanced.useBackgroundQueue = true; // Generate in background thread, later inject with framelistener.
Ogre::MeshLodGenerator::getSingleton().generateLodLevels(config);

// Or you can use autoconfigured LOD:
Ogre::MeshLodGenerator::getSingleton().generateAutoconfiguredLodLevels(mesh);
```

```cpp
delete Ogre::MeshLodGenerator::getSingletonPtr();
```

# Extending MeshLodGenerator {#meshlod-extend}
MeshLodGenerator is built up of 6 components:
- Ogre::LodData: The core of the algorithm, containing the Mesh network representation.
- Ogre::LodCollapseCost: Provides an interface for collapse cost calculation algorithms (like curvature, quadric error, profiling, outside weight, normals)
- Ogre::LodInputProvider: Initializes LodData based on a source (It can be mesh or buffered(for threading)).
- Ogre::LodOutputProvider: Bakes the Lod to a target (It can be mesh, buffered, mesh+compressed and buffered+compressed)
- Ogre::LodCollapser: This will reduce the smallest costing vertices to the requested amount of vertices. Creates/destroys faces.
- Ogre::MeshLodGenerator: Frontend, which can receive LodConfig. It will create/run the required internal components to generate the requested LodConfig. Only this class is using the LodConfig.

## Replacing a component
You can replace or inherit from any component by passing it to Ogre::MeshLodGenerator::generateLodLevels() function as parameter.

For example there is a hidden feature (which can't be enabled in LodConfig) to use Quadric Error Metrics instead of Curvature to calculate collapse cost:
```cpp
Ogre::MeshLodGenerator gen;
gen.generateLodLevels(config, std::make_shared<LodCollapseCostQuadric>());
```
In the above example MeshLodGenerator will use every option from LodConfig, which is not related to collapse cost. This means outsideWeight, outsideWalkAngle and profile options will be ignored.

Internally MeshLodGenerator uses 2 steps:
* _resolveComponents(): Creates the components which are still nullptr and configures them based on LodConfig.
* _process(): Runs the components. This may be called on background thread depending on LodConfig.
