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
        if( ExampleFrameListener::frameRenderingQueued(evt) == false )
            return false;
        // Rotate fountains
        mFountainNode->yaw(Degree(evt.timeSinceLastFrame * 30));

        // Call default
        return true;

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



        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");

        // Add entity to the root scene node
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);


        // Green nimbus around Ogre
        //mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(
        //        mSceneMgr->createParticleSystem("Nimbus", "Examples/GreenyNimbus"));

        // Create some nice fireworks

        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(
                mSceneMgr->createParticleSystem("Fireworks", "Examples/Fireworks"));

        // Create shared node for 2 fountains
        mFountainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // fountain 1
        ParticleSystem* pSys2 = mSceneMgr->createParticleSystem("fountain1",
            "Examples/PurpleFountain");
        // Point the fountain at an angle
        SceneNode* fNode = mFountainNode->createChildSceneNode();
        fNode->translate(200,-100,0);
        fNode->rotate(Vector3::UNIT_Z, Degree(20));
        fNode->attachObject(pSys2);

        // fountain 2
        ParticleSystem* pSys3 = mSceneMgr->createParticleSystem("fountain2",
            "Examples/PurpleFountain");
        // Point the fountain at an angle
        fNode = mFountainNode->createChildSceneNode();
        fNode->translate(-200,-100,0);
        fNode->rotate(Vector3::UNIT_Z, Degree(-20));
        fNode->attachObject(pSys3);




        // Create a rainstorm
        ParticleSystem* pSys4 = mSceneMgr->createParticleSystem("rain",
            "Examples/Rain");
        SceneNode* rNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        rNode->translate(0,1000,0);
        rNode->attachObject(pSys4);
        // Fast-forward the rain so it looks more natural
        pSys4->fastForward(5);


        // Aureola around Ogre perpendicular to the ground
        ParticleSystem* pSys5 = mSceneMgr->createParticleSystem("Aureola",
            "Examples/Aureola");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pSys5);

		// Set nonvisible timeout
		ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);
    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new ParticleFrameListener(mWindow, mCamera, mFountainNode);
        mRoot->addFrameListener(mFrameListener);
    }


};

