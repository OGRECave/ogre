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

#include "OgreAndroidGLContext.h"

namespace Ogre {
    AndroidGLContext::AndroidGLContext(const AndroidGLSupport *glsupport, int handle)
		:mGLSupport(glsupport), mHandle(handle), mDelegate(0)
    {
    }

    AndroidGLContext::~AndroidGLContext()
    {
    }
	
	void AndroidGLContext::setCurrent()
    {
		if(mDelegate)
			mDelegate->setCurrent(mHandle);
    }

    void AndroidGLContext::endCurrent()
    {
		if(mDelegate)
			mDelegate->endCurrent(mHandle);
    }

    GLES2Context* AndroidGLContext::clone() const
    {
        return new AndroidGLContext(mGLSupport, mHandle);
    }
	
	void AndroidGLContext::setDelegate(AndroidGLContextDelegate *delegate)
	{
		mDelegate = delegate;
	}
	
	AndroidGLContextDelegate *AndroidGLContext::getDelegate()
	{
		return mDelegate;
	}
	
	int AndroidGLContext::getHandle() const
	{
		return mHandle;
	}
}
