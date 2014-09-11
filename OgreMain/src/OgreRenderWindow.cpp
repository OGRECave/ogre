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
#include "OgreStableHeaders.h"
#include "OgreRenderWindow.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreViewport.h"
#include "OgreSceneManager.h"

namespace Ogre {

    RenderWindow::RenderWindow()
        : RenderTarget(), mIsFullScreen(false), mIsPrimary(false), mLeft(0), mTop(0)
    {
        mAutoDeactivatedOnFocusChange = true;
    }

    //-----------------------------------------------------------------------
    void RenderWindow::getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth,
		int& left, int& top)
    {
        width = mWidth;
        height = mHeight;
        colourDepth = mColourDepth;
        left = mLeft;
        top = mTop;
    }
    //-----------------------------------------------------------------------
    bool RenderWindow::isFullScreen(void) const
    {
        return mIsFullScreen;
    }
	//-----------------------------------------------------------------------
    bool RenderWindow::isPrimary(void) const
    {
        return mIsPrimary;
    }

    bool RenderWindow::isDeactivatedOnFocusChange() const
    {
        return mAutoDeactivatedOnFocusChange;
    }

    void RenderWindow::setDeactivateOnFocusChange(bool deactivate)
    {
        mAutoDeactivatedOnFocusChange = deactivate;
    }
}
