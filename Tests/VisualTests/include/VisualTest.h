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

#define VISUAL_TEST_TIMESTEP 0.01f

/** The base class for a visual test scene */
class VisualTest : public OgreBites::SdkSample
{
public:

    static Ogre::String TRANSIENT_RESOURCE_GROUP;

    VisualTest()
        :mFrameNr(0)
    {
        mInfo["Title"] = "Untitled Test";
        mInfo["Description"] = "";
        mInfo["Category"] = "Unsorted";
        mInfo["Thumbnail"] = "";
        mInfo["Help"] = "";
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
        if(*(mScreenshotFrames.begin()) == mFrameNr)
        {
            Ogre::String filename = "TestShots/" + mInfo["Title"] + 
                "_VisualTest_"+Ogre::StringConverter::toString(mFrameNr)+".png";
            mWindow->writeContentsToFile(filename);
            mScreenshotFrames.erase(mScreenshotFrames.begin());
        }

        // if all the shots have been taken, the test can exit
        if(mScreenshotFrames.empty())
            mDone = true;
    }

    /** Adds a screenshot frame to the list - this should
     *    be done during setup of the test. */
    void addScreenshotFrame(unsigned int frame)
    {
        mScreenshotFrames.insert(frame);
    }

    virtual bool frameStarted(const Ogre::FrameEvent& evt)
    {
        // temporary, this will be called by the TestContext once it's set up
        testFrameStarted();
        return true;
    }

    virtual bool frameEnded(const Ogre::FrameEvent& evt)
    {
        // temporary, this will be called by the TestContext once it's set up
        testFrameEnded();
        return true;
    }

    // NOTE: This is temporary, for the sake of making these work with the existing Sample Browser
    // the timing stuff will be moved to the TestContext, and the cameraman/tray stuff will be disabled entirely
    virtual void _setup(Ogre::RenderWindow* window, OIS::Keyboard* keyboard, 
        OIS::Mouse* mouse, OgreBites::FileSystemLayer* fsLayer)
    {
        OgreBites::SdkSample::_setup(window, keyboard, mouse, fsLayer);

        // rest frame count
        mFrameNr = 0;
        
        // Seed rand with a predictable value
        srand(5); // 5 is completely arbitrary, the idea is simply to use a constant

        // Give a fixed timestep for particles and other time-dependent things in OGRE
        Ogre::ControllerManager::getSingleton().setFrameDelay(VISUAL_TEST_TIMESTEP);

        // disable extraneous SdkSample stuff
        mTrayMgr->hideFrameStats();
        mTrayMgr->hideCursor();
        mCameraMan->setStyle(OgreBites::CS_MANUAL);
    }

    virtual void _shutdown()
    {
        // set frame delay back to zero, so the sample browser's
        // time dependent animations and such behave.
        Ogre::ControllerManager::getSingleton().setFrameDelay(0.f);
        SdkSample::_shutdown();
    }

protected:

    // The current frame number - in order to keep things deterministic, 
    // we track frame numbers, rather than actual timings.
    unsigned int mFrameNr;
    
    // a list of frame numbers at which to trigger screenshots
    std::set<unsigned int> mScreenshotFrames;

};

#endif
