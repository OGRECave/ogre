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

#ifndef _OgreMetalDepthTexture_H_
#define _OgreMetalDepthTexture_H_

#include "OgreMetalTexture.h"

namespace Ogre
{
    class _OgreMetalExport MetalDepthTexture : public MetalTexture
    {
    public:
        // Constructor
        MetalDepthTexture( bool shareableDepthBuffer, ResourceManager* creator, const String& name,
                           ResourceHandle handle, const String& group, bool isManual,
                           ManualResourceLoader* loader, MetalDevice *device );

        virtual ~MetalDepthTexture();

        void _setMetalTexture( id<MTLTexture> metalTexture );

        bool getShareableDepthBuffer(void) const        { return mShareableDepthBuffer; }

    protected:
        bool mShareableDepthBuffer;

        /// @copydoc Texture::createInternalResourcesImpl
        virtual void createInternalResourcesImpl(void);
        /// @copydoc Resource::freeInternalResourcesImpl
        virtual void freeInternalResourcesImpl(void);

        /// @copydoc Resource::prepareImpl
        virtual void prepareImpl(void);
        /// @copydoc Resource::unprepareImpl
        virtual void unprepareImpl(void);

        /// internal method, create MetalHardwarePixelBuffers for every face and mipmap level.
        void _createSurfaceList();

        virtual void _autogenerateMipmaps(void) {}
    };

    class _OgreMetalExport MetalDepthPixelBuffer : public HardwarePixelBuffer
    {
    protected:
        RenderTexture   *mDummyRenderTexture;

        virtual PixelBox lockImpl( const Box &lockBox, LockOptions options );
        virtual void unlockImpl(void);

        /// Notify HardwarePixelBuffer of destruction of render target.
        virtual void _clearSliceRTT( size_t zoffset );

    public:
        MetalDepthPixelBuffer( MetalDepthTexture *parentTexture, const String &baseName,
                                 uint32 width, uint32 height, uint32 depth, PixelFormat format );
        virtual ~MetalDepthPixelBuffer();

        virtual void blitFromMemory( const PixelBox &src, const Box &dstBox );
        virtual void blitToMemory( const Box &srcBox, const PixelBox &dst );
        virtual RenderTexture* getRenderTarget( size_t slice=0 );
    };

    class _OgreMetalExport MetalDepthTextureTarget : public RenderTexture //MetalRenderTexture
    {
        MetalDepthTexture *mUltimateTextureOwner;

    public:
        MetalDepthTextureTarget( MetalDepthTexture *ultimateTextureOwner,
                                   const String &name, HardwarePixelBuffer *buffer,
                                   uint32 zoffset );
        virtual ~MetalDepthTextureTarget();

        virtual bool requiresTextureFlipping(void) const        { return false; }

        /// Depth buffers never resolve; only colour buffers do. (we need mFsaaResolveDirty to be always
        /// true so that the proper path is taken in MetalTexture::getGLID)
        virtual void setFsaaResolveDirty(void)  {}

        virtual void setDepthBufferPool( uint16 poolId );

        /// Notifies the ultimate texture owner the buffer changed
        virtual bool attachDepthBuffer( DepthBuffer *depthBuffer );
        virtual void detachDepthBuffer(void);

        virtual void getFormatsForPso( PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
                                       bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const;

        void getCustomAttribute( const String& name, void* pData );
    };
}

#endif
