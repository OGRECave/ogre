# Volume Component {#volume}

Welcome to the Volume Component of OGRE. It is a component to render volumes. It can handle any volume data but featurewise has a tendency towards terrains.
The terrain aspect means, that it's all about huge meshes being displayed with high performance via a level of detail mechanism. Thanks to volume rendering, caves, cliffs, holes and similar geometry can be displayed. Also constructive solid geometry gets easy.

@tableofcontents

A dense list of the features:
* Volume Rendering via Dual Marching Cubes
* LOD mechanism via a chunk tree and marching squares skirts for crack patching
* Data-Sources: 3D Textures with density values and the ability to buildup a CSG-Tree with 3D Textures, Spheres, Cubes, Planes, Intersection, Union, Difference and Negation, SimplexNoise addition
* Loading a 3D Texture Scene from config files
* An own file format for discrete density values which compresses a lot better
* Realtime editing
* Serialization and Deserialization from and to an own file format for discrete density values which compresses a lot better than 3D textures
* A triplanar texturing example material
* A triplanar texturing SubRenderState for the RTSS

This documentation is only intended for the usage of the Volume Component, not the algorithms which make it work. If you want to get to know the theory behind it, you can read the articles on this page: http://volume-gfx.com/

# How to use it {#howto}

Here is an example loading the volume scene from a configuration file. The configuration file must be findable by the resource system of course. Only "OgreVolumeChunk.h" has to be included.
```cpp
Ogre::Volume::Chunk *volumeRoot = OGRE_NEW Ogre::Volume::Chunk();
SceneNode *volumeRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VolumeParent");
volumeRoot->load(volumeRootNode, mSceneMgr, "volumeTerrain.cfg");
```
The first line creates the volume chunk which is the MovableObject holding the Volume-Root. Next, a SceneNode is created where the volume(-tree) is attached to. And lastly, the volume is loaded from the configuration file "volumeTerrain.cfg".
mSceneMgr is the SceneManager who should show the volume. Later, when you don't need it anymore, you have to free the volumeRoot Chunk via OGRE_DELETE.

# Manual creation of a CSG-Tree {#creation}

This example skips the configuration file and loads a simple CSG-Scene: An union of a 3D Texture and a sphere with 5 LOD levels. It also setups a material LOD system.
First, create a sphere with the radius 5 at the coordinates 128, 150, 128:
```cpp
Ogre::Volume::CSGSphereSource sphere (5, Vector3(128, 150, 128));
```
Now a 3D texture from "volumeTerrainBig.dds" which has the world dimensions 256, 256, 256. The next flag indicates that the trilinear interpolation of the value-selection should be activated. We switch off the trilinear interpolation and the sobel filter of the normal for the sake of faster loading times:
```cpp
Ogre::Volume::TextureSource volumeTexture ("volumeTerrainBig.dds", 256, 256, 256, true, false, false);
```
Now combine them:
```cpp
Ogre::Volume::CSGUnionSource unionSrc (&sphere, &volumeTexture);
```
Set the general parameters, see the comments for their meaning:
```cpp
Ogre::Volume::ChunkParameters parameters;
parameters.sceneManager = mSceneMgr;       // The SceneManager to use
parameters.src = &unionSrc;                // The just created density source.
parameters.baseError = 1.8;                // The error of the highest LOD-level
parameters.errorMultiplicator = 0.9;       // The factor between each LOD-level (error = baseError * errorMultiplicator  * level)
parameters.skirtFactor = 0.7;              // Controls how long the skirts are. The lower the number, the shorter the skirts are. This saves geometry. But if they are too short, cracks might occur.
parameters.scale = 10;                     // The displayed volume will be scaled by this factor.
parameters.maxScreenSpaceError = 30;       // The screen space error controlling when the LOD-levels change.
```

Create the root-chunk and load now. The two vectors define the area to be scanned in the volume-source. The following integer determines the amount of LOD-levels.
```cpp
Ogre::Volume::Chunk *volumeRoot = OGRE_NEW Chunk();
volumeRoot->load(mVolumeRootNode, Vector3::ZERO, Vector3(256), 5, &parameters);
```
Now setup the global volume material:
```cpp
volumeRoot->setMaterial("triplanarReference");
```
# Getting the triangles of the chunks {#triangles}

When you want to integrate the volume triangles in a physics engine for example, you need to hand over the exact triangles. For this scenario, the ChunkParameters hold a pointer to a ''MeshBuilderCallback'' and a number ''lodCallbackLod''. The first is an interface with a single function ''trianglesReady'' which is called, when the triangles of a chunk are ready. The chunks are chosen via the ''lodCallbackLod''. If you build a volume with 3 LOD levels and you want the triangles of the highest detail level, you set this parameter to 3. Example:

```cpp
class MyMeshBuilderCallback : public MeshBuilderCallback
{
public:
    virtual  ready(const SimpleRenderable *simpleRenderable, const VecVertex &vertices, const VecIndices &indices, size_t level, int inProcess)
    {
        size_t count = indices.size();
        for (size_t i = 0; i < count; i += 3)
        {
            Vertex v1 = vertices[indices[i]];
            Vertex v2 = vertices[indices[i + 1]];
            Vertex v3 = vertices[indices[i + 2]];
            // Do something with the Triangle...
        }
    }
};
```

And using it like this:
```cpp
ChunkParameters parameters;
...
MyMeshBuilderCallback callback;
parameters.lodCallback = &callback;
parameters.lodCallbackLod = 3;
...
rootChunk->load(parent, from, to, 3, &parameters);
```
Or in case of loading from a configuration file:
```cpp
MyMeshBuilderCallback callback;
rootChunk->load(parent, sceneManager, "myVolume.cfg", 0, &callback, 3);
```

You might have seen, that a pointer to a SimpleRenderable is handed in, too. This is actually the Chunk whose triangles are loaded. It's parent class is used to not create a circular dependency. You might get the triangle data from it, too by getting the Renderoperation.

# Intersecting a ray with a volume {#intersecting}

When you want to do something exactly on the volume surface, you can cast rays and find their first intersection point. Behold that the triangle representation might be slightly different than the actual volume surface. And you have to scale the ray origin just like the volume. Here is an example which uses the negative z-axis of the camera as ray as it might be done like in an ego-shooter:
```cpp
Ray ray(mCamera->getPosition() / rootChunk->getScale(), -mCamera->getOrientation().zAxis());
Vector3 intersection;
Real scale = mVolumeRoot->getChunkParameters()->scale;
bool intersects = mVolumeRoot->getChunkParameters()->src->getFirstRayIntersection(ray, intersection, scale);
if (intersects)
{
    intersection *= scale; // As it is in volume space.
    // Do something with intersection.
}
```

# Editing a Volume made from a GridSource {#editing}

A usecase is realtime editing of volume terrain as seen as in the sample. Let's union the terrain with a sphere of the radius 2.5 and the center 123/123/123. __volumeRoot__ is the Chunk instance with which the terrain was initially loaded. The factor 1.5 is just to have a save border around the sphere which also gets updated. The rest of the parameters are 5 LOD levels and a volume mesh covering an area of 384^3.
```cpp
Vector3 center(123);
Real radius = (Real)2.5;
CSGSphereSource sphere(radius, center);
CSGUnionSource operation;
static_cast<GridSource*>(volumeRoot->getChunkParameters()->src)->combineWithSource(&operation, &sphere, center, radius * (Real)1.5);
volumeRoot->getChunkParameters()->updateFrom = center - radius * (Real)1.5;
volumeRoot->getChunkParameters()->updateTo = center + radius * (Real)1.5;
volumeRoot->load(volumeRootNode, Vector3::ZERO, Vector3(384), 5, volumeRoot->getChunkParameters());
```
