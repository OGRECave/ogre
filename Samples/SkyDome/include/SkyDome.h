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
        SkyDome.h
    \brief
        Specialisation of OGRE's framework application to show the
        skydome feature
*/


#include "ExampleApplication.h"

// Event handler to add ability to alter curvature
class SkyDomeFrameListener : public ExampleFrameListener
{
protected:
    Real mCurvature;
    Real mTiling;
    SceneManager* mSceneMgr;
public:
    SkyDomeFrameListener(RenderWindow* win, Camera* cam, SceneManager* mgr)
        : ExampleFrameListener(win, cam)
    {
        mSceneMgr = mgr;
        mCurvature = 1;
        mTiling = 15;

    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        // Change curvature / tiling
        // Delay timer to stop too quick updates of curvature
        static Real timeDelay = 0;

        bool updateSky;
        updateSky = false;
        
        if(!ExampleFrameListener::frameRenderingQueued(evt))
            return false;
        
        if (mKeyboard->isKeyDown(OIS::KC_H) && timeDelay <= 0)
        {
            mCurvature += 1;
            timeDelay = 0.1;
            updateSky = true;
        }
        if (mKeyboard->isKeyDown(OIS::KC_G) && timeDelay <= 0)
        {
            mCurvature -= 1;
            timeDelay = 0.1;
            updateSky = true;
        }

        if (mKeyboard->isKeyDown(OIS::KC_U) && timeDelay <= 0)
        {
            mTiling += 1;
            timeDelay = 0.1;
            updateSky = true;
        }
        if (mKeyboard->isKeyDown(OIS::KC_Y) && timeDelay <= 0)
        {
            mTiling -= 1;
            timeDelay = 0.1;
            updateSky = true;
        }

        if (timeDelay > 0)
            timeDelay -= evt.timeSinceLastFrame;

        if (updateSky)
        {
            mSceneMgr->setSkyDome(true, "Examples/CloudySky", mCurvature, mTiling);
        }

        return true;

    }
};

class SkyDomeApplication : public ExampleApplication
{
public:
    SkyDomeApplication()
    {
    }

protected:
    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        Entity *ent;

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
            p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Examples/RustySteel");

        mSceneMgr->getRootSceneNode()->attachObject(ent);

        ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

    }
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new SkyDomeFrameListener(mWindow, mCamera, mSceneMgr);
        mRoot->addFrameListener(mFrameListener);
    }


    bool setup()
    {
        ExampleApplication::setup();
        LogManager::getSingleton().setLogDetail( LL_BOREME );
        return true;
    }
};
