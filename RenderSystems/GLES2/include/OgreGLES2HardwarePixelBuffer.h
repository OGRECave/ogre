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

#ifndef __GLES2HardwarePixelBuffer_H__
#define __GLES2HardwarePixelBuffer_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreGLHardwarePixelBufferCommon.h"

namespace Ogre {
    class GLES2TextureBuffer: public GLHardwarePixelBufferCommon
            {
        public:
            /** Texture constructor */
            GLES2TextureBuffer(GLES2Texture* parent, GLint face, GLint level, GLint width,
                               GLint height, GLint depth);
            virtual ~GLES2TextureBuffer();

            void bindToFramebuffer(uint32 attachment, uint32 zoffset) override;

            /// Upload a box of pixels to this buffer on the card
            virtual void upload(const PixelBox &data, const Box &dest);

            /// Download a box of pixels from the card
            virtual void download(const PixelBox &data);

            /// Hardware implementation of blitFromMemory
            virtual void blitFromMemory(const PixelBox &src_orig, const Box &dstBox);

            // Copy from framebuffer
            void copyFromFramebuffer(size_t zoffset);

            /// @copydoc HardwarePixelBuffer::blit
            void blit(const HardwarePixelBufferSharedPtr &src, const Box &srcBox, const Box &dstBox);
            void blitToMemory(const Box &srcBox, const PixelBox &dst) override;

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        // Friends.
        protected:
            friend class GLES2Texture;
                
            void updateTextureId(GLuint textureID);
#endif
                
        protected:
            // Blitting implementation
            void blitFromTexture(GLES2TextureBuffer *src, const Box &srcBox, const Box &dstBox);
            void _blitFromMemory(const PixelBox &src, const Box &dst);

            // In case this is a texture level
            GLenum mTarget;
            GLenum mFaceTarget; // same as mTarget in case of GL_TEXTURE_xD, but cubemap face for cubemaps
            GLuint mTextureID;
            GLint mLevel;

            void buildMipmaps(const PixelBox &data);
    };

    class GLES2RenderBuffer: public GLHardwarePixelBufferCommon
    {
        void blitFromMemory(const PixelBox& src, const Box& dstBox) override { OgreAssertDbg(false, "Not supported"); }
        void blitToMemory(const Box& srcBox, const PixelBox& dst) override { OgreAssertDbg(false, "Not supported"); }
        public:
            GLES2RenderBuffer(GLenum format, uint32 width, uint32 height, GLsizei numSamples);
            virtual ~GLES2RenderBuffer();

            void bindToFramebuffer(uint32 attachment, uint32 zoffset) override;

        protected:
            // In case this is a render buffer
            GLuint mRenderbufferID;
            GLsizei mNumSamples;
    };
}

#endif
