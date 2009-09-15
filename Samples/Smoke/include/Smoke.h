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
/*
-----------------------------------------------------------------------------
Filename:    ParticleApplication.cpp
Description: Specialisation of OGRE's framework application to show the
             environment mapping feature.
-----------------------------------------------------------------------------
*/


#include "ExampleApplication.h"


// Event handler to add ability to alter curvature
class ParticleFrameListener : public ExampleFrameListener
{
protected:
    SceneNode* mFountainNode;
public:
    ParticleFrameListener(RenderWindow* win, Camera* cam, SceneNode* fountainNode)
        : ExampleFrameListener(win, cam)
    {
        mFountainNode = fountainNode;
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {

        // Rotate fountains
//        mFountainNode->yaw(evt.timeSinceLastFrame * 30);

        // Call default
        return ExampleFrameListener::frameRenderingQueued(evt);

    }
};



class ParticleApplication : public ExampleApplication
{
public:
    ParticleApplication() {}

protected:
    SceneNode* mFountainNode;

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

       
        // Create shared node for 2 fountains
        mFountainNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());

        // smoke
        ParticleSystem* pSys2 = mSceneMgr->createParticleSystem("fountain1", 
            "Examples/Smoke");
        // Point the fountain at an angle
        SceneNode* fNode = static_cast<SceneNode*>(mFountainNode->createChild());
        fNode->attachObject(pSys2);

    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new ParticleFrameListener(mWindow, mCamera, mFountainNode);
        mRoot->addFrameListener(mFrameListener);
    }


};

