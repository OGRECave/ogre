// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef __TinyHardwarePixelBuffer_H__
#define __TinyHardwarePixelBuffer_H__

#include "OgreHardwarePixelBuffer.h"

namespace Ogre {
    class TinyHardwarePixelBuffer: public HardwarePixelBuffer
    {
        PixelBox mBuffer;
    public:
        /// Should be called by HardwareBufferManager
        TinyHardwarePixelBuffer(const PixelBox& data, Usage usage);

        /// Lock a box
        PixelBox lockImpl(const Box &lockBox,  LockOptions options) override {  return mBuffer.getSubVolume(lockBox); }

        /// Unlock a box
        void unlockImpl(void) override {}

        /// @copydoc HardwarePixelBuffer::blitFromMemory
        void blitFromMemory(const PixelBox &src, const Box &dstBox) override;

        /// @copydoc HardwarePixelBuffer::blitToMemory
        void blitToMemory(const Box &srcBox, const PixelBox &dst) override;
    };
}

#endif
