
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
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreHlmsPbs.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

#include "OgreWireAabb.h"

using namespace Demo;

namespace Demo
{
    const Ogre::uint32 c_areaLightsPoolId = 759384;

    AreaApproxLightsGameState::AreaApproxLightsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createAreaMask(void)
    {
        const Ogre::uint32 texWidth = 256u;
        const Ogre::uint32 texHeight = 256u;
        const Ogre::PixelFormat texFormat = Ogre::PF_L8;

        //Fill the texture with a hollow rectangle, 10-pixel thick.
        size_t sizeBytes = Ogre::PixelUtil::calculateSizeBytes(
                               texWidth, texHeight, 1u, 1u, texFormat, 1u );
        Ogre::uint8 *data = reinterpret_cast<Ogre::uint8*>(
                                OGRE_MALLOC( sizeBytes, Ogre::MEMCATEGORY_GENERAL ) );
        Ogre::Image image;
        image.loadDynamicImage( data, texWidth, texHeight, 1u, texFormat, true, 1u, 0u );
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
        image.generateMipmaps( false, Ogre::Image::FILTER_GAUSSIAN_HIGH );

        {
            //Ensure the lower mips have black borders. This is done to prevent certain artifacts,
            //Ensure the higher mips have grey borders. This is done to prevent certain artifacts.
            data = image.getData();
            for( size_t i=0u; i<image.getNumMipmaps() + 1u; ++i )
            {
                const Ogre::uint8 borderColour = i >= 5u ? 127u : 0u;
                Ogre::uint32 currWidth = std::max( texWidth >> i, 1u );
                Ogre::uint32 currHeight = std::max( texHeight >> i, 1u );

                size_t bytesPerRow = Ogre::PixelUtil::getMemorySize( currWidth, 1u, 1u, texFormat );

                memset( data, borderColour, bytesPerRow );
                memset( data + bytesPerRow * (currHeight - 1u), borderColour, bytesPerRow );

                for( size_t y=1; y<currWidth - 1u; ++y )
                {
                    data[y*bytesPerRow+0] = borderColour;
                    data[y*bytesPerRow+bytesPerRow-1u] = borderColour;
                }

                data += bytesPerRow * currHeight;
            }
        }

        //Please note the texture CAN be coloured. The sample uses a monochrome texture,
        //but you coloured textures are supported too. However they will burn a little
        //more GPU performance.
        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
        mAreaMaskTex = hlmsTextureManager->reservePoolId(
                           c_areaLightsPoolId, Ogre::HlmsTextureManager::TEXTURE_TYPE_MONOCHROME,
                           image.getWidth(), image.getHeight(), 8u,
                           image.getNumMipmaps(), image.getFormat(),
                           false, false );

        //Create the texture now, from the Image
        hlmsTextureManager->createOrRetrieveTexture(
                    "AreaLightMask0", "AreaLightMask0",
                    Ogre::HlmsTextureManager::TEXTURE_TYPE_MONOCHROME, c_areaLightsPoolId, &image );

        //Set the texture mask to PBS.
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::Hlms *hlms = root->getHlmsManager()->getHlms( Ogre::HLMS_PBS );
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlms ) );
        Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>( hlms );

        pbs->setAreaLightMasks( mAreaMaskTex );
        pbs->setAreaLightForwardSettings( 2u, 2u );
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createAreaPlaneMesh(void)
    {
        Ogre::v1::MeshPtr lightPlaneMeshV1 =
                Ogre::v1::MeshManager::getSingleton().createPlane( "LightPlane v1",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Z, 0.0f ), 1.0f, 1.0f,
                                            1, 1, true, 1, 1.0f, 1.0f, Ogre::Vector3::UNIT_Y,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );
        Ogre::MeshPtr lightPlaneMesh = Ogre::MeshManager::getSingleton().createManual(
                    "LightPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
        lightPlaneMesh->importV1( lightPlaneMeshV1.get(), true, true, true );

        Ogre::v1::MeshManager::getSingleton().remove( lightPlaneMeshV1 );
    }
    //-----------------------------------------------------------------------------------
    Ogre::HlmsUnlitDatablock* AreaApproxLightsGameState::createPlaneForAreaLight( Ogre::Light *light )
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
                                          Ogre::StringConverter::toString( s_materialCounter++ );
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
            datablock->setTextureSwizzle( 0, Ogre::HlmsUnlitDatablock::R_MASK,
                                          Ogre::HlmsUnlitDatablock::R_MASK,
                                          Ogre::HlmsUnlitDatablock::R_MASK,
                                          Ogre::HlmsUnlitDatablock::R_MASK );
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
        const Ogre::Vector2 rectSize = light->getRectSize();
        planeNode->setScale( rectSize.x, rectSize.y, 1.0f );

        /* For debugging ranges & AABBs
        Ogre::WireAabb *wireAabb = sceneManager->createWireAabb();
        wireAabb->track( light );*/

        return datablock;
    }
    //-----------------------------------------------------------------------------------
    void AreaApproxLightsGameState::createScene01(void)
    {
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
            Ogre::Item *item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
            item->setDatablock( "Marble" );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( item );

            //Change the addressing mode of the roughness map to wrap via code.
            //Detail maps default to wrap, but the rest to clamp.
            assert( dynamic_cast<Ogre::HlmsPbsDatablock*>( item->getSubItem(0)->getDatablock() ) );
            Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                                                            item->getSubItem(0)->getDatablock() );
            //Make a hard copy of the sampler block
            Ogre::HlmsSamplerblock samplerblock( *datablock->getSamplerblock( Ogre::PBSM_ROUGHNESS ) );
            samplerblock.mU = Ogre::TAM_WRAP;
            samplerblock.mV = Ogre::TAM_WRAP;
            samplerblock.mW = Ogre::TAM_WRAP;
            //Set the new samplerblock. The Hlms system will
            //automatically create the API block if necessary
            datablock->setSamplerblock( Ogre::PBSM_ROUGHNESS, samplerblock );
        }

        for( int i=0; i<4; ++i )
        {
            for( int j=0; j<4; ++j )
            {
                Ogre::String meshName;

                if( i == j )
                    meshName = "Sphere1000.mesh";
                else
                    meshName = "Cube_d.mesh";

                Ogre::Item *item = sceneManager->createItem( meshName,
                                                             Ogre::ResourceGroupManager::
                                                             AUTODETECT_RESOURCE_GROUP_NAME,
                                                             Ogre::SCENE_DYNAMIC );
                if( i % 2 == 0 )
                    item->setDatablock( "Rocks" );
                else
                    item->setDatablock( "Marble" );

                item->setVisibilityFlags( 0x000000001 );

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

        {
            size_t numSpheres = 0;
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );

            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

            const int numX = 8;
            const int numZ = 8;

            const float armsLength = 1.0f;
            const float startX = (numX-1) / 2.0f;
            const float startZ = (numZ-1) / 2.0f;

            for( int x=0; x<numX; ++x )
            {
                for( int z=0; z<numZ; ++z )
                {
                    Ogre::String datablockName = "Test" + Ogre::StringConverter::toString( numSpheres++ );
                    Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                                hlmsPbs->createDatablock( datablockName,
                                                          datablockName,
                                                          Ogre::HlmsMacroblock(),
                                                          Ogre::HlmsBlendblock(),
                                                          Ogre::HlmsParamVec() ) );

                    Ogre::HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                            createOrRetrieveTexture( "SaintPetersBasilica.dds",
                                                     Ogre::HlmsTextureManager::TEXTURE_TYPE_ENV_MAP );

                    datablock->setTexture( Ogre::PBSM_REFLECTION, texLocation.xIdx, texLocation.texture );
                    datablock->setDiffuse( Ogre::Vector3( 0.0f, 1.0f, 0.0f ) );

                    datablock->setRoughness( std::max( 0.02f, x / Ogre::max( 1, (float)(numX-1) ) ) );
                    datablock->setFresnel( Ogre::Vector3( z / Ogre::max( 1, (float)(numZ-1) ) ), false );

                    Ogre::Item *item = sceneManager->createItem( "Sphere1000.mesh",
                                                                 Ogre::ResourceGroupManager::
                                                                 AUTODETECT_RESOURCE_GROUP_NAME,
                                                                 Ogre::SCENE_DYNAMIC );
                    item->setDatablock( datablock );
                    item->setVisibilityFlags( 0x000000002 );

                    Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                            createChildSceneNode( Ogre::SCENE_DYNAMIC );
                    sceneNode->setPosition( Ogre::Vector3( armsLength * x - startX,
                                                           1.0f,
                                                           armsLength * z - startZ ) );
                    sceneNode->attachObject( item );
                }
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

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
        //Increase the strength 10x to showcase this light. Area approx lights are not
        //physically based so the value is more arbitrary than the other light types
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_AREA_APPROX );
        light->setRectSize( Ogre::Vector2( 15.0f, 15.0f ) );
        lightNode->setPosition( -10.0f, 6.0f, 10.0f );
        //light->setDirection( Ogre::Vector3( 1, 0, 0 ).normalisedCopy() );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        //light->setDirection( Ogre::Vector3( 0, -1, 0 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        //Set the texture for this area light. The parameters to createOrRetrieveTexture
        //do not matter much, as the texture has already been created.
        Ogre::HlmsTextureManager::TextureLocation texLocation =
                hlmsTextureManager->createOrRetrieveTexture(
                    "AreaLightMask0", "AreaLightMask0",
                    Ogre::HlmsTextureManager::TEXTURE_TYPE_MONOCHROME, c_areaLightsPoolId );
        light->setTexture( texLocation.texture, texLocation.xIdx );
        //Control the diffuse mip (this is the default value)
        light->mTexLightMaskDiffuseMipStart = (Ogre::uint16)(0.95f * 65535);

        mAreaLightPlaneDatablocks[0] = createPlaneForAreaLight( light );

        mAreaLights[0] = light;
        mLightNodes[1] = lightNode;

        light = sceneManager->createLight();
        lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_AREA_APPROX );
        light->setRectSize( Ogre::Vector2( 5.0f, 5.0f ) );
        lightNode->setPosition( 5.0f, 4.0f, -5.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );
        //When the array index is 0xFFFF, the light won't use a texture (default value).
        light->mTextureLightMaskIdx = std::numeric_limits<Ogre::uint16>::max();

        mAreaLightPlaneDatablocks[1] = createPlaneForAreaLight( light );

        mAreaLights[1] = light;
        mLightNodes[2] = lightNode;

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
        outText += "\nPress F3 to toggle PBR LTC. ";
        outText += mAreaLights[0]->getType() == Ogre::Light::LT_AREA_APPROX ?
                    "[Approximation (Textured)]" : "[LTC (No texture)]";
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
        else if( arg.keysym.sym == SDLK_F3 )
        {
            for( int i=0; i<2; ++i )
            {
                mAreaLights[i]->setType( mAreaLights[i]->getType() == Ogre::Light::LT_AREA_APPROX ?
                                             Ogre::Light::LT_AREA_LTC : Ogre::Light::LT_AREA_APPROX );

                if( mAreaLights[i]->mTextureLightMaskIdx != std::numeric_limits<Ogre::uint16>::max() )
                {
                    Ogre::HlmsUnlitDatablock *datablock = mAreaLightPlaneDatablocks[i];
                    if( mAreaLights[i]->getType() == Ogre::Light::LT_AREA_APPROX )
                        datablock->setTexture( 0, mAreaLights[i]->mTextureLightMaskIdx, mAreaMaskTex );
                    else
                        datablock->setTexture( 0, 0, Ogre::TexturePtr() );
                }
            }
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
