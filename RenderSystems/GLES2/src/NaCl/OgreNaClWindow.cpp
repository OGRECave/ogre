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

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLES2Prerequisites.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreNaClGLSupport.h"
#include "OgreNaClWindow.h"
#include "OgreNaClGLContext.h"

#include <iostream>
#include <algorithm>
#include <climits>

namespace Ogre {
	NaClWindow::NaClWindow(NaClGLSupport *glsupport)
		: mGLSupport(glsupport), mClosed(false), mContext(0), mInstance(0), mSwapCallback(0)
	{
	}

	NaClWindow::~NaClWindow()
	{
		if(mContext)
			delete mContext;
	}

	void NaClWindow::getCustomAttribute( const String& name, void* pData )
	{
		if(name == "pp::Instance")
		{
			*static_cast<pp::Instance**>(pData) = mInstance;
            return;
		}
        else if(name == "GLCONTEXT")
        {
            *static_cast<GLES2Context **>(pData) = mContext;
            return;
        }
	}

	void NaClWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
	{
        // todo
	}

	void NaClWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
	{
		LogManager::getSingleton().logMessage("\tinitNativeCreatedWindow called");

		if (miscParams)
		{
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

            mInstance = NULL;
			opt = miscParams->find("pp::Instance");
			if(opt != end)
			{
                LogManager::getSingleton().logMessage("\tgetting pp::Instance - if stopped here - it means the parameter is null!");
				mInstance = (pp::Instance*)(Ogre::StringConverter::parseUnsignedLong(opt->second));
                LogManager::getSingleton().logMessage("\tgot the pp::Instance.");
			}
            opt = miscParams->find("SwapCallback");
            if(opt != end)
            {
                LogManager::getSingleton().logMessage("\tgetting SwapCallback - if stopped here - it means the parameter is null!");
                mSwapCallback = (pp::CompletionCallback*)(Ogre::StringConverter::parseUnsignedLong(opt->second));
                LogManager::getSingleton().logMessage("\tgot the SwapCallback.");
            }
			
			if(mInstance != NULL)
			{                
				mContext = new NaClGLContext(this, mGLSupport, mInstance, mSwapCallback);
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"pp::Instance parameter required for NaCl windows.",
					"NaClWindow::initNativeCreatedWindow" );
			}
		}
        LogManager::getSingleton().logMessage("\tinitNativeCreatedWindow ended");
	}

	void NaClWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		LogManager::getSingleton().logMessage("\tcreateNativeWindow called");
	}

	void NaClWindow::reposition( int left, int top )
	{
		LogManager::getSingleton().logMessage("\treposition called");
	}

	void NaClWindow::resize(uint width, uint height)
	{
		LogManager::getSingleton().logMessage("\tresize called");

        mWidth  = width;
        mHeight  = height;

        mContext->resize();
	}

	void NaClWindow::windowMovedOrResized()
	{
		LogManager::getSingleton().logMessage("\twindowMovedOrResized called");
	}
	
	void NaClWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
	
	}
		
	bool NaClWindow::requiresTextureFlipping() const
	{
		return false;
	}
		
	void NaClWindow::destroy(void)
	{
		LogManager::getSingleton().logMessage("\tdestroy called");
	}
		
	bool NaClWindow::isClosed(void) const
	{
		return mClosed;
	}

    void NaClWindow::create(const String& name, uint width, uint height,
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

    void NaClWindow::swapBuffers()
    {
        if (mClosed)
        {
            return;
        }

        mContext->swapBuffers();

    }


}