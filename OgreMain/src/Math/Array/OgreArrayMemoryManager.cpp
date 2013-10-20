/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreException.h"

namespace Ogre
{
	const size_t ArrayMemoryManager::MAX_MEMORY_SLOTS = (size_t)(-ARRAY_PACKED_REALS) - 1
															- OGRE_PREFETCH_SLOT_DISTANCE;

	ArrayMemoryManager::ArrayMemoryManager( ManagerType managerType, size_t const *elementsMemSize,
											size_t numElementsSize, uint16 depthLevel,
											size_t hintMaxNodes, size_t cleanupThreshold,
											size_t maxHardLimit, RebaseListener *rebaseListener ) :
							mElementsMemSizes( elementsMemSize ),
							mTotalMemoryMultiplier( 0 ),
							mUsedMemory( 0 ),
							mMaxMemory( hintMaxNodes ),
							mMaxHardLimit( maxHardLimit ),
							mCleanupThreshold( cleanupThreshold ),
							mRebaseListener( rebaseListener ),
							mLevel( depthLevel ),
							mManagerType( managerType )
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
		mMaxMemory		= std::max<size_t>( 2, mMaxMemory ) + OGRE_PREFETCH_SLOT_DISTANCE;
		mMaxHardLimit	+= OGRE_PREFETCH_SLOT_DISTANCE;

		// Round up max memory & hard limit to the next multiple of ARRAY_PACKED_REALS
		mMaxMemory    = ( (mMaxMemory + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS ) *
							ARRAY_PACKED_REALS;
		mMaxHardLimit = ( (mMaxHardLimit + ARRAY_PACKED_REALS - 1) / ARRAY_PACKED_REALS ) *
							ARRAY_PACKED_REALS;

		if( !mRebaseListener )
		{
			//If there's no listener to rebase, we can't later grow the memory pool or perform cleanups.
			mMaxHardLimit		= mMaxMemory;
			mCleanupThreshold	= -1;
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
//			*itor = reinterpret_cast<char*>(_aligned_malloc(mMaxMemory * ElementsMemSize[i], 16));
			*itor = (char*)OGRE_MALLOC_SIMD( mMaxMemory * mElementsMemSizes[i],
											 MEMCATEGORY_SCENE_OBJECTS );
			memset( *itor, 0, mMaxMemory * mElementsMemSizes[i] );
			++i;
			++itor;
		}

		slotsRecreated( 0 );
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
			mRebaseListener->buildDiffList( mManagerType, mLevel, mMemoryPools, diffsList );

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
				memset( tmp + mMaxMemory * mElementsMemSizes[i], 0,
						(newMemory - mMaxMemory) * mElementsMemSizes[i] );
				OGRE_FREE_SIMD( *itor, MEMCATEGORY_SCENE_OBJECTS );
				*itor = tmp;
				++i;
				++itor;
			}

			const size_t prevNumSlots = mMaxMemory;
			mMaxMemory = newMemory;

			//Rebase all ptrs
			mRebaseListener->applyRebase( mManagerType, mLevel, mMemoryPools, diffsList );

			slotsRecreated( prevNumSlots );
		}

		return nextSlot;
	}
	//-----------------------------------------------------------------------------------
	void ArrayMemoryManager::destroySlot( const char *ptrToFirstElement, uint8 index )
	{
		const char *basePtr	= ptrToFirstElement;

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
						memcpy( *itPools + ( newEnd - lastRange ) * mElementsMemSizes[i],
								*itPools + newEnd * mElementsMemSizes[i],
								( mUsedMemory - newEnd ) * mElementsMemSizes[i] );

						//We need to default-initialize the garbage left after.
						memset( *itPools + (mUsedMemory - lastRange) * mElementsMemSizes[i], 0,
								lastRange * mElementsMemSizes[i] );

						++i;
						++itPools;
					}

					mUsedMemory -= lastRange;
					slotsRecreated( mUsedMemory );

					mRebaseListener->performCleanup( mManagerType, mLevel, mMemoryPools,
														mElementsMemSizes, (newEnd - lastRange),
														lastRange );
					
					itor += lastRange;
				}

				mAvailableSlots.clear();
			}
		}
	}
}
