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

using namespace Ogre;
using namespace OgreBites;

SamplePlugin* sp;
Sample* s;

class _OgreSampleClassExport Sample_Compute : public SdkSample
{
    Entity* mOgreEnt;

    TexturePtr mImage;
    HardwarePixelBufferSharedPtr mPixelBuffer;

    //HardwareCounterBufferSharedPtr mBuffer;

 public:
        
    Sample_Compute() : mOgreEnt(NULL)
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

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_COMPUTE_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support compute programs, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_Compute::testCapabilities");
        }
    }

    // Just override the mandatory create scene method
    void setupContent(void)
    {
        mCamera->setPosition(0, 0, -40);
        mCamera->lookAt(0,0,0);
        // mCamera->setNearClipDistance(0.1);
        // mCamera->setFarClipDistance(100);
        
        Ogre::MovablePlane plane(Ogre::Vector3::UNIT_Z, 10);

        Ogre::MeshManager::getSingleton().createPlane("PlaneMesh", "General", plane, 20, 20);
        mOgreEnt = mSceneMgr->createEntity("Plane", "PlaneMesh");
        mOgreEnt->setMaterialName("Compute");
        //mOgreEnt->setMaterialName("BaseWhiteNoLighting");
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
            PF_B8G8R8A8,  // Pixel format  //TODO support formats from GL3+
            TU_DYNAMIC   // usage
        );

        mImage->createShaderAccessPoint(0);

        mPixelBuffer = mImage->getBuffer(0,0);

        // Lock the buffer so we can write to it.
        mPixelBuffer->lock(HardwareBuffer::HBL_DISCARD);
        const PixelBox &pb = mPixelBuffer->getCurrentLock();

        // Update the contents of pb here
        // Image data starts at pb.data and has format pb.format
        // Here we assume data.format is PF_X8R8G8B8 so we can address pixels as uint32.
        uint *data = static_cast<uint*>(pb.data);
        size_t height = pb.getHeight();
        size_t width = pb.getWidth();
        size_t pitch = pb.rowPitch; // Skip between rows of image
        for (size_t y = 0; y < height; ++y)
        {
            for(size_t x = 0; x < width; ++x)
            {
                // 0xXXRRGGBB -> fill the buffer with yellow pixels
                data[pitch * y + x] = 0x00FFFF00;
            }
        }

        // Unlock the buffer again (frees it for use by the GPU)
        mPixelBuffer->unlock();
    }

    void cleanupContent()
    {
        // Read image load/store data.
//        mPixelBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
//        const PixelBox &pb = mPixelBuffer->getCurrentLock();
//        uint *data = static_cast<uint*>(pb.data);
//        size_t height = pb.getHeight();
//        size_t width = pb.getWidth();
//        size_t pitch = pb.rowPitch; // Skip between rows of image
//        printf("Buffer values.\n");
//         for (size_t y = 0; y < height; ++y)
//         {
//             for(size_t x = 0; x < width; ++x)
//             {
//                 std::cout << " " << std::hex << data[pitch * y + x];
//             }
//             std::cout << std::endl;
//         }
//        std::cout << std::hex << data[0];
//        std::cout << " " << data[1] << std::endl;
//        mPixelBuffer->unlock();

        //MeshManager::getSingleton().remove(mTetrahedraMesh->getName());
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
        /*
        Real seconds = (Real)(Root::getSingleton().getTimer()->getMilliseconds()) / 1000.0;
        Pass* renderPass = mOgreEnt->getSubEntity(0)->getMaterial()->getBestTechnique()->getPass(0);

        Ogre::GpuProgramParametersSharedPtr pParams = renderPass->getComputeProgramParameters();
        pParams->setNamedConstant("roll", Real(fmod(seconds, 1.0)));
         */
        return SdkSample::frameRenderingQueued(evt); 
    }
};

#ifndef OGRE_STATIC_LIB

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_Compute;
    sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
    sp->addSample(s);
    Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Root::getSingleton().uninstallPlugin(sp); 
    OGRE_DELETE sp;
    delete s;
}

#endif
