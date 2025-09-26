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
    camnode->setPosition(Vector3(0,20,60));
    camnode->lookAt(Vector3(0,0,-45), Node::TS_WORLD);
}

void Sample_ThreadedResourcePrep::cleanupContent()
{
    MeshManager::getSingleton().remove("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    for (size_t i=0; i<mMeshQueue.size(); i++)
    {
        MeshInfo& mi = mMeshQueue[i];
        mi.mesh->removeListener(this);
    }
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
    mi.mesh = MeshManager::getSingleton().load(name, RGN_DEFAULT);
    mi.mesh->addListener(this);
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
    defineSelectableMesh("ogrehead.mesh",Vector3(0,3.5,0), Vector3(0.2, 0.2, 0.2), Degree(0));
    defineSelectableMesh("facial.mesh",Vector3(-35,3,2), Vector3(0.1, 0.1, 0.1), Degree(0));
    defineSelectableMesh("penguin.mesh",Vector3(12,3,0), Vector3(0.1, 0.1, 0.1), Degree(0));
    defineSelectableMesh("knot.mesh",Vector3(-14,5,16), Vector3(0.04, 0.04, 0.04), Degree(0));
    defineSelectableMesh("spine.mesh",Vector3(-11,0.5,0), Vector3(0.1, 0.1, 0.1), Degree(0));
    defineSelectableMesh("ninja.mesh",Vector3(-25,0.5,5), Vector3(0.05, 0.05, 0.05), Degree(205));
    defineSelectableMesh("jaiqua.mesh",Vector3(-23,0.5,-12), Vector3(0.5, 0.5, 0.5), Degree(180));
    defineSelectableMesh("fish.mesh",Vector3(-2,0.5,20), Vector3(1, 1, 1), Degree(0));
    defineSelectableMesh("robot.mesh",Vector3(-20,0.5,-2), Vector3(0.1, 0.1, 0.1), Degree(-85));
    defineSelectableMesh("Sinbad.mesh",Vector3(30,5,5), Vector3(1, 1, 1), Degree(-25));
    defineSelectableMesh("Sword.mesh",Vector3(10.5,5,18), Vector3(1, 1, 1), Degree(75));
    defineSelectableMesh("Barrel.mesh",Vector3(12,3,18), Vector3(1, 1, 1), Degree(0));
    defineSelectableMesh("DamagedHelmet.mesh",Vector3(22,2.5,18), Vector3(4, 4, 4), Degree(0));
    defineSelectableMesh("dragon.mesh",Vector3(75,28,-25), Vector3(0.2, 0.2, 0.2), Degree(203));
    defineSelectableMesh("razor.mesh",Vector3(-15,17,10), Vector3(0.1, 0.1, 0.1), Degree(-16));
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

    mThreadedMeshChk = mTrayMgr->createCheckBox(TL_TOPLEFT, "ThrMeshChk", "Bg. mesh prep", TOPLEFT_W);

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
    mMeshStatLabel->setCaption(
        StringUtil::format("Reloads: %d, Avg. time: %.3f (Last: %.3f)",
            mStats.totalReloads, mStats.avgReloadTime, mStats.lastReloadTime));
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

    mReloadBtn->setCaption(StringUtil::format("Reload %d mesh(es)", (int)mBatchSlider->getValue()));
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

void Sample_ThreadedResourcePrep::performSyncPrep()
{
    mNumLoadedMeshes = 0;
    for (size_t i = 0; i < mBatchSize; i++)
    {
        loadMeshOnQueue(i);
    }

    mStats.recordReloadTime(static_cast<double>(mLoadingTotalMillis) * 0.001);
    refreshStatsUi();
}

void Sample_ThreadedResourcePrep::loadMeshOnQueue(size_t i)
{
    long long startMillis = mTimer.getMilliseconds();

    MeshInfo& mi = mMeshQueue[i];

    mi.mesh->load();
    mi.entity = mSceneMgr->createEntity(mi.mesh);
    mi.sceneNode->attachObject(mi.entity);

    mLoadingTotalMillis += mTimer.getMilliseconds() - startMillis;
    mNumLoadedMeshes++;
}

void Sample_ThreadedResourcePrep::performThreadedPrep()
{
    mNumLoadedMeshes = 0;
    // mBatchSize must remain unchanged until all meshes are spawned!
    mBatchSlider->getOverlayElement()->setVisible(false);
    // No [*load] button must be pressed while working.
    mUnloadBtn->getOverlayElement()->setVisible(false);
    mReloadBtn->getOverlayElement()->setVisible(false);

    for (size_t i = 0; i < mBatchSize; i++)
    {
        MeshInfo& mi = mMeshQueue[i];
        ResourceBackgroundQueue::getSingleton().prepare(mi.mesh);
    }
}

void Sample_ThreadedResourcePrep::preparingComplete(Resource* resource)
{
    for (size_t i = 0; i < mBatchSize; i++)
    {
        MeshInfo& mi = mMeshQueue[i];
        if (mi.meshName == resource->getName())
        {
            //"##### BEGIN TEST of the early analyzer #####\n";

            if (mi.mesh->getLoadingState() != Resource::LOADSTATE_PREPARED)
            {
                Ogre::LogManager::getSingleton().stream() << "[ThreadedResourcePrep]" << mi.meshName << " is not in LOADSTATE_PREPARED, but: " << mi.mesh->getLoadingState();
            }
            else
            {
                EarlyMeshAnalyzer analyzer;
                analyzer.discoverLinkedResources(mi.mesh);
                Ogre::LogManager::getSingleton().stream() << "[ThreadedResourcePrep]" << mi.meshName << ": skeletons:" << analyzer.skeletonNames.size() << ", materials:" << analyzer.materialNames.size();
            }

            // "##### END TEST of the early analyzer #####\n";

            loadMeshOnQueue(i);
            break;
        }
    }

    if (mNumLoadedMeshes == mBatchSize)
    {
        // We're done!
        mBatchSlider->getOverlayElement()->setVisible(true);
        mUnloadBtn->getOverlayElement()->setVisible(true);
        mReloadBtn->getOverlayElement()->setVisible(true);
        mStats.recordReloadTime(static_cast<double>(mLoadingTotalMillis) * 0.001);
        refreshStatsUi();
    }
}

void Sample_ThreadedResourcePrep::buttonHit(Button* button)
{
    if (button == mReloadBtn)
    {
        performSyncUnload();
        mLoadingTotalMillis = 0;
        if (mThreadedMeshChk->isChecked())
        {
            performThreadedPrep();
        }
        else
        {
            performSyncPrep();
        }
    }
    else if (button == mUnloadBtn)
    {
        performSyncUnload();
    }
}

void Sample_ThreadedResourcePrep::checkBoxToggled(CheckBox* box)
{
    if (box == mThreadedMeshChk)
    {
        mStats.resetStats();
        refreshStatsUi();
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

EarlyMeshAnalyzer::EarlyMeshAnalyzer()
{
    mVersion = "[MeshSerializer_v1.100]"; // see `MeshSerializerImpl::MeshSerializerImpl()`
}

enum EearlyMeshChunkID // see internal header 'OgreMeshFileFormat.h'
{
    M_HEADER                = 0x1000,
    M_MESH                  = 0x3000,
    M_SUBMESH               = 0x4000,
    M_GEOMETRY              = 0x5000,
    M_MESH_SKELETON_LINK    = 0x6000,
    M_MESH_BONE_ASSIGNMENT  = 0x7000,
    M_MESH_LOD_LEVEL        = 0x8000,
    M_MESH_BOUNDS           = 0x9000,
    M_SUBMESH_NAME_TABLE    = 0xA000,
    M_EDGE_LISTS            = 0xB000,
    M_POSES                 = 0xC000,
    M_ANIMATIONS            = 0xD000,
    M_TABLE_EXTREMES        = 0xE000,
};

void EarlyMeshAnalyzer::discoverLinkedResources(MeshPtr& mesh)
{
    // Materials and skeleton aren't known until the mesh is `load()`-ed.
    // `MeshSerializerImpl` takes care of properly parsing the binary mesh format.
    // To find the info early, we're on our own.
    DataStreamPtr stream = mesh->copyPreparedMeshFileData();
    stream->seek(0);

    // ----- see `MeshSerializerImpl::importMesh()` -----
    determineEndianness(stream);
    readFileHeader(stream);
    pushInnerChunk(stream);

    unsigned short streamID = readChunk(stream);
    while(!stream->eof())
    {
        switch (streamID)
        {
        case M_MESH:
            readMesh(stream);
            break;
        }

        streamID = readChunk(stream);
    }

    popInnerChunk(stream);
}

/// stream overhead = ID + size
const long MSTREAM_OVERHEAD_SIZE = sizeof(uint16) + sizeof(uint32);

void EarlyMeshAnalyzer::readMesh(DataStreamPtr& stream)
{
    // ----- see `MeshSerializerImpl::readMesh()` -----
    readBools(stream, &skeletallyAnimated, 1);

    // Find all substreams
    if (!stream->eof())
    {
        pushInnerChunk(stream);
        unsigned short streamID = readChunk(stream);
        while(!stream->eof() &&
            (streamID == M_GEOMETRY ||
                streamID == M_SUBMESH ||
                streamID == M_MESH_SKELETON_LINK ||
                streamID == M_MESH_BONE_ASSIGNMENT ||
                streamID == M_MESH_LOD_LEVEL ||
                streamID == M_MESH_BOUNDS ||
                streamID == M_SUBMESH_NAME_TABLE ||
                streamID == M_EDGE_LISTS ||
                streamID == M_POSES ||
                streamID == M_ANIMATIONS ||
                streamID == M_TABLE_EXTREMES))
        {
            size_t dataStartPos = stream->tell();
            switch(streamID)
            {
            case M_GEOMETRY:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_SUBMESH:
                materialNames.push_back(readString(stream)); // see `MeshSerializerImpl::readSubMesh()`
                stream->skip(mCurrentstreamLen - (MSTREAM_OVERHEAD_SIZE + (stream->tell() - dataStartPos)));
                break;
            case M_MESH_SKELETON_LINK:
                skeletonNames.push_back(readString(stream)); // see `MeshSerializerImpl::readSkeletonLink()`
                break;
            case M_MESH_BONE_ASSIGNMENT:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_MESH_LOD_LEVEL:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_MESH_BOUNDS:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_SUBMESH_NAME_TABLE:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_EDGE_LISTS:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_POSES:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_ANIMATIONS:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            case M_TABLE_EXTREMES:
                stream->skip(mCurrentstreamLen - MSTREAM_OVERHEAD_SIZE);
                break;
            }

            if (!stream->eof())
            {
                streamID = readChunk(stream);
            }

        }
        if (!stream->eof())
        {
            // Backpedal back to start of stream
            backpedalChunkHeader(stream);
        }
        popInnerChunk(stream);
    }
}
