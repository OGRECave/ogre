// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#pragma once

#include "OgreColourValue.h"
#include "OgreEntity.h"
#include "OgreHardwareBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreLogManager.h"
#include "OgreMath.h"
#include "OgrePixelFormat.h"
#include "OgrePlatform.h"
#include "OgrePrerequisites.h"
#include "SdkSample.h"
#include "SamplePlugin.h"
#include <algorithm>
#include <numeric>

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_GaussianSplatting : public SdkSample
{
    std::vector<int> mIndices;
    std::vector<Vector3f> mPositions;
    Entity* mSplat;
    HardwareIndexBufferPtr mIndexBuffer;
    Vector3f mLastCamPos;
public:
    Sample_GaussianSplatting()
    {
        mInfo["Title"] = "Gaussian Splatting";
        mInfo["Description"] = "Shows how to render 3D gaussian splats";
        mInfo["Thumbnail"] = "thumb_gsplatting.png";
        mInfo["Category"] = "Geometry";
    }

    void testCapabilities(const RenderSystemCapabilities* caps) override
    {
        requireMaterial("Example/GaussianSplatting");
    }

    bool frameStarted(const FrameEvent& e) override
    {
        // sort by distance to camera
        Vector3f camPos(mCamera->getDerivedPosition());

        if((camPos - mLastCamPos).squaredLength() < 0.01)
            return true;

        mLastCamPos = camPos;

        std::vector<float> distances(mIndices.size());
        for (size_t i = 0; i < mIndices.size(); ++i)
            distances[i] = mPositions[i].dotProduct(camPos); // project on camera vector

        // argsort
        std::sort(mIndices.begin(), mIndices.end(), [&distances](int a, int b) { return distances[a] < distances[b]; });

        mIndexBuffer->writeData(0, mIndexBuffer->getSizeInBytes(), mIndices.data(), true);
        return true;
    }

    void setupContent() override
    {
        mViewport->setBackgroundColour(ColourValue(0.5, 0.5, 0.5));

        mSplat = mSceneMgr->createEntity("lego.mesh");
        mSplat->setMaterialName("Example/GaussianSplatting");
        auto splatnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        splatnode->attachObject(mSplat);

        // get positions for sorting
        auto sub = mSplat->getMesh()->getSubMesh(0);
        auto vertexBuffer = sub->vertexData->vertexBufferBinding->getBuffer(0);
        auto numVertices = sub->vertexData->vertexCount;

        mPositions.resize(numVertices);
        {
            HardwareBufferLockGuard data(vertexBuffer, HardwareBuffer::HBL_READ_ONLY);
            PixelUtil::bulkPixelConversion(data.pData, PF_FLOAT16_RGB, mPositions.data(), PF_FLOAT32_RGB, numVertices);
        }

        // create index buffer
        auto idata = sub->indexData;
        idata->indexCount = numVertices;
        mIndexBuffer = vertexBuffer->getManager()->createIndexBuffer(HardwareIndexBuffer::IT_32BIT, numVertices, HBU_CPU_TO_GPU);
        idata->indexBuffer = mIndexBuffer;
        mIndices.resize(numVertices);
        std::iota(mIndices.begin(), mIndices.end(), 0);

        mCamera->setNearClipDistance(0.4);
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(-75), 3);
        mCameraNode->setFixedYawAxis(true, Vector3::UNIT_Z);

        mTrayMgr->createCheckBox(TL_TOPLEFT, "points", "Point Rendering")->setChecked(false, false);
        mTrayMgr->showCursor();
    }

    void checkBoxToggled(CheckBox* box) override
    {
        mSplat->setMaterialName(box->isChecked() ? "Example/Pointcloud" : "Example/GaussianSplatting");
    }
};
