/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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

namespace Ogre {
    //-----------------------------------------------------------------------
    // PROFILE DEFINITIONS
    //-----------------------------------------------------------------------
    template<> Profiler* Singleton<Profiler>::ms_Singleton = 0;
    Profiler* Profiler::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    Profiler& Profiler::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
    Profile::Profile(const String& profileName) {

        mName = profileName;

        Ogre::Profiler::getSingleton().beginProfile(profileName);

    }
    //-----------------------------------------------------------------------
    Profile::~Profile() {

        Ogre::Profiler::getSingleton().endProfile(mName);

    }
    //-----------------------------------------------------------------------


    //-----------------------------------------------------------------------
    // PROFILER DEFINITIONS
    //-----------------------------------------------------------------------
    Profiler::Profiler() {

        // init some variables
        mTimer = 0;
        mTotalFrameTime = 0;
        mUpdateDisplayFrequency = 0;
        mCurrentFrame = 0;
        mEnabled = mNewEnableState = false; // the profiler starts out as disabled
        mEnableStateChangePending = false;
        mInitialized = false;
        maxProfiles = 50;

        // by default the display will be updated every 10 frames
        mUpdateDisplayFrequency = 10;

    }
    //-----------------------------------------------------------------------
    Profiler::~Profiler() {

        if (!mProfileHistory.empty()) {
            // log the results of our profiling before we quit
            logResults();
        }

        // clear all our lists
        mProfiles.clear();
        mProfileFrame.clear();
        mProfileHistoryMap.clear();
        mProfileHistory.clear();
        mDisabledProfiles.clear();
        mProfileBars.clear();

    }
    //-----------------------------------------------------------------------
    void Profiler::initialize() {

        // init some gui characteristics
        mBarHeight = 10; //0.02;
        mGuiBorderWidth = 10; //0.02;
        mGuiHeight = 25; //0.05;
        mGuiWidth = 250; //0.15;
        mBarIndent = mGuiWidth;
        mBarLineWidth = 2;

        // create a new overlay to hold our Profiler display
        mOverlay = OverlayManager::getSingleton().create("Profiler");
        mOverlay->setZOrder(500);

        // this panel will be the main container for our profile bars
        mProfileGui = createContainer();

        OverlayElement* element;

        // we create the little "ticks" above the profiles
        for (uint k = 1; k < 10; ++k) { // we don't want a tick at 0% or 100%

            if (k != 5) { // we don't want a tick at 50%
                element = createTextArea("ProfileKeyLine" + StringConverter::toString(k), 20, 10, 2, mGuiWidth * (1 + k * .1), 9, "|");
                mProfileGui->addChild(element);
            }

        }

        // we create a 0% marker
        element = createTextArea("ProfileKey0", 50, 10, 2, mGuiWidth * 0.99, 9, "0%");
        mProfileGui->addChild(element);

        // we create a 50% marker
        element = createTextArea("ProfileyKey50", 50, 10, 2, mGuiWidth * 1.48, 9, "50%");
        mProfileGui->addChild(element);

        // we create a 100% marker
        element = createTextArea("ProfileKey100", 50, 10, 2, mGuiWidth * 1.98, 9, "100%");
        mProfileGui->addChild(element);

        // we create an initial pool of 50 profile bars
        for (uint i = 0; i < maxProfiles; ++i) {

            // this is for the profile name and the number of times it was called in a frame
            element = createTextArea("profileText" + StringConverter::toString(i), 90, mBarHeight, mGuiBorderWidth + (mBarHeight * 2) * i, 0, 14, "", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the current frame time
            element = createPanel("currBar" + StringConverter::toString(i), 0, mBarHeight, mGuiBorderWidth + (mBarHeight * 2) * i, mBarIndent, "Core/ProfilerCurrent", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the minimum frame time
            element = createPanel("minBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight * 2) * i, 0, "Core/ProfilerMin", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the maximum frame time
            element = createPanel("maxBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight * 2) * i, 0, "Core/ProfilerMax", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

            // this indicates the average frame time
            element = createPanel("avgBar" + StringConverter::toString(i), mBarLineWidth, mBarHeight, mGuiBorderWidth + (mBarHeight * 2) * i, 0, "Core/ProfilerAvg", false);
            mProfileGui->addChild(element);
            mProfileBars.push_back(element);

        }

        // throw everything all the GUI stuff into the overlay and display it
        mOverlay->add2D(mProfileGui);
        mOverlay->show();

    }
    //-----------------------------------------------------------------------
    void Profiler::setTimer(Timer* t) {

        mTimer = t;

    }
    //-----------------------------------------------------------------------
    Timer* Profiler::getTimer() {

        assert(mTimer && "Timer not set!");
        return mTimer;

    }
    //-----------------------------------------------------------------------
    void Profiler::setEnabled(bool enabled) {

        if (!mInitialized && enabled) {

            // the user wants to enable the Profiler for the first time
            // so we initialize the GUI stuff
            initialize();
            mInitialized = true;
            mEnabled = true;

        }
        else {
            // We store this enable/disable request until the frame ends
            // (don't want to screw up any open profiles!)
            mEnableStateChangePending = true;
            mNewEnableState = enabled;
        }

    }
    //-----------------------------------------------------------------------
    bool Profiler::getEnabled() const {

        return mEnabled;

    }
    //-----------------------------------------------------------------------
    void Profiler::disableProfile(const String& profileName) {

        // make sure the profile isn't already disabled
        DisabledProfileMap::iterator iter;
        iter = mDisabledProfiles.find(profileName);

        // make sure you don't disable a profile in the middle of that profile
        ProfileStack::iterator pIter;
        for (pIter = mProfiles.begin(); pIter != mProfiles.end(); ++pIter) {

            if (profileName == (*pIter).name)
                break;

        }

        // if those two conditions are met, disable the profile
        if ( (iter == mDisabledProfiles.end()) && (pIter == mProfiles.end()) ) {

            mDisabledProfiles.insert(std::pair<String, bool>(profileName, true));

        }

    }
    //-----------------------------------------------------------------------
    void Profiler::enableProfile(const String& profileName) {

        // make sure the profile is actually disabled
        DisabledProfileMap::iterator iter;
        iter = mDisabledProfiles.find(profileName);

        // make sure you don't enable a profile in the middle of that profile
        ProfileStack::iterator pIter;
        for (pIter = mProfiles.begin(); pIter != mProfiles.end(); ++pIter) {

            if (profileName == (*pIter).name)
                break;

        }

        // if those two conditions are met, enable the profile by removing it from
        // the disabled list
        if ( (iter != mDisabledProfiles.end()) && (pIter == mProfiles.end()) ) {

            mDisabledProfiles.erase(iter);

        }

    }
    //-----------------------------------------------------------------------
    void Profiler::beginProfile(const String& profileName) {

        // if the profiler is enabled
        if (!mEnabled) {

            return;

        }

        // empty string is reserved for the root
        assert ((profileName != "") && ("Profile name can't be an empty string"));

        ProfileStack::iterator iter;
        for (iter = mProfiles.begin(); iter != mProfiles.end(); ++iter) {

            if ((*iter).name == profileName) {

                break;

            }

        }

        // make sure this profile isn't being used more than once
        assert ((iter == mProfiles.end()) && ("This profile name is already being used"));

        // we only process this profile if isn't disabled
        DisabledProfileMap::iterator dIter;
        dIter = mDisabledProfiles.find(profileName);
        if ( dIter != mDisabledProfiles.end() ) {

            return;

        }

        ProfileInstance p;
		p.hierarchicalLvl = static_cast<uint>(mProfiles.size());

        // this is the root, it has no parent
        if (mProfiles.empty()) {

            p.parent = "";

        }
        // otherwise peek at the stack and use the top as the parent
        else {

            ProfileInstance parent = mProfiles.back();
            p.parent = parent.name;

        }

        // need a timer to profile!
        assert (mTimer && "Timer not set!");

        ProfileFrameList::iterator fIter;
        ProfileHistoryList::iterator hIter;

        // we check to see if this profile has been called in the frame before
        for (fIter = mProfileFrame.begin(); fIter != mProfileFrame.end(); ++fIter) {

            if ((*fIter).name == profileName)
                break;

        }
        // if it hasn't been called before, set its position in the stack
        if (fIter == mProfileFrame.end()) {

            ProfileFrame f;
            f.name = profileName;
            f.frameTime = 0;
            f.calls = 0;
            f.hierarchicalLvl = (uint) mProfiles.size();
            mProfileFrame.push_back(f);

        }

        // we check to see if this profile has been called in the app before
        ProfileHistoryMap::iterator histMapIter;
        histMapIter = mProfileHistoryMap.find(profileName);

        // if not we add a profile with just the name into the history
        if (histMapIter == mProfileHistoryMap.end()) {

            ProfileHistory h;
            h.name = profileName;
            h.numCallsThisFrame = 0;
            h.totalTime = 0;
            h.totalCalls = 0;
            h.maxTime = 0;
            h.minTime = 1;
            h.hierarchicalLvl = p.hierarchicalLvl;
            h.currentTime = 0;

            // we add this to the history
            hIter = mProfileHistory.insert(mProfileHistory.end(), h);

            // for quick look-ups, we'll add it to the history map as well
            mProfileHistoryMap.insert(std::pair<String, ProfileHistoryList::iterator>(profileName, hIter));

        }

        // add the stats to this profile and push it on the stack
        // we do this at the very end of the function to get the most
        // accurate timing results
        p.name = profileName;
        p.currTime = mTimer->getMicroseconds();
        p.accum = 0;
        mProfiles.push_back(p);

    }
    //-----------------------------------------------------------------------
    void Profiler::endProfile(const String& profileName) {

		// if the profiler received a request to be enabled or disabled
		// we reached the end of the frame so we can safely do this
		if (mEnableStateChangePending) {

			changeEnableState();

		}

     // if the profiler is enabled
        if(!mEnabled) {

            return;

        }

        // need a timer to profile!
        assert (mTimer && "Timer not set!");

        // get the end time of this profile
        // we do this as close the beginning of this function as possible
        // to get more accurate timing results
        ulong endTime = mTimer->getMicroseconds();

        // empty string is reserved for designating an empty parent
        assert ((profileName != "") && ("Profile name can't be an empty string"));

        // we only process this profile if isn't disabled
        DisabledProfileMap::iterator dIter;
        dIter = mDisabledProfiles.find(profileName);
        if ( dIter != mDisabledProfiles.end() ) {

            return;

        }

        // stack shouldnt be empty
        assert (!mProfiles.empty());

        // get the start of this profile
        ProfileInstance bProfile;
        bProfile = mProfiles.back();
        mProfiles.pop_back();

        // calculate the elapsed time of this profile
        ulong timeElapsed = endTime - bProfile.currTime;

        // update parent's accumalator if it isn't the root
        if (bProfile.parent != "") {

            // find the parent
            ProfileStack::iterator iter;
            for(iter = mProfiles.begin(); iter != mProfiles.end(); ++iter) {

                if ((*iter).name == bProfile.parent)
                    break;

            }

            // the parent should be found 
            assert(iter != mProfiles.end());

            // add this profile's time to the parent's accumlator
            (*iter).accum += timeElapsed;

        }

        // we find the profile in this frame
        ProfileFrameList::iterator iter;
        for (iter = mProfileFrame.begin(); iter != mProfileFrame.end(); ++iter) {

            if ((*iter).name == bProfile.name)
                break;

        }

        // we subtract the time the children profiles took from this profile
        (*iter).frameTime += timeElapsed - bProfile.accum;
        (*iter).calls++;

        // the stack is empty and all the profiles have been completed
        // we have reached the end of the frame so process the frame statistics
        if (mProfiles.empty()) {

            // we know that the time elapsed of the main loop is the total time the frame took
            mTotalFrameTime = timeElapsed;

            // we got all the information we need, so process the profiles
            // for this frame
            processFrameStats();

            // clear the frame stats for next frame
            mProfileFrame.clear();

            // we display everything to the screen
            displayResults();

        }

    }
    //-----------------------------------------------------------------------
    void Profiler::processFrameStats() {

        ProfileFrameList::iterator frameIter;
        ProfileHistoryList::iterator historyIter;

        // we set the number of times each profile was called per frame to 0
        // because not all profiles are called every frame
        for (historyIter = mProfileHistory.begin(); historyIter != mProfileHistory.end(); ++historyIter) {

            (*historyIter).numCallsThisFrame = 0;

        }

        // iterate through each of the profiles processed during this frame
        for (frameIter = mProfileFrame.begin(); frameIter != mProfileFrame.end(); ++frameIter) {

            String s = (*frameIter).name;

            // use our map to find the appropriate profile in the history
            historyIter = (*mProfileHistoryMap.find(s)).second;

            // extract the frame stats
            ulong frameTime = (*frameIter).frameTime;
            uint calls = (*frameIter).calls;
            uint lvl = (*frameIter).hierarchicalLvl;

            // calculate what percentage of frame time this profile took
            Real framePercentage = (Real) frameTime / (Real) mTotalFrameTime;

            // update the profile stats
            (*historyIter).currentTime = framePercentage;
            (*historyIter).totalTime += framePercentage;
            (*historyIter).totalCalls++;
            (*historyIter).numCallsThisFrame = calls;
            (*historyIter).hierarchicalLvl = lvl;

            // if we find a new minimum for this profile, update it
            if ((framePercentage) < ((*historyIter).minTime)) {

                (*historyIter).minTime = framePercentage;

            }

            // if we find a new maximum for this profile, update it
            if ((framePercentage) > ((*historyIter).maxTime)) {

                (*historyIter).maxTime = framePercentage;

            }

        }

    }
    //-----------------------------------------------------------------------
    void Profiler::displayResults() {

        if (!mEnabled) {

            return;

        }

        // if its time to update the display
        if (mCurrentFrame >= mUpdateDisplayFrequency) {

            mCurrentFrame = 0;

            ProfileHistoryList::iterator iter;
            ProfileBarList::iterator bIter;

            OverlayElement* g;

            Real newGuiHeight = mGuiHeight;

            int temp = 0; // dummy variable for weird Ogre issue

            // go through each profile and display it
            for (iter = mProfileHistory.begin(), bIter = mProfileBars.begin(); 
				iter != mProfileHistory.end() && bIter != mProfileBars.end(); 
				++iter, ++bIter) 
			{

                // display the profile's name and the number of times it was called in a frame
                g = *bIter;
                g->show();
                g->setCaption(String((*iter).name + " (" + StringConverter::toString((*iter).numCallsThisFrame) + ")"));
                g->setLeft(10 + (*iter).hierarchicalLvl * 15);

                // display the main bar that show the percentage of the frame time that this
                // profile has taken
                bIter++;
                g = *bIter;
                g->show();
                // most of this junk has been set before, but we do this to get around a weird
                // Ogre gui issue (bug?)
                g->setMetricsMode(GMM_PIXELS);
                g->setHeight(mBarHeight);
                g->setWidth(((*iter).currentTime) * mGuiWidth);
                g->setLeft(mGuiWidth);
                g->setTop(mGuiBorderWidth + temp * mBarHeight * 2);

                // display line to indicate the minimum frame time for this profile
                bIter++;
                g = *bIter;
                g->show();
                g->setLeft(mBarIndent + (*iter).minTime * mGuiWidth);

                // display line to indicate the maximum frame time for this profile
                bIter++;
                g = *bIter;
                g->show();
                g->setLeft(mBarIndent + (*iter).maxTime * mGuiWidth);

                // display line to indicate the average frame time for this profile
                bIter++;
                g = *bIter;
                g->show();
                if ((*iter).totalCalls != 0)
                    g->setLeft(mBarIndent + ((*iter).totalTime / (*iter).totalCalls) * mGuiWidth);
                else
                    g->setLeft(mBarIndent);
                // we set the height of the display with respect to the number of profiles displayed
                newGuiHeight += mBarHeight * 2;

                temp++;

            }

            // set the main display dimensions
            mProfileGui->setMetricsMode(GMM_PIXELS);
            mProfileGui->setHeight(newGuiHeight);
            mProfileGui->setWidth(mGuiWidth * 2 + 15);
            mProfileGui->setTop(5);
            mProfileGui->setLeft(5);

            // we hide all the remaining pre-created bars
            for (; bIter != mProfileBars.end(); ++bIter) {

                (*bIter)->hide();

            }

        }

        // not time to update the display yet
        else {

            mCurrentFrame++;

        }

    }
    //-----------------------------------------------------------------------
    bool Profiler::watchForMax(const String& profileName) {

        ProfileHistoryMap::iterator mapIter;
        ProfileHistoryList::iterator iter;

        mapIter = mProfileHistoryMap.find(profileName);

        // if we don't find the profile, return false
        if (mapIter == mProfileHistoryMap.end())
            return false;

        iter = (*mapIter).second;

        return ((*iter).currentTime == (*iter).maxTime);

    }
    //-----------------------------------------------------------------------
    bool Profiler::watchForMin(const String& profileName) {

        ProfileHistoryMap::iterator mapIter;
        ProfileHistoryList::iterator iter;

        mapIter = mProfileHistoryMap.find(profileName);

        // if we don't find the profile, return false
        if (mapIter == mProfileHistoryMap.end())
            return false;

        iter = (*mapIter).second;

        return ((*iter).currentTime == (*iter).minTime);

    }
    //-----------------------------------------------------------------------
    bool Profiler::watchForLimit(const String& profileName, Real limit, bool greaterThan) {

        ProfileHistoryMap::iterator mapIter;
        ProfileHistoryList::iterator iter;

        mapIter = mProfileHistoryMap.find(profileName);

        // if we don't find the profile, return false
        if (mapIter == mProfileHistoryMap.end())
            return false;

        iter = (*mapIter).second;

        if (greaterThan)
            return ((*iter).currentTime > limit);
        else
            return ((*iter).currentTime < limit);

    }
    //-----------------------------------------------------------------------
    void Profiler::logResults() {

        ProfileHistoryList::iterator iter;

        LogManager::getSingleton().logMessage("----------------------Profiler Results----------------------");

        for (iter = mProfileHistory.begin(); iter != mProfileHistory.end(); ++iter) {

            // create an indent that represents the hierarchical order of the profile
            String indent = "";
            for (uint i = 0; i < (*iter).hierarchicalLvl; ++i) {

                indent = indent + "   ";

            }

            LogManager::getSingleton().logMessage(indent + "Name " + (*iter).name + " | Min " + StringConverter::toString((*iter).minTime) + " | Max " + StringConverter::toString((*iter).maxTime) + " | Avg "+ StringConverter::toString((*iter).totalTime / (*iter).totalCalls));

        }

        LogManager::getSingleton().logMessage("------------------------------------------------------------");

    }
    //-----------------------------------------------------------------------
    void Profiler::reset() {

        ProfileHistoryList::iterator iter;
        for (iter = mProfileHistory.begin(); iter != mProfileHistory.end(); ++iter) {
        
            (*iter).currentTime = (*iter).maxTime = (*iter).totalTime = 0;
            (*iter).numCallsThisFrame = (*iter).totalCalls = 0;

            (*iter).minTime = 1;

        }

    }
    //-----------------------------------------------------------------------
    void Profiler::setUpdateDisplayFrequency(uint freq) {

        mUpdateDisplayFrequency = freq;

    }
    //-----------------------------------------------------------------------
    uint Profiler::getUpdateDisplayFrequency() const {

        return mUpdateDisplayFrequency;

    }
    //-----------------------------------------------------------------------
    void Profiler::changeEnableState() {

        if (mNewEnableState) {

            mOverlay->show();

        }
        else {

            mOverlay->hide();

        }
        mEnabled = mNewEnableState;
        mEnableStateChangePending = false;

    }
    //-----------------------------------------------------------------------
    OverlayContainer* Profiler::createContainer() {

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
                                         uint fontSize, const String& caption, bool show) {


        OverlayElement* textArea = 
			OverlayManager::getSingleton().createOverlayElement("TextArea", name);
        textArea->setMetricsMode(GMM_PIXELS);
        textArea->setWidth(width);
        textArea->setHeight(height);
        textArea->setTop(top);
        textArea->setLeft(left);
        textArea->setParameter("font_name", "BlueHighway");
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
                                      const String& materialName, bool show) {

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
