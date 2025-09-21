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

#include "ThreadedResourcePrep.h"

Sample_ThreadedResourcePrep::Sample_ThreadedResourcePrep()
{
    mInfo["Title"] = "Threaded Resource Prep";
    mInfo["Description"] = "Showcases and benchmarks background resource queue";
    mInfo["Category"] = "Unsorted";
}

void Sample_ThreadedResourcePrep::setupContent()
{
    // Make this viewport work with shader generator scheme.
    mViewport->setMaterialScheme(MSN_SHADERGEN);
    // update scheme for FFP supporting rendersystems
    MaterialManager::getSingleton().setActiveScheme(mViewport->getMaterialScheme());

    // set background
    mViewport->setBackgroundColour(ColourValue(0.9f, 0.9f, 0.7f));

    // use full ambient lighting
    mSceneMgr->setAmbientLight(ColourValue(1.f, 1.f, 1.f));

    // create a floor mesh resource
    MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Plane(Vector3::UNIT_Y, 0), 100, 100, 10, 10, true, 1, 10, 10, Vector3::UNIT_Z);

    // create a floor entity, give it a material, and place it at the origin
    Entity* floor = mSceneMgr->createEntity("Floor", "floor");
    floor->setMaterialName("Examples/Rockwall");
    floor->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->attachObject(floor);

    setupSelectableMeshes();
    setupControls();

    mCameraMan->setStyle(CS_FREELOOK);
}

void Sample_ThreadedResourcePrep::cleanupContent()
{
    MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void Sample_ThreadedResourcePrep::defineSelectableMesh(String name, Vector3 pos, Vector3 scale)
{
    // Start with all meshes in scene
    MeshInfo mi;
    mi.sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
    mi.meshName = name;
    mi.meshScale = scale;
    mi.mesh = MeshManager::getSingleton().load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mi.entity = mSceneMgr->createEntity(mi.mesh);
    mi.sceneNode->attachObject(mi.entity);
    mi.sceneNode->scale(scale);

    mSelectableMeshes[name] = mi;
}

void Sample_ThreadedResourcePrep::setupSelectableMeshes()
{
    // Each has dedicated spot so they can appear in any order.
    // We only want meshes with associated materials - common case.
    defineSelectableMesh("ogrehead.mesh",Vector3(0,3,0), Vector3(0.1, 0.1, 0.1));
    defineSelectableMesh("knot.mesh",Vector3(0,5,-10), Vector3(0.04, 0.04, 0.04));
    defineSelectableMesh("spine.mesh",Vector3(-11,0.5,-10), Vector3(0.1, 0.1, 0.1));
    defineSelectableMesh("ninja.mesh",Vector3(-25,0.5,-30), Vector3(0.1, 0.1, 0.1));
    defineSelectableMesh("robot.mesh",Vector3(24,0.5,-32), Vector3(0.1, 0.1, 0.1));
    defineSelectableMesh("Sinbad.mesh",Vector3(30,3,-32), Vector3(1, 1, 1));
    defineSelectableMesh("Sword.mesh",Vector3(33,3,-30), Vector3(1, 1, 1));
    defineSelectableMesh("dragon.mesh",Vector3(0,15,-40), Vector3(0.1, 0.1, 0.1));
}

void Sample_ThreadedResourcePrep::setupControls()
{
    mTrayMgr->showCursor();

    // create a menu to choose the model displayed
    mMeshMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Mesh", "Mesh", 370, 290, 10);
    for (const auto& p : mSelectableMeshes)
    {
        mMeshMenu->addItem(p.first);
    }
}


