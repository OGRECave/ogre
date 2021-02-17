
# DotScene Overview

DotScene (aka .scene) is just a standardized XML file format.

This file format is meant to be used to set up a scene in [Ogre](http://www.ogre3d.org/). It is useful for any type of application/ game. Editors can export to .scene format, and apps can load the format.

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

## How to use DotScene
In recent OGRE3D versions DotScene has been incorporated into the main OGRE repo and made into a Plugin.
So it has to be loaded in `plugins.cfg`, add the following line:
```
Plugin=Plugin_DotScene
```
Also, it is required to complie against the library (in a similar fashion to OgreBites) located in: `OgreSDK\ogre-1.12.11\lib\OGRE`

Include the header `#include <OgreDotSceneLoader.h>
`, located in: `OgreSDK\ogre-1.12.11\include\OGRE\Plugins\DotScene`

And to use the library , create a DataStream and pass it to the loader:
```
Ogre::String groupName = "Scene";
Ogre::String filename = "myScene.scene";
Ogre::SceneNode attachmentNode = mSceneMgr->getRootSceneNode();

Ogre::DataStreamPtr stream(Ogre::Root::openFileStream(filename, groupName));

Ogre::DotSceneLoader *loader = new Ogre::DotSceneLoader();
loader->load(stream, groupName, attachmentNode );
```

## How to use DotScene (DEPRECATED)
This method is deprecated, but it is simpler and might be useful for someone who is still using older versions of OGRE3D.

Load the plugin (in `plugins.cfg`, add the following line):
```
Plugin=Plugin_DotScene
```
Include the header `#include <OgreSceneLoaderManager.h>
`

And to use the library , just call the loader from the Singleton:
```
Ogre::String filename = "myScene.scene";
Ogre::String groupName = "Scene";
Ogre::SceneNode attachmentNode = mSceneMgr->getRootSceneNode();

Ogre::SceneLoaderManager::getSingletonPtr()->load(filename,  groupName, attachmentNode );
```
