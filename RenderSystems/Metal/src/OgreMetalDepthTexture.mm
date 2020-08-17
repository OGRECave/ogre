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

#include "OgreMetalDepthTexture.h"
#include "OgreMetalRenderSystem.h"
#include "OgreMetalHardwarePixelBuffer.h"
#include "OgreMetalRenderTexture.h"
#include "OgreMetalDepthBuffer.h"
#include "OgreRoot.h"

#import <Metal/MTLRenderPass.h>

namespace Ogre
{
    MetalDepthTexture::MetalDepthTexture( bool shareableDepthBuffer, ResourceManager* creator,
                                          const String& name, ResourceHandle handle,
                                          const String& group, bool isManual,
                                          ManualResourceLoader* loader, MetalDevice *device )
        : MetalTexture( creator, name, handle, group, isManual, loader, device ),
          mShareableDepthBuffer( shareableDepthBuffer )
    {
        mMipmapsHardwareGenerated = true;
    }
    //-----------------------------------------------------------------------------------
    MetalDepthTexture::~MetalDepthTexture()
    {
        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            freeInternalResources();
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTexture::_setMetalTexture( id<MTLTexture> metalTexture )
    {
        mTexture = metalTexture;
    }
    //-----------------------------------------------------------------------------------
    // Creation / loading methods
    void MetalDepthTexture::createInternalResourcesImpl(void)
    {
        if( mFSAA < 1u )
            mFSAA = 1u;

        _createSurfaceList();

        // Get final internal format.
        mFormat = getBuffer(0,0)->getFormat();

        mSize = calculateSize();
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        mTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTexture::prepareImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTexture::unprepareImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            HardwarePixelBuffer *buf = OGRE_NEW MetalDepthPixelBuffer( this, mName,
                                                                               mWidth, mHeight,
                                                                               mDepth, mFormat );

            mSurfaceList.push_back( HardwarePixelBufferSharedPtr(buf) );
        }
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    MetalDepthPixelBuffer::MetalDepthPixelBuffer( MetalDepthTexture *parentTexture,
                                                      const String &baseName,
                                                      uint32 width, uint32 height,
                                                      uint32 depth, PixelFormat format ) :
        HardwarePixelBuffer( width, height, depth, format,
                             HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false ),
        mDummyRenderTexture( 0 )
    {
        String name = "DepthTexture/" + StringConverter::toString((size_t)this) + "/" + baseName;

        mDummyRenderTexture = OGRE_NEW MetalDepthTextureTarget( parentTexture, name, this, 0 );
        // mDummyRenderTexture->setPreferDepthTexture( true );
        // mDummyRenderTexture->setDesiredDepthBufferFormat( format );

        RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
        renderSystem->setDepthBufferFor( mDummyRenderTexture );

        //TODO: Should we do this?
        Root::getSingleton().getRenderSystem()->attachRenderTarget( *mDummyRenderTexture );
    }
    //-----------------------------------------------------------------------------------
    MetalDepthPixelBuffer::~MetalDepthPixelBuffer()
    {
        if( mDummyRenderTexture )
            Root::getSingleton().getRenderSystem()->destroyRenderTarget( mDummyRenderTexture->getName() );
    }
    //-----------------------------------------------------------------------------------
    PixelBox MetalDepthPixelBuffer::lockImpl( const Box &lockBox, LockOptions options )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalDepthPixelBuffer::lockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthPixelBuffer::unlockImpl(void)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalDepthPixelBuffer::unlockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthPixelBuffer::_clearSliceRTT( size_t zoffset )
    {
        mDummyRenderTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthPixelBuffer::blitFromMemory( const PixelBox &src, const Box &dstBox )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalDepthPixelBuffer::blitFromMemory" );
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthPixelBuffer::blitToMemory( const Box &srcBox, const PixelBox &dst )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalDepthPixelBuffer::blitToMemory" );
    }
    //-----------------------------------------------------------------------------------
    RenderTexture* MetalDepthPixelBuffer::getRenderTarget( size_t slice )
    {
        return mDummyRenderTexture;
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    MetalDepthTextureTarget::MetalDepthTextureTarget( MetalDepthTexture *ultimateTextureOwner,
                                                          const String &name,
                                                          HardwarePixelBuffer *buffer,
                                                          uint32 zoffset ) :
        RenderTexture( buffer, zoffset ),
        mUltimateTextureOwner( ultimateTextureOwner )
    {
        mName = name;
        mWidth      = ultimateTextureOwner->getWidth();
        mHeight     = ultimateTextureOwner->getHeight();
        // mFormat     = ultimateTextureOwner->getFormat();
        mFSAA       = ultimateTextureOwner->getFSAA();
        mFSAAHint   = ultimateTextureOwner->getFSAAHint();
        // mFsaaResolveDirty = true; //Should be permanent true.

        if( !ultimateTextureOwner->getShareableDepthBuffer() )
            mDepthBufferPoolId = DepthBuffer::POOL_MANUAL_USAGE;
    }
    //-----------------------------------------------------------------------------------
    MetalDepthTextureTarget::~MetalDepthTextureTarget()
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTextureTarget::setDepthBufferPool( uint16 poolId )
    {
        const uint16 oldPoolId = mDepthBufferPoolId;

        RenderTexture::setDepthBufferPool( poolId );

        if( oldPoolId != poolId )
        {
            RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
            renderSystem->setDepthBufferFor( this);
        }
    }
    //-----------------------------------------------------------------------------------
    bool MetalDepthTextureTarget::attachDepthBuffer( DepthBuffer *depthBuffer )
    {
        bool retVal = RenderTexture::attachDepthBuffer( depthBuffer);

        if( mDepthBuffer )
        {
            assert( dynamic_cast<MetalDepthBuffer*>(mDepthBuffer) );
            MetalDepthBuffer *metalDepthBuffer = static_cast<MetalDepthBuffer*>(mDepthBuffer);
            mUltimateTextureOwner->_setMetalTexture( metalDepthBuffer->mDepthAttachmentDesc.texture );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTextureTarget::detachDepthBuffer(void)
    {
        RenderTexture::detachDepthBuffer();
        mUltimateTextureOwner->_setMetalTexture( 0 );
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTextureTarget::getFormatsForPso(
            PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
            bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const
    {
        for( size_t i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            outFormats[i] = PF_UNKNOWN;
            outHwGamma[i] = false;
        }
    }
    //-----------------------------------------------------------------------------------
    void MetalDepthTextureTarget::getCustomAttribute( const String& name, void* pData )
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
