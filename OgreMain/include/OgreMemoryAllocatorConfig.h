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
Torus Knot Software Ltd
---------------------------------------------------------------------------
*/

#ifndef __MemoryAllocatorConfig_H__
#define __MemoryAllocatorConfig_H__

#include "OgrePrerequisites.h"

/*
	This file lists hooks up all the allocators. You can modify this
	file to alter the allocation routines used for Ogre's main objects.

	When customising memory allocation, all you need to do is provide one or
	more custom allocation policy classes. These classes need to implement:

	// Allocate bytes - file/line/func information should be optional, 
	// will be provided when available but not everywhere (e.g. release mode, STL allocations)
	static inline void* allocateBytes(size_t count, const char* file = 0, int line = 0, const char* func = 0);
	// Free bytes
	static inline void deallocateBytes(void* ptr);
	// Return the max number of bytes available to be allocated in a single allocation
	static inline size_t getMaxAllocationSize();

	Ogre provides a few examples of alternative allocation schemes, and in each 
	case provides an aligned version of the policies too. You should at the
	least provide both and make sure the default policies are defined, as
	shown below.

*/

namespace Ogre
{
	/** A set of categories that indicate the purpose of a chunk of memory
	being allocated. 
	These categories will be provided at allocation time in order to allow
	the allocation policy to vary its behaviour if it wishes. This allows you
	to use a single policy but still have variant behaviour. The level of 
	control it gives you is at a higher level than assigning different 
	policies to different classes, but is the only control you have over
	general allocations that are primitive types.
	*/
	enum MemoryCategory
	{
		/// Geometry held in main memory
		MEMCATEGORY_GEOMETRY, 
		/// Animation data like tracks, bone matrices
		MEMCATEGORY_ANIMATION, 
		/// Nodes, control data
		MEMCATEGORY_SCENE_CONTROL,
		/// Scene object instances
		MEMCATEGORY_SCENE_OBJECTS,
		/// General purpose
		MEMCATEGORY_GENERAL
	};
}

#include "OgreMemoryAllocatedObject.h"
#include "OgreMemorySTLAllocator.h"

#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_NED

#  include "OgreMemoryNedAlloc.h"
namespace Ogre
{
	// configure default allocators based on the options above
	// notice how we're not using the memory categories here but still roughing them out
	// in your allocators you might choose to create different policies per category

	// configurable category, for general malloc
	// notice how we ignore the category here, you could specialise
	template <MemoryCategory Cat> class CategorisedAllocPolicy : public NedAllocPolicy{};
	template <MemoryCategory Cat, size_t align = 0> class CategorisedAlignAllocPolicy : public NedAlignedAllocPolicy<align>{};
}

#elif OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_STD

#  include "OgreMemoryStdAlloc.h"
namespace Ogre
{
	// configure default allocators based on the options above
	// notice how we're not using the memory categories here but still roughing them out
	// in your allocators you might choose to create different policies per category

	// configurable category, for general malloc
	// notice how we ignore the category here
	template <MemoryCategory Cat> class CategorisedAllocPolicy : public StdAllocPolicy{};
	template <MemoryCategory Cat, size_t align = 0> class CategorisedAlignAllocPolicy : public StdAlignedAllocPolicy<align>{};

	// if you wanted to specialise the allocation per category, here's how it might work:
	// template <> class CategorisedAllocPolicy<MEMCATEGORY_SCENE_OBJECTS> : public YourSceneObjectAllocPolicy{};
	// template <size_t align> class CategorisedAlignAllocPolicy<MEMCATEGORY_SCENE_OBJECTS, align> : public YourSceneObjectAllocPolicy<align>{};
	
	
}

#else
	
// your allocators here?

#endif

namespace Ogre
{
	// Useful shortcuts
	typedef CategorisedAllocPolicy<MEMCATEGORY_GENERAL> GeneralAllocPolicy;
	typedef CategorisedAllocPolicy<MEMCATEGORY_GEOMETRY> GeometryAllocPolicy;
	typedef CategorisedAllocPolicy<MEMCATEGORY_ANIMATION> AnimationAllocPolicy;
	typedef CategorisedAllocPolicy<MEMCATEGORY_SCENE_CONTROL> SceneCtlAllocPolicy;
	typedef CategorisedAllocPolicy<MEMCATEGORY_SCENE_OBJECTS> SceneObjAllocPolicy;

	// Now define all the base classes for each allocation
	typedef AllocatedObject<GeneralAllocPolicy> GeneralAllocatedObject;
	typedef AllocatedObject<GeometryAllocPolicy> GeometryAllocatedObject;
	typedef AllocatedObject<AnimationAllocPolicy> AnimationAllocatedObject;
	typedef AllocatedObject<SceneCtlAllocPolicy> SceneCtlAllocatedObject;
	typedef AllocatedObject<SceneObjAllocPolicy> SceneObjAllocatedObject;

	// Per-class allocators defined here
	typedef SceneObjAllocatedObject EntityAlloc;
	typedef SceneCtlAllocatedObject NodeAlloc;

	// Containers (by-value only)
	// Will  be of the form:
	// typedef STLAllocator<T, DefaultAllocPolicy, Category> TAlloc;
	// for use in std::vector<T, TAlloc> 
	


}

// define macros 

#if OGRE_DEBUG_MODE

#	define OGRE_MALLOC(bytes, category) ::Ogre::CategorisedAllocPolicy<category>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
#	define OGRE_FREE(ptr, category) ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes(ptr)

// aligned allocation
#	define OGRE_MALLOC_ALIGNED_SIMD(bytes, category) ::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
#	define OGRE_FREE_ALIGNED_SIMD(ptr, category) ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr)
#	define OGRE_MALLOC_ALIGNED(bytes, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
#	define OGRE_FREE_ALIGNED(ptr, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr)

// new / delete (alignment determined by per-class policy)
#	define OGRE_NEW new (__FILE__, __LINE__, __FUNCTION__)
#	define OGRE_DELETE delete

#else // !OGRE_DEBUG_MODE

#	define OGRE_MALLOC(bytes, category) ::Ogre::CategorisedAllocPolicy<category>::allocateBytes(bytes)
#	define OGRE_FREE(ptr, category) ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes(ptr)

// aligned allocation
#	define OGRE_MALLOC_ALIGNED_SIMD(bytes, category) ::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(bytes)
#	define OGRE_FREE_ALIGNED_SIMD(ptr, category) ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr)
#	define OGRE_MALLOC_ALIGNED(bytes, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(bytes)
#	define OGRE_FREE_ALIGNED(ptr, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr)

// new / delete (alignment determined by per-class policy)
#	define OGRE_NEW new 
#	define OGRE_DELETE delete

#endif // OGRE_DEBUG_MODE


#endif
