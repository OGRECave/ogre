
#include "StereoRenderingGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreCamera.h"

using namespace Demo;

namespace Demo
{
    StereoRenderingGameState::StereoRenderingGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
        memset( mWASD, 0, sizeof(mWASD) );
    }
    //-----------------------------------------------------------------------------------
    void StereoRenderingGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        const float armsLength = 2.5f;

        for( int i=0; i<4; ++i )
        {
            for( int j=0; j<4; ++j )
            {
                Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                             Ogre::ResourceGroupManager::
                                                             AUTODETECT_RESOURCE_GROUP_NAME,
                                                             Ogre::SCENE_DYNAMIC );

                size_t idx = i * 4 + j;

                mSceneNode[idx] = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                        createChildSceneNode( Ogre::SCENE_DYNAMIC );

                mSceneNode[idx]->setPosition( (i - 1.5f) * armsLength,
                                              0.0f,
                                              (j - 1.5f) * armsLength );

                mSceneNode[idx]->roll( Ogre::Radian( idx ) );

                mSceneNode[idx]->attachObject( item );
            }
        }

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->createSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void StereoRenderingGameState::update( float timeSinceLast )
    {
        for( int i=0; i<16; ++i )
            mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.25f) );

        int camMovementZ = mWASD[2] - mWASD[0];
        int camMovementX = mWASD[3] - mWASD[1];

        if( camMovementZ || camMovementX )
        {
            Ogre::Vector3 camMovementDir( camMovementX, 0, camMovementZ );
            camMovementDir.normalise();
            camMovementDir *= timeSinceLast * 10.0f;

            Ogre::Node *cameraNode = mGraphicsSystem->getCamera()->getParentNode();
            cameraNode->translate( camMovementDir, Ogre::Node::TS_LOCAL );
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void StereoRenderingGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_w )
            mWASD[0] = true;
        else if( arg.keysym.sym == SDLK_a )
            mWASD[1] = true;
        else if( arg.keysym.sym == SDLK_s )
            mWASD[2] = true;
        else if( arg.keysym.sym == SDLK_d )
            mWASD[3] = true;
        else
            TutorialGameState::keyPressed( arg );
    }
    //-----------------------------------------------------------------------------------
    void StereoRenderingGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_w )
            mWASD[0] = false;
        else if( arg.keysym.sym == SDLK_a )
            mWASD[1] = false;
        else if( arg.keysym.sym == SDLK_s )
            mWASD[2] = false;
        else if( arg.keysym.sym == SDLK_d )
            mWASD[3] = false;
        else
            TutorialGameState::keyReleased( arg );
    }
}
