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
/*
  -----------------------------------------------------------------------------
  Filename:    Isosurf.cpp
  Description: Demonstrates the use of the geometry shader to tessellate an 
  isosurface using marching tetrahedrons. Partial implementation of cg 
  Isosurf sample from NVIDIA's OpenGL SDK 10 : 
  http://developer.download.nvidia.com/SDK/10/opengl/samples.html
  -----------------------------------------------------------------------------
*/

#include "Isosurf.h"

namespace OgreBites {
    using namespace Ogre;

    Sample_Isosurf::Sample_Isosurf()
    { 
        mInfo["Title"] = "Isosurf";
        mInfo["Description"] = "A demo of procedural geometry manipulation using geometry shaders.";
        mInfo["Thumbnail"] = "thumb_isosurf.png";
        mInfo["Category"] = "Geometry";
    }

    void Sample_Isosurf::testCapabilities(const RenderSystemCapabilities* caps)
    {
        requireMaterial("Ogre/Isosurf/TessellateTetrahedra");
    }

    // Just override the mandatory create scene method
    void Sample_Isosurf::setupContent(void)
    {
        mCameraNode->setPosition(0, 0, -40);
        mCameraNode->lookAt(Vector3(0,0,0), Node::TS_PARENT);
        mCamera->setNearClipDistance(0.1);
        mCamera->setFarClipDistance(100);
        
        mTetrahedraMesh = ProceduralTools::generateTetrahedra();
        // Create tetrahedra and add it to the root scene node
        tetrahedra = mSceneMgr->createEntity("TetrahedraEntity", mTetrahedraMesh->getName());
        //tetrahedra->setDebugDisplayEnabled(true);
        Ogre::SceneNode* parentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        parentNode->attachObject(tetrahedra);
        parentNode->setScale(10,10,10);
    }

    void Sample_Isosurf::cleanupContent()
    {
        MeshManager::getSingleton().remove(mTetrahedraMesh->getName(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

    bool Sample_Isosurf::frameRenderingQueued(const FrameEvent& evt)
    {
        Real seconds = (Real)(Root::getSingleton().getTimer()->getMilliseconds()) / 1000.0;
        Ogre::Pass* renderPass = tetrahedra->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);
        if (renderPass->hasVertexProgram())
        {
            Ogre::Vector4 constParam = Ogre::Vector4(-0.5, 0.0, 0.0, 0.2);
            renderPass->getVertexProgramParameters()->setNamedConstant("Metaballs[0]", constParam);

            Ogre::Vector4 timeParam = Ogre::Vector4(
                0.1 + Ogre::Math::Sin(seconds)*0.5, Ogre::Math::Cos(seconds)*0.5, 0.0, 0.1);
            renderPass->getVertexProgramParameters()->setNamedConstant("Metaballs[1]", timeParam);
        }
        
        return SdkSample::frameRenderingQueued(evt); 
    }
};
