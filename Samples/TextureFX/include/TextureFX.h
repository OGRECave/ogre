/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        TextureFX.h
    \brief
        Shows OGRE's ability to handle different types of texture effects.
*/


#include "ExampleApplication.h"

class TextureEffectsApplication : public ExampleApplication
{
public:
    TextureEffectsApplication() {}

protected:

    void createScalingPlane()
    {
        // Set up a material for the plane

        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/TextureEffect1");

        SceneNode* node = 
            mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-250,-40,-100));

        node->attachObject(planeEnt);
    }

    void createScrollingKnot()
    {
        Entity *ent = mSceneMgr->createEntity("knot", "knot.mesh");


        ent->setMaterialName("Examples/TextureEffect2");
        // Add entity to the root scene node
        SceneNode* node = 
            mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(200,50,150));

        node->attachObject(ent);

    }

    void createWateryPlane()
    {
        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity("WaterPlane", SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/TextureEffect3");

        mSceneMgr->getRootSceneNode()->attachObject(planeEnt);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);


        createScalingPlane();
        createScrollingKnot();
        createWateryPlane();


        // Set up a material for the skydome
        MaterialPtr skyMat = MaterialManager::getSingleton().create("SkyMat",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        // Perform no dynamic lighting on the sky
        skyMat->setLightingEnabled(false);
        // Use a cloudy sky
        TextureUnitState* t = skyMat->getTechnique(0)->getPass(0)->createTextureUnitState("clouds.jpg");
        // Scroll the clouds
        t->setScrollAnimation(0.15, 0);

        // System will automatically set no depth write

        // Create a skydome
        mSceneMgr->setSkyDome(true, "SkyMat", -5, 2);





    }

};
