/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _OgreNULLHardwarePixelBuffer_H_
#define _OgreNULLHardwarePixelBuffer_H_

#include "OgreNULLPrerequisites.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {
namespace v1 {
    class NULLHardwarePixelBuffer : public HardwarePixelBuffer
    {
    protected:
        virtual PixelBox lockImpl( const Image::Box &lockBox,  LockOptions options );
        virtual void unlockImpl(void);

        // Internal buffer; either on-card or in system memory, freed/allocated on demand
        // depending on buffer usage
        PixelBox mBuffer;
        LockOptions mCurrentLockOptions;

        // Buffer allocation/freeage
        void allocateBuffer( size_t bytes );
        void freeBuffer(void);
        // Upload a box of pixels to this buffer on the card
        virtual void upload(const PixelBox &data, const Image::Box &dest);
        // Download a box of pixels from the card
        virtual void download(const PixelBox &data);

    public:
        NULLHardwarePixelBuffer( uint32 inWidth, uint32 inHeight, uint32 inDepth,
                                 PixelFormat inFormat, bool hwGamma,
                                 HardwareBuffer::Usage usage );
        virtual ~NULLHardwarePixelBuffer();

        virtual void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
        virtual void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);

        /// Get rendertarget for z slice
        virtual RenderTexture *getRenderTarget(size_t zoffset);
    };
}
};
#endif
