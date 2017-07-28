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
#include "OgreGL3PlusPixelFormat.h"
#include "OgreGL3PlusFBORenderTexture.h"

namespace Ogre
{

    GL3PlusDepthBuffer::GL3PlusDepthBuffer( uint16 poolId, GL3PlusRenderSystem *renderSystem,
                                            GL3PlusContext *creatorContext,
                                            GLenum depthFormat, GLenum stencilFormat,
                                            uint32 width, uint32 height, uint32 fsaa,
                                            uint32 multiSampleQuality, PixelFormat pixelFormat,
                                            bool isDepthTexture, bool _isManual ) :
                DepthBuffer( poolId, 0, width, height, fsaa, "", pixelFormat,
                             isDepthTexture, _isManual, renderSystem ),
                mMultiSampleQuality( multiSampleQuality ),
                mCreatorContext( creatorContext ),
                mDepthBufferName( 0 ),
                mStencilBufferName( 0 )
    {
        GLenum formatType;

        switch( depthFormat )
        {
        case GL_DEPTH_COMPONENT16:
            mBitDepth = 16;
            formatType = GL_UNSIGNED_SHORT;
            break;
        case GL_DEPTH_COMPONENT24:
            mBitDepth = 24;
            formatType = GL_UNSIGNED_INT;
            break;
        case GL_DEPTH24_STENCIL8:  // Packed depth / stencil
            mBitDepth = 24;
            formatType = GL_UNSIGNED_INT_24_8;
            break;
        case GL_DEPTH_COMPONENT32:
            mBitDepth = 32;
            formatType = GL_UNSIGNED_INT;
            break;
        case GL_DEPTH_COMPONENT32F:
            mBitDepth = 32;
            formatType = GL_FLOAT;
            break;
        case GL_DEPTH32F_STENCIL8:  // Packed depth / stencil
            mBitDepth = 32;
            formatType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
        default:
            mBitDepth = 0;
            formatType = GL_UNSIGNED_INT;
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

                OCGE( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
                OCGE( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
                OCGE( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
                OCGE( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
                OCGE( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE) );
                OCGE( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL) );
                
                if( hasGL42 || support->checkExtension("GL_ARB_texture_storage") )
                {
                    OCGE( glTexStorage2D( GL_TEXTURE_2D, GLint(1), depthFormat,
                                          GLsizei(mWidth), GLsizei(mHeight) ) );
                }
                else
                {
                    OCGE( glTexImage2D( GL_TEXTURE_2D, GLint(0), depthFormat,
                                        GLsizei(mWidth), GLsizei(mHeight),
                                        0, GL_DEPTH_COMPONENT, formatType, NULL ) );
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

        if( depthFormat == GL_DEPTH24_STENCIL8 || depthFormat == GL_DEPTH32F_STENCIL8 )
            mStencilBufferName = mDepthBufferName;
    }

    GL3PlusDepthBuffer::~GL3PlusDepthBuffer()
    {
        if( mDepthTexture )
        {
            assert( !hasSeparateStencilBuffer() && "OpenGL specs don't allow depth textures "
                    "with separate stencil format. We should have never hit this path." );

            if( mDepthBufferName )
                glDeleteTextures( 1, &mDepthBufferName );
        }
        else
        {
            if( mDepthBufferName )
                glDeleteFramebuffers( 1, &mDepthBufferName );
            if( hasSeparateStencilBuffer() )
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
            OCGE( glRenderbufferStorage( GL_RENDERBUFFER, format,
                                         GLsizei(mWidth), GLsizei(mHeight) ) );
        }
        else
        {
            OCGE( glRenderbufferStorageMultisample( GL_RENDERBUFFER, GLsizei(mFsaa), format,
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

        //Now check this DepthBuffer is for FBOs and RenderTarget is for FBOs
        //(or that this DepthBuffer is for a RenderWindow, and RenderTarget is for RenderWindow)
        //while skipping depth textures.
        GL3PlusFrameBufferObject *fbo = 0;
        renderTarget->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_FBO, &fbo);

        if( !fbo && !renderTarget->getForceDisableColourWrites() )
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
    void GL3PlusDepthBuffer::bindToFramebuffer( GLenum target )
    {
        if( mDepthTexture )
        {
            assert( !hasSeparateStencilBuffer() && "OpenGL specs don't allow depth textures "
                    "with separate stencil format. We should have never hit this path." );

            OCGE( glFramebufferTexture( target, GL_DEPTH_ATTACHMENT, mDepthBufferName, 0 ) );
            
            if( mStencilBufferName )
                OCGE( glFramebufferTexture( target, GL_STENCIL_ATTACHMENT, mStencilBufferName, 0 ) );
        }
        else
        {
            if( mDepthBufferName )
            {
                OCGE( glFramebufferRenderbuffer( target, GL_DEPTH_ATTACHMENT,
                                                 GL_RENDERBUFFER, mDepthBufferName ) );
            }
            if( mStencilBufferName )
            {
                OCGE( glFramebufferRenderbuffer( target, GL_STENCIL_ATTACHMENT,
                                                 GL_RENDERBUFFER, mStencilBufferName ) );
            }
        }
    }
    //---------------------------------------------------------------------
    bool GL3PlusDepthBuffer::hasSeparateStencilBuffer() const
    {
        return mStencilBufferName && mDepthBufferName != mStencilBufferName;
    }
    //---------------------------------------------------------------------
    bool GL3PlusDepthBuffer::copyToImpl( DepthBuffer *_destination )
    {
        GL3PlusDepthBuffer *destination = static_cast<GL3PlusDepthBuffer*>( _destination );

        //Viewport state gets affected.
        mRenderSystem->_setViewport( 0 );

        // Store old binding so it can be restored later
        GLint oldfb, oldDrawBuffer;
        OCGE( glGetIntegerv( GL_FRAMEBUFFER_BINDING, &oldfb ) );
        OCGE( glGetIntegerv( GL_DRAW_BUFFER, &oldDrawBuffer ) );

        GL3PlusFBOManager *fboMan = static_cast<GL3PlusFBOManager*>(
                                            GL3PlusRTTManager::getSingletonPtr() );

        // Set up temporary FBO
        OCGE( glBindFramebuffer( GL_READ_FRAMEBUFFER, this->getDepthBuffer() ?
                                                        fboMan->getTemporaryFBO(0) : 0 ) );
        OCGE( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, destination->getDepthBuffer() ?
                                                        fboMan->getTemporaryFBO(1) : 0 ) );

        OCGE( glViewport( 0, 0, mWidth, mHeight ) );

        if( this->getDepthBuffer() )
            this->bindToFramebuffer( GL_READ_FRAMEBUFFER );
        else
            OCGE( glReadBuffer( GL_DEPTH_ATTACHMENT ) );

        if( destination->getDepthBuffer() )
            destination->bindToFramebuffer( GL_DRAW_FRAMEBUFFER );
        else
            OCGE( glDrawBuffer( GL_DEPTH_ATTACHMENT ) );

        GLenum readStatus, drawStatus;
        OCGE( readStatus = glCheckFramebufferStatus( GL_READ_FRAMEBUFFER ) );
        OCGE( drawStatus = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER ) );

        const bool retVal = readStatus == GL_FRAMEBUFFER_COMPLETE &&
                            drawStatus == GL_FRAMEBUFFER_COMPLETE;
        if( retVal )
        {
            OCGE( glBlitFramebuffer( 0, 0, mWidth, mHeight,
                                     0, 0, mWidth, mHeight,
                                     GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                                     GL_NEAREST ) );
        }

        //Unbind them from the temporary FBOs (we don't want "accidental" bugs)
        OCGE( glFramebufferRenderbuffer( GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                         GL_RENDERBUFFER, 0 ) );
        OCGE( glFramebufferRenderbuffer( GL_READ_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                         GL_RENDERBUFFER, 0 ) );
        OCGE( glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                         GL_RENDERBUFFER, 0 ) );
        OCGE( glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                         GL_RENDERBUFFER, 0 ) );

        // Reset read buffer/framebuffer
        OCGE( glReadBuffer( GL_NONE ) );
        OCGE( glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 ) );

        // Restore old framebuffer
        OCGE( glDrawBuffer( oldDrawBuffer ) );
        OCGE( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, oldfb ) );

        return retVal;
    }
}
