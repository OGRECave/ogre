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
        EnvMapping.cpp
    \brief
        Specialisation of OGRE's framework application to show the
        environment mapping feature.
*/

#include "ExampleApplication.h"

class EnvMapApplication : public ExampleApplication
{
public:
    EnvMapApplication() {}

protected:

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


        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");


        // Set material loaded from Example.material
        ent->setMaterialName("Examples/EnvMappedRustySteel");

        // Add entity to the root scene node
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);


        

    }

};
