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
#include "OgreNULLHardwarePixelBuffer.h"
#include "OgreNULLTexture.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
namespace v1 {
    //-------------------------------------------------------------------------
    NULLHardwarePixelBuffer::NULLHardwarePixelBuffer( uint32 inWidth, uint32 inHeight,
                                                      uint32 inDepth, PixelFormat inFormat,
                                                      bool hwGamma,
                                                      HardwareBuffer::Usage usage ) :
        HardwarePixelBuffer(inWidth, inHeight, inDepth, inFormat, hwGamma, usage, false, false),
        mBuffer(inWidth, inHeight, inDepth, inFormat)
    {
    }
    //-------------------------------------------------------------------------
    NULLHardwarePixelBuffer::~NULLHardwarePixelBuffer()
    {
        uint8 *p = reinterpret_cast<uint8*>( mBuffer.data );
        delete [] p;
        mBuffer.data = 0;
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::allocateBuffer( size_t bytes )
    {
        if( mBuffer.data )
            return; // Already allocated
        mBuffer.data = new uint8[bytes];
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::freeBuffer(void)
    {
        // Free buffer if we're STATIC to save memory
        if( mUsage & HBU_STATIC )
        {
            uint8 *p = reinterpret_cast<uint8*>( mBuffer.data );
            delete [] p;
            mBuffer.data = 0;
        }
    }
    //-------------------------------------------------------------------------
    PixelBox NULLHardwarePixelBuffer::lockImpl(const Image::Box &lockBox,  LockOptions options)
    {
        //Allocate memory for the entire image, as the buffer
        //maynot be freed and be reused in subsequent calls.
        allocateBuffer( PixelUtil::getMemorySize( mWidth, mHeight, mDepth, mFormat ) );

        mBuffer = PixelBox( lockBox.getWidth(), lockBox.getHeight(),
                            lockBox.getDepth(), mFormat, mBuffer.data );
        mCurrentLock = mBuffer;
        mCurrentLock.left    = lockBox.left;
        mCurrentLock.right  += lockBox.left;
        mCurrentLock.top     = lockBox.top;
        mCurrentLock.bottom += lockBox.top;

        if(options != HardwareBuffer::HBL_DISCARD)
        {
            // Download the old contents of the texture
            download( mCurrentLock );
        }
        mCurrentLockOptions = options;
        mLockedBox = lockBox;
        mCurrentLock = mBuffer;

        return mBuffer;
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::unlockImpl(void)
    {
        if (mCurrentLockOptions != HardwareBuffer::HBL_READ_ONLY)
        {
            // From buffer to card, only upload if was locked for writing.
            upload(mCurrentLock, mLockedBox);
        }
        freeBuffer();

        mBuffer = PixelBox( mWidth, mHeight, mDepth, mFormat, mBuffer.data );
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
    {
        if (!mBuffer.contains(dstBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Destination box out of range",
                        "NULLHardwarePixelBuffer::blitFromMemory");
        }

        PixelBox scaled;

        if (src.getWidth() != dstBox.getWidth() ||
            src.getHeight() != dstBox.getHeight() ||
            src.getDepth() != dstBox.getDepth())
        {
            // Scale to destination size.
            // This also does pixel format conversion if needed.
            allocateBuffer( mSizeInBytes );
            scaled = mBuffer.getSubVolume(dstBox);
            Image::scale(src, scaled, Image::FILTER_BILINEAR);
        }
        else
        {
            allocateBuffer( mSizeInBytes );
            // No scaling or conversion needed.
            scaled = src;
        }

        upload(scaled, dstBox);
        freeBuffer();
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
    {
        if (!mBuffer.contains(srcBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "source box out of range",
                        "NULLHardwarePixelBuffer::blitToMemory");
        }

        if (srcBox.left == 0 && srcBox.right == getWidth() &&
             srcBox.top == 0 && srcBox.bottom == getHeight() &&
             srcBox.front == 0 && srcBox.back == getDepth() &&
             dst.getWidth() == getWidth() &&
             dst.getHeight() == getHeight() &&
             dst.getDepth() == getDepth() )
         {
             // The direct case: the user wants the entire texture in a format supported by GL
             // so we don't need an intermediate buffer
             download(dst);
         }
         else
         {
             // Use buffer for intermediate copy
             allocateBuffer( mSizeInBytes );
             // Download entire buffer
             download(mBuffer);
             if(srcBox.getWidth() != dst.getWidth() ||
                srcBox.getHeight() != dst.getHeight() ||
                srcBox.getDepth() != dst.getDepth())
             {
                 // We need scaling
                 Image::scale(mBuffer.getSubVolume(srcBox), dst, Image::FILTER_BILINEAR);
             }
             else
             {
                 // Just copy the bit that we need
                 PixelUtil::bulkPixelConversion(mBuffer.getSubVolume(srcBox), dst);
             }
             freeBuffer();
         }
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::upload( const PixelBox &data, const Image::Box &dest )
    {
    }
    //-------------------------------------------------------------------------
    void NULLHardwarePixelBuffer::download( const PixelBox &data )
    {
    }
    //-------------------------------------------------------------------------
    RenderTexture *NULLHardwarePixelBuffer::getRenderTarget(size_t zoffset)
    {
        return new NULLRenderTexture(this, zoffset);
    }
}
};
