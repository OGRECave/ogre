
#include "LocalCubemapsGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"

#include "OgreLwString.h"

#include "Cubemaps/OgreParallaxCorrectedCubemap.h"

#include "LocalCubemapScene.h"

using namespace Demo;

namespace Demo
{
    LocalCubemapsGameState::LocalCubemapsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mParallaxCorrectedCubemap( 0 ),
        mUseMultipleProbes( true ),
        mRegenerateProbes( true ),
        mRoughnessDirty( false )
    {
        memset( mMaterials, 0, sizeof(mMaterials) );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::setupParallaxCorrectCubemaps(void)
    {
        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

        if( mParallaxCorrectedCubemap )
        {
            hlmsPbs->setParallaxCorrectedCubemap( 0 );

            delete mParallaxCorrectedCubemap;
            mParallaxCorrectedCubemap = 0;
        }

        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
        Ogre::CompositorWorkspaceDef *workspaceDef = compositorManager->getWorkspaceDefinition(
                    "LocalCubemapsProbeWorkspace" );

        mParallaxCorrectedCubemap = new Ogre::ParallaxCorrectedCubemap(
                    Ogre::Id::generateNewId<Ogre::ParallaxCorrectedCubemap>(),
                    mGraphicsSystem->getRoot(),
                    mGraphicsSystem->getSceneManager(),
                    workspaceDef, 250, 1u << 25u );

        mParallaxCorrectedCubemap->setEnabled( true, 1024, 1024, Ogre::PF_R8G8B8A8 );

        Ogre::CubemapProbe *probe = 0;
        Ogre::Aabb roomShape( Ogre::Vector3( -0.505, 3.400016, 5.066226 ),
                              Ogre::Vector3( 5.064587, 3.891282, 9.556003 ) );
        Ogre::Aabb probeArea;
        probeArea.mHalfSize = Ogre::Vector3( 5.064587, 3.891282, 3.891282 );

        if( mUseMultipleProbes )
        {
            //Probe 00
            probe = mParallaxCorrectedCubemap->createProbe();
            probe->setTextureParams( 1024, 1024 );
            probe->initWorkspace();

            probeArea.mCenter = Ogre::Vector3( -0.505, 3.400016, -0.598495 );
            probe->set( probeArea.mCenter, probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                        Ogre::Matrix3::IDENTITY, roomShape );
        }

        //Probe 01
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024 );
        probe->initWorkspace();

        probeArea.mCenter = Ogre::Vector3( -0.505, 3.400016, 5.423867 );
        probe->set( mUseMultipleProbes ? probeArea.mCenter : roomShape.mCenter,
                    mUseMultipleProbes ? probeArea : roomShape,
                    Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, roomShape );

        if( mUseMultipleProbes )
        {
            //Probe 02
            probe = mParallaxCorrectedCubemap->createProbe();
            probe->setTextureParams( 1024, 1024 );
            probe->initWorkspace();

            probeArea.mCenter = Ogre::Vector3( -0.505, 3.400016, 10.657585 );
            probe->set( probeArea.mCenter, probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                        Ogre::Matrix3::IDENTITY, roomShape );
        }

        hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::forceUpdateAllProbes(void)
    {
        const Ogre::CubemapProbeVec &probes = mParallaxCorrectedCubemap->getProbes();

        Ogre::CubemapProbeVec::const_iterator itor = probes.begin();
        Ogre::CubemapProbeVec::const_iterator end  = probes.end();

        while( itor != end )
        {
            (*itor)->mDirty = true;
            ++itor;
        }

        mParallaxCorrectedCubemap->updateAllDirtyProbes();
        mRoughnessDirty = false;
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::createScene01(void)
    {
        setupParallaxCorrectCubemaps();

        //Setup a scene similar to that of PBS sample, except
        //we apply the cubemap to everything via C++ code
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        const float armsLength = 2.5f;

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
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

            Ogre::HlmsBlendblock blendblock;
            Ogre::HlmsMacroblock macroblock;

            Ogre::HlmsPbsDatablock *datablock;
            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Red", "Red",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::Red );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            mMaterials[0] = datablock;

            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Green", "Green",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::Green );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            mMaterials[1] = datablock;

            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Blue", "Blue",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::Blue );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            mMaterials[2] = datablock;

            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Cream", "Cream",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::White );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            mMaterials[3] = datablock;
        }

        generateScene( sceneManager );

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( 1.0f );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mLightNodes[0] = lightNode;

        sceneManager->setAmbientLight( Ogre::ColourValue( 0.3f, 0.5f, 0.7f ) * 0.1f * 0.75f,
                                       Ogre::ColourValue( 0.6f, 0.45f, 0.3f ) * 0.065f * 0.75f,
                                       -light->getDirection() + Ogre::Vector3::UNIT_Y * 0.2f );

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        //lightNode->setPosition( -2.0f, 6.0f, 10.0f );
        lightNode->setPosition( -12.0f, 6.0f, 8.0f );
        light->setDirection( Ogre::Vector3( 1.5, -1, -0.5 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        light->setSpotlightOuterAngle( Ogre::Degree( 80.0f ) );

        mLightNodes[1] = lightNode;

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( 2.0f, 6.0f, -3.0f );
        light->setDirection( Ogre::Vector3( -0.5, -1, 0.5 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        light->setSpotlightOuterAngle( Ogre::Degree( 80.0f ) );

        mLightNodes[2] = lightNode;

        mCameraController = new CameraController( mGraphicsSystem, false );
        mCameraController->mCameraBaseSpeed = 1.0f;
        mCameraController->mCameraSpeedBoost = 10.0f;

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        mParallaxCorrectedCubemap->setUpdatedTrackedDataFromCamera( camera );

        TutorialGameState::createScene01();

        mParallaxCorrectedCubemap->updateAllDirtyProbes();
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::destroyScene(void)
    {
        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
        hlmsPbs->setParallaxCorrectedCubemap( 0 );

        delete mParallaxCorrectedCubemap;
        mParallaxCorrectedCubemap = 0;
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::update( float timeSinceLast )
    {
        /*if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }*/

        //Have the parallax corrected cubemap system keep track of the camera.
        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        mParallaxCorrectedCubemap->setUpdatedTrackedDataFromCamera( camera );

        //camera->setPosition( Ogre::Vector3( -0.505, 3.400016, 5.423867 ) );
        //camera->setPosition( -1.03587, 2.50012, 3.62891 );
        //camera->setOrientation( Ogre::Quaternion::IDENTITY );

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        char tmpBuffer[64];
        Ogre::LwString roughnessStr( Ogre::LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        roughnessStr.a( Ogre::LwString::Float( mMaterials[0]->getRoughness(), 2 ) );

        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2/F3 to adjust material roughness: ";
        outText += roughnessStr.c_str();
        outText += "\nPress F5 to regenerate probes when adjusting roughness: ";
        outText += mRegenerateProbes ? "[Slow & Accurate]" : "[Fast]";
        outText += "\nPress F6 to toggle number of probes. Num probes: ";
        outText += mUseMultipleProbes ? "3" : "1";
        outText += "\nProbes blending: ";
        outText += Ogre::StringConverter::toString( mParallaxCorrectedCubemap->getNumCollectedProbes() );

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        outText += "\nCamera: ";
        outText += Ogre::StringConverter::toString( camera->getPosition().x ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().y ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().z );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            float roughness = mMaterials[0]->getRoughness();
            for( int i=0; i<4; ++i )
                mMaterials[i]->setRoughness( Ogre::Math::Clamp( roughness - 0.1f, 0.02f, 1.0f ) );

            mRoughnessDirty = true;
            if( mRegenerateProbes )
                forceUpdateAllProbes();
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            float roughness = mMaterials[0]->getRoughness();
            for( int i=0; i<4; ++i )
                mMaterials[i]->setRoughness( Ogre::Math::Clamp( roughness + 0.1f, 0.02f, 1.0f ) );

            mRoughnessDirty = true;
            if( mRegenerateProbes )
                forceUpdateAllProbes();
        }
        else if( arg.keysym.sym == SDLK_F5 )
        {
            mRegenerateProbes = !mRegenerateProbes;
            if( mRegenerateProbes && mRoughnessDirty )
                forceUpdateAllProbes();
        }
        else if( arg.keysym.sym == SDLK_F6 )
        {
            mUseMultipleProbes = !mUseMultipleProbes;
            setupParallaxCorrectCubemaps();
            mParallaxCorrectedCubemap->updateAllDirtyProbes();
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
