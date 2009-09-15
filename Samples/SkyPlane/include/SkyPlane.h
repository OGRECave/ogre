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
        SkyPlane.h
    \brief
        Specialisation of OGRE's framework application to show the
        skyplane feature where a fixed constant-distance
        skyplane is displayed in the background.
*/

#include "ExampleApplication.h"

class SkyPlaneApplication : public ExampleApplication
{
public:
    SkyPlaneApplication() {}

protected:
    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Define the required skyplane
        Plane plane;
        // 5000 world units from the camera
        plane.d = 5000;
        // Above the camera, facing down
        plane.normal = -Vector3::UNIT_Y;
        // Create the plane 10000 units wide, tile the texture 3 times
        mSceneMgr->setSkyPlane(true, plane, "Examples/SpaceSkyPlane",10000,3);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        // Also add a nice dragon in
        Entity *ent = mSceneMgr->createEntity("dragon", "dragon.mesh");
        mSceneMgr->getRootSceneNode()->attachObject(ent);



    }

};
