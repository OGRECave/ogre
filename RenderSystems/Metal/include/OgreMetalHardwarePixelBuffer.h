/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#ifndef _OgreMetalHardwarePixelBuffer_H_
#define _OgreMetalHardwarePixelBuffer_H_

#include "OgreMetalPrerequisites.h"
#include "OgreHardwarePixelBuffer.h"

#import <Metal/MTLTexture.h>

namespace Ogre {
    class _OgreMetalExport MetalHardwarePixelBuffer : public HardwarePixelBuffer
    {
    protected:
        /// Lock a box
        PixelBox lockImpl(const Box &lockBox, LockOptions options) override;

        /// Unlock a box
        void unlockImpl(void) override;

        // Internal buffer; either on-card or in system memory, freed/allocated on demand
        // depending on buffer usage
        PixelBox mBuffer;

        bool mHwGamma;

        // Buffer allocation/freeage
        void allocateBuffer();

        void freeBuffer();

        // Upload a box of pixels to this buffer on the card
        virtual void upload(const PixelBox &data, const Box &dest);

        // Download a box of pixels from the card
        virtual void download(const PixelBox &data);

    public:
        /// Should be called by HardwareBufferManager
        MetalHardwarePixelBuffer( uint32 width, uint32 height, uint32 depth,
                                  PixelFormat format, bool hwGamma,
                                  HardwareBuffer::Usage usage );

        /// @copydoc HardwarePixelBuffer::blitFromMemory
        void blitFromMemory(const PixelBox &src, const Box &dstBox) override;

        /// @copydoc HardwarePixelBuffer::blitToMemory
        void blitToMemory(const Box &srcBox, const PixelBox &dst) override;

        virtual ~MetalHardwarePixelBuffer();

        /** Bind surface to frame buffer. Needs FBO extension.
        */
        virtual void bindToFramebuffer(uint32 attachment, size_t zoffset);
    };

    /// Texture surface.
    class _OgreMetalExport MetalTextureBuffer : public MetalHardwarePixelBuffer
    {
    public:
        /** Texture constructor */
        MetalTextureBuffer( __unsafe_unretained id<MTLTexture> renderTexture,
                            __unsafe_unretained id<MTLTexture> resolveTexture,
                            MetalDevice *device,
                            const String &baseName, MTLTextureType target,
                            int width, int height, int depth, PixelFormat format,
                            int face, int level, Usage usage,
                            bool writeGamma, uint fsaa );
        virtual ~MetalTextureBuffer();

        void bindToFramebuffer(uint32 attachment, size_t zoffset) override;

        /// Upload a box of pixels to this buffer on the card
        void upload(const PixelBox &data, const Box &dest) override;

        /// Download a box of pixels from the card
        void download(const PixelBox &data) override;

        /// Hardware implementation of blitFromMemory
        void blitFromMemory(const PixelBox &src_orig, const Box &dstBox) override;

        /// Lock a box
//            PixelBox lockImpl(const Box &lockBox, LockOptions options) { return PixelBox(); }

        // Copy from framebuffer
        void copyFromFramebuffer(size_t zoffset);

        /// @copydoc HardwarePixelBuffer::blit
        void blit( const HardwarePixelBufferSharedPtr &src,
                   const Box &srcBox, const Box &dstBox ) override;
        // Blitting implementation
        void blitFromTexture( MetalTextureBuffer *src, const Box &srcBox,
                              const Box &dstBox );

    protected:
        __unsafe_unretained id<MTLTexture> mTexture;
        // In case this is a texture level
        MTLTextureType mTarget;
        uint32 mBufferId;
        int mFace;
        int mLevel;
    };
}

#endif
