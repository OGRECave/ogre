/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2008 Torus Knot Software Ltd
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

#ifndef __MemoryStdAlloc_H__
#define __MemoryStdAlloc_H__

#include <memory>
#include <limits>

#include "OgreAlignedAllocator.h"
#include "OgreMemoryTracker.h"

namespace Ogre
{

	/**	A "standard" allocation policy for use with AllocatedObject and 
		STLAllocator. This is the class that actually does the allocation
		and deallocation of physical memory, and is what you will want to 
		provide a custom version of if you wish to change how memory is allocated.
		@par
		This class just delegates to the global malloc/free.
	*/
	class _OgreExport StdAllocPolicy
	{
	public:
		static inline void* allocateBytes(size_t count, 
			const char* file = 0, int line = 0, const char* func = 0)
		{
			void* ptr = malloc(count);
#if OGRE_MEMORY_TRACKER
			// this alloc policy doesn't do pools
			MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#else
			// avoid unused params warning
			count=file+line-func;
#endif
			return ptr;
		}

		static inline void deallocateBytes(void* ptr)
		{
#if OGRE_MEMORY_TRACKER
			MemoryTracker::get()._recordDealloc(ptr);
#endif
			free(ptr);
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

		static inline void* allocateBytes(size_t count, 
			const char* file = 0, int line = 0, const char* func = 0)
		{
			void* ptr = Alignment ? AlignedMemory::allocate(count, Alignment)
				: AlignedMemory::allocate(count);
#if OGRE_MEMORY_TRACKER
			// this alloc policy doesn't do pools
			MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#else
			// avoid unused params warning
			file;line;func;
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



}// namespace Ogre
#endif // __MemoryStdAlloc_H__
