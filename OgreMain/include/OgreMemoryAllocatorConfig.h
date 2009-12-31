/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __MemoryAllocatorConfig_H__
#define __MemoryAllocatorConfig_H__

#include "OgreMemoryAllocatedObject.h" 

/** \addtogroup Core
*  @{
*/
/** \addtogroup Memory
*  @{
*/
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
	@par
	The base macros you can use are listed below, in order of preference and 
	with their conditions stated:
	<ul>
	<li>OGRE_NEW - use to allocate an object which have custom new/delete operators
		to handle custom allocations, usually this means it's derived from Ogre::AllocatedObject. 
		Free the memory using OGRE_DELETE. You can in fact use the regular new/delete
		for these classes but you won't get any line number debugging if you do.
		The memory category is automatically derived for these classes; for all other
		allocations you have to specify it.
	</li>
	<li>OGRE_NEW_T - use to allocate a single class / struct that does not have custom
		new/delete operators, either because it is non-virtual (Vector3, Quaternion),
		or because it is from an external library (e.g. STL). You must
		deallocate with OGRE_DELETE_T if you expect the destructor to be called. 
		You may free the memory using OGRE_FREE if you are absolutely sure there
		is no destructor to be called. 
		These macros ensure that constructors and destructors are called correctly 
		even though the memory originates externally (via placement new). Also note 
		that you have to specify the type and memory category so that the correct 
		allocator can be derived, when both allocating
		and freeing.
	</li>
	<li>OGRE_NEW_ARRAY_T - as OGRE_NEW_T except with an extra parameter to construct
		multiple instances in contiguous memory. Again constructors and destructors
		are called. Free with OGRE_DELETE_ARRAY_T.
	</li>
	<li>OGRE_ALLOC_T - use to allocate a set of primitive types conveniently with type safety. 
	This <i>can</i> also be used for classes and structs but it is <b>imperative</b> that
	you understand that neither the constructor nor the destructor will be called. 
	Sometimes you want this because it's more efficient just to grab/free a chunk of
	memory without having to iterate over each element constructing / destructing.
	Free the memory with OGRE_FREE. </li>
	<li>OGRE_MALLOC - the most raw form of allocation, just a set of bytes. 
		Use OGRE_FREE to release.</li>
	<li>_SIMD and _ALIGN variants - all of the above have variations which allow
		aligned memory allocations. The _SIMD versions align automatically to the
		SIMD requirements of your platform, the _ALIGN variants allow user-defined
		alignment to be specified. </li>
	</ul>
	Here are some examples:
	@code
	/// AllocatedObject subclass, with custom operator new / delete
	AllocatedClass* obj = OGRE_NEW AllocatedClass();
	OGRE_DELETE obj;
	AllocatedClass* array = OGRE_NEW AllocatedClass[10];
	OGRE_DELETE [] obj;
	/// Non-virtual or external class, constructors / destructors called
	ExternalClass* obj = OGRE_NEW_T(ExternalClass, MEMCATEGORY_GENERAL)(constructorArgs);
	OGRE_DELETE_T(obj, ExternalClass, MEMCATEGORY_GENERAL);
	ExternalClass* obj = OGRE_NEW_ARRAY_T(ExternalClass, 10, MEMCATEGORY_GENERAL);
	OGRE_DELETE_ARRAY_T(obj, NonVirtualClass, 10, MEMCATEGORY_GENERAL);
	/// Primitive types
	long* pLong = OGRE_ALLOC_T(long, 10, MEMCATEGORY_GENERAL);
	OGRE_FREE(pLong, MEMCATEGORY_GENERAL);
	/// Primitive type with constructor (you can mismatch OGRE_NEW_T and OGRE_FREE because no destructor)
	long* pLong = OGRE_NEW_T(long, MEMCATEGORY_GENERAL)(0);
	OGRE_FREE(pLong, MEMCATEGORY_GENERAL);
	/// Raw memory
	void* pVoid = OGRE_MALLOC(1024, MEMCATEGORY_GENERAL);
	OGRE_FREE(pVoid, MEMCATEGORY_GENERAL);
	@endcode
	OGRE_ALLOC_T is also the route to go for allocating real primitive types like 
	int & float. You free the memory using OGRE_FREE, and both variants have SIMD
	and custom alignment variants.
*/
/** @} */
/** @} */

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Memory
	*  @{
	*/

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
	/** @} */
	/** @} */

}

#include "OgreMemoryAllocatedObject.h"
#include "OgreMemorySTLAllocator.h"

#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_NEDPOOLING

#  include "OgreMemoryNedPooling.h"
namespace Ogre
{
	// configure default allocators based on the options above
	// notice how we're not using the memory categories here but still roughing them out
	// in your allocators you might choose to create different policies per category

	// configurable category, for general malloc
	// notice how we ignore the category here, you could specialise
	template <MemoryCategory Cat> class CategorisedAllocPolicy : public NedPoolingPolicy{};
	template <MemoryCategory Cat, size_t align = 0> class CategorisedAlignAllocPolicy : public NedPoolingAlignedPolicy<align>{};
}

#elif OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_NED

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
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_GENERAL> GeneralAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_GEOMETRY> GeometryAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_ANIMATION> AnimationAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_SCENE_CONTROL> SceneCtlAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_SCENE_OBJECTS> SceneObjAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_RESOURCE> ResourceAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_SCRIPTING> ScriptingAllocPolicy;
	typedef CategorisedAllocPolicy<Ogre::MEMCATEGORY_RENDERSYS> RenderSysAllocPolicy;

	// Now define all the base classes for each allocation
	typedef AllocatedObject<GeneralAllocPolicy> GeneralAllocatedObject;
	typedef AllocatedObject<GeometryAllocPolicy> GeometryAllocatedObject;
	typedef AllocatedObject<AnimationAllocPolicy> AnimationAllocatedObject;
	typedef AllocatedObject<SceneCtlAllocPolicy> SceneCtlAllocatedObject;
	typedef AllocatedObject<SceneObjAllocPolicy> SceneObjAllocatedObject;
	typedef AllocatedObject<ResourceAllocPolicy> ResourceAllocatedObject;
	typedef AllocatedObject<ScriptingAllocPolicy> ScriptingAllocatedObject;
	typedef AllocatedObject<RenderSysAllocPolicy> RenderSysAllocatedObject;


	// Per-class allocators defined here
	// NOTE: small, non-virtual classes should not subclass an allocator
	// the virtual function table could double their size and make them less efficient
	// use primitive or STL allocators / deallocators for those
	typedef ScriptingAllocatedObject	AbstractNodeAlloc;
	typedef AnimationAllocatedObject	AnimableAlloc;
	typedef AnimationAllocatedObject	AnimationAlloc;
	typedef GeneralAllocatedObject		ArchiveAlloc;
	typedef GeometryAllocatedObject		BatchedGeometryAlloc;
	typedef RenderSysAllocatedObject	BufferAlloc;
	typedef GeneralAllocatedObject		CodecAlloc;
	typedef ResourceAllocatedObject		CompositorInstAlloc;
	typedef GeneralAllocatedObject		ConfigAlloc;
	typedef GeneralAllocatedObject		ControllerAlloc;
	typedef GeometryAllocatedObject		DebugGeomAlloc;
	typedef GeneralAllocatedObject		DynLibAlloc;
	typedef GeometryAllocatedObject		EdgeDataAlloc;
	typedef GeneralAllocatedObject		FactoryAlloc;
	typedef SceneObjAllocatedObject		FXAlloc;
	typedef GeneralAllocatedObject		ImageAlloc;
	typedef GeometryAllocatedObject		IndexDataAlloc;
	typedef GeneralAllocatedObject		LogAlloc;
	typedef SceneObjAllocatedObject		MovableAlloc;
	typedef SceneCtlAllocatedObject		NodeAlloc;
	typedef SceneObjAllocatedObject		OverlayAlloc;
	typedef RenderSysAllocatedObject	GpuParamsAlloc;
	typedef ResourceAllocatedObject		PassAlloc;
	typedef GeometryAllocatedObject		PatchAlloc;
	typedef GeneralAllocatedObject		PluginAlloc;
	typedef GeneralAllocatedObject		ProfilerAlloc;
	typedef GeometryAllocatedObject		ProgMeshAlloc;
	typedef SceneCtlAllocatedObject		RenderQueueAlloc;
	typedef RenderSysAllocatedObject	RenderSysAlloc;
	typedef GeneralAllocatedObject		RootAlloc;
	typedef ResourceAllocatedObject		ResourceAlloc;
	typedef GeneralAllocatedObject		SerializerAlloc;
	typedef SceneCtlAllocatedObject		SceneMgtAlloc;
	typedef ScriptingAllocatedObject    ScriptCompilerAlloc;
	typedef ScriptingAllocatedObject    ScriptTranslatorAlloc;
	typedef SceneCtlAllocatedObject		ShadowDataAlloc;
	typedef GeneralAllocatedObject		StreamAlloc;
	typedef SceneObjAllocatedObject		SubEntityAlloc;
	typedef ResourceAllocatedObject		SubMeshAlloc;
	typedef ResourceAllocatedObject		TechniqueAlloc;
	typedef GeneralAllocatedObject		TimerAlloc;
	typedef ResourceAllocatedObject		TextureUnitStateAlloc;
	typedef GeneralAllocatedObject		UtilityAlloc;
	typedef GeometryAllocatedObject		VertexDataAlloc;
	typedef RenderSysAllocatedObject	ViewportAlloc;
	typedef SceneCtlAllocatedObject		LodAlloc;

	// Containers (by-value only)
	// Will  be of the form:
	// typedef STLAllocator<T, DefaultAllocPolicy, Category> TAlloc;
	// for use in vector<T, TAlloc>::type 
	


}

// Util functions
namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Memory
	*  @{
	*/

	/** Utility function for constructing an array of objects with placement new,
		without using new[] (which allocates an undocumented amount of extra memory
		and so isn't appropriate for custom allocators).
	*/
	template<typename T>
	T* constructN(T* basePtr, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
		{
			new ((void*)(basePtr+i)) T();
		}
		return basePtr;
	}
	/** @} */
	/** @} */

}
// define macros 

/** \addtogroup Core
*  @{
*/
/** \addtogroup Memory
*  @{
*/

#if OGRE_DEBUG_MODE

/// Allocate a block of raw memory, and indicate the category of usage
#	define OGRE_MALLOC(bytes, category) ::Ogre::CategorisedAllocPolicy<category>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for a primitive type, and indicate the category of usage
#	define OGRE_ALLOC_T(T, count, category) static_cast<T*>(::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__))
/// Free the memory allocated with OGRE_MALLOC or OGRE_ALLOC_T. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE(ptr, category) ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type with constructor parameters
#	define OGRE_NEW_T(T, category) new (::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define OGRE_NEW_ARRAY_T(T, count, category) ::Ogre::constructN(static_cast<T*>(::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__)), count) 
/// Free the memory allocated with OGRE_NEW_T. Category is required to be restated to ensure the matching policy is used
#	define OGRE_DELETE_T(ptr, T, category) if(ptr){(ptr)->~T(); ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes((void*)ptr);}
/// Free the memory allocated with OGRE_NEW_ARRAY_T. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define OGRE_DELETE_ARRAY_T(ptr, T, count, category) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes((void*)ptr);}

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries, and indicate the category of usage
#	define OGRE_MALLOC_SIMD(bytes, category) ::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of raw memory aligned to user defined boundaries, and indicate the category of usage
#	define OGRE_MALLOC_ALIGN(bytes, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for a primitive type aligned to SIMD boundaries, and indicate the category of usage
#	define OGRE_ALLOC_T_SIMD(T, count, category) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__))
/// Allocate a block of memory for a primitive type aligned to user defined boundaries, and indicate the category of usage
#	define OGRE_ALLOC_T_ALIGN(T, count, category, align) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__))
/// Free the memory allocated with either OGRE_MALLOC_SIMD or OGRE_ALLOC_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_SIMD(ptr, category) ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr)
/// Free the memory allocated with either OGRE_MALLOC_ALIGN or OGRE_ALLOC_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_ALIGN(ptr, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr)

/// Allocate space for one primitive type, external type or non-virtual type aligned to SIMD boundaries
#	define OGRE_NEW_T_SIMD(T, category) new (::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_NEW_ARRAY_T_SIMD(T, count, category) ::Ogre::constructN(static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__)), count) 
/// Free the memory allocated with OGRE_NEW_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define OGRE_DELETE_T_SIMD(ptr, T, category) if(ptr){(ptr)->~T(); ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr);}
/// Free the memory allocated with OGRE_NEW_ARRAY_T_SIMD. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define OGRE_DELETE_ARRAY_T_SIMD(ptr, T, count, category) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes(ptr);}
/// Allocate space for one primitive type, external type or non-virtual type aligned to user defined boundaries
#	define OGRE_NEW_T_ALIGN(T, category, align) new (::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_NEW_ARRAY_T_ALIGN(T, count, category, align) ::Ogre::constructN(static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__)), count) 
/// Free the memory allocated with OGRE_NEW_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define OGRE_DELETE_T_ALIGN(ptr, T, category, align) if(ptr){(ptr)->~T(); ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr);}
/// Free the memory allocated with OGRE_NEW_ARRAY_T_ALIGN. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define OGRE_DELETE_ARRAY_T_ALIGN(ptr, T, count, category, align) if(ptr){for (size_t _b = 0; _b < count; ++_b) { (ptr)[_b].~T();} ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes(ptr);}

// new / delete for classes deriving from AllocatedObject (alignment determined by per-class policy)
// Also hooks up the file/line/function params
// Can only be used with classes that derive from AllocatedObject since customised new/delete needed
#	define OGRE_NEW new (__FILE__, __LINE__, __FUNCTION__)
#	define OGRE_DELETE delete


#else // !OGRE_DEBUG_MODE

/// Allocate a block of raw memory, and indicate the category of usage
#	define OGRE_MALLOC(bytes, category) ::Ogre::CategorisedAllocPolicy<category>::allocateBytes(bytes)
/// Allocate a block of memory for a primitive type, and indicate the category of usage
#	define OGRE_ALLOC_T(T, count, category) static_cast<T*>(::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T)*(count)))
/// Free the memory allocated with OGRE_MALLOC or OGRE_ALLOC_T. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE(ptr, category) ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type with constructor parameters
#	define OGRE_NEW_T(T, category) new (::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define OGRE_NEW_ARRAY_T(T, count, category) ::Ogre::constructN(static_cast<T*>(::Ogre::CategorisedAllocPolicy<category>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with OGRE_NEW_T. Category is required to be restated to ensure the matching policy is used
#	define OGRE_DELETE_T(ptr, T, category) if(ptr){(ptr)->~T(); ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes((void*)ptr);}
/// Free the memory allocated with OGRE_NEW_ARRAY_T. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define OGRE_DELETE_ARRAY_T(ptr, T, count, category) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Ogre::CategorisedAllocPolicy<category>::deallocateBytes((void*)ptr);}

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries, and indicate the category of usage
#	define OGRE_MALLOC_SIMD(bytes, category) ::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(bytes)
/// Allocate a block of raw memory aligned to user defined boundaries, and indicate the category of usage
#	define OGRE_MALLOC_ALIGN(bytes, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(bytes)
/// Allocate a block of memory for a primitive type aligned to SIMD boundaries, and indicate the category of usage
#	define OGRE_ALLOC_T_SIMD(T, count, category) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T)*(count)))
/// Allocate a block of memory for a primitive type aligned to user defined boundaries, and indicate the category of usage
#	define OGRE_ALLOC_T_ALIGN(T, count, category, align) static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*(count)))
/// Free the memory allocated with either OGRE_MALLOC_SIMD or OGRE_ALLOC_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_SIMD(ptr, category) ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes((void*)ptr)
/// Free the memory allocated with either OGRE_MALLOC_ALIGN or OGRE_ALLOC_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define OGRE_FREE_ALIGN(ptr, category, align) ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type aligned to SIMD boundaries
#	define OGRE_NEW_T_SIMD(T, category) new (::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_NEW_ARRAY_T_SIMD(T, count, category) ::Ogre::constructN(static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with OGRE_NEW_T_SIMD. Category is required to be restated to ensure the matching policy is used
#	define OGRE_DELETE_T_SIMD(ptr, T, category) if(ptr){(ptr)->~T(); ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes((void*)ptr);}
/// Free the memory allocated with OGRE_NEW_ARRAY_T_SIMD. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define OGRE_DELETE_ARRAY_T_SIMD(ptr, T, count, category) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Ogre::CategorisedAlignAllocPolicy<category>::deallocateBytes((void*)ptr);}
/// Allocate space for one primitive type, external type or non-virtual type aligned to user defined boundaries
#	define OGRE_NEW_T_ALIGN(T, category, align) new (::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define OGRE_NEW_ARRAY_T_ALIGN(T, count, category, align) ::Ogre::constructN(static_cast<T*>(::Ogre::CategorisedAlignAllocPolicy<category, align>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with OGRE_NEW_T_ALIGN. Category is required to be restated to ensure the matching policy is used
#	define OGRE_DELETE_T_ALIGN(ptr, T, category, align) if(ptr){(ptr)->~T(); ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes((void*)ptr);}
/// Free the memory allocated with OGRE_NEW_ARRAY_T_ALIGN. Category is required to be restated to ensure the matching policy is used, count and type to call destructor
#	define OGRE_DELETE_ARRAY_T_ALIGN(ptr, T, count, category, align) if(ptr){for (size_t _b = 0; _b < count; ++_b) { (ptr)[_b].~T();} ::Ogre::CategorisedAlignAllocPolicy<category, align>::deallocateBytes((void*)ptr);}

// new / delete for classes deriving from AllocatedObject (alignment determined by per-class policy)
#	define OGRE_NEW new 
#	define OGRE_DELETE delete

#endif // OGRE_DEBUG_MODE


namespace Ogre
{
	/** Function which invokes OGRE_DELETE on a given pointer. 
	@remarks
		Useful to pass custom deletion policies to external libraries (e. g. boost).
	*/
	template<typename T>
	void deletePtr(T* ptr)
	{
		OGRE_DELETE ptr;
	}
}

/** @} */
/** @} */

#endif
