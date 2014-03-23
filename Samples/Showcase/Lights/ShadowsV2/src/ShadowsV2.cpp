/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    @file 
        Shadows.cpp
    @brief
        Shows a few ways to use Ogre's shadowing techniques
*/

#include "SamplePlugin.h"
#include "ShadowsV2.h"

#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"
#include "Compositor/OgreCompositorShadowNode.h"

using namespace Ogre;
using namespace OgreBites;

Sample_ShadowsV2::Sample_ShadowsV2()
    : mFloorPlane( 0 )
    , mMainLight( 0 )
    , mMinFlareSize( 40 )
    , mMaxFlareSize( 80 )
    , mPssm( true )
{
    mInfo["Title"] = "Shadows v2";
    mInfo["Description"] =
            "Shows how to setup a shadow scene using depth-based shadow mapping.\n"
            "Shadow mapping involves setting up custom shaders and a proper compositor.\n\n"
            "This sample supports 8 different lights. Only the first one is a directional "
            "light and shadow caster. The rest of the lights are point lights.\n\n"
            "Note in this CTP (Community Technology Preview) only directional shadow caster"
            " lights have been thoroughly tested. Point and Spot casters should work with "
            "propper shader tweaks, but this hasn't been tested yet.\n\n"
            "Relevant Media files:\n"
            "   * Examples_Shadows.material\n"
            "   * Examples_Shadows.program\n"
            "   * Example_Shadows.compositor\n"
            "   * Example_ShadowsVp.glsl\n"
            "   * Example_ShadowsCasterFp.glsl\n"
            "   * Example_ShadowsFp.glsl";
    mInfo["Thumbnail"] = "thumb_shadows.png";
    mInfo["Category"] = "Showcase";
}
//---------------------------------------------------------------------------------------
CompositorWorkspace* Sample_ShadowsV2::setupCompositor(void)
{
    //In Compositor sample, we show how to use clever rewiring of connections and
    //disable nodes that aren't needed in order to turn on/off postprocessing effects without
    //recreating any buffer (making it extremely fast and GPU memory efficient).
    //
    //However, in this sample when we need to toggle PSSM shadows, we just destroy the
    //current compositor instance, modify the node, and recreate the instance; since in
    //a real world application, we would rarely have to switch between shadow methods.
    CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

    CompositorNodeDef *nodeDef = compositorManager->getNodeDefinitionNonConst( "ExampleShadows_Node" );
    CompositorTargetDef *targetDef = nodeDef->getTargetPass( 0 );

    assert( targetDef->getRenderTargetName() == "rt_renderwindow" &&
            "Media compositor file was modified. Make sure C++ code is in sync with those changes!" );

    CompositorPassDefVec &passes = targetDef->getCompositorPassesNonConst();
    assert( dynamic_cast<CompositorPassSceneDef*>( passes[1] ) &&
            "Media compositor file was modified. Make sure C++ code is in sync with those changes!" );
    CompositorPassSceneDef *passScene = static_cast<CompositorPassSceneDef*>( passes[1] );

    passScene->mShadowNode = mPssm ? "ExampleShadows_PssmShadowNode" : "ExampleShadows_FocusedShadowNode";

    const Ogre::IdString workspaceName( "ShadowsV2Workspace" );
    if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
    {
        CompositorWorkspaceDef *workspaceDef = compositorManager->addWorkspaceDefinition( workspaceName );
        workspaceDef->connectOutput( "ExampleShadows_Node", 0 );
    }

    return compositorManager->addWorkspace( mSceneMgr, mWindow, mCamera, workspaceName, true );
}
//---------------------------------------------------------------------------------------
void Sample_ShadowsV2::setupContent(void)
{
    mSceneMgr->setAmbientLight( ColourValue( 0.1f, 0.1f, 0.1f ) );

    SceneNode *lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mMainLight = mSceneMgr->createLight();
    mMainLight->setName( "Sun" );
    lightNode->attachObject( mMainLight );
    mMainLight->setType( Light::LT_DIRECTIONAL );
    mMainLight->setDirection( Vector3( -0.1f, -1.0f, -1.25f ).normalisedCopy() );
    //mMainLight->setDirection( Vector3::NEGATIVE_UNIT_Y );

    // create a mesh for our ground
    MeshManager::getSingleton().createPlane( "ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        Plane(Vector3::UNIT_Y, -20), 1000, 1000, 1, 1, true, 1, 6, 6, Vector3::UNIT_Z );

    mFloorPlane = mSceneMgr->createEntity( "ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            SCENE_STATIC );
    mFloorPlane->setMaterialName( mPssm ? "Example_Shadows_Floor_pssm" : "Example_Shadows_Floor" );
    mFloorPlane->setCastShadows( true );
    mSceneMgr->getRootSceneNode( SCENE_STATIC )->attachObject( mFloorPlane );

    Entity *penguin = mSceneMgr->createEntity( "penguin.mesh" );
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->attachObject( penguin );
    penguin->setName( "Penguin" );
    penguin->setMaterialName( mPssm ? "Example_Shadows_Penguin_pssm" :"Example_Shadows_Penguin" );
    mCasters.push_back( penguin );

    mCamera->setPosition( 0, 20, 50.0f );
    mCamera->lookAt( 0, 20, 0 );

    //Shadow quality is very sensible to these settings. The defaults are insane
    //(covers a huge distance) which is often not needed. Using more conservative
    //values greatly improves quality
    mSceneMgr->setShadowDirectionalLightExtrusionDistance( 200.0f );
    mSceneMgr->setShadowFarDistance( 200.0f );
    mCamera->setNearClipDistance( 0.1f );
    mCamera->setFarClipDistance( 5000.0f );

    createDebugOverlays();
}
//---------------------------------------------------------------------------------------
void Sample_ShadowsV2::createDebugOverlays(void)
{
    ////////////////////////////////////////////////////////////
    // BEGIN DEBUG
    IdString shadowNodeName = mPssm ? "ExampleShadows_PssmShadowNode" : "ExampleShadows_FocusedShadowNode";

    Ogre::MaterialPtr baseWhite = Ogre::MaterialManager::getSingletonPtr()->
                                                            getByName("Examples_Shadows_DebugView");
    Ogre::MaterialPtr DepthShadowTexture = baseWhite->clone("DepthShadowTexture0");
    Ogre::TextureUnitState* textureUnit = DepthShadowTexture->getTechnique(0)->getPass(0)->
                                                            getTextureUnitState(0);
    CompositorShadowNode *shadowNode = mWorkspace->findShadowNode( shadowNodeName );
    Ogre::TexturePtr tex = shadowNode->getLocalTextures()[0].textures[0];
    textureUnit->setTextureName( tex->getName() );

    if( mPssm )
    {
        DepthShadowTexture = baseWhite->clone("DepthShadowTexture1");
        textureUnit = DepthShadowTexture->getTechnique(0)->getPass(0)->getTextureUnitState(0);
        tex = shadowNode->getLocalTextures()[1].textures[0];
        textureUnit->setTextureName(tex->getName());

        DepthShadowTexture = baseWhite->clone("DepthShadowTexture2");
        textureUnit = DepthShadowTexture->getTechnique(0)->getPass(0)->getTextureUnitState(0);
        tex = shadowNode->getLocalTextures()[2].textures[0];
        textureUnit->setTextureName(tex->getName());
    }

    Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
    // Create an overlay
    Overlay *debugOverlay = overlayManager.create( "OverlayName" );

    // Create a panel
    Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(
        overlayManager.createOverlayElement("Panel", "PanelName0"));
    panel->setMetricsMode(Ogre::GMM_PIXELS);
    panel->setPosition(10, 10);
    panel->setDimensions(100, 100);
    panel->setMaterialName("DepthShadowTexture0");
    debugOverlay->add2D(panel);

    if( mPssm )
    {
        panel = static_cast<Ogre::OverlayContainer*>(
            overlayManager.createOverlayElement("Panel", "PanelName1"));
        panel->setMetricsMode(Ogre::GMM_PIXELS);
        panel->setPosition(120, 10);
        panel->setDimensions(100, 100);
        panel->setMaterialName("DepthShadowTexture1");
        debugOverlay->add2D(panel);
        panel = static_cast<Ogre::OverlayContainer*>(
            overlayManager.createOverlayElement("Panel", "PanelName2"));
        panel->setMetricsMode(Ogre::GMM_PIXELS);
        panel->setPosition(230, 10);
        panel->setDimensions(100, 100);
        panel->setMaterialName("DepthShadowTexture2");
        debugOverlay->add2D(panel);
    }

    debugOverlay->show();

    // END DEBUG
    ////////////////////////////////////////////////////////////
}
