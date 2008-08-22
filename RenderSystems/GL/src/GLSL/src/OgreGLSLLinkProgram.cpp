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

#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreStringConverter.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreException.h"

namespace Ogre {

	//  a  builtin				custom attrib name
	// ----------------------------------------------
	//	0  gl_Vertex			vertex
	//  1  n/a					blendWeights		
	//	2  gl_Normal			normal
	//	3  gl_Color				colour
	//	4  gl_SecondaryColor	secondary_colour
	//	5  gl_FogCoord			fog_coord
	//  7  n/a					blendIndices
	//	8  gl_MultiTexCoord0	uv0
	//	9  gl_MultiTexCoord1	uv1
	//	10 gl_MultiTexCoord2	uv2
	//	11 gl_MultiTexCoord3	uv3
	//	12 gl_MultiTexCoord4	uv4
	//	13 gl_MultiTexCoord5	uv5
	//	14 gl_MultiTexCoord6	uv6, tangent
	//	15 gl_MultiTexCoord7	uv7, binormal
	GLSLLinkProgram::CustomAttribute GLSLLinkProgram::msCustomAttributes[] = {
		CustomAttribute("vertex", GLGpuProgram::getFixedAttributeIndex(VES_POSITION, 0)),
		CustomAttribute("blendWeights", GLGpuProgram::getFixedAttributeIndex(VES_BLEND_WEIGHTS, 0)),
		CustomAttribute("normal", GLGpuProgram::getFixedAttributeIndex(VES_NORMAL, 0)),
		CustomAttribute("colour", GLGpuProgram::getFixedAttributeIndex(VES_DIFFUSE, 0)),
		CustomAttribute("secondary_colour", GLGpuProgram::getFixedAttributeIndex(VES_SPECULAR, 0)),
		CustomAttribute("blendIndices", GLGpuProgram::getFixedAttributeIndex(VES_BLEND_INDICES, 0)),
		CustomAttribute("uv0", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 0)),
		CustomAttribute("uv1", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 1)),
		CustomAttribute("uv2", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 2)),
		CustomAttribute("uv3", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 3)),
		CustomAttribute("uv4", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 4)),
		CustomAttribute("uv5", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 5)),
		CustomAttribute("uv6", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 6)),
		CustomAttribute("uv7", GLGpuProgram::getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 7)),
		CustomAttribute("tangent", GLGpuProgram::getFixedAttributeIndex(VES_TANGENT, 0)),
		CustomAttribute("binormal", GLGpuProgram::getFixedAttributeIndex(VES_BINORMAL, 0)),
	};

	GLint getGLGeometryInputPrimitiveType(RenderOperation::OperationType operationType, bool requiresAdjacency)
	{
		switch (operationType)
		{
		case RenderOperation::OT_POINT_LIST:
			return GL_POINTS;
		case RenderOperation::OT_LINE_LIST:
		case RenderOperation::OT_LINE_STRIP:
			return requiresAdjacency ? GL_LINES_ADJACENCY_EXT : GL_LINES;
		default:
		case RenderOperation::OT_TRIANGLE_LIST:
		case RenderOperation::OT_TRIANGLE_STRIP:
		case RenderOperation::OT_TRIANGLE_FAN:
			return requiresAdjacency ? GL_TRIANGLES_ADJACENCY_EXT : GL_TRIANGLES;
		}
	}
	//-----------------------------------------------------------------------
	GLint getGLGeometryOutputPrimitiveType(RenderOperation::OperationType operationType)
	{
		switch (operationType)
		{
		case RenderOperation::OT_POINT_LIST:
			return GL_POINTS;
		case RenderOperation::OT_LINE_STRIP:
			return GL_LINE_STRIP;
		case RenderOperation::OT_TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		default:
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
				"Geometry shader output operation type can only be point list,"
				"line strip or triangle strip",
				"GLSLLinkProgram::activate");
		}
	}
	//-----------------------------------------------------------------------
	GLSLLinkProgram::GLSLLinkProgram(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* geometryProgram, GLSLGpuProgram* fragmentProgram)
        : mVertexProgram(vertexProgram)
		, mGeometryProgram(geometryProgram)
		, mFragmentProgram(fragmentProgram)
		, mUniformRefsBuilt(false)
        , mLinked(false)

	{
			//checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error prior to Creating GLSL Program Object", 0);
		    glGetError(); //Clean up the error. Otherwise will flood log.
		    mGLHandle = glCreateProgramObjectARB();
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error Creating GLSL Program Object", 0 );


			// tell shaders to attach themselves to the LinkProgram
			// let the shaders do the attaching since they may have several children to attach
			if (mVertexProgram)
			{
				mVertexProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
				setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
			}
			if (mGeometryProgram)
			{
				mGeometryProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
				//Don't set adjacency flag. We handle it internally and expose "false"
			}
			if (mFragmentProgram)
			{
				mFragmentProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
			}

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgram::~GLSLLinkProgram(void)
	{
		glDeleteObjectARB(mGLHandle);

	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::activate(void)
	{
		if (!mLinked)
		{
			// Some drivers (e.g. OS X on nvidia) incorrectly determine the attribute binding automatically
			// and end up aliasing existing built-ins. So avoid! Bind all possible attribs
			// the linker will resolve which ones were actually used

			size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
			for (size_t i = 0; i < numAttribs; ++i)
			{
				const CustomAttribute& a = msCustomAttributes[i];
				glBindAttribLocationARB(mGLHandle, a.attrib, a.name.c_str());
			}

			if (mGeometryProgram)
			{
				RenderOperation::OperationType inputOperationType = mGeometryProgram->getGLSLProgram()->getInputOperationType();
				glProgramParameteriEXT(mGLHandle,GL_GEOMETRY_INPUT_TYPE_EXT,
					getGLGeometryInputPrimitiveType(inputOperationType, mGeometryProgram->isAdjacencyInfoRequired()));
				
				RenderOperation::OperationType outputOperationType = mGeometryProgram->getGLSLProgram()->getOutputOperationType();
				switch (outputOperationType)
				{
				case RenderOperation::OT_POINT_LIST:
				case RenderOperation::OT_LINE_STRIP:
				case RenderOperation::OT_TRIANGLE_STRIP:
					break;
				
				}
				glProgramParameteriEXT(mGLHandle,GL_GEOMETRY_OUTPUT_TYPE_EXT,
					getGLGeometryOutputPrimitiveType(outputOperationType));

				glProgramParameteriEXT(mGLHandle,GL_GEOMETRY_VERTICES_OUT_EXT,
					mGeometryProgram->getGLSLProgram()->getMaxOutputVertices());
			}

			glLinkProgramARB( mGLHandle );
			glGetObjectParameterivARB( mGLHandle, GL_OBJECT_LINK_STATUS_ARB, &mLinked );
			// force logging and raise exception if not linked
			checkForGLSLError( "GLSLLinkProgram::Activate",
				"Error linking GLSL Program Object : ", mGLHandle, !mLinked, !mLinked );
			if(mLinked)
			{
				logObjectInfo( String("GLSL link result : "), mGLHandle );
				buildGLUniformReferences();
				extractAttributes();
			}

		}

		if (mLinked)
		{
			checkForGLSLError( "GLSLLinkProgram::Activate",
				"Error prior to using GLSL Program Object : ", mGLHandle, false, false);

		    glUseProgramObjectARB( mGLHandle );

			checkForGLSLError( "GLSLLinkProgram::Activate",
				"Error using GLSL Program Object : ", mGLHandle, false, false);
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::extractAttributes(void)
	{
		size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);

		for (size_t i = 0; i < numAttribs; ++i)
		{
			const CustomAttribute& a = msCustomAttributes[i];
			GLint attrib = glGetAttribLocationARB(mGLHandle, a.name.c_str());

			if (attrib != -1)
			{
				mValidAttributes.insert(a.attrib);
			}
		}
	}
	//-----------------------------------------------------------------------
	GLuint GLSLLinkProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		return GLGpuProgram::getFixedAttributeIndex(semantic, index);
	}
	//-----------------------------------------------------------------------
	bool GLSLLinkProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
	{
		return mValidAttributes.find(getAttributeIndex(semantic, index)) != mValidAttributes.end();
	}
	//-----------------------------------------------------------------------
	void GLSLLinkProgram::buildGLUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			const GpuConstantDefinitionMap* vertParams = 0;
			const GpuConstantDefinitionMap* fragParams = 0;
			const GpuConstantDefinitionMap* geomParams = 0;
			if (mVertexProgram)
			{
				vertParams = &(mVertexProgram->getGLSLProgram()->getConstantDefinitions().map);
			}
			if (mGeometryProgram)
			{
				geomParams = &(mGeometryProgram->getGLSLProgram()->getConstantDefinitions().map);
			}
			if (mFragmentProgram)
			{
				fragParams = &(mFragmentProgram->getGLSLProgram()->getConstantDefinitions().map);
			}

			GLSLLinkProgramManager::getSingleton().extractUniforms(
				mGLHandle, vertParams, geomParams, fragParams, mGLUniformReferences);

			mUniformRefsBuilt = true;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updateUniforms(GpuProgramParametersSharedPtr params, 
		GpuProgramType fromProgType)
	{
		// iterate through uniform reference list and update uniform values
		GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
		GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

		for (;currentUniform != endUniform; ++currentUniform)
		{
			// Only pull values from buffer it's supposed to be in (vertex or fragment)
			// This method will be called twice, once for vertex program params, 
			// and once for fragment program params.
			if (fromProgType == currentUniform->mSourceProgType)
			{
				const GpuConstantDefinition* def = currentUniform->mConstantDef;
				GLsizei glArraySize = (GLsizei)def->arraySize;

				// get the index in the parameter real list
				switch (def->constType)
				{
				case GCT_FLOAT1:
					glUniform1fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_FLOAT2:
					glUniform2fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_FLOAT3:
					glUniform3fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_FLOAT4:
					glUniform4fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_MATRIX_2X2:
					glUniformMatrix2fvARB(currentUniform->mLocation, glArraySize, 
						GL_TRUE, params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_MATRIX_2X3:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix2x3fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_2X4:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix2x4fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_3X2:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix3x2fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_3X3:
					glUniformMatrix3fvARB(currentUniform->mLocation, glArraySize, 
						GL_TRUE, params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_MATRIX_3X4:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix3x4fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_4X2:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix4x2fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_4X3:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix4x3fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_4X4:
					glUniformMatrix4fvARB(currentUniform->mLocation, glArraySize, 
						GL_TRUE, params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_INT1:
					glUniform1ivARB(currentUniform->mLocation, glArraySize, 
						(GLint*)params->getIntPointer(def->physicalIndex));
					break;
				case GCT_INT2:
					glUniform2ivARB(currentUniform->mLocation, glArraySize, 
						(GLint*)params->getIntPointer(def->physicalIndex));
					break;
				case GCT_INT3:
					glUniform3ivARB(currentUniform->mLocation, glArraySize, 
						(GLint*)params->getIntPointer(def->physicalIndex));
					break;
				case GCT_INT4:
					glUniform4ivARB(currentUniform->mLocation, glArraySize, 
						(GLint*)params->getIntPointer(def->physicalIndex));
					break;
				case GCT_SAMPLER1D:
				case GCT_SAMPLER1DSHADOW:
				case GCT_SAMPLER2D:
				case GCT_SAMPLER2DSHADOW:
				case GCT_SAMPLER3D:
				case GCT_SAMPLERCUBE:
					// samplers handled like 1-element ints
					glUniform1ivARB(currentUniform->mLocation, 1, 
						(GLint*)params->getIntPointer(def->physicalIndex));
					break;

				} // end switch
#if OGRE_DEBUG_MODE
				checkForGLSLError( "GLSLLinkProgram::updateUniforms", "Error updating uniform", 0 );
#endif

			}
  
  		} // end for
	}


	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
	{
		if (params->hasPassIterationNumber())
		{
			size_t index = params->getPassIterationNumberIndex();

			GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
			GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

			// need to find the uniform that matches the multi pass entry
			for (;currentUniform != endUniform; ++currentUniform)
			{
				// get the index in the parameter real list
				if (index == currentUniform->mConstantDef->physicalIndex)
				{
					glUniform1fvARB( currentUniform->mLocation, 1, params->getFloatPointer(index));
					// there will only be one multipass entry
					return;
				}
			}
		}

    }
} // namespace Ogre
