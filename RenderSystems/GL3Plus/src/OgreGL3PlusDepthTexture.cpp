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

#include "OgreGL3PlusDepthTexture.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusRenderTexture.h"
#include "OgreGL3PlusFrameBufferObject.h"
#include "OgreGL3PlusDepthBuffer.h"
#include "OgreRoot.h"

namespace Ogre
{
    GL3PlusDepthTexture::GL3PlusDepthTexture( bool shareableDepthBuffer, ResourceManager* creator,
                                              const String& name, ResourceHandle handle,
                                              const String& group, bool isManual,
                                              ManualResourceLoader* loader, GL3PlusSupport& support )
        : GL3PlusTexture( creator, name, handle, group, isManual, loader, support ),
          mShareableDepthBuffer( shareableDepthBuffer )
    {
        mMipmapsHardwareGenerated = true;
    }
    //-----------------------------------------------------------------------------------
    GL3PlusDepthTexture::~GL3PlusDepthTexture()
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
    void GL3PlusDepthTexture::_setGlTextureName( GLuint textureName )
    {
        mTextureID = textureName;
    }
    //-----------------------------------------------------------------------------------
    // Creation / loading methods
    void GL3PlusDepthTexture::createInternalResourcesImpl(void)
    {
        _createSurfaceList();

        // Get final internal format.
        mFormat = getBuffer(0,0)->getFormat();

        mSize = calculateSize();
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
        mTextureID = 0;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTexture::prepareImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTexture::unprepareImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTexture::loadImpl()
    {
        createRenderTexture();
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            v1::HardwarePixelBuffer *buf = OGRE_NEW v1::GL3PlusDepthPixelBuffer( this, mName,
                                                                                 mWidth, mHeight,
                                                                                 mDepth, mFormat );

            mSurfaceList.push_back( v1::HardwarePixelBufferSharedPtr(buf) );

            // Check for error
            if (buf->getWidth() == 0 ||
                buf->getHeight() == 0 ||
                buf->getDepth() == 0)
            {
                OGRE_EXCEPT(
                    Exception::ERR_RENDERINGAPI_ERROR,
                    "Zero sized texture surface on texture "+getName()+
                    " face "+StringConverter::toString(face)+
                    " mipmap 0"
                    ". The GL driver probably refused to create the texture.",
                    "GL3PlusDepthTexture::_createSurfaceList");
            }
        }
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
namespace v1
{
    GL3PlusDepthPixelBuffer::GL3PlusDepthPixelBuffer( GL3PlusDepthTexture *parentTexture,
                                                      const String &baseName,
                                                      uint32 width, uint32 height,
                                                      uint32 depth, PixelFormat format ) :
        HardwarePixelBuffer( width, height, depth, format, false,
                             HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false ),
        mDummyRenderTexture( 0 )
    {
        String name = "DepthTexture/" + StringConverter::toString((size_t)this) + "/" + baseName;

        mDummyRenderTexture = OGRE_NEW GL3PlusDepthTextureTarget( parentTexture, name, this, 0 );
        mDummyRenderTexture->setPreferDepthTexture( true );
        mDummyRenderTexture->setDesiredDepthBufferFormat( format );

        RenderSystem *renderSystem = Root::getSingleton().getRenderSystem();
        renderSystem->setDepthBufferFor( mDummyRenderTexture, true );

        //TODO: Should we do this?
        Root::getSingleton().getRenderSystem()->attachRenderTarget( *mDummyRenderTexture );
    }
    //-----------------------------------------------------------------------------------
    GL3PlusDepthPixelBuffer::~GL3PlusDepthPixelBuffer()
    {
        if( mDummyRenderTexture )
            Root::getSingleton().getRenderSystem()->destroyRenderTarget( mDummyRenderTexture->getName() );
    }
    //-----------------------------------------------------------------------------------
    PixelBox GL3PlusDepthPixelBuffer::lockImpl( const Image::Box &lockBox, LockOptions options )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusDepthPixelBuffer::lockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthPixelBuffer::unlockImpl(void)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusDepthPixelBuffer::unlockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthPixelBuffer::_clearSliceRTT( size_t zoffset )
    {
        mDummyRenderTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthPixelBuffer::blitFromMemory( const PixelBox &src, const Image::Box &dstBox )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusDepthPixelBuffer::blitFromMemory" );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthPixelBuffer::blitToMemory( const Image::Box &srcBox, const PixelBox &dst )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusDepthPixelBuffer::blitToMemory" );
    }
    //-----------------------------------------------------------------------------------
    RenderTexture* GL3PlusDepthPixelBuffer::getRenderTarget( size_t slice )
    {
        return mDummyRenderTexture;
    }
}
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    GL3PlusDepthTextureTarget::GL3PlusDepthTextureTarget( GL3PlusDepthTexture *ultimateTextureOwner,
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
    GL3PlusDepthTextureTarget::~GL3PlusDepthTextureTarget()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTextureTarget::setDepthBufferPool( uint16 poolId )
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
    bool GL3PlusDepthTextureTarget::attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch )
    {
        bool retVal = RenderTexture::attachDepthBuffer( depthBuffer, exactFormatMatch );

        if( mDepthBuffer )
        {
            assert( dynamic_cast<GL3PlusDepthBuffer*>(mDepthBuffer) );
            GL3PlusDepthBuffer *gl3PlusDepthBuffer = static_cast<GL3PlusDepthBuffer*>(mDepthBuffer);
            mUltimateTextureOwner->_setGlTextureName( gl3PlusDepthBuffer->getDepthBuffer() );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTextureTarget::detachDepthBuffer(void)
    {
        RenderTexture::detachDepthBuffer();
        mUltimateTextureOwner->_setGlTextureName( 0 );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDepthTextureTarget::getFormatsForPso(
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
    void GL3PlusDepthTextureTarget::getCustomAttribute( const String& name, void* pData )
    {
        if( name == GL3PlusRenderTexture::CustomAttributeString_FBO )
        {
            *static_cast<GL3PlusFrameBufferObject**>(pData) = 0;
        }
        else if (name == "GL_MULTISAMPLEFBOID")
        {
            if( mDepthBuffer )
            {
                assert( dynamic_cast<GL3PlusDepthBuffer*>(mDepthBuffer) );
                GL3PlusDepthBuffer *gl3PlusDepthBuffer = static_cast<GL3PlusDepthBuffer*>(mDepthBuffer);
                *static_cast<GLuint*>(pData) = gl3PlusDepthBuffer->getDepthBuffer();
            }
        }
    }
}
