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

#include "TestContext.h"
#include "VisualTest.h"
#include "SamplePlugin.h"

#include "OgrePlatform.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_SYMBIAN
#include <coecntrl.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

void TestContext::setup()
{
    SampleContext::setup();
    // for now we just go right into the tests, eventually there'll be a menu screen beforehand
    runSample(loadTests());
}

OgreBites::Sample* TestContext::loadTests()
{
    OgreBites::Sample* startSample = 0;
    Ogre::StringVector sampleList;

    // Just the one for now:
    sampleList.push_back("VisualTests");

    // This should eventually be loaded from a config file or something to that effect
    Ogre::String sampleDir = "../lib/";

    for (Ogre::StringVector::iterator i = sampleList.begin(); i != sampleList.end(); i++)
    {
        try   // try to load the plugin
        {
            mRoot->loadPlugin(sampleDir + *i);
        }
        catch (Ogre::Exception e)   // plugin couldn't be loaded
        {
            continue;
        }

        Ogre::Plugin* p = mRoot->getInstalledPlugins().back();   // acquire plugin instance
        OgreBites::SamplePlugin* sp = dynamic_cast<OgreBites::SamplePlugin*>(p);

        if (!sp)
            continue;
        
        // go through every sample in the plugin...
        OgreBites::SampleSet newSamples = sp->getSamples();
        for (OgreBites::SampleSet::iterator j = newSamples.begin(); j != newSamples.end(); j++)
        {
            mTests.push_back(*j);
            Ogre::NameValuePairList& info = (*j)->getInfo();   // acquire custom sample info
            Ogre::NameValuePairList::iterator k;

            // give sample default title and category if none found
            k= info.find("Title");
            if (k == info.end() || k->second.empty()) info["Title"] = "Untitled";
            k = info.find("Category");
            if (k == info.end() || k->second.empty()) info["Category"] = "Unsorted";
            k = info.find("Thumbnail");
            if (k == info.end() || k->second.empty()) info["Thumbnail"] = "thumb_error.png";
        }

        startSample = *newSamples.begin();
    }

    return startSample;
}

bool TestContext::frameStarted(const Ogre::FrameEvent& evt)
{
    captureInputDevices();

    // pass a fixed timestep along to the tests
    Ogre::FrameEvent fixed_evt = Ogre::FrameEvent();
    fixed_evt.timeSinceLastFrame = mTimestep;
    fixed_evt.timeSinceLastEvent = mTimestep;

    if(mCurrentTest) // if a test is running
    {
        // handles timing
        mCurrentTest->testFrameStarted();
        // regular update function
        return mCurrentTest->frameStarted(fixed_evt);
    }
    else if(mCurrentSample) // if a generic sample is running
    {
        return mCurrentSample->frameStarted(evt);
    }

    // quit if nothing else is running
    return false;
}

bool TestContext::frameEnded(const Ogre::FrameEvent& evt)
{
    // pass a fixed timestep along to the tests
    Ogre::FrameEvent fixed_evt = Ogre::FrameEvent();
    fixed_evt.timeSinceLastFrame = mTimestep;
    fixed_evt.timeSinceLastEvent = mTimestep;

    if(mCurrentTest) // if a test is running
    {
        // handles screen captures and such
        mCurrentTest->testFrameEnded();

        // move onto the next test
        if(mCurrentTest->isDone())
        {
            runSample(0);
            return true;
        }

        // regular update function
        return mCurrentTest->frameEnded(fixed_evt);
    }
    else if(mCurrentSample) // if a generic sample is running
    {
        return mCurrentSample->frameEnded(evt);
    }

    // quit if nothing else is running
    return false;
}

void TestContext::runSample(OgreBites::Sample* s)
{
    // reset frame timing
    Ogre::ControllerManager::getSingleton().setFrameDelay(0);
    Ogre::ControllerManager::getSingleton().setTimeFactor(1.f);

    OgreBites::Sample* sampleToRun = s;

    // if a valid test is passed, then run it, if null, grab the next one from the deque
    if(s)
    {
        sampleToRun = s;
    }
    else if(!mTests.empty())
    {
        mTests.pop_front();
        if(!mTests.empty())
            sampleToRun = mTests.front();
    }

    // check if this is a VisualTest
    mCurrentTest = dynamic_cast<VisualTest*>(sampleToRun);

    // set things up to be deterministic
    if(mCurrentTest)
    {
       // Seed rand with a predictable value
        srand(5); // 5 is completely arbitrary, the idea is simply to use a constant

        // Give a fixed timestep for particles and other time-dependent things in OGRE
        Ogre::ControllerManager::getSingleton().setFrameDelay(mTimestep);
    }

    SampleContext::runSample(sampleToRun);
}

Ogre::Real TestContext::getTimestep()
{
    return mTimestep;
}

void TestContext::setTimestep(Ogre::Real timestep)
{
    // ensure we're getting a positive value
    mTimestep = timestep >= 0.f ? timestep : mTimestep;
}

// main, platform-specific stuff is copied from SampleBrowser and not guaranteed to work...

#if OGRE_PLATFORM != OGRE_PLATFORM_SYMBIAN    

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
#else
int main(int argc, char *argv[])
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
    [pool release];
    return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    mAppDelegate = [[AppDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:mAppDelegate];
    int retVal = NSApplicationMain(argc, (const char **) argv);

    [pool release];

    return retVal;
#else

    try
    {
        TestContext tc = TestContext();
        tc.go();
    }
    catch (Ogre::Exception& e)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occurred: " << e.getFullDescription().c_str() << std::endl;
#endif
    }

#endif
    return 0;
}

#endif // OGRE_PLATFORM != OGRE_PLATFORM_SYMBIAN    
