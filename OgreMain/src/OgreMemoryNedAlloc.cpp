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
#include "OgreStableHeaders.h"
#include "OgrePrerequisites.h"
#include "OgreMemoryNedAlloc.h"
#include "OgrePlatformInformation.h"
#include "OgreMemoryTracker.h"

#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_NED

// include ned implementation
// don't abort() on asserts, behave as normal assert()
#define ABORT_ON_ASSERT_FAILURE 0
#include <nedmalloc.c>

namespace Ogre
{

	//---------------------------------------------------------------------
	void* NedAllocImpl::allocBytes(size_t count, 
		const char* file, int line, const char* func)
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
	//---------------------------------------------------------------------
	void NedAllocImpl::deallocBytes(void* ptr)
	{
		// deal with null
		if (!ptr)
			return;
#if OGRE_MEMORY_TRACKER
		MemoryTracker::get()._recordDealloc(ptr);
#endif
		nedalloc::nedfree(ptr);
	}
	//---------------------------------------------------------------------
	void* NedAllocImpl::allocBytesAligned(size_t align, size_t count, 
		const char* file, int line, const char* func)
	{
		// default to platform SIMD alignment if none specified
		void* ptr =  align ? nedalloc::nedmemalign(align, count)
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
	//---------------------------------------------------------------------
	void NedAllocImpl::deallocBytesAligned(size_t align, void* ptr)
	{
		// deal with null
		if (!ptr)
			return;
#if OGRE_MEMORY_TRACKER
		// this alloc policy doesn't do pools (yet, ned can do it)
		MemoryTracker::get()._recordDealloc(ptr);
#endif
		nedalloc::nedfree(ptr);
	}


}


#endif

