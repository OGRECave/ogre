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

#include "OgreGLSLExtSupport.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreGLSLProgram.h"

namespace Ogre {
    namespace GLSL {
	GLuint GLSLGpuProgram::mVertexShaderCount = 0;
	GLuint GLSLGpuProgram::mFragmentShaderCount = 0;
	GLuint GLSLGpuProgram::mGeometryShaderCount = 0;
    //-----------------------------------------------------------------------------
	GLSLGpuProgram::GLSLGpuProgram(GLSLProgram* parent) : 
        GLGpuProgram(parent->getCreator(), parent->getName(), parent->getHandle(), 
            parent->getGroup(), false, 0), mGLSLProgram(parent)
    {
        mType = parent->getType();
        mSyntaxCode = "glsl";

		if (parent->getType() == GPT_VERTEX_PROGRAM)
		{
			mProgramID = ++mVertexShaderCount;
		}
		else if (parent->getType() == GPT_FRAGMENT_PROGRAM)
		{
			mProgramID = ++mFragmentShaderCount;
		}
		else
		{
			mProgramID = ++mGeometryShaderCount;
		}

        // transfer skeletal animation status from parent
        mSkeletalAnimation = mGLSLProgram->isSkeletalAnimationIncluded();
		// there is nothing to load
		mLoadFromFile = false;
    }
    //-----------------------------------------------------------------------
    GLSLGpuProgram::~GLSLGpuProgram()
    {
        // have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
    }
	//-----------------------------------------------------------------------------
    void GLSLGpuProgram::loadImpl(void)
    {
		// nothing to load
    }

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::unloadImpl(void)
	{
		// nothing to unload
	}

	//-----------------------------------------------------------------------------
    void GLSLGpuProgram::loadFromSource(void)
    {
		// nothing to load
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::bindProgram(void)
	{
		// Tell the Link Program Manager what shader is to become active
		switch (mType)
		{
		case GPT_VERTEX_PROGRAM:
			GLSLLinkProgramManager::getSingleton().setActiveVertexShader( this );
			break;
		case GPT_FRAGMENT_PROGRAM:
			GLSLLinkProgramManager::getSingleton().setActiveFragmentShader( this );
			break;
		case GPT_GEOMETRY_PROGRAM:
			GLSLLinkProgramManager::getSingleton().setActiveGeometryShader( this );
			break;
		default:
		        break;
		}
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::unbindProgram(void)
	{
		// Tell the Link Program Manager what shader is to become inactive
		if (mType == GPT_VERTEX_PROGRAM)
		{
			GLSLLinkProgramManager::getSingleton().setActiveVertexShader( NULL );
		}
		else if (mType == GPT_GEOMETRY_PROGRAM)
		{
			GLSLLinkProgramManager::getSingleton().setActiveGeometryShader( NULL );
		}
		else // its a fragment shader
		{
			GLSLLinkProgramManager::getSingleton().setActiveFragmentShader( NULL );
		}

	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
	{
		// link can throw exceptions, ignore them at this point
		try
		{
			// activate the link program object
			GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
			// pass on parameters from params to program object uniforms
			linkProgram->updateUniforms(params, mask, mType);
		}
		catch (Exception&) {}
	
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params)
	{
		// activate the link program object
		GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
		// pass on parameters from params to program object uniforms
		linkProgram->updatePassIterationUniforms( params );
		
	}
	//-----------------------------------------------------------------------------
	GLuint GLSLGpuProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		// get link program - only call this in the context of bound program
		GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();

		if (linkProgram->isAttributeValid(semantic, index))
		{
			return linkProgram->getAttributeIndex(semantic, index);
		}
		else
		{
			// fall back to default implementation, allow default bindings
			return GLGpuProgram::getAttributeIndex(semantic, index);
		}
		
	}
	//-----------------------------------------------------------------------------
	bool GLSLGpuProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
	{
		// get link program - only call this in the context of bound program
		GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();

		if (linkProgram->isAttributeValid(semantic, index))
		{
			return true;
		}
		else
		{
			// fall back to default implementation, allow default bindings
			return GLGpuProgram::isAttributeValid(semantic, index);
		}
	}

}

}

