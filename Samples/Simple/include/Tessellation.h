#ifndef __Tessellation_H__
#define __Tessellation_H__

#include "SdkSample.h"
#include "OgreImage.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Tessellation : public SdkSample
{
 public:

    Sample_Tessellation()
    {
        mInfo["Title"] = "Tessellation";
        mInfo["Description"] = "Sample for tessellation support (Hull, Domain shaders)";
        mInfo["Thumbnail"] = "thumb_tessellation.png";
        mInfo["Category"] = "ShaderFeatures";
        mInfo["Help"] = "Top Left: Multi-frame\nTop Right: Scrolling\nBottom Left: Rotation\nBottom Right: Scaling";
    }

    void testCapabilities(const RenderSystemCapabilities* caps) override
    {
        requireMaterial("Ogre/TessellationExample");
    }

 protected:

    void setupContent() override
    {
        // set our camera
        mTrayMgr->showCursor();
        mCameraNode->setPosition(0, 0, 20);
        mCameraMan->setStyle(CS_ORBIT);

        // create a plain with float3 tex cord
        ManualObject* tObject = mSceneMgr->createManualObject("TesselatedObject");

        // create a triangle that uses our material
        tObject->begin("Ogre/TessellationExample", RenderOperation::OT_TRIANGLE_LIST);
        tObject->position(-10, -10, 0);
        tObject->position(0, 10, 0);
        tObject->position(10, -10, 0);
        tObject->end();

        // attach it to a node and position appropriately
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(tObject);
    }
};

#endif
