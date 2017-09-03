# Basic Tutorial 1 - Full source code {#basictutorial1fullsource}

## Header file

```cpp

This source file is a part of OGRE

#ifndef SRC_TUTORIALAPPLICATION_HPP_
#define SRC_TUTORIALAPPLICATION_HPP_

#include <OGRE/Ogre.h>
#include <OGRE/Bites/OgreApplicationContext.h>
#include <OGRE/Bites/OgreInput.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Bites/OgreApplicationContext.h>

class TutorialApplication
        : public OgreBites::ApplicationContext
        , public OgreBites::InputListener
{
public:
    TutorialApplication();
    virtual ~TutorialApplication();

    void setup();
    bool keyPressed(const OgreBites::KeyboardEvent& evt);
};

#endif /* SRC_TUTORIALAPPLICATION_HPP_ */


```

## Source file

```cpp

This source file is a part of OGRE

#include "TutorialApplication.hpp"

TutorialApplication::TutorialApplication()
    : OgreBites::ApplicationContext("Tutorial Application")
{
    addInputListener(this);
}


TutorialApplication::~TutorialApplication() {}


void TutorialApplication::setup()
{
    // do not forget to call the base first
    OgreBites::ApplicationContext::setup();

    // get a pointer to the already created root
    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager(Ogre::ST_GENERIC);

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    // -- tutorial section start --
    scnMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
    Ogre::Light* light = scnMgr->createLight("MainLight");
    light->setPosition(20, 80, 50);

    // also need to tell where we are
    Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    camNode->setPosition(0, 47, 222);

    // create the camera
    Ogre::Camera* cam = scnMgr->createCamera("myCam");
    cam->setNearClipDistance(5); // specific to this sample
    cam->setAutoAspectRatio(true);
    camNode->attachObject(cam);

    // and tell it to render into the main window
    getRenderWindow()->addViewport(cam);

    Ogre::Entity* ogreEntity = scnMgr->createEntity("ogrehead.mesh");
    Ogre::SceneNode* ogreNode = scnMgr->getRootSceneNode()->createChildSceneNode();
    ogreNode->attachObject(ogreEntity);

    Ogre::Entity* ogreEntity2 = scnMgr->createEntity("ogrehead.mesh");
    Ogre::SceneNode* ogreNode2 = scnMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(84, 48, 0));
    ogreNode2->attachObject(ogreEntity2);

    Ogre::Entity* ogreEntity3 = scnMgr->createEntity("ogrehead.mesh");

    Ogre::SceneNode* ogreNode3 = scnMgr->getRootSceneNode()->createChildSceneNode();
    ogreNode3->setPosition(0, 104, 0);
    ogreNode3->setScale(2, 1.2, 1);
    ogreNode3->attachObject(ogreEntity3);

    Ogre::Entity* ogreEntity4 = scnMgr->createEntity("ogrehead.mesh");

    Ogre::SceneNode* ogreNode4 = scnMgr->getRootSceneNode()->createChildSceneNode();
    ogreNode4->setPosition(-84, 48, 0);
    ogreNode4->roll(Ogre::Degree(-90));
    ogreNode4->attachObject(ogreEntity4);

    // -- tutorial section end --
}


bool TutorialApplication::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}


```

## Main source file

```cpp

#include <exception>
#include <iostream>

#include "TutorialApplication.hpp"


int main(int argc, char **argv)
{
    try
    {
        TutorialApplication app;
        app.initApp();
        app.getRoot()->startRendering();
        app.closeApp();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred during execution: " << e.what() << '\n';
        return 1;
    }

    return 0;
}


```
