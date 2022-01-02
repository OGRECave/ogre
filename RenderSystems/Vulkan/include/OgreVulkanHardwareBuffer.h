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

namespace Ogre
{
    class VulkanHardwareBuffer : public HardwareBuffer
    {
    private:
        VkBuffer mBuffer;
        VkDeviceMemory  mMemory;
        VulkanDevice *mDevice;
        uint32 mTarget;
        void *mMappedPtr;

        void discard();
    public:
        VulkanHardwareBuffer(uint32 target, size_t sizeBytes, Usage usage, bool useShadowBuffer, VulkanDevice* device);
        virtual ~VulkanHardwareBuffer();

        void _notifyDeviceStalled(void) {}

        VkBuffer getVkBuffer() const { return mBuffer; }

        void* lockImpl(size_t offset, size_t length, LockOptions options) override;
        void unlockImpl() override;

        void readData( size_t offset, size_t length, void *pDest ) override;

        void writeData( size_t offset, size_t length, const void *pSource,
                        bool discardWholeBuffer = false ) override;

        void copyData( HardwareBuffer& srcBuffer, size_t srcOffset, size_t dstOffset,
                       size_t length, bool discardWholeBuffer = false ) override;

        void _updateFromShadow() override;
    };
}

#endif
