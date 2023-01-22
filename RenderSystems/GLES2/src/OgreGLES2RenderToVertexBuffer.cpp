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

#include "OgreGLES2RenderToVertexBuffer.h"

#include "OgreHardwareBufferManager.h"
#include "OgreGLES2HardwareBuffer.h"
#include "OgreRenderable.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLSLESProgramManager.h"

namespace Ogre {
//-----------------------------------------------------------------------------
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
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "GL RenderToVertexBuffer"
                "can only output point lists, line lists, or triangle lists",
                "OgreGLES2RenderToVertexBuffer::getR2VBPrimitiveType");
        }
    }
//-----------------------------------------------------------------------------
    GLES2RenderToVertexBuffer::GLES2RenderToVertexBuffer()
    {
        mVertexBuffers[0].reset();
        mVertexBuffers[1].reset();

        // Create query objects
        OGRE_CHECK_GL_ERROR(glGenQueries(1, &mPrimitivesDrawnQuery));
    }
//-----------------------------------------------------------------------------
    GLES2RenderToVertexBuffer::~GLES2RenderToVertexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteQueries(1, &mPrimitivesDrawnQuery));
    }
//-----------------------------------------------------------------------------
    void GLES2RenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
        if (!mVertexBuffers[0] || mVertexBuffers[0]->getSizeInBytes() != bufSize)
        {
            // Buffers don't match. Need to reallocate.
            mResetRequested = true;
        }

        Ogre::Pass* r2vbPass = derivePass(sceneMgr);

        bindVerticesOutput(r2vbPass);

        RenderOperation renderOp;
        auto targetBufferIndex = mTargetBufferIndex;
        if (mResetRequested || mResetsEveryUpdate)
        {
            // Use source data to render to first buffer
            mSourceRenderable->getRenderOperation(renderOp);
        }
        else
        {
            // Use current front buffer to render to back buffer
            this->getRenderOperation(renderOp);
        }

        if (!mVertexBuffers[targetBufferIndex] || 
            mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
        {
            reallocateBuffer(targetBufferIndex);
        }

        auto vertexBuffer = mVertexBuffers[targetBufferIndex]->_getImpl<GLES2HardwareBuffer>();
/*        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLESProgramPipeline* programPipeline =
                GLSLESProgramPipelineManager::getSingleton().getActiveProgramPipeline();
            programPipeline->getVertexArrayObject()->bind();
        }
        else
        {
            GLSLESLinkProgram* linkProgram = GLSLESLinkProgramManager::getSingleton().getActiveLinkProgram();
            linkProgram->getVertexArrayObject()->bind();
        }
        */
        // Bind the target buffer
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffer->getGLBufferId()));

        // Disable rasterization
        OGRE_CHECK_GL_ERROR(glEnable(GL_RASTERIZER_DISCARD));

        RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
        // Draw the object
        OGRE_CHECK_GL_ERROR(glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mPrimitivesDrawnQuery));

        OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(getR2VBPrimitiveType(mOperationType)));

        targetRenderSystem->_render(renderOp);
        
        OGRE_CHECK_GL_ERROR(glEndTransformFeedback());

        // Finish the query
        OGRE_CHECK_GL_ERROR(glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN));

        OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));

        // Read back query results
        GLuint primitivesWritten;
        OGRE_CHECK_GL_ERROR(glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten));
        mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

        // Switch the vertex binding
        mVertexData->vertexBufferBinding->unsetAllBindings();
        mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
        mTargetBufferIndex = mTargetBufferIndex == 0 ? 1 : 0;

        // Enable rasterization
        OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));

        // Clear the reset flag
        mResetRequested = false;
    }
//-----------------------------------------------------------------------------
    void GLES2RenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    {
        VertexDeclaration* declaration = mVertexData->vertexDeclaration;
        size_t elemCount = declaration->getElementCount();

        if (elemCount > 0)
        {
            GLuint linkProgramId = 0;
            // Have GLSL shaders, using varying attributes
            GLSLESProgramCommon* linkProgram = GLSLESProgramManager::getSingleton().getActiveProgram();
            linkProgramId = linkProgram->getGLProgramHandle();

            // Note: 64 is the minimum number of interleaved attributes allowed by GL_EXT_transform_feedback
            // So we are using it. Otherwise we could query during rendersystem initialisation and use a dynamic sized array.
            // But that would require C99.
            const GLchar *names[64];
            for (unsigned short e = 0; e < elemCount; e++)
            {
                const VertexElement* element = declaration->getElement(e);
                String varyingName = getSemanticVaryingName(element->getSemantic(), element->getIndex());
                names[e] = varyingName.c_str();
            }

            OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(linkProgramId, elemCount, names, GL_INTERLEAVED_ATTRIBS));
            OGRE_CHECK_GL_ERROR(glLinkProgram(linkProgramId));
        }
    }
}
