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
#include "OgreMetalNullTexture.h"
#include "OgreMetalDepthBuffer.h"
#include "OgreRoot.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    MetalNullTexture::MetalNullTexture( ResourceManager* creator, const String& name,
                                        ResourceHandle handle, const String& group,
                                        bool isManual,  ManualResourceLoader* loader,
                                        MetalDevice *device ) :
        MetalTexture( creator, name, handle, group, isManual, loader, device )
    {
    }
    //---------------------------------------------------------------------
    MetalNullTexture::~MetalNullTexture()
    {
        freeInternalResourcesImpl();
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::loadImage( const Image &img )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalNullTexture::loadImage" );
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::loadImpl()
    {
        createInternalResources();
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::freeInternalResources(void)
    {
        freeInternalResourcesImpl();
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        assert( !mTexture );
        assert( !mMsaaTexture );
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::createInternalResources(void)
    {
        createInternalResourcesImpl();
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::createInternalResourcesImpl(void)
    {
        // If mSrcWidth and mSrcHeight are zero, the requested extents have probably been set
        // through setWidth and setHeight, which set mWidth and mHeight. Take those values.
        if(mSrcWidth == 0 || mSrcHeight == 0)
        {
            mSrcWidth = mWidth;
            mSrcHeight = mHeight;
        }

        _createSurfaceList();

        // Get final internal format.
        mFormat = getBuffer(0,0)->getFormat();

        mSize = calculateSize();
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::_createSurfaceList(void)
    {
        // Create new list of surfaces
        mSurfaceList.clear();

        v1::HardwarePixelBuffer *buffer = OGRE_NEW v1::MetalNullPixelBuffer( this, mName,
                                                                             mWidth, mHeight,
                                                                             mDepth, mFormat );

        mSurfaceList.push_back( v1::HardwarePixelBufferSharedPtr(buffer) );
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::prepareImpl( void )
    {
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::unprepareImpl( void )
    {
    }
    //---------------------------------------------------------------------
    void MetalNullTexture::postLoadImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
namespace v1
{
    MetalNullPixelBuffer::MetalNullPixelBuffer( MetalNullTexture *parentTexture,
                                                const String &baseName,
                                                uint32 width, uint32 height,
                                                uint32 Null, PixelFormat format ) :
        HardwarePixelBuffer( width, height, Null, format, false,
                             HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false ),
        mDummyRenderTexture( 0 )
    {
        String name = "NullTexture/" + StringConverter::toString((size_t)this) + "/" + baseName;

        mDummyRenderTexture = OGRE_NEW MetalNullTextureTarget( parentTexture, name, this, 0 );
        mDummyRenderTexture->setDepthBufferPool( DepthBuffer::POOL_NO_DEPTH );

        //TODO: Should we do this?
        Root::getSingleton().getRenderSystem()->attachRenderTarget( *mDummyRenderTexture );
    }
    //-----------------------------------------------------------------------------------
    MetalNullPixelBuffer::~MetalNullPixelBuffer()
    {
        if( mDummyRenderTexture )
            Root::getSingleton().getRenderSystem()->destroyRenderTarget( mDummyRenderTexture->getName() );
    }
    //-----------------------------------------------------------------------------------
    PixelBox MetalNullPixelBuffer::lockImpl( const Box &lockBox, LockOptions options )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalNullPixelBuffer::lockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void MetalNullPixelBuffer::unlockImpl(void)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalNullPixelBuffer::unlockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void MetalNullPixelBuffer::_clearSliceRTT( size_t zoffset )
    {
        mDummyRenderTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalNullPixelBuffer::blitFromMemory( const PixelBox &src, const Box &dstBox )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalNullPixelBuffer::blitFromMemory" );
    }
    //-----------------------------------------------------------------------------------
    void MetalNullPixelBuffer::blitToMemory( const Box &srcBox, const PixelBox &dst )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalNullPixelBuffer::blitToMemory" );
    }
    //-----------------------------------------------------------------------------------
    RenderTexture* MetalNullPixelBuffer::getRenderTarget( size_t slice )
    {
        return mDummyRenderTexture;
    }
}
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    MetalNullTextureTarget::MetalNullTextureTarget( MetalNullTexture *ultimateTextureOwner,
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
    }
    //-----------------------------------------------------------------------------------
    MetalNullTextureTarget::~MetalNullTextureTarget()
    {
    }
    //-----------------------------------------------------------------------------------
    bool MetalNullTextureTarget::attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL,
                     "Null formats don't use a depth buffer. "
                     "Call setDepthBufferPool( DepthBuffer::POOL_NO_DEPTH ) "
                     "on this RTT before rendering!\n"
                     "If you're manually setting the compositor, "
                     "set TextureDefinition::depthBufferId to 0",
                     "MetalNullTextureTarget::MetalNullTextureTarget" );

        return false;
    }
    //-----------------------------------------------------------------------------------
    void MetalNullTextureTarget::getCustomAttribute( const String& name, void* pData )
    {
        if( name == "MetalRenderTargetCommon" )
        {
            *static_cast<MetalRenderTargetCommon**>(pData) = 0;
        }
        else if( name == "mNumMRTs" )
        {
            *static_cast<uint8*>(pData) = 0;
        }
        else if( name == "MetalDevice" )
        {
            *static_cast<MetalDevice**>(pData) = mUltimateTextureOwner->getOwnerDevice();
        }
        else
        {
            RenderTarget::getCustomAttribute( name, pData );
        }
    }
}
