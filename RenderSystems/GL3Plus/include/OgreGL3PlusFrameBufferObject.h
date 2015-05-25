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
#ifndef __OgreGL3PlusFBO_H__
#define __OgreGL3PlusFBO_H__

#include "OgreGL3PlusRenderTexture.h"
#include "OgreGL3PlusContext.h"

namespace Ogre {
    
    class GL3PlusFBOManager;
    struct GL3PlusSurfaceDesc;

    /** Frame Buffer Object abstraction.
    */
    class _OgreGL3PlusExport GL3PlusFrameBufferObject
    {
    public:
        GL3PlusFrameBufferObject(GL3PlusFBOManager *manager, uint fsaa);
        ~GL3PlusFrameBufferObject();

        /** Bind a surface to a certain attachment point.
            attachment: 0..OGRE_MAX_MULTIPLE_RENDER_TARGETS-1
        */
        void bindSurface(size_t attachment, const GL3PlusSurfaceDesc &target);
        /** Unbind attachment
        */
        void unbindSurface(size_t attachment);
        
        /** Bind FrameBufferObject
        */
        void bind();

        /** Swap buffers - only useful when using multisample buffers.
        */
        void swapBuffers();

        /** This function acts very similar to @see GL3PlusFBORenderTexture::attachDepthBuffer
            The difference between D3D & OGL is that D3D setups the DepthBuffer before rendering,
            while OGL setups the DepthBuffer per FBO. So the DepthBuffer (RenderBuffer) needs to
            be attached for OGL.
        */
        void attachDepthBuffer( DepthBuffer *depthBuffer );
        void detachDepthBuffer();

        /// Get the GL id for the FBO
        GLuint getGLFBOID() const { return mFB; }
        /// Get the GL id for the multisample FBO
        GLuint getGLMultisampleFBOID() const { return mMultisampleFB; }
        
        /// Accessors
        uint32 getWidth();
        uint32 getHeight();
        PixelFormat getFormat();
        GLsizei getFSAA();
        
        GL3PlusFBOManager *getManager() { return mManager; }
        const GL3PlusSurfaceDesc &getSurface(size_t attachment) { return mColour[attachment]; }
    private:
        GL3PlusFBOManager *mManager;
        GLsizei mNumSamples;
        GLuint mFB;
        GLuint mMultisampleFB;
        GL3PlusSurfaceDesc mMultisampleColourBuffer;
        // Arbitrary number of texture surfaces
        GL3PlusSurfaceDesc mColour[OGRE_MAX_MULTIPLE_RENDER_TARGETS];


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
