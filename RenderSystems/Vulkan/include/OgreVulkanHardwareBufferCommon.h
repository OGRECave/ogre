/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#ifndef _OgreVulkanHardwareBufferCommon_H_
#define _OgreVulkanHardwareBufferCommon_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreHardwareBuffer.h"
#include "Vao/OgreVulkanVaoManager.h"

namespace Ogre
{
namespace v1
{
    class _OgreVulkanExport VulkanHardwareBufferCommon
    {
    private:
        VulkanRawBuffer mBuffer;
        VulkanDevice *mDevice;
        VulkanDiscardBuffer *mDiscardBuffer;
        VaoManager *mVaoManager;
        StagingBuffer *mStagingBuffer;
        uint32 mLastFrameUsed;
        uint32 mLastFrameGpuWrote;

    public:
        VulkanHardwareBufferCommon( size_t sizeBytes, HardwareBuffer::Usage usage, uint16 alignment,
                                    VulkanDiscardBufferManager *discardBufferManager,
                                    VulkanDevice *device );
        virtual ~VulkanHardwareBufferCommon();

        void _notifyDeviceStalled( void );

        /** Returns the actual API buffer, but first sets mLastFrameUsed as we
            assume you're calling this function to use the buffer in the GPU.
        @param outOffset
            Out. Guaranteed to be written. Used by HBU_DISCARDABLE buffers which
            need an offset to the internal ring buffer we've allocated.
        @return
            The MTLBuffer in question.
        */
        VkBuffer getBufferName( size_t &outOffset );
        VkBuffer getBufferNameForGpuWrite( size_t &outOffset );

        /// @see HardwareBuffer.
        void *lockImpl( size_t offset, size_t length, HardwareBuffer::LockOptions options,
                        bool isLocked );
        /// @see HardwareBuffer.
        void unlockImpl( size_t lockStart, size_t lockSize );

        /// @see HardwareBuffer.
        void readData( size_t offset, size_t length, void *pDest );

        /// @see HardwareBuffer.
        void writeData( size_t offset, size_t length, const void *pSource,
                        bool discardWholeBuffer = false );
        /// @see HardwareBuffer.
        void copyData( VulkanHardwareBufferCommon *srcBuffer, size_t srcOffset, size_t dstOffset,
                       size_t length, bool discardWholeBuffer = false );

        size_t getSizeBytes( void ) const { return mBuffer.mSize; }
    };
}
}

#endif
