
#include "ShadowMapDebuggingGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlay.h"

using namespace Demo;

namespace Demo
{
    ShadowMapDebuggingGameState::ShadowMapDebuggingGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true ),
        mDebugOverlayPSSM( 0 ),
        mDebugOverlaySpotlights( 0 )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane( "Plane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 50.0f, 50.0f,
                                            1, 1, true, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_Z );

        Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createManual(
                    "Plane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        planeMesh->importV1( planeMeshV1.get(), true, true, true );

        {
            Ogre::Item *item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( item );
        }

        float armsLength = 2.5f;

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
                                              2.0f,
                                              (j - 1.5f) * armsLength );
                mSceneNode[idx]->setScale( 0.65f, 0.65f, 0.65f );

                mSceneNode[idx]->roll( Ogre::Radian( (Ogre::Real)idx ) );

                mSceneNode[idx]->attachObject( item );
            }
        }

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->createSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mLightNodes[0] = lightNode;

        light = sceneManager->createLight();
        lightNode = sceneManager->createSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( -10.0f, 10.0f, 10.0f );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        mLightNodes[1] = lightNode;

        light = sceneManager->createLight();
        lightNode = sceneManager->createSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( 10.0f, 10.0f, -10.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        mLightNodes[2] = lightNode;

        mCameraController = new CameraController( mGraphicsSystem, false );

        createShadowMapDebugOverlays();

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::createShadowMapDebugOverlays(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        Ogre::Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );

        Ogre::HlmsMacroblock macroblock;
        macroblock.mDepthCheck = false;
        Ogre::HlmsBlendblock blendblock;

        Ogre::CompositorShadowNode *shadowNode = workspace->findShadowNode( "ShadowMapDebuggingShadowNode" );
        for( int i=0; i<5; ++i )
        {
            const Ogre::String datablockName( "depthShadow" + Ogre::StringConverter::toString( i ) );
            Ogre::HlmsUnlitDatablock *depthShadow = (Ogre::HlmsUnlitDatablock*)hlmsUnlit->createDatablock(
                        datablockName, datablockName,
                        macroblock, blendblock,
                        Ogre::HlmsParamVec() );
            Ogre::TexturePtr tex = shadowNode->getLocalTextures()[i].textures[0];
            depthShadow->setTexture( 0, 0, tex, 0 );
        }

        Ogre::v1::OverlayManager &overlayManager = Ogre::v1::OverlayManager::getSingleton();
        // Create an overlay
        mDebugOverlayPSSM       = overlayManager.create("PSSM Overlays");
        mDebugOverlaySpotlights = overlayManager.create("Spotlight overlays");

        for( int i=0; i<3; ++i )
        {
            // Create a panel
            Ogre::v1::OverlayContainer* panel = static_cast<Ogre::v1::OverlayContainer*>(
                        overlayManager.createOverlayElement( "Panel", "PanelName" +
                                                             Ogre::StringConverter::toString( i ) ));
            panel->setMetricsMode( Ogre::v1::GMM_RELATIVE_ASPECT_ADJUSTED );
            panel->setPosition( 100 + i * 1600, 10000 - 1600 );
            panel->setDimensions( 1500, 1500 );
            panel->setMaterialName( "depthShadow" + Ogre::StringConverter::toString( i ) );
            mDebugOverlayPSSM->add2D( panel );
        }

        for( int i=3; i<5; ++i )
        {
            // Create a panel
            Ogre::v1::OverlayContainer* panel = static_cast<Ogre::v1::OverlayContainer*>(
                        overlayManager.createOverlayElement( "Panel", "PanelName" +
                                                             Ogre::StringConverter::toString( i ) ));
            panel->setMetricsMode( Ogre::v1::GMM_RELATIVE_ASPECT_ADJUSTED );
            panel->setPosition( 100 + i * 1600, 10000 - 1600 );
            panel->setDimensions( 1500, 1500 );
            panel->setMaterialName( "depthShadow" + Ogre::StringConverter::toString( i ) );
            mDebugOverlaySpotlights->add2D( panel );
        }

        mDebugOverlayPSSM->show();
        mDebugOverlaySpotlights->show();
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
        outText += "\nPress F3 to show/hide PSSM splits. ";
        outText += mDebugOverlayPSSM->getVisible() ? "[On]" : "[Off]";
        outText += "\nPress F4 to show/hide spotlight maps. ";
        outText += mDebugOverlaySpotlights->getVisible() ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_F2 )
        {
            mAnimateObjects = !mAnimateObjects;
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            if( mDebugOverlayPSSM->getVisible() )
                mDebugOverlayPSSM->hide();
            else
                mDebugOverlayPSSM->show();
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            if( mDebugOverlaySpotlights->getVisible() )
                mDebugOverlaySpotlights->hide();
            else
                mDebugOverlaySpotlights->show();
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
