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
    std::unique_ptr<Rectangle2D> mRect;

    Sample_AtomicCounters()
    {
        mInfo["Title"] = "Atomic Counters";
        mInfo["Description"] = "An example of using atomic counters to visualise GPU rasterization order";
        mInfo["Thumbnail"] = "thumb_atomicc.png";
        mInfo["Category"] = "Unsorted";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_READ_WRITE_BUFFERS))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support atomic counters, "
                        "so you cannot run this sample. Sorry!");
        }
    }

    bool frameEnded(const FrameEvent& evt)
    {
        GpuProgramManager::getSingleton().getSharedParameters("CounterBuffer")->setNamedConstant("ac", 0);
        return true;
    }

    void setupContent()
    {
        mViewport->setBackgroundColour(ColourValue(0.3, 0.3, 0.3));

        float w = 480.0 / mWindow->getWidth();
        float h = 480.0 / mWindow->getHeight();

        mRect.reset(new Rectangle2D);
        mRect->setCorners(-w, h, w, -h);
        mRect->setBoundingBox(AxisAlignedBox::BOX_INFINITE);

        MaterialPtr mat = MaterialManager::getSingleton().getByName("Example/RasterizationOrder");
        mRect->setMaterial(mat);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mRect.get());
    }
};
