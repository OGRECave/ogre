
#include "StencilTestGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"
#include "OgreRoot.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"

using namespace Demo;

namespace Demo
{
    StencilTestGameState::StencilTestGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mSceneNode( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void StencilTestGameState::createScene01(void)
    {
        //We MUST do this so the renderWindowDepthBuffer & rt_renderwindow can share
        //the same depth buffer.
        mGraphicsSystem->getRenderWindow()->setPreferDepthTexture( true );

        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        //Make sure both RQs 0 & 1 are able to hold Items.
        Ogre::RenderQueue *renderQueue = sceneManager->getRenderQueue();
        renderQueue->setRenderQueueMode( 0, Ogre::RenderQueue::FAST );
        renderQueue->setRenderQueueMode( 1, Ogre::RenderQueue::FAST );

        {
            Ogre::Item *item = sceneManager->createItem( "Sphere1000.mesh",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_STATIC );

            //This object is modifies the stencil.
            item->setRenderQueueGroup( 0 );

            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
                                                    createChildSceneNode( Ogre::SCENE_STATIC );
            sceneNode->scale( Ogre::Vector3( 2.0f ) );
            sceneNode->attachObject( item );

            //Set an Unlit datablock with alpha blend and alpha = 0 (fully transparent)
            //A more efficient approach would be to write to only the depth buffer. But that
            //is a little more complex for this sample
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();

            Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_UNLIT );

            assert( dynamic_cast<Ogre::HlmsUnlit*>( hlms ) );
            Ogre::HlmsUnlit *unlitHlms = static_cast<Ogre::HlmsUnlit*>( hlms );

            Ogre::HlmsUnlitDatablock *datablock = static_cast<Ogre::HlmsUnlitDatablock*>(
                        unlitHlms->createDatablock( "SphereOfTruth", "SphereOfTruth",
                                                    Ogre::HlmsMacroblock(),
                                                    Ogre::HlmsBlendblock(),
                                                    Ogre::HlmsParamVec() ) );
            item->setDatablock( datablock );
        }

        {
            Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_DYNAMIC );

            //This object is affected by the stencil.
            item->setRenderQueueGroup( 1 );

            mSceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                    createChildSceneNode( Ogre::SCENE_DYNAMIC );

            mSceneNode->setPosition( 0.0f, -0.5f, 0.5f );
            mSceneNode->attachObject( item );
        }

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        camera->setPosition( Ogre::Vector3( 2.5f, 2.5f, 7.5f ) );
        camera->lookAt( Ogre::Vector3( 0, 0, 0 ) );

        mCameraController = new CameraController( mGraphicsSystem );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void StencilTestGameState::update( float timeSinceLast )
    {
        //mSceneNode->yaw( Ogre::Radian(timeSinceLast * i * 0.25f) );

        TutorialGameState::update( timeSinceLast );
    }
}
