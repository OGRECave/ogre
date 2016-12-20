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

#include "OgreGL3PlusNullTexture.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusRenderTexture.h"
#include "OgreGL3PlusFrameBufferObject.h"
#include "OgreRoot.h"
#include "OgreDepthBuffer.h"

namespace Ogre
{
    GL3PlusNullTexture::GL3PlusNullTexture( ResourceManager* creator,
                                            const String& name, ResourceHandle handle,
                                            const String& group, bool isManual,
                                            ManualResourceLoader* loader, GL3PlusSupport& support )
        : GL3PlusTexture( creator, name, handle, group, isManual, loader, support )
    {
        if( !support.hasMinGLVersion( 4, 3 ) &&
            !support.checkExtension( "GL_ARB_framebuffer_no_attachments" ) )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "PF_NULL format requires OpenGL 4.3 or the "
                         "GL_ARB_framebuffer_no_attachments extension. "
                         "Try updating your video card drivers.",
                         "GL3PlusNullTexture::GL3PlusNullTexture" );
        }
    }
    //-----------------------------------------------------------------------------------
    GL3PlusNullTexture::~GL3PlusNullTexture()
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
    // Creation / loading methods
    void GL3PlusNullTexture::createInternalResourcesImpl(void)
    {
        _createSurfaceList();

        // Get final internal format.
        mFormat = getBuffer(0,0)->getFormat();

        mSize = calculateSize();
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullTexture::freeInternalResourcesImpl()
    {
        mSurfaceList.clear();
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullTexture::prepareImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullTexture::unprepareImpl()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullTexture::loadImpl()
    {
        createRenderTexture();
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullTexture::_createSurfaceList()
    {
        mSurfaceList.clear();

        for (uint8 face = 0; face < getNumFaces(); face++)
        {
            v1::HardwarePixelBuffer *buf = OGRE_NEW v1::GL3PlusNullPixelBuffer( this, mName,
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
                    "GL3PlusNullTexture::_createSurfaceList");
            }
        }
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
namespace v1
{
    GL3PlusNullPixelBuffer::GL3PlusNullPixelBuffer( GL3PlusNullTexture *parentTexture,
                                                      const String &baseName,
                                                      uint32 width, uint32 height,
                                                      uint32 Null, PixelFormat format ) :
        HardwarePixelBuffer( width, height, Null, format, false,
                             HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false ),
        mDummyRenderTexture( 0 )
    {
        String name = "NullTexture/" + StringConverter::toString((size_t)this) + "/" + baseName;

        mDummyRenderTexture = OGRE_NEW GL3PlusNullTextureTarget( parentTexture, name, this, 0 );
        mDummyRenderTexture->setDepthBufferPool( DepthBuffer::POOL_NO_DEPTH );

        //TODO: Should we do this?
        Root::getSingleton().getRenderSystem()->attachRenderTarget( *mDummyRenderTexture );
    }
    //-----------------------------------------------------------------------------------
    GL3PlusNullPixelBuffer::~GL3PlusNullPixelBuffer()
    {
        if( mDummyRenderTexture )
            Root::getSingleton().getRenderSystem()->destroyRenderTarget( mDummyRenderTexture->getName() );
    }
    //-----------------------------------------------------------------------------------
    PixelBox GL3PlusNullPixelBuffer::lockImpl( const Image::Box &lockBox, LockOptions options )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusNullPixelBuffer::lockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullPixelBuffer::unlockImpl(void)
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusNullPixelBuffer::unlockImpl" );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullPixelBuffer::_clearSliceRTT( size_t zoffset )
    {
        mDummyRenderTexture = 0;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullPixelBuffer::blitFromMemory( const PixelBox &src, const Image::Box &dstBox )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusNullPixelBuffer::blitFromMemory" );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullPixelBuffer::blitToMemory( const Image::Box &srcBox, const PixelBox &dst )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "GL3PlusNullPixelBuffer::blitToMemory" );
    }
    //-----------------------------------------------------------------------------------
    RenderTexture* GL3PlusNullPixelBuffer::getRenderTarget( size_t slice )
    {
        return mDummyRenderTexture;
    }
}
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    GL3PlusNullTextureTarget::GL3PlusNullTextureTarget( GL3PlusNullTexture *ultimateTextureOwner,
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
    GL3PlusNullTextureTarget::~GL3PlusNullTextureTarget()
    {
    }
    //-----------------------------------------------------------------------------------
    bool GL3PlusNullTextureTarget::attachDepthBuffer( DepthBuffer *depthBuffer, bool exactFormatMatch )
    {
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL,
                     "Null formats don't use a depth buffer. "
                     "Call setDepthBufferPool( DepthBuffer::POOL_NO_DEPTH ) "
                     "on this RTT before rendering!\n"
                     "If you're manually setting the compositor, "
                     "set TextureDefinition::depthBufferId to 0",
                     "GL3PlusNullTextureTarget::attachDepthBuffer" );

        return false;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusNullTextureTarget::getCustomAttribute( const String& name, void* pData )
    {
        if( name == GL3PlusRenderTexture::CustomAttributeString_FBO )
        {
            *static_cast<GL3PlusFrameBufferObject**>(pData) = 0;
        }
    }
}
