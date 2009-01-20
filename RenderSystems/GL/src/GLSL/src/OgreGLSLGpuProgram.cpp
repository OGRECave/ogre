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
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreGLSLProgram.h"

namespace Ogre {

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
        // have to call this here reather than in Resource destructor
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
		// tell the Link Program Manager what shader is to become active
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
		}
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::unbindProgram(void)
	{
		// tell the Link Program Manager what shader is to become inactive
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
		// activate the link program object
		GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
		// pass on parameters from params to program object uniforms
		linkProgram->updateUniforms(params, mask, mType);
		
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

