/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __GLESHardwarePixelBuffer_H__
#define __GLESHardwarePixelBuffer_H__

#include "OgreGLESPrerequisites.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {
    class _OgrePrivate GLESHardwarePixelBuffer: public HardwarePixelBuffer
    {
        protected:
            /// Lock a box
            PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);

            /// Unlock a box
            void unlockImpl(void);

            // Internal buffer; either on-card or in system memory, freed/allocated on demand
            // depending on buffer usage
            PixelBox mBuffer;
            GLenum mGLInternalFormat; // GL internal format
            LockOptions mCurrentLockOptions;

            // Buffer allocation/freeage
            void allocateBuffer();

            void freeBuffer();

            // Upload a box of pixels to this buffer on the card
            virtual void upload(const PixelBox &data);

            // Download a box of pixels from the card
            virtual void download(const PixelBox &data);

        public:
            /// Should be called by HardwareBufferManager
            GLESHardwarePixelBuffer(size_t mWidth, size_t mHeight, size_t mDepth,
                                  PixelFormat mFormat,
                                  HardwareBuffer::Usage usage);

            void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
            void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);

            ~GLESHardwarePixelBuffer();

            /** Bind surface to frame buffer. Needs FBO extension.
            */
            virtual void bindToFramebuffer(GLenum attachment, size_t zoffset);
            GLenum getGLFormat() { return mGLInternalFormat; }
    };

    /** Texture surface.
    */
    class _OgrePrivate GLESTextureBuffer: public GLESHardwarePixelBuffer
    {
        public:
            /** Texture constructor */
            GLESTextureBuffer(const String &baseName, GLuint id,
                            GLint width, GLint height, GLint format,
                            GLint face,
                            GLint level, Usage usage, bool softwareMipmap, bool writeGamma, uint fsaa);
            ~GLESTextureBuffer();

            RenderTexture* getRenderTarget(size_t);

            // Upload a box of pixels to this buffer on the card
            virtual void upload(const PixelBox &data);

            // Notify TextureBuffer of destruction of render target
            void _clearSliceRTT(size_t zoffset)
            {
                mSliceTRT[zoffset] = 0;
            }

            // Copy from framebuffer
            void copyFromFramebuffer(size_t zoffset);

        protected:
            // In case this is a texture level
            GLuint mTextureID;
            GLint mFace;
            GLint mLevel;
            bool mSoftwareMipmap;

            typedef std::vector<RenderTexture*> SliceTRT;
            SliceTRT mSliceTRT;

            static void buildMipmaps(const PixelBox &data);
    };

     /** Renderbuffer surface.  Needs FBO extension.
     */
    class _OgrePrivate GLESRenderBuffer: public GLESHardwarePixelBuffer
    {
        public:
            GLESRenderBuffer(GLenum format, size_t width, size_t height, GLsizei numSamples);
            ~GLESRenderBuffer();

            virtual void bindToFramebuffer(GLenum attachment, size_t zoffset);

        protected:
            // In case this is a  render buffer
            GLuint mRenderbufferID;
    };
};

#endif
