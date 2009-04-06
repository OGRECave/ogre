/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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
--------------------------------------------------------------------------*/

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreEGLSupport.h"
#include "OgreEGLWindow.h"
#include "OgreEGLContext.h"
#include "OgreGLESPixelFormat.h"

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

//	Moved EGLWindow::create to native source because it has native calls in it
/*    void EGLWindow::create(const String& name, uint width, uint height,
                           bool fullScreen, const NameValuePairList *miscParams)
    {
        String title = name;
        uint samples = 0;
        int gamma;
        short frequency = 0;
        bool vsync = false;
        ::EGLContext eglContext = 0;
		int left = 0;
		int top  = 0;

		getLeftAndTopFromNativeWindow(left, top, width, height);

        mIsFullScreen = fullScreen;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            if ((opt = miscParams->find("currentGLContext")) != end &&
                StringConverter::parseBool(opt->second))
            {
                eglContext = eglGetCurrentContext();
                if (eglContext)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                                "currentGLContext was specified with no current GL context",
                                "EGLWindow::create");
                }

                eglContext = eglGetCurrentContext();
                mEglSurface = eglGetCurrentSurface(EGL_DRAW);
            }

            // Note: Some platforms support AA inside ordinary windows
            if ((opt = miscParams->find("FSAA")) != end)
            {
                samples = StringConverter::parseUnsignedInt(opt->second);
            }

            if ((opt = miscParams->find("displayFrequency")) != end)
            {
                frequency = (short)StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("vsync")) != end)
            {
                vsync = StringConverter::parseBool(opt->second);
            }

            if ((opt = miscParams->find("gamma")) != end)
            {
                gamma = StringConverter::parseBool(opt->second);
            }

            if ((opt = miscParams->find("left")) != end)
            {
                left = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("top")) != end)
            {
                top = StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("title")) != end)
            {
                title = opt->second;
            }

            if ((opt = miscParams->find("externalGLControl")) != end)
            {
                mIsExternalGLControl = StringConverter::parseBool(opt->second);
            }
		}

		initNativeCreatedWindow(miscParams);

        if (mEglSurface)
        {
            mEglConfig = mGLSupport->getGLConfigFromDrawable (mEglSurface, &width, &height);
        }

        if (!mEglConfig && eglContext)
        {
            mEglConfig = mGLSupport->getGLConfigFromContext(eglContext);

            if (!mEglConfig)
            {
                // This should never happen.
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unexpected failure to determine a EGLFBConfig",
                            "EGLWindow::create");
            }
        }

        mIsExternal = (mEglSurface != 0);



        if (!mEglConfig)
        {
            int minAttribs[] = {
                EGL_LEVEL, 0,
                EGL_DEPTH_SIZE, 16,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
            };

            int maxAttribs[] = {
                EGL_SAMPLES, samples,
                EGL_STENCIL_SIZE, INT_MAX,
                EGL_NONE
            };

            mEglConfig = mGLSupport->selectGLConfig(minAttribs, maxAttribs);
            mHwGamma = false;
        }

        if (!mIsTopLevel)
        {
            mIsFullScreen = false;
            left = top = 0;
        }

        if (mIsFullScreen)
        {
            mGLSupport->switchMode (width, height, frequency);
        }

		if (!mIsExternal)
        {
			createNativeWindow(left, top, width, height, title);
		}

		mContext = createEGLContext();

        ::EGLSurface oldDrawableDraw = eglGetCurrentSurface(EGL_DRAW);
        ::EGLSurface oldDrawableRead = eglGetCurrentSurface(EGL_READ);
        ::EGLContext oldContext  = eglGetCurrentContext();

        int glConfigID;

        mGLSupport->getGLConfigAttrib(mEglConfig, EGL_CONFIG_ID, &glConfigID);
        LogManager::getSingleton().logMessage("EGLWindow::create used FBConfigID = " + StringConverter::toString(glConfigID));

        mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = left;
        mTop = top;
        mActive = true;
		mVisible = true;

        mClosed = false;
	}*/

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

  

    void EGLWindow::swapBuffers(bool waitForVSync)
    {
        if (mClosed || mIsExternalGLControl)
        {
            return;
        }

        glFlush();
        if (eglSwapBuffers(mEglDisplay, mEglSurface) == EGL_FALSE)
        {
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
				"Win32Window::copyContentsToMemory" );
		}

		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen? FB_FRONT : FB_BACK;
		}

		GLenum format = GLESPixelUtil::getGLOriginFormat(dst.format);
		GLenum type = GLESPixelUtil::getGLOriginDataType(dst.format);

		if ((format == 0) || (type == 0))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Unsupported format.",
				"GtkEGLWindow::copyContentsToMemory" );
		}


		// Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));

		// Must change the packing to ensure no overruns!
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		//glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
		glReadPixels((GLint)dst.left, (GLint)dst.top,
			(GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
			format, type, dst.data);

		// restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		//vertical flip
		{
			size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.format);
			size_t height = dst.getHeight();
			uchar *tmpData = new uchar[rowSpan * height];
			uchar *srcRow = (uchar *)dst.data, *tmpRow = tmpData + (height - 1) * rowSpan;

			while (tmpRow >= tmpData)
			{
				memcpy(tmpRow, srcRow, rowSpan);
				srcRow += rowSpan;
				tmpRow -= rowSpan;
			}
			memcpy(dst.data, tmpData, rowSpan * height);

			delete [] tmpData;
		}

    }


    ::EGLSurface EGLWindow::createSurfaceFromWindow(::EGLDisplay display,
                                                    NativeWindowType win)
    {
        ::EGLSurface surface;

        surface = eglCreateWindowSurface(display, mEglConfig, win, NULL);

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
