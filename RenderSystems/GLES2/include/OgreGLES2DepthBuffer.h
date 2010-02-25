/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
-----------------------------------------------------------------------------
*/
#ifndef __GLES2DepthBuffer_H__
#define __GLES2DepthBuffer_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreDepthBuffer.h"


namespace Ogre 
{
    class GLES2Context;
    class GLES2RenderBuffer;
    class GLES2RenderSystem;
	/**
		@copydoc DepthBuffer

		OpenGL supports 2 different methods: FBO & Copy.
		Each one has it's own limitations. Non-FBO methods are solved using "dummy" DepthBuffers.
		That is, a DepthBuffer pointer is attached to the RenderTarget (for the sake of consistency)
		but it doesn't actually contain a Depth surface/renderbuffer (mDepthBuffer & mStencilBuffer are
		null pointers all the time) Those dummy DepthBuffers are identified thanks to their GL context.
		Note that FBOs don't allow sharing with the main window's depth buffer. Therefore even
		when FBO is enabled, a dummy DepthBuffer is still used to manage the windows.
	*/
	class GLES2DepthBuffer : public DepthBuffer
	{
	public:
		GLES2DepthBuffer( uint16 poolId, GLES2RenderSystem *renderSystem, GLES2Context *creatorContext,
						GLES2RenderBuffer *depth, GLES2RenderBuffer *stencil,
						uint32 width, uint32 height, uint32 fsaa, uint32 multiSampleQuality,
						bool isManual );
		~GLES2DepthBuffer();

		/// @copydoc DepthBuffer::isCompatible
		virtual bool isCompatible( RenderTarget *renderTarget ) const;

		GLES2Context* getGLContext() const { return mCreatorContext; }
		GLES2RenderBuffer* getDepthBuffer() const  { return mDepthBuffer; }
		GLES2RenderBuffer* getStencilBuffer() const { return mStencilBuffer; }

	protected:
		uint32						mMultiSampleQuality;
		GLES2Context					*mCreatorContext;
		GLES2RenderBuffer			*mDepthBuffer;
		GLES2RenderBuffer			*mStencilBuffer;
		GLES2RenderSystem			*mRenderSystem;
	};
}
#endif
