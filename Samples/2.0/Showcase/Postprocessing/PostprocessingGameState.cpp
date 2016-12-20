
#include "PostprocessingGameState.h"
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

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorNodeDef.h"

#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"

#include "OgreTexture.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

#define COMPOSITORS_PER_PAGE 8

using namespace Ogre;

namespace Demo
{
    PostprocessingGameState::PostprocessingGameState( const String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mCurrentPage( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::importV1Mesh( const Ogre::String &meshName )
    {
        Ogre::v1::MeshPtr v1Mesh;
        Ogre::MeshPtr v2Mesh;

        v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                    meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );

        //Create a v2 mesh to import to, with a different name (arbitrary).
        v2Mesh = Ogre::MeshManager::getSingleton().createManual(
                    meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

        v2Mesh->importV1( v1Mesh.get(), true, true, true );

        //Free memory
        v1Mesh->unload();
        Ogre::v1::MeshManager::getSingleton().remove( meshName );
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::createCustomTextures(void)
    {
        TexturePtr tex = TextureManager::getSingleton().createManual(
            "HalftoneVolume",
            "General",
            TEX_TYPE_3D,
            64,64,64,
            0,
            PF_L8,
            TU_DYNAMIC_WRITE_ONLY
        );

        if(!tex.isNull())
        {
            v1::HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
            ptr->lock( v1::HardwareBuffer::HBL_DISCARD );
            const PixelBox &pb = ptr->getCurrentLock();
            Ogre::uint8 *data = static_cast<Ogre::uint8*>(pb.data);

            size_t height = pb.getHeight();
            size_t width = pb.getWidth();
            size_t depth = pb.getDepth();
            size_t rowPitch = pb.rowPitch;
            size_t slicePitch = pb.slicePitch;

            for (size_t z = 0; z < depth; ++z)
            {
                for (size_t y = 0; y < height; ++y)
                {
                    for(size_t x = 0; x < width; ++x)
                    {
                        float fx = 32-(float)x+0.5f;
                        float fy = 32-(float)y+0.5f;
                        float fz = 32-((float)z)/3+0.5f;
                        float distanceSquare = fx*fx+fy*fy+fz*fz;
                        data[slicePitch*z + rowPitch*y + x] =  0x00;
                        if (distanceSquare < 1024.0f)
                            data[slicePitch*z + rowPitch*y + x] +=  0xFF;
                    }
                }
            }
            ptr->unlock();
        }

        Ogre::RenderWindow *renderWindow = mGraphicsSystem->getRenderWindow();

        TexturePtr tex2 = TextureManager::getSingleton().createManual(
            "DitherTex",
            "General",
            TEX_TYPE_2D,
            renderWindow->getWidth(), renderWindow->getHeight() , 1,
            0,
            PF_L8,
            TU_DYNAMIC_WRITE_ONLY
        );

        v1::HardwarePixelBufferSharedPtr ptr2 = tex2->getBuffer(0,0);
        ptr2->lock( v1::HardwareBuffer::HBL_DISCARD );
        const PixelBox &pb2 = ptr2->getCurrentLock();
        Ogre::uint8 *data2 = static_cast<Ogre::uint8*>(pb2.data);

        size_t height2 = pb2.getHeight();
        size_t width2 = pb2.getWidth();
        size_t rowPitch2 = pb2.rowPitch;

        for (size_t y = 0; y < height2; ++y)
        {
            for(size_t x = 0; x < width2; ++x)
            {
                data2[rowPitch2*y + x] = Ogre::Math::RangeRandom(64.0,192);
            }
        }

        ptr2->unlock();
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::togglePostprocess( IdString nodeName )
    {
        Root *root = mGraphicsSystem->getRoot();
        CompositorManager2 *compositorManager = root->getCompositorManager2();
        CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();

        const IdString workspaceName( "PostprocessingSampleWorkspace" );

        //Disable/Enable the node (it was already instantiated in setupCompositor())
        CompositorNode *node = workspace->findNode( nodeName );
        node->setEnabled( !node->getEnabled() );

        //The workspace instance can't return a non-const version of
        //its definition, so we perform the lookup this way.
        CompositorWorkspaceDef *workspaceDef =
                                        compositorManager->getWorkspaceDefinition( workspaceName );

        //Try both methods alternating them, just for the sake of testing and demonstrating them.
        //The 1st user 5 toggles will use method 1, the next 5 toggles will use method 2, and repeat
        static int g_methodCount = 0;

        if( g_methodCount < 5 )
        {
            //-------------------------------------------------------------------------------------------
            //
            //  METHOD 1 (the masochist way, 'cos it's hard, prone to bugs, but very flexible):
            //      When enabling: Interleave the node between the 1st and 2nd node.
            //      When disabling: Manually disconnect the node in the middle, then fix broken
            //      connections.
            //      Normally this method is not recommended, but if you're looking to make a GUI node
            //      editor, the knowledge from this code is very useful.
            //
            //-------------------------------------------------------------------------------------------
            CompositorWorkspaceDef::ChannelRouteList &channelRouteList = workspaceDef->_getChannelRoutes();
            if( node->getEnabled() )
            {
                //Enabling
                if( channelRouteList.size() == 1 )
                {
                    //No compositor node yet activated
                    channelRouteList.pop_back();
                    workspaceDef->connect( "PostprocessingSampleStdRenderer", node->getName() );
                    workspaceDef->connect( node->getName(), 0, "FinalComposition", 1 );
                }
                else
                {
                    //There is at least one compositor active already, interleave

                    const IdString firstNodeName( "PostprocessingSampleStdRenderer" );

                    //Find the first node "PostprocessingSampleStdRenderer", and put the new compo
                    //after that once (theoretically, we could put it anywhere we want in the chain)
                    CompositorWorkspaceDef::ChannelRouteList::iterator it = channelRouteList.begin();
                    CompositorWorkspaceDef::ChannelRouteList::iterator en = channelRouteList.end();

                    CompositorWorkspaceDef::ChannelRoute *firstNodeChannel0 = 0;
                    CompositorWorkspaceDef::ChannelRoute *firstNodeChannel1 = 0;

                    while( it != en )
                    {
                        if( it->outNode == firstNodeName )
                        {
                            if( it->inChannel == 0 )
                                firstNodeChannel0 = &(*it);
                            else
                                firstNodeChannel1 = &(*it);
                        }
                        ++it;
                    }

                    IdString old2ndNode = firstNodeChannel0->inNode; //Will now become the 3rd node

                    firstNodeChannel0->inNode       = node->getName();
                    //firstNodeChannel0->inChannel= 0 //Channel stays the same
                    firstNodeChannel1->inNode       = node->getName();
                    //firstNodeChannel1->inChannel= 1 //Channel stays the same

                    workspaceDef->connect( node->getName(), old2ndNode );
                }
            }
            else
            {
                //Disabling
                if( channelRouteList.size() == 3 )
                {
                    //After disabling us, there will be no more compositors active
                    channelRouteList.clear();
                    workspaceDef->connect( "PostprocessingSampleStdRenderer", 0, "FinalComposition", 1 );
                }
                else
                {
                    //Find our channel route
                    CompositorWorkspaceDef::ChannelRouteList::iterator it = channelRouteList.begin();
                    CompositorWorkspaceDef::ChannelRouteList::iterator en = channelRouteList.end();

                    IdString currentNode = node->getName();
                    IdString prevNode; //We assume all inputs are coming from the same node
                    IdString nextNode; //We assume our node doesn't output to more than one node simultaneously

                    while( it != en )
                    {
                        if( it->inNode == currentNode )
                        {
                            prevNode = it->outNode;
                            it = channelRouteList.erase( it );
                        }
                        else if( it->outNode == currentNode )
                        {
                            nextNode = it->inNode;
                            it = channelRouteList.erase( it );
                        }
                        else
                        {
                            ++it;
                        }
                    }

                    if( nextNode == "FinalComposition" )
                        workspaceDef->connect( prevNode, 0, nextNode, 1 );
                    else
                        workspaceDef->connect( prevNode, nextNode );
                }
            }
        }
        else
        {
            //-------------------------------------------------------------------------------------------
            //
            //  METHOD 2 (the easy way):
            //      Reconstruct the whole connection from scratch based on a copy (be it a cloned,
            //      untouched workspace definition, a custom file, or the very own workspace instance)
            //      but leaving the node we're disabling unplugged.
            //      This method is much safer and easier, the **recommended** way for most usage
            //      scenarios involving toggling compositors on and off frequently. With a few tweaks,
            //      it can easily be adapted to complex compositors too.
            //
            //-------------------------------------------------------------------------------------------
            workspaceDef->clearAllInterNodeConnections();

            IdString finalCompositionId = "FinalComposition";
            const CompositorNodeVec &nodes = workspace->getNodeSequence();

            IdString lastInNode;
            CompositorNodeVec::const_iterator it = nodes.begin();
            CompositorNodeVec::const_iterator en = nodes.end();

            while( it != en )
            {
                CompositorNode *outNode = *it;

                if( outNode->getEnabled() && outNode->getName() != finalCompositionId )
                {
                    //Look for the next enabled node we can connect to
                    CompositorNodeVec::const_iterator it2 = it + 1;
                    while( it2 != en && (!(*it2)->getEnabled() || (*it2)->getName() == finalCompositionId) )
                        ++it2;

                    if( it2 != en )
                    {
                        lastInNode = (*it2)->getName();
                        workspaceDef->connect( outNode->getName(), lastInNode );
                    }

                    it = it2 - 1;
                }

                ++it;
            }

            if( lastInNode == IdString() )
                lastInNode = "PostprocessingSampleStdRenderer";

            workspaceDef->connect( lastInNode, 0, "FinalComposition", 1 );

            //Not needed unless we'd called workspaceDef->clearOutputConnections
            //workspaceDef->connectOutput( "FinalComposition", 0 );
        }

        g_methodCount = (g_methodCount + 1) % 10;

        //Now that we're done, tell the instance to update itself.
        workspace->reconnectAllNodes();
    }
    //-----------------------------------------------------------------------------------
    CompositorWorkspace* PostprocessingGameState::setupCompositor(void)
    {
        mCompositorNames.clear();

        createExtraEffectsFromCode();

        Root *root = mGraphicsSystem->getRoot();
        CompositorManager2 *compositorManager = root->getCompositorManager2();

        const IdString workspaceName( "PostprocessingSampleWorkspace" );
        CompositorWorkspaceDef *workspaceDef =
                compositorManager->getWorkspaceDefinition( workspaceName );

        //Clear the definition made with scripts as example
        workspaceDef->clearAll();

        CompositorManager2::CompositorNodeDefMap nodeDefs = compositorManager->getNodeDefinitions();

        //iterate through Compositor Managers resources and add name keys to menu
        CompositorManager2::CompositorNodeDefMap::const_iterator itor = nodeDefs.begin();
        CompositorManager2::CompositorNodeDefMap::const_iterator end  = nodeDefs.end();

        IdString compositorId = "Ogre/Postprocess";

        // Add all compositor resources to the view container
        while( itor != end )
        {
            if( itor->second->mCustomIdentifier == compositorId )
            {
                mCompositorNames.push_back(itor->second->getNameStr());

                //Manually disable the node and add it to the workspace without any connection
                itor->second->setStartEnabled( false );
                workspaceDef->addNodeAlias( itor->first, itor->first );
            }

            ++itor;
        }

        workspaceDef->connect( "PostprocessingSampleStdRenderer", 0, "FinalComposition", 1 );
        workspaceDef->connectExternal( 0, "FinalComposition", 0 );

        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();
        RenderWindow *renderWindow = mGraphicsSystem->getRenderWindow();
        Camera *camera = mGraphicsSystem->getCamera();
        CompositorWorkspace *workspace = compositorManager->addWorkspace( sceneManager,
                                                                          renderWindow,
                                                                          camera,
                                                                          workspaceName, true );

        //workspace->setListener( &mWorkspaceListener );

        return workspace;
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::createExtraEffectsFromCode(void)
    {
        Root *root = mGraphicsSystem->getRoot();
        CompositorManager2 *compositorManager = root->getCompositorManager2();

        // Bloom compositor is loaded from script but here is the hard coded equivalent
        if( !compositorManager->hasNodeDefinition( "Bloom" ) )
        {
            CompositorNodeDef *bloomDef = compositorManager->addNodeDefinition( "Bloom" );

            //Input channels
            bloomDef->addTextureSourceName( "rt_input", 0, TextureDefinitionBase::TEXTURE_INPUT );
            bloomDef->addTextureSourceName( "rt_output", 1, TextureDefinitionBase::TEXTURE_INPUT );

            bloomDef->mCustomIdentifier = "Ogre/Postprocess";

            //Local textures
            bloomDef->setNumLocalTextureDefinitions( 2 );
            {
                TextureDefinitionBase::TextureDefinition *texDef = bloomDef->addTextureDefinition( "rt0" );
                texDef->widthFactor     = 0.25f;
                texDef->heightFactor    = 0.25f;
                texDef->formatList.push_back( Ogre::PF_R8G8B8 );

                texDef = bloomDef->addTextureDefinition( "rt1" );
                texDef->widthFactor     = 0.25f;
                texDef->heightFactor    = 0.25f;
                texDef->formatList.push_back( Ogre::PF_R8G8B8 );
            }

            bloomDef->setNumTargetPass( 4 );

            {
                CompositorTargetDef *targetDef = bloomDef->addTargetPass( "rt0" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Postprocess/BrightPass2";
                    passQuad->addQuadTextureSource( 0, "rt_input", 0 );
                }
            }
            {
                CompositorTargetDef *targetDef = bloomDef->addTargetPass( "rt1" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Postprocess/BlurV";
                    passQuad->addQuadTextureSource( 0, "rt0", 0 );
                }
            }
            {
                CompositorTargetDef *targetDef = bloomDef->addTargetPass( "rt0" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Postprocess/BluH";
                    passQuad->addQuadTextureSource( 0, "rt1", 0 );
                }
            }
            {
                CompositorTargetDef *targetDef = bloomDef->addTargetPass( "rt_output" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Postprocess/BloomBlend2";
                    passQuad->addQuadTextureSource( 0, "rt_input", 0 );
                    passQuad->addQuadTextureSource( 1, "rt0", 0 );
                }
            }

            //Output channels
            bloomDef->setNumOutputChannels( 2 );
            bloomDef->mapOutputChannel( 0, "rt_output" );
            bloomDef->mapOutputChannel( 1, "rt_input" );
        }

        //Glass compositor is loaded from script but here is the hard coded equivalent
        if( !compositorManager->hasNodeDefinition( "Glass" ) )
        {
            CompositorNodeDef *glassDef = compositorManager->addNodeDefinition( "Glass" );

            //Input channels
            glassDef->addTextureSourceName( "rt_input", 0, TextureDefinitionBase::TEXTURE_INPUT );
            glassDef->addTextureSourceName( "rt_output", 1, TextureDefinitionBase::TEXTURE_INPUT );

            glassDef->mCustomIdentifier = "Ogre/Postprocess";

            glassDef->setNumTargetPass( 1 );

            {
                CompositorTargetDef *targetDef = glassDef->addTargetPass( "rt_output" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Postprocess/Glass";
                    passQuad->addQuadTextureSource( 0, "rt_input", 0 );
                }
            }

            //Output channels
            glassDef->setNumOutputChannels( 2 );
            glassDef->mapOutputChannel( 0, "rt_output" );
            glassDef->mapOutputChannel( 1, "rt_input" );
        }

        if( !compositorManager->hasNodeDefinition( "Motion Blur" ) )
        {
            /// Motion blur effect
            CompositorNodeDef *motionBlurDef = compositorManager->addNodeDefinition( "Motion Blur" );

            //Input channels
            motionBlurDef->addTextureSourceName( "rt_input", 0, TextureDefinitionBase::TEXTURE_INPUT );
            motionBlurDef->addTextureSourceName( "rt_output", 1, TextureDefinitionBase::TEXTURE_INPUT );

            motionBlurDef->mCustomIdentifier = "Ogre/Postprocess";

            //Local textures
            motionBlurDef->setNumLocalTextureDefinitions( 1 );
            {
                TextureDefinitionBase::TextureDefinition *texDef =
                                                            motionBlurDef->addTextureDefinition( "sum" );
                texDef->width   = 0;
                texDef->height  = 0;
                texDef->formatList.push_back( Ogre::PF_R8G8B8 );
            }

            motionBlurDef->setNumTargetPass( 3 );

            /// Initialisation pass for sum texture
            {
                CompositorTargetDef *targetDef = motionBlurDef->addTargetPass( "sum" );
                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mNumInitialPasses = 1;
                    passQuad->mMaterialName = "Ogre/Copy/4xFP32";
                    passQuad->addQuadTextureSource( 0, "rt_input", 0 );
                }
            }
            /// Do the motion blur
            {
                CompositorTargetDef *targetDef = motionBlurDef->addTargetPass( "rt_output" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Postprocess/Combine";
                    passQuad->addQuadTextureSource( 0, "rt_input", 0 );
                    passQuad->addQuadTextureSource( 1, "sum", 0 );
                }
            }
            /// Copy back sum texture for the next frame
            {
                CompositorTargetDef *targetDef = motionBlurDef->addTargetPass( "sum" );

                {
                    CompositorPassQuadDef *passQuad;
                    passQuad = static_cast<CompositorPassQuadDef*>( targetDef->addPass( PASS_QUAD ) );
                    passQuad->mMaterialName = "Ogre/Copy/4xFP32";
                    passQuad->addQuadTextureSource( 0, "rt_output", 0 );
                }
            }

            //Output channels
            motionBlurDef->setNumOutputChannels( 2 );
            motionBlurDef->mapOutputChannel( 0, "rt_output" );
            motionBlurDef->mapOutputChannel( 1, "rt_input" );
        }
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::createScene01(void)
    {
        SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        createExtraEffectsFromCode();
        createCustomTextures();

        Ogre::v1::MeshManager::getSingleton().createPlane( "MyPlane",
                                            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            Ogre::Plane( Ogre::Vector3::UNIT_Y, 1.0f ), 50.0f, 50.0f,
                                            1, 1, true, 1, 8.1f, 8.12f, Ogre::Vector3::UNIT_Z,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC,
                                            Ogre::v1::HardwareBuffer::HBU_STATIC );
        importV1Mesh( "MyPlane" );

        Ogre::Item *item = sceneManager->createItem(
                    "MyPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    Ogre::SCENE_STATIC );
        item->setDatablock( "Rocks" );

        HlmsManager *hlmsManager = mGraphicsSystem->getRoot()->getHlmsManager();
        Ogre::Hlms *hlms = hlmsManager->getHlms(Ogre::HLMS_PBS);
        Ogre::HlmsPbsDatablock *datablock = static_cast<Ogre::HlmsPbsDatablock*>(
                                                    hlms->getDatablock( "Rocks" ) );

        for( size_t i=PBSM_DIFFUSE; i<=PBSM_ROUGHNESS; ++i )
        {
            HlmsSamplerblock samplerblock;
            samplerblock.mU = TAM_WRAP;
            samplerblock.mV = TAM_WRAP;
            datablock->setSamplerblock( static_cast<PbsTextureTypes>(i), samplerblock );
        }

        Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC )->
                                                createChildSceneNode( Ogre::SCENE_STATIC );
        sceneNode->setPosition( 0, -1.9, 0 );
        sceneNode->attachObject( item );

        importV1Mesh( "tudorhouse.mesh" );

        // House
        item = sceneManager->createItem( "tudorhouse.mesh",
                                         ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                         SCENE_STATIC );
        sceneNode = sceneManager->getRootSceneNode()->
                        createChildSceneNode( SCENE_STATIC, Vector3(3.5f, 4.5f, -2.0f) );
        sceneNode->scale( 0.01f, 0.01f, 0.01f );
        sceneNode->attachObject( item );

        item = sceneManager->createItem( "tudorhouse.mesh",
                                         ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                         SCENE_STATIC );
        sceneNode = sceneManager->getRootSceneNode()->
                        createChildSceneNode( SCENE_STATIC, Vector3(-3.5f, 4.5f, -2.0f) );
        sceneNode->scale( 0.01f, 0.01f, 0.01f );
        sceneNode->attachObject( item );

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        light = sceneManager->createLight();
        lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.8f, 0.4f, 0.2f ); //Warm
        light->setSpecularColour( 0.8f, 0.4f, 0.2f );
        //light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        lightNode->setPosition( -10.0f, 10.0f, 10.0f );
        light->setDirection( Ogre::Vector3( 1, -1, -1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        light = sceneManager->createLight();
        lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setDiffuseColour( 0.2f, 0.4f, 0.8f ); //Cold
        light->setSpecularColour( 0.2f, 0.4f, 0.8f );
        //light->setPowerScale( Ogre::Math::PI );
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        lightNode->setPosition( 10.0f, 10.0f, -10.0f );
        light->setDirection( Ogre::Vector3( -1, -1, 1 ).normalisedCopy() );
        light->setAttenuationBasedOnRadius( 10.0f, 0.01f );

        Camera *camera = mGraphicsSystem->getCamera();
        camera->move( Vector3( -8.0f, -2.6f, -4.5f ) );
        camera->lookAt( Vector3::ZERO );
        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::update( float timeSinceLast )
    {
        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::generateDebugText( float timeSinceLast, String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        if( mDisplayHelpMode == 0 )
            return;

        CompositorWorkspace *workspace = mGraphicsSystem->getCompositorWorkspace();
        Ogre::StringVector::const_iterator itor = mCompositorNames.begin() + mCurrentPage;
        Ogre::StringVector::const_iterator end  = mCompositorNames.begin() +
                                                    std::min( mCurrentPage + COMPOSITORS_PER_PAGE,
                                                              mCompositorNames.size() );

        size_t idx = 0;
        while( itor != end )
        {
            CompositorNode *node = workspace->findNode( *itor );
            outText += "\n";
            outText += Ogre::StringConverter::toString( ++idx );
            outText += ". [";
            outText += node->getEnabled() ? "X" : " ";
            outText += "] ";
            outText += *itor;
            ++itor;
        }

        outText += "\n\nSPACE: Next page [";
        outText += StringConverter::toString( mCurrentPage / COMPOSITORS_PER_PAGE + 1 );
        outText += "/";
        outText += StringConverter::toString( alignToNextMultiple( mCompositorNames.size(),
                                                                   COMPOSITORS_PER_PAGE ) /
                                              COMPOSITORS_PER_PAGE );
        outText += "]";
    }
    //-----------------------------------------------------------------------------------
    void PostprocessingGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym >= SDLK_1 && arg.keysym.sym <= SDLK_8 )
        {
            size_t idx = mCurrentPage + arg.keysym.sym - SDLK_1;

            if( idx < mCompositorNames.size() )
            {
                togglePostprocess( mCompositorNames[idx] );
            }
        }
        else if( arg.keysym.sym >= SDLK_KP_1 && arg.keysym.sym <= SDLK_KP_8 )
        {
            size_t idx = mCurrentPage + arg.keysym.sym - SDLK_KP_1;

            if( idx < mCompositorNames.size() )
            {
                togglePostprocess( mCompositorNames[idx] );
            }
        }
        else if( arg.keysym.sym == SDLK_SPACE )
        {
            mCurrentPage += COMPOSITORS_PER_PAGE;
            if( mCurrentPage >= mCompositorNames.size() )
                mCurrentPage = 0;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
