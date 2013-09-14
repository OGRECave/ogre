/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

  Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLProgramPipelineManager.h"
#include "OgreStringConverter.h"

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
        //TODO Add other RenderOperation allowed when no GS present.
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "GL RenderToVertexBuffer"
                        "can only output point lists, line lists, or triangle lists",
                        "OgreGL3PlusRenderToVertexBuffer::getR2VBPrimitiveType");
        }
    }
    //-----------------------------------------------------------------------------
    static GLint getVertexCountPerPrimitive(RenderOperation::OperationType operationType)
    {
        // We can only get points, lines or triangles since they are the only
        // legal R2VB output primitive types.
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            return 1;
        case RenderOperation::OT_LINE_LIST:
            return 2;
        default:
        case RenderOperation::OT_TRIANGLE_LIST:
            return 3;
        }
    }
    //-----------------------------------------------------------------------------
    GL3PlusRenderToVertexBuffer::GL3PlusRenderToVertexBuffer()
        : mFrontBufferIndex(-1)
        , mFirstUpdate(true)
    {
        mVertexBuffers[0].setNull();
        mVertexBuffers[1].setNull();

        // Create query objects.
        OGRE_CHECK_GL_ERROR(glGenQueries(1, &mPrimitivesDrawnQuery));

        // GL4+
        // glGenTransformFeedbacks(1, mFeedbackObject);
    }
    //-----------------------------------------------------------------------------
    GL3PlusRenderToVertexBuffer::~GL3PlusRenderToVertexBuffer()
    {
        OGRE_CHECK_GL_ERROR(glDeleteQueries(1, &mPrimitivesDrawnQuery));
    }
    //-----------------------------------------------------------------------------
    void GL3PlusRenderToVertexBuffer::getRenderOperation(RenderOperation& op)
    {
        op.operationType = mOperationType;
        op.useIndexes = false;
        op.vertexData = mVertexData;
    }
    //-----------------------------------------------------------------------------
    // void GL3PlusRenderToVertexBuffer::update(SceneManager* sceneMgr)
    // {
    //     size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
    //     if (mVertexBuffers[0].isNull() || mVertexBuffers[0]->getSizeInBytes() != bufSize)
    //     {
    //         // Buffers don't match. Need to reallocate.
    //         mResetRequested = true;
    //     }

    //     // Single pass only for now.
    //     Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);

    //     // Set pass before binding buffers to activate the GPU programs.
    //     sceneMgr->_setPass(r2vbPass);
    //     if (mFirstUpdate)
    //     {
    //         bindVerticesOutput(r2vbPass);
    //         mFirstUpdate = false;
    //     }

    //     RenderOperation renderOp;
    //     size_t targetBufferIndex;
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
    //         targetBufferIndex = 1 - mFrontBufferIndex;
    //     }

    //     if (mVertexBuffers[targetBufferIndex].isNull() ||
    //         mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
    //     {
    //         reallocateBuffer(targetBufferIndex);
    //     }

    //     // Disable rasterization.
    //     OGRE_CHECK_GL_ERROR(glEnable(GL_RASTERIZER_DISCARD));

    //     // Bind vertex array object.
    //     if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
    //     {
    //         GLSLProgramPipeline* programPipeline =
    //             GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
    //         programPipeline->getVertexArrayObject()->bind();
    //     }
    //     else
    //     {
    //         GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
    //         linkProgram->getVertexArrayObject()->bind();
    //     }
    //     //TODO now bind vertex buffer 
    //     GL3PlusHardwareVertexBuffer* vertexBuffer = static_cast<GL3PlusHardwareVertexBuffer*>(mVertexBuffers[targetBufferIndex].getPointer());
    //     OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->getGLBufferId()));
    //     float buffer[3] = {0, 1, 2};
    //     GLvoid* pBuffer = buffer;
    //     OGRE_CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, pBuffer));
    //     OGRE_CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    //     // // Disable rasterization.
    //     // //TODO is this second disabling of rasterization needed?
    //     // OGRE_CHECK_GL_ERROR(glEnable(GL_RASTERIZER_DISCARD));



    //     // Bind shader parameters.
    //     RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
    //     targetRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
    //     targetRenderSystem->_setViewMatrix(Matrix4::IDENTITY);
    //     targetRenderSystem->_setProjectionMatrix(Matrix4::IDENTITY);
    //     if (r2vbPass->hasVertexProgram())
    //     {
    //         targetRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM,
    //                                                      r2vbPass->getVertexProgramParameters(), GPV_ALL);
    //     }
    //     if (r2vbPass->hasFragmentProgram())
    //     {
    //         targetRenderSystem->bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM,
    //                                                      r2vbPass->getFragmentProgramParameters(), GPV_ALL);
    //     }
    //     if (r2vbPass->hasGeometryProgram())
    //     {
    //         targetRenderSystem->bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM,
    //                                                      r2vbPass->getGeometryProgramParameters(), GPV_ALL);
    //     }
    //     //TODO add tessellation stages

    //     // Bind the target buffer.
    //     OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vertexBuffer->getGLBufferId()));

    //     // 'Render' data to the transform buffer.
    //     OGRE_CHECK_GL_ERROR(glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mPrimitivesDrawnQuery));
    //     OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(getR2VBPrimitiveType(mOperationType)));
    //     // OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(GL_LINES));
    //     // OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(GL_TRIANGLES));

    //     // targetRenderSystem->_render(renderOp);
    //     OGRE_CHECK_GL_ERROR(glDrawArrays(GL_POINTS, 0, 1));
        
    //     //TODO GL 4+
    //     //glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mFeedbackObject);
    //     //glDrawTransformFeedback(getR2VBPrimitiveType(mOperationType), mFeedbackObject);

    //     OGRE_CHECK_GL_ERROR(glEndTransformFeedback());

    //     // Finish the query.
    //     OGRE_CHECK_GL_ERROR(glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN));

    //     // Read back query results.
    //     GLuint primitivesWritten;
    //     OGRE_CHECK_GL_ERROR(glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten));
    //     mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

    //     // Switch the vertex binding if necessary.
    //     if (targetBufferIndex != mFrontBufferIndex)
    //     {
    //         mVertexData->vertexBufferBinding->unsetAllBindings();
    //         mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
    //         mFrontBufferIndex = targetBufferIndex;
    //     }

    //     // Enable rasterization.
    //     OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));

    //     // Clear the reset flag.
    //     mResetRequested = false;
    // }
    void GL3PlusRenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        // Single pass only for now.
        Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);

        // Set pass before binding buffers to activate the GPU programs.
        sceneMgr->_setPass(r2vbPass);
        if (mFirstUpdate)
        {
            bindVerticesOutput(r2vbPass);
            mFirstUpdate = false;
        }

        size_t sourceBufferIndex = 0;
        size_t targetBufferIndex = 1;

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

        // Bind vertex array.
        OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VertexBuffer[targetBufferIndex]));
        OGRE_CHECK_GL_ERROR(glBindVertexArray(VertexArray[sourceBufferIndex]));

        // 'Render' data to the transform buffer.
        OGRE_CHECK_GL_ERROR(glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mPrimitivesDrawnQuery));
        OGRE_CHECK_GL_ERROR(glBeginTransformFeedback(GL_POINTS));

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
        targetRenderSystem->_render(renderOp);
        // OGRE_CHECK_GL_ERROR(glDrawArrays(GL_POINTS, 0, 1));
        
        //TODO GL 4+
        //glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mFeedbackObject);
        //glDrawTransformFeedback(getR2VBPrimitiveType(mOperationType), mFeedbackObject);

        OGRE_CHECK_GL_ERROR(glEndTransformFeedback());

        // Finish the query.
        OGRE_CHECK_GL_ERROR(glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN));

        // Read back query results.
        GLuint primitivesWritten;
        OGRE_CHECK_GL_ERROR(glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten));
        mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

        // Enable rasterization.
        OGRE_CHECK_GL_ERROR(glDisable(GL_RASTERIZER_DISCARD));
    }
    //-----------------------------------------------------------------------------
    void GL3PlusRenderToVertexBuffer::reallocateBuffer(size_t index)
    {
        assert(index == 0 || index == 1);
        if (!mVertexBuffers[index].isNull())
        {
            mVertexBuffers[index].setNull();
        }

        mVertexBuffers[index] = HardwareBufferManager::getSingleton().createVertexBuffer(
            mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount,
            HardwareBuffer::HBU_DYNAMIC
// #if OGRE_DEBUG_MODE
//             // Allow reading the contents of the buffer in debug mode.
//             HardwareBuffer::HBU_DYNAMIC
// #else
//             HardwareBuffer::HBU_STATIC_WRITE_ONLY
// #endif
        );
    }
    //-----------------------------------------------------------------------------
    String GL3PlusRenderToVertexBuffer::getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index)
    {
        switch (semantic)
        {
        case VES_POSITION:
            return "gl_Position";
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
    //-----------------------------------------------------------------------------
    // void GL3PlusRenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    // {
    //     VertexDeclaration* declaration = mVertexData->vertexDeclaration;
    //     size_t elemCount = declaration->getElementCount();

    //     if (elemCount > 0)
    //     {
    //         GLuint linkProgramId = 0;
    //         // Have GLSL shaders, using varying attributes.
    //         if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
    //         {
    //             GLSLProgramPipeline* programPipeline =
    //                 GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
    //             // Linkprogramid = programPipeline->getGLProgramPipelineHandle();
    //             GLSLGpuProgram* glslGpuProgram = 0;
    //             if (glslGpuProgram = programPipeline->getGeometryProgram())
    //                 linkProgramId = glslGpuProgram->getGLSLProgram()->getGLProgramHandle();
    //             //TODO include tessellation stages
    //             else // vertex program
    //                 linkProgramId = programPipeline->getVertexProgram()->getGLSLProgram()->getGLProgramHandle();
    //         }
    //         else
    //         {
    //             GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
    //             linkProgramId = linkProgram->getGLProgramHandle();
    //         }

    //         // Note: 64 is the minimum number of interleaved
    //         // attributes allowed by GL_EXT_transform_feedback So we
    //         // are using it. Otherwise we could query during
    //         // rendersystem initialisation and use a dynamic sized
    //         // array.  But that would require C99. 
    //         // const GLchar *names[64];
    //         // const GLchar* names[1] = {"gl_Position"};//, "oUv0", "oUv1", "oUv2" };
    //         // elemCount = 1;
    //         const GLchar* names[1] = {
    //             "Position0"
    //             // "FireworkData.Pos"
    //             // "gl_Position" 
    //             // "outputTimer",
    //             // "outputType",
    //             // "outputVel"
    //         };
    //         elemCount = 1;
    //         //FIXME Shader variable names should be dynamically determined like below.
    //         //vector<const GLchar*>::type names;
    //         // std::vector<const GLchar*> names;
    //         // for (unsigned short e = 0; e < elemCount; e++)
    //         // {
    //         //     const VertexElement* element = declaration->getElement(e);
    //         //     String varyingName = getSemanticVaryingName(element->getSemantic(), element->getIndex());
    //         //     names.push_back(varyingName.c_str());
    //         // }

    //         // Store the output in a buffer.  The buffer has the same
    //         // structure as the shader output vertex data.
    //         // OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(linkProgramId, elemCount, names, GL_INTERLEAVED_ATTRIBS));
    //         OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(linkProgramId, elemCount, &names[0], GL_INTERLEAVED_ATTRIBS));

    //         if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
    //         {
    //             GLSLProgramPipeline* programPipeline =
    //                 GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
    //             programPipeline->activate();
    //         }
    //         else
    //         {
    //             OGRE_CHECK_GL_ERROR(glLinkProgram(linkProgramId));
    //         }

    //         // Check if program linking was successful.
    //         GLint didLink = 0;
    //         OGRE_CHECK_GL_ERROR(glGetProgramiv(linkProgramId, GL_LINK_STATUS, &didLink));
    //         logObjectInfo(String("RVB GLSL link result : "), linkProgramId);
    //         if (glIsProgram(linkProgramId))
    //         {
    //             glValidateProgram(linkProgramId);
    //         }
    //         logObjectInfo(String("RVB GLSL validation result : "), linkProgramId);

    //         // glGetTranformFeedbackVarying seems broken in AMD drivers,
    //         // returning a type value 0 no matter the actual type.
    //         // Not sure about Nvidia.
    //         GLchar Name[64];
    //         GLsizei Length(0);
    //         GLsizei Size(0);
    //         GLenum Type(0);
    //         OGRE_CHECK_GL_ERROR(glGetTransformFeedbackVarying(
    //             linkProgramId, 0, 64, &Length, &Size, &Type, Name
    //         ));
    //         std::cout << "Varying: " << Name <<" "<< Length <<" "<< Size <<" "<< Type << std::endl;
    //         bool Validated(false);
    //         Validated = (Size == 1) && (Type == GL_FLOAT_VEC4);
    //         std::cout << Validated << " " << GL_FLOAT_VEC4 << std::endl;
    //     }
    // }
    void GL3PlusRenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    {
        // VertexDeclaration* declaration = mVertexData->vertexDeclaration;
        // size_t elemCount = declaration->getElementCount();

        // if (elemCount == 0)
        //     return;

        // Get program object ID.
        GLuint linkProgramId = 0;
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLProgramPipeline* programPipeline =
                GLSLProgramPipelineManager::getSingleton().getCurrentProgramPipeline();
            GLSLGpuProgram* glslGpuProgram = 0;
            if (glslGpuProgram = programPipeline->getGeometryProgram())
                linkProgramId = glslGpuProgram->getGLSLProgram()->getGLProgramHandle();
            else // vertex program
                linkProgramId = programPipeline->getVertexProgram()->getGLSLProgram()->getGLProgramHandle();
        }
        else
        {
            GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            linkProgramId = linkProgram->getGLProgramHandle();
        }

        size_t sourceBufferIndex = 0;
        size_t targetBufferIndex = 1;
        // Bind vertex array object.
        OGRE_CHECK_GL_ERROR(glGenVertexArrays(2, &VertexArray[0]));
        // Bind and fill vertex buffers.
        float bufferData[3] = {0.0, 1.0, 2.0};
        float * buffer = bufferData;
        OGRE_CHECK_GL_ERROR(glGenBuffers(2, &VertexBuffer[0]));
        for (int i = 0; i < 2; i++)
        {
            OGRE_CHECK_GL_ERROR(glBindVertexArray(VertexArray[i]));
            //glBindTransformFeedback
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer[i]));
            OGRE_CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(buffer)*(i+1), buffer, GL_DYNAMIC_DRAW));
            OGRE_CHECK_GL_ERROR(glEnableVertexAttribArray(0));
            OGRE_CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(buffer)*(i+1), 0));
            OGRE_CHECK_GL_ERROR(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VertexBuffer[i]));
        }
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

        // Store the output in a buffer.  The buffer has the same
        // structure as the shader output vertex data.
        const GLchar* names[1] = {
            "Position0"
        };
        size_t elemCount = 1;
        OGRE_CHECK_GL_ERROR(glTransformFeedbackVaryings(linkProgramId, elemCount, names, GL_INTERLEAVED_ATTRIBS));

        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLProgramPipeline* programPipeline =
                GLSLProgramPipelineManager::getSingleton().getCurrentProgramPipeline();
            programPipeline->activate();
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glLinkProgram(linkProgramId));
        }

        // Check if program linking was successful.
        GLint didLink = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(linkProgramId, GL_LINK_STATUS, &didLink));
        logObjectInfo(String("RVB GLSL link result : "), linkProgramId);
        if (glIsProgram(linkProgramId))
        {
            glValidateProgram(linkProgramId);
        }
        logObjectInfo(String("RVB GLSL validation result : "), linkProgramId);

        GLchar Name[64];
        GLsizei Length(0);
        GLsizei Size(0);
        GLenum Type(0);
        OGRE_CHECK_GL_ERROR(glGetTransformFeedbackVarying(
            linkProgramId, 0, 64, &Length, &Size, &Type, Name
        ));
        std::cout << "Varying: " << Name <<" "<< Length <<" "<< Size <<" "<< Type << std::endl;
        bool Validated(false);
        Validated = (Size == 1) && (Type == GL_FLOAT_VEC4);
        std::cout << Validated << " " << GL_FLOAT_VEC4 << std::endl;
    }
}
