
#include "OgreStableHeaders.h"

#include "OgreOfflineProfiler.h"
#include "OgreTimer.h"
#include "OgreLwString.h"

#include "OgreRoot.h"
#include "OgreLogManager.h"

namespace Ogre
{
    OfflineProfiler::OfflineProfiler() :
        mPaused( false ),
        mTlsHandle( OGRE_TLS_INVALID_HANDLE ),
        mBytesPerPool( sizeof( ProfileSample ) * 10000 )
    {
        Threads::CreateTls( &mTlsHandle );
    }
    //-----------------------------------------------------------------------------------
    OfflineProfiler::~OfflineProfiler()
    {
        if( !mThreadData.empty() &&
            (!mOnShutdownPerFramePath.empty() || !mOnShutdownAccumPath.empty()) )
        {
            dumpProfileResults( mOnShutdownPerFramePath, mOnShutdownAccumPath );
        }

        mMutex.lock();
        PerThreadDataArray::const_iterator itor = mThreadData.begin();
        PerThreadDataArray::const_iterator end  = mThreadData.end();

        while( itor != end )
            delete *itor++;
        mThreadData.clear();
        mMutex.unlock();

        Threads::DestroyTls( mTlsHandle );
        mTlsHandle = OGRE_TLS_INVALID_HANDLE;
    }
    //-----------------------------------------------------------------------------------
    OfflineProfiler::PerThreadData::PerThreadData( bool startPaused, size_t bytesPerPool ) :
        mPaused( startPaused ),
        mPauseRequest( startPaused ),
        mResetRequest( false ),
        mRoot( 0 ),
        mCurrentSample( 0 ),
        mTimer( OGRE_NEW Ogre::Timer() ),
        mTotalAccumTime( 0 ),
        mCurrMemoryPoolOffset( 0 ),
        mBytesPerPool( bytesPerPool )
    {
        createNewPool();
        mCurrentSample = allocateSample( 0 );
        mRoot = mCurrentSample;

        const char *rootName = "OfflineProfiler Root";
        strcpy( (char*)mCurrentSample->nameStr, rootName );
        mCurrentSample->nameStr[OGRE_OFFLINE_PROFILER_NAME_STR_LENGTH-1u] = '\0';
        mCurrentSample->nameHash = IdString( rootName );
    }
    //-----------------------------------------------------------------------------------
    OfflineProfiler::PerThreadData::~PerThreadData()
    {
        destroyAllPools();
        delete mTimer;
        mTimer = 0;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::destroySampleAndChildren( ProfileSample *sample )
    {
        FastArray<ProfileSample*>::const_iterator itor = sample->children.begin();
        FastArray<ProfileSample*>::const_iterator end  = sample->children.end();

        while( itor != end )
        {
            destroySampleAndChildren( *itor );
            (*itor)->children.destroy();
            ++itor;
        }

        sample->children.destroy();
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::createNewPool(void)
    {
        uint8 *newPool = reinterpret_cast<uint8*>(
                             OGRE_MALLOC( mBytesPerPool, MEMCATEGORY_GENERAL ) );
        memset( newPool, 0, mBytesPerPool );
        mMemoryPool.push_back( newPool );
        mCurrMemoryPoolOffset = 0;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::destroyAllPools(void)
    {
        if( mCurrentSample )
        {
            destroySampleAndChildren( mCurrentSample );
            mCurrentSample = 0;
        }

        FastArray<uint8_t*>::const_iterator itor = mMemoryPool.begin();
        FastArray<uint8_t*>::const_iterator end  = mMemoryPool.end();

        while( itor != end )
        {
            OGRE_FREE( *itor, MEMCATEGORY_GENERAL );
            ++itor;
        }

        mMemoryPool.clear();
        mCurrMemoryPoolOffset = 0;
    }
    //-----------------------------------------------------------------------------------
    OfflineProfiler::ProfileSample* OfflineProfiler::PerThreadData::allocateSample(
            ProfileSample *parent )
    {
        const size_t bytesNeeded = sizeof( ProfileSample );
        if( mCurrMemoryPoolOffset + bytesNeeded > mBytesPerPool )
            createNewPool();

        ProfileSample *newSample = reinterpret_cast<ProfileSample*>( mMemoryPool.back() +
                                                                     mCurrMemoryPoolOffset );
        newSample = new (newSample) ProfileSample();
        newSample->parent = parent;
        mCurrMemoryPoolOffset += bytesNeeded;

        return newSample;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::setPauseRequest( bool bPause )
    {
        mPauseRequest = bPause;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::requestReset(void)
    {
        mResetRequest = true;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::reset(void)
    {
        destroyAllPools();
        createNewPool();
        mTotalAccumTime = 0;
        mCurrentSample = allocateSample( 0 );
        mRoot = mCurrentSample;
        mResetRequest = false;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::profileBegin( const char *name,
                                                       ProfileSampleFlags::ProfileSampleFlags flags )
    {
        if( mPaused != mPauseRequest )
            mPaused = mPauseRequest;

        if( mResetRequest )
            reset();

        if( mPaused )
            return;

        mMutex.lock();
        IdString nameHash( name );

        ProfileSample *sample = 0;

        //Look if our last sibling has the same name (i.e. similar behavior to RMTSF_Aggregate)
        if( flags == ProfileSampleFlags::Aggregate && !mCurrentSample->children.empty() )
        {
            if( mCurrentSample->children.back()->nameHash == nameHash )
                sample = mCurrentSample->children.back();
        }

        if( !sample )
        {
            sample = allocateSample( mCurrentSample );
            mCurrentSample->children.push_back( sample );

            strcpy( (char*)sample->nameStr, name );
            sample->nameStr[OGRE_OFFLINE_PROFILER_NAME_STR_LENGTH-1u] = '\0';
            sample->nameHash = nameHash;
            sample->usStart = mTimer->getMicroseconds();
        }

        mCurrentSample = sample;
        mMutex.unlock();
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::profileEnd(void)
    {
        if( mPaused )
            return;

        //Measure before the lock! Other threads will not be using our mTimer anyway
        const uint64 usEnd = mTimer->getMicroseconds();

        mMutex.lock();
        const uint64 usTaken = usEnd - mCurrentSample->usStart;
        mCurrentSample->usTaken = usTaken;
        mCurrentSample = mCurrentSample->parent;

        OGRE_ASSERT_HIGH( mCurrentSample &&
                          "Called OfflineProfiler::profileEnd more times than profileBegin!" );

        if( !mCurrentSample->parent )
            mTotalAccumTime += usTaken;
        mMutex.unlock();
    }
    //-----------------------------------------------------------------------------------
    OfflineProfiler::PerThreadData* OfflineProfiler::allocatePerThreadData(void)
    {
        PerThreadData *perThreadData = new PerThreadData( mPaused, mBytesPerPool );

        mMutex.lock();
        mThreadData.push_back( perThreadData );
        mMutex.unlock();

        Threads::SetTls( mTlsHandle, perThreadData );

        return perThreadData;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::
    PerThreadData::dumpSample( ProfileSample *sample, LwString &tmpStr, String &outCsvString,
                               map<IdString, ProfileSample>::type &accumStats, uint32 stackDepth )
    {
        map<IdString, ProfileSample>::type::iterator itAccum = accumStats.find( sample->nameHash );
        if( itAccum == accumStats.end() )
        {
            ProfileSample newSample;
            memcpy( newSample.nameStr, sample->nameStr, OGRE_OFFLINE_PROFILER_NAME_STR_LENGTH );
            newSample.usTaken = 0;
            accumStats[sample->nameHash] = newSample;
            itAccum = accumStats.find( sample->nameHash );
        }

        itAccum->second.usTaken += sample->usTaken;

        const float msTaken = (float)(((double)sample->usTaken) / 1000.0);

        tmpStr.clear();
        tmpStr.a( stackDepth, "|", (const char*)sample->nameStr, "|",
                  LwString::Float( msTaken, 2 ), "|" );
        tmpStr.a( LwString::Float( (float)(100.0 * ((double)sample->usTaken /
                                           (double)mTotalAccumTime)), 2 ), "%\n" );
        outCsvString += tmpStr.c_str();

        FastArray<ProfileSample*>::const_iterator itor = sample->children.begin();
        FastArray<ProfileSample*>::const_iterator end  = sample->children.end();

        while( itor != end )
        {
            dumpSample( *itor, tmpStr, outCsvString, accumStats, stackDepth + 1u );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::
    PerThreadData::dumpProfileResultsStr( String &outCsvStringPerFrame, String &outCsvStringAccum )
    {
        mMutex.lock();
        String csvString0;
        String csvString1;

        csvString0 += "Stack Depth|Name|Milliseconds|%\n";

        char tmpBuffer[128];
        LwString tmpStr( LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );

        map<IdString, ProfileSample>::type accumStats;

        {
            uint32 stackDepth = 0;

            FastArray<ProfileSample*>::const_iterator itor = mRoot->children.begin();
            FastArray<ProfileSample*>::const_iterator end  = mRoot->children.end();

            while( itor != end )
            {
                dumpSample( *itor, tmpStr, csvString0, accumStats, stackDepth + 1u );
                ++itor;
            }
        }

        {
            csvString1 += "Name|Milliseconds|%\n";

            map<IdString, ProfileSample>::type::const_iterator itor = accumStats.begin();
            map<IdString, ProfileSample>::type::const_iterator end  = accumStats.end();

            while( itor != end )
            {
                const ProfileSample *sample = &itor->second;
                const float msTaken = (float)(((double)sample->usTaken) / 1000.0);
                tmpStr.clear();
                tmpStr.a( (const char*)sample->nameStr, "|", LwString::Float( msTaken, 2 ), "|" );
                tmpStr.a( LwString::Float( (float)(100.0 * ((double)sample->usTaken /
                                                   (double)mTotalAccumTime)), 2 ), "%\n" );
                csvString1 += tmpStr.c_str();
                ++itor;
            }
        }

        outCsvStringPerFrame.swap( csvString0 );
        outCsvStringAccum.swap( csvString1 );

        reset();
        mMutex.unlock();
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::PerThreadData::dumpProfileResults( const String &fullPathPerFrame,
                                                             const String &fullPathAccum )
    {
        String csvStringPerFrame;
        String csvStringAccum;
        dumpProfileResultsStr( csvStringPerFrame, csvStringAccum );

        if( !fullPathPerFrame.empty() )
        {
            std::ofstream outFile( fullPathPerFrame.c_str(), std::ios::binary | std::ios::out );
            outFile.write( (const char*)&csvStringPerFrame[0], csvStringPerFrame.size() );
            outFile.close();
        }

        if( !fullPathAccum.empty() )
        {
            std::ofstream outFile( fullPathAccum.c_str(), std::ios::binary | std::ios::out );
            outFile.write( (const char*)&csvStringAccum[0], csvStringAccum.size() );
            outFile.close();
        }
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::setPaused( bool bPaused )
    {
        if( mPaused == bPaused )
            return;

        mPaused = bPaused;

        mMutex.lock();

        PerThreadDataArray::const_iterator itor = mThreadData.begin();
        PerThreadDataArray::const_iterator end  = mThreadData.end();

        while( itor != end )
        {
            (*itor)->setPauseRequest( bPaused );
            ++itor;
        }

        mMutex.unlock();
    }
    //-----------------------------------------------------------------------------------
    bool OfflineProfiler::isPaused(void) const
    {
        return mPaused;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::reset(void)
    {
        mMutex.lock();

        PerThreadDataArray::const_iterator itor = mThreadData.begin();
        PerThreadDataArray::const_iterator end  = mThreadData.end();

        while( itor != end )
        {
            (*itor)->requestReset();
            ++itor;
        }

        mMutex.unlock();
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::profileBegin( const char *name, ProfileSampleFlags::ProfileSampleFlags flags )
    {
        PerThreadData *perThreadData = reinterpret_cast<PerThreadData*>( Threads::GetTls( mTlsHandle ) );

        if( !perThreadData )
            perThreadData = allocatePerThreadData();

        perThreadData->profileBegin( name, flags );
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::profileEnd(void)
    {
        PerThreadData *perThreadData = reinterpret_cast<PerThreadData*>( Threads::GetTls( mTlsHandle ) );

        OGRE_ASSERT_HIGH( perThreadData && "Called OfflineProfiler::profileEnd before profileBegin!" );

        perThreadData->profileEnd();
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::dumpProfileResults( const String &fullPathPerFrame,
                                              const String &fullPathAccum )
    {
        mMutex.lock();

        String actualFullPathPerFrame = fullPathPerFrame;
        String actualFullPathAccum = fullPathAccum;

        size_t idx = 0;

        PerThreadDataArray::const_iterator itor = mThreadData.begin();
        PerThreadDataArray::const_iterator end  = mThreadData.end();

        while( itor != end )
        {
            actualFullPathPerFrame.resize( fullPathPerFrame.size() );
            actualFullPathAccum.resize( fullPathAccum.size() );

            if( !actualFullPathPerFrame.empty() )
                actualFullPathPerFrame += StringConverter::toString( idx ) + ".csv";
            if( !actualFullPathAccum.empty() )
                actualFullPathAccum += StringConverter::toString( idx ) + ".csv";

            (*itor)->dumpProfileResults( actualFullPathPerFrame, actualFullPathAccum );
            ++idx;
            ++itor;
        }

        mMutex.unlock();
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::setDumpPathsOnShutdown( const String &fullPathPerFrame,
                                                  const String &fullPathAccum )
    {
        mOnShutdownPerFramePath = fullPathPerFrame;
        mOnShutdownAccumPath = fullPathAccum;

        if( !fullPathPerFrame.empty() || !fullPathAccum.empty() )
        {
            LogManager::getSingleton().logMessage( "[INFO] Will log profiling results on shutdown to " +
                                                   fullPathPerFrame + " and " + fullPathAccum );
        }
    }
}
