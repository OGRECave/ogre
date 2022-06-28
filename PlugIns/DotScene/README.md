# DotScene Overview

DotScene (aka .scene) is just a standardized XML file format.

This file format is meant to be used to set up a scene or scene-part. It is useful for any type of application/ game. Editors can export to .scene format, and apps can load the format.

Besides Ogre, the [jMonkeyEngine](http://jmonkeyengine.org/) also supports loading .scene files.

## Index
 - [What is DotScene?](#what-is-dotscene?)
 - [User Data](#user-data)
 - [How to use DotScene](#how-to-use-dotscene)
 - [Instancing](#instancing)
	- [Static Geometry](#static-geometry)
	- [Instancie Manager](#instancie-manager)

## What is DotScene?
DotScene file does not contain any mesh data, texture data, etc. It just contains elements that describe a scene.

A simple .scene file example:
```xml
<scene formatVersion="1.1">
    <nodes>
        <node name="Robot" id="3">
            <position x="10.0" y="5" z="10.5" />
            <scale x="1" y="1" z="1" />
            <entity name="Robot" meshFile="robot.mesh"/>
        </node>
        <node name="Omni01" id="5">
            <position x="-23" y="49" z="18" />
            <rotation qx="0" qy="0" qz="0" qw="1" />
            <scale x="1" y="1" z="1" />
            <light name="Omni01" type="point">
                <colourDiffuse r="0.4" g="0.4" b="0.5" />
                <colourSpecular r="0.5" g="0.5" b="0.5" />
            </light>
        </node>
    </nodes>
</scene>
```

## User Data

To add logic properties to the scene you can use the `<userData>` node as following:

```xml
<entity meshFile="Cube.mesh" name="Cube" >
    <userData>
        <property data="1.0" name="mass" type="float" />
        <property data="1.0" name="mass_radius" type="float" />
    </userData>
</entity>
```

On the C++ side, these are acessible via e.g.
```cpp
mSceneMgr->getEntity("Cube")->getUserObjectBindings().getUserAny("mass");
```

## How to use DotScene
To use DotScene it has to be loaded as another OGRE Plugin.

In `plugins.cfg`, add the following line:
```ini
Plugin=Plugin_DotScene
```

The Plugin will be then automatically used when you call `SceneNode::loadChildren()` like
```cpp
SceneNode* attachmentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

// Set the desired resource group first and then load the Scene
ResourceGroupManager::getSingleton().setWorldResourceGroupName("MyGroup");
attachmentNode->loadChildren("myScene.scene");
```

If there is a TerrainGroup defined in the .scene file, you can get it by:
```cpp
attachmentNode->getUserObjectBindings().getUserAny("TerrainGroup");
```
The type is `std::shared_ptr<Ogre::TerrainGroup>`, hence `attachmentNode` owns it and will take it down on destruction.

## Instancing
The DotScene Plugin can process the static / instanced attributes of the Entity definition to add them to the scene as either [Static Geometry](https://ogrecave.github.io/ogre/api/latest/class_ogre_1_1_static_geometry.html) or [Instanced meshes](https://ogrecave.github.io/ogre/api/latest/class_ogre_1_1_instance_manager.html).

### Static Geometry
If your scene has entities with the *static* property then the name referenced by the property is interpreted by the plugin as the name of the Static Geometry Group
For example:
```xml
<entity meshFile="Cube.mesh" name="Cube" static="Foliage" />
```

The plugin requires that the Static Geometry group or instance is created before loading the Scene and built afterwards.
Continuing with the example above, supose you created a scene with entities belonging to the "Foliage" group.

```cpp
SceneNode* attachmentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

// Create the StaticGeometry
Ogre::StaticGeometry* sg = mSceneMgr->createStaticGeometry("Foliage");

// Set the desired resource group first and then load the Scene
ResourceGroupManager::getSingleton().setWorldResourceGroupName("MyGroup");
attachmentNode->loadChildren("myScene.scene");

// Build the StaticGeometry after loading the Scene
sg->build();
```

Any configuration for the StaticGeometry should be done before the `build()` call.

Please consult the documentation to know more about StaticGeometry and its use:
 - [Ogre::StaticGeometry Class Reference](https://ogrecave.github.io/ogre/api/latest/class_ogre_1_1_static_geometry.html)
 - [Tutorial - Static Geometry](https://ogrecave.github.io/ogre/api/latest/tut__static_geom.html)


### Instance Manager
If your scene has entities with the *instanced* property then the name referenced by the property is interpreted by the plugin as the name of the Instance Manager
For example:
```xml
<entity meshFile="Cube.mesh" name="Cube" instanced="Foliage" />
```

The plugin requires that the Instance Manager is created before loading the Scene.
Continuing with the example above, supose you created a scene with entities that you want to create with the "Foliage" Instance Manager.

> **NOTE:** Be aware that only the first submesh of the mesh is taken into account, if you have an Entity with many submeshes only the first one will be shown.

```cpp
SceneNode* attachmentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

// Create the InstanceManager
Ogre::InstanceManager* im = mSceneMgr->createInstanceManager("Foliage", "Cube.mesh", "MyGroup", Ogre::InstanceManager::ShaderBased, 80, Ogre::IM_USEALL);

// Set the desired resource group first and then load the Scene
ResourceGroupManager::getSingleton().setWorldResourceGroupName("MyGroup");
attachmentNode->loadChildren("myScene.scene");
```

Any configuration for the InstanceManager should be done before calling `loadChildren()`

Please consult the documentation to know more about Instancing and its use:
 - [Ogre::InstanceManager Class Reference](https://ogrecave.github.io/ogre/api/latest/class_ogre_1_1_instance_manager.html)
 - [What is instancing?](https://ogrecave.github.io/ogre/api/latest/instancing.html)
