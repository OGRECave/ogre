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
#include "SdkSample.h"
#include "SamplePlugin.h"
#include "OgreRectangle2D.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_AtomicCounters : public SdkSample
{
 public:
    Sample_AtomicCounters()
    {
        mInfo["Title"] = "Atomic Counters";
        mInfo["Description"] = "An example of using atomic counters to visualise GPU rasterization order";
        mInfo["Thumbnail"] = "thumb_atomicc.png";
        mInfo["Category"] = "Unsorted";
    }

    void testCapabilities(const RenderSystemCapabilities* caps) override
    {
        requireMaterial("Example/RasterizationOrder");
    }

    bool frameEnded(const FrameEvent& evt) override
    {
        GpuProgramManager::getSingleton().getSharedParameters("CounterBuffer")->setNamedConstant("ac", 0);
        return true;
    }

    void setupContent() override
    {
        mViewport->setBackgroundColour(ColourValue(0.3, 0.3, 0.3));

        float w = 480.0 / mWindow->getWidth();
        float h = 480.0 / mWindow->getHeight();

        auto rect = mSceneMgr->createScreenSpaceRect();
        rect->setCorners(-w, h, w, -h, false);

        MaterialPtr mat = MaterialManager::getSingleton().getByName("Example/RasterizationOrder");
        rect->setMaterial(mat);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(rect);
    }
};
