# DotScene Overview

DotScene (aka .scene) is just a standardized XML file format.

This file format is meant to be used to set up a scene or scene-part. It is useful for any type of application/ game. Editors can export to .scene format, and apps can load the format.

Besides Ogre, the [jMonkeyEngine](http://jmonkeyengine.org/) also supports loading .scene files.

## What is DotScene?
DotScene file does not contain any mesh data, texture data, etc. It just contains elements that describe a scene.

A simple .scene file example:
```xml
<scene formatVersion="">
    <nodes>
        <node name="Robot" id="3">
            <position x="10.0" y="5" z="10.5" />
            <scale x="1" y="1" z="1" />
            <entity name="Robot" meshFile="robot.mesh" static="false" />
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

// set the desired resource group first
ResourceGroupManager::getSingleton().setWorldResourceGroupName("MyGroup");
attachmentNode->loadChildren("myScene.scene");
```

If there is a TerrainGroup defined in the .scene file, you can get it by:
```cpp
attachmentNode->getUserObjectBindings().getUserAny("TerrainGroup");
```
The type is `std::shared_ptr<Ogre::TerrainGroup>`, hence `attachmentNode` owns it and will take it down on destruction.
