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
        Transparency.h
    \brief
        Specialisation of OGRE's framework application to show the transparency,
        or scene blending features.
    \par
        Note that this is a little rudimentary - it's because whilst
        OGRE supports lots of blending options, the SceneManager has
        to ensure the rendering order is correct when object transparency
        is enabled. Right now this is not quite right in the default
        manager so this scene is kept deliberately simple.
*/

#include "ExampleApplication.h"

class TransApplication : public ExampleApplication
{
public:
    TransApplication() {}

protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);


        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/BumpyMetal");

        // Create an entity from a model (will be loaded automatically)
        Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.mesh");

        knotEnt->setMaterialName("Examples/TransparentTest");

        // Attach the 2 new entities to the root of the scene
        SceneNode* rootNode = mSceneMgr->getRootSceneNode();
        rootNode->attachObject(planeEnt);
        rootNode->attachObject(knotEnt);

        // Add a whole bunch of extra transparent entities
        Entity *cloneEnt;
        for (int n = 0; n < 10; ++n)
        {
            // Create a new node under the root
            SceneNode* node = mSceneMgr->createSceneNode();
            // Random translate
            Vector3 nodePos;
            nodePos.x = Math::SymmetricRandom() * 500.0;
            nodePos.y = Math::SymmetricRandom() * 500.0;
            nodePos.z = Math::SymmetricRandom() * 500.0;
            node->setPosition(nodePos);
            rootNode->addChild(node);
            // Clone knot
            char cloneName[12];
            sprintf(cloneName, "Knot%d", n);
            cloneEnt = knotEnt->clone(cloneName);
            // Attach to new node
            node->attachObject(cloneEnt);

        }
    }

};
