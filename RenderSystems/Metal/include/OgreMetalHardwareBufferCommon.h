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

#ifndef _OgreMetalHardwareBufferCommon_H_
#define _OgreMetalHardwareBufferCommon_H_

#include "OgreMetalPrerequisites.h"
#include "OgreHardwareBuffer.h"

namespace Ogre
{
    /// Common buffer operations for most v1 buffer interfaces used in Metal
    /// This implementation treats:
    ///		Ignores STATIC and DYNAMIC bit in buffers
    ///		Lack of WRITE_ONLY and DISCARDABLE buffer puts it in slowest path.
    ///		Puts WRITE_ONLY in device memory and uses staging buffers to avoid blocking.
    ///			Use WRITE_ONLY when possible.
    ///		When DISCARDABLE bit is set, it uses MetalDiscardBuffer.
    class _OgreMetalExport MetalHardwareBufferCommon : public HardwareBuffer
    {
    private:
        id<MTLBuffer>       mBuffer;
        MetalDevice         *mDevice;
        MetalDiscardBuffer  *mDiscardBuffer;
        StagingBuffer       *mStagingBuffer;
        uint32              mLastFrameUsed;
        uint32              mLastFrameGpuWrote;

        StagingBuffer* createStagingBuffer( size_t sizeBytes, bool forUpload );
    public:
        MetalHardwareBufferCommon(size_t sizeBytes, Usage usage, bool useShadowBuffer, uint16 alignment,
                                  MetalDiscardBufferManager* discardBufferManager, MetalDevice* device);
        virtual ~MetalHardwareBufferCommon();

        void _notifyDeviceStalled(void);

        /** Returns the actual API buffer, but first sets mLastFrameUsed as we
            assume you're calling this function to use the buffer in the GPU.
        @param outOffset
            Out. Guaranteed to be written. Used by HBU_DISCARDABLE buffers which
            need an offset to the internal ring buffer we've allocated.
        @return
            The MTLBuffer in question.
        */
        id<MTLBuffer> getBufferName( size_t &outOffset );
        id<MTLBuffer> getBufferNameForGpuWrite(void);

        void* lockImpl(size_t offset, size_t length, LockOptions options) override;
        void unlockImpl() override;

        void readData( size_t offset, size_t length, void* pDest ) override;

        void writeData( size_t offset, size_t length,
                        const void* pSource, bool discardWholeBuffer = false ) override;

        void writeDataImpl(size_t offset, size_t length, const void* pSource, bool discardWholeBuffer);

        void copyData( HardwareBuffer& srcBuffer, size_t srcOffset,
                       size_t dstOffset, size_t length, bool discardWholeBuffer = false ) override;

        void _updateFromShadow(void) override;
    };
}

#endif
