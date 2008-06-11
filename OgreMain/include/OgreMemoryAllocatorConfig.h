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

/** @file

	This file configures Ogre's memory allocators. You can modify this
	file to alter the allocation routines used for Ogre's main objects.

	When customising memory allocation, all you need to do is provide one or
	more custom allocation policy classes. These classes need to implement:

	@code
	// Allocate bytes - file/line/func information should be optional, 
	// will be provided when available but not everywhere (e.g. release mode, STL allocations)
	static inline void* allocateBytes(size_t count, const char* file = 0, int line = 0, const char* func = 0);
	// Free bytes
	static inline void deallocateBytes(void* ptr);
	// Return the max number of bytes available to be allocated in a single allocation
	static inline size_t getMaxAllocationSize();
	@endcode

	Policies are then used as implementations for the wrapper classes and macros 
	which call them. AllocatedObject for example provides the hooks to override
	the new and delete operators for a class and redirect the functionality to the
	policy. STLAllocator is a class which is provided to STL containers in order
	to hook up allocation of the containers members to the allocation policy.
	@par
	In addition to linking allocations to policies, this class also defines
	a number of macros to allow debugging information to be passed along with
	allocations, such as the file and line number they originate from. It's
	important to realise that we do not redefine the 'new' and 'delete' symbols 
	with macros, because that's very difficult to consistently do when other
	libraries are also trying to do the same thing; instead we use dedicated
	'OGRE_' prefixed macros. See OGRE_NEW and related items.

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
		/// General purpose
		MEMCATEGORY_GENERAL = 0,
		/// Geometry held in main memory
		MEMCATEGORY_GEOMETRY = 1, 
		/// Animation data like tracks, bone matrices
		MEMCATEGORY_ANIMATION = 2, 
		/// Nodes, control data
		MEMCATEGORY_SCENE_CONTROL = 3,
		/// Scene object instances
		MEMCATEGORY_SCENE_OBJECTS = 4,
		/// Other resources
		MEMCATEGORY_RESOURCE = 5,
		/// Scripting
		MEMCATEGORY_SCRIPTING = 6,
		/// Rendersystem structures
		MEMCATEGORY_RENDERSYS = 7,

		
		// sentinel value, do not use 
		MEMCATEGORY_COUNT = 8
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
	typedef CategorisedAllocPolicy<MEMCATEGORY_RESOURCE> ResourceAllocPolicy;
	typedef CategorisedAllocPolicy<MEMCATEGORY_SCRIPTING> ScriptingAllocPolicy;
	typedef CategorisedAllocPolicy<MEMCATEGORY_RENDERSYS> RenderSysAllocPolicy;

	// Now define all the base classes for each allocation
	typedef AllocatedObject<GeneralAllocPolicy> GeneralAllocatedObject;
	typedef AllocatedObject<GeometryAllocPolicy> GeometryAllocatedObject;
	typedef AllocatedObject<AnimationAllocPolicy> AnimationAllocatedObject;
	typedef AllocatedObject<SceneCtlAllocPolicy> SceneCtlAllocatedObject;
	typedef AllocatedObject<SceneObjAllocPolicy> SceneObjAllocatedObject;
	typedef AllocatedObject<ResourceAllocPolicy> ResourceAllocatedObject;
	typedef AllocatedObject<ScriptingAllocPolicy> ScriptingAllocatedObject;
	typedef AllocatedObject<RenderSysAllocPolicy> RenderSysAllocatedObject;

	typedef AllocatedObjectTemplated<GeneralAllocPolicy> GeneralAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<GeometryAllocPolicy> GeometryAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<AnimationAllocPolicy> AnimationAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<SceneCtlAllocPolicy> SceneCtlAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<SceneObjAllocPolicy> SceneObjAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<ResourceAllocPolicy> ResourceAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<ScriptingAllocPolicy> ScriptingAllocatedObjectTemplated;
	typedef AllocatedObjectTemplated<RenderSysAllocPolicy> RenderSysAllocatedObjectTemplated;


	// Per-class allocators defined here
	// NOTE: small, non-virtual classes should not subclass an allocator
	// the virtual function table could double their size and make them less efficient
	// use primitive or STL allocators / deallocators for those
	typedef ScriptingAllocatedObject	AbstractNodeAlloc;
	typedef AnimationAllocatedObject	AnimableAlloc;
	typedef AnimationAllocatedObject	AnimationAlloc;
	typedef GeneralAllocatedObject		AnyAlloc;
	typedef GeneralAllocatedObjectTemplated		AnyHolderAlloc;
	typedef GeneralAllocatedObject		ArchiveAlloc;
	typedef GeometryAllocatedObject		BatchedGeometryAlloc;
	typedef RenderSysAllocatedObject	BufferAlloc;
	typedef GeneralAllocatedObject		CodecAlloc;
	typedef ResourceAllocatedObject		CompositorInstAlloc;
	typedef GeneralAllocatedObject		ConfigAlloc;
	typedef GeneralAllocatedObject		ControllerAlloc;
	typedef GeometryAllocatedObject		EdgeDataAlloc;
	typedef SceneObjAllocatedObject		FXAlloc;
	typedef GeneralAllocatedObject		ImageAlloc;
	typedef GeometryAllocatedObject		IndexDataAlloc;
	typedef SceneObjAllocatedObject		MovableAlloc;
	typedef SceneCtlAllocatedObject		NodeAlloc;
	typedef SceneObjAllocatedObject		OverlayAlloc;
	typedef ResourceAllocatedObject		PassAlloc;
	typedef GeometryAllocatedObject		PatchAlloc;
	typedef GeometryAllocatedObject		ProgMeshAlloc;
	typedef SceneCtlAllocatedObject		RenderQueueAlloc;
	typedef RenderSysAllocatedObject	RenderSysAlloc;
	typedef ResourceAllocatedObject		ResourceAlloc;
	typedef SceneCtlAllocatedObject		SceneMgtAlloc;
	typedef ScriptingAllocatedObject    ScriptTranslatorAlloc;
	typedef SceneCtlAllocatedObject		ShadowCameraAlloc;
	typedef SceneCtlAllocatedObject		ShadowRenderableAlloc;
	typedef SceneCtlAllocatedObject		SplineAlloc;
	typedef GeneralAllocatedObject		StreamAlloc;
	typedef SceneObjAllocatedObject		SubEntityAlloc;
	typedef ResourceAllocatedObject		SubMeshAlloc;
	typedef ResourceAllocatedObject		TechniqueAlloc;
	typedef ResourceAllocatedObject		TextureUnitStateAlloc;
	typedef GeometryAllocatedObject		VertexDataAlloc;

	// Containers (by-value only)
	// Will  be of the form:
	// typedef STLAllocator<T, DefaultAllocPolicy, Category> TAlloc;
	// for use in std::vector<T, TAlloc> 
	


}

// define macros 

#if OGRE_DEBUG_MODE

/// Allocate a block of raw memory, and indicate the category of usage
#	define OGRE_MALLOC(bytes, category) ::Ogre::CategorisedAllocPolicy<category>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define OGRE_ALLOC_T(T, count, category) static_cast<T*>(::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T)*count, __FILE__, __LINE__, __FUNCTION__))
/// Free the memory allocated with either OGRE_MALLOC or OGRE_ALLOC_T. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE(ptr, category) ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes(ptr)

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries, and indicate the category of usage
#	define OGRE_MALLOC_SIMD(bytes, category) ::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_ALLOC_T_SIMD(T, count, category) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T)*count, __FILE__, __LINE__, __FUNCTION__))
/// Free the memory allocated with either OGRE_MALLOC_SIMD or OGRE_ALLOC_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_SIMD(ptr, category) ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr)
/// Allocate a block of raw memory aligned to user defined boundaries, and indicate the category of usage
#	define OGRE_MALLOC_ALIGN(bytes, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_ALLOC_T_ALIGN(T, count, category, align) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*count, __FILE__, __LINE__, __FUNCTION__))
/// Free the memory allocated with either OGRE_MALLOC_ALIGN or OGRE_ALLOC_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_ALIGN(ptr, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr)

// new / delete (alignment determined by per-class policy)
// Also hooks up the file/line/function params
// Can only be used with classes that derive from AllocatedObject since customised new/delete needed
#	define OGRE_NEW new (__FILE__, __LINE__, __FUNCTION__)
#	define OGRE_DELETE delete

#else // !OGRE_DEBUG_MODE

/// Allocate a block of raw memory, and indicate the category of usage
#	define OGRE_MALLOC(bytes, category) ::Ogre::CategorisedAllocPolicy<category>::allocateBytes(bytes)
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define OGRE_ALLOC_T(T, count, category) static_cast<T*>(::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T)*count))
/// Free the memory allocated with either OGRE_MALLOC or OGRE_ALLOC_T. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE(ptr, category) ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes(ptr)

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries, and indicate the category of usage
#	define OGRE_MALLOC_SIMD(bytes, category) ::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(bytes)
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_ALLOC_T_SIMD(T, count, category) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T)*count))
/// Free the memory allocated with either OGRE_MALLOC_SIMD or OGRE_ALLOC_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_SIMD(ptr, category) ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr)
/// Allocate a block of raw memory aligned to user defined boundaries, and indicate the category of usage
#	define OGRE_MALLOC_ALIGN(bytes, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(bytes)
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_ALLOC_T_ALIGN(T, count, category, align) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*count))
/// Free the memory allocated with either OGRE_MALLOC_ALIGN or OGRE_ALLOC_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_ALIGN(ptr, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr)

// new / delete (alignment determined by per-class policy)
#	define OGRE_NEW new 
#	define OGRE_DELETE delete

#endif // OGRE_DEBUG_MODE


#endif
