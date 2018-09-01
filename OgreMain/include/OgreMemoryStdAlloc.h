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

#ifndef __MemoryStdAlloc_H__
#define __MemoryStdAlloc_H__

#include <memory>
#include <limits>

#include "OgreAlignedAllocator.h"
#include "OgreMemoryTracker.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_STD

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Memory
    *  @{
    */
    /** A "standard" allocation policy for use with AllocatedObject and 
        STLAllocator. This is the class that actually does the allocation
        and deallocation of physical memory, and is what you will want to 
        provide a custom version of if you wish to change how memory is allocated.
        @par
        This class just delegates to the global malloc/free.
    */
    class _OgreExport StdAllocPolicy
    {
    public:
        static inline DECL_MALLOC void* allocateBytes(size_t count, 
#if OGRE_MEMORY_TRACKER
			const char* file = 0, int line = 0, const char* func = 0
#else
			const char*  = 0, int  = 0, const char* = 0
#endif
            )
		{
			void* ptr = new unsigned char[count];
#if OGRE_MEMORY_TRACKER
			// this alloc policy doesn't do pools
			MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#endif
			return ptr;
		}

		static inline void deallocateBytes(void* ptr)
		{
#if OGRE_MEMORY_TRACKER
			MemoryTracker::get()._recordDealloc(ptr);
#endif
			delete[]((unsigned char*)ptr);
		}

		/// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return std::numeric_limits<size_t>::max();
		}
	private:
		// no instantiation
		StdAllocPolicy()
		{ }
	};

	/**	A "standard" allocation policy for use with AllocatedObject and 
		STLAllocator, which aligns memory at a given boundary (which should be
		a power of 2). This is the class that actually does the allocation
		and deallocation of physical memory, and is what you will want to 
		provide a custom version of if you wish to change how memory is allocated.
		@par
		This class just delegates to the global malloc/free, via AlignedMemory.
		@note
		template parameter Alignment equal to zero means use default
		platform dependent alignment.

	*/
	template <size_t Alignment = 0>
	class StdAlignedAllocPolicy
	{
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment
			[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

        static inline DECL_MALLOC void* allocateBytes(size_t count, 
#if OGRE_MEMORY_TRACKER
			const char* file = 0, int line = 0, const char* func = 0
#else
			const char*  = 0, int  = 0, const char* = 0
#endif
            )
		{
			void* ptr = Alignment ? AlignedMemory::allocate(count, Alignment)
				: AlignedMemory::allocate(count);
#if OGRE_MEMORY_TRACKER
			// this alloc policy doesn't do pools
			MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#endif
			return ptr;
		}

		static inline void deallocateBytes(void* ptr)
		{
#if OGRE_MEMORY_TRACKER
			MemoryTracker::get()._recordDealloc(ptr);
#endif
			AlignedMemory::deallocate(ptr);
		}

		/// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return std::numeric_limits<size_t>::max();
		}
	private:
		// No instantiation
		StdAlignedAllocPolicy()
		{ }
	};

#endif
	/** @} */
	/** @} */

}// namespace Ogre


#include "OgreHeaderSuffix.h"

#endif // __MemoryStdAlloc_H__
