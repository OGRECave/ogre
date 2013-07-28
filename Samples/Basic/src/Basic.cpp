/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
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

class _OgreSampleClassExport Sample_Basic : public SdkSample
{
    Entity* mOgreEnt;

    //HardwareCounterBufferSharedPtr mBuffer;

 public:
        
    Sample_Basic() 
    { 
        mInfo["Title"] = "Basic";
        mInfo["Description"] = "A basic example using every available shader type.";
        mInfo["Thumbnail"] = "thumb_basic.png";
        mInfo["Category"] = "Tests";
    }

    void testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support geometry programs, "
                        "so you cannot run this sample. Sorry!", 
                        "Sample_Basic::testCapabilities");
        }
        //TODO check for tessellation shaders and compute shaders too
    }

    // Just override the mandatory create scene method
    void setupContent(void)
    {
        mCamera->setPosition(0, 0, -40);
        mCamera->lookAt(0,0,0);
        // mCamera->setNearClipDistance(0.1);
        // mCamera->setFarClipDistance(100);
        
        mOgreEnt = mSceneMgr->createEntity("PlainHead", "ogrehead.mesh");
        mOgreEnt->setMaterialName("JAJ/Basic");
        //mOgreEnt->setMaterialName("BaseWhiteNoLighting");
        SceneNode* ogre = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ogre->setPosition(50, -50, 140);
        ogre->setDirection(0,0,1);
        ogre->attachObject(mOgreEnt);

        //mBuffer = HardwareBufferManager::getSingleton().createCounterBuffer(sizeof(uint32), HardwareBuffer::HBU_WRITE_ONLY, false);
    }

    void cleanupContent()
    {
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
                    const uint* counter = pParams->getUnsignedIntPointer(atom_counter_def->physicalIndex);
                    //const uint* counter2 = ;
                    std::cout << "FOUND THE ATOMS: " << *counter << " " << std::endl; //<< *counter2 << std::endl;
                }
            }
        }

        // renderPass->getFragmentProgramParameters()->getConstantDefinition("atom_counter").getValue();
        return SdkSample::frameRenderingQueued(evt); 
    }
};

#ifndef OGRE_STATIC_LIB

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_Basic;
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
