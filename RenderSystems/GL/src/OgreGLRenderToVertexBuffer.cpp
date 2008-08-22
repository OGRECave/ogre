/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreGLRenderToVertexBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreGLHardwareVertexBuffer.h"
#include "OgreRenderable.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreGLSLLinkProgramManager.h"
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
		default:
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "GL RenderToVertexBuffer"
				"can only output point lists, line lists, or triangle lists",
				"OgreGLRenderToVertexBuffer::getR2VBPrimitiveType");
		}
	}
//-----------------------------------------------------------------------------
	static GLint getVertexCountPerPrimitive(RenderOperation::OperationType operationType)
	{
		//We can only get points, lines or triangles since they are the only
		//legal R2VB output primitive types
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
	void checkGLError(bool logError, bool throwException, 
		const Ogre::String& sectionName = StringUtil::BLANK)
	{
		String msg;
		bool foundError = false;

		// get all the GL errors
		GLenum glErr = glGetError();
		while (glErr != GL_NO_ERROR)
        {
			const char* glerrStr = (const char*)gluErrorString(glErr);
			if (glerrStr)
			{
				msg += String(glerrStr);
			}
			glErr = glGetError();
			foundError = true;	
        }

		if (foundError && (logError || throwException))
		{
			String fullErrorMessage = "GL Error : " + msg + " in " + sectionName;
			if (logError)
			{
				LogManager::getSingleton().getDefaultLog()->logMessage(fullErrorMessage);
			}
			if (throwException)
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					fullErrorMessage, "OgreGLRenderToVertexBuffer");
			}
		}
	}
//-----------------------------------------------------------------------------
	GLRenderToVertexBuffer::GLRenderToVertexBuffer() : mFrontBufferIndex(-1)
	{
		mVertexBuffers[0].setNull();
		mVertexBuffers[1].setNull();

		 // create query objects
		glGenQueries(1, &mPrimitivesDrawnQuery);
	}
//-----------------------------------------------------------------------------
	GLRenderToVertexBuffer::~GLRenderToVertexBuffer()
	{
		glDeleteQueries(1, &mPrimitivesDrawnQuery);
	}
//-----------------------------------------------------------------------------
	void GLRenderToVertexBuffer::getRenderOperation(RenderOperation& op)
	{
		op.operationType = mOperationType;
		op.useIndexes = false;
		op.vertexData = mVertexData;
	}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	void GLRenderToVertexBuffer::update(SceneManager* sceneMgr)
	{
		checkGLError(true, false, "start of GLRenderToVertexBuffer::update");

		size_t bufSize = mVertexData->vertexDeclaration->getVertexSize(0) * mMaxVertexCount;
		if (mVertexBuffers[0].isNull() || mVertexBuffers[0]->getSizeInBytes() != bufSize)
		{
			//Buffers don't match. Need to reallocate.
			mResetRequested = true;
		}
		
		//Single pass only for now
		Ogre::Pass* r2vbPass = mMaterial->getBestTechnique()->getPass(0);
		//Set pass before binding buffers to activate the GPU programs
		sceneMgr->_setPass(r2vbPass);
		
		//TODO : We get a GL error after setPass (related to glTexEnvi), but 
		//everything works fine. Understand why and solve.
		checkGLError(false, false);

		bindVerticesOutput(r2vbPass);

		RenderOperation renderOp;
		size_t targetBufferIndex;
		if (mResetRequested || mResetsEveryUpdate)
		{
			//Use source data to render to first buffer
			mSourceRenderable->getRenderOperation(renderOp);
			targetBufferIndex = 0;
		}
		else
		{
			//Use current front buffer to render to back buffer
			this->getRenderOperation(renderOp);
			targetBufferIndex = 1 - mFrontBufferIndex;
		}

		if (mVertexBuffers[targetBufferIndex].isNull() || 
			mVertexBuffers[targetBufferIndex]->getSizeInBytes() != bufSize)
		{
			reallocateBuffer(targetBufferIndex);
		}

		GLHardwareVertexBuffer* vertexBuffer = static_cast<GLHardwareVertexBuffer*>(mVertexBuffers[targetBufferIndex].getPointer());
		GLuint bufferId = vertexBuffer->getGLBufferId();

		//Bind the target buffer
		glBindBufferOffsetNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, bufferId, 0);

		glBeginTransformFeedbackNV(getR2VBPrimitiveType(mOperationType));

		glEnable(GL_RASTERIZER_DISCARD_NV);    // disable rasterization

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV, mPrimitivesDrawnQuery);

		RenderSystem* targetRenderSystem = Root::getSingleton().getRenderSystem();
		//Draw the object
		targetRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
		targetRenderSystem->_setViewMatrix(Matrix4::IDENTITY);
		targetRenderSystem->_setProjectionMatrix(Matrix4::IDENTITY);
		if (r2vbPass->hasVertexProgram())
		{
			targetRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM, 
				r2vbPass->getVertexProgramParameters());
		}
		if (r2vbPass->hasGeometryProgram())
		{
			targetRenderSystem->bindGpuProgramParameters(GPT_GEOMETRY_PROGRAM,
				r2vbPass->getGeometryProgramParameters());
		}
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

		//Switch the vertex binding if neccesary
		if (targetBufferIndex != mFrontBufferIndex)
		{
			mVertexData->vertexBufferBinding->unsetAllBindings();
			mVertexData->vertexBufferBinding->setBinding(0, mVertexBuffers[targetBufferIndex]);
			mFrontBufferIndex = targetBufferIndex;
		}

		glDisable(GL_RASTERIZER_DISCARD_NV);    // enable rasterization

		//Clear the reset flag
		mResetRequested = false;
	}
//-----------------------------------------------------------------------------
	void GLRenderToVertexBuffer::reallocateBuffer(size_t index)
	{
		assert(index == 0 || index == 1);
		if (!mVertexBuffers[index].isNull())
		{
			mVertexBuffers[index].setNull();
		}
		
		mVertexBuffers[index] = HardwareBufferManager::getSingleton().createVertexBuffer(
			mVertexData->vertexDeclaration->getVertexSize(0), mMaxVertexCount, 
#if OGRE_DEBUG_MODE
			//Allow to read the contents of the buffer in debug mode
			HardwareBuffer::HBU_DYNAMIC
#else
			HardwareBuffer::HBU_STATIC_WRITE_ONLY
#endif
			);
	}
//-----------------------------------------------------------------------------
	String GLRenderToVertexBuffer::getSemanticVaryingName(VertexElementSemantic semantic, unsigned short index)
	{
		switch (semantic)
		{
		case VES_POSITION:
			return "gl_Position";
		case VES_TEXTURE_COORDINATES:
			return String("gl_TexCoord[") + StringConverter::toString(index) + "]";
		case VES_DIFFUSE:
			return "gl_FrontColor";
		case VES_SPECULAR:
			return "gl_FrontSecondaryColor";
		//TODO : Implement more?
		default:
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unsupported vertex element sematic in render to vertex buffer", 
				"OgreGLRenderToVertexBuffer::getSemanticVaryingName");
		}
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
				"Unsupported vertex element sematic in render to vertex buffer", 
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
			sampleProgram = pass->getVertexProgram().getPointer();
		}
		else if (pass->hasGeometryProgram())
		{
			sampleProgram = pass->getGeometryProgram().getPointer();
		}
		if ((sampleProgram != 0) && (sampleProgram->getLanguage() == "glsl"))
		{
			useVaryingAttributes = true;
		}

		if (useVaryingAttributes)
		{
			//Have GLSL shaders, using varying attributes
			GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
			GLhandleARB linkProgramId = linkProgram->getGLHandle();
			
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