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
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{

    //-----------------------------------------------------------------------------
    RenderTexture::RenderTexture(HardwarePixelBuffer *buffer, uint32 zoffset):
        mBuffer(buffer), mZOffset(zoffset)
    {
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;
        mWidth = mBuffer->getWidth();
        mHeight = mBuffer->getHeight();

        if(PixelUtil::isDepth(mBuffer->getFormat()))
            mDepthBufferPoolId = DepthBuffer::POOL_NO_DEPTH;
    }
    RenderTexture::~RenderTexture()
    {
        mBuffer->_clearSliceRTT(0);
    }

    void RenderTexture::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        if (buffer == FB_AUTO) buffer = FB_FRONT;
        OgreAssert(buffer == FB_FRONT, "Invalid buffer");

        mBuffer->blitToMemory(src, dst);
    }
    //---------------------------------------------------------------------
    PixelFormat RenderTexture::suggestPixelFormat() const
    {
        return mBuffer->getFormat();
    }
    //-----------------------------------------------------------------------------
    MultiRenderTarget::MultiRenderTarget(const String &name)
    {
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;
        mName = name;
        /// Width and height is unknown with no targets attached
        mWidth = mHeight = 0;
    }
    void MultiRenderTarget::bindSurface(size_t attachment, RenderTexture* target)
    {
        if(PixelUtil::isDepth(target->suggestPixelFormat()))
            setDepthBufferPool(DepthBuffer::POOL_NO_DEPTH); // unbinds any previously bound depth render buffer

        for (size_t i = mBoundSurfaces.size(); i <= attachment; ++i)
        {
            mBoundSurfaces.push_back(0);
        }
        mBoundSurfaces[attachment] = target;

        bindSurfaceImpl(attachment, target);
    }

    //-----------------------------------------------------------------------------
    void MultiRenderTarget::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "Cannot get MultiRenderTargets pixels",
                    "MultiRenderTarget::copyContentsToMemory");
    }
}
