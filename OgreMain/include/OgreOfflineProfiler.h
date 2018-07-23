
#ifndef _OgreOfflineProfiler_H_
#define _OgreOfflineProfiler_H_

#include "OgrePrerequisites.h"
#include "OgreProfilerCommon.h"
#include "OgreIdString.h"
#include "Threading/OgreLightweightMutex.h"
#include "Threading/OgreThreads.h"

namespace Ogre
{
#define OGRE_OFFLINE_PROFILER_NAME_STR_LENGTH 64
    class LwString;

    /**
    @class OfflineProfiler
        Simple profiler that will produce a CSV file for offline
        analysis once dumpProfileResults is called
    @remarks
        Because this profiler collects sample undefinitely, it will cause
        memory consumption to grow over time.
        You can use setPaused to halt such growth temporarily, which is
        specially useful if whatever you want to profile is localized to
        particular execution moment.
    */
    class _OgreExport OfflineProfiler
    {
        struct ProfileSample
        {
            uint8       nameStr[OGRE_OFFLINE_PROFILER_NAME_STR_LENGTH];
            IdString    nameHash;
            uint64      usStart;
            uint64      usTaken;

            ProfileSample               *parent;
            FastArray<ProfileSample*>   children;
        };

        class PerThreadData
        {
            bool                mPaused;
            bool                mPauseRequest;
            bool                mResetRequest;
            ProfileSample       *mRoot;
            ProfileSample       *mCurrentSample;
            Timer               *mTimer;

            uint64              mTotalAccumTime;

            FastArray<uint8_t*> mMemoryPool;
            size_t              mCurrMemoryPoolOffset;
            size_t              mBytesPerPool;

            /** Protects:
                    * mCurrentSample
                    * mMemoryPool
                    * mCurrMemoryPoolOffset
                    * mTotalAccumTime
            */
            LightweightMutex    mMutex;

            void createNewPool(void);
            void destroyAllPools(void);
            ProfileSample* allocateSample( ProfileSample *parent );

            static void destroySampleAndChildren( ProfileSample *sample );

            void dumpSample( ProfileSample *sample, LwString &tmpStr,
                             String &outCsvString, map<IdString, ProfileSample>::type &accumStats,
                             uint32 stackDepth );

            void reset(void);

        public:
            PerThreadData( bool startPaused, size_t bytesPerPool );
            ~PerThreadData();

            void setPauseRequest( bool bPause );
            void requestReset(void);

            void profileBegin( const char *name, ProfileSampleFlags::ProfileSampleFlags flags );
            void profileEnd(void);

            void dumpProfileResultsStr( String &outCsvStringPerFrame, String &outCsvStringAccum );
            void dumpProfileResults( const String &fullPathPerFrame, const String &fullPathAccum );
        };

        typedef FastArray<PerThreadData*> PerThreadDataArray;

        bool                mPaused;

        LightweightMutex	mMutex;		//Protects mThreadData
        TlsHandle			mTlsHandle;
        PerThreadDataArray	mThreadData;

        size_t              mBytesPerPool;

        String              mOnShutdownPerFramePath;
        String              mOnShutdownAccumPath;

        PerThreadData* allocatePerThreadData(void);

    public:
        OfflineProfiler();
        ~OfflineProfiler();

        /** Pauses collection of samples. Note that other threads may be in the middle of a
            collection (i.e. they've called profileBegin but haven't yet called profileEnd).
            Threads will pause after their collection ends (i.e. only after they end up
            calling profileEnd).

            Likewise, if you resume sampling, a thread that already called profileBegin while
            it was paused, thus it will resume sampling in its next profileBegin call.

            TL;DR: It is thread safe, but do not assume pause is immediate (i.e. worker threads
            may still add a new sample after calling setPaused( true ))
        @param bPaused
            True to pause. False to resume.
        */
        void setPaused( bool bPaused );

        /// Returns true if sampling is paused. Note that some worker threads may still be
        /// collecting samples even if this returns true (and likewise, they may take a
        /// bit of time to resume again)
        ///
        /// @see    OfflineProfiler::setPaused
        bool isPaused(void) const;

        /// Destroys all collected samples and starts over. Worker threads will
        /// honour this request as soon as they see it (which is the next time
        /// they call profileBegin).
        void reset(void);

        void profileBegin( const char *name, ProfileSampleFlags::ProfileSampleFlags flags );
        void profileEnd(void);

        /** Dumps CSV data into two CSV files
        @param fullPathPerFrame
            Full path to csv without extension to generate where to dump the per-frame CSV data.
            Empty string to skip it.
            Note that the CSV extension will be appended, and the actual filename
            my vary as there will be one file per thread that was collected.
        @param fullPathAccum
            Full path to csv without extension to generate where to dump the per-frame CSV data.
            Empty string to skip it.
            Note that the CSV extension will be appended, and the actual filename
            my vary as there will be one file per thread that was collected.
        */
        void dumpProfileResults( const String &fullPathPerFrame, const String &fullPathAccum );

        /** Ogre will call dumpProfileResults for your on shutdown if you set these paths
        @param fullPathPerFrame
            Full path to csv without extension to generate where to dump the per-frame CSV data.
            Empty string to skip it.
            Note that the CSV extension will be appended, and the actual filename
            my vary as there will be one file per thread that was collected.
        @param fullPathAccum
            Full path to csv without extension to generate where to dump the per-frame CSV data.
            Empty string to skip it.
            Note that the CSV extension will be appended, and the actual filename
            my vary as there will be one file per thread that was collected.
        @see    OfflineProfiler::dumpProfileResults
        */
        void setDumpPathsOnShutdown( const String &fullPathPerFrame, const String &fullPathAccum );
    };
}

#endif
