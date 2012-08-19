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
#include "OgreStableHeaders.h"
/*

    Although the code is original, many of the ideas for the profiler were borrowed from 
"Real-Time In-Game Profiling" by Steve Rabin which can be found in Game Programming
Gems 1.

    This code can easily be adapted to your own non-Ogre project. The only code that is 
Ogre-dependent is in the visualization/logging routines and the use of the Timer class.

    Enjoy!

*/

#include "OgreProfiler.h"
#include "OgreTimer.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayElement.h"
#include "OgreOverlayContainer.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    // PROFILE DEFINITIONS
    //-----------------------------------------------------------------------
    template<> Profiler* Singleton<Profiler>::msSingleton = 0;
    Profiler* Profiler::getSingletonPtr(void)
    {
        return msSingleton;
    }
    Profiler& Profiler::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    Profile::Profile(const String& profileName, uint32 groupID) 
		: mName(profileName)
		, mGroupID(groupID)
	{
        Ogre::Profiler::getSingleton().beginProfile(profileName, groupID);
    }
    //-----------------------------------------------------------------------
    Profile::~Profile()
    {
        Ogre::Profiler::getSingleton().endProfile(mName, mGroupID);
    }
    //-----------------------------------------------------------------------


    //-----------------------------------------------------------------------
    // PROFILER DEFINITIONS
    //-----------------------------------------------------------------------
    Profiler::Profiler() 
        : mCurrent(&mRoot)
        , mLast(NULL)
        , mRoot()
        , mInitialized(false)
		, mMaxDisplayProfiles(50)
		, mOverlay(0)
		, mProfileGui(0)
		, mBarHeight(10)
		, mGuiHeight(25)
		, mGuiWidth(250)
		, mGuiLeft(0)
        , mGuiTop(0)
		, mBarIndent(250)
		, mGuiBorderWidth(10)
		, mBarLineWidth(2)
		, mBarSpacing(3)
		, mUpdateDisplayFrequency(10)
		, mCurrentFrame(0)
		, mTimer(0)
		, mTotalFrameTime(0)
		, mEnabled(false)
		, mNewEnableState(false)
		, mProfileMask(0xFFFFFFFF)
		, mDisplayMode(DISPLAY_MILLISECONDS)
		, mMaxTotalFrameTime(0)
		, mAverageFrameTime(0)
		, mResetExtents(false)
	{
		mRoot.hierarchicalLvl = 0 - 1;
    }
	//-----------------------------------------------------------------------
	Profiler::ProfileInstance::ProfileInstance(void)
		: parent(NULL)
        , frameNumber(0)
		, accum(0)
		, hierarchicalLvl(0)
	{
		history.numCallsThisFrame = 0;
		history.totalTimePercent = 0;
		history.totalTimeMillisecs = 0;
		history.totalCalls = 0;
		history.maxTimePercent = 0;
		history.maxTimeMillisecs = 0;
		history.minTimePercent = 1;
		history.minTimeMillisecs = 100000;
		history.currentTimePercent = 0;
		history.currentTimeMillisecs = 0;

		frame.frameTime = 0;
		frame.calls = 0;
	}
	Profiler::ProfileInstance::~ProfileInstance(void)
	{                                        
		for(ProfileChildren::iterator it = children.begin(); it != children.end(); ++it)
		{
			ProfileInstance* instance = it->second;
			OGRE_DELETE instance;
		}
		children.clear();
	}
    //-----------------------------------------------------------------------
    Profiler::~Profiler()
    {
		if (!mRoot.children.empty()) 
		{
            // log the results of our profiling before we quit
            logResults();
        }

        // clear all our lists
        mDisabledProfiles.clear();
        mProfileBars.clear();
    }
	//---------------------------------------------------------------------
	void Profiler::setOverlayDimensions(Real width, Real height)
	{
		mGuiWidth = width;
		mGuiHeight = height;
		mBarIndent = mGuiWidth;

		mProfileGui->setDimensions(width, height);
	}
	//---------------------------------------------------------------------
	void Profiler::setOverlayPosition(Real left, Real top)
	{
		mGuiLeft = left;
		mGuiTop = top;

		mProfileGui->setPosition(left, top);
	}
	//---------------------------------------------------------------------
	Real Profiler::getOverlayWidth() const
	{
		return mGuiWidth;
	}
	//---------------------------------------------------------------------
	Real Profiler::getOverlayHeight() const
	{
		return mGuiHeight;
	}
	//---------------------------------------------------------------------
	Real Profiler::getOverlayLeft() const
	{
		return mGuiLeft;
	}
	//---------------------------------------------------------------------
	Real Profiler::getOverlayTop() const
	{
		return mGuiTop;
	}
	//---------------------------------------------------------------------
    void Profiler::initialize() 
	{
        // create a new overlay to hold our Profiler display
        mOverlay = OverlayManager::getSingleton().create("Profiler");
        mOverlay->setZOrder(500);

        // this panel will be the main container for our profile bars
        mProfileGui = createContainer();

        OverlayElement* element;

        // we create an initial pool of 50 profile bars
        for (uint i = 0; i < mMaxDisplayProfiles; ++i) 
		{

            // this is for the profile name and the number of times it was called in a frame
            element = createTextArea("profileText" + StringConverter::toString(i), 90, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, 14, "", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the current frame time
            element = createPanel("currBar" + StringConverter::toString(i), 0, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, mBarIndent, "Core/ProfilerCurrent", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the minimum frame time
            element = createPanel("minBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, "Core/ProfilerMin", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the maximum frame time
            element = createPanel("maxBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, "Core/ProfilerMax", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the average frame time
            element = createPanel("avgBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, "Core/ProfilerAvg", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

			// this indicates the text of the frame time
			element = createTextArea("statText" + StringConverter::toString(i), 20, mBarHeight, mGuiBorderWidth + (mBarHeight + mBarSpacing) * i, 0, 14, "", false);
			mProfileGui->addChild(element);
			mProfileBars.push_back(element);
        }

        // throw everything all the GUI stuff into the overlay and display it
        mOverlay->add2D(mProfileGui);
    }
    //-----------------------------------------------------------------------
    void Profiler::setTimer(Timer* t)
    {
        mTimer = t;
    }
    //-----------------------------------------------------------------------
    Timer* Profiler::getTimer()
    {
        assert(mTimer && "Timer not set!");
        return mTimer;
    }
    //-----------------------------------------------------------------------
    void Profiler::setEnabled(bool enabled) 
	{
        if (!mInitialized && enabled) 
		{
            // the user wants to enable the Profiler for the first time
            // so we initialize the GUI stuff
            initialize();
            mInitialized = true;
        }
        else
        {
            OverlayContainer* container = dynamic_cast<OverlayContainer*>(mProfileGui);
			if (container)
			{
				OverlayContainer::ChildIterator children = container->getChildIterator();
				while (children.hasMoreElements())
				{
                    OverlayElement* element = children.getNext();
                    OverlayContainer* parent = element->getParent();
                    if (parent) parent->removeChild(element->getName());
                    OverlayManager::getSingleton().destroyOverlayElement(element);
				}
			}
            if(mProfileGui)
                OverlayManager::getSingleton().destroyOverlayElement(mProfileGui);
            if(mOverlay)
                OverlayManager::getSingleton().destroy(mOverlay);			
			
			mProfileBars.clear();
            mInitialized = false;
			mEnabled = false;
        }
        // We store this enable/disable request until the frame ends
        // (don't want to screw up any open profiles!)
        mNewEnableState = enabled;
    }
    //-----------------------------------------------------------------------
    bool Profiler::getEnabled() const
    {
        return mEnabled;
    }
    //-----------------------------------------------------------------------
    void Profiler::disableProfile(const String& profileName)
	{
        // even if we are in the middle of this profile, endProfile() will still end it.

        mDisabledProfiles.insert(profileName);
    }
    //-----------------------------------------------------------------------
    void Profiler::enableProfile(const String& profileName) 
	{
		mDisabledProfiles.erase(profileName);
    }
    //-----------------------------------------------------------------------
    void Profiler::beginProfile(const String& profileName, uint32 groupID) 
	{
		// regardless of whether or not we are enabled, we need the application's root profile (ie the first profile started each frame)
		// we need this so bogus profiles don't show up when users enable profiling mid frame
		// so we check

		// if the profiler is enabled
		if (!mEnabled) 
			return;

		// mask groups
		if ((groupID & mProfileMask) == 0)
			return;

		// we only process this profile if isn't disabled
		if (mDisabledProfiles.find(profileName) != mDisabledProfiles.end()) 
			return;

		// empty string is reserved for the root
		// not really fatal anymore, however one shouldn't name one's profile as an empty string anyway.
		assert ((profileName != "") && ("Profile name can't be an empty string"));

		// this would be an internal error.
		assert (mCurrent);

		// need a timer to profile!
		assert (mTimer && "Timer not set!");

		ProfileInstance*& instance = mCurrent->children[profileName];
		if(instance)
		{	// found existing child.

			// Sanity check.
			assert(instance->name == profileName);

			if(instance->frameNumber != mCurrentFrame)
			{	// new frame, reset stats
				instance->frame.calls = 0;
				instance->frame.frameTime = 0;
				instance->frameNumber = mCurrentFrame;
			}
		}
		else
		{	// new child!
			instance = OGRE_NEW ProfileInstance();
			instance->name = profileName;
			instance->parent = mCurrent;
			instance->hierarchicalLvl = mCurrent->hierarchicalLvl + 1;
		}

		instance->frameNumber = mCurrentFrame;

		mCurrent = instance;

		// we do this at the very end of the function to get the most
		// accurate timing results
		mCurrent->currTime = mTimer->getMicroseconds();
    }
    //-----------------------------------------------------------------------
    void Profiler::endProfile(const String& profileName, uint32 groupID) 
	{
		if(!mEnabled) 
		{
			// if the profiler received a request to be enabled or disabled
			if(mNewEnableState != mEnabled) 
			{	// note mNewEnableState == true to reach this.
				changeEnableState();

				// NOTE we will be in an 'error' state until the next begin. ie endProfile will likely get invoked using a profileName that was never started.
				// even then, we can't be sure that the next beginProfile will be the true start of a new frame
			}

			return;
		}
		else
		{
			if(mNewEnableState != mEnabled) 
			{	// note mNewEnableState == false to reach this.
				changeEnableState();

				// unwind the hierarchy, should be easy enough
				mCurrent = &mRoot;
				mLast = NULL;
			}

			if(&mRoot == mCurrent && mLast)
			{	// profiler was enabled this frame, but the first subsequent beginProfile was NOT the beinging of a new frame as we had hoped.
				// we have a bogus ProfileInstance in our hierarchy, we will need to remove it, then update the overlays so as not to confuse ze user

				// we could use mRoot.children.find() instead of this, except we'd be compairing strings instead of a pointer.
				// the string way could be faster, but i don't believe it would.
				ProfileChildren::iterator it = mRoot.children.begin(), endit = mRoot.children.end();
				for(;it != endit; ++it)
				{
					if(mLast == it->second)
					{
						mRoot.children.erase(it);
						break;
					}
				}

				// with mLast == NULL we won't reach this code, in case this isn't the end of the top level profile
				ProfileInstance* last = mLast;
				mLast = NULL;
				OGRE_DELETE last;

				processFrameStats();
				displayResults();
			}
		}

		if(&mRoot == mCurrent)
			return;

		// mask groups
		if ((groupID & mProfileMask) == 0)
			return;

		// need a timer to profile!
		assert (mTimer && "Timer not set!");

		// get the end time of this profile
		// we do this as close the beginning of this function as possible
		// to get more accurate timing results
		const ulong endTime = mTimer->getMicroseconds();

		// empty string is reserved for designating an empty parent
		assert ((profileName != "") && ("Profile name can't be an empty string"));

		// we only process this profile if isn't disabled
		// we check the current instance name against the provided profileName as a guard against disabling a profile name /after/ said profile began
		if(mCurrent->name != profileName && mDisabledProfiles.find(profileName) != mDisabledProfiles.end()) 
			return;

		// calculate the elapsed time of this profile
		const ulong timeElapsed = endTime - mCurrent->currTime;

		// update parent's accumulator if it isn't the root
		if (&mRoot != mCurrent->parent) 
		{
			// add this profile's time to the parent's accumlator
			mCurrent->parent->accum += timeElapsed;
		}

		mCurrent->frame.frameTime += timeElapsed;
		++mCurrent->frame.calls;

		mLast = mCurrent;
		mCurrent = mCurrent->parent;

		if (&mRoot == mCurrent) 
		{
			// the stack is empty and all the profiles have been completed
			// we have reached the end of the frame so process the frame statistics

			// we know that the time elapsed of the main loop is the total time the frame took
			mTotalFrameTime = timeElapsed;

			if(timeElapsed > mMaxTotalFrameTime)
				mMaxTotalFrameTime = timeElapsed;

			// we got all the information we need, so process the profiles
			// for this frame
			processFrameStats();

			// we display everything to the screen
			displayResults();
		}

	}
    void Profiler::beginGPUEvent(const String& event)
    {
        Root::getSingleton().getRenderSystem()->beginProfileEvent(event);
    }
    void Profiler::endGPUEvent(const String& event)
    {
        Root::getSingleton().getRenderSystem()->endProfileEvent();
    }
    void Profiler::markGPUEvent(const String& event)
    {
        Root::getSingleton().getRenderSystem()->markProfileEvent(event);
    }
	//-----------------------------------------------------------------------
	void Profiler::processFrameStats(ProfileInstance* instance, Real& maxFrameTime)
	{
		// calculate what percentage of frame time this profile took
		const Real framePercentage = (Real) instance->frame.frameTime / (Real) mTotalFrameTime;

		const Real frameTimeMillisecs = (Real) instance->frame.frameTime / 1000.0f;

		// update the profile stats
		instance->history.currentTimePercent = framePercentage;
		instance->history.currentTimeMillisecs = frameTimeMillisecs;
		if(mResetExtents)
		{
			instance->history.totalTimePercent = framePercentage;
			instance->history.totalTimeMillisecs = frameTimeMillisecs;
			instance->history.totalCalls = 1;
		}
		else
		{
			instance->history.totalTimePercent += framePercentage;
			instance->history.totalTimeMillisecs += frameTimeMillisecs;
			instance->history.totalCalls++;
		}
		instance->history.numCallsThisFrame = instance->frame.calls;

		// if we find a new minimum for this profile, update it
		if (frameTimeMillisecs < instance->history.minTimeMillisecs || mResetExtents)
		{
			instance->history.minTimePercent = framePercentage;
			instance->history.minTimeMillisecs = frameTimeMillisecs;
		}

		// if we find a new maximum for this profile, update it
		if (frameTimeMillisecs > instance->history.maxTimeMillisecs || mResetExtents)
		{
			instance->history.maxTimePercent = framePercentage;
			instance->history.maxTimeMillisecs = frameTimeMillisecs;
		}

		if(instance->frame.frameTime > maxFrameTime)
			maxFrameTime = (Real)instance->frame.frameTime;

		ProfileChildren::iterator it = instance->children.begin(), endit = instance->children.end();
		for(;it != endit; ++it)
		{
			ProfileInstance* child = it->second;

			// we set the number of times each profile was called per frame to 0
			// because not all profiles are called every frame
			child->history.numCallsThisFrame = 0;

			if(child->frame.calls > 0)
			{
				processFrameStats(child, maxFrameTime);
			}
		}
	}
	//-----------------------------------------------------------------------
	void Profiler::processFrameStats(void) 
	{
		Real maxFrameTime = 0;

		ProfileChildren::iterator it = mRoot.children.begin(), endit = mRoot.children.end();
		for(;it != endit; ++it)
		{
			ProfileInstance* child = it->second;

			// we set the number of times each profile was called per frame to 0
			// because not all profiles are called every frame
			child->history.numCallsThisFrame = 0;

			if(child->frame.calls > 0)
			{
				processFrameStats(child, maxFrameTime);
			}
		}

		// Calculate whether the extents are now so out of date they need regenerating
		if (mCurrentFrame == 0)
			mAverageFrameTime = maxFrameTime;
		else
			mAverageFrameTime = (mAverageFrameTime + maxFrameTime) * 0.5f;

		if ((Real)mMaxTotalFrameTime > mAverageFrameTime * 4)
		{
			mResetExtents = true;
			mMaxTotalFrameTime = (ulong)mAverageFrameTime;
		}
		else
			mResetExtents = false;
	}
	//-----------------------------------------------------------------------
	void Profiler::displayResults(ProfileInstance* instance, ProfileBarList::iterator& bIter, Real& maxTimeMillisecs, Real& newGuiHeight, int& profileCount)
	{
		OverlayElement* g;

		// display the profile's name and the number of times it was called in a frame
		g = *bIter;
		++bIter;
		g->show();
		g->setCaption(String(instance->name + " (" + StringConverter::toString(instance->history.numCallsThisFrame) + ")"));
		g->setLeft(10 + instance->hierarchicalLvl * 15.0f);


		// display the main bar that show the percentage of the frame time that this
		// profile has taken
		g = *bIter;
		++bIter;
		g->show();
		// most of this junk has been set before, but we do this to get around a weird
		// Ogre gui issue (bug?)
		g->setMetricsMode(GMM_PIXELS);
		g->setHeight(mBarHeight);

		if (mDisplayMode == DISPLAY_PERCENTAGE)
			g->setWidth( (instance->history.currentTimePercent) * mGuiWidth);
		else
			g->setWidth( (instance->history.currentTimeMillisecs / maxTimeMillisecs) * mGuiWidth);

		g->setLeft(mGuiWidth);
		g->setTop(mGuiBorderWidth + profileCount * (mBarHeight + mBarSpacing));



		// display line to indicate the minimum frame time for this profile
		g = *bIter;
		++bIter;
		g->show();
		if(mDisplayMode == DISPLAY_PERCENTAGE)
			g->setLeft(mBarIndent + instance->history.minTimePercent * mGuiWidth);
		else
			g->setLeft(mBarIndent + (instance->history.minTimeMillisecs / maxTimeMillisecs) * mGuiWidth);

		// display line to indicate the maximum frame time for this profile
		g = *bIter;
		++bIter;
		g->show();
		if(mDisplayMode == DISPLAY_PERCENTAGE)
			g->setLeft(mBarIndent + instance->history.maxTimePercent * mGuiWidth);
		else
			g->setLeft(mBarIndent + (instance->history.maxTimeMillisecs / maxTimeMillisecs) * mGuiWidth);

		// display line to indicate the average frame time for this profile
		g = *bIter;
		++bIter;
		g->show();
		if(instance->history.totalCalls != 0)
		{
			if (mDisplayMode == DISPLAY_PERCENTAGE)
				g->setLeft(mBarIndent + (instance->history.totalTimePercent / instance->history.totalCalls) * mGuiWidth);
			else
				g->setLeft(mBarIndent + ((instance->history.totalTimeMillisecs / instance->history.totalCalls) / maxTimeMillisecs) * mGuiWidth);
		}
		else
			g->setLeft(mBarIndent);

		// display text
		g = *bIter;
		++bIter;
		g->show();
		if (mDisplayMode == DISPLAY_PERCENTAGE)
		{
			g->setLeft(mBarIndent + instance->history.currentTimePercent * mGuiWidth + 2);
			g->setCaption(StringConverter::toString(instance->history.currentTimePercent * 100.0f, 3, 3) + "%");
		}
		else
		{
			g->setLeft(mBarIndent + (instance->history.currentTimeMillisecs / maxTimeMillisecs) * mGuiWidth + 2);
			g->setCaption(StringConverter::toString(instance->history.currentTimeMillisecs, 3, 3) + "ms");
		}

		// we set the height of the display with respect to the number of profiles displayed
		newGuiHeight += mBarHeight + mBarSpacing;

		++profileCount;

		// display children
		ProfileChildren::iterator it = instance->children.begin(), endit = instance->children.end();
		for(;it != endit; ++it)
		{
			ProfileInstance* child = it->second;
			displayResults(child, bIter, maxTimeMillisecs, newGuiHeight, profileCount);
		}
	}
	//-----------------------------------------------------------------------
    void Profiler::displayResults(void) 
	{
		// if its time to update the display
		if (!(mCurrentFrame % mUpdateDisplayFrequency)) 
		{
			Real newGuiHeight = mGuiHeight;
			int profileCount = 0;
			Real maxTimeMillisecs = (Real)mMaxTotalFrameTime / 1000.0f;

			// ensure the root won't be culled
			mRoot.frame.calls = 1;

			ProfileBarList::iterator bIter = mProfileBars.begin();
			ProfileChildren::iterator it = mRoot.children.begin(), endit = mRoot.children.end();
			for(;it != endit; ++it)
			{
				ProfileInstance* child = it->second;
				displayResults(child, bIter, maxTimeMillisecs, newGuiHeight, profileCount);
			}
			

			// set the main display dimensions
			mProfileGui->setMetricsMode(GMM_PIXELS);
			mProfileGui->setHeight(newGuiHeight);
			mProfileGui->setWidth(mGuiWidth * 2 + 15);
			mProfileGui->setTop(5);
			mProfileGui->setLeft(5);

			// we hide all the remaining pre-created bars
			for (; bIter != mProfileBars.end(); ++bIter) 
			{
				(*bIter)->hide();
			}
		}
		++mCurrentFrame;
    }
    //-----------------------------------------------------------------------
    bool Profiler::watchForMax(const String& profileName) 
	{
		assert ((profileName != "") && ("Profile name can't be an empty string"));

		return mRoot.watchForMax(profileName);
    }
	//-----------------------------------------------------------------------
	bool Profiler::ProfileInstance::watchForMax(const String& profileName) 
	{
        ProfileChildren::iterator it = children.begin(), endit = children.end();
		for(;it != endit; ++it)
		{
			ProfileInstance* child = it->second;
			if( (child->name == profileName && child->watchForMax()) || child->watchForMax(profileName))
				return true;
		}
		return false;
    }
    //-----------------------------------------------------------------------
    bool Profiler::watchForMin(const String& profileName) 
	{
		assert ((profileName != "") && ("Profile name can't be an empty string"));
		return mRoot.watchForMin(profileName);
    }
	//-----------------------------------------------------------------------
    bool Profiler::ProfileInstance::watchForMin(const String& profileName) 
	{
		ProfileChildren::iterator it = children.begin(), endit = children.end();
		for(;it != endit; ++it)
		{
			ProfileInstance* child = it->second;
			if( (child->name == profileName && child->watchForMin()) || child->watchForMin(profileName))
				return true;
		}
		return false;
    }
    //-----------------------------------------------------------------------
    bool Profiler::watchForLimit(const String& profileName, Real limit, bool greaterThan) 
	{
        assert ((profileName != "") && ("Profile name can't be an empty string"));
		return mRoot.watchForLimit(profileName, limit, greaterThan);
    }
	//-----------------------------------------------------------------------
    bool Profiler::ProfileInstance::watchForLimit(const String& profileName, Real limit, bool greaterThan) 
	{
        ProfileChildren::iterator it = children.begin(), endit = children.end();
		for(;it != endit; ++it)
		{
			ProfileInstance* child = it->second;
			if( (child->name == profileName && child->watchForLimit(limit, greaterThan)) || child->watchForLimit(profileName, limit, greaterThan))
				return true;
		}
		return false;
    }
    //-----------------------------------------------------------------------
    void Profiler::logResults() 
	{
        LogManager::getSingleton().logMessage("----------------------Profiler Results----------------------");

		for(ProfileChildren::iterator it = mRoot.children.begin(); it != mRoot.children.end(); ++it)
		{
			it->second->logResults();
		}

        LogManager::getSingleton().logMessage("------------------------------------------------------------");
    }
	//-----------------------------------------------------------------------
	void Profiler::ProfileInstance::logResults() 
	{
		// create an indent that represents the hierarchical order of the profile
		String indent = "";
		for (uint i = 0; i < hierarchicalLvl; ++i) 
		{
			indent = indent + "\t";
		}

		LogManager::getSingleton().logMessage(indent + "Name " + name + 
						" | Min " + StringConverter::toString(history.minTimePercent) + 
						" | Max " + StringConverter::toString(history.maxTimePercent) + 
						" | Avg "+ StringConverter::toString(history.totalTimePercent / history.totalCalls));   

		for(ProfileChildren::iterator it = children.begin(); it != children.end(); ++it)
		{
			it->second->logResults();
		}
	}
    //-----------------------------------------------------------------------
    void Profiler::reset() 
	{
        mRoot.reset();
        mMaxTotalFrameTime = 0;
    }
	//-----------------------------------------------------------------------
	void Profiler::ProfileInstance::reset(void)
	{
		history.currentTimePercent = history.maxTimePercent = history.totalTimePercent = 0;
		history.currentTimeMillisecs = history.maxTimeMillisecs = history.totalTimeMillisecs = 0;
		history.numCallsThisFrame = history.totalCalls = 0;

		history.minTimePercent = 1;
		history.minTimeMillisecs = 100000;
		for(ProfileChildren::iterator it = children.begin(); it != children.end(); ++it)
		{
			it->second->reset();
		}
	}
    //-----------------------------------------------------------------------
    void Profiler::setUpdateDisplayFrequency(uint freq)
    {
        mUpdateDisplayFrequency = freq;
    }
    //-----------------------------------------------------------------------
    uint Profiler::getUpdateDisplayFrequency() const
    {
        return mUpdateDisplayFrequency;
    }
    //-----------------------------------------------------------------------
    void Profiler::changeEnableState() 
	{
        if (mNewEnableState) 
		{
            mOverlay->show();
        }
        else 
		{
            mOverlay->hide();
        }
        mEnabled = mNewEnableState;
    }
    //-----------------------------------------------------------------------
    OverlayContainer* Profiler::createContainer()
    {
        OverlayContainer* container = (OverlayContainer*) 
			OverlayManager::getSingleton().createOverlayElement(
				"BorderPanel", "profiler");
        container->setMetricsMode(GMM_PIXELS);
        container->setMaterialName("Core/StatsBlockCenter");
        container->setHeight(mGuiHeight);
        container->setWidth(mGuiWidth * 2 + 15);
        container->setParameter("border_size", "1 1 1 1");
        container->setParameter("border_material", "Core/StatsBlockBorder");
        container->setParameter("border_topleft_uv", "0.0000 1.0000 0.0039 0.9961");
        container->setParameter("border_top_uv", "0.0039 1.0000 0.9961 0.9961");
        container->setParameter("border_topright_uv", "0.9961 1.0000 1.0000 0.9961");
        container->setParameter("border_left_uv","0.0000 0.9961 0.0039 0.0039");
        container->setParameter("border_right_uv","0.9961 0.9961 1.0000 0.0039");
        container->setParameter("border_bottomleft_uv","0.0000 0.0039 0.0039 0.0000");
        container->setParameter("border_bottom_uv","0.0039 0.0039 0.9961 0.0000");
        container->setParameter("border_bottomright_uv","0.9961 0.0039 1.0000 0.0000");
        container->setLeft(5);
        container->setTop(5);

        return container;
    }
    //-----------------------------------------------------------------------
    OverlayElement* Profiler::createTextArea(const String& name, Real width, Real height, Real top, Real left, 
                                         uint fontSize, const String& caption, bool show)
    {
        OverlayElement* textArea = 
			OverlayManager::getSingleton().createOverlayElement("TextArea", name);
        textArea->setMetricsMode(GMM_PIXELS);
        textArea->setWidth(width);
        textArea->setHeight(height);
        textArea->setTop(top);
        textArea->setLeft(left);
        textArea->setParameter("font_name", "SdkTrays/Value");
        textArea->setParameter("char_height", StringConverter::toString(fontSize));
        textArea->setCaption(caption);
        textArea->setParameter("colour_top", "1 1 1");
        textArea->setParameter("colour_bottom", "1 1 1");

        if (show) {
            textArea->show();
        }
        else {
            textArea->hide();
        }

        return textArea;
    }
    //-----------------------------------------------------------------------
    OverlayElement* Profiler::createPanel(const String& name, Real width, Real height, Real top, Real left, 
                                      const String& materialName, bool show)
    {
        OverlayElement* panel = 
			OverlayManager::getSingleton().createOverlayElement("Panel", name);
        panel->setMetricsMode(GMM_PIXELS);
        panel->setWidth(width);
        panel->setHeight(height);
        panel->setTop(top);
        panel->setLeft(left);
        panel->setMaterialName(materialName);

        if (show) {
            panel->show();
        }
        else {
            panel->hide();
        }

        return panel;
    }
    //-----------------------------------------------------------------------

}
