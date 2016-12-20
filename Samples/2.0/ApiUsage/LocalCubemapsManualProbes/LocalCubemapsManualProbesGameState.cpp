
#include "LocalCubemapsManualProbesGameState.h"
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

#include "LocalCubemapsManualProbesScene.h"

#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"

using namespace Demo;

namespace Demo
{
    LocalCubemapsManualProbesGameState::LocalCubemapsManualProbesGameState(
            const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mParallaxCorrectedCubemap( 0 )
    {
        memset( mMaterials, 0, sizeof(mMaterials) );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsManualProbesGameState::setupParallaxCorrectCubemaps(void)
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
        Ogre::Vector3 probeCenter;
        Ogre::Aabb probeArea;
        Ogre::Aabb probeShape;

        //Probe 00
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024, true );
        probe->initWorkspace();

        probeCenter = Ogre::Vector3( -0.0, 1.799999, 0.232415 );
        probeArea.mCenter = Ogre::Vector3( -0.0, 12.011974, 0.232414 );
        probeArea.mHalfSize = Ogre::Vector3( 12.589478, 24.297745, 10.569476 ) * 0.5;
        probeShape.mCenter = Ogre::Vector3( -0.0, 12.011974, 0.232414 );
        probeShape.mHalfSize = Ogre::Vector3( 12.589478, 24.297745, 10.569476 ) * 0.5;
        probe->set( probeCenter, probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, probeShape );

        //Probe 01
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024, true );
        probe->initWorkspace();

        probeCenter = Ogre::Vector3( -5.232418, 1.799997, 18.313454 );
        probeArea.mCenter = Ogre::Vector3( -5.263578, 12.011974, 13.145589 );
        probeArea.mHalfSize = Ogre::Vector3( 2.062322, 24.297745, 17.866102 ) * 0.5;
        probeShape.mCenter = Ogre::Vector3( -5.211694, 12.011974, 8.478205 );
        probeShape.mHalfSize = Ogre::Vector3( 2.166091, 24.297745, 27.200872 ) * 0.5;
        probe->set( probeCenter, probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, probeShape );

        //Probe 02
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024, true );
        probe->initWorkspace();

        probeCenter = Ogre::Vector3( 3.767576, 1.799997, 20.84387 );
        probeArea.mCenter = Ogre::Vector3( 2.632758, 12.011975, 22.444103 );
        probeArea.mHalfSize = Ogre::Vector3( 10.365083, 24.297745, 21.705084 ) * 0.5;
        probeShape.mCenter = Ogre::Vector3( 3.752187, 12.011975, 22.444103 );
        probeShape.mHalfSize = Ogre::Vector3( 8.126225, 24.297745, 21.705084 ) * 0.5;
        probe->set( probeCenter, probeArea, Ogre::Vector3( 0.7f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, probeShape );

        //Probe 03
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024, true );
        probe->initWorkspace();

        probeCenter = Ogre::Vector3( -2.565753, 1.799996, 20.929661 );
        probeArea.mCenter = Ogre::Vector3( -2.703529, 12.011974, 21.099365 );
        probeArea.mHalfSize = Ogre::Vector3( 7.057773, 24.297745, 2.166093 ) * 0.5;
        probeShape.mCenter = Ogre::Vector3( 0.767578, 12.011974, 21.099365 );
        probeShape.mHalfSize = Ogre::Vector3( 13.999986, 24.297745, 2.166093 ) * 0.5;
        probe->set( probeCenter, probeArea, Ogre::Vector3( 0.7f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, probeShape );

        hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );

//        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().load(
//                    "SkyPostprocess", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
//                staticCast<Ogre::Material>();
//        Ogre::TextureUnitState *tu = mat->getBestTechnique()->getPass(0)->getTextureUnitState(0);
//        tu->setTexture( mParallaxCorrectedCubemap->getBlendCubemap() );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsManualProbesGameState::createScene01(void)
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

            struct DemoMaterials
            {
                Ogre::String matName;
                Ogre::ColourValue colour;
            };

            DemoMaterials materials[4] =
            {
                { "Red", Ogre::ColourValue::Red },
                { "Green", Ogre::ColourValue::Green },
                { "Blue", Ogre::ColourValue::Blue },
                { "Cream", Ogre::ColourValue::White },
            };

            for( int i=0; i<4; ++i )
            {
                for( int j=0; j<4; ++j )
                {
                    Ogre::String finalName = materials[i].matName + "_P" +
                            Ogre::StringConverter::toString(j);

                    Ogre::HlmsPbsDatablock *datablock;
                    datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                                hlmsPbs->createDatablock( finalName, finalName,
                                                          macroblock, blendblock,
                                                          Ogre::HlmsParamVec() ) );
                    datablock->setBackgroundDiffuse( materials[i].colour );
                    datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
                    datablock->setRoughness( 0.02 );
                    //datablock->setCubemapProbe( mParallaxCorrectedCubemap->getProbes()[j] );
                    mMaterials[i*4+j] = datablock;
                }
            }
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
        camera->setPosition( Ogre::Vector3( 3.767576, 1.799997, 20.84387 ) );
        camera->lookAt( camera->getPosition() + Ogre::Vector3( -1, 0, 0 ) );
        mParallaxCorrectedCubemap->setUpdatedTrackedDataFromCamera( camera );

        TutorialGameState::createScene01();

        mParallaxCorrectedCubemap->updateAllDirtyProbes();

        //Updates the probe after assigning the manual ones, as results will be different.
        //Whether they look better or worse depends on how good you've subdivided the
        //scene and assigned the manual probes.
        const Ogre::CubemapProbeVec &probes = mParallaxCorrectedCubemap->getProbes();
        for( int i=0; i<4*4; ++i )
            mMaterials[i]->setCubemapProbe( probes[i%4] );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsManualProbesGameState::destroyScene(void)
    {
        for( int i=0; i<4*4; ++i )
            mMaterials[i]->setCubemapProbe( 0 );

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
        hlmsPbs->setParallaxCorrectedCubemap( 0 );

        delete mParallaxCorrectedCubemap;
        mParallaxCorrectedCubemap = 0;
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsManualProbesGameState::update( float timeSinceLast )
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
    void LocalCubemapsManualProbesGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        char tmpBuffer[64];
        Ogre::LwString roughnessStr( Ogre::LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        roughnessStr.a( Ogre::LwString::Float( mMaterials[0]->getRoughness(), 2 ) );

        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2/F3 to adjust material roughness: ";
        outText += roughnessStr.c_str();
        outText += "\nPress F4 to toggle mode. ";
        outText += mMaterials[0]->getCubemapProbe() == 0 ? "[Auto]" : "[Manual]";
        outText += "\nProbes blending: ";
        outText += Ogre::StringConverter::toString( mParallaxCorrectedCubemap->getNumCollectedProbes() );

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        outText += "\nCamera: ";
        outText += Ogre::StringConverter::toString( camera->getPosition().x ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().y ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().z );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsManualProbesGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            float roughness = mMaterials[0]->getRoughness();
            for( int i=0; i<4*4; ++i )
                mMaterials[i]->setRoughness( Ogre::Math::Clamp( roughness - 0.1f, 0.02f, 1.0f ) );
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            float roughness = mMaterials[0]->getRoughness();
            for( int i=0; i<4*4; ++i )
                mMaterials[i]->setRoughness( Ogre::Math::Clamp( roughness + 0.1f, 0.02f, 1.0f ) );
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            const Ogre::CubemapProbeVec &probes = mParallaxCorrectedCubemap->getProbes();
            for( int i=0; i<4*4; ++i )
            {
                if( mMaterials[i]->getCubemapProbe() )
                    mMaterials[i]->setCubemapProbe( 0 );
                else
                    mMaterials[i]->setCubemapProbe( probes[i%4] );
            }
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
