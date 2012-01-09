/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreAndroidGLSupport.h"
#include "OgreAndroidWindow.h"
#include "OgreAndroidGLContext.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
	AndroidWindow::AndroidWindow(AndroidGLSupport *glsupport)
		: mGLSupport(glsupport), mClosed(false), mContext(0), mHandle(0), mDelegate(0)
	{
	}

	AndroidWindow::~AndroidWindow()
	{
		if(mContext)
			delete mContext;
	}

	void AndroidWindow::getCustomAttribute( const String& name, void* pData )
	{
		if(name == "HANDLE")
		{
			*(int*)pData = mHandle;
			return;
		}
		else if(name == "GLCONTEXT")
		{
			*static_cast<AndroidGLContext**>(pData) = mContext;
            return;
		}
	}

	AndroidGLContext * AndroidWindow::createGLContext(int handle) const
	{
		return new AndroidGLContext(mGLSupport, handle);
	}

	void AndroidWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
	{
		// We don't have a native window.... but I think all android windows are origined
		left = top = 0;
	}

	void AndroidWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
	{
		LogManager::getSingleton().logMessage("\tinitNativeCreatedWindow called");
		if (miscParams)
		{
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

			opt = miscParams->find("externalWindowHandle");
			if(opt != end)
			{
				mHandle = Ogre::StringConverter::parseInt(opt->second);
			}
			
			int ctxHandle = -1;
			opt = miscParams->find("externalGLContext");
			if(opt != end)
			{
				ctxHandle = Ogre::StringConverter::parseInt(opt->second);
			}
			
			if(ctxHandle != -1)
			{
				mContext = new AndroidGLContext(mGLSupport, ctxHandle);
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"externalGLContext parameter required for Android windows.",
					"AndroidWindow::initNativeCreatedWindow" );
			}
		}
	}

	void AndroidWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		LogManager::getSingleton().logMessage("\tcreateNativeWindow called");
	}

	void AndroidWindow::reposition( int left, int top )
	{
		LogManager::getSingleton().logMessage("\treposition called");
	}

	void AndroidWindow::resize(uint width, uint height)
	{
		LogManager::getSingleton().logMessage("\tresize called");
	}

	void AndroidWindow::windowMovedOrResized()
	{
		LogManager::getSingleton().logMessage("\twindowMovedOrResized called");
	}
	
	void AndroidWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
	
	}
		
	bool AndroidWindow::requiresTextureFlipping() const
	{
		return false;
	}
		
	void AndroidWindow::destroy(void)
	{
		LogManager::getSingleton().logMessage("\tdestroy called");
	}
		
	bool AndroidWindow::isClosed(void) const
	{
		return mClosed;
	}

    void AndroidWindow::create(const String& name, uint width, uint height,
                           bool fullScreen, const NameValuePairList *miscParams)
    {
        LogManager::getSingleton().logMessage("\tcreate called");
		
		initNativeCreatedWindow(miscParams);
		
		mName = name;
        mWidth = width;
        mHeight = height;
        mLeft = 0;
        mTop = 0;
        mActive = true;
		//mVisible = true;

        mClosed = false;
	}



}