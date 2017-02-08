
#include "Tutorial_TerrainGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"
#include "Utils/MeshUtils.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"

#include "OgreLwString.h"
#include "OgreGpuProgramManager.h"

#include "OgreItem.h"

using namespace Demo;

namespace Demo
{
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLockCameraToGround( false ),
        mTimeOfDay( Ogre::Math::PI * /*0.25f*//*0.55f*/0.1f ),
        mAzimuth( 0 ),
        mTerra( 0 ),
        mSunLight( 0 ),
        mHlmsPbsTerraShadows( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    Ogre::CompositorWorkspace* Tutorial_TerrainGameState::setupCompositor()
    {
        // The first time this function gets called Terra is not initialized. This is a very possible
        // scenario i.e. load a level without Terrain, but we still need a workspace to render.
        //
        // Thus we pass a PF_NULL texture to the workspace as a dud that barely consumes any
        // memory (it consumes no GPU memory btw) by specifying PF_NULL. Alternatively you
        // could use a different workspace (either defined in script or programmatically) that
        // doesn't require specifying a second external texture. But using a dud is just simpler.
        //
        // The second time we get called, Terra will be initialized and we can pass the
        // proper external texture filled with the UAV so Ogre can place the right
        // barriers.
        //
        // Note: We *could* delay the creation of the workspace in this sample until Terra
        // is initialized; instead of creating the workspace unnecessarily twice.
        // However we're doing this on purpose to show how to deal with perfectly valid &
        // very common scenarios.
        using namespace Ogre;

        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        RenderWindow *renderWindow = mGraphicsSystem->getRenderWindow();
        Camera *camera = mGraphicsSystem->getCamera();
        CompositorManager2 *compositorManager = root->getCompositorManager2();

        CompositorWorkspace *oldWorkspace = mGraphicsSystem->getCompositorWorkspace();
        if( oldWorkspace )
        {
            TexturePtr terraShadowTex = oldWorkspace->getExternalRenderTargets()[1].textures.back();
            if( terraShadowTex->getFormat() == PF_NULL )
            {
                ResourcePtr resourcePtr( terraShadowTex );
                TextureManager::getSingleton().remove( resourcePtr );
            }
            compositorManager->removeWorkspace( oldWorkspace );
        }

        CompositorChannelVec externalChannels( 2 );
        //Render window
        externalChannels[0].target = renderWindow;

        //Terra's Shadow texture
        ResourceLayoutMap initialLayouts;
        ResourceAccessMap initialUavAccess;
        if( mTerra )
        {
            //Terra is initialized
            const ShadowMapper *shadowMapper = mTerra->getShadowMapper();
            shadowMapper->fillUavDataForCompositorChannel( externalChannels[1], initialLayouts,
                                                           initialUavAccess );
        }
        else
        {
            //The texture is not available. Create a dummy dud using PF_NULL.
            TexturePtr nullTex = TextureManager::getSingleton().createManual(
                        "DummyNull", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        TEX_TYPE_2D, 1, 1, 0, PF_NULL );
            externalChannels[1].target = nullTex->getBuffer(0)->getRenderTarget();
            externalChannels[1].textures.push_back( nullTex );
        }

        return compositorManager->addWorkspace( sceneManager, externalChannels, camera,
                                                "Tutorial_TerrainWorkspace", true, -1,
                                                (UavBufferPackedVec*)0, &initialLayouts,
                                                &initialUavAccess );
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::createScene01(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                  &sceneManager->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                  sceneManager, 0, root->getCompositorManager2(),
                                  mGraphicsSystem->getCamera() );
        mTerra->setCastShadows( false );

        //mTerra->load( "Heightmap.png", Ogre::Vector3::ZERO, Ogre::Vector3( 256.0f, 1.0f, 256.0f ) );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 128.0f, 5.0f, 128.0f ) );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 1024.0f, 5.0f, 1024.0f ) );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 4096.0f * 4, 15.0f * 64.0f*4, 4096.0f * 4 ) );
        mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), Ogre::Vector3( 4096.0f, 4096.0f, 4096.0f ) );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), Ogre::Vector3( 14096.0f, 14096.0f, 14096.0f ) );

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC );
        Ogre::SceneNode *sceneNode = rootNode->createChildSceneNode( Ogre::SCENE_STATIC );
        sceneNode->attachObject( mTerra );

        Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
        Ogre::HlmsDatablock *datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
//        Ogre::HlmsDatablock *datablock = hlmsManager->getHlms( Ogre::HLMS_USER3 )->getDefaultDatablock();
//        Ogre::HlmsMacroblock macroblock;
//        macroblock.mPolygonMode = Ogre::PM_WIREFRAME;
        //datablock->setMacroblock( macroblock );
        mTerra->setDatablock( datablock );

        {
            mHlmsPbsTerraShadows = new Ogre::HlmsPbsTerraShadows();
            mHlmsPbsTerraShadows->setTerra( mTerra );
            //Set the PBS listener so regular objects also receive terrain shadows
            Ogre::Hlms *hlmsPbs = root->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
            hlmsPbs->setListener( mHlmsPbsTerraShadows );
        }

        mSunLight = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( mSunLight );
        mSunLight->setPowerScale( Ogre::Math::PI );
        mSunLight->setType( Ogre::Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        sceneManager->setAmbientLight( Ogre::ColourValue( 0.33f, 0.61f, 0.98f ) * 0.01f,
                                       Ogre::ColourValue( 0.02f, 0.53f, 0.96f ) * 0.01f,
                                       Ogre::Vector3::UNIT_Y );

        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 100000.0f );
        mGraphicsSystem->getCamera()->setPosition( -10.0f, 80.0f, 10.0f );


        MeshUtils::importV1Mesh( "tudorhouse.mesh",
                                 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        //Create some meshes to show off terrain shadows.
        Ogre::Item *item = sceneManager->createItem( "tudorhouse.mesh",
                                                     Ogre::ResourceGroupManager::
                                                     AUTODETECT_RESOURCE_GROUP_NAME,
                                                     Ogre::SCENE_STATIC );
        Ogre::Vector3 objPos( 3.5f, 4.5f, -2.0f );
        mTerra->getHeightAt( objPos );
        objPos.y += -Ogre::min( item->getLocalAabb().getMinimum().y, Ogre::Real(0.0f) ) * 0.01f - 0.5f;
        sceneNode = rootNode->createChildSceneNode( Ogre::SCENE_STATIC, objPos );
        sceneNode->scale( 0.01f, 0.01f, 0.01f );
        sceneNode->attachObject( item );

        item = sceneManager->createItem( "tudorhouse.mesh",
                                         Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                         Ogre::SCENE_STATIC );
        objPos = Ogre::Vector3( -3.5f, 4.5f, -2.0f );
        mTerra->getHeightAt( objPos );
        objPos.y += -Ogre::min( item->getLocalAabb().getMinimum().y, Ogre::Real(0.0f) ) * 0.01f - 0.5f;
        sceneNode = rootNode->createChildSceneNode( Ogre::SCENE_STATIC, objPos );
        sceneNode->scale( 0.01f, 0.01f, 0.01f );
        sceneNode->attachObject( item );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::destroyScene(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::Hlms *hlmsPbs = root->getHlmsManager()->getHlms( Ogre::HLMS_PBS );

        //Unset the PBS listener and destroy it
        if( hlmsPbs->getListener() == mHlmsPbsTerraShadows )
        {
            hlmsPbs->setListener( 0 );
            delete mHlmsPbsTerraShadows;
            mHlmsPbsTerraShadows = 0;
        }

        delete mTerra;
        mTerra = 0;

        TutorialGameState::destroyScene();
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::update( float timeSinceLast )
    {
        static float accumTime = 0;
        //mSunLight->setDirection( Ogre::Vector3( cosf( mTimeOfDay ), -sinf( mTimeOfDay ), -1.0 ).normalisedCopy() );
        //mSunLight->setDirection( Ogre::Vector3( 0, -sinf( mTimeOfDay ), -1.0 ).normalisedCopy() );
        mSunLight->setDirection( Ogre::Quaternion( Ogre::Radian(mAzimuth), Ogre::Vector3::UNIT_Y ) *
                                 Ogre::Vector3( cosf( mTimeOfDay ), -sinf( mTimeOfDay ), 0.0 ).normalisedCopy() );
        //mSunLight->setDirection( -Ogre::Vector3::UNIT_Y );

        //Force update the shadow map every frame to avoid the feeling we're "cheating" the
        //user in this sample with higher framerates than what he may encounter in many of
        //his possible uses.
        const float lightEpsilon = 0.0f;
        mTerra->update( mSunLight->getDerivedDirectionUpdated(), lightEpsilon );

        TutorialGameState::update( timeSinceLast );

        //Camera must be locked to ground *after* we've moved it. Otherwise
        //fast motion may go below the terrain for 1 or 2 frames.
        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        Ogre::Vector3 camPos = camera->getPosition();
        if( mLockCameraToGround && mTerra->getHeightAt( camPos ) )
            camera->setPosition( camPos + Ogre::Vector3::UNIT_Y * 10.0f );
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        if( mDisplayHelpMode == 0 )
        {
            outText += "\nCtrl+F4 will reload Terra's shaders.";
        }
        else if( mDisplayHelpMode == 1 )
        {
            char tmp[128];
            Ogre::LwString str( Ogre::LwString::FromEmptyPointer(tmp, sizeof(tmp)) );
            Ogre::Vector3 camPos = mGraphicsSystem->getCamera()->getPosition();

            using namespace Ogre;

            outText += "\nF2 Lock Camera to Ground: [";
            outText += mLockCameraToGround ? "Yes]" : "No]";
            outText += "\n+/- to change time of day. [";
            outText += StringConverter::toString( mTimeOfDay * 180.0f / Math::PI ) + "]";
            outText += "\n9/6 to change azimuth. [";
            outText += StringConverter::toString( mAzimuth * 180.0f / Math::PI ) + "]";
            outText += "\n\nCamera: ";
            str.a( "[", LwString::Float( camPos.x, 2, 2 ), ", ",
                        LwString::Float( camPos.y, 2, 2 ), ", ",
                        LwString::Float( camPos.z, 2, 2 ), "]" );
            outText += str.c_str();
            outText += "\nLightDir: ";
            str.clear();
            str.a( "[", LwString::Float( mSunLight->getDirection().x, 2, 2 ), ", ",
                        LwString::Float( mSunLight->getDirection().y, 2, 2 ), ", ",
                        LwString::Float( mSunLight->getDirection().z, 2, 2 ), "]" );
            outText += str.c_str();
        }
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_F4 && (arg.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) )
        {
            //Hot reload of Terra shaders.
            Ogre::Root *root = mGraphicsSystem->getRoot();
            Ogre::HlmsManager *hlmsManager = root->getHlmsManager();

            Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_USER3 );
            Ogre::GpuProgramManager::getSingleton().clearMicrocodeCache();
            hlms->reloadFrom( hlms->getDataFolder() );
        }
        else if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.scancode == SDL_SCANCODE_KP_PLUS )
        {
            mTimeOfDay += 0.1f;
            mTimeOfDay = Ogre::min( mTimeOfDay, (float)Ogre::Math::PI );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_MINUS ||
                 arg.keysym.scancode == SDL_SCANCODE_KP_MINUS )
        {
            mTimeOfDay -= 0.1f;
            mTimeOfDay = Ogre::max( mTimeOfDay, 0 );
        }

        if( arg.keysym.scancode == SDL_SCANCODE_KP_9 )
        {
            mAzimuth += 0.1f;
            mAzimuth = fmodf( mAzimuth, Ogre::Math::TWO_PI );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_KP_6 )
        {
            mAzimuth -= 0.1f;
            mAzimuth = fmodf( mAzimuth, Ogre::Math::TWO_PI );
            if( mAzimuth < 0 )
                mAzimuth = Ogre::Math::TWO_PI + mAzimuth;
        }

        if( arg.keysym.scancode == SDL_SCANCODE_F2 )
        {
            mLockCameraToGround = !mLockCameraToGround;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
