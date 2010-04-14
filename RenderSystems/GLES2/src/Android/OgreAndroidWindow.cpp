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
--------------------------------------------------------------------------*/

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
		: mGLSupport(glsupport), mClosed(false)
	{
	}

	AndroidWindow::~AndroidWindow()
	{

	}

	void AndroidWindow::getCustomAttribute( const String& name, void* pData )
	{
		
	}

	AndroidGLContext * AndroidWindow::createGLContext() const
	{
		return new AndroidGLContext(mGLSupport);
	}

	void AndroidWindow::getLeftAndTopFromNativeWindow( int & left, int & top, uint width, uint height )
	{
	}

	void AndroidWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
	{
		if (miscParams)
		{
			NameValuePairList::const_iterator opt;
			NameValuePairList::const_iterator end = miscParams->end();

		}
	}

	void AndroidWindow::createNativeWindow( int &left, int &top, uint &width, uint &height, String &title )
	{
		WindowEventUtilities::_addRenderWindow(this);
	}

	void AndroidWindow::reposition( int left, int top )
	{
		
	}

	void AndroidWindow::resize(uint width, uint height)
	{
		
	}

	void AndroidWindow::windowMovedOrResized()
	{
		
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
	
	}
		
	bool AndroidWindow::isClosed(void) const
	{
		return mClosed;
	}

	//Moved EGLWindow::create to native source because it has native calls in it
    void AndroidWindow::create(const String& name, uint width, uint height,
                           bool fullScreen, const NameValuePairList *miscParams)
    {
        
	}



}