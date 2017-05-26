
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
#include "OgreHlmsPbs.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlay.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"

#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"
#include "Utils/MiscUtils.h"

using namespace Demo;

namespace Demo
{
    const Ogre::String c_shadowMapFilters[Ogre::HlmsPbs::NumShadowFilter] =
    {
        "PCF 2x2",
        "PCF 3x3",
        "PCF 4x4",
        "ESM"
    };

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
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( -10.0f, 10.0f, 10.0f );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

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

        mLightNodes[2] = lightNode;

        mCameraController = new CameraController( mGraphicsSystem, false );

        createShadowMapDebugOverlays();

#if !OGRE_NO_JSON
        //For ESM, setup the filter settings (radius and gaussian deviation).
        //It controls how blurry the shadows will look.
        Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
        Ogre::HlmsCompute *hlmsCompute = hlmsManager->getComputeHlms();

        Ogre::uint8 kernelRadius = 8;
        float gaussianDeviationFactor = 0.5f;
        Ogre::uint16 K = 80;
        Ogre::HlmsComputeJob *job = 0;

        //Setup compute shader filter (faster for large kernels; but
        //beware of mobile hardware where compute shaders are slow)
        //For reference large kernels means kernelRadius > 2 (approx)
        job = hlmsCompute->findComputeJob( "ESM/GaussianLogFilterH" );
        MiscUtils::setGaussianLogFilterParams( job, kernelRadius, gaussianDeviationFactor, K );
        job = hlmsCompute->findComputeJob( "ESM/GaussianLogFilterV" );
        MiscUtils::setGaussianLogFilterParams( job, kernelRadius, gaussianDeviationFactor, K );

        //Setup pixel shader filter (faster for small kernels, also to use as a fallback
        //on GPUs that don't support compute shaders, or where compute shaders are slow).
        MiscUtils::setGaussianLogFilterParams( "ESM/GaussianLogFilterH", kernelRadius,
                                               gaussianDeviationFactor, K );
        MiscUtils::setGaussianLogFilterParams( "ESM/GaussianLogFilterV", kernelRadius,
                                               gaussianDeviationFactor, K );
#endif

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    const char* ShadowMapDebuggingGameState::chooseEsmShadowNode(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::RenderSystem *renderSystem = root->getRenderSystem();

        const Ogre::RenderSystemCapabilities *capabilities = renderSystem->getCapabilities();
        bool hasCompute = capabilities->hasCapability( Ogre::RSC_COMPUTE_PROGRAM );

        if( !hasCompute )
        {
            //There's no choice.
            return "ShadowMapDebuggingEsmShadowNodePixelShader";
        }
        else
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            //On iOS, the A7 GPUs have slow compute shaders.
            Ogre::DriverVersion driverVersion = capabilities->getDriverVersion();
            if( driverVersion.major == 1 );
                return "ShadowMapDebuggingEsmShadowNodePixelShader";
#endif
            return "ShadowMapDebuggingEsmShadowNodeCompute";
        }
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::setupShadowNode( bool forEsm )
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();

        Ogre::CompositorNodeDef *nodeDef =
                compositorManager->getNodeDefinitionNonConst( "ShadowMapDebuggingRenderingNode" );

        Ogre::CompositorTargetDef *targetDef = nodeDef->getTargetPass( 0 );
        const Ogre::CompositorPassDefVec &passes = targetDef->getCompositorPasses();

        assert( dynamic_cast<Ogre::CompositorPassSceneDef*>( passes[1] ) );
        Ogre::CompositorPassSceneDef *passSceneDef =
                static_cast<Ogre::CompositorPassSceneDef*>( passes[1] );

        if( forEsm && passSceneDef->mShadowNode == "ShadowMapDebuggingShadowNode" )
        {
            destroyShadowMapDebugOverlays();
            mGraphicsSystem->stopCompositor();
            passSceneDef->mShadowNode = chooseEsmShadowNode();
            mGraphicsSystem->restartCompositor();
            createShadowMapDebugOverlays();
        }
        else if( !forEsm && passSceneDef->mShadowNode != "ShadowMapDebuggingShadowNode" )
        {
            destroyShadowMapDebugOverlays();
            mGraphicsSystem->stopCompositor();
            passSceneDef->mShadowNode = "ShadowMapDebuggingShadowNode";
            mGraphicsSystem->restartCompositor();
            createShadowMapDebugOverlays();
        }
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::createShadowMapDebugOverlays(void)
    {
        destroyShadowMapDebugOverlays();

        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        Ogre::Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );

        Ogre::HlmsMacroblock macroblock;
        macroblock.mDepthCheck = false;
        Ogre::HlmsBlendblock blendblock;

        bool isUsingEsm = false;
        {
            Ogre::Hlms *hlms = root->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
            assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
            Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );
            isUsingEsm = pbs->getShadowFilter() == Ogre::HlmsPbs::ExponentialShadowMaps;
        }

        const Ogre::String shadowNodeName = isUsingEsm ? chooseEsmShadowNode() :
                                                         "ShadowMapDebuggingShadowNode";

        Ogre::CompositorShadowNode *shadowNode = workspace->findShadowNode( shadowNodeName );
        const Ogre::CompositorShadowNodeDef *shadowNodeDef = shadowNode->getDefinition();

        for( int i=0; i<5; ++i )
        {
            const Ogre::String datablockName( "depthShadow" + Ogre::StringConverter::toString( i ) );
            Ogre::HlmsUnlitDatablock *depthShadow =
                    (Ogre::HlmsUnlitDatablock*)hlmsUnlit->getDatablock( datablockName );

            if( !depthShadow )
            {
                depthShadow = (Ogre::HlmsUnlitDatablock*)hlmsUnlit->createDatablock(
                            datablockName, datablockName, macroblock, blendblock,
                            Ogre::HlmsParamVec() );
            }

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
    void ShadowMapDebuggingGameState::destroyShadowMapDebugOverlays(void)
    {
        Ogre::v1::OverlayManager &overlayManager = Ogre::v1::OverlayManager::getSingleton();

        if( mDebugOverlayPSSM )
        {
            Ogre::v1::Overlay::Overlay2DElementsIterator itor =
                    mDebugOverlayPSSM->get2DElementsIterator();
            while( itor.hasMoreElements() )
            {
                Ogre::v1::OverlayContainer *panel = itor.getNext();
                overlayManager.destroyOverlayElement( panel );
            }
            overlayManager.destroy( mDebugOverlayPSSM );
            mDebugOverlayPSSM = 0;
        }

        if( mDebugOverlaySpotlights )
        {
            Ogre::v1::Overlay::Overlay2DElementsIterator itor =
                    mDebugOverlaySpotlights->get2DElementsIterator();
            while( itor.hasMoreElements() )
            {
                Ogre::v1::OverlayContainer *panel = itor.getNext();
                overlayManager.destroyOverlayElement( panel );
            }
            overlayManager.destroy( mDebugOverlaySpotlights );
            mDebugOverlaySpotlights = 0;
        }
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
        Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

        assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
        outText += "\nPress F3 to show/hide PSSM splits. ";
        outText += mDebugOverlayPSSM->getVisible() ? "[On]" : "[Off]";
        outText += "\nPress F4 to show/hide spotlight maps. ";
        outText += mDebugOverlaySpotlights->getVisible() ? "[On]" : "[Off]";
        outText += "\nPress F5 to switch filter. [" + c_shadowMapFilters[pbs->getShadowFilter()] + "]";
    }
    //-----------------------------------------------------------------------------------
    void ShadowMapDebuggingGameState::keyReleased( const SDL_KeyboardEvent &arg )
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
        else if( arg.keysym.sym == SDLK_F5 )
        {
            Ogre::Hlms *hlms = mGraphicsSystem->getRoot()->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

            assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
            Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

            Ogre::HlmsPbs::ShadowFilter nextFilter = static_cast<Ogre::HlmsPbs::ShadowFilter>(
                        (pbs->getShadowFilter() + 1u) % Ogre::HlmsPbs::NumShadowFilter );

#if OGRE_NO_JSON
            if( nextFilter == Ogre::HlmsPbs::ExponentialShadowMaps )
            {
                nextFilter = static_cast<Ogre::HlmsPbs::ShadowFilter>(
                                 (nextFilter + 1u) % Ogre::HlmsPbs::NumShadowFilter );
            }
#endif

            if( nextFilter == Ogre::HlmsPbs::ExponentialShadowMaps )
                pbs->getHlmsManager()->setShadowMappingUseBackFaces( false );
            else
                pbs->getHlmsManager()->setShadowMappingUseBackFaces( true );

            pbs->setShadowSettings( nextFilter );

            if( nextFilter == Ogre::HlmsPbs::ExponentialShadowMaps )
                setupShadowNode( true );
            else
                setupShadowNode( false );
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
