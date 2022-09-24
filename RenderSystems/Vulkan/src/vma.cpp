#include "OgreVulkanPrerequisites.h"

#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#ifndef _MSVC_LANG
#define _MSVC_LANG 0
#endif

#define VMA_SYSTEM_ALIGNED_MALLOC Ogre::AlignedMemory::allocate
#define VMA_SYSTEM_ALIGNED_FREE Ogre::AlignedMemory::deallocate

// #define VMA_ASSERT(expr) OgreAssert(expr, "")
#include "vk_mem_alloc.h"