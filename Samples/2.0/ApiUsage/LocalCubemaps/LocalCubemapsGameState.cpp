
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

#include "Cubemaps/OgreParallaxCorrectedCubemap.h"

#include "LocalCubemapScene.h"

using namespace Demo;

namespace Demo
{
    LocalCubemapsGameState::LocalCubemapsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mAnimateObjects( true ),
        mCubeCamera( 0 ),
        mLocalCubemapsWorkspace( 0 ),
        mParallaxCorrectedCubemap( 0 )
    {
        memset( mSceneNode, 0, sizeof(mSceneNode) );
    }
    //-----------------------------------------------------------------------------------
    Ogre::CompositorWorkspace* LocalCubemapsGameState::setupCompositor()
    {
        // We first create the Cubemap workspace and pass it to the final workspace
        // that does the real rendering.
        //
        // If in your application you need to create a workspace but don't have a cubemap yet,
        // you can either programatically modify the workspace definition (which is cumbersome)
        // or just pass a PF_NULL texture that works as a dud and barely consumes any memory.
        // See Tutorial_Terrain for an example of PF_NULL dud.
        using namespace Ogre;

        Root *root = mGraphicsSystem->getRoot();
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        RenderWindow *renderWindow = mGraphicsSystem->getRenderWindow();
        Camera *camera = mGraphicsSystem->getCamera();
        CompositorManager2 *compositorManager = root->getCompositorManager2();

        //A RenderTarget created with TU_AUTOMIPMAP means the compositor still needs to
        //explicitly generate the mipmaps by calling generate_mipmaps. It's just an API
        //hint to tell the GPU we will be using the mipmaps auto generation routines.
        mLocalCubemaps = TextureManager::getSingleton().createManual(
                    "LocalCubemaps", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_CUBE_MAP, 1024, 1024,
                    PixelUtil::getMaxMipmapCount( 1024, 1024, 1 ),
                    PF_A8B8G8R8, TU_RENDERTARGET|TU_AUTOMIPMAP, 0, true );

        // Create the camera used to render to our cubemap
        mCubeCamera = sceneManager->createCamera( "CubeMapCamera", true, true );
        mCubeCamera->setFOVy( Degree(90) );
        mCubeCamera->setAspectRatio( 1 );
        mCubeCamera->setFixedYawAxis(false);
        mCubeCamera->setNearClipDistance(0.5);
        //The default far clip distance is way too big for a cubemap-capable camera,
        //hich prevents Ogre from better culling.
        mCubeCamera->setFarClipDistance( 10000 );
        mCubeCamera->setPosition( 0, 1.0, 0 );

        //Setup the cubemap's compositor.
        CompositorChannelVec cubemapExternalChannels( 1 );
        //Any of the cubemap's render targets will do
        cubemapExternalChannels[0].target = mLocalCubemaps->getBuffer(0)->getRenderTarget();
        cubemapExternalChannels[0].textures.push_back( mLocalCubemaps );

        const Ogre::IdString workspaceName( "Tutorial_LocalCubemaps_cubemap" );
        if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
        {
            CompositorWorkspaceDef *workspaceDef = compositorManager->addWorkspaceDefinition(
                                                                                    workspaceName );
            //"CubemapRendererNode" has been defined in scripts.
            //Very handy (as it 99% the same for everything)
            workspaceDef->connectExternal( 0, "CubemapRendererNode", 0 );
        }

        ResourceLayoutMap initialCubemapLayouts;
        ResourceAccessMap initialCubemapUavAccess;
        mLocalCubemapsWorkspace =
                compositorManager->addWorkspace( sceneManager, cubemapExternalChannels, mCubeCamera,
                                                 workspaceName, true, -1, (UavBufferPackedVec*)0,
                                                 &initialCubemapLayouts, &initialCubemapUavAccess );

        //Now setup the regular renderer
        CompositorChannelVec externalChannels( 2 );
        //Render window
        externalChannels[0].target = renderWindow;
        externalChannels[1].target = mLocalCubemaps->getBuffer(0)->getRenderTarget();
        externalChannels[1].textures.push_back( mLocalCubemaps );

        return compositorManager->addWorkspace( sceneManager, externalChannels, camera,
                                                "Tutorial_LocalCubemapsWorkspace",
                                                true, -1, (UavBufferPackedVec*)0,
                                                &initialCubemapLayouts, &initialCubemapUavAccess );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::setupParallaxCorrectCubemaps(void)
    {
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

        //Probe 00
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024 );
        probe->initWorkspace();

        probeArea.mCenter = Ogre::Vector3( -0.505, 3.400016, -0.598495 );
        probe->set( probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, roomShape );

        //Probe 01
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024 );
        probe->initWorkspace();

        probeArea.mCenter = Ogre::Vector3( -0.505, 3.400016, 5.423867 );
        probe->set( probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, roomShape );

        //Probe 02
        probe = mParallaxCorrectedCubemap->createProbe();
        probe->setTextureParams( 1024, 1024 );
        probe->initWorkspace();

        probeArea.mCenter = Ogre::Vector3( -0.505, 3.400016, 10.657585 );
        probe->set( probeArea, Ogre::Vector3( 1.0f, 1.0f, 0.3f ),
                    Ogre::Matrix3::IDENTITY, roomShape );

        Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
        Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
        hlmsPbs->setParallaxCorrectedCubemap( mParallaxCorrectedCubemap );
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
            datablock->setTexture( Ogre::PBSM_REFLECTION, 0, mParallaxCorrectedCubemap->_tempGetBlendCubemap() );

            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Green", "Green",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::Green );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            datablock->setTexture( Ogre::PBSM_REFLECTION, 0, mParallaxCorrectedCubemap->_tempGetBlendCubemap() );

            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Blue", "Blue",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::Blue );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            datablock->setTexture( Ogre::PBSM_REFLECTION, 0, mParallaxCorrectedCubemap->_tempGetBlendCubemap() );

            datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                        hlmsPbs->createDatablock( "Cream", "Cream",
                                                  macroblock, blendblock,
                                                  Ogre::HlmsParamVec() ) );
            datablock->setBackgroundDiffuse( Ogre::ColourValue::White );
            datablock->setFresnel( Ogre::Vector3( 0.1f ), false );
            datablock->setRoughness( 0.65 );
            datablock->setTexture( Ogre::PBSM_REFLECTION, 0, mParallaxCorrectedCubemap->_tempGetBlendCubemap() );
        }

        generateScene( sceneManager );
#if 0
        {
            Ogre::Item *item = sceneManager->createItem( planeMesh, Ogre::SCENE_DYNAMIC );
            //item->setDatablock( "Marble" );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, -1, 0 );
            sceneNode->attachObject( item );

            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );
            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );
            Ogre::HlmsPbsDatablock *datablock = (Ogre::HlmsPbsDatablock *)hlmsPbs->getDefaultDatablock();
            datablock->setTexture( Ogre::PBSM_REFLECTION, 0, mParallaxCorrectedCubemap->_tempGetBlendCubemap() );
            datablock->setRoughness( 0.02 );
            datablock->setFresnel( Ogre::Vector3( 1 ), false );
        }

        if( false )
        {
            Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                         Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_DYNAMIC );
            //item->setDatablock( "Marble" );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->setPosition( 0, (-1 + 10) * 0.5, 0 );
            sceneNode->scale( Ogre::Vector3( 6 ) );
            sceneNode->attachObject( item );
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
                mObjects.push_back( item );
            }
        }

        {
            size_t numSpheres = 0;
            Ogre::HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
            Ogre::HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

            assert( dynamic_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms( Ogre::HLMS_PBS ) ) );

            Ogre::HlmsPbs *hlmsPbs = static_cast<Ogre::HlmsPbs*>( hlmsManager->getHlms(Ogre::HLMS_PBS) );

            const int numX = 1;
            const int numZ = 1;

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

                    //Set the dynamic cubemap to these materials.
                    //datablock->setTexture( Ogre::PBSM_REFLECTION, 0, mLocalCubemaps );
//                    datablock->setDiffuse( Ogre::Vector3( 0.0f, 1.0f, 0.0f ) );

//                    datablock->setRoughness( std::max( 0.02f, x / Ogre::max( 1, (float)(numX-1) ) ) );
//                    datablock->setFresnel( Ogre::Vector3( z / Ogre::max( 1, (float)(numZ-1) ) ), false );
                    datablock->setDiffuse( Ogre::Vector3( 0.0f, 0.0f, 0.0f ) );
                    datablock->setRoughness( 0.02f );
                    datablock->setFresnel( Ogre::Vector3( 1.0f ), false );

                    Ogre::Item *item = sceneManager->createItem( "Sphere1000.mesh",
                                                                 Ogre::ResourceGroupManager::
                                                                 AUTODETECT_RESOURCE_GROUP_NAME,
                                                                 Ogre::SCENE_DYNAMIC );
                    item->setDatablock( datablock );
                    item->setVisibilityFlags( 0x000000002 | 0x00000004 );

                    Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                            createChildSceneNode( Ogre::SCENE_DYNAMIC );
                    sceneNode->setPosition( Ogre::Vector3( armsLength * x - startX,
                                                           1.0f,
                                                           armsLength * z - startZ ) );
                    sceneNode->attachObject( item );
                    mSpheres.push_back( item );
                }
            }
        }
#endif

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
        lightNode->setPosition( -2.0f, 6.0f, 10.0f );
        light->setDirection( Ogre::Vector3( 0.5, -1, -0.5 ).normalisedCopy() );
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

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        mParallaxCorrectedCubemap->mTrackedPosition = camera->getDerivedPosition();

        TutorialGameState::createScene01();
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

//        Ogre::Root *root = mGraphicsSystem->getRoot();
//        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
//        Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();

//        compositorManager->removeWorkspace( mLocalCubemapsWorkspace );
//        mLocalCubemapsWorkspace = 0;

//        Ogre::TextureManager::getSingleton().remove( mLocalCubemaps->getHandle() );
//        mLocalCubemaps.setNull();

//        sceneManager->destroyCamera( mCubeCamera );
//        mCubeCamera = 0;
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
        mParallaxCorrectedCubemap->mTrackedPosition = camera->getDerivedPosition();

        //camera->setPosition( Ogre::Vector3( -0.505, 3.400016, 5.423867 ) );
        //camera->setPosition( -1.03587, 2.50012, 3.62891 );
        //camera->setOrientation( Ogre::Quaternion::IDENTITY );

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void LocalCubemapsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        Ogre::uint32 visibilityMask = mGraphicsSystem->getSceneManager()->getVisibilityMask();

        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle animation. ";
        outText += mAnimateObjects ? "[On]" : "[Off]";
        outText += "\nPress F3 to show/hide animated objects. ";
        outText += (visibilityMask & 0x000000001) ? "[On]" : "[Off]";

        Ogre::Camera *camera = mGraphicsSystem->getCamera();
        outText += Ogre::StringConverter::toString( camera->getPosition().x ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().y ) + ", " +
                Ogre::StringConverter::toString( camera->getPosition().z );

        outText += "\nProbes blending: ";
        outText += Ogre::StringConverter::toString( mParallaxCorrectedCubemap->getNumCollectedProbes() );
        //outText += "\nPress F4 to show/hide spheres from the reflection. ";
        //outText += (mSpheres.back()->getVisibilityFlags() & 0x000000004) ? "[On]" : "[Off]";
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
            std::vector<Ogre::MovableObject*>::const_iterator itor = mSpheres.begin();
            std::vector<Ogre::MovableObject*>::const_iterator end  = mSpheres.end();
            while( itor != end )
            {
                Ogre::uint32 visibilityMask = (*itor)->getVisibilityFlags();
                bool showPalette = (visibilityMask & 0x00000004) != 0;
                showPalette = !showPalette;
                visibilityMask &= ~0x00000004;
                visibilityMask |= (Ogre::uint32)(showPalette) << 2;

                (*itor)->setVisibilityFlags( visibilityMask );
                ++itor;
            }
        }
//        else if( arg.keysym.sym == SDLK_KP_PLUS )
//        {
//            Ogre::CubemapProbeVec probes = mParallaxCorrectedCubemap->getProbes();
//            Ogre::Vector3 probePos = probes[0]->getProbePos();
//            Ogre::Aabb aabb = probes[0]->getArea();
//            probePos += Ogre::Vector3::UNIT_Y * 0.5f;
//            //aabb.mCenter += Ogre::Vector3::UNIT_Y * 0.5f;
//            probes[0]->set( probePos, aabb,
//                            probes[0]->getAabbOrientation(),
//                            probes[0]->getAabbFalloff() );
//        }
//        else if( arg.keysym.sym == SDLK_KP_MINUS )
//        {
//            Ogre::CubemapProbeVec probes = mParallaxCorrectedCubemap->getProbes();
//            Ogre::Vector3 probePos = probes[0]->getProbePos();
//            Ogre::Aabb aabb = probes[0]->getArea();
//            probePos -= Ogre::Vector3::UNIT_Y * 0.5f;
//            //aabb.mCenter -= Ogre::Vector3::UNIT_Y * 0.5f;
//            probes[0]->set( probePos, aabb,
//                            probes[0]->getAabbOrientation(),
//                            probes[0]->getAabbFalloff() );
//        }
//        else if( arg.keysym.sym == SDLK_KP_9 )
//        {
//            Ogre::CubemapProbeVec probes = mParallaxCorrectedCubemap->getProbes();
//            Ogre::Vector3 probePos = probes[0]->getProbePos();
//            Ogre::Aabb aabb = probes[0]->getArea();
//            //probePos += Ogre::Vector3::UNIT_Y * 0.5f;
//            aabb.mCenter += Ogre::Vector3::UNIT_Y * 0.5f;
//            probes[0]->set( probePos, aabb,
//                            probes[0]->getAabbOrientation(),
//                            probes[0]->getAabbFalloff() );
//        }
//        else if( arg.keysym.sym == SDLK_KP_6 )
//        {
//            Ogre::CubemapProbeVec probes = mParallaxCorrectedCubemap->getProbes();
//            Ogre::Vector3 probePos = probes[0]->getProbePos();
//            Ogre::Aabb aabb = probes[0]->getArea();
//            //probePos -= Ogre::Vector3::UNIT_Y * 0.5f;
//            aabb.mCenter -= Ogre::Vector3::UNIT_Y * 0.5f;
//            probes[0]->set( probePos, aabb,
//                            probes[0]->getAabbOrientation(),
//                            probes[0]->getAabbFalloff() );
//        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
