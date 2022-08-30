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
#ifndef __GLVERTEXARRAYOBJECT_H__
#define __GLVERTEXARRAYOBJECT_H__

#include "OgreHardwareVertexBuffer.h"

namespace Ogre {
    class GLContext;
    class GLSLProgramCommon;
    class GLRenderSystemCommon;

    /** Specialisation of VertexDeclaration for OpenGL Vertex Array Object usage */
    class GLVertexArrayObject : public VertexDeclaration
    {
    protected:
        /// Context that was used to create VAO. It could already be destroyed, so do not dereference this field blindly
        GLContext* mCreatorContext;
        /// VAOs are not shared between contexts, nor even could be destroyed using the wrong context.
        uint32 mVAO;
        bool mNeedsUpdate;

        std::vector<std::pair<uint32, HardwareVertexBuffer*> > mAttribsBound;
        std::vector<uint32> mInstanceAttribsBound;
        size_t mVertexStart;

        void notifyChanged() override { mNeedsUpdate = true; }
    public:
        GLVertexArrayObject();
        ~GLVertexArrayObject();
        void notifyContextDestroyed(GLContext* context) { if(mCreatorContext == context) { mCreatorContext = 0; mVAO = 0; } }
        void bind(GLRenderSystemCommon* rs);
        bool needsUpdate(VertexBufferBinding* vertexBufferBinding, size_t vertexStart);
        void bindToGpu(GLRenderSystemCommon* rs, VertexBufferBinding* vertexBufferBinding, size_t vertexStart);
    };

}

#endif
