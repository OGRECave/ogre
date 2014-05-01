/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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
--------------------------------------------------------------------------*/

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreEGLSupport.h"
#include "OgreEGLWindow.h"
#include "OgreEGLContext.h"
#include "OgreGLES2PixelFormat.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
    EGLWindow::EGLWindow(EGLSupport *glsupport)
        : mGLSupport(glsupport),
          mContext(0),
		  mEglConfig(0),
		  mEglSurface(0),
		  mWindow(0),
		  mNativeDisplay(0),
		  mEglDisplay(EGL_NO_DISPLAY)
    {
        mIsTopLevel = false;
        mIsFullScreen = false;
        mClosed = false;
        mActive = true;//todo
        mIsExternalGLControl = false;
		mVisible = false;
    }

    EGLWindow::~EGLWindow()
    {
        destroy();

        if (mContext)
        {
            delete mContext;
        }

        mContext = 0;
    }

    void EGLWindow::destroy(void)
    {
        if (mClosed)
        {
            return;
        }

        mClosed = true;
        mActive = false;

        if (!mIsExternal)
        {
            WindowEventUtilities::_removeRenderWindow(this);
        }

        if (mIsFullScreen)
        {
            mGLSupport->switchMode();
            switchFullScreen(false);
        }
    }

    void EGLWindow::setFullscreen(bool fullscreen, uint width, uint height)
    {
        short frequency = 0;

        if (mClosed || !mIsTopLevel)
        {
            return;
        }

        if (fullscreen == mIsFullScreen && width == mWidth && height == mHeight)
        {
            return;
        }
		if (fullscreen)
        {
            mGLSupport->switchMode(width, height, frequency);
        }
        else
        {
            mGLSupport->switchMode();
        }

        if (mIsFullScreen != fullscreen)
        {
            switchFullScreen(fullscreen);
        }

        if (!mIsFullScreen)
        {
            resize(width, height);
            reposition(mLeft, mTop);
        }
    }

    bool EGLWindow::isClosed() const
    {
        return mClosed;
    }

    bool EGLWindow::isVisible() const
    {
        return mVisible;
    }

    void EGLWindow::setVisible(bool visible)
    {
        mVisible = visible;
    }

  

    void EGLWindow::swapBuffers()
    {
        if (mClosed || mIsExternalGLControl)
        {
            return;
        }

        if (eglSwapBuffers(mEglDisplay, mEglSurface) == EGL_FALSE)
        {
			EGL_CHECK_ERROR
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to SwapBuffers",
                        __FUNCTION__);
        }
    }

    void EGLWindow::getCustomAttribute( const String& name, void* pData )
    {
        if (name == "DISPLAYNAME")
        {
            *static_cast<String*>(pData) = mGLSupport->getDisplayName();
            return;
        }
        else if (name == "DISPLAY")
        {
            *static_cast<EGLDisplay*>(pData) = mEglDisplay;
            return;
        }
        else if (name == "GLCONTEXT")
        {
            *static_cast<EGLContext**>(pData) = mContext;
            return;
        } 
		else if (name == "WINDOW")
		{
			*static_cast<NativeWindowType*>(pData) = mWindow;
			return;
		} 
	}

    void EGLWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
		if ((dst.left < 0) || (dst.right > mWidth) ||
			(dst.top < 0) || (dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Invalid box.",
				"EGLWindow::copyContentsToMemory" );
		}

		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen? FB_FRONT : FB_BACK;
		}

		GLenum format = GLES2PixelUtil::getGLOriginFormat(dst.format);
		GLenum type = GLES2PixelUtil::getGLOriginDataType(dst.format);

		if ((format == 0) || (type == 0))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Unsupported format.",
				"EGLWindow::copyContentsToMemory" );
		}


		// Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));

		OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));

#if OGRE_NO_GLES3_SUPPORT == 0
        if(dst.getWidth() != dst.rowPitch)
            glPixelStorei(GL_PACK_ROW_LENGTH, dst.rowPitch);
#endif
		// Must change the packing to ensure no overruns!
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

#if OGRE_NO_GLES3_SUPPORT == 0
		glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
#endif
        glReadPixels((GLint)0, (GLint)(mHeight - dst.getHeight()),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());

		// restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
#if OGRE_NO_GLES3_SUPPORT == 0
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
#endif

        PixelUtil::bulkPixelVerticalFlip(dst);
    }


    ::EGLSurface EGLWindow::createSurfaceFromWindow(::EGLDisplay display,
                                                    NativeWindowType win)
    {
        ::EGLSurface surface;

        surface = eglCreateWindowSurface(display, mEglConfig, (EGLNativeWindowType)win, NULL);
        EGL_CHECK_ERROR

        if (surface == EGL_NO_SURFACE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to create EGLSurface based on X NativeWindowType",
                        __FUNCTION__);
        }
        return surface;
    }

	bool EGLWindow::requiresTextureFlipping() const
	{
		return false;
	}

}
