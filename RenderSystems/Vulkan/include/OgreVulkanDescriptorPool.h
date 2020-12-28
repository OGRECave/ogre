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
#ifndef _OgreVulkanDescriptorPool_H_
#define _OgreVulkanDescriptorPool_H_

#include "OgreVulkanPrerequisites.h"

struct VkDescriptorPoolSize;

namespace Ogre
{
    /**
    @brief The VulkanDescriptorPool class
        A VulkanDescriptorPool manages the pool of a single set (i.e. one VkDescriptorSetLayout)

        It asks for a VulkanRootLayout pointer and the setIdx in order to gather all the data.

        However it is not tied to a single VulkanRootLayout and
        may be shared by multiple VulkanRootLayouts
    */
    class _OgreVulkanExport VulkanDescriptorPool : public RenderSysAlloc
    {
        struct Pool
        {
            VkDescriptorPool pool;
            size_t size;
            size_t capacity;

            Pool( size_t _capacity );
            bool isFull( void ) const { return size == capacity; }
        };

        size_t mCurrentCapacity;
        FastArray<Pool> mPools;
        FastArray<VkDescriptorPoolSize> mPoolSizes;

        size_t mCurrentPoolIdx;
        uint32 mLastFrameUsed;

        bool mAdvanceFrameScheduled;
        VulkanVaoManager *mVaoManager;

        void createNewPool( VulkanDevice *device );
        void createNewPool( VulkanDevice *device, const size_t newCapacity );

    public:
        VulkanDescriptorPool( VulkanVaoManager *vaoManager, const VulkanRootLayout *rootLayout,
                              size_t setIdx, const size_t capacity = 16u );
        ~VulkanDescriptorPool();

        void deinitialize( VulkanDevice *device );

        VkDescriptorSet allocate( VulkanDevice *device, VkDescriptorSetLayout setLayout );
        void reset( VulkanDevice *device );

        size_t getCurrentCapacity( void ) const { return mCurrentCapacity; }

        void _advanceFrame( void );
        bool isAvailableInCurrentFrame( void ) const;
    };
}  // namespace Ogre

#endif
