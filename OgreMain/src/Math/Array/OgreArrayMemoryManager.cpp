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

#include "Math/Array/OgreArrayConfig.h"
#include "Math/Array/OgreArrayMemoryManager.h"
#include "Math/Array/OgreArrayQuaternion.h"
#include "Math/Array/OgreArrayAabb.h"
#include "Math/Simple/OgreAabb.h"
#include "Math/Array/OgreArrayAabb.h"
#include "Math/Array/OgreArrayMatrix4.h"

#include "OgreMatrix4.h"

#include "OgreException.h"

namespace Ogre
{
    const size_t ArrayMemoryManager::MAX_MEMORY_SLOTS = (size_t)(-ARRAY_PACKED_REALS) - 1
                                                            - OGRE_PREFETCH_SLOT_DISTANCE;

    ArrayMemoryManager::ArrayMemoryManager( size_t const *elementsMemSize,
                                            const CleanupRoutines *initRoutines,
                                            CleanupRoutines const *cleanupRoutines,
                                            size_t numElementsSize, uint16 depthLevel,
                                            size_t hintMaxNodes, size_t cleanupThreshold,
                                            size_t maxHardLimit, RebaseListener *rebaseListener ) :
                            mElementsMemSizes( elementsMemSize ),
                            mInitRoutines( initRoutines ),
                            mCleanupRoutines( cleanupRoutines ),
                            mTotalMemoryMultiplier( 0 ),
                            mUsedMemory( 0 ),
                            mMaxMemory( hintMaxNodes ),
                            mMaxHardLimit( maxHardLimit ),
                            mCleanupThreshold( cleanupThreshold ),
                            mRebaseListener( rebaseListener ),
                            mLevel( depthLevel )
    {
        //If the assert triggers, their values will overflow to 0 when
        //trying to round to nearest multiple of ARRAY_PACKED_REALS
        assert( mMaxHardLimit < (size_t)(-ARRAY_PACKED_REALS) &&
                mMaxMemory < (size_t)(-ARRAY_PACKED_REALS) );
        assert( mMaxMemory <= mMaxHardLimit );

        mMemoryPools.resize( numElementsSize, 0 );
        for( size_t i=0; i<numElementsSize; ++i )
            mTotalMemoryMultiplier += mElementsMemSizes[i];

        //If mMaxMemory == 1, it cannot grow because 1/2 truncates to 0
        mMaxMemory      = std::max<size_t>( 2, mMaxMemory ) + OGRE_PREFETCH_SLOT_DISTANCE;
        mMaxHardLimit   += OGRE_PREFETCH_SLOT_DISTANCE;

        // Round up max memory & hard limit to the next multiple of ARRAY_PACKED_REALS
        mMaxMemory    = ( (mMaxMemory + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS ) *
                            ARRAY_PACKED_REALS;
        mMaxHardLimit = ( (mMaxHardLimit + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS ) *
                            ARRAY_PACKED_REALS;

        if( !mRebaseListener )
        {
            //If there's no listener to rebase, we can't later grow the memory pool or perform cleanups.
            mMaxHardLimit       = mMaxMemory;
            mCleanupThreshold   = -1;
        }
    }
    //-----------------------------------------------------------------------------------
    void ArrayMemoryManager::initialize()
    {
        assert( mUsedMemory == 0 && "Calling initialize twice"
                " with used slots will cause dangling ptrs" );
        destroy();

        size_t i=0;
        MemoryPoolVec::iterator itor = mMemoryPools.begin();
        MemoryPoolVec::iterator end  = mMemoryPools.end();

        while( itor != end )
        {
//          *itor = reinterpret_cast<char*>(_aligned_malloc(mMaxMemory * ElementsMemSize[i], 16));
            *itor = (char*)OGRE_MALLOC_SIMD( mMaxMemory * mElementsMemSizes[i],
                                             MEMCATEGORY_SCENE_OBJECTS );
            if( mInitRoutines && mInitRoutines[i] )
                mInitRoutines[i]( *itor, 0, 0, 0, 0, mMaxMemory, mElementsMemSizes[i] );
            else
                memset( *itor, 0, mMaxMemory * mElementsMemSizes[i] );
            ++i;
            ++itor;
        }

        initializeEmptySlots( 0 );
    }
    //-----------------------------------------------------------------------------------
    void ArrayMemoryManager::destroy()
    {
        MemoryPoolVec::iterator itor = mMemoryPools.begin();
        MemoryPoolVec::iterator end  = mMemoryPools.end();

        while( itor != end )
        {
            OGRE_FREE_SIMD( *itor, MEMCATEGORY_SCENE_OBJECTS );
            *itor++ = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    size_t ArrayMemoryManager::getNumUsedSlotsIncludingFragmented() const
    {
        return mUsedMemory;
    }
    //-----------------------------------------------------------------------------------
    size_t ArrayMemoryManager::getFreeMemory() const
    {
        return ( mMaxMemory - OGRE_PREFETCH_SLOT_DISTANCE - mUsedMemory + mAvailableSlots.size() )
                * mTotalMemoryMultiplier;
    }
    //-----------------------------------------------------------------------------------
    size_t ArrayMemoryManager::getUsedMemory() const
    {
        return ( mUsedMemory - mAvailableSlots.size() ) * mTotalMemoryMultiplier;
    }
    //-----------------------------------------------------------------------------------
    size_t ArrayMemoryManager::getWastedMemory() const
    {
        return mAvailableSlots.size() * mTotalMemoryMultiplier;
    }
    //-----------------------------------------------------------------------------------
    size_t ArrayMemoryManager::getAllMemory() const
    {
        return mMaxMemory * mTotalMemoryMultiplier;
    }
    //-----------------------------------------------------------------------------------
    size_t ArrayMemoryManager::createNewSlot()
    {
        size_t nextSlot = mUsedMemory;
        ++mUsedMemory;

        //See if we can reuse a slot that was previously acquired and released
        if( !mAvailableSlots.empty() )
        {
            nextSlot = mAvailableSlots.back();
            mAvailableSlots.pop_back();
            --mUsedMemory;
        }

        if( mUsedMemory > mMaxMemory - OGRE_PREFETCH_SLOT_DISTANCE )
        {
            if( mMaxMemory >= mMaxHardLimit )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                            "Trying to allocate more memory than the limit allowed by user",
                            "ArrayMemoryManager::createNewNode" );
            }

            //Build the diff list for rebase later.
            PtrdiffVec diffsList;
            diffsList.reserve( mUsedMemory );
            mRebaseListener->buildDiffList( mLevel, mMemoryPools, diffsList );

            //Reallocate, grow by 50% increments, rounding up to next multiple of ARRAY_PACKED_REALS
            size_t newMemory = std::min( mMaxMemory + (mMaxMemory >> 1), mMaxHardLimit );
            newMemory+= (ARRAY_PACKED_REALS - newMemory % ARRAY_PACKED_REALS) % ARRAY_PACKED_REALS;
            newMemory = std::min( newMemory, mMaxHardLimit );

            size_t i=0;
            MemoryPoolVec::iterator itor = mMemoryPools.begin();
            MemoryPoolVec::iterator end  = mMemoryPools.end();

            while( itor != end )
            {
                //Reallocate
                char *tmp = (char*)OGRE_MALLOC_SIMD( newMemory * mElementsMemSizes[i],
                                                     MEMCATEGORY_SCENE_OBJECTS );
                memcpy( tmp, *itor, mMaxMemory * mElementsMemSizes[i] );
                if( mInitRoutines && mInitRoutines[i] )
                {
                    mInitRoutines[i]( tmp + mMaxMemory * mElementsMemSizes[i], 0, 0, 0, 0,
                                      newMemory - mMaxMemory, mElementsMemSizes[i] );
                }
                else
                {
                    memset( tmp + mMaxMemory * mElementsMemSizes[i], 0,
                            (newMemory - mMaxMemory) * mElementsMemSizes[i] );
                }
                OGRE_FREE_SIMD( *itor, MEMCATEGORY_SCENE_OBJECTS );
                *itor = tmp;
                ++i;
                ++itor;
            }

            const size_t prevNumSlots = mMaxMemory;
            mMaxMemory = newMemory;
            initializeEmptySlots( prevNumSlots );

            //Rebase all ptrs
            mRebaseListener->applyRebase( mLevel, mMemoryPools, diffsList );
        }

        return nextSlot;
    }
    //-----------------------------------------------------------------------------------
    void ArrayMemoryManager::destroySlot( const char *ptrToFirstElement, uint8 index )
    {
        const char *basePtr = ptrToFirstElement;

        const size_t slot = ( basePtr - mMemoryPools[0] ) / mElementsMemSizes[0] + index;

        assert( slot < mMaxMemory && "This slot does not belong to this ArrayMemoryManager" );

        if( slot + 1 == mUsedMemory )
        {
            //Lucky us, LIFO. We're done.
            --mUsedMemory;
        }
        else
        {
            //Not so lucky, add to "reuse" pool
            mAvailableSlots.push_back( slot );

            //The pool is getting to big? Do some cleanup (depending
            //on fragmentation, may take a performance hit)
            if( mAvailableSlots.size() > mCleanupThreshold )
            {
                //Sort, last values first. This may improve performance in some
                //scenarios by reducing the amount of data to be shifted
                std::sort( mAvailableSlots.begin(), mAvailableSlots.end(), std::greater<size_t>() );
                SlotsVec::const_iterator itor = mAvailableSlots.begin();
                SlotsVec::const_iterator end  = mAvailableSlots.end();

                while( itor != end )
                {
                    //First see if we have a continuous range of unused slots
                    size_t lastRange = 1;
                    SlotsVec::const_iterator it = itor + 1;
                    while( it != end && (*itor - lastRange) == *it )
                    {
                        ++lastRange;
                        ++it;
                    }

                    size_t i=0;
                    const size_t newEnd = *itor + 1;
                    MemoryPoolVec::iterator itPools = mMemoryPools.begin();
                    MemoryPoolVec::iterator enPools = mMemoryPools.end();

                    //Shift everything N slots (N = lastRange)
                    while( itPools != enPools )
                    {
                        char *dstPtr    = *itPools + ( newEnd - lastRange ) * mElementsMemSizes[i];
                        size_t indexDst = ( newEnd - lastRange ) % ARRAY_PACKED_REALS;
                        char *srcPtr    = *itPools + newEnd * mElementsMemSizes[i];
                        size_t indexSrc = newEnd % ARRAY_PACKED_REALS;
                        size_t numSlots = ( mUsedMemory - newEnd );
                        size_t numFreeSlots = lastRange;
                        mCleanupRoutines[i]( dstPtr, indexDst, srcPtr, indexSrc,
                                             numSlots, numFreeSlots, mElementsMemSizes[i] );
                        ++i;
                        ++itPools;
                    }

                    mUsedMemory -= lastRange;
                    initializeEmptySlots( mUsedMemory );

                    mRebaseListener->performCleanup( mLevel, mMemoryPools,
                                                     mElementsMemSizes, (newEnd - lastRange),
                                                     lastRange );
                    
                    itor += lastRange;
                }

                mAvailableSlots.clear();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void cleanerFlat( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                        size_t numSlots, size_t numFreeSlots, size_t elementsMemSize )
    {
        memmove( dstPtr, srcPtr, numSlots * elementsMemSize );

        //We need to default-initialize the garbage left after.
        memset( dstPtr + numSlots * elementsMemSize, 0, numFreeSlots * elementsMemSize );
    }
    //-----------------------------------------------------------------------------------
    inline
    void cleanerArrayVector3( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                              size_t numSlots, size_t numFreeSlots, size_t elementsMemSize,
                              Vector3 defaultVal, const ArrayVector3 &_defaultValArray )
    {
        ArrayVector3 *dst = reinterpret_cast<ArrayVector3*>( dstPtr - indexDst * elementsMemSize );
        ArrayVector3 *src = reinterpret_cast<ArrayVector3*>( srcPtr - indexSrc * elementsMemSize );

        for( size_t i=0; i<numSlots; ++i )
        {
            Vector3 tmp;
            src->getAsVector3( tmp, indexSrc );
            dst->setFromVector3( tmp, indexDst );

            ++indexDst;
            ++indexSrc;

            if( indexDst >= ARRAY_PACKED_REALS )
            {
                ++dst;
                indexDst = 0;
            }
            if( indexSrc >= ARRAY_PACKED_REALS )
            {
                ++src;
                indexSrc = 0;
            }
        }

        //We need to default-initialize the garbage left after.
        size_t scalarRemainder = std::min( ARRAY_PACKED_REALS - indexDst, numFreeSlots );
        for( size_t i=0; i<scalarRemainder; ++i )
        {
            dst->setFromVector3( defaultVal, indexDst );
            ++indexDst;
        }

        ++dst;

        ArrayVector3 defaultValArray = _defaultValArray;

        //Keep default initializing, but now on bulk (faster)
        size_t remainder = numFreeSlots - scalarRemainder;
        for( size_t i=0; i<remainder; i += ARRAY_PACKED_REALS )
            *dst++ = defaultValArray;
    }
    //-----------------------------------------------------------------------------------
    void cleanerArrayVector3Zero( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                  size_t numSlots, size_t numFreeSlots, size_t elementsMemSize )
    {
        cleanerArrayVector3( dstPtr, indexDst, srcPtr, indexSrc,
                             numSlots, numFreeSlots, elementsMemSize,
                             Vector3::ZERO, ArrayVector3::ZERO );
    }
    //-----------------------------------------------------------------------------------
    void cleanerArrayVector3Unit( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                  size_t numSlots, size_t numFreeSlots, size_t elementsMemSize )
    {
        cleanerArrayVector3( dstPtr, indexDst, srcPtr, indexSrc,
                             numSlots, numFreeSlots, elementsMemSize,
                             Vector3::UNIT_SCALE, ArrayVector3::UNIT_SCALE );
    }
    //-----------------------------------------------------------------------------------
    void cleanerArrayQuaternion( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                                 size_t numSlots, size_t numFreeSlots, size_t elementsMemSize )
    {
        ArrayQuaternion *dst = reinterpret_cast<ArrayQuaternion*>( dstPtr - indexDst * elementsMemSize );
        ArrayQuaternion *src = reinterpret_cast<ArrayQuaternion*>( srcPtr - indexSrc * elementsMemSize );

        for( size_t i=0; i<numSlots; ++i )
        {
            Quaternion tmp;
            src->getAsQuaternion( tmp, indexSrc );
            dst->setFromQuaternion( tmp, indexDst );

            ++indexDst;
            ++indexSrc;

            if( indexDst >= ARRAY_PACKED_REALS )
            {
                ++dst;
                indexDst = 0;
            }
            if( indexSrc >= ARRAY_PACKED_REALS )
            {
                ++src;
                indexSrc = 0;
            }
        }

        //We need to default-initialize the garbage left after.
        size_t scalarRemainder = std::min( ARRAY_PACKED_REALS - indexDst, numFreeSlots );
        for( size_t i=0; i<scalarRemainder; ++i )
        {
            dst->setFromQuaternion( Quaternion::IDENTITY, indexDst );
            ++indexDst;
        }

        ++dst;

        //Keep default initializing, but now on bulk (faster)
        size_t remainder = numFreeSlots - scalarRemainder;
        for( size_t i=0; i<remainder; i += ARRAY_PACKED_REALS )
            *dst++ = ArrayQuaternion::IDENTITY;
    }
    //-----------------------------------------------------------------------------------
    void cleanerArrayAabb( char *dstPtr, size_t indexDst, char *srcPtr, size_t indexSrc,
                            size_t numSlots, size_t numFreeSlots, size_t elementsMemSize )
    {
        ArrayAabb *dst = reinterpret_cast<ArrayAabb*>( dstPtr - indexDst * elementsMemSize );
        ArrayAabb *src = reinterpret_cast<ArrayAabb*>( srcPtr - indexSrc * elementsMemSize );

        for( size_t i=0; i<numSlots; ++i )
        {
            Aabb tmp;
            src->getAsAabb( tmp, indexSrc );
            dst->setFromAabb( tmp, indexDst );

            ++indexDst;
            ++indexSrc;

            if( indexDst >= ARRAY_PACKED_REALS )
            {
                ++dst;
                indexDst = 0;
            }
            if( indexSrc >= ARRAY_PACKED_REALS )
            {
                ++src;
                indexSrc = 0;
            }
        }

        //We need to default-initialize the garbage left after.
        size_t scalarRemainder = std::min( ARRAY_PACKED_REALS - indexDst, numFreeSlots );
        for( size_t i=0; i<scalarRemainder; ++i )
        {
            dst->setFromAabb( Aabb::BOX_ZERO, indexDst );
            ++indexDst;
        }

        ++dst;

        //Keep default initializing, but now on bulk (faster)
        size_t remainder = numFreeSlots - scalarRemainder;
        for( size_t i=0; i<remainder; i += ARRAY_PACKED_REALS )
            *dst++ = ArrayAabb::BOX_ZERO;
    }
}
