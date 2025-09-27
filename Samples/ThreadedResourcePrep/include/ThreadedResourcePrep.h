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

#ifndef _Sample_ThreadedResourcePrep_H_
#define _Sample_ThreadedResourcePrep_H_

#include "SdkSample.h"
#include "SamplePlugin.h"

#include <Ogre.h>
#include <deque>

using namespace Ogre;
using namespace OgreBites;

class Sample_ThreadedResourcePrep;

struct MeshInfo
{
    // Each resource has dedicated spot, so they can appear in any order.
    String meshName;
    Vector3 meshScale;
    Degree meshYaw;
    SceneNode* sceneNode = nullptr;

    // Only used when mesh is currently loaded
    MeshPtr mesh;
    Entity* entity = nullptr;
};

struct Stats
{
    int totalReloads = 0;
    float lastReloadTime = 0.f;
    float avgReloadTime = 0.f;

    void recordReloadTime(float reloadTime)
    {
        totalReloads++;
        lastReloadTime = reloadTime;
        avgReloadTime = (totalReloads == 1) ? reloadTime : avgReloadTime * 0.5 + reloadTime * 0.5;
    }

    void resetStats()
    {
        totalReloads = 0;
        lastReloadTime = 0;
        avgReloadTime = 0;
    }
};

// Materials and skeleton linked from mesh file aren't known until the mesh is `load()`-ed.
// `MeshSerializerImpl` takes care of properly parsing the binary mesh format.
// To find the resources early (so we can `prepare()` them in time), we're on our own.
struct EarlyMeshAnalyzer: public Serializer
{
    EarlyMeshAnalyzer();
    void discoverLinkedResources(MeshPtr& mesh);

    StringVector materialNames;
    StringVector skeletonNames;
    bool skeletallyAnimated = false;

protected:
    void readMesh(DataStreamPtr& stream);
};

class LinkedResourcePrepper: public Resource::Listener
{
public:
    LinkedResourcePrepper(size_t queuePos, Sample_ThreadedResourcePrep* goal);
    ~LinkedResourcePrepper();

    void hookResource(ResourcePtr resource);
    void startPreparing();

    // Resource listener
    void preparingComplete(Resource*) override;

private:
    size_t mQueuePos = 0;
    std::vector<Ogre::ResourcePtr> mHookedResources;
    Sample_ThreadedResourcePrep* mGoal = nullptr;
    size_t mNumPrepared = 0; // accumulator
};

class _OgreSampleClassExport Sample_ThreadedResourcePrep : public SdkSample, public Resource::Listener
{
public:
    Sample_ThreadedResourcePrep();

    void loadMeshOnQueue(size_t i);

private:
    // SdkSample setup
    void setupContent() override;
    void cleanupContent() override;
    // Custom setup
    void setupSelectableMeshes();
    void defineSelectableMesh(String name, Vector3 pos, Vector3 scale, Degree yaw);
    void setupControls();
    // SdkSample UI
    void itemSelected(SelectMenu* menu) override;
    void buttonHit(Button* button) override;
    void sliderMoved(Slider* slider) override;
    void checkBoxToggled(CheckBox* box) override;
    // Custom UI
    void refreshStatsUi();
    void refreshMeshUi();
    void setThreadingUiVisible(bool vis);

    // The heavy lifting
    void performSyncUnload();
    void performSyncPrep();
    void performThreadedPrep();
    static void forceUnloadAllDependentResources(MeshPtr& mesh);
    static void forceUnloadAllDependentTextures(Pass* pass);
    void requestLinkedResourcesThreadedPrep(size_t i);

    // Resource listener
    void preparingComplete(Resource*) override;

    std::deque<MeshInfo> mMeshQueue;
    std::vector<std::unique_ptr<LinkedResourcePrepper>> mPreppers;
    SelectMenu* mMeshMenu;
    Label* mMeshStatLabel;
    Stats mStats;
    Button* mReloadBtn;
    Button* mUnloadBtn;
    Slider* mBatchSlider;
    size_t mBatchSize = 0;
    Timer mTimer;
    CheckBox* mThreadedMeshChk;
    CheckBox* mThreadedAllChk;

    // accumulators of `loadMeshOnQueue()`:
    long long mLoadingTotalMillis;
    size_t mNumLoadedMeshes;
};

#endif // _Sample_ThreadedResourcePrep_H_