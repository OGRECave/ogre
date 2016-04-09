
#include "Tutorial_TerrainGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "Terra/Terra.h"
#include "Terra/TerraShadowMapper.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "Compositor/OgreCompositorManager2.h"

#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"

using namespace Demo;

namespace Demo
{
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mTimeOfDay( Ogre::Math::PI * /*0.25f*/0.55f ),
        mAzimuth( 0 ),
        mTerra( 0 ),
        mSunLight( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    Ogre::CompositorWorkspace* Tutorial_TerrainGameState::setupCompositor()
    {
        using namespace Ogre;

        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        RenderWindow *renderWindow = mGraphicsSystem->getRenderWindow();
        Camera *camera = mGraphicsSystem->getCamera();
        CompositorManager2 *compositorManager = root->getCompositorManager2();

        CompositorChannelVec externalChannels( 2 );
        //Render window
        externalChannels[0].target = renderWindow;

        //Terra's Shadow texture
        ResourceLayoutMap initialLayouts;
        ResourceAccessMap initialUavAccess;
        if( mTerra )
        {
            const ShadowMapper *shadowMapper = mTerra->getShadowMapper();
            shadowMapper->fillUavDataForCompositorChannel( externalChannels[1], initialLayouts,
                                                           initialUavAccess );
        }
        else
        {
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
        mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 4096.0f * 4, 15.0f * 64.0f*4, 4096.0f * 4 ) );

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

        mSunLight = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( mSunLight );
        mSunLight->setPowerScale( Ogre::Math::PI );
        mSunLight->setType( Ogre::Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 100000.0f );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::destroyScene(void)
    {
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

        mTerra->update( mSunLight->getDerivedDirectionUpdated() );

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\n+/- to change time of day. [";
        outText += Ogre::StringConverter::toString( mTimeOfDay * 180.0f / Ogre::Math::PI ) + "]";
        outText += "\n9/6 to change azimuth. [";
        outText += Ogre::StringConverter::toString( mAzimuth * 180.0f / Ogre::Math::PI ) + "]";
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.scancode == SDL_SCANCODE_KP_PLUS )
        {
            mTimeOfDay += 0.1f;
            mTimeOfDay = Ogre::min( mTimeOfDay, Ogre::Math::PI );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_MINUS ||
                 arg.keysym.scancode == SDL_SCANCODE_KP_MINUS )
        {
            mTimeOfDay -= 0.1f;
            mTimeOfDay = Ogre::max( mTimeOfDay, 0 );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_KP_9 )
        {
            mAzimuth += 0.1f;
            mAzimuth = fmodf( mAzimuth, Ogre::Math::TWO_PI );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_KP_6 )
        {
            mAzimuth -= 0.1f;
            mAzimuth = fmodf( mAzimuth, Ogre::Math::TWO_PI );
            if( mAzimuth < 0 )
                mAzimuth = Ogre::Math::TWO_PI - mAzimuth;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
