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

class _OgreSampleClassExport Sample_ThreadedResourcePrep : public SdkSample
{
public:
    Sample_ThreadedResourcePrep();

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
    // Custom UI
    void refreshStatsUi();
    void refreshMeshUi();

    // The heavy lifting
    void performReload();
    static void forceUnloadAllDependentResources(MeshPtr& mesh);
    static void forceUnloadAllDependentTextures(Pass* pass);

    std::deque<MeshInfo> mMeshQueue;
    SelectMenu* mMeshMenu;
    Label* mMeshStatLabel;
    Button* mReloadBtn;
    Slider* mBulkSlider;
    size_t mBulkSize = 0;
    Timer mTimer;
    Stats mStats;
};

#endif // _Sample_ThreadedResourcePrep_H_