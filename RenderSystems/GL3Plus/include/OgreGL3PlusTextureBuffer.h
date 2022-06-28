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

#ifndef __GL3PlusTextureBuffer_H__
#define __GL3PlusTextureBuffer_H__

#include "OgreGL3PlusHardwarePixelBuffer.h"

namespace Ogre {

    /** Texture surface.
     */
    class _OgreGL3PlusExport GL3PlusTextureBuffer: public GLHardwarePixelBufferCommon
    {
        GL3PlusRenderSystem* mRenderSystem;
    public:
        /** Texture constructor */
        GL3PlusTextureBuffer(GL3PlusTexture* parent, GLint face, GLint level, uint32 width,
                             uint32 height, uint32 depth);
        ~GL3PlusTextureBuffer();

        virtual void bindToFramebuffer(uint32 attachment, uint32 zoffset);

        /// Upload a box of pixels to this buffer on the card.
        virtual void upload(const PixelBox &data, const Box &dest);

        /// Download a box of pixels from the card.
        virtual void download(const PixelBox &data);

        /// Hardware implementation of blitFromMemory.
        void blitFromMemory(const PixelBox &src_orig, const Box &dstBox) override;

        /// Copy from framebuffer.
        void copyFromFramebuffer(uint32 zoffset);

        /// @copydoc HardwarePixelBuffer::blit
        void blit(const HardwarePixelBufferSharedPtr &src,
                  const Box &srcBox, const Box &dstBox);

        void blitToMemory(const Box &srcBox, const PixelBox &dst) override;
    protected:
        // Blitting implementation
        void blitFromTexture(GL3PlusTextureBuffer* src, const Box& srcBox, const Box& dstBox);
        void _blitFromMemory(const PixelBox& src, const Box& dst);
        // In case this is a texture level.
        GLenum mTarget;
        // Same as mTarget in case of GL_TEXTURE_xD, but cubemap face
        // for cubemaps.
        GLenum mFaceTarget;
        GLuint mTextureID;
        GLint mLevel;

        void _bindToFramebuffer(GLenum attachment, uint32 zoffset, GLenum which);
    };

}

#endif
