
#include "AreaApproxLightsGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"
#include "OgreSubMesh2.h"

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

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

using namespace Demo;

namespace Demo
{
    AreaApproxLightsGameState::AreaApproxLightsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createAreaMask(void)
    {
        mAreaMaskTex = Ogre::TextureManager::getSingleton().createManual(
                           "Area Light Masks", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                           Ogre::TEX_TYPE_2D_ARRAY, 256u, 256u, Ogre::MIP_UNLIMITED,
                           Ogre::PF_L8, Ogre::TU_STATIC_WRITE_ONLY );

        Ogre::uint32 texWidth = mAreaMaskTex->getWidth();
        Ogre::uint32 texHeight = mAreaMaskTex->getHeight();
        const Ogre::PixelFormat texFormat = mAreaMaskTex->getFormat();

        //Fill the texture with a hollow rectangle, 10-pixel thick.
        size_t sizeBytes = Ogre::PixelUtil::calculateSizeBytes(
                               texWidth, texHeight, 1u, 1u,
                               texFormat, mAreaMaskTex->getNumMipmaps() );
        Ogre::uint8 *data = reinterpret_cast<Ogre::uint8*>(
                                OGRE_MALLOC( sizeBytes, Ogre::MEMCATEGORY_GENERAL ) );
        Ogre::Image image;
        image.loadDynamicImage( data, texWidth, texHeight, 1u,
                                texFormat, true, 1u, mAreaMaskTex->getNumMipmaps() );
        for( size_t y=0; y<texHeight; ++y )
        {
            for( size_t x=0; x<texWidth; ++x )
            {
                if( (y >= 10 && y <= texHeight - 11) &&
                    (x >= 10 && x <= texWidth - 11)  &&
                    ( (y >= 10 && y <= 20) || (x >= 10 && x <= 20) ||
                      (y >= texHeight - 21 && y <= texHeight - 11) ||
                      (x >= texWidth - 21 && x <= texWidth - 11) ) )
                {
                    *data++ = 255;
                }
                else
                    *data++ = 0;
            }
        }

        //Generate the mipmaps so roughness works
        image.generateMipmaps( false, Ogre::Image::FILTER_GAUSSIAN );

        //Upload to GPU
        Ogre::uint8 const *srcData = image.getData();
        for( size_t i=0; i<mAreaMaskTex->getNumMipmaps() + 1u; ++i )
        {
            Ogre::uint32 currWidth = std::max( texWidth >> i, 1u );
            Ogre::uint32 currHeight = std::max( texHeight >> i, 1u );
            Ogre::v1::HardwarePixelBufferSharedPtr pixelBufferBuf = mAreaMaskTex->getBuffer( 0, i );
            const Ogre::PixelBox &currImage =
                    pixelBufferBuf->lock( Ogre::Box( 0, 0, 0, currWidth, currHeight, 1 ),
                                          Ogre::v1::HardwareBuffer::HBL_DISCARD );
            Ogre::uint8 *dstData = reinterpret_cast<Ogre::uint8*>( currImage.data );
            for( size_t y=0; y<currHeight; ++y )
            {
                memcpy( dstData, srcData,
                        Ogre::PixelUtil::getMemorySize( currWidth, 1u, 1u, texFormat ) );
                srcData += Ogre::PixelUtil::getMemorySize( currWidth, 1u, 1u, texFormat );
                dstData += Ogre::PixelUtil::getMemorySize( currImage.rowPitch, 1u, 1u, texFormat );
            }
            pixelBufferBuf->unlock();
        }

        //Set the texture mask to PBS.
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::Hlms *hlms = root->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

        pbs->setAreaLightMasks( mAreaMaskTex );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createAreaPlaneMesh(void)
    {
        Ogre::v1::MeshPtr lightPlaneMeshV1 =
                Ogre::v1::MeshManager::getSingleton().createPlane( "LightPlane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Z, 0.0f ), 0.5f, 0.5f,
                                            1, 1, true, 1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );
        Ogre::MeshPtr lightPlaneMesh = Ogre::MeshManager::getSingleton().createManual(
                    "LightPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
        lightPlaneMesh->importV1( lightPlaneMeshV1.get(), true, true, true );

        Ogre::v1::MeshManager::getSingleton().remove( lightPlaneMeshV1 );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createPlaneForAreaLight( Ogre::Light *light )
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::Hlms *hlmsUnlit = root->getHlmsManager()->getHlms( Ogre::HLMS_UNLIT );

        //Setup an unlit material, double-sided, with textures
        //(if it has one) and same colour as the light.
        //IMPORTANT: these materials are never destroyed once they're not needed (they will
        //be destroyed by Ogre on shutdown). Watchout for this to prevent memory leaks in
        //a real implementation
        static Ogre::uint32 s_materialCounter = 0;
        const Ogre::String materialName = "LightPlane Material" +
                                          Ogre::StringConverter::toString( s_materialCounter );
        Ogre::HlmsMacroblock macroblock;
        macroblock.mCullMode = Ogre::CULL_NONE;
        Ogre::HlmsDatablock *datablockBase =
                hlmsUnlit->createDatablock( materialName, materialName, macroblock,
                                            Ogre::HlmsBlendblock(), Ogre::HlmsParamVec() );

        assert( dynamic_cast<Ogre::HlmsUnlitDatablock*>( datablockBase ) );
        Ogre::HlmsUnlitDatablock *datablock = static_cast<Ogre::HlmsUnlitDatablock*>( datablockBase );

        if( light->mTextureLightMaskIdx != std::numeric_limits<Ogre::uint16>::max() )
        {
            Ogre::HlmsSamplerblock samplerblock;
            samplerblock.mMaxAnisotropy = 8.0f;
            samplerblock.setFiltering( Ogre::TFO_ANISOTROPIC );

            datablock->setTexture( 0, light->mTextureLightMaskIdx, mAreaMaskTex, &samplerblock );
        }

        datablock->setUseColour( true );
        datablock->setColour( light->getDiffuseColour() );

        //Create the plane Item
        Ogre::SceneNode *lightNode = light->getParentSceneNode();
        Ogre::SceneNode *planeNode = lightNode->createChildSceneNode();

        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        Ogre::Item *item = sceneManager->createItem(
                               "LightPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
        item->setCastShadows( false );
        item->setDatablock( datablock );
        planeNode->attachObject( item );

        //Math the plane size to that of the area light
        const Ogre::Vector2 rectSize = light->getRectHalfSize();
        planeNode->setScale( rectSize.x, rectSize.y, 1.0f );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createScene01(void)
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

        createAreaMask();
        createAreaPlaneMesh();

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
        light->setType( Ogre::Light::LT_AREA_APPROX );
		light->setRectHalfSize( Ogre::Vector2( 15.0f, 15.0f ) );
        lightNode->setPosition( -10.0f, 10.0f, 10.0f );
        //light->setDirection( Ogre::Vector3( 1, 0, 0 ).normalisedCopy() );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        //light->setDirection( Ogre::Vector3( 0, -1, 0 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        //Set the array index of the light mask in mAreaMaskTex
        light->mTextureLightMaskIdx = 0u;
        //5.0f is an arbitrary value. It looks good for meters.
        light->mTextureMaskMipScale = 5.0f;

        createPlaneForAreaLight( light );

        mLightNodes[1] = lightNode;

        /*light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_SPOTLIGHT );
        lightNode->setPosition( 10.0f, 10.0f, -10.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        mLightNodes[2] = lightNode;*/

        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::destroyScene(void)
    {
        mAreaMaskTex.reset();
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );

            mLightNodes[1]->roll( Ogre::Radian( timeSinceLast ) );
            mLightNodes[1]->yaw( Ogre::Radian( timeSinceLast ) );
        }

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::keyReleased( const SDL_KeyboardEvent &arg )
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
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
