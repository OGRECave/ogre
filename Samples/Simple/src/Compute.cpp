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
#include "Compute.h"

namespace OgreBites {
    Sample_Compute::Sample_Compute() : mOgreEnt(NULL)
    { 
        mInfo["Title"] = "Compute";
        mInfo["Description"] = "A basic example of the compute shader.";
        mInfo["Thumbnail"] = "thumb_compute.png";
        mInfo["Category"] = "Tests";
        mInfo["Help"] = "The shader is executed in groups of 16x16 workers\n"
                "in total there are 16x16 groups forming a grid of 256x256\n"
                "each worker writes the color based on the local id\n"
                "the sine overlay is based on the global id";
    }

    void Sample_Compute::testCapabilities(const RenderSystemCapabilities* caps)
    {
        requireMaterial("Compute/Compositor");
    }

    // Just override the mandatory create scene method
    void Sample_Compute::setupContent(void)
    {
        mCameraNode->setPosition(0, 0, -40);
        mCameraNode->lookAt(Vector3(0,0,0), Node::TS_PARENT);
        // mCamera->setNearClipDistance(0.1);
        // mCamera->setFarClipDistance(100);
        
        Ogre::MovablePlane plane(Ogre::Vector3::UNIT_Z, 10);

        Ogre::MeshManager::getSingleton().createPlane("PlaneMesh", "General", plane, 20, 20);
        mOgreEnt = mSceneMgr->createEntity("Plane", "PlaneMesh");

        MaterialPtr mat = MaterialManager::getSingleton().getByName("Compute/Show");
        mOgreEnt->setMaterial(mat);
        SceneNode* ogre = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ogre->setPosition(0, 0, 0);
        ogre->setDirection(0,0,1);
        ogre->attachObject(mOgreEnt);

        ////////////////////////////////
        // Image Load/Store
        ////////////////////////////////

        mImage = TextureManager::getSingleton().createManual(
            "ImageData", // Name of texture
            "General",   // Name of resource group in which the texture should be created
            TEX_TYPE_2D, // Texture type
            256, 256, 1, // Width, Height, Depth
            0,           // Number of mipmaps
            PF_BYTE_RGBA, // Pixel format
            TU_DYNAMIC   // usage
        );

        // make it accessible to the compute shader (write)
        mImage->createShaderAccessPoint(0);
        // make it accessible for normal sampling (read)
        mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(mImage);

        mPixelBuffer = mImage->getBuffer(0,0);

        // Lock the buffer so we can write to it.
        mPixelBuffer->lock(HardwareBuffer::HBL_DISCARD);
        const PixelBox &pb = mPixelBuffer->getCurrentLock();

        // Update the contents of pb here
        // Image data starts at pb.data and has format pb.format
        // we can address pixels as uint32.
        {
            uint* data = reinterpret_cast<uint*>(pb.data);
            size_t height = pb.getHeight();
            size_t width = pb.getWidth();

            // 0xAABBGGRR -> fill the buffer with yellow pixels
            std::fill(data, data + width * height, 0x0000FFFF);

            // Unlock the buffer again (frees it for use by the GPU)
            mPixelBuffer->unlock();
        }

        CompositorManager::getSingleton().addCompositor(mViewport, "Compute");
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, "Compute", true);
    }
}
