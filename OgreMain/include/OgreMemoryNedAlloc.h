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

#ifndef __MemoryNedAlloc_H__
#define __MemoryNedAlloc_H__

#include "OgreMemoryTracker.h"

// This is an example of using a non-standard allocator with Ogre
// Note, you need nedmalloc available on your build path to support this
// See http://nedprod.com/programs/portable/nedmalloc/index.html

#include <nedmalloc.h>

namespace Ogre
{

	/**	An allocation policy for use with AllocatedObject and 
	STLAllocator. This is the class that actually does the allocation
	and deallocation of physical memory, and is what you will want to 
	provide a custom version of if you wish to change how memory is allocated.
	@par
	This allocation policy uses nedmalloc 
		(http://nedprod.com/programs/portable/nedmalloc/index.html). 
	*/
	class _OgreExport NedAllocPolicy
	{
	public:
		static inline void* allocateBytes(size_t count, 
			const char* file = 0, int line = 0, const char* func = 0)
		{
			void* ptr = nedalloc::nedmalloc(count);
#if OGRE_MEMORY_TRACKER
			// this alloc policy doesn't do pools (yet, ned can do it)
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
			nedalloc::nedfree(ptr);
		}

		/// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return std::numeric_limits<size_t>::max();
		}

	private:
		// No instantiation
		NedAllocPolicy()
		{ }
	};

	/**	An allocation policy for use with AllocatedObject and 
	STLAllocator, which aligns memory at a given boundary (which should be
	a power of 2). This is the class that actually does the allocation
	and deallocation of physical memory, and is what you will want to 
	provide a custom version of if you wish to change how memory is allocated.
	@par
	This allocation policy uses nedmalloc 
		(http://nedprod.com/programs/portable/nedmalloc/index.html). 
	@note
		template parameter Alignment equal to zero means use default
		platform dependent alignment.
	*/
	template <size_t Alignment = 0>
	class NedAlignedAllocPolicy
	{
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment
			[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

		static inline void* allocateBytes(size_t count, 
			const char* file = 0, int line = 0, const char* func = 0)
		{
			// default to platform SIMD alignment if none specified
			void* ptr =  Alignment ? nedalloc::nedmemalign(Alignment, count)
				: nedalloc::nedmemalign(OGRE_SIMD_ALIGNMENT, count);
#if OGRE_MEMORY_TRACKER
			// this alloc policy doesn't do pools (yet, ned can do it)
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
			// this alloc policy doesn't do pools (yet, ned can do it)
			MemoryTracker::get()._recordDealloc(ptr);
#endif
			nedalloc::nedfree(ptr);
		}

		/// Get the maximum size of a single allocation
		static inline size_t getMaxAllocationSize()
		{
			return std::numeric_limits<size_t>::max();
		}
	private:
		// no instantiation allowed
		NedAlignedAllocPolicy()
		{ }
	};

	// you might also want to declare policies based on ned's pooled allocators
	// if you want - that is lefts as an exercise for the user



}// namespace Ogre
#endif // __MemoryNedAlloc_H__
