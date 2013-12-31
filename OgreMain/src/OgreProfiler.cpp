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
		, mUpdateDisplayFrequency(10)
		, mCurrentFrame(0)
		, mTimer(0)
		, mTotalFrameTime(0)
		, mEnabled(false)
		, mNewEnableState(false)
		, mProfileMask(0xFFFFFFFF)
		, mMaxTotalFrameTime(0)
		, mAverageFrameTime(0)
		, mResetExtents(false)
	{
		mRoot.hierarchicalLvl = 0 - 1;
    }
	//-----------------------------------------------------------------------
	ProfileInstance::ProfileInstance(void)
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
	ProfileInstance::~ProfileInstance(void)
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
            for( TProfileSessionListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
                (*i)->initializeSession();

            mInitialized = true;
        }
        else
        {
			for( TProfileSessionListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
                (*i)->finializeSession();

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
    void Profiler::changeEnableState() 
	{
		for( TProfileSessionListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
			(*i)->changeEnableState(mNewEnableState);

        mEnabled = mNewEnableState;
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
    //-----------------------------------------------------------------------
	void Profiler::beginGPUEvent(const String& event)
    {
        Root::getSingleton().getRenderSystem()->beginProfileEvent(event);
    }
	//-----------------------------------------------------------------------
    void Profiler::endGPUEvent(const String& event)
    {
        Root::getSingleton().getRenderSystem()->endProfileEvent();
    }
	//-----------------------------------------------------------------------
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
    void Profiler::displayResults() 
	{
		// if its time to update the display
		if (!(mCurrentFrame % mUpdateDisplayFrequency)) 
		{
			// ensure the root won't be culled
			mRoot.frame.calls = 1;

			for( TProfileSessionListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
				(*i)->displayResults(mRoot, mMaxTotalFrameTime);
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
	bool ProfileInstance::watchForMax(const String& profileName) 
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
    bool ProfileInstance::watchForMin(const String& profileName) 
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
    bool ProfileInstance::watchForLimit(const String& profileName, Real limit, bool greaterThan) 
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
	void ProfileInstance::logResults() 
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
	void ProfileInstance::reset(void)
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
	void Profiler::addListener(ProfileSessionListener* listener)
	{
		mListeners.push_back(listener);
	}
    //-----------------------------------------------------------------------
	void Profiler::removeListener(ProfileSessionListener* listener)
	{
        mListeners.erase(std::find(mListeners.begin(), mListeners.end(), listener));
	}
    //-----------------------------------------------------------------------
}
