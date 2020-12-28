/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#ifndef _OgreVulkanCache_H_
#define _OgreVulkanCache_H_

#include "OgreVulkanPrerequisites.h"

#include "vulkan/vulkan_core.h"

namespace Ogre
{
    /**
     */
    class _OgreVulkanExport VulkanCache : public RenderSysAlloc
    {
    protected:
        struct VkRenderPassCreateInfoCmp
        {
            enum CmpResult
            {
                CmpResultEqual,
                CmpResultLess,
                CmpResultGreater
            };

            bool operator()( const VkRenderPassCreateInfo &a, const VkRenderPassCreateInfo &b ) const;
            CmpResult cmp( const VkAttachmentDescription &a, const VkAttachmentDescription &b ) const;
            CmpResult cmp( const VkAttachmentReference &a, const VkAttachmentReference &b ) const;
            CmpResult cmp( const VkSubpassDescription &a, const VkSubpassDescription &b ) const;
            CmpResult cmp( const VkSubpassDependency &a, const VkSubpassDependency &b ) const;
        };

        VulkanDevice *mDevice;

        typedef map<VkRenderPassCreateInfo, VkRenderPass, VkRenderPassCreateInfoCmp>::type
            VkRenderPassMap;

        VkRenderPassMap mRenderPassCache;

        static void copySubpass( const VkAttachmentReference **dstStruct,
                                 const VkAttachmentReference *src, uint32_t attachmentCount,
                                 VkAttachmentReference *memoryBuffer, size_t &assignedIdx );

    public:
        VulkanCache( VulkanDevice *device );
        ~VulkanCache();

        VkRenderPass getRenderPass( const VkRenderPassCreateInfo &renderPassCi );
    };
}  // namespace Ogre

#endif
