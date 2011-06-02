/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "SdkSample.h"

/** The base class for a visual test scene */
class VisualTest : public OgreBites::Sample
{
public:

    // resource group that will be automatically unloaded after the close of the sample
    static Ogre::String TRANSIENT_RESOURCE_GROUP;

    VisualTest()
        :mFrameNr(0)
    {
        mInfo["Title"] = "Untitled Test";
        mInfo["Description"] = "";
        mInfo["Category"] = "Tests";
        mInfo["Thumbnail"] = "thumb_visual_tests.png";
        mInfo["Help"] = "";

        Ogre::ResourceGroupManager& rgm = Ogre::ResourceGroupManager::getSingleton();
        if (!rgm.resourceGroupExists(TRANSIENT_RESOURCE_GROUP))
            rgm.createResourceGroup(TRANSIENT_RESOURCE_GROUP);
    }

    virtual ~VisualTest() {}

    /** Called at the start of each test frame; increments frame count
     *    and handles any other housekeeping details for the test. */
    void testFrameStarted()
    {
        ++mFrameNr;
    }

    /** Called at the end of each test frame, takes the actual screenshots
     *    and signals that the test is done after the final shot is complete. */
    void testFrameEnded()
    {
        // if all the shots have been taken, the test can exit
        if(mScreenshotFrames.empty())
        {
            mDone = true;
        }
        else if(*(mScreenshotFrames.begin()) == mFrameNr)
        {
            Ogre::String filename = mInfo["Title"] + 
                "_VisualTest_"+Ogre::StringConverter::toString(mFrameNr)+".png";
            mWindow->writeContentsToFile(filename);
            mScreenshotFrames.erase(mScreenshotFrames.begin());
        }
    }

    /** Adds a screenshot frame to the list - this should
     *    be done during setup of the test. */
    void addScreenshotFrame(unsigned int frame)
    {
        mScreenshotFrames.insert(frame);
    }

    /** Does some basic setup tasks */
    virtual void _setup(Ogre::RenderWindow* window, OIS::Keyboard* keyboard, 
        OIS::Mouse* mouse, OgreBites::FileSystemLayer* fsLayer)
    {
        OgreBites::Sample::_setup(window, keyboard, mouse, fsLayer);

        // reset frame count
        mFrameNr = 0;
        
        // always take a screenshot after 1000 frames for now, for testing...
        addScreenshotFrame(1000);
    }

    /** Clean up */
    virtual void _shutdown()
    {
        mSceneMgr->destroyCamera(mCamera);
        OgreBites::Sample::_shutdown();
    }

    /** set up the camera and viewport */
    virtual void setupView()
    {
        mCamera = mSceneMgr->createCamera("MainCamera");
        mViewport = mWindow->addViewport(mCamera);
        mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
        mCamera->setNearClipDistance(0.5f);
        mCamera->setFarClipDistance(10000.f);
    }

    /** Unload all resources used by this sample */
    virtual void unloadResources()
    {
        Ogre::ResourceGroupManager::getSingleton().clearResourceGroup(TRANSIENT_RESOURCE_GROUP);
        Sample::unloadResources();
    }

    /** Changes aspect ratio to match any window resizings */
    virtual void windowResized(Ogre::RenderWindow* rw)
    {
        mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
    }

protected:

    // The current frame number - in order to keep things deterministic, 
    // we track frame numbers, rather than actual timings.
    unsigned int mFrameNr;
    
    // a set of frame numbers at which to trigger screenshots
    std::set<unsigned int> mScreenshotFrames;

    // The camera for this sample
    Ogre::Camera* mCamera;
    // The viewport for this sample
    Ogre::Viewport* mViewport;

};

#endif
