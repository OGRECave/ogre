
#include "OgreOfflineProfiler.h"
#include "OgreTimer.h"
#include "OgreLwString.h"

#include "OgreRoot.h"
#include "OgreLogManager.h"

namespace Ogre
{
    OfflineProfiler::OfflineProfiler() :
        mRoot( 0 ),
        mCurrentSample( 0 ),
        mTimer( OGRE_NEW Ogre::Timer() ),
        mTotalAccumTime( 0 ),
        mCurrMemoryPoolOffset( 0 ),
        mBytesPerPool( sizeof( ProfileSample ) * 10000 )
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
    OfflineProfiler::~OfflineProfiler()
    {
        if( !mCurrentSample->children.empty() &&
            (!mOnShutdownPerFramePath.empty() || !mOnShutdownAccumPath.empty()) )
        {
            dumpProfileResults( mOnShutdownPerFramePath, mOnShutdownAccumPath );
        }

        destroyAllPools();
        delete mTimer;
        mTimer = 0;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::destroySampleAndChildren( ProfileSample *sample )
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
    void OfflineProfiler::createNewPool(void)
    {
        uint8 *newPool = reinterpret_cast<uint8*>(
                             OGRE_MALLOC( mBytesPerPool, MEMCATEGORY_GENERAL ) );
        memset( newPool, 0, mBytesPerPool );
        mMemoryPool.push_back( newPool );
        mCurrMemoryPoolOffset = 0;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::destroyAllPools(void)
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
    OfflineProfiler::ProfileSample* OfflineProfiler::allocateSample( ProfileSample *parent )
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
    void OfflineProfiler::profileBegin( const char *name )
    {
        IdString nameHash( name );

        ProfileSample *sample = 0;

        //Look if our last sibling has the same name (i.e. similar behavior to RMTSF_Aggregate)
        if( !mCurrentSample->children.empty() )
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
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::profileEnd(void)
    {
        const uint64 usEnd = mTimer->getMicroseconds();
        const uint64 usTaken = usEnd - mCurrentSample->usStart;
        mCurrentSample->usTaken = usTaken;
        mCurrentSample = mCurrentSample->parent;

        OGRE_ASSERT_HIGH( mCurrentSample &&
                          "Called OfflineProfiler::profileEnd more times than profileBegin!" );

        if( !mCurrentSample->parent )
            mTotalAccumTime += usTaken;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::dumpSample( ProfileSample *sample, LwString &tmpStr,
                                      String &outCsvString,
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
    void OfflineProfiler::reset(void)
    {
        destroyAllPools();
        createNewPool();
        mTotalAccumTime = 0;
        mCurrentSample = allocateSample( 0 );
        mRoot = mCurrentSample;
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::dumpProfileResultsStr( String &outCsvStringPerFrame, String &outCsvStringAccum )
    {
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
    }
    //-----------------------------------------------------------------------------------
    void OfflineProfiler::dumpProfileResults( const String &fullPathPerFrame,
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
