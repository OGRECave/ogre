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
#include "OgreGLVertexArrayObject.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreGLSLProgramCommon.h"
#include "OgreGLRenderSystemCommon.h"

namespace Ogre {
    GLVertexArrayObject::GLVertexArrayObject() : mVertexStart(0), mVAO(0) {
        mRenderSystem = static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem());
    }

    bool GLVertexArrayObject::needsUpdate(GLSLProgramCommon* program,
                                          VertexBufferBinding* vertexBufferBinding,
                                          size_t vertexStart)
    {
        VertexDeclaration::VertexElementList::const_iterator elemIter, elemEnd;
        elemEnd = mElementList.end();

        for (elemIter = mElementList.begin(); elemIter != elemEnd; ++elemIter)
        {
            const VertexElement & elem = *elemIter;

            uint16 source = elem.getSource();

            if (!vertexBufferBinding->isBufferBound(source))
                continue; // Skip unbound elements

            VertexElementSemantic sem = elem.getSemantic();
            unsigned short elemIndex = elem.getIndex();

            if (!program->isAttributeValid(sem, elemIndex))
                continue; // Skip unused elements

            uint32 attrib = (uint32)program->getAttributeIndex(sem, elemIndex);

            const HardwareVertexBufferSharedPtr& vertexBuffer = vertexBufferBinding->getBuffer(source);
            AttribBinding binding = {attrib, sem, vertexBuffer.get()};
            if (std::find(mAttribsBound.begin(), mAttribsBound.end(), binding) ==
                mAttribsBound.end())
                return true;

            if (vertexBuffer->getIsInstanceData() &&
                std::find(mInstanceAttribsBound.begin(), mInstanceAttribsBound.end(), attrib) ==
                    mInstanceAttribsBound.end())
                return true;
        }

        if(vertexStart != mVertexStart) {
            return true;
        }

        return false;
    }

    void GLVertexArrayObject::bindToShader(GLSLProgramCommon* program,
                                           VertexBufferBinding* vertexBufferBinding,
                                           size_t vertexStart)
    {
        mAttribsBound.clear();
        mInstanceAttribsBound.clear();

        VertexDeclaration::VertexElementList::const_iterator elemIter, elemEnd;
        elemEnd = mElementList.end();

        for (elemIter = mElementList.begin(); elemIter != elemEnd; ++elemIter)
        {
            const VertexElement& elem = *elemIter;

            uint16 source = elem.getSource();

            if (!vertexBufferBinding->isBufferBound(source))
                continue; // Skip unbound elements

            VertexElementSemantic sem = elem.getSemantic();
            unsigned short elemIndex = elem.getIndex();

            if (!program->isAttributeValid(sem, elemIndex))
                continue; // Skip unused elements

            uint32 attrib = (uint32)program->getAttributeIndex(sem, elemIndex);

            const HardwareVertexBufferSharedPtr& vertexBuffer = vertexBufferBinding->getBuffer(source);

            AttribBinding binding = {attrib, sem, vertexBuffer.get()};
            mAttribsBound.push_back(binding);

            mRenderSystem->bindVertexElementToGpu(elem, vertexBuffer, vertexStart, program);

            if (vertexBuffer->getIsInstanceData())
                mInstanceAttribsBound.push_back(attrib);
        }

        mVertexStart = vertexStart;
    }
}
