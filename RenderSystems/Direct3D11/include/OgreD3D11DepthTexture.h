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
#ifndef _OgreD3D11DepthTexture_H_
#define _OgreD3D11DepthTexture_H_

#include "OgreD3D11Texture.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    class _OgreD3D11Export D3D11DepthTexture : public D3D11Texture
    {
    protected:
        bool mShareableDepthBuffer;

	public:
		/// constructor 
        D3D11DepthTexture( bool shareableDepthBuffer, ResourceManager* creator,
                           const String& name, ResourceHandle handle,
                           const String& group, bool isManual, ManualResourceLoader* loader,
                           D3D11Device & device );
		/// destructor
        virtual ~D3D11DepthTexture();

        void _setD3DShaderResourceView( ID3D11ShaderResourceView *depthTextureView );
        bool getShareableDepthBuffer(void) const        { return mShareableDepthBuffer; }

        virtual void loadImage( const Image &img );

		bool HasAutoMipMapGenerationEnabled() const { return mAutoMipMapGeneration; }

        /// @copydoc Texture::createInternalResources
        virtual void createInternalResources(void);
        /// @copydoc Texture::createInternalResourcesImpl
        virtual void createInternalResourcesImpl(void);
        /// @copydoc Texture::freeInternalResources
        virtual void freeInternalResources(void);
        /// free internal resources
        virtual void freeInternalResourcesImpl(void);

        /// internal method, create D3D11HardwarePixelBuffers for every face and
        /// mipmap level. This method must be called after the D3D texture object was created
        void _createSurfaceList(void);

        virtual void _autogenerateMipmaps(void) {}

        /// @copydoc Resource::prepareImpl
        virtual void prepareImpl(void);
        /// @copydoc Resource::unprepareImpl
        virtual void unprepareImpl(void);
        /// overridden from Resource
        virtual void loadImpl();
        /// overridden from Resource
        virtual void postLoadImpl();
    };

namespace v1
{
    class _OgreD3D11Export D3D11DepthPixelBuffer : public HardwarePixelBuffer
    {
    protected:
        RenderTexture   *mDummyRenderTexture;

        virtual PixelBox lockImpl( const Image::Box &lockBox, LockOptions options );
        virtual void unlockImpl(void);

        /// Notify HardwarePixelBuffer of destruction of render target.
        virtual void _clearSliceRTT( size_t zoffset );

    public:
        D3D11DepthPixelBuffer( D3D11DepthTexture *parentTexture, const String &baseName,
                               uint32 width, uint32 height, uint32 depth, PixelFormat format );
        virtual ~D3D11DepthPixelBuffer();

        virtual void blitFromMemory( const PixelBox &src, const Image::Box &dstBox );
        virtual void blitToMemory( const Image::Box &srcBox, const PixelBox &dst );
        virtual RenderTexture* getRenderTarget( size_t slice=0 );
    };
}

    class _OgreD3D11Export D3D11DepthTextureTarget : public RenderTexture //D3D11RenderTexture
    {
        D3D11DepthTexture *mUltimateTextureOwner;

    public:
        D3D11DepthTextureTarget( D3D11DepthTexture *ultimateTextureOwner,
                                 const String &name, v1::HardwarePixelBuffer *buffer,
                                 uint32 zoffset );
        virtual ~D3D11DepthTextureTarget();

        virtual bool requiresTextureFlipping(void) const        { return false; }

        /// @copydoc RenderTarget::getForceDisableColourWrites
        virtual bool getForceDisableColourWrites(void) const    { return true; }

        /// Depth buffers never resolve; only colour buffers do. (we need mFsaaResolveDirty to be always
        /// true so that the proper path is taken in GL3PlusTexture::getGLID)
        virtual void setFsaaResolveDirty(void)  {}

        virtual void setDepthBufferPool( uint16 poolId );

        /// Notifies the ultimate texture owner the buffer changed
        virtual bool attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch );
        virtual void detachDepthBuffer(void);

        virtual void getFormatsForPso( PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
                                       bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const;

        void getCustomAttribute( const String& name, void* pData );
    };
}

#endif
