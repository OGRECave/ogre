
#include "OgrePrerequisites.h"
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
            ProfileSample       *mRoot;
            ProfileSample       *mCurrentSample;
            Timer               *mTimer;

            uint64              mTotalAccumTime;

            FastArray<uint8_t*> mMemoryPool;
            size_t              mCurrMemoryPoolOffset;
            size_t              mBytesPerPool;

            LightweightMutex    mMutex;

            void createNewPool(void);
            void destroyAllPools(void);
            ProfileSample* allocateSample( ProfileSample *parent );

            static void destroySampleAndChildren( ProfileSample *sample );

            void dumpSample( ProfileSample *sample, LwString &tmpStr,
                             String &outCsvString, map<IdString, ProfileSample>::type &accumStats,
                             uint32 stackDepth );
        public:
            PerThreadData( size_t bytesPerPool );
            ~PerThreadData();

            void profileBegin( const char *name );
            void profileEnd(void);

            void dumpProfileResultsStr( String &outCsvStringPerFrame, String &outCsvStringAccum );
            void dumpProfileResults( const String &fullPathPerFrame, const String &fullPathAccum );
        };

        typedef FastArray<PerThreadData*> PerThreadDataArray;

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

        void profileBegin( const char *name );
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
