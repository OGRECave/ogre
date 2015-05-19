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
#include "OgreGL3PlusDepthBuffer.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreGL3PlusFrameBufferObject.h"

namespace Ogre
{

    GL3PlusDepthBuffer::GL3PlusDepthBuffer( uint16 poolId, GL3PlusRenderSystem *renderSystem,
                                            GL3PlusContext *creatorContext,
                                            GLenum depthFormat, GLenum stencilFormat,
                                            uint32 width, uint32 height, uint32 fsaa,
                                            uint32 multiSampleQuality, PixelFormat pixelFormat,
                                            bool isDepthTexture, bool _isManual ) :
                DepthBuffer( poolId, 0, width, height, fsaa, "", pixelFormat,
                             isDepthTexture, _isManual ),
                mMultiSampleQuality( multiSampleQuality ),
                mCreatorContext( creatorContext ),
                mDepthBufferName( 0 ),
                mStencilBufferName( 0 ),
                mRenderSystem( renderSystem )
    {
        switch( depthFormat )
        {
        case GL_DEPTH_COMPONENT16:
            mBitDepth = 16;
            break;
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH24_STENCIL8:  // Packed depth / stencil
            mBitDepth = 24;
            break;
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_DEPTH32F_STENCIL8:
            mBitDepth = 32;
            break;
        default:
            mBitDepth = 0;
            break;
        }

        if( depthFormat == GL_NONE && stencilFormat == GL_NONE )
        {
            assert( _isManual &&
                    "depthFormat = GL_NONE && stencilFormat = GL_NONE but _isManual = false" );
            return; //Most likely a dummy depth buffer
        }

        if( mDepthTexture )
        {
            assert( stencilFormat == GL_NONE && "OpenGL specs don't allow depth textures "
                    "with separate stencil format. We should have never hit this path." );

            const GL3PlusSupport *support = renderSystem->getGLSupport();
            const bool hasGL42 = support->hasMinGLVersion( 4, 2 );
            const bool hasGL43 = support->hasMinGLVersion( 4, 3 );

            OCGE( glActiveTexture( GL_TEXTURE0 ) );
            OCGE( glGenTextures( 1, &mDepthBufferName ) );

            if( !mFsaa )
            {
                OCGE( glBindTexture( GL_TEXTURE_2D, mDepthBufferName ) );
                OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 ) );
                OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 ) );

                if( hasGL42 || support->checkExtension("GL_ARB_texture_storage") )
                {
                    OCGE( glTexStorage2D( GL_TEXTURE_2D, GLint(1), depthFormat,
                                          GLsizei(mWidth), GLsizei(mHeight) ) );
                }
                else
                {
                    OCGE( glTexImage2D( GL_TEXTURE_2D, GLint(0), depthFormat,
                                        GLsizei(mWidth), GLsizei(mHeight),
                                        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL ) );
                }
            }
            else
            {
                OCGE( glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, mDepthBufferName ) );
                OCGE( glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BASE_LEVEL, 0 ) );
                OCGE( glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0 ) );

                if( hasGL43 || support->checkExtension("GL_ARB_texture_storage_multisample") )
                {
                    OCGE( glTexStorage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE,
                                                     GLsizei(mFsaa), depthFormat,
                                                     GLsizei(mWidth), GLsizei(mHeight), GL_TRUE ) );
                }
                else
                {
                    OCGE( glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE,
                                                   GLsizei(mFsaa), depthFormat,
                                                   GLsizei(mWidth), GLsizei(mHeight), GL_TRUE ) );
                }
            }
        }
        else
        {
            if( depthFormat != GL_NONE )
                mDepthBufferName = createRenderBuffer( depthFormat );
            if( stencilFormat != GL_NONE )
                mStencilBufferName = createRenderBuffer( stencilFormat );
        }
    }

    GL3PlusDepthBuffer::~GL3PlusDepthBuffer()
    {
        if( mDepthTexture )
        {
            assert( mStencilBufferName == 0 && "OpenGL specs don't allow depth textures "
                    "with separate stencil format. We should have never hit this path." );

            if( mDepthBufferName )
                glDeleteTextures( 1, &mDepthBufferName );
        }
        else
        {
            if( mDepthBufferName )
                glDeleteFramebuffers( 1, &mDepthBufferName );
            if( mStencilBufferName )
                glDeleteFramebuffers( 1, &mStencilBufferName );
        }
    }
    //---------------------------------------------------------------------
    GLuint GL3PlusDepthBuffer::createRenderBuffer( GLenum format )
    {
        GLuint renderBufferName = 0;
        OCGE( glGenRenderbuffers( 1, &renderBufferName ) );
        OCGE( glBindRenderbuffer( GL_RENDERBUFFER, renderBufferName ) );

        if( !mFsaa )
        {
            OCGE( glRenderbufferStorageMultisample( GL_RENDERBUFFER, GLsizei(mFsaa), format,
                                                    GLsizei(mWidth), GLsizei(mHeight) ) );
        }
        else
        {
            OCGE( glRenderbufferStorage( GL_RENDERBUFFER, format,
                                         GLsizei(mWidth), GLsizei(mHeight) ) );
        }

        return renderBufferName;
    }
    //---------------------------------------------------------------------
    bool GL3PlusDepthBuffer::isCompatible( RenderTarget *renderTarget, bool exactFormatMatch ) const
    {
        bool retVal = false;

        //Check standard stuff first.
        if( mRenderSystem->getCapabilities()->hasCapability( RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL ) )
        {
            retVal = DepthBuffer::isCompatible( renderTarget, exactFormatMatch );
        }
        else
        {
            if( this->getWidth() != renderTarget->getWidth() ||
                this->getHeight() != renderTarget->getHeight() ||
                this->getFsaa() != renderTarget->getFSAA() ||
                (exactFormatMatch && mDepthTexture != renderTarget->prefersDepthTexture()) ||
                (!exactFormatMatch && (mFormat != PF_D24_UNORM_S8_UINT || mFormat != PF_D24_UNORM) ) )
            {
                retVal = false;
            }
            else
            {
                retVal = true;
            }
        }

        //Now check this is the appropriate format
        GL3PlusFrameBufferObject *fbo = 0;
        renderTarget->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_FBO, &fbo);

        if( !fbo )
        {
            GL3PlusContext *windowContext = 0;
            renderTarget->getCustomAttribute( GL3PlusRenderTexture::CustomAttributeString_GLCONTEXT,
                                              &windowContext );

            //Non-FBO targets and FBO depth surfaces don't play along,
            //only dummies which match the same context
            if( !mDepthBufferName && !mStencilBufferName && mCreatorContext == windowContext )
                retVal = true;
        }

        return retVal;
    }
    //---------------------------------------------------------------------
    void GL3PlusDepthBuffer::bindToFramebuffer(void)
    {
        //Either GL_DEPTH_ATTACHMENT, GL_DEPTH_STENCIL or GL_STENCIL_ATTACHMENT
        GLenum originFormat = GL3PlusPixelUtil::getGLOriginFormat( mFormat );

        if( mDepthTexture )
        {
            assert( mStencilBufferName == 0 && "OpenGL specs don't allow depth textures "
                    "with separate stencil format. We should have never hit this path." );

            OCGE( glFramebufferTexture( GL_FRAMEBUFFER, originFormat, mDepthBufferName, 0 ) );
        }
        else
        {
            if( mDepthBufferName )
            {
                OCGE( glFramebufferRenderbuffer( GL_FRAMEBUFFER, originFormat,
                                                 GL_RENDERBUFFER, mDepthBufferName ) );
            }
            if( mStencilBufferName )
            {
                OCGE( glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                 GL_RENDERBUFFER, mDepthBufferName ) );
            }
        }
    }
}
