
#include "DecalsGameState.h"
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

#include "OgreDecal.h"
#include "OgreTextureManager.h"
#include "OgreWireAabb.h"

using namespace Demo;

namespace Demo
{
    Ogre::SceneNode *g_decalNode = 0;

    DecalsGameState::DecalsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true ),
        mNumSpheres( 0 ),
        mTransparencyMode( Ogre::HlmsPbsDatablock::None ),
        mTransparencyValue( 1.0f ),
        mDecalDebugVisual( 0 )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::createDecalDebugData(void)
    {
        Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane( "DebugDecalPlane",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 0.0f ), 2.0f, 2.0f,
                                            1, 1, false, 1, 4.0f, 4.0f, Ogre::Vector3::UNIT_X,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );

        Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createManual(
                    "DebugDecalPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        planeMesh->importV1( planeMeshV1.get(), true, true, false );
        Ogre::v1::MeshManager::getSingleton().remove( planeMeshV1 );

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        Ogre::Hlms *hlmsUnlit = hlmsManager->getHlms( Ogre::HLMS_UNLIT );

        Ogre::HlmsMacroblock macroblock;
        macroblock.mDepthWrite = false;
        macroblock.mCullMode = Ogre::CULL_NONE;
        Ogre::HlmsBlendblock blendblock;
        blendblock.setBlendType( Ogre::SBT_ADD );

        Ogre::HlmsParamVec params;
        params.push_back( std::pair<Ogre::IdString, Ogre::String>( "diffuse", "0.2 0.0 0.0" ) );
        hlmsUnlit->createDatablock( "DebugDecalMat", "DebugDecalMat", macroblock, blendblock, params );

        params.clear();
        params.push_back( std::pair<Ogre::IdString, Ogre::String>( "diffuse", "0.15 0.2 0.0" ) );
        hlmsUnlit->createDatablock( "DebugDecalMatPlane", "DebugDecalMatPlane", macroblock, blendblock, params );
    }
    //-----------------------------------------------------------------------------------
    DebugDecalVisual* DecalsGameState::attachDecalDebugHelper( Ogre::SceneNode *decalNode )
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::Item *plane = sceneManager->createItem( "DebugDecalPlane" );
        Ogre::Item *cube = sceneManager->createItem( "Cube_d.mesh" );

        cube->setDatablockOrMaterialName( "DebugDecalMat" );
        plane->setDatablockOrMaterialName( "DebugDecalMatPlane" );

        Ogre::SceneNode *sceneNode = decalNode->createChildSceneNode(
                                         decalNode->isStatic() ? Ogre::SCENE_STATIC : Ogre::SCENE_DYNAMIC );
        sceneNode->attachObject( plane );
        sceneNode->attachObject( cube );
        sceneNode->setScale( Ogre::Vector3( 0.5f ) );

        DebugDecalVisual *retVal = new DebugDecalVisual();
        retVal->plane       = plane;
        retVal->cube        = cube;
        retVal->sceneNode   = sceneNode;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::destroyDecalDebugHelper( DebugDecalVisual *decalDebugVisual )
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        sceneManager->destroyItem( decalDebugVisual->cube );
        sceneManager->destroyItem( decalDebugVisual->plane );
        decalDebugVisual->sceneNode->getParentSceneNode()->removeAndDestroyChild(
                    decalDebugVisual->sceneNode );

        delete decalDebugVisual;
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        createDecalDebugData();

        sceneManager->setForwardClustered( true, 16, 8, 24, 96, 4, 2, 50 );
        //sceneManager->setForwardClustered( true, 128, 64, 8, 96, 4, 2, 50 );

        {
            const Ogre::uint32 decalDiffuseId = 1;
            const Ogre::uint32 decalNormalId = 1;
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

            Ogre::WireAabb *wireAabb = sceneManager->createWireAabb();

            Ogre::Decal *decal = sceneManager->createDecal();
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode()->createChildSceneNode();
            sceneNode->attachObject( decal );
            sceneNode->setPosition( Ogre::Vector3( 0, 0.4, 0 ) );
            sceneNode->setOrientation( Ogre::Quaternion( Ogre::Degree( 45.0f ), Ogre::Vector3::UNIT_Y ) );
            sceneNode->setScale( Ogre::Vector3( 10.0f ) );
            wireAabb->track( decal );

            Ogre::HlmsTextureManager::TextureLocation texLocation;
            texLocation = hlmsTextureManager->createOrRetrieveTexture( "floor_diffuse.png",
                                                                       "floor_diffuse.PNG",
                                                                       Ogre::HlmsTextureManager::
                                                                       TEXTURE_TYPE_DIFFUSE,
                                                                       decalDiffuseId );
            decal->setDiffuseTexture( texLocation.texture, texLocation.xIdx );
            sceneManager->setDecalsDiffuse( texLocation.texture );

            texLocation = hlmsTextureManager->createOrRetrieveTexture( "floor_bump.png",
                                                                       "floor_bump.PNG",
                                                                       Ogre::HlmsTextureManager::
                                                                       TEXTURE_TYPE_NORMALS,
                                                                       decalNormalId );
            decal->setNormalTexture( texLocation.texture, texLocation.xIdx );
            sceneManager->setDecalsNormals( texLocation.texture );

            g_decalNode = sceneNode;
            /*Ogre::Light *light = sceneManager->createLight();
            Ogre::SceneNode *sceneNode = sceneManager->createSceneNode();
            sceneNode->attachObject( light );
            sceneNode->setPosition( Ogre::Vector3( 0, 0.4, 0 ) );
            light->setType( Ogre::Light::LT_POINT );
            light->setAttenuation( 1.0f, 1.0, 0, 0 );
            light->setDiffuseColour( 20, 20, 20 );
            light->setCastShadows( false );
            wireAabb->track( light );*/
            g_decalNode = sceneNode;

            mDecalDebugVisual = attachDecalDebugHelper( sceneNode );
            mDecalDebugVisual->sceneNode->setVisible( false );
        }

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
            mNumSpheres = 0;
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
                    Ogre::String datablockName = "Test" + Ogre::StringConverter::toString( mNumSpheres++ );
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

        Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
        visibilityMask &= ~0x00000002u;
        visibilityMask &= ~0x00000001u;
        mGraphicsSystem->getSceneManager()->setVisibilityMask( visibilityMask );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::destroyScene(void)
    {
        if( mDecalDebugVisual )
        {
            destroyDecalDebugHelper( mDecalDebugVisual );
            mDecalDebugVisual = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::update( float timeSinceLast )
    {
        if( mAnimateObjects )
        {
            for( int i=0; i<16; ++i )
                mSceneNode[i]->yaw( Ogre::Radian(timeSinceLast * i * 0.125f) );
        }

        //g_decalNode->yaw( Ogre::Radian(timeSinceLast * 2 * 0.125f) );

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();

        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
        outText += "\nPress F3 to show/hide animated objects. ";
        outText += (visibilityMask & 0x000000001) ? "[On]" : "[Off]";
        outText += "\nPress F4 to show/hide palette of spheres. ";
        outText += (visibilityMask & 0x000000002) ? "[On]" : "[Off]";
        outText += "\nPress F5 to toggle transparency mode. ";
        outText += mTransparencyMode == Ogre::HlmsPbsDatablock::Fade ? "[Fade]" : "[Transparent]";
        outText += "\n+/- to change transparency. [";
        outText += Ogre::StringConverter::toString( mTransparencyValue ) + "]";
        outText += "\nPress F6 to show/hide Decal's debug visualization. ";
        outText += mDecalDebugVisual->cube->isVisible() ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::setTransparencyToMaterials(void)
    {
        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();

        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );

        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

        Ogre::HlmsPbsDatablock::TransparencyModes mode =
                static_cast<Ogre::HlmsPbsDatablock::TransparencyModes>( mTransparencyMode );

        if( mTransparencyValue >= 1.0f )
            mode = Ogre::HlmsPbsDatablock::None;

        if( mTransparencyMode < 1.0f && mode == Ogre::HlmsPbsDatablock::None )
            mode = Ogre::HlmsPbsDatablock::Transparent;

        for( size_t i=0; i<mNumSpheres; ++i )
        {
            Ogre::String datablockName = "Test" + Ogre::StringConverter::toString( i );
            Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->getDatablock( datablockName ) );

            datablock->setTransparency( mTransparencyValue, mode );
        }
    }
    //-----------------------------------------------------------------------------------
    void DecalsGameState::keyReleased( const SDL_KeyboardEvent &arg )
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
            Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
            bool showMovingObjects = (visibilityMask & 0x00000001);
            showMovingObjects = !showMovingObjects;
            visibilityMask &= ~0x00000001;
            visibilityMask |= (Ogre::uint32)showMovingObjects;
            mGraphicsSystem->getSceneManager()->setVisibilityMask( visibilityMask );
        }
        else if( arg.keysym.sym == SDLK_F4 )
        {
            Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();
            bool showPalette = (visibilityMask & 0x00000002) != 0;
            showPalette = !showPalette;
            visibilityMask &= ~0x00000002;
            visibilityMask |= (Ogre::uint32)(showPalette) << 1;
            mGraphicsSystem->getSceneManager()->setVisibilityMask( visibilityMask );
        }
        else if( arg.keysym.sym == SDLK_F5 )
        {
            mTransparencyMode = mTransparencyMode == Ogre::HlmsPbsDatablock::Fade ?
                                                            Ogre::HlmsPbsDatablock::Transparent :
                                                            Ogre::HlmsPbsDatablock::Fade;
            if( mTransparencyValue != 1.0f )
                setTransparencyToMaterials();
        }
        else if( arg.keysym.sym == SDLK_F6 )
        {
            mDecalDebugVisual->sceneNode->setVisible( !mDecalDebugVisual->cube->isVisible() );
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_KP_PLUS )
        {
            if( mTransparencyValue < 1.0f )
            {
                mTransparencyValue += 0.1f;
                mTransparencyValue = Ogre::min( mTransparencyValue, 1.0f );
                setTransparencyToMaterials();
            }
        }
        else if( arg.keysym.scancode == SDL_SCANCODE_MINUS ||
                 arg.keysym.scancode == SDL_SCANCODE_KP_MINUS )
        {
            if( mTransparencyValue > 0.0f )
            {
                mTransparencyValue -= 0.1f;
                mTransparencyValue = Ogre::max( mTransparencyValue, 0.0f );
                setTransparencyToMaterials();
            }
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
