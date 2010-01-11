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
#ifndef __OgreGLES2FBO_H__
#define __OgreGLES2FBO_H__

#include "OgreGLES2RenderTexture.h"
#include "OgreGLES2Context.h"

namespace Ogre {
    
    class GLES2FBOManager;
    struct GLES2SurfaceDesc;

    /** Frame Buffer Object abstraction.
    */
    class _OgrePrivate GLES2FrameBufferObject
    {
    public:
        GLES2FrameBufferObject(GLES2FBOManager *manager, uint fsaa);
        ~GLES2FrameBufferObject();

        /** Bind a surface to a certain attachment point.
            attachment: 0..OGRE_MAX_MULTIPLE_RENDER_TARGETS-1
        */
        void bindSurface(size_t attachment, const GLES2SurfaceDesc &target);
        /** Unbind attachment
        */
        void unbindSurface(size_t attachment);
        
        /** Bind FrameBufferObject
        */
        void bind();

		/** Swap buffers - only useful when using multisample buffers.
		*/
		void swapBuffers();
        
        /// Accessors
        size_t getWidth();
        size_t getHeight();
        PixelFormat getFormat();
        
        GLES2FBOManager *getManager() { return mManager; }
		const GLES2SurfaceDesc &getSurface(size_t attachment) { return mColour[attachment]; }
    private:
        GLES2FBOManager *mManager;
		GLsizei mNumSamples;
        GLuint mFB;
		GLuint mMultisampleFB;
		GLES2SurfaceDesc mMultisampleColourBuffer;
        GLES2SurfaceDesc mDepth;
        GLES2SurfaceDesc mStencil;
        // Arbitrary number of texture surfaces
        GLES2SurfaceDesc mColour[OGRE_MAX_MULTIPLE_RENDER_TARGETS];


		/** Initialise object (find suitable depth and stencil format).
            Must be called every time the bindings change.
            It fails with an exception (ERR_INVALIDPARAMS) if:
            - Attachment point 0 has no binding
            - Not all bound surfaces have the same size
            - Not all bound surfaces have the same internal format
        */
        void initialise();
    };

}

#endif
