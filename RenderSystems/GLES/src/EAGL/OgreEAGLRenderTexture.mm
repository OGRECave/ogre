/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreRoot.h"
#include "OgreEAGLRenderTexture.h"

namespace Ogre {
    EAGLPBuffer::EAGLPBuffer(EAGLSupport* glsupport, PixelComponentType format,
                           size_t width, size_t height)
        : GLESPBuffer(format, width, height)
    {
        initEAGLPBuffer();
    }

	//Changed the constructor to a member function so that the
	//native constructor would be called first. This member
	//function is then called from the native constructor.
    void EAGLPBuffer::initEAGLPBuffer()
    {
//        GLuint textureFrameBuffer;

        // Create framebuffer
//        glGenFramebuffersOES(1, &textureFrameBuffer);
//        glBindFramebufferOES(GL_FRAMEBUFFER_OES, textureFrameBuffer);
//        
//        // Attach renderbuffer
//        glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, texture.name, 0);
//        
//        // Unbind frame buffer
//        glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);

        // OR
        
//        GLuint texture, FBO;
//        GLint status;
//        glGenBuffers(1, &FBO);
//        GL_CHECK_ERROR
//        glBindFramebufferOES(GL_FRAMEBUFFER_OES, FBO);
//        GL_CHECK_ERROR
//        glGenTextures(1, &texture);
//        GL_CHECK_ERROR
//        glBindTexture(GL_TEXTURE_2D, texture);
//        GL_CHECK_ERROR
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        GL_CHECK_ERROR
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        GL_CHECK_ERROR
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
//        GL_CHECK_ERROR
//        glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, texture, 0);
//        GL_CHECK_ERROR
//        
//        status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
//        GL_CHECK_ERROR
//        
//        if(status != GL_FRAMEBUFFER_COMPLETE_OES)
//        {
//            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
//                        "Failed to create Frame Buffer Object",
//                        "EAGLRenderTexture::initEAGLPBuffer");
//        }
//
////	These are now initialized in the native constructors.
////	mGLSupport = glsupport;
////        mGlDisplay = mGLSupport->getGLDisplay();
////        mEglDrawable = 0;
////        EAGLConfig glConfig = 0;
//
////        bool isFloat = false;
//        int bits = 0;
//
//        switch (mFormat)
//        {
//            case PCT_BYTE:
//                bits = 8;
//                break;
//
//            case PCT_SHORT:
//            case PCT_FLOAT16:
//                bits = 16;
//                break;
//
//            case PCT_FLOAT32:
//                bits = 32;
//                break;
//
//            case PCT_COUNT:
//            default:
//                break;
//        }
//
//        if (mFormat == PCT_FLOAT16 || mFormat == PCT_FLOAT32)
//        {
//            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
//                        "No support for Floating point PBuffers",
//                        "EAGLRenderTexture::initEAGLPBuffer");
//        }
//
//        GLint glConfigID = 0;
//        GLint iWidth = 0, iHeight = 0;
//
////        glGetConfigAttrib(mGlDisplay, glConfig, GL_CONFIG_ID, &glConfigID);
////        GL_CHECK_ERROR;
////        glQuerySurface(mGlDisplay, mEglDrawable, GL_WIDTH, &iWidth);
////        GL_CHECK_ERROR;
////        glQuerySurface(mGlDisplay, mEglDrawable, GL_HEIGHT, &iHeight);
////        GL_CHECK_ERROR;
//
//        mWidth = iWidth;
//        mHeight = iHeight;
        LogManager::getSingleton().logMessage(LML_NORMAL, "EAGLPBuffer::create unimplemented");
//        LogManager::getSingleton().logMessage(LML_NORMAL, "EAGLPBuffer::create used final dimensions " + StringConverter::toString(mWidth) + " x " + StringConverter::toString(mHeight));
//        LogManager::getSingleton().logMessage("EAGLPBuffer::create used FBConfigID " + StringConverter::toString(glConfigID));
    }

    EAGLPBuffer::~EAGLPBuffer()
    {
//		eglDestroySurface(mGlDisplay, mEglDrawable);
        LogManager::getSingleton().logMessage(LML_NORMAL, "EAGLPBuffer::PBuffer destroyed");
    }
}
