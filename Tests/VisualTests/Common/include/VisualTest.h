/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __VisualTest_H__
#define __VisualTest_H__

#include "OgreBuildSettings.h"

#include "Sample.h"
#include "Ogre.h"

// resource group that will be automatically unloaded after the close of the sample
#define TRANSIENT_RESOURCE_GROUP "VisualTestTransient"
#define ASSETS_RESOURCE_GROUP "General"

/** The base class for a visual test scene */
class VisualTest : public OgreBites::Sample
{
 public:

    VisualTest()
    {
        mInfo["Category"] = "Tests";
        mInfo["Thumbnail"] = "thumb_visual_tests.png";
        Ogre::ResourceGroupManager& rgm = Ogre::ResourceGroupManager::getSingleton();
        if (!rgm.resourceGroupExists(TRANSIENT_RESOURCE_GROUP))
            rgm.createResourceGroup(TRANSIENT_RESOURCE_GROUP, true);
    }

    /** set up the camera and viewport */
    void setupView() override
    {
        mCamera = mSceneMgr->createCamera("MainCamera");
        mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mCameraNode->attachObject(mCamera);
        mCameraNode->setFixedYawAxis(true); // fix lookAt calls
        mViewport = mWindow->addViewport(mCamera);
        mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
        mCamera->setNearClipDistance(0.5f);
        mCamera->setFarClipDistance(10000.f);
    }

    /** Unload all resources used by this sample */
    void unloadResources() override
    {
#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
        auto& rtShaderGen = Ogre::RTShader::ShaderGenerator::getSingleton();
        auto schemRenderState = rtShaderGen.getRenderState(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
        schemRenderState->reset();
#endif
        Ogre::ResourceGroupManager::getSingleton().clearResourceGroup(TRANSIENT_RESOURCE_GROUP);
        Sample::unloadResources();
        mAnimStateList.clear();
    }

    /** Default frame started callback, advances animations */
    bool frameStarted(const Ogre::FrameEvent& evt) override
    {
        for(unsigned int i = 0; i < mAnimStateList.size(); ++i)
            mAnimStateList[i]->addTime(evt.timeSinceLastFrame);
        return true;
    }

    bool keyPressed(const OgreBites::KeyboardEvent& evt) override
    {
        if (evt.keysym.sym == OgreBites::SDLK_F6)
            mCamera->getViewport()->getTarget()->writeContentsToTimestampedFile("screenshot", ".png");

        return true;
    }

 protected:
    // a list of animation states to automatically update
    std::vector<Ogre::AnimationState*> mAnimStateList;
};

#endif
