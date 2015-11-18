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

#ifndef __GL3PlusNullTexture_H__
#define __GL3PlusNullTexture_H__

#include "OgreGL3PlusTexture.h"

namespace Ogre
{
    class _OgreGL3PlusExport GL3PlusNullTexture : public GL3PlusTexture
    {
    public:
        // Constructor
        GL3PlusNullTexture( ResourceManager* creator, const String& name,
                             ResourceHandle handle, const String& group, bool isManual,
                             ManualResourceLoader* loader, GL3PlusSupport& support );

        virtual ~GL3PlusNullTexture();

    protected:

        /// @copydoc Texture::createInternalResourcesImpl
        virtual void createInternalResourcesImpl(void);
        /// @copydoc Resource::freeInternalResourcesImpl
        virtual void freeInternalResourcesImpl(void);

        virtual void _autogenerateMipmaps(void) {}

        /// @copydoc Resource::prepareImpl
        virtual void prepareImpl(void);
        /// @copydoc Resource::unprepareImpl
        virtual void unprepareImpl(void);
        /// @copydoc Resource::loadImpl
        virtual void loadImpl(void);

        /// internal method, create GL3PlusHardwarePixelBuffers for every face and mipmap level.
        void _createSurfaceList();
    };

namespace v1
{
    class _OgreGL3PlusExport GL3PlusNullPixelBuffer : public HardwarePixelBuffer
    {
    protected:
        RenderTexture   *mDummyRenderTexture;

        virtual PixelBox lockImpl( const Image::Box &lockBox, LockOptions options );
        virtual void unlockImpl(void);

        /// Notify HardwarePixelBuffer of destruction of render target.
        virtual void _clearSliceRTT( size_t zoffset );

    public:
        GL3PlusNullPixelBuffer( GL3PlusNullTexture *parentTexture, const String &baseName,
                                 uint32 width, uint32 height, uint32 Null, PixelFormat format );
        virtual ~GL3PlusNullPixelBuffer();

        virtual void blitFromMemory( const PixelBox &src, const Image::Box &dstBox );
        virtual void blitToMemory( const Image::Box &srcBox, const PixelBox &dst );
        virtual RenderTexture* getRenderTarget( size_t slice=0 );
    };
}

    class _OgreGL3PlusExport GL3PlusNullTextureTarget : public RenderTexture //GL3PlusRenderTexture
    {
        GL3PlusNullTexture *mUltimateTextureOwner;

    public:
        GL3PlusNullTextureTarget( GL3PlusNullTexture *ultimateTextureOwner,
                                   const String &name, v1::HardwarePixelBuffer *buffer,
                                   uint32 zoffset );
        virtual ~GL3PlusNullTextureTarget();

        virtual bool requiresTextureFlipping() const { return true; }

        /// @copydoc RenderTarget::getForceDisableColourWrites
        virtual bool getForceDisableColourWrites(void) const    { return true; }

        /// Null buffers never resolve; only colour buffers do. (we need mFsaaResolveDirty to be always
        /// true so that the proper path is taken in GL3PlusTexture::getGLID)
        virtual void setFsaaResolveDirty(void)  {}

        /// Notifies the ultimate texture owner the buffer changed
        virtual bool attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch );

        void getCustomAttribute( const String& name, void* pData );
    };
}

#endif
