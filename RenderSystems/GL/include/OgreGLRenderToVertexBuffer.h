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
#ifndef __GLRenderToVertexBuffer_H__
#define __GLRenderToVertexBuffer_H__

#include "OgreRenderToVertexBuffer.h"
#include "OgreGLPrerequisites.h"

namespace Ogre {
    /**
        An object which renders geometry to a vertex.
    @remarks
        This is especially useful together with geometry shaders, as you can
        render procedural geometry which will get saved to a vertex buffer for
        reuse later, without regenerating it again. You can also create shaders
        that run on previous results of those shaders, creating stateful 
        shaders.
    */
    class _OgreGLExport GLRenderToVertexBuffer : public RenderToVertexBuffer
    {    
    public:
        /** C'tor */
        GLRenderToVertexBuffer();
        /** D'tor */
        virtual ~GLRenderToVertexBuffer();

        /**
            Get the render operation for this buffer 
        */
        virtual void getRenderOperation(RenderOperation& op);

        /**
            Update the contents of this vertex buffer by rendering
        */
        virtual void update(SceneManager* sceneMgr);
    protected:
        void reallocateBuffer(size_t index);
        void bindVerticesOutput(Pass* pass);
        GLint getGLSemanticType(VertexElementSemantic semantic);
        String getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index);
        HardwareVertexBufferSharedPtr mVertexBuffers[2];
        size_t mFrontBufferIndex;
        GLuint mPrimitivesDrawnQuery;
    };
}

#endif
