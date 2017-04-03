
#include "StaticShadowMapsGameState.h"
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
#include "OgreHlmsPbs.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlay.h"

using namespace Demo;

namespace Demo
{
    const Ogre::String c_shadowMapFilters[Ogre::HlmsPbs::NumShadowFilter] =
    {
        "PCF 2x2",
        "PCF 3x3",
        "PCF 4x4"
    };

    StaticShadowMapsGameState::StaticShadowMapsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( false ),
        mUpdateShadowMaps( true ),
        mShadowNode( 0 ),
        mDebugOverlayPSSM( 0 ),
        mDebugOverlaySpotlights( 0 )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void StaticShadowMapsGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane( "Plane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 50.0f, 50.0f,
                                            1, 1, true, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_Z,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );

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

        Ogre::CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        mShadowNode = workspace->findShadowNode( "StaticShadowMapsShadowNode" );

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mLightNodes[0] = lightNode;

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_POINT );
        lightNode->setPosition( -2.0f, 5.0f, 2.0f );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        mShadowNode->setLightFixedToShadowMap( 3, light );

        mLightNodes[1] = lightNode;

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( 10.0f, 10.0f, -10.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        mShadowNode->setLightFixedToShadowMap( 4, light );

        mLightNodes[2] = lightNode;

        mCameraController = new CameraController( mGraphicsSystem, false );

        createShadowMapDebugOverlays();

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void StaticShadowMapsGameState::createShadowMapDebugOverlays(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        Ogre::Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );

        Ogre::HlmsMacroblock macroblock;
        macroblock.mDepthCheck = false;
        Ogre::HlmsBlendblock blendblock;

        Ogre::CompositorShadowNode *shadowNode =
                workspace->findShadowNode( "StaticShadowMapsShadowNode" );
        const Ogre::CompositorShadowNodeDef *shadowNodeDef = shadowNode->getDefinition();

        for( int i=0; i<5; ++i )
        {
            const Ogre::String datablockName( "depthShadow" + Ogre::StringConverter::toString( i ) );
            Ogre::HlmsUnlitDatablock *depthShadow = (Ogre::HlmsUnlitDatablock*)hlmsUnlit->createDatablock(
                        datablockName, datablockName,
                        macroblock, blendblock,
                        Ogre::HlmsParamVec() );

            const Ogre::ShadowTextureDefinition *shadowTexDef =
                    shadowNodeDef->getShadowTextureDefinition( i );

            Ogre::TexturePtr tex = shadowNode->getDefinedTexture( shadowTexDef->getTextureNameStr(),
                                                                  shadowTexDef->mrtIndex );
            depthShadow->setTexture( 0, shadowTexDef->arrayIdx, tex, 0 );

            //If it's an UV atlas, then only display the relevant section.
            Ogre::Matrix4 uvOffsetScale;
            uvOffsetScale.makeTransform( Ogre::Vector3( shadowTexDef->uvOffset.x,
                                                        shadowTexDef->uvOffset.y, 0.0f ),
                                         Ogre::Vector3( shadowTexDef->uvLength.x,
                                                        shadowTexDef->uvLength.y, 1.0f ),
                                         Ogre::Quaternion::IDENTITY );
            depthShadow->setEnableAnimationMatrix( 0, true );
            depthShadow->setAnimationMatrix( 0, uvOffsetScale );
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
    void StaticShadowMapsGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );

            if( mUpdateShadowMaps )
            {
                //We don't need to call setStaticShadowMapDirty( 4 ) because 3 & 4 share
                //the same atlas and we pass true to the last parameter. If you're going to
                //call setStaticShadowMapDirty() on all of the shadow maps, see the function's
                //documentation on passing false to prevent a O(N^2) behavior.
                //If you do pass false, make sure all of the linked static shadow maps are
                //flagged as dirty otherwise the results may not be what you expect.
                mShadowNode->setStaticShadowMapDirty( 3, true );
            }
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void StaticShadowMapsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

        assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

        TutorialGameState::generateDebugText( timeSinceLast, outText );
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
        if( mAnimateObjects )
        {
            outText += "\nPress F3 to update shadow maps. ";
            outText += mUpdateShadowMaps ? "[On]" : "[Off]";
        }
        outText += "\nPress F4 to show/hide PSSM splits. ";
        outText += mDebugOverlayPSSM->getVisible() ? "[On]" : "[Off]";
        outText += "\nPress F5 to show/hide spotlight maps. ";
        outText += mDebugOverlaySpotlights->getVisible() ? "[On]" : "[Off]";
        outText += "\nPress F6 to switch filter. [" + c_shadowMapFilters[pbs->getShadowFilter()] + "]";
    }
    //-----------------------------------------------------------------------------------
    void StaticShadowMapsGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            mAnimateObjects = !mAnimateObjects;
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            if( mAnimateObjects )
                mUpdateShadowMaps = !mUpdateShadowMaps;
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            if( mDebugOverlayPSSM->getVisible() )
                mDebugOverlayPSSM->hide();
            else
                mDebugOverlayPSSM->show();
        }
        else if( arg.keysym.sym == SDLK_F5 )
        {
            if( mDebugOverlaySpotlights->getVisible() )
                mDebugOverlaySpotlights->hide();
            else
                mDebugOverlaySpotlights->show();
        }
        else if( arg.keysym.sym == SDLK_F6 )
        {
            Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

            assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
            Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

            pbs->setShadowSettings( static_cast<Ogre::HlmsPbs::ShadowFilter>(
                                        (pbs->getShadowFilter() + 1) %
                                        Ogre::HlmsPbs::NumShadowFilter ) );
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
