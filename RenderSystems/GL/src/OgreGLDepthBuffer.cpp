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
#include "OgreGLDepthBuffer.h"
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLRenderSystem.h"
#include "OgreGLFrameBufferObject.h"

namespace Ogre
{
    GLDepthBuffer::GLDepthBuffer( uint16 poolId, GLRenderSystem *renderSystem, GLContext *creatorContext,
                                    GLRenderBuffer *depth, GLRenderBuffer *stencil,
                                    uint32 width, uint32 height, uint32 fsaa,
                                    bool manual )
    : GLDepthBufferCommon( poolId, renderSystem, creatorContext, depth, stencil, width, height, fsaa,  manual)
    {
        if( mDepthBuffer )
        {
            switch( mDepthBuffer->getGLFormat() )
            {
            case GL_DEPTH_COMPONENT16:
                mBitDepth = 16;
                break;
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH24_STENCIL8_EXT:
	            mBitDepth = 24;
	            break;
            case GL_DEPTH_COMPONENT32:
                mBitDepth = 32;
                break;
            }
        }
    }
}
