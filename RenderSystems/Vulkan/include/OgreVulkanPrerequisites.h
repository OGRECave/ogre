/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

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
#ifndef _OgreVulkanPrerequisites_H_
#define _OgreVulkanPrerequisites_H_

#include "OgrePrerequisites.h"

#include "OgreLogManager.h"

#ifdef __MINGW32__
#    ifndef UINT64_MAX
#        define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */
#    endif
#endif

#if defined( __LP64__ ) || defined( _WIN64 ) || ( defined( __x86_64__ ) && !defined( __ILP32__ ) ) || \
    defined( _M_X64 ) || defined( __ia64 ) || defined( _M_IA64 ) || defined( __aarch64__ ) || \
    defined( __powerpc64__ )
#    define OGRE_VK_NON_DISPATCHABLE_HANDLE( object ) typedef struct object##_T *object;
#else
#    define OGRE_VK_NON_DISPATCHABLE_HANDLE( object ) typedef uint64_t object;
#endif

typedef struct VkInstance_T *VkInstance;
typedef struct VkPhysicalDevice_T *VkPhysicalDevice;
typedef struct VkDevice_T *VkDevice;
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkDeviceMemory )
typedef struct VkCommandBuffer_T *VkCommandBuffer;

OGRE_VK_NON_DISPATCHABLE_HANDLE( VkBuffer )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkBufferView )

OGRE_VK_NON_DISPATCHABLE_HANDLE( VkSurfaceKHR )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkSwapchainKHR )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkImage )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkSemaphore )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkFence )

OGRE_VK_NON_DISPATCHABLE_HANDLE( VkRenderPass )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkFramebuffer )

OGRE_VK_NON_DISPATCHABLE_HANDLE( VkShaderModule )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkDescriptorSetLayout )

OGRE_VK_NON_DISPATCHABLE_HANDLE( VkDescriptorPool )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkDescriptorSet )

OGRE_VK_NON_DISPATCHABLE_HANDLE( VkPipelineLayout )
OGRE_VK_NON_DISPATCHABLE_HANDLE( VkPipeline )

#undef OGRE_VK_NON_DISPATCHABLE_HANDLE

struct VkPipelineShaderStageCreateInfo;

#define OGRE_VULKAN_CONST_SLOT_START 16u
#define OGRE_VULKAN_TEX_SLOT_START 24u
#define OGRE_VULKAN_PARAMETER_SLOT 23u
#define OGRE_VULKAN_UAV_SLOT_START 28u

#define OGRE_VULKAN_CS_PARAMETER_SLOT 7u
#define OGRE_VULKAN_CS_CONST_SLOT_START 0u
#define OGRE_VULKAN_CS_UAV_SLOT_START 8u
#define OGRE_VULKAN_CS_TEX_SLOT_START 16u

namespace Ogre
{
    // Forward declarations
    class VulkanBufferInterface;
    class VulkanCache;
    class VulkanDescriptorPool;
    struct VulkanDevice;
    class VulkanDynamicBuffer;
    struct VulkanGlobalBindingTable;
    class VulkanGpuProgramManager;
    class VulkanProgram;
    class VulkanProgramFactory;
    class VulkanQueue;
    class VulkanRenderSystem;
    class VulkanRootLayout;
    class VulkanStagingBuffer;
    class VulkanTextureGpu;
    class VulkanTextureGpuManager;
    class VulkanVaoManager;
    class VulkanWindow;
    class VulkanDiscardBuffer;
    class VulkanDiscardBufferManager;

    typedef FastArray<VkSemaphore> VkSemaphoreArray;
    typedef FastArray<VkFence> VkFenceArray;

    namespace v1
    {
        class VulkanHardwareBufferCommon;
        class VulkanHardwareIndexBuffer;
        class VulkanHardwareVertexBuffer;
    }  // namespace v1

    namespace SubmissionType
    {
        enum SubmissionType
        {
            /// Send the work we have so far to the GPU, but there may be more under way
            ///
            /// A fence won't be generated automatically to protect this work, but
            /// there may be one if getCurrentFence or acquireCurrentFence
            /// was called
            FlushOnly,
            /// Same as FlushOnly + fences the data so that
            /// VaoManager::mDynamicBufferCurrentFrame can be incremented and
            /// waitForTailFrameToFinish works without stalling.
            ///
            /// It's like forcing the end of a frame without swapping windows, e.g.
            /// when waiting for textures to finish streaming.
            NewFrameIdx,
            /// Same as NewFrameIdx + swaps windows (presents).
            /// This is the real end of the frame.
            EndFrameAndSwap
        };
    }

}  // namespace Ogre

#define OGRE_VK_EXCEPT( code, num, desc, src ) \
    OGRE_EXCEPT( code, desc + ( "\nVkResult = " + vkResultToString( num ) ), src )

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#    define checkVkResult( result, functionName ) \
        do \
        { \
            if( result != VK_SUCCESS ) \
            { \
                OGRE_VK_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, result, functionName " failed", \
                                __FUNCSIG__ ); \
            } \
        } while( 0 )
#else
#    define checkVkResult( result, functionName ) \
        do \
        { \
            if( result != VK_SUCCESS ) \
            { \
                OGRE_VK_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, result, functionName " failed", \
                                __PRETTY_FUNCTION__ ); \
            } \
        } while( 0 )
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    if !defined( __MINGW32__ )
#        define WIN32_LEAN_AND_MEAN
#        ifndef NOMINMAX
#            define NOMINMAX  // required to stop windows.h messing up std::min
#        endif
#    endif
#endif

// Lots of generated code in here which triggers the new VC CRT security warnings
#if !defined( _CRT_SECURE_NO_DEPRECATE )
#    define _CRT_SECURE_NO_DEPRECATE
#endif

#if( OGRE_PLATFORM == OGRE_PLATFORM_WIN32 ) && !defined( __MINGW32__ ) && !defined( OGRE_STATIC_LIB )
#    ifdef RenderSystem_Vulkan_EXPORTS
#        define _OgreVulkanExport __declspec( dllexport )
#    else
#        if defined( __MINGW32__ )
#            define _OgreVulkanExport
#        else
#            define _OgreVulkanExport __declspec( dllimport )
#        endif
#    endif
#elif defined( OGRE_GCC_VISIBILITY )
#    define _OgreVulkanExport __attribute__( ( visibility( "default" ) ) )
#else
#    define _OgreVulkanExport
#endif

#endif  //#ifndef _OgreVulkanPrerequisites_H_
