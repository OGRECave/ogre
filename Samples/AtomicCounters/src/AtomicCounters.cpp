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

static SamplePlugin* sp;
static Sample* s;

class _OgreSampleClassExport Sample_AtomicCounters : public SdkSample
{
    Entity* mOgreEnt;

    TexturePtr mImage;
    HardwarePixelBufferSharedPtr mPixelBuffer;

    //HardwareCounterBufferSharedPtr mBuffer;

 public:

    Sample_AtomicCounters()
    {
        mInfo["Title"] = "Atomic Counters";
        mInfo["Description"] = "An example of using atomic counters to visualise GPU rasterization order";
        mInfo["Thumbnail"] = "thumb_atomicc.png";
        mInfo["Category"] = "Unsorted";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "Sample currently under construction.  Try again soon!",
                    "Sample_Compute::testCapabilities");

        if (!caps->hasCapability(RSC_ATOMIC_COUNTERS))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support atomic counters, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_Compute::testCapabilities");
        }
        else if (!caps->hasCapability(RSC_COMPUTE_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support compute programs, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_Compute::testCapabilities");
        }
        else if (!caps->hasCapability(RSC_TESSELLATION_HULL_PROGRAM) || !caps->hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support tesselation programs, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_Compute::testCapabilities");
        }
        else if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support geometry programs, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_Compute::testCapabilities");
        }
    }

    // Just override the mandatory create scene method
    void setupContent(void)
    {
        mCameraNode->setPosition(0, 0, -40);
        mCamera->lookAt(0,0,0);
        // mCamera->setNearClipDistance(0.1);
        // mCamera->setFarClipDistance(100);

        mOgreEnt = mSceneMgr->createEntity("PlainHead", "ogrehead.mesh");
        mOgreEnt->setMaterialName("AtomicCounters");
        //mOgreEnt->setMaterialName("BaseWhiteNoLighting");
        SceneNode* ogre = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ogre->setPosition(50, -50, 140);
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
            PF_X8R8G8B8  // Pixel format  //TODO support formats from GL3+
            //TU_DYNAMIC   // usage
        );

        mImage->createShaderAccessPoint(0);

        mPixelBuffer = mImage->getBuffer(0,0);

        // Lock the buffer so we can write to it.
        mPixelBuffer->lock(HardwareBuffer::HBL_DISCARD);
        const PixelBox &pb = mPixelBuffer->getCurrentLock();

        // Update the contents of pb here
        // Image data starts at pb.data and has format pb.format
        // Here we assume data.format is PF_X8R8G8B8 so we can address pixels as uint32.
        uint *data = reinterpret_cast<uint*>(pb.data);
        size_t height = pb.getHeight();
        size_t width = pb.getWidth();
        size_t pitch = pb.rowPitch; // Skip between rows of image
        for (size_t y = 0; y < height; ++y)
        {
            for(size_t x = 0; x < width; ++x)
            {
                // 0xXXRRGGBB -> fill the buffer with yellow pixels
                // data[pitch * y + x] = 0x00FFFF00;
                data[pitch * y + x] = 0x0087CEEB;
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
        Real seconds = (Real)(Root::getSingleton().getTimer()->getMilliseconds()) / 1000.0;
        Pass* renderPass = mOgreEnt->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);
        if (renderPass->hasFragmentProgram())
        {
            Ogre::GpuProgramParametersSharedPtr pParams = renderPass->getFragmentProgramParameters();
            if ( pParams.isNull() )
            {
                //printf("SAD PANDA!");
            }
            else
            {
                if ( pParams->_findNamedConstantDefinition( "ColourMe[0]" ) )
                {
                    Vector4 constParam = Ogre::Vector4(0.5, 0.1, 0.0, 1.0);
                    renderPass->getFragmentProgramParameters()->setNamedConstant("ColourMe[0]", constParam);

                    Vector4 timeParam = Ogre::Vector4(
                        Ogre::Math::Sin(seconds)*0.5, 0.0, Ogre::Math::Cos(seconds)*0.5, 0.0);
                    renderPass->getFragmentProgramParameters()->setNamedConstant("ColourMe[1]", timeParam);
                }
                const Ogre::GpuConstantDefinition* atom_counter_def;
                if ( (atom_counter_def = &pParams->getConstantDefinition("atom_counter")) )
                {
                    //TODO lock buffer, retrieve counter value similar to compute above
                    //const uint* counter = pParams->getUnsignedIntPointer(atom_counter_def->physicalIndex);
                    //const uint* counter2 = ;
                    //std::cout << "FOUND THE ATOMS: " << *counter << " " << std::endl; //<< *counter2 << std::endl;
                }
            }
        }

        // renderPass->getFragmentProgramParameters()->getConstantDefinition("atom_counter").getValue();
        return SdkSample::frameRenderingQueued(evt);
    }
};
