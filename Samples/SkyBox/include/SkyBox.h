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
        SkyBox.h
    \brief
        Specialisation of OGRE's framework application to show the
        skybox feature where a wrap-around environment is projected
        onto a cube around the camera.
*/

#include "ExampleApplication.h"

ParticleSystem *pThrusters;

class SkyBoxFrameListener : public ExampleFrameListener
{
private:
    static float fDefDim;
    static float fDefVel;

public:
    SkyBoxFrameListener(RenderWindow* win, Camera* cam) : ExampleFrameListener( win, cam )
    {
    }

    bool frameRenderingQueued( const FrameEvent& evt )
    {
        if( ExampleFrameListener::frameRenderingQueued( evt ) == false )
		return false;

        if( mKeyboard->isKeyDown( OIS::KC_N ) )
        {
            pThrusters->setDefaultDimensions( fDefDim + 0.25, fDefDim + 0.25 );
            fDefDim += 0.25;
        }

        if( mKeyboard->isKeyDown( OIS::KC_M ) )
        {
            pThrusters->setDefaultDimensions( fDefDim - 0.25, fDefDim - 0.25 );
            fDefDim -= 0.25;
        }

        if( mKeyboard->isKeyDown( OIS::KC_H ) )
        {
            pThrusters->getEmitter( 0 )->setParticleVelocity( fDefVel + 1 );
            pThrusters->getEmitter( 1 )->setParticleVelocity( fDefVel + 1 );
            fDefVel += 1;            
        }

        if( mKeyboard->isKeyDown( OIS::KC_J ) && !( fDefVel < 0.0f ) )
        {
            pThrusters->getEmitter( 0 )->setParticleVelocity( fDefVel - 1 );
            pThrusters->getEmitter( 1 )->setParticleVelocity( fDefVel - 1 );
            fDefVel -= 1;            
        }

        return true;
    }
};

float SkyBoxFrameListener::fDefDim = 25.0f;
float SkyBoxFrameListener::fDefVel = 50.0f;

class SkyBoxApplication : public ExampleApplication
{
public:
    SkyBoxApplication() {}

protected:
    virtual void createFrameListener(void)
    {
        mFrameListener= new SkyBoxFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skybox
        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 50 );

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        // Also add a nice starship in
        Entity *ent = mSceneMgr->createEntity( "razor", "razor.mesh" );

        mSceneMgr->getRootSceneNode()->attachObject( ent );

        pThrusters = mSceneMgr->createParticleSystem( "ParticleSys1", 200 );

        pThrusters ->setMaterialName( "Examples/Flare" );
        pThrusters ->setDefaultDimensions( 25, 25 );

        ParticleEmitter *pEmit1 = pThrusters ->addEmitter( "Point" );
        ParticleEmitter *pEmit2 = pThrusters ->addEmitter( "Point" );

        // Thruster 1
        pEmit1->setAngle( Degree(3) );
        pEmit1->setTimeToLive( 0.2 );
        pEmit1->setEmissionRate( 70 );

        pEmit1->setParticleVelocity( 50 );

        pEmit1->setDirection(- Vector3::UNIT_Z);
        pEmit1->setColour( ColourValue::White, ColourValue::Red);        

        // Thruster 2
        pEmit2->setAngle( Degree(3) );
        pEmit2->setTimeToLive( 0.2 );
        pEmit2->setEmissionRate( 70 );

        pEmit2->setParticleVelocity( 50 );

        pEmit2->setDirection( -Vector3::UNIT_Z );
        pEmit2->setColour( ColourValue::White, ColourValue::Red );

        // Set the position of the thrusters
        pEmit1->setPosition( Vector3( 5.7f, 0.0f, 0.0f ) );
        pEmit2->setPosition( Vector3( -18.0f, 0.0f, 0.0f ) );

        mSceneMgr->getRootSceneNode()->createChildSceneNode( Vector3( 0.0f, 6.5f, -67.0f ) )
            ->attachObject(pThrusters);
    }

};
