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
        mInfo["Category"] = "ShaderFeatures";
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
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(0), 30);

        mOgreEnt = mSceneMgr->createEntity(SceneManager::PT_PLANE);

        mOgreEnt->setMaterial(MaterialManager::getSingleton().getByName("Compute/Show"));
        SceneNode* ogre = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ogre->setScale(0.1,0.1,0.1);
        ogre->attachObject(mOgreEnt);

        CompositorManager::getSingleton().addCompositor(mViewport, "Compute");
        CompositorManager::getSingleton().setCompositorEnabled(mViewport, "Compute", true);
    }
}
