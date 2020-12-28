/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#ifndef _OgreVulkanStagingTexture_H_
#define _OgreVulkanStagingTexture_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreStagingTextureBufferImpl.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class _OgreVulkanExport VulkanStagingTexture : public StagingTextureBufferImpl
    {
        VkBuffer mVboName;
        VulkanDynamicBuffer *mDynamicBuffer;
        size_t mUnmapTicket;

        void *mMappedPtr;
        void *mLastMappedPtr;

        virtual bool belongsToUs( const TextureBox &box );
        virtual void *RESTRICT_ALIAS_RETURN mapRegionImplRawPtr( void );

    public:
        VulkanStagingTexture( VaoManager *vaoManager, PixelFormatGpu formatFamily, size_t size,
                              size_t internalBufferStart, size_t vboPoolIdx, VkBuffer vboName,
                              VulkanDynamicBuffer *dynamicBuffer );
        virtual ~VulkanStagingTexture();

        void _unmapBuffer( void );

        bool supportsFormat( uint32 width, uint32 height, uint32 depth, uint32 slices,
                             PixelFormatGpu pixelFormat ) const;

        virtual void startMapRegion( void );
        virtual void stopMapRegion( void );

        virtual void upload( const TextureBox &srcBox, TextureGpu *dstTexture, uint8 mipLevel,
                             const TextureBox *cpuSrcBox = 0, const TextureBox *dstBox = 0,
                             bool skipSysRamCopy = false );

        VkBuffer _getVboName( void ) const { return mVboName; }

        VulkanDynamicBuffer *_getDynamicBuffer( void ) { return mDynamicBuffer; }
        void _resetDynamicBuffer( void ) { mDynamicBuffer = 0; }
    };
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif
