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
#include "OgreGL3PlusHardwareVertexBuffer.h"
#include "OgreGL3PlusVertexArrayObject.h"
#include "OgreRenderable.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLSLMonolithicProgramManager.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLSeparableProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreTechnique.h"
#include <iostream>

namespace Ogre {
namespace v1 {
    GL3PlusRenderToVertexBuffer::GL3PlusRenderToVertexBuffer()
        : mTargetBufferIndex(0)
        , mFirstUpdate(true)
    {
        mVertexBuffers[0].setNull();
        mVertexBuffers[1].setNull();

        // Create query objects.
        OGRE_CHECK_GL_ERROR(glGenQueries(1, &mPrimitivesDrawnQuery));

        //TODO GL4+
        // glGenTransformFeedbacks(1, mFeedbackObject);
    }


    GL3PlusRenderToVertexBuffer::~GL3PlusRenderToVertexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteQueries(1, &mPrimitivesDrawnQuery));
    }


    static GLint getR2VBPrimitiveType(OperationType operationType)
    {
        switch (operationType)
        {
        case OT_POINT_LIST:
            return GL_POINTS;
        case OT_LINE_LIST:
            return GL_LINES;
        case OT_TRIANGLE_LIST:
            return GL_TRIANGLES;
            //TODO Add other RenderOperation allowed when no GS present.
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "GL RenderToVertexBuffer"
                        "can only output point lists, line lists, or triangle lists",
                        "OgreGL3PlusRenderToVertexBuffer::getR2VBPrimitiveType");
        }
    }


    static GLint getVertexCountPerPrimitive(OperationType operationType)
    {
        // We can only get points, lines or triangles since they are the only
        // legal R2VB output primitive types.
        switch (operationType)
        {
        case OT_POINT_LIST:
            return 1;
        case OT_LINE_LIST:
            return 2;
        default:
        case OT_TRIANGLE_LIST:
            return 3;
        }
    }


    void GL3PlusRenderToVertexBuffer::getRenderOperation(RenderOperation& op)
    {
        op.operationType = mOperationType;
        op.useIndexes = false;
        op.vertexData = mVertexData;
    }


    void GL3PlusRenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    {
        VertexDeclaration* declaration = mVertexData->vertexDeclaration;
        size_t elemCount = declaration->getElementCount();

        if (elemCount == 0)
            return;

        // Get program object ID.
        GLuint programId = 0;
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLSeparableProgram* separableProgram =
                GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
            GLSLShader* glslGpuProgram = 0;
            if ((glslGpuProgram = separableProgram->getGeometryShader()))
                programId = glslGpuProgram->getGLProgramHandle();
            //TODO include tessellation stages
            else // vertex program
                programId = separableProgram->getVertexShader()->getGLProgramHandle();
        }
        else
        {
            GLSLMonolithicProgram* monolithicProgram = GLSLMonolithicProgramManager::getSingleton().getActiveMonolithicProgram();
            programId = monolithicProgram->getGLProgramHandle();
        }

        // Store the output in a buffer.  The buffer has the same
        // structure as the shader output vertex data.
        // Note: 64 is the minimum number of interleaved
        // attributes allowed by GL_EXT_transform_feedback so we
        // are using it. Otherwise we could query during
        // rendersystem initialisation and use a dynamic sized
        // array.  But that would require C99.
        size_t sourceBufferIndex = mTargetBufferIndex == 0 ? 1 : 0;

        // Bind and fill vertex arrays + buffers.
        reallocateBuffer(sourceBufferIndex);
        reallocateBuffer(mTargetBufferIndex);
        // GL3PlusHardwareVertexBuffer* sourceVertexBuffer = static_cast<GL3PlusHardwareVertexBuffer*>(mVertexBuffers[mSourceBufferIndex].getPointer());
        // GL3PlusHardwareVertexBuffer* targetVertexBuffer = static_cast<GL3PlusHardwareVertexBuffer*>(mVertexBuffers[mTargetBufferIndex].getPointer());

        //TODO GL4+ glBindTransformFeedback

        // Dynamically determine shader output variable names.
        std::vector<String> nameStrings;
        std::vector<const GLchar*> names;
        for (uint e = 0; e < elemCount; e++)
        {
            const VertexElement* element = declaration->getElement(e);
            String name = getSemanticVaryingName(element->getSemantic(), element->getIndex());
            nameStrings.push_back(name);
        }

        // Convert to const char * for GL
        for (uint e = 0; e < elemCount; e++)
        {
            names.push_back(nameStrings[e].c_str());
        }

        //TODO replace glTransformFeedbackVaryings with in-shader specification (GL 4.4)
        OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(programId, elemCount, &names[0], GL_INTERLEAVED_ATTRIBS));

        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLSeparableProgram* separableProgram =
                GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
            separableProgram->activate();
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glLinkProgram(programId));
        }

#if OGRE_DEBUG_MODE
        // Check if program linking was successful.
        GLint didLink = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(programId, GL_LINK_STATUS, &didLink));
        logObjectInfo(String("RVB GLSL link result : "), programId);
        if (glIsProgram(programId))
        {
            glValidateProgram(programId);
        }
        logObjectInfo(String("RVB GLSL validation result : "), programId);

        // Check if varyings were successfully set.
        GLchar Name[64];
        GLsizei Length(0);
        GLsizei Size(0);
        GLenum Type(0);
        // bool Validated = false;
        for (size_t i = 0; i < elemCount; i++)
        {
            OGRE_CHECK_GL_ERROR(glGetTransformFeedbackVarying(
                programId, i, 64, &Length, &Size, &Type, Name
            ));
            std::cout << "Varying " << i << ": " << Name <<" "<< Length <<" "<< Size <<" "<< Type << std::endl;
            // Validated = (Size == 1) && (Type == GL_FLOAT_VEC3);
            // std::cout << Validated << " " << GL_FLOAT_VEC3 << std::endl;
        }
#endif
    }


    void GL3PlusRenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        //     size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
        //     if (mVertexBuffers[0].isNull() || mVertexBuffers[0]->getSizeInBytes() != bufSize)
        //     {
        //         // Buffers don't match. Need to reallocate.
        //         mResetRequested = true;
        //     }

        //     if (mResetRequested || mResetsEveryUpdate)
        //     {
        //         // Use source data to render to first buffer.
        //         mSourceRenderable->getRenderOperation(renderOp);
        //         targetBufferIndex = 0;
        //     }
        //     else
        //     {
        //         // Use current front buffer to render to back buffer.
        //         this->getRenderOperation(renderOp);
        //         targetBufferIndex = 1 - mSourceBufferIndex;
        //     }

        //     if (mVertexBuffers[targetBufferIndex].isNull() ||
        //         mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
        //     {
        //         reallocateBuffer(targetBufferIndex);
        //     }

        // Single pass only for now.
        Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);

        // Set pass before binding buffers to activate the GPU programs.
        //sceneMgr->_setPass(r2vbPass); TODO
        if (mFirstUpdate)
        {
            bindVerticesOutput(r2vbPass);
            mFirstUpdate = false;
        }

        // size_t targetBufferIndex = mSourceBufferIndex == 0 ? 0 : 1;

        // Disable rasterization.
        OGRE_CHECK_GL_ERROR(glEnable(GL_RASTERIZER_DISCARD));

        // Bind shader parameters.
        RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
        targetRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
        targetRenderSystem->_setViewMatrix(Matrix4::IDENTITY);
        targetRenderSystem->_setProjectionMatrix(Matrix4::IDENTITY);
        if (r2vbPass->hasVertexProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM,
                                                         r2vbPass->getVertexProgramParameters(), GPV_ALL);
        }
        if (r2vbPass->hasFragmentProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM,
                                                         r2vbPass->getFragmentProgramParameters(), GPV_ALL);
        }
        if (r2vbPass->hasGeometryProgram())
        {
            targetRenderSystem->bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM,
                                                         r2vbPass->getGeometryProgramParameters(), GPV_ALL);
        }
        //TODO add tessellation stages

        // Bind source vertex array + target tranform feedback buffer.
        GL3PlusHardwareVertexBuffer* targetVertexBuffer = static_cast<GL3PlusHardwareVertexBuffer*>(mVertexBuffers[mTargetBufferIndex].getPointer());
        // OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VertexBuffer[mTargetBufferIndex]));
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, targetVertexBuffer->getGLBufferId()));
        // OGRE_CHECK_GL_ERROR(glBindVertexArray(VertexArray[mSourceBufferIndex]));
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLSeparableProgram* separableProgram =
                GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
            separableProgram->activate();
            separableProgram->getVertexArrayObject()->bind();
        }
        else
        {
            GLSLMonolithicProgram* monolithicProgram = GLSLMonolithicProgramManager::getSingleton().getActiveMonolithicProgram();
            monolithicProgram->getVertexArrayObject()->bind();
        }

        // 'Render' data to the transform buffer.
        OGRE_CHECK_GL_ERROR(glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mPrimitivesDrawnQuery));
        OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(getR2VBPrimitiveType(mOperationType)));

        RenderOperation renderOp;
        if (mResetRequested || mResetsEveryUpdate)
        {
            // Use source data to render to first buffer.
            mSourceRenderable->getRenderOperation(renderOp, false);
        }
        else
        {
            // Use current front buffer to render to back buffer.
            this->getRenderOperation(renderOp);
        }
        renderOp.renderToVertexBuffer = true;
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


    void GL3PlusRenderToVertexBuffer::reallocateBuffer(size_t index)
    {
        assert(index == 0 || index == 1);
        if (!mVertexBuffers[index].isNull())
        {
            mVertexBuffers[index].setNull();
        }

        // Transform feedback buffer must be at least as large as the
        // number of output primitives. AMD drivers seem to prefer
        // that the array be at least one primitive larger than this.
        mVertexBuffers[index] = HardwareBufferManager::getSingleton().createVertexBuffer(
            mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount + 1,
#if OGRE_DEBUG_MODE
            // Allow reading the contents of the buffer in debug mode.
            HardwareBuffer::HBU_DYNAMIC
#else
            HardwareBuffer::HBU_STATIC_WRITE_ONLY
#endif
        );
    }


    String GL3PlusRenderToVertexBuffer::getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index)
    {
        switch (semantic)
        {
        case VES_POSITION:
            // Since type of gl_Position cannot be redefined, it is
            // better to use a custom variable name.
            // return "gl_Position";
            return "oPos";
        case VES_TEXTURE_COORDINATES:
            return String("oUv") + StringConverter::toString(index);
        case VES_DIFFUSE:
            return "oColour";
        case VES_SPECULAR:
            return "oSecColour";
            //TODO : Implement more?
        default:
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Unsupported vertex element sematic in render to vertex buffer",
                        "OgreGL3PlusRenderToVertexBuffer::getSemanticVaryingName");
        }
    }
}
}
