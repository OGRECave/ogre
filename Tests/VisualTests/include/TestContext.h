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

#ifndef __TestContext_H__
#define __TestContext_H__

#include "SampleContext.h"
#include "VisualTest.h"

/** The common environment that all of the tests run in */
class TestContext : public OgreBites::SampleContext
{
public:

    TestContext() :mTimestep(0.01f) {}
    virtual ~TestContext() {}

    /** Does basic setup for the context */
    virtual void setup();

    /** Frame listener callback, handles updating of the tests at the start of frames 
     *        @param evt The frame event (passed in for the framelistener) */
    virtual bool frameStarted(const Ogre::FrameEvent& evt);

    /** Frame listener callback, handles updating of the tests at the end of frames 
     *        @param evt The frame event (passed in for the framelistener) */
    virtual bool frameEnded(const Ogre::FrameEvent& evt);

    /** Runs a given test or sample
     *        @param s The OgreBites::Sample to run 
     *        @remarks If s is a VisualTest, then timing and rand will be setup for
     *            determinism. */
    virtual void runSample(OgreBites::Sample* s);

    /** Loads test plugins
     *        @return The initial tets or sample to run */
    OgreBites::Sample* loadTests();

    /** Setup the Root */
    virtual void createRoot();

    /** Called after tests successfully complete, generates output */
    virtual void finishedTests();

    /** Sets the timstep value
     *        @param timestep The time to simulate elapsed between each frame 
     *        @remarks Use with care! Screenshots produced at different timesteps
     *            will almost certainly turn out different. */
    void setTimestep(Ogre::Real timestep);

    /** Gets the current timestep value */
    Ogre::Real getTimestep();

protected:

    // The tests to be run
    std::deque<OgreBites::Sample*> mTests;

    // The active test (0 if none is active)
    VisualTest* mCurrentTest;

    // The current frame of a running test
    unsigned int mCurrentFrame;

    // The timestep
    Ogre::Real mTimestep;

    // name of this set
    Ogre::String mTestSetName;
    
    // path to the output directory
    Ogre::String mOutputDir;
};

#endif
