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

#include "OgreGLWindow.h"
#include "OgreException.h"
#include "OgreGLRenderSystemCommon.h"
#include "OgreRoot.h"

namespace Ogre
{
GLWindow::GLWindow() : mContext(0)
{
    mIsTopLevel = false;
    mIsFullScreen = false;
    mIsExternal = false;
    mIsExternalGLControl = false;
    mActive = false;
    mHidden = false;
    mVisible = false;
    mVSync = false;
}

//-------------------------------------------------------------------------------------------------//
void GLWindow::setVSyncInterval(unsigned int interval)
{
    mVSyncInterval = interval;
    if (mVSync)
        setVSyncEnabled(true);
}

void GLWindow::copyContentsToMemory(const Box& src, const PixelBox& dst, FrameBuffer buffer)
{
    if (mClosed)
        return;

    if (src.right > mWidth || src.bottom > mHeight || src.front != 0 || src.back != 1 ||
        dst.getWidth() != src.getWidth() || dst.getHeight() != src.getHeight() || dst.getDepth() != 1)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid box");
    }

    if (buffer == FB_AUTO)
    {
        buffer = mIsFullScreen ? FB_FRONT : FB_BACK;
    }

    static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem())
        ->_copyContentsToMemory(getViewport(0), src, dst, buffer);
}
} // namespace Ogre
