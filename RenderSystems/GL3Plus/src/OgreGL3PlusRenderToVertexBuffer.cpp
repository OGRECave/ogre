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

#include "OgreGL3PlusRenderToVertexBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRenderable.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLSLProgramManager.h"
#include "OgreGLSLShader.h"

namespace Ogre {

    GL3PlusRenderToVertexBuffer::GL3PlusRenderToVertexBuffer()
        : mTargetBufferIndex(0)
        , mFirstUpdate(true)
    {
        mVertexBuffers[0].reset();
        mVertexBuffers[1].reset();

        // Create query objects.
        OGRE_CHECK_GL_ERROR(glGenQueries(1, &mPrimitivesDrawnQuery));

        //TODO GL4+
        // glGenTransformFeedbacks(1, mFeedbackObject);
    }


    GL3PlusRenderToVertexBuffer::~GL3PlusRenderToVertexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteQueries(1, &mPrimitivesDrawnQuery));
    }


    static GLint getR2VBPrimitiveType(RenderOperation::OperationType operationType)
    {
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            return GL_POINTS;
        case RenderOperation::OT_LINE_LIST:
            return GL_LINES;
        case RenderOperation::OT_TRIANGLE_LIST:
            return GL_TRIANGLES;
            //TODO Add other RenderOperation allowed when no GS present.
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "GL RenderToVertexBuffer"
                        "can only output point lists, line lists, or triangle lists",
                        "OgreGL3PlusRenderToVertexBuffer::getR2VBPrimitiveType");
        }
    }

    void GL3PlusRenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    {
        VertexDeclaration* declaration = mVertexData->vertexDeclaration;
        size_t elemCount = declaration->getElementCount();

        if (elemCount == 0)
            return;

        //TODO GL4+ glBindTransformFeedback

        // Dynamically determine shader output variable names.
        std::vector<String> nameStrings;

        for (uint e = 0; e < elemCount; e++)
        {
            const VertexElement* element = declaration->getElement(e);
            String name = getSemanticVaryingName(element->getSemantic(), element->getIndex());
            nameStrings.push_back(name);
        }

        GLSLProgram* program = GLSLProgramManager::getSingleton().getActiveProgram();

        program->setTransformFeedbackVaryings(nameStrings);
    }


    void GL3PlusRenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        //     size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
        //     if (!mVertexBuffers[0] || mVertexBuffers[0]->getSizeInBytes() != bufSize)
        //     {
        //         // Buffers don't match. Need to reallocate.
        //         mResetRequested = true;
        //     }

        Ogre::Pass* r2vbPass = derivePass(sceneMgr);

        if (mFirstUpdate)
        {
            bindVerticesOutput(r2vbPass);
            mFirstUpdate = false;
        }

        if (!mVertexBuffers[mTargetBufferIndex] ||
            mVertexBuffers[mTargetBufferIndex]->getSizeInBytes() <
                mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount)
        {
            reallocateBuffer(mTargetBufferIndex);
        }

        // Disable rasterization.
        OGRE_CHECK_GL_ERROR(glEnable(GL_RASTERIZER_DISCARD));

        // Bind source vertex array + target tranform feedback buffer.
        auto targetVertexBuffer = mVertexBuffers[mTargetBufferIndex]->_getImpl<GL3PlusHardwareBuffer>();
        // OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VertexBuffer[mTargetBufferIndex]));
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, targetVertexBuffer->getGLBufferId()));
        // OGRE_CHECK_GL_ERROR(glBindVertexArray(VertexArray[mSourceBufferIndex]));

        GLSLProgramManager::getSingleton().getActiveProgram()->activate();

        // 'Render' data to the transform buffer.
        OGRE_CHECK_GL_ERROR(glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mPrimitivesDrawnQuery));
        OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(getR2VBPrimitiveType(mOperationType)));

        RenderOperation renderOp;
        if (mResetRequested || mResetsEveryUpdate)
        {
            // Use source data to render to first buffer.
            mSourceRenderable->getRenderOperation(renderOp);
        }
        else
        {
            // Use current front buffer to render to back buffer.
            this->getRenderOperation(renderOp);
        }

        RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
        targetRenderSystem->_render(renderOp);
        // OGRE_CHECK_GL_ERROR(glDrawArrays(GL_POINTS, 0, 1));

        //TODO GL4+
        //glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mFeedbackObject);
        //glDrawTransformFeedback(getR2VBPrimitiveType(mOperationType), mFeedbackObject);

        OGRE_CHECK_GL_ERROR(glEndTransformFeedback());
        OGRE_CHECK_GL_ERROR(glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN));

        // Read back query results.
        GLuint primitivesWritten;
        OGRE_CHECK_GL_ERROR(glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten));
        mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

        // Switch the vertex binding.
        mVertexData->vertexBufferBinding->unsetAllBindings();
        mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[mTargetBufferIndex]);
        mTargetBufferIndex = mTargetBufferIndex == 0 ? 1 : 0;

        // Enable rasterization.
        OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));

        // Clear the reset flag.
        mResetRequested = false;
    }
}
