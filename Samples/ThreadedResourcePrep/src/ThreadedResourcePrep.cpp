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

#include <sstream>
#include <iomanip>

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

    // set lights
    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.4));
    Light* light = mSceneMgr->createLight();
    mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(200, 100, 100))->attachObject(light);
    light->setDiffuseColour(0.9, 0.9, 0.95);

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

    mCameraMan->setStyle(CS_MANUAL);
    SceneNode* camnode = mCameraMan->getCamera();
    camnode->setPosition(Vector3(0,45,80));
    camnode->lookAt(Vector3(0,0,0), Node::TS_WORLD);
}

void Sample_ThreadedResourcePrep::cleanupContent()
{
    MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

const Vector3 TIGHTEN_SPOTS(0.7, 1, 0.7);

void Sample_ThreadedResourcePrep::defineSelectableMesh(String name, Vector3 pos, Vector3 scale, Degree yaw)
{
    // Fill mesh info
    MeshInfo mi;
    mi.sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos * TIGHTEN_SPOTS);
    mi.meshName = name;
    mi.meshScale = scale;
    mi.meshYaw = yaw;

    // Start with all meshes in scene
    mi.mesh = MeshManager::getSingleton().load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mi.entity = mSceneMgr->createEntity(mi.mesh);
    mi.sceneNode->attachObject(mi.entity);
    mi.sceneNode->scale(scale);
    mi.sceneNode->yaw(yaw);

    mMeshQueue.push_back(mi);
}

void Sample_ThreadedResourcePrep::setupSelectableMeshes()
{
    // Each mesh has dedicated spot so they can appear in any order.
    // We prefer meshes with associated materials (the common case).

    defineSelectableMesh("sibenik.mesh",Vector3(0,0,-35), Vector3(0.5, 0.5, 0.5), Degree(180));
    defineSelectableMesh("ogrehead.mesh",Vector3(0,3,0), Vector3(0.2, 0.2, 0.2), Degree(0));
    defineSelectableMesh("facial.mesh",Vector3(-35,3,-1), Vector3(0.1, 0.1, 0.1), Degree(0));
    defineSelectableMesh("penguin.mesh",Vector3(12,3,0), Vector3(0.1, 0.1, 0.1), Degree(0));
    defineSelectableMesh("knot.mesh",Vector3(-14,5,16), Vector3(0.04, 0.04, 0.04), Degree(0));
    defineSelectableMesh("spine.mesh",Vector3(-11,0.5,0), Vector3(0.1, 0.1, 0.1), Degree(0));
    defineSelectableMesh("ninja.mesh",Vector3(-25,0.5,5), Vector3(0.05, 0.05, 0.05), Degree(205));
    defineSelectableMesh("jaiqua.mesh",Vector3(-23,0.5,-9), Vector3(0.5, 0.5, 0.5), Degree(180));
    defineSelectableMesh("fish.mesh",Vector3(-2,0.5,20), Vector3(1, 1, 1), Degree(0));
    defineSelectableMesh("robot.mesh",Vector3(-20,0.5,-2), Vector3(0.1, 0.1, 0.1), Degree(-85));
    defineSelectableMesh("Sinbad.mesh",Vector3(30,5,5), Vector3(1, 1, 1), Degree(-25));
    defineSelectableMesh("Sword.mesh",Vector3(11,5,18), Vector3(1, 1, 1), Degree(75));
    defineSelectableMesh("Barrel.mesh",Vector3(12,2,18), Vector3(1, 1, 1), Degree(0));
    defineSelectableMesh("DamagedHelmet.mesh",Vector3(22,1.5,18), Vector3(4, 4, 4), Degree(0));
    defineSelectableMesh("dragon.mesh",Vector3(-5,30,10), Vector3(0.1, 0.1, 0.1), Degree(216));
    defineSelectableMesh("razor.mesh",Vector3(15,25,10), Vector3(0.1, 0.1, 0.1), Degree(-16));
    // these have no material but are big - good for the benchmark
    defineSelectableMesh("geosphere4500.mesh",Vector3(40,0,-25), Vector3(0.01, 0.01, 0.01), Degree(180));
    defineSelectableMesh("geosphere8000.mesh",Vector3(-40,0,-25), Vector3(0.01, 0.01, 0.01), Degree(180));
}

void Sample_ThreadedResourcePrep::setupControls()
{
    mTrayMgr->showCursor();

    const float BOTTOM_W = 370;
    const float TOPLEFT_W = 200;

    // create a menu to choose the model displayed
    mMeshMenu = mTrayMgr->createLongSelectMenu(TL_BOTTOM, "Mesh", "Mesh", BOTTOM_W, 290, 10);

    mMeshStatLabel = mTrayMgr->createLabel(TL_BOTTOM, "MeshStat", "", BOTTOM_W);
    refreshStatsUi();

    mReloadBtn = mTrayMgr->createButton(TL_BOTTOM, "ReloadBtn", "Reload!", BOTTOM_W/2);

    mBatchSlider = mTrayMgr->createThickSlider(TL_TOPLEFT, "BatchSlider", "Batch length", TOPLEFT_W, 50, 1, (Real)mMeshQueue.size(), (int)mMeshQueue.size());

    mUnloadBtn = mTrayMgr->createButton(TL_TOPLEFT, "UnloadBtn", "Unload batch", TOPLEFT_W);

    mBatchSlider->setValue((Real)mMeshQueue.size(), /* notifyListener: */false);
    mBatchSize = mMeshQueue.size();

    refreshMeshUi();
    mMeshMenu->selectItem(0, /* notifyListener: */false);
}

void Sample_ThreadedResourcePrep::itemSelected(SelectMenu* menu)
{
    if (menu == mMeshMenu)
    {
        // Find selected mesh by name (account for "<in bulk>" in name)
        StringVector toks = Ogre::StringUtil::split(mMeshMenu->getSelectedItem());
        auto selectedMeshItor = std::find_if (mMeshQueue.begin(), mMeshQueue.end(),
            [&toks](MeshInfo& mi)
            {
                return mi.meshName == toks[0];
            });

        // focus camera
        SceneNode* camnode = mCameraMan->getCamera();
        camnode->lookAt(selectedMeshItor->sceneNode->getPosition(), Node::TS_WORLD);

        // Move selected item to front of queue
        MeshInfo mi = *selectedMeshItor;
        mMeshQueue.erase(selectedMeshItor);
        mMeshQueue.push_front(mi);
        refreshMeshUi();

        mStats.resetStats();
        refreshStatsUi();
    }
}

void Sample_ThreadedResourcePrep::sliderMoved(Slider* slider)
{
    if (slider == mBatchSlider)
    {
        size_t bulkSize = (size_t)mBatchSlider->getValue();
        if (bulkSize != mBatchSize)
        {
            mBatchSize = bulkSize;
            refreshMeshUi();
            mStats.resetStats();
            refreshStatsUi();
        }
    }
}

void Sample_ThreadedResourcePrep::refreshStatsUi()
{
    std::stringstream buf;
    buf << "Reloads: " << mStats.totalReloads 
        << ", Avg. time: " << std::setprecision(3) << mStats.avgReloadTime 
        << " (Last: " << std::setprecision(3) << mStats.lastReloadTime << ")";
    mMeshStatLabel->setCaption(buf.str());
}

void Sample_ThreadedResourcePrep::refreshMeshUi()
{
    mMeshMenu->clearItems();
    
    for (size_t i=0; i<mMeshQueue.size(); i++)
    {
        if (i < mBatchSize)
        {
            mMeshMenu->addItem(mMeshQueue[i].meshName + " (in batch)");
        }
        else
        {
            mMeshMenu->addItem(mMeshQueue[i].meshName);
        }
    }

    mReloadBtn->setCaption("Reload " + StringConverter::toString((int)mBatchSlider->getValue()) + " mesh(es)");
}

void Sample_ThreadedResourcePrep::performSyncUnload()
{
    // Sync unload
    for (size_t i = 0; i < mBatchSize; i++)
    {
        MeshInfo& mi = mMeshQueue[i];

        mi.sceneNode->destroyAllObjects();
        mi.entity = nullptr;
        forceUnloadAllDependentResources(mi.mesh);
        mi.mesh->unload();
    }
}

void Sample_ThreadedResourcePrep::performSyncLoad()
{
    // Sync load
    long long start = mTimer.getMilliseconds();
    for (size_t i = 0; i < mBatchSize; i++)
    {
        MeshInfo& mi = mMeshQueue[i];
        mi.mesh->load();
        mi.entity = mSceneMgr->createEntity(mi.mesh);
        mi.sceneNode->attachObject(mi.entity);
    }
    long long end = mTimer.getMilliseconds();

    mStats.recordReloadTime(static_cast<double>(end - start) * 0.001);
}

void Sample_ThreadedResourcePrep::buttonHit(Button* button)
{
    if (button == mReloadBtn)
    {
        performSyncUnload();
        performSyncLoad();
        refreshStatsUi();
    }
    else if (button == mUnloadBtn)
    {
        performSyncUnload();
    }
}

void Sample_ThreadedResourcePrep::forceUnloadAllDependentTextures(Pass* pass)
{
    for (TextureUnitState* tus: pass->getTextureUnitStates())
    {
        for (size_t iFrame = 0; iFrame < tus->getNumFrames(); iFrame++)
        {
            TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(tus->getFrameTextureName(iFrame));
            if (tex)
            {
                tex->unload();
            }
        }
    }
}

void Sample_ThreadedResourcePrep::forceUnloadAllDependentResources(MeshPtr& mesh)
{
    for (SubMesh* submesh: mesh->getSubMeshes())
    {
        if (submesh->getMaterial())
        {
            for (Technique* teq: submesh->getMaterial()->getSupportedTechniques())
            {
                for (Pass* pass: teq->getPasses())
                {
                    forceUnloadAllDependentTextures(pass);
                }
            }
            submesh->getMaterial()->unload();
        }
    }
}
