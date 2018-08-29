
#include "InstantRadiosityGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"

#include "OgreHlmsPbsDatablock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"

#include "OgreLwString.h"

#include "../LocalCubemaps/LocalCubemapScene.h"

#include "InstantRadiosity/OgreInstantRadiosity.h"
#include "OgreIrradianceVolume.h"
#include "OgreForward3D.h"

using namespace Demo;

namespace Demo
{
    InstantRadiosityGameState::InstantRadiosityGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLightNode( 0 ),
        mLight( 0 ),
        mCurrentType( Ogre::Light::LT_SPOTLIGHT ),
        mInstantRadiosity( 0 ),
        mIrradianceVolume( 0 ),
        mIrradianceCellSize(1.5f)
    {
        mDisplayHelpMode        = 2;
        mNumDisplayHelpModes    = 3;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::createLight(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode();

        if( mLight )
        {
            sceneManager->destroyLight( mLight );
            mLight = 0;
            mLightNode->getParentSceneNode()->removeAndDestroyChild( mLightNode );
            mLightNode = 0;
        }

        mLight = sceneManager->createLight();
        mLightNode = rootNode->createChildSceneNode();
        mLightNode->attachObject( mLight );
        mLight->setPowerScale( Ogre::Math::PI );

        switch( mCurrentType )
        {
        default:
        case Ogre::Light::LT_SPOTLIGHT:
            mLight->setType( Ogre::Light::LT_SPOTLIGHT );
            mLight->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );
            mLightNode->setPosition( Ogre::Vector3( -0.505, 3.400016, 5.423867 ) );
            mLight->setAttenuation( 23.0f, 0.5f, 0.0f, 0.5f );
            break;
        case Ogre::Light::LT_POINT:
            mLight->setType( Ogre::Light::LT_POINT );
            mLightNode->setPosition( Ogre::Vector3( -0.505, 3.400016, 5.423867 ) );
            mLight->setAttenuation( 23.0f, 0.5f, 0.0f, 0.5f );
            break;
        case Ogre::Light::LT_DIRECTIONAL:
            mLight->setType( Ogre::Light::LT_DIRECTIONAL );
            mLight->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
            mLight->setAttenuation( std::numeric_limits<Ogre::Real>::max(), 1.0f, 0, 0 );
            break;
        }

        mInstantRadiosity->build();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::updateIrradianceVolume(void)
    {
        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

        if( !hlmsPbs->getIrradianceVolume() )
            return;

        Ogre::Vector3 volumeOrigin;
        Ogre::Real lightMaxPower;
        Ogre::uint32 numBlocksX, numBlocksY, numBlocksZ;
        mInstantRadiosity->suggestIrradianceVolumeParameters( Ogre::Vector3( mIrradianceCellSize ),
                                                              volumeOrigin, lightMaxPower,
                                                              numBlocksX, numBlocksY, numBlocksZ);
        mIrradianceVolume->createIrradianceVolumeTexture(numBlocksX, numBlocksY, numBlocksZ);
        mInstantRadiosity->fillIrradianceVolume( mIrradianceVolume,
                                                 Ogre::Vector3(mIrradianceCellSize),
                                                 volumeOrigin, lightMaxPower, false );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::createScene01(void)
    {
        //Setup a scene similar to that of PBS sample, except
        //we apply the cubemap to everything via C++ code
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
                Ogre::String finalName = materials[i].matName;

                Ogre::HlmsPbsDatablock *datablock;
                datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                            hlmsPbs->createDatablock( finalName, finalName,
                                                      macroblock, blendblock,
                                                      Ogre::HlmsParamVec() ) );
                datablock->setBackgroundDiffuse( materials[i].colour );
                datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
                datablock->setRoughness( 0.02 );
            }
        }

        generateScene( sceneManager );

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        mInstantRadiosity = new Ogre::InstantRadiosity( sceneManager, hlmsManager );
        mInstantRadiosity->mVplThreshold = 0.0005f;

        //Guide where to shoot the rays for directional lights the 3 windows + the
        //hole in the ceiling). We use a sphere radius of 30 to ensure when the directional
        //light's dir is towards -X, we actually hit walls (instead of going through these
        //walls and generating incorrect results).
        mInstantRadiosity->mAoI.push_back( Ogre::InstantRadiosity::AreaOfInterest(
                    Ogre::Aabb( Ogre::Vector3( -0.746887f, 7.543859f, 5.499001f ),
                                Ogre::Vector3( 2.876101f, 2.716137f, 6.059607f ) * 0.5f ), 30.0f ) );
        mInstantRadiosity->mAoI.push_back( Ogre::InstantRadiosity::AreaOfInterest(
                    Ogre::Aabb( Ogre::Vector3( -6.26f, 3.969576f, 6.628003f ),
                                Ogre::Vector3( 1.673888f, 6.04f, 1.3284f ) * 0.5f ), 30.0f ) );
        mInstantRadiosity->mAoI.push_back( Ogre::InstantRadiosity::AreaOfInterest(
                    Ogre::Aabb( Ogre::Vector3( -6.26f, 3.969576f, 3.083399f ),
                                Ogre::Vector3( 1.673888f, 6.04f, 1.3284f ) * 0.5f ), 30.0f ) );
        mInstantRadiosity->mAoI.push_back( Ogre::InstantRadiosity::AreaOfInterest(
                    Ogre::Aabb( Ogre::Vector3( -6.26f, 3.969576f, -0.415852f ),
                                Ogre::Vector3( 1.673888f, 6.04f, 1.3284f ) * 0.5f ), 30.0f ) );

        createLight();

        mCameraController = new CameraController( mGraphicsSystem, false );
        mCameraController->mCameraBaseSpeed = 1.0f;
        mCameraController->mCameraSpeedBoost = 10.0f;

        sceneManager->setForwardClustered( true, 16, 8, 24, 96, 0, 2, 50 );
        //Required by InstantRadiosity
        sceneManager->getForwardPlus()->setEnableVpls( true );

        mIrradianceVolume = new Ogre::IrradianceVolume(hlmsManager);

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::destroyScene(void)
    {
//        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
//        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
//        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
//        hlmsPbs->setParallaxCorrectedCubemap( 0 );

        delete mInstantRadiosity;
        mInstantRadiosity = 0;
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::update( float timeSinceLast )
    {
        /*if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }*/

        std::map<SDL_Keycode, SDL_Keysym>::const_iterator itor = mKeysHold.begin();
        std::map<SDL_Keycode, SDL_Keysym>::const_iterator end  = mKeysHold.end();

        bool needsIrradianceVolumeRebuild = false;
        bool changedVplSetting = false;
        bool needsRebuild = false;
        while( itor != end )
        {
            const SDL_Keysym &keySym = itor->second;
            const bool reverse = (keySym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) != 0;
            const float modPerFrame = reverse ? -timeSinceLast : timeSinceLast;
            if( keySym.sym == SDLK_h )
            {
                mInstantRadiosity->mCellSize += modPerFrame;
                mInstantRadiosity->mCellSize = Ogre::max( mInstantRadiosity->mCellSize,
                                                          Ogre::Real(0.001f) );
                needsRebuild = true;
            }
            if( keySym.sym == SDLK_j )
            {
                mInstantRadiosity->mBias += modPerFrame;
                mInstantRadiosity->mBias = Ogre::Math::Clamp( mInstantRadiosity->mBias,
                                                              Ogre::Real(0.0f), Ogre::Real(1.0f) );
                needsRebuild = true;
            }
            if( keySym.sym == SDLK_u )
            {
                mInstantRadiosity->mVplMaxRange += modPerFrame * 4.0f;
                changedVplSetting = true;
                needsIrradianceVolumeRebuild = true;
            }
            if( keySym.sym == SDLK_i )
            {
                mInstantRadiosity->mVplPowerBoost += modPerFrame * 2.0f;
                changedVplSetting = true;
                needsIrradianceVolumeRebuild = true;
            }
            if( keySym.sym == SDLK_o )
            {
                mInstantRadiosity->mVplThreshold += modPerFrame * 0.05f;
                changedVplSetting = true;
            }
            if( keySym.sym == SDLK_p )
            {
                mInstantRadiosity->mVplIntensityRangeMultiplier += modPerFrame * 10.0;
                mInstantRadiosity->mVplIntensityRangeMultiplier =
                        Ogre::max( mInstantRadiosity->mVplIntensityRangeMultiplier, 0.01 );
                changedVplSetting = true;
                needsIrradianceVolumeRebuild = true;
            }
            if( keySym.sym == SDLK_m )
            {
                mIrradianceCellSize += modPerFrame * 10.0f;
                mIrradianceCellSize = std::max( mIrradianceCellSize, Ogre::Real(0.1f) );
                needsIrradianceVolumeRebuild = true;
            }

            ++itor;
        }

        if( changedVplSetting && !needsRebuild )
            mInstantRadiosity->updateExistingVpls();
        if( needsRebuild )
            mInstantRadiosity->build();

        if( needsIrradianceVolumeRebuild || needsRebuild )
            updateIrradianceVolume();

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        if( mDisplayHelpMode != 2 )
            return;

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

        outText += "\nF2 to toggle debug VPL markers ";
        outText += mInstantRadiosity->getEnableDebugMarkers() ? "[On]" : "[Off]";
        outText += "\nF3 to change light type ";
        switch( mCurrentType )
        {
        default:
        case Ogre::Light::LT_SPOTLIGHT:
            outText += "[Spot]";
            break;
        case Ogre::Light::LT_POINT:
            outText += "[Point]";
            break;
        case Ogre::Light::LT_DIRECTIONAL:
            outText += "[Directional]";
            break;
        }
        outText += "\nF4 to toggle intensity for max range ";
        outText += mInstantRadiosity->mVplUseIntensityForMaxRange ? "[On]" : "[Off]";
        outText += "\nF5 to use Irradiance Volumes instead of VPLs ";
        outText += hlmsPbs->getIrradianceVolume() ? "[Irradiance]" : "[VPL]";

        outText += "\nHold [Shift] to change value in opposite direction";
        outText += "\nVPL Max range [U]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mVplMaxRange );
        outText += "\nVPL Power Boost [I]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mVplPowerBoost );
        outText += "\nVPL Threshold [O]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mVplThreshold );
        if( mInstantRadiosity->mVplUseIntensityForMaxRange )
        {
            outText += "\nVPL Intensity Range Multiplier [P]: ";
            outText += Ogre::StringConverter::toString(
                        mInstantRadiosity->mVplIntensityRangeMultiplier );
        }

        outText += "\nNum Rays [G]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mNumRays );
        outText += "\nCluster size [H]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mCellSize );
        outText += "\nBias [J]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mBias );
        outText += "\nSpread Iterations [K]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mNumSpreadIterations );
        outText += "\nNum bounces [L]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mNumRayBounces );

        if( hlmsPbs->getIrradianceVolume() )
        {
            outText += "\nIrradiance Cell Size [M]: ";
            outText += Ogre::StringConverter::toString( mIrradianceCellSize );
        }

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        outText += "\nCamera: ";
        outText += Ogre::StringConverter::toString( camera->getPosition().x ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().y ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().z );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        mKeysHold[arg.keysym.sym] = arg.keysym;
        TutorialGameState::keyPressed( arg );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        mKeysHold.erase( arg.keysym.sym );

        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS|KMOD_LSHIFT|KMOD_RSHIFT)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            mInstantRadiosity->setEnableDebugMarkers( !mInstantRadiosity->getEnableDebugMarkers() );
        }
        else if( arg.keysym.sym == SDLK_F3 )
        {
            mCurrentType = static_cast<Ogre::Light::LightTypes>( (mCurrentType + 1) %
                                                                 Ogre::Light::LT_VPL );
            createLight();
            updateIrradianceVolume();
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            mInstantRadiosity->mVplUseIntensityForMaxRange =
                    !mInstantRadiosity->mVplUseIntensityForMaxRange;
            mInstantRadiosity->updateExistingVpls();
            updateIrradianceVolume();
        }
        else if( arg.keysym.sym == SDLK_F5 )
        {
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

            if( !hlmsPbs->getIrradianceVolume() )
            {
                hlmsPbs->setIrradianceVolume( mIrradianceVolume );
                updateIrradianceVolume();
                mInstantRadiosity->setUseIrradianceVolume(true);
            }
            else
            {
                hlmsPbs->setIrradianceVolume( 0 );
                mInstantRadiosity->setUseIrradianceVolume(false);
            }
        }
        else if( arg.keysym.sym == SDLK_g )
        {
            const bool reverse = (arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) != 0;
            if( reverse )
                mInstantRadiosity->mNumRays >>= 1u;
            else
                mInstantRadiosity->mNumRays <<= 1u;

            //Too many rays and the app will become unresponsive
            mInstantRadiosity->mNumRays = std::max<size_t>( mInstantRadiosity->mNumRays, 1u );
            mInstantRadiosity->mNumRays = std::min<size_t>( mInstantRadiosity->mNumRays, 32768u );

            mInstantRadiosity->build();
            updateIrradianceVolume();
        }
        else if( arg.keysym.sym == SDLK_k )
        {
            const bool reverse = (arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) != 0;
            if( reverse )
            {
                if( mInstantRadiosity->mNumSpreadIterations )
                    --mInstantRadiosity->mNumSpreadIterations;
            }
            else
            {
                if( mInstantRadiosity->mNumSpreadIterations < 10 )
                    ++mInstantRadiosity->mNumSpreadIterations;
            }

            mInstantRadiosity->build();
            updateIrradianceVolume();
        }
        else if( arg.keysym.sym == SDLK_l )
        {
            const bool reverse = (arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) != 0;
            if( reverse )
            {
                if( mInstantRadiosity->mNumRayBounces )
                    --mInstantRadiosity->mNumRayBounces;
            }
            else
            {
                if( mInstantRadiosity->mNumRayBounces < 5 )
                    ++mInstantRadiosity->mNumRayBounces;
            }

            mInstantRadiosity->build();
            updateIrradianceVolume();
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
