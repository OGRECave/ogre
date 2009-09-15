/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreEGLRenderTexture.h"
#include "OgreEGLContext.h"
#include "OgreEGLSupport.h"

#include <iostream>
#include <climits>

namespace Ogre {
    EGLPBuffer::EGLPBuffer(EGLSupport* glsupport, PixelComponentType format,
                           size_t width, size_t height)
        : GLESPBuffer(format, width, height)
    {

    }

	//Changed the constructor to a member function so that the
	//native constructor would be called first. This member
	//function is then called from the native constructor.
    void EGLPBuffer::initEGLPBuffer()
    {

//	These are now initialized in the native constructors.
//	mGLSupport = glsupport;
//        mGlDisplay = mGLSupport->getGLDisplay();
        mEglDrawable = 0;
        ::EGLConfig glConfig = 0;

        bool isFloat = false;
        int bits = 0;

        switch (mFormat)
        {
            case PCT_BYTE:
                bits = 8;
                break;

            case PCT_SHORT:
            case PCT_FLOAT16:
                bits = 16;
                break;

            case PCT_FLOAT32:
                bits = 32;
                break;

            default:
                break;
        }

        if (mFormat == PCT_FLOAT16 || mFormat == PCT_FLOAT32)
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "No support for Floating point PBuffers",
                        "EGLRenderTexture::initEGLPBuffer");
        }

        int minAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };

        int maxAttribs[] = {
            EGL_RED_SIZE, bits,
            EGL_GREEN_SIZE, bits,
            EGL_BLUE_SIZE, bits,
            EGL_ALPHA_SIZE, bits,
            EGL_STENCIL_SIZE, INT_MAX,
            EGL_NONE
        };

        int pBufferAttribs[] = {
			// First we specify the width of the surface...
            EGL_WIDTH, mWidth,
			// ...then the height of the surface...
            EGL_HEIGHT, mHeight,
			/* ... then we specifiy the target for the texture
			that will be created when the pbuffer is created...*/
			EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
			/*..then the format of the texture that will be created
			when the pBuffer is bound to a texture...*/
            EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
			// The final thing is EGL_NONE which signifies the end.
            EGL_NONE
        };


        glConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);

        EGL_CHECK_ERROR;
        mEglDrawable = eglCreatePbufferSurface(mGlDisplay, glConfig, pBufferAttribs);
        EGL_CHECK_ERROR;

        if (!glConfig || !mEglDrawable)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Unable to create Pbuffer",
                        "EGLPBuffer::EGLPBuffer");
        }
        GLint glConfigID;
        GLint iWidth, iHeight;

        eglGetConfigAttrib(mGlDisplay, glConfig, EGL_CONFIG_ID, &glConfigID);
        EGL_CHECK_ERROR;
        eglQuerySurface(mGlDisplay, mEglDrawable, EGL_WIDTH, &iWidth);
        EGL_CHECK_ERROR;
        eglQuerySurface(mGlDisplay, mEglDrawable, EGL_HEIGHT, &iHeight);
        EGL_CHECK_ERROR;

        mWidth = iWidth;
        mHeight = iHeight;
        LogManager::getSingleton().logMessage(LML_NORMAL, "EGLPBuffer::create used final dimensions " + StringConverter::toString(mWidth) + " x " + StringConverter::toString(mHeight));
        LogManager::getSingleton().logMessage("EGLPBuffer::create used FBConfigID " + StringConverter::toString(glConfigID));

    }

    EGLPBuffer::~EGLPBuffer()
    {
		eglDestroySurface(mGlDisplay, mEglDrawable);
        LogManager::getSingleton().logMessage(LML_NORMAL, "EGLPBuffer::PBuffer destroyed");
    }


}
