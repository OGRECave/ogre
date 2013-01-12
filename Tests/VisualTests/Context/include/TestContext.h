/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

class TestBatch;

/** The common environment that all of the tests run in */
class TestContext : public OgreBites::SampleContext
{
public:

    TestContext(int argc, char** argv);
    virtual ~TestContext();

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
     *        @param set The name of the test set to load
     *        @return The initial tets or sample to run */
    OgreBites::Sample* loadTests(Ogre::String set);

    /** Setup the Root */
    virtual void createRoot();

    /** Start it up */
    virtual void go(OgreBites::Sample* initialSample = 0);

    /** Handles the config dialog */
    virtual bool oneTimeConfig();

    /** Set up directories for the tests to output to */
    virtual void setupDirectories(Ogre::String batchName);

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
    
    // The timestep
    Ogre::Real mTimestep;

    // Path to the test plugin directory
    Ogre::String mPluginDirectory;

    // List of available test sets
    std::map<Ogre::String, Ogre::StringVector> mTestSets;

    // The tests to be run
    std::deque<OgreBites::Sample*> mTests;
    
    // Path to the output directory for the running test
    Ogre::String mOutputDir;

    // The active test (0 if none is active)
    VisualTest* mCurrentTest;

    // The current frame of a running test
    unsigned int mCurrentFrame;

    // Info about the running batch of tests
    TestBatch* mBatch;

    // command line options
    bool mReferenceSet; // is a reference set being generated
    bool mGenerateHtml; // should html output be created
    bool mForceConfig; // force the config dialog
    bool mHelp; // show usage details
    Ogre::String mRenderSystemName; // render system to use
    Ogre::String mBatchName;// optional name for this batch
    Ogre::String mCompareWith;// set to compare against
    Ogre::String mComment;// optional comment
    Ogre::String mTestSetName;// name of the test set to use
    Ogre::String mSummaryOutputDir;// location to output a test summary (used for CTest)

};

#endif
