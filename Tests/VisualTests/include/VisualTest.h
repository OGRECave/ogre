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
class VisualTest : public OgreBites::SdkSample
{
public:

    VisualTest()
        :mFrameNr(0)
    {
        mInfo["Title"] = "Untitled Test";
        mInfo["Description"] = "";
        mInfo["Category"] = "Unsorted";
        mInfo["Thumbnail"] = "";
        mInfo["Help"] = "";

        // Seed rand with a predictable value
        srand(5); // 5 is completely arbitrary, the idea is simply to use a constant
        // Give a fixed timestep for particles and other time-dependent things in OGRE
        Ogre::ControllerManager::getSingleton().setFrameDelay(0.01f);
    }

    virtual ~VisualTest() {}

    /** Called at the start of each test frame; increments frame count
     *    and handles any other housekeeping details for the test. */
    bool testFrameStarted(const Ogre::FrameEvent& evt)
    {
        ++mFrameNr;
        return true;
    }

    /** Called at the end of each test frame, takes the actual screenshots
     *    and signals that the test is done after the final shot is complete. */
    bool testFrameEnded(const Ogre::FrameEvent& evt)
    {
        // TODO: take screenshots here
        return true;
    }

    /** Adds a screenshot frame to the list - this should
     *    be done during setup of the test. */
    void addScreenshotFrame(unsigned int frame)
    {
        mScreenshotFrames.push_back(frame);
    }

protected:

    // The current frame number - in order to keep things deterministic, 
    // we track frame numbers, rather than actual timings.
    unsigned int mFrameNr;
    
    // a list of frame numbers at which to trigger screenshots
    std::list<unsigned int> mScreenshotFrames;

};

#endif
