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
#include "OgreD3D11DepthTexture.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11DepthBuffer.h"

namespace Ogre 
{
    //---------------------------------------------------------------------
    D3D11DepthTexture::D3D11DepthTexture( bool shareableDepthBuffer,
                                          ResourceManager* creator, const String& name,
                                          ResourceHandle handle, const String& group,
                                          bool isManual,  ManualResourceLoader* loader,
                                          D3D11Device &device ) :
        D3D11Texture( creator, name, handle, group, isManual, loader, device ),
        mShareableDepthBuffer( shareableDepthBuffer )
    {
    }
    //---------------------------------------------------------------------
    D3D11DepthTexture::~D3D11DepthTexture()
    {
        freeInternalResourcesImpl();
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::_setD3DShaderResourceView( ID3D11ShaderResourceView *depthTextureView )
    {
        mpShaderResourceView = depthTextureView;
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::loadImage( const Image &img )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "D3D11DepthTexture::loadImage" );
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::loadImpl()
    {
        createInternalResources();
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::freeInternalResources(void)
    {
        freeInternalResourcesImpl();
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();        
        assert( !mpTex );
        mpShaderResourceView = 0;
        assert( !mp1DTex );
        assert( !mp2DTex );
        assert( !mp3DTex );
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::createInternalResources(void)
    {
        createInternalResourcesImpl();
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::createInternalResourcesImpl(void)
    {
        // If mSrcWidth and mSrcHeight are zero, the requested extents have probably been set
        // through setWidth and setHeight, which set mWidth and mHeight. Take those values.
        if(mSrcWidth == 0 || mSrcHeight == 0) {
            mSrcWidth = mWidth;
            mSrcHeight = mHeight;
        }

        _createSurfaceList();

        // Get final internal format.
        mFormat = getBuffer(0,0)->getFormat();

        mSize = calculateSize();
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::_createSurfaceList(void)
    {
        // Create new list of surfaces
        mSurfaceList.clear();

        v1::HardwarePixelBuffer *buffer = OGRE_NEW v1::D3D11DepthPixelBuffer( this, mName,
                                                                              mWidth, mHeight,
                                                                              mDepth, mFormat );

        mSurfaceList.push_back( v1::HardwarePixelBufferSharedPtr(buffer) );
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::prepareImpl( void )
    {
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::unprepareImpl( void )
    {
    }
    //---------------------------------------------------------------------
    void D3D11DepthTexture::postLoadImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
namespace v1
{
    D3D11DepthPixelBuffer::D3D11DepthPixelBuffer( D3D11DepthTexture *parentTexture,
                                                  const String &baseName,
                                                  uint32 width, uint32 height,
                                                  uint32 depth, PixelFormat format ) :
        HardwarePixelBuffer( width, height, depth, format, false,
                             HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false ),
        mDummyRenderTexture( 0 )
    {
        String name = "DepthTexture/" + StringConverter::toString((size_t)this) + "/" + baseName;

        mDummyRenderTexture = OGRE_NEW D3D11DepthTextureTarget( parentTexture, name, this, 0 );
        mDummyRenderTexture->setPreferDepthTexture( true );
        mDummyRenderTexture->setDesiredDepthBufferFormat( format );

        RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
        renderSystem->setDepthBufferFor( mDummyRenderTexture, true );

        //TODO: Should we do this?
        Root::getSingleton().getRenderSystem()->attachRenderTarget( *mDummyRenderTexture );
    }
    //-----------------------------------------------------------------------------------
    D3D11DepthPixelBuffer::~D3D11DepthPixelBuffer()
    {
        if( mDummyRenderTexture )
            Root::getSingleton().getRenderSystem()->destroyRenderTarget( mDummyRenderTexture->getName() );
    }
    //-----------------------------------------------------------------------------------
    PixelBox D3D11DepthPixelBuffer::lockImpl( const Image::Box &lockBox, LockOptions options )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "D3D11DepthPixelBuffer::lockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthPixelBuffer::unlockImpl(void)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "D3D11DepthPixelBuffer::unlockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthPixelBuffer::_clearSliceRTT( size_t zoffset )
    {
        mDummyRenderTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthPixelBuffer::blitFromMemory( const PixelBox &src, const Image::Box &dstBox )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "D3D11DepthPixelBuffer::blitFromMemory" );
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthPixelBuffer::blitToMemory( const Image::Box &srcBox, const PixelBox &dst )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "D3D11DepthPixelBuffer::blitToMemory" );
    }
    //-----------------------------------------------------------------------------------
    RenderTexture* D3D11DepthPixelBuffer::getRenderTarget( size_t slice )
    {
        return mDummyRenderTexture;
    }
}
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    D3D11DepthTextureTarget::D3D11DepthTextureTarget( D3D11DepthTexture *ultimateTextureOwner,
                                                      const String &name,
                                                      v1::HardwarePixelBuffer *buffer,
                                                      uint32 zoffset ) :
        RenderTexture( buffer, zoffset ),
        mUltimateTextureOwner( ultimateTextureOwner )
    {
        mName = name;
        mWidth      = ultimateTextureOwner->getWidth();
        mHeight     = ultimateTextureOwner->getHeight();
        mFormat     = ultimateTextureOwner->getFormat();
        mFSAA       = ultimateTextureOwner->getFSAA();
        mFSAAHint   = ultimateTextureOwner->getFSAAHint();
        mFsaaResolveDirty = true; //Should be permanent true.

        if( !ultimateTextureOwner->getShareableDepthBuffer() )
            mDepthBufferPoolId = DepthBuffer::POOL_NON_SHAREABLE;
    }
    //-----------------------------------------------------------------------------------
    D3D11DepthTextureTarget::~D3D11DepthTextureTarget()
    {
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthTextureTarget::setDepthBufferPool( uint16 poolId )
    {
        const uint16 oldPoolId = mDepthBufferPoolId;

        RenderTexture::setDepthBufferPool( poolId );

        if( oldPoolId != poolId )
        {
            RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
            renderSystem->setDepthBufferFor( this, true );
        }
    }
    //-----------------------------------------------------------------------------------
    bool D3D11DepthTextureTarget::attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch )
    {
        bool retVal = RenderTexture::attachDepthBuffer( depthBuffer, exactFormatMatch );

        if( mDepthBuffer )
        {
            assert( dynamic_cast<D3D11DepthBuffer*>(mDepthBuffer) );
            D3D11DepthBuffer *d3dDepthBuffer = static_cast<D3D11DepthBuffer*>(mDepthBuffer);
            mUltimateTextureOwner->_setD3DShaderResourceView( d3dDepthBuffer->getDepthTextureView() );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthTextureTarget::detachDepthBuffer(void)
    {
        RenderTexture::detachDepthBuffer();
        mUltimateTextureOwner->_setD3DShaderResourceView( 0 );
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthTextureTarget::getFormatsForPso(
            PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
            bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const
    {
        for( size_t i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            outFormats[i] = PF_NULL;
            outHwGamma[i] = false;
        }
    }
    //-----------------------------------------------------------------------------------
    void D3D11DepthTextureTarget::getCustomAttribute( const String& name, void* pData )
    {
        if( name == "ID3D11RenderTargetView" )
        {
            *static_cast<ID3D11RenderTargetView**>(pData) = 0;
        }
        else if( name == "numberOfViews" )
        {
            *static_cast<uint*>(pData) = 0;
        }
    }
}
