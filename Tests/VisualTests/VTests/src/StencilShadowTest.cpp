/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "StencilShadowTest.h"
#include "OgreMovablePlane.h"

StencilShadowTest::StencilShadowTest()
{
    mInfo["Title"] = "VTests_StencilShadows";
    mInfo["Description"] = "Tests basic stencil shadow functionality.";
    
    // take screenshot almost immediately, since the scene is static
    addScreenshotFrame(10);
}
//---------------------------------------------------------------------------

void StencilShadowTest::setupContent()
{
    // turn ambient light off
    mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));

    // turn on stencil shadows
    mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
    
    // add a couple lights
    Ogre::Light* light = mSceneMgr->createLight("Light1");
    light->setDiffuseColour(0.5f,0.4f,0.35f);
    light->setSpecularColour(0, 0, 0);
    light->setAttenuation(8000,1,0.0005,0);
    mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(220, 100, 0))->attachObject(light);
    light->setCastShadows(true);
    light->setType(Light::LT_POINT);
    light = mSceneMgr->createLight("Light2");
    light->setDiffuseColour(0.5f,0.4f,0.35f);
    light->setSpecularColour(0, 0, 0);
    light->setAttenuation(8000,1,0.0005,0);
    mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(220, 100, -200))->attachObject(light);
    light->setCastShadows(true);
    light->setType(Light::LT_POINT);
    
    // create a ground plane to receive some shadows
    Plane pln = MovablePlane("plane");
    pln.normal = Vector3::UNIT_Y;
    pln.d = 107;
    MeshManager::getSingleton().createPlane("ground_plane",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, pln,
        1500,1500,50,50,true,1,5,5,Vector3::UNIT_Z);
    Ogre::Entity* groundPlane = mSceneMgr->createEntity( "plane", "ground_plane" );
    groundPlane->setMaterialName("Examples/Rocky");
    groundPlane->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundPlane);

    // and a couple objects to cast the shadows
    Ogre::Entity* bar = mSceneMgr->createEntity( "barrel", "Barrel.mesh" );
    bar->setCastShadows(true);
    Ogre::SceneNode* barNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    barNode->attachObject(bar);
    barNode->setScale(7,7,7);
    barNode->setPosition(Ogre::Vector3(0,-85,-320));
    Ogre::Entity* head = mSceneMgr->createEntity( "ogrehead", "ogrehead.mesh" );
    head->setCastShadows(true);
    Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    headNode->attachObject(head);
    headNode->setPosition(Ogre::Vector3(-100,-80,-320));
    Ogre::Entity* torus = mSceneMgr->createEntity( "torus", "knot.mesh" );
    torus->setCastShadows(true);
    torus->setMaterialName("Examples/RustySteel");
    Ogre::SceneNode* torusNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    torusNode->setScale(0.5,0.5,0.5);
    torusNode->attachObject(torus);
    torusNode->setPosition(Ogre::Vector3(100,-60,-320));
    
    // point the camera down a bit
    mCameraNode->pitch(Ogre::Degree(-20.f));
}
//-----------------------------------------------------------------------


