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
/*

    Although the code is original, many of the ideas for the profiler were borrowed from 
"Real-Time In-Game Profiling" by Steve Rabin which can be found in Game Programming
Gems 1.

    This code can easily be adapted to your own non-Ogre project. The only code that is 
Ogre-dependent is in the visualization/logging routines and the use of the Timer class.

    Enjoy!

*/

#ifndef __Profiler_H__
#define __Profiler_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreString.h"
#include "OgreOverlay.h"

#if OGRE_PROFILING == 1
#	define OgreProfile( a ) Ogre::Profile _OgreProfileInstance( (a) )
#	define OgreProfileBegin( a ) Ogre::Profiler::getSingleton().beginProfile( (a) )
#	define OgreProfileEnd( a ) Ogre::Profiler::getSingleton().endProfile( (a) )
#	define OgreProfileGroup( a, g ) Ogre::Profile _OgreProfileInstance( (a), (g) )
#	define OgreProfileBeginGroup( a, g ) Ogre::Profiler::getSingleton().beginProfile( (a), (g) )
#	define OgreProfileEndGroup( a, g ) Ogre::Profiler::getSingleton().endProfile( (a), (g) )
#	define OgreProfileBeginGPUEvent( g ) Ogre::Profiler::getSingleton().beginGPUEvent(g)
#	define OgreProfileEndGPUEvent( g ) Ogre::Profiler::getSingleton().endGPUEvent(g)
#	define OgreProfileMarkGPUEvent( e ) Ogre::Profiler::getSingleton().markGPUEvent(e)
#else
#   define OgreProfile( a )
#   define OgreProfileBegin( a )
#   define OgreProfileEnd( a )
#	define OgreProfileGroup( a, g ) 
#	define OgreProfileBeginGroup( a, g ) 
#	define OgreProfileEndGroup( a, g ) 
#	define OgreProfileBeginGPUEvent( e )
#	define OgreProfileEndGPUEvent( e )
#	define OgreProfileMarkGPUEvent( e )
#endif

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

	/** List of reserved profiling masks
	*/
	enum ProfileGroupMask
	{
		/// User default profile
		OGREPROF_USER_DEFAULT = 0x00000001,
		/// All in-built Ogre profiling will match this mask
		OGREPROF_ALL = 0xFF000000,
		/// General processing
		OGREPROF_GENERAL = 0x80000000,
		/// Culling
		OGREPROF_CULLING = 0x40000000,
		/// Rendering
		OGREPROF_RENDERING = 0x20000000
	};

    /** An individual profile that will be processed by the Profiler
        @remarks
            Use the macro OgreProfile(name) instead of instantiating this profile directly
        @remarks
            We use this Profile to allow scoping rules to signify the beginning and end of
            the profile. Use the Profiler singleton (through the macro OgreProfileBegin(name)
            and OgreProfileEnd(name)) directly if you want a profile to last
            outside of a scope (i.e. the main game loop).
        @author Amit Mathew (amitmathew (at) yahoo (dot) com)
    */
	class _OgreExport Profile : 
		public ProfilerAlloc 
	{

        public:
            Profile(const String& profileName, uint32 groupID = (uint32)OGREPROF_USER_DEFAULT);
            ~Profile();

        protected:

            /// The name of this profile
            String mName;
			/// The group ID
			uint32 mGroupID;
			

    };

    /** The profiler allows you to measure the performance of your code
        @remarks
            Do not create profiles directly from this unless you want a profile to last
            outside of its scope (i.e. the main game loop). For most cases, use the macro
            OgreProfile(name) and braces to limit the scope. You must enable the Profile
            before you can used it with setEnabled(true). If you want to disable profiling
            in Ogre, simply set the macro OGRE_PROFILING to 0.
        @author Amit Mathew (amitmathew (at) yahoo (dot) com)
        @todo resolve artificial cap on number of profiles displayed
        @todo fix display ordering of profiles not called every frame
    */
    class _OgreExport Profiler : 
		public Singleton<Profiler>,
		public ProfilerAlloc
	{
        public:
            Profiler();
            ~Profiler();

            /** Sets the timer for the profiler */
            void setTimer(Timer* t);

            /** Retrieves the timer for the profiler */
            Timer* getTimer();

            /** Begins a profile
            @remarks 
                Use the macro OgreProfileBegin(name) instead of calling this directly 
                so that profiling can be ignored in the release version of your app. 
            @remarks 
                You only use the macro (or this) if you want a profile to last outside
                of its scope (i.e. the main game loop). If you use this function, make sure you 
                use a corresponding OgreProfileEnd(name). Usually you would use the macro 
                OgreProfile(name). This function will be ignored for a profile that has been 
                disabled or if the profiler is disabled.
            @param profileName Must be unique and must not be an empty string
			@param groupID A profile group identifier, which can allow you to mask profiles
            */
            void beginProfile(const String& profileName, uint32 groupID = (uint32)OGREPROF_USER_DEFAULT);

            /** Ends a profile
            @remarks 
                Use the macro OgreProfileEnd(name) instead of calling this directly so that
                profiling can be ignored in the release version of your app.
            @remarks
                This function is usually not called directly unless you want a profile to
                last outside of its scope. In most cases, using the macro OgreProfile(name) 
                which will call this function automatically when it goes out of scope. Make 
                sure the name of this profile matches its corresponding beginProfile name. 
                This function will be ignored for a profile that has been disabled or if the
                profiler is disabled.
			@param profileName Must be unique and must not be an empty string
			@param groupID A profile group identifier, which can allow you to mask profiles
            */
            void endProfile(const String& profileName, uint32 groupID = (uint32)OGREPROF_USER_DEFAULT);

            /** Mark the beginning of a GPU event group
             @remarks Can be safely called in the middle of the profile.
             */
            void beginGPUEvent(const String& event);

            /** Mark the end of a GPU event group
             @remarks Can be safely called in the middle of the profile.
             */
            void endGPUEvent(const String& event);

            /** Mark a specific, ungrouped, GPU event
             @remarks Can be safely called in the middle of the profile.
             */
            void markGPUEvent(const String& event);

            /** Sets whether this profiler is enabled. Only takes effect after the
                the frame has ended.
                @remarks When this is called the first time with the parameter true,
                it initializes the GUI for the Profiler
            */
            void setEnabled(bool enabled);

            /** Gets whether this profiler is enabled */
            bool getEnabled() const;

            /** Enables a previously disabled profile 
            @remarks Can be safely called in the middle of the profile.
            */
            void enableProfile(const String& profileName);

            /** Disables a profile
            @remarks Can be safely called in the middle of the profile.
            */
            void disableProfile(const String& profileName);

			/** Set the mask which all profiles must pass to be enabled. 
			*/
			void setProfileGroupMask(uint32 mask) { mProfileMask = mask; }
			/** Get the mask which all profiles must pass to be enabled. 
			*/
			uint32 getProfileGroupMask() const { return mProfileMask; }

            /** Returns true if the specified profile reaches a new frame time maximum
            @remarks If this is called during a frame, it will be reading the results
            from the previous frame. Therefore, it is best to use this after the frame
            has ended.
            */
            bool watchForMax(const String& profileName);

            /** Returns true if the specified profile reaches a new frame time minimum
            @remarks If this is called during a frame, it will be reading the results
            from the previous frame. Therefore, it is best to use this after the frame
            has ended.
            */
            bool watchForMin(const String& profileName);

            /** Returns true if the specified profile goes over or under the given limit
                frame time
            @remarks If this is called during a frame, it will be reading the results
            from the previous frame. Therefore, it is best to use this after the frame
            has ended.
            @param limit A number between 0 and 1 representing the percentage of frame time
            @param greaterThan If true, this will return whether the limit is exceeded. Otherwise,
            it will return if the frame time has gone under this limit.
            */
            bool watchForLimit(const String& profileName, Real limit, bool greaterThan = true);

            /** Outputs current profile statistics to the log */
            void logResults();

            /** Clears the profiler statistics */
            void reset();

			enum DisplayMode
			{
				/// Display % frame usage on the overlay
				DISPLAY_PERCENTAGE,
				/// Display milliseconds on the overlay
				DISPLAY_MILLISECONDS
			};

			/** Set the display mode for the overlay. 
			*/
			void setDisplayMode(DisplayMode d) { mDisplayMode = d; }
			/** Get the display mode for the overlay. 
			*/
			DisplayMode getDisplayMode() const { return mDisplayMode; }

            /** Sets the Profiler so the display of results are updated every n frames*/
            void setUpdateDisplayFrequency(uint freq);

            /** Gets the frequency that the Profiler display is updated */
            uint getUpdateDisplayFrequency() const;

			/** Set the size of the profiler overlay, in pixels. */
			void setOverlayDimensions(Real width, Real height);

			/** Set the position of the profiler overlay, in pixels. */
			void setOverlayPosition(Real left, Real top);

			Real getOverlayWidth() const;
			Real getOverlayHeight() const;
			Real getOverlayLeft() const;
			Real getOverlayTop() const;

            /** Override standard Singleton retrieval.
            @remarks
            Why do we do this? Well, it's because the Singleton
            implementation is in a .h file, which means it gets compiled
            into anybody who includes it. This is needed for the
            Singleton template to work, but we actually only want it
            compiled into the implementation of the class based on the
            Singleton, not all of them. If we don't change this, we get
            link errors when trying to use the Singleton-based class from
            an outside dll.
            @par
            This method just delegates to the template version anyway,
            but the implementation stays in this single compilation unit,
            preventing link errors.
            */
            static Profiler& getSingleton(void);
            /** Override standard Singleton retrieval.
            @remarks
            Why do we do this? Well, it's because the Singleton
            implementation is in a .h file, which means it gets compiled
            into anybody who includes it. This is needed for the
            Singleton template to work, but we actually only want it
            compiled into the implementation of the class based on the
            Singleton, not all of them. If we don't change this, we get
            link errors when trying to use the Singleton-based class from
            an outside dll.
            @par
            This method just delegates to the template version anyway,
            but the implementation stays in this single compilation unit,
            preventing link errors.
            */
            static Profiler* getSingletonPtr(void);

			

        protected:

            /** Initializes the profiler's GUI elements */
            void initialize();

			class ProfileInstance;
			friend class ProfileInstance;
            typedef list<OverlayElement*>::type ProfileBarList;

			void displayResults(void);
            /** Prints the profiling results of each frame 
            @remarks Recursive, for all the little children. */
            void displayResults(ProfileInstance* instance, ProfileBarList::iterator& bIter, Real& maxTimeMillisecs, Real& newGuiHeight, int& profileCount);

			/** Processes frame stats for all of the mRoot's children */
			void processFrameStats(void);
            /** Processes specific ProfileInstance and it's children recursively.*/
            void processFrameStats(ProfileInstance* instance, Real& maxFrameTime);

            /** Handles a change of the profiler's enabled state*/
            void changeEnableState();

            /** An internal function to create the container which will hold our display elements*/
            OverlayContainer* createContainer();

            /** An internal function to create a text area */
            OverlayElement* createTextArea(const String& name, Real width, Real height, Real top, Real left, 
                                       uint fontSize, const String& caption, bool show = true);

            /** An internal function to create a panel */
            OverlayElement* createPanel(const String& name, Real width, Real height, Real top, Real left, 
                                    const String& materialName, bool show = true);


            /// Represents the total timing information of a profile
            /// since profiles can be called more than once each frame
            struct ProfileFrame 
			{

                /// The total time this profile has taken this frame
                ulong	frameTime;

                /// The number of times this profile was called this frame
                uint	calls;

                /// The hierarchical level of this profile, 0 being the main loop
                uint	hierarchicalLvl;

            };
			
            /// Represents a history of each profile during the duration of the app
            struct ProfileHistory 
			{
                /// The current percentage of frame time this profile has taken
                Real	currentTimePercent; 
				/// The current frame time this profile has taken in milliseconds
				Real	currentTimeMillisecs;

                /// The maximum percentage of frame time this profile has taken
                Real	maxTimePercent; 
				/// The maximum frame time this profile has taken in milliseconds
				Real	maxTimeMillisecs; 

                /// The minimum percentage of frame time this profile has taken
                Real	minTimePercent; 
				/// The minimum frame time this profile has taken in milliseconds
				Real	minTimeMillisecs; 

                /// The number of times this profile has been called each frame
                uint	numCallsThisFrame;

                /// The total percentage of frame time this profile has taken
                Real	totalTimePercent;
				/// The total frame time this profile has taken in milliseconds
				Real	totalTimeMillisecs;

                /// The total number of times this profile was called
                /// (used to calculate average)
                ulong	totalCalls; 

                /// The hierarchical level of this profile, 0 being the root profile
                uint	hierarchicalLvl;

			};

			/// Represents an individual profile call
			class ProfileInstance :
				public ProfilerAlloc
			{
				friend class Profiler;
			public:
				ProfileInstance(void);
				virtual ~ProfileInstance(void);

				typedef Ogre::map<String,ProfileInstance*>::type ProfileChildren;

				void logResults();
				void reset();

				inline bool watchForMax(void) { return history.currentTimePercent == history.maxTimePercent; }
				inline bool watchForMin(void) { return history.currentTimePercent == history.minTimePercent; }
				inline bool watchForLimit(Real limit, bool greaterThan = true)
				{
					if (greaterThan)
						return history.currentTimePercent > limit;
					else
						return history.currentTimePercent < limit;
				}

				bool watchForMax(const String& profileName);
				bool watchForMin(const String& profileName);
				bool watchForLimit(const String& profileName, Real limit, bool greaterThan = true);
                                
				/// The name of the profile
				String          name;

				/// The name of the parent, null if root
				ProfileInstance* parent;

				ProfileChildren children;

				ProfileFrame frame;
				ulong frameNumber;

				ProfileHistory history;

				/// The time this profile was started
				ulong           currTime;

				/// Represents the total time of all child profiles to subtract
				/// from this profile
				ulong           accum;

				/// The hierarchical level of this profile, 0 being the root profile
				uint            hierarchicalLvl;
			};

			// lol. Uses typedef; put's original container type in name.
            typedef set<String>::type DisabledProfileMap;
			typedef ProfileInstance::ProfileChildren ProfileChildren;

			ProfileInstance* mCurrent;
			ProfileInstance* mLast;
			ProfileInstance mRoot;

            /// Holds the names of disabled profiles
            DisabledProfileMap mDisabledProfiles;

            /// Holds the display bars for each profile results
            ProfileBarList mProfileBars;

            /// Whether the GUI elements have been initialized
            bool mInitialized;

            /// The max number of profiles we can display
            uint mMaxDisplayProfiles;

            /// The overlay which contains our profiler results display
            Overlay* mOverlay;

            /// The window that displays the profiler results
            OverlayContainer* mProfileGui;

            /// The height of each bar
            Real mBarHeight;

            /// The height of the stats window
            Real mGuiHeight;

            /// The width of the stats window
            Real mGuiWidth;

			/// The horz position of the stats window
			Real mGuiLeft;

			/// The vertical position of the stats window
			Real mGuiTop;

			/// The size of the indent for each profile display bar
            Real mBarIndent;

            /// The width of the border between the profile window and each bar
            Real mGuiBorderWidth;

            /// The width of the min, avg, and max lines in a profile display
            Real mBarLineWidth;

			/// The distance between bars
			Real mBarSpacing;

            /// The number of frames that must elapse before the current
            /// frame display is updated
            uint mUpdateDisplayFrequency;

            /// The number of elapsed frame, used with mUpdateDisplayFrequency
            uint mCurrentFrame;

            /// The timer used for profiling
            Timer* mTimer;

            /// The total time each frame takes
            ulong mTotalFrameTime;

            /// Whether this profiler is enabled
            bool mEnabled;

            /// Keeps track of the new enabled/disabled state that the user has requested
            /// which will be applied after the frame ends
            bool mNewEnableState;

			/// Mask to decide whether a type of profile is enabled or not
			uint32 mProfileMask;

			/// How to display the overlay
			DisplayMode mDisplayMode;

			/// The max frame time recorded
			ulong mMaxTotalFrameTime;

			/// Rolling average of millisecs
			Real mAverageFrameTime;
			bool mResetExtents;


    }; // end class
	/** @} */
	/** @} */

} // end namespace

#endif
