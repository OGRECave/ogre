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
#ifndef __SampleContext_H__
#define __SampleContext_H__

#include "OgreApplicationContext.h"

#include "Sample.h"
#include "OgreRenderWindow.h"

namespace OgreBites
{
    /*=============================================================================
    | Base class responsible for setting up a common context for samples.
    | May be subclassed for specific sample types (not specific samples).
    | Allows one sample to run at a time, while maintaining a sample queue.
    =============================================================================*/
    class SampleContext : public ApplicationContext, public InputListener
    {
    public:
        Ogre::RenderWindow* mWindow;

        SampleContext(const Ogre::String& appName = OGRE_VERSION_NAME)
        : ApplicationContext(appName), mWindow(NULL)
        {
            mCurrentSample = 0;
            mSamplePaused = false;
            mLastRun = false;
            mLastSample = 0;
        }

        virtual Sample* getCurrentSample()
        {
            return mCurrentSample;
        }

        /*-----------------------------------------------------------------------------
        | Quits the current sample and starts a new one.
        -----------------------------------------------------------------------------*/
        virtual void runSample(Sample* s)
        {
            Ogre::Profiler* prof = Ogre::Profiler::getSingletonPtr();
            if (prof)
                prof->setEnabled(false);

            if (mCurrentSample)
            {
                mCurrentSample->_shutdown();    // quit current sample
                mSamplePaused = false;          // don't pause the next sample
            }

            mWindow->removeAllViewports();                  // wipe viewports
            mWindow->resetStatistics();

            if (s)
            {
                // retrieve sample's required plugins and currently installed plugins
                for (const auto& p : s->getRequiredPlugins())
                {
                    bool found = false;
                    // try to find the required plugin in the current installed plugins
                    for (const auto *i : mRoot->getInstalledPlugins())
                    {
                        if (i->getName() == p)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)  // throw an exception if a plugin is not found
                    {
                        OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "Sample requires plugin: " + p);
                    }
                }

                // test system capabilities against sample requirements
                s->testCapabilities(mRoot->getRenderSystem()->getCapabilities());
                s->_setup(this);   // start new sample
            }

            if (prof)
                prof->setEnabled(true);

            mCurrentSample = s;
        }

        /*-----------------------------------------------------------------------------
        | This function encapsulates the entire lifetime of the context.
        -----------------------------------------------------------------------------*/
        virtual void go(Sample* initialSample = 0)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            mLastRun = true;  // assume this is our last run

            initApp();
            loadStartUpSample();
#else
            while (!mLastRun)
            {
                mLastRun = true;  // assume this is our last run

                initApp();

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
                // restore the last sample if there was one or, if not, start initial sample
                if (!mFirstRun) recoverLastSample();
                else if (initialSample) runSample(initialSample);
#endif

                loadStartUpSample();

                if (mRoot->getRenderSystem() != NULL)
                {
                    mRoot->startRendering();    // start the render loop
                }

                closeApp();

                mFirstRun = false;
            }
#endif
        }

        virtual void loadStartUpSample() {}

        bool isCurrentSamplePaused()
        {
            return !mCurrentSample || mSamplePaused;
        }

        virtual void pauseCurrentSample()
        {
            if (!isCurrentSamplePaused())
            {
                mSamplePaused = true;
                mCurrentSample->paused();
            }
        }

        virtual void unpauseCurrentSample()
        {
            if (mCurrentSample && mSamplePaused)
            {
                mSamplePaused = false;
                mCurrentSample->unpaused();
            }
        }

        /*-----------------------------------------------------------------------------
        | Processes frame started events.
        -----------------------------------------------------------------------------*/
        bool frameStarted(const Ogre::FrameEvent& evt) override
        {
            pollEvents();

            // manually call sample callback to ensure correct order
            return !isCurrentSamplePaused() ? mCurrentSample->frameStarted(evt) : true;
        }

        /*-----------------------------------------------------------------------------
        | Processes rendering queued events.
        -----------------------------------------------------------------------------*/
        bool frameRenderingQueued(const Ogre::FrameEvent& evt) override
        {
            // manually call sample callback to ensure correct order
            return !isCurrentSamplePaused() ? mCurrentSample->frameRenderingQueued(evt) : true;
        }

        /*-----------------------------------------------------------------------------
        | Processes frame ended events.
        -----------------------------------------------------------------------------*/
        bool frameEnded(const Ogre::FrameEvent& evt) override
        {
            // manually call sample callback to ensure correct order
            if (mCurrentSample && !mSamplePaused && !mCurrentSample->frameEnded(evt)) return false;
            // quit if window was closed
            if (mWindow->isClosed()) return false;
            // go into idle mode if current sample has ended
            if (mCurrentSample && mCurrentSample->isDone()) runSample(0);

            return true;
        }

        bool isFirstRun() { return mFirstRun; }
        void setFirstRun(bool flag) { mFirstRun = flag; }
        bool isLastRun() { return mLastRun; }
        void setLastRun(bool flag) { mLastRun = flag; }
    protected:
        /*-----------------------------------------------------------------------------
        | Reconfigures the context. Attempts to preserve the current sample state.
        -----------------------------------------------------------------------------*/
        void reconfigure(const Ogre::String& renderer)
        {
            // save current sample state
            mLastSample = mCurrentSample;
            if (mCurrentSample) mCurrentSample->saveState(mLastSampleState);

            mLastRun = false;             // we want to go again with the new settings
            mNextRenderer = renderer;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            // Need to save the config on iOS to make sure that changes are kept on disk
            mRoot->saveConfig();
#endif
            mRoot->queueEndRendering(); // break from render loop
        }
        using ApplicationContextBase::reconfigure; // unused, silence warning

        /*-----------------------------------------------------------------------------
        | Recovers the last sample after a reset. You can override in the case that
        | the last sample is destroyed in the process of resetting, and you have to
        | recover it through another means.
        -----------------------------------------------------------------------------*/
        virtual void recoverLastSample()
        {
            runSample(mLastSample);
            mLastSample->restoreState(mLastSampleState);
            mLastSample = 0;
            mLastSampleState.clear();
        }

        /*-----------------------------------------------------------------------------
        | Cleans up and shuts down the context.
        -----------------------------------------------------------------------------*/
        void shutdown() override
        {
            if (mCurrentSample)
            {
                mCurrentSample->_shutdown();
                mCurrentSample = 0;
            }

            ApplicationContext::shutdown();
        }

        Sample* mCurrentSample;         // The active sample (0 if none is active)
        bool mSamplePaused;             // whether current sample is paused
        bool mLastRun;                  // whether or not this is the final run
        Sample* mLastSample;            // last sample run before reconfiguration
        Ogre::NameValuePairList mLastSampleState;     // state of last sample
    };
}

#endif
