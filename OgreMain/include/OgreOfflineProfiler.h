
#include "OgrePrerequisites.h"
#include "OgreIdString.h"

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

        ProfileSample       *mRoot;
        ProfileSample       *mCurrentSample;
        Timer               *mTimer;

        uint64              mTotalAccumTime;

        FastArray<uint8_t*> mMemoryPool;
        size_t              mCurrMemoryPoolOffset;
        size_t              mBytesPerPool;

        String              mOnShutdownPerFramePath;
        String              mOnShutdownAccumPath;

        void destroySampleAndChildren( ProfileSample *sample );

        void createNewPool(void);
        void destroyAllPools(void);
        ProfileSample* allocateSample( ProfileSample *parent );

        void dumpSample( ProfileSample *sample, LwString &tmpStr,
                         String &outCsvString, map<IdString, ProfileSample>::type &accumStats,
                         uint32 stackDepth );

    public:
        OfflineProfiler();
        ~OfflineProfiler();

        void profileBegin( const char *name );
        void profileEnd(void);

        /// Destroys all collected samples and starts over
        void reset(void);

        /** Dumps CSV data into the input strings
        @param outCsvStringPerFrame [out]
            CSV data containing per-frame stats
        @param outCsvStringAccum [out]
            CSV data containing accumulated stats from all frames for each sample name
            Useful as summary
        */
        void dumpProfileResultsStr( String &outCsvStringPerFrame, String &outCsvStringAccum );

        /** Dumps CSV data into two CSV files
        @param fullPathPerFrame
            Full path to csv to generate where to dump the per-frame CSV data.
            Empty string to skip it.
        @param fullPathAccum
            Full path to csv to generate where to dump the accumulated CSV data.
            Empty string to skip it.
        */
        void dumpProfileResults( const String &fullPathPerFrame, const String &fullPathAccum );

        /** Ogre will call dumpProfileResults for your on shutdown if you set these paths
        @param fullPathPerFrame
            Full path to csv to generate where to dump the per-frame CSV data.
            Empty string to skip it.
        @param fullPathAccum
            Full path to csv to generate where to dump the accumulated CSV data.
            Empty string to skip it.
        @see    OfflineProfiler::dumpProfileResults
        */
        void setDumpPathsOnShutdown( const String &fullPathPerFrame, const String &fullPathAccum );
    };
}
