
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
#include "OgreForward3D.h"

using namespace Demo;

namespace Demo
{
    InstantRadiosityGameState::InstantRadiosityGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLightNode( 0 ),
        mLight( 0 ),
        mInstantRadiosity( 0 ),
        mCurrentType( Ogre::Light::LT_SPOTLIGHT )
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
            break;
        case Ogre::Light::LT_POINT:
            mLight->setType( Ogre::Light::LT_POINT );
            mLightNode->setPosition( Ogre::Vector3( -0.505, 3.400016, 5.423867 ) );
            break;
        case Ogre::Light::LT_DIRECTIONAL:
            mLight->setType( Ogre::Light::LT_DIRECTIONAL );
            mLight->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
            break;
        }
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

        createLight();

        mCameraController = new CameraController( mGraphicsSystem, false );
        mCameraController->mCameraBaseSpeed = 1.0f;
        mCameraController->mCameraSpeedBoost = 10.0f;

        sceneManager->setForward3D( true, 4, 4, 4, 96, 0.5, 20 );
        sceneManager->getForward3D()->setFadeAttenuationRange( true );

        TutorialGameState::createScene01();

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        mInstantRadiosity = new Ogre::InstantRadiosity( sceneManager, hlmsManager );
        mInstantRadiosity->build();
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

        bool changedVplSetting = false;
        bool needsRebuild = false;
        while( itor != end )
        {
            const SDL_Keysym &keySym = itor->second;
            const bool reverse = (keySym.mod & (KMOD_LSHIFT|KMOD_RSHIFT));
            const float modPerFrame = reverse ? -timeSinceLast : timeSinceLast;
            if( keySym.sym == SDLK_j )
            {
                mInstantRadiosity->mCellSize += modPerFrame;
                mInstantRadiosity->mCellSize = Ogre::max( mInstantRadiosity->mCellSize,
                                                          Ogre::Real(0.001f) );
                needsRebuild = true;
            }
            if( keySym.sym == SDLK_k )
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
            }
            if( keySym.sym == SDLK_i )
            {
                mInstantRadiosity->mVplPowerBoost += modPerFrame * 2.0f;
                changedVplSetting = true;
            }
            if( keySym.sym == SDLK_o )
            {
                mInstantRadiosity->mVplThreshold += modPerFrame * 0.05f;
                changedVplSetting = true;
            }

            ++itor;
        }

        if( changedVplSetting && !needsRebuild )
            mInstantRadiosity->updateExistingVpls();
        if( needsRebuild )
            mInstantRadiosity->build();

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void InstantRadiosityGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        outText += "\nHold [Shift] to change value in opposite direction";
        outText += "\nVPL Max range [U]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mVplMaxRange );
        outText += "\nVPL Power Boost [I]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mVplPowerBoost );
        outText += "\nVPL Threshold [O]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mVplThreshold );

        outText += "\nNum Rays [H]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mNumRays );
        outText += "\nCluster size [J]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mCellSize );
        outText += "\nBias [K]: ";
        outText += Ogre::StringConverter::toString( mInstantRadiosity->mBias );

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

        if( arg.keysym.sym == SDLK_h )
        {
            const bool reverse = (arg.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT));
            if( reverse )
                mInstantRadiosity->mNumRays >>= 1u;
            else
                mInstantRadiosity->mNumRays <<= 1u;

            //Too many rays and the app will become unresponsive
            mInstantRadiosity->mNumRays = std::max<size_t>( mInstantRadiosity->mNumRays, 1u );
            mInstantRadiosity->mNumRays = std::min<size_t>( mInstantRadiosity->mNumRays, 32768u );

            mInstantRadiosity->build();
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
