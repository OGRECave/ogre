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

#include "OgreGLRenderToVertexBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreGLHardwareBuffer.h"
#include "OgreRenderable.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreLogManager.h"

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
                "OgreGLRenderToVertexBuffer::getR2VBPrimitiveType");
        }
    }
//-----------------------------------------------------------------------------
    static void checkGLError(bool logError, bool throwException,
        const Ogre::String& sectionName = BLANKSTRING)
    {
        String msg;
        bool foundError = false;

        // get all the GL errors
        GLenum glErr = glGetError();
        while (glErr != GL_NO_ERROR)
        {
            msg += glErrorToString(glErr);
            glErr = glGetError();
            foundError = true;  
        }

        if (foundError && (logError || throwException))
        {
            String fullErrorMessage = "GL Error : " + msg + " in " + sectionName;
            if (logError)
            {
                LogManager::getSingleton().getDefaultLog()->logMessage(fullErrorMessage, LML_CRITICAL);
            }
            if (throwException)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    fullErrorMessage, "OgreGLRenderToVertexBuffer");
            }
        }
    }
//-----------------------------------------------------------------------------
    GLRenderToVertexBuffer::GLRenderToVertexBuffer()
    {
        mVertexBuffers[0].reset();
        mVertexBuffers[1].reset();

         // create query objects
        glGenQueries(1, &mPrimitivesDrawnQuery);
    }
//-----------------------------------------------------------------------------
    GLRenderToVertexBuffer::~GLRenderToVertexBuffer()
    {
        glDeleteQueries(1, &mPrimitivesDrawnQuery);
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
    void GLRenderToVertexBuffer::update(SceneManager* sceneMgr)
    {
        checkGLError(true, false, "start of GLRenderToVertexBuffer::update");

        size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
        if (!mVertexBuffers[0] || mVertexBuffers[0]->getSizeInBytes() != bufSize)
        {
            //Buffers don't match. Need to reallocate.
            mResetRequested = true;
        }

        Ogre::Pass* r2vbPass = derivePass(sceneMgr);
        
        checkGLError(true, false);

        bindVerticesOutput(r2vbPass);

        RenderOperation renderOp;
        auto targetBufferIndex = mTargetBufferIndex;
        if (mResetRequested || mResetsEveryUpdate)
        {
            //Use source data to render to first buffer
            mSourceRenderable->getRenderOperation(renderOp);
        }
        else
        {
            //Use current front buffer to render to back buffer
            this->getRenderOperation(renderOp);
        }

        if (!mVertexBuffers[targetBufferIndex] || 
            mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
        {
            reallocateBuffer(targetBufferIndex);
        }

        GLHardwareBuffer* vertexBuffer = mVertexBuffers[targetBufferIndex]->_getImpl<GLHardwareBuffer>();
        GLuint bufferId = vertexBuffer->getGLBufferId();

        //Bind the target buffer
        glBindBufferOffsetNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, bufferId, 0);

        glBeginTransformFeedbackNV(getR2VBPrimitiveType(mOperationType));

        glEnable(GL_RASTERIZER_DISCARD_NV);    // disable rasterization

        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV, mPrimitivesDrawnQuery);

        GLRenderSystem* targetRenderSystem = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        //Draw the object
        targetRenderSystem->setWorldMatrix(Matrix4::IDENTITY);
        targetRenderSystem->setViewMatrix(Matrix4::IDENTITY);
        targetRenderSystem->setProjectionMatrix(Matrix4::IDENTITY);
        targetRenderSystem->_render(renderOp);
        
        //Finish the query
        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV);
        glDisable(GL_RASTERIZER_DISCARD_NV);
        glEndTransformFeedbackNV();

        //read back query results
        GLuint primitivesWritten;
        glGetQueryObjectuiv(mPrimitivesDrawnQuery, GL_QUERY_RESULT, &primitivesWritten);
        mVertexData->vertexCount = primitivesWritten * getVertexCountPerPrimitive(mOperationType);

        checkGLError(true, true, "GLRenderToVertexBuffer::update");

        //Switch the vertex binding
        mVertexData->vertexBufferBinding->unsetAllBindings();
        mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
        mTargetBufferIndex = mTargetBufferIndex == 0 ? 1 : 0;

        glDisable(GL_RASTERIZER_DISCARD_NV);    // enable rasterization

        //Clear the reset flag
        mResetRequested = false;
    }
//-----------------------------------------------------------------------------
    GLint GLRenderToVertexBuffer::getGLSemanticType(VertexElementSemantic semantic)
    {
        switch (semantic)
        {
        case VES_POSITION:
            return GL_POSITION;
        case VES_TEXTURE_COORDINATES:
            return GL_TEXTURE_COORD_NV;
        case VES_DIFFUSE:
            return GL_PRIMARY_COLOR;
        case VES_SPECULAR:
            return GL_SECONDARY_COLOR_NV;
        //TODO : Implement more?
        default:
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Unsupported vertex element semantic in render to vertex buffer",
                "OgreGLRenderToVertexBuffer::getGLSemanticType");
            
        }
    }
//-----------------------------------------------------------------------------
    void GLRenderToVertexBuffer::bindVerticesOutput(Pass* pass)
    {
        VertexDeclaration* declaration = mVertexData->vertexDeclaration;
        bool useVaryingAttributes = false;
        
        //Check if we are FixedFunc/ASM shaders (Static attributes) or GLSL (Varying attributes)
        //We assume that there isn't a mix of GLSL and ASM as this is illegal
        GpuProgram* sampleProgram = 0;
        if (pass->hasVertexProgram())
        {
            sampleProgram = pass->getVertexProgram().get();
        }
        else if (pass->hasGeometryProgram())
        {
            sampleProgram = pass->getGeometryProgram().get();
        }
        if (sampleProgram && (sampleProgram->_getBindingDelegate()->getLanguage() == "glsl"))
        {
            useVaryingAttributes = true;
        }

        if (useVaryingAttributes)
        {
            //Have GLSL shaders, using varying attributes
            GLSL::GLSLLinkProgram* linkProgram = GLSL::GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            uint linkProgramId = linkProgram->getGLHandle();
            
            std::vector<GLint> locations;
            for (unsigned short e=0; e < declaration->getElementCount(); e++)
            {
                const VertexElement* element =declaration->getElement(e);
                String varyingName = getSemanticVaryingName(element->getSemantic(), element->getIndex());
                GLint location = glGetVaryingLocationNV(linkProgramId, varyingName.c_str());
                if (location < 0)
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "GLSL link program does not output " + varyingName + 
                        " so it cannot fill the requested vertex buffer", 
                        "OgreGLRenderToVertexBuffer::bindVerticesOutput");
                }
                locations.push_back(location);
            }
            glTransformFeedbackVaryingsNV(
                linkProgramId, static_cast<GLsizei>(locations.size()), 
                &locations[0], GL_INTERLEAVED_ATTRIBS_NV);
        }
        else
        {
            //Either fixed function or assembly (CG = assembly) shaders
            std::vector<GLint> attribs;
            for (unsigned short e=0; e < declaration->getElementCount(); e++)
            {
                const VertexElement* element = declaration->getElement(e);
                //Type
                attribs.push_back(getGLSemanticType(element->getSemantic()));
                //Number of components
                attribs.push_back(VertexElement::getTypeCount(element->getType()));
                //Index
                attribs.push_back(element->getIndex());
            }
            
            glTransformFeedbackAttribsNV(
                static_cast<GLuint>(declaration->getElementCount()), 
                &attribs[0], GL_INTERLEAVED_ATTRIBS_NV);
        }

        checkGLError(true, true, "GLRenderToVertexBuffer::bindVerticesOutput");
    }
}
