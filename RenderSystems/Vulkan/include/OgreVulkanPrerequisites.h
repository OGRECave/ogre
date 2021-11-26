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
#include "OgrePixelFormat.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#include <volk.h>


namespace Ogre
{
    // Forward declarations
    class VulkanDescriptorPool;
    struct VulkanDevice;
    class VulkanGpuProgramManager;
    class VulkanProgram;
    class VulkanProgramFactory;
    class VulkanQueue;
    class VulkanRenderSystem;
    class VulkanTextureGpu;
    class VulkanTextureGpuManager;
    class VulkanWindow;

    template <typename T>
    using FastArray = std::vector<T>;

    /// Aligns the input 'offset' to the next multiple of 'alignment'.
    /// Alignment can be any value except 0. Some examples:
    ///
    /// alignToNextMultiple( 0, 4 ) = 0;
    /// alignToNextMultiple( 1, 4 ) = 4;
    /// alignToNextMultiple( 2, 4 ) = 4;
    /// alignToNextMultiple( 3, 4 ) = 4;
    /// alignToNextMultiple( 4, 4 ) = 4;
    /// alignToNextMultiple( 5, 4 ) = 8;
    ///
    /// alignToNextMultiple( 0, 3 ) = 0;
    /// alignToNextMultiple( 1, 3 ) = 3;
    inline size_t alignToNextMultiple( size_t offset, size_t alignment )
    {
        return ( (offset + alignment - 1u) / alignment ) * alignment;
    }

    class VulkanHardwareBuffer;

    typedef Texture TextureGpu;
    typedef PixelFormat PixelFormatGpu;

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

#define OGRE_ASSERT_HIGH(x) OgreAssert((x), "high")
#define OGRE_ASSERT_MEDIUM(x) OgreAssert((x), "medium")
#define OGRE_ASSERT_LOW(x) OgreAssert((x), "low")

#define OGRE_VK_CHECK(vkcall) \
{ \
    VkResult result = vkcall; \
    if (result != VK_SUCCESS) \
    { \
        String vkfunc = #vkcall; \
        vkfunc = vkfunc.substr(0, vkfunc.find('(')); \
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, vkfunc + " failed with " + vkResultToString(result)); \
    } \
}

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
#elif defined( RenderSystem_Vulkan_EXPORTS )
#    define _OgreVulkanExport __attribute__( ( visibility( "default" ) ) )
#else
#    define _OgreVulkanExport
#endif

#endif  //#ifndef _OgreVulkanPrerequisites_H_
