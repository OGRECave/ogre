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

#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLProgramPipelineManager.h"
#include "OgreRoot.h"

namespace Ogre {

	GLuint GLSLGpuProgram::mVertexShaderCount = 0;
	GLuint GLSLGpuProgram::mFragmentShaderCount = 0;
	GLuint GLSLGpuProgram::mGeometryShaderCount = 0;
	GLuint GLSLGpuProgram::mHullShaderCount = 0;
	GLuint GLSLGpuProgram::mDomainShaderCount = 0;
	GLuint GLSLGpuProgram::mComputeShaderCount = 0;
    //-----------------------------------------------------------------------------
	GLSLGpuProgram::GLSLGpuProgram(GLSLProgram* parent) : 
        GL3PlusGpuProgram(parent->getCreator(), parent->getName(), parent->getHandle(), 
            parent->getGroup(), false, 0), mGLSLProgram(parent)
    {
        mType = parent->getType();
        mSyntaxCode = "glsl" + StringConverter::toString(Root::getSingleton().getRenderSystem()->getNativeShadingLanguageVersion());

        mLinked = 0;

		if (parent->getType() == GPT_VERTEX_PROGRAM)
		{
			mProgramID = ++mVertexShaderCount;
		}
		else if (parent->getType() == GPT_FRAGMENT_PROGRAM)
		{
			mProgramID = ++mFragmentShaderCount;
		}
		else if (parent->getType() == GPT_GEOMETRY_PROGRAM)
		{
			mProgramID = ++mGeometryShaderCount;
		}
		else if (parent->getType() == GPT_HULL_PROGRAM)
		{
			mProgramID = ++mHullShaderCount;
		}
        else if (parent->getType() == GPT_COMPUTE_PROGRAM)
		{
			mProgramID = ++mComputeShaderCount;
		}
		else
		{
			mProgramID = ++mDomainShaderCount;
		}

        // Transfer skeletal animation status from parent
        mSkeletalAnimation = mGLSLProgram->isSkeletalAnimationIncluded();
		// There is nothing to load
		mLoadFromFile = false;
    }
    //-----------------------------------------------------------------------
    GLSLGpuProgram::~GLSLGpuProgram()
    {
        // Have to call this here rather than in Resource destructor
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
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Tell the Program Pipeline Manager what pipeline is to become active
            switch (mType)
            {
                case GPT_VERTEX_PROGRAM:
                    GLSLProgramPipelineManager::getSingleton().setActiveVertexLinkProgram( this );
                    break;
                case GPT_FRAGMENT_PROGRAM:
                    GLSLProgramPipelineManager::getSingleton().setActiveFragmentLinkProgram( this );
                    break;
                case GPT_GEOMETRY_PROGRAM:
                    GLSLProgramPipelineManager::getSingleton().setActiveGeometryLinkProgram( this );
                    break;
                case GPT_HULL_PROGRAM:
                    GLSLProgramPipelineManager::getSingleton().setActiveTessHullLinkProgram( this );
                    break;
                case GPT_DOMAIN_PROGRAM:
                    GLSLProgramPipelineManager::getSingleton().setActiveTessDomainLinkProgram( this );
                    break;
                case GPT_COMPUTE_PROGRAM:
                    GLSLProgramPipelineManager::getSingleton().setActiveComputeLinkProgram( this );
                default:
                    break;
            }
        }
        else
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
                case GPT_HULL_PROGRAM:
                    GLSLLinkProgramManager::getSingleton().setActiveHullShader( this );
                    break;
                case GPT_DOMAIN_PROGRAM:
                    GLSLLinkProgramManager::getSingleton().setActiveDomainShader( this );
                    break;
                case GPT_COMPUTE_PROGRAM:
                    GLSLLinkProgramManager::getSingleton().setActiveComputeShader( this );
                default:
                    break;
            }
        }
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::unbindProgram(void)
	{
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Tell the Program Pipeline Manager what pipeline is to become inactive
            if (mType == GPT_VERTEX_PROGRAM)
            {
                GLSLProgramPipelineManager::getSingleton().setActiveVertexLinkProgram( NULL );
            }
            else if (mType == GPT_GEOMETRY_PROGRAM)
            {
                GLSLProgramPipelineManager::getSingleton().setActiveGeometryLinkProgram( NULL );
            }
            else if (mType == GPT_HULL_PROGRAM)
            {
                GLSLProgramPipelineManager::getSingleton().setActiveTessHullLinkProgram( NULL );
            }
            else if (mType == GPT_DOMAIN_PROGRAM)
            {
                GLSLProgramPipelineManager::getSingleton().setActiveTessDomainLinkProgram( NULL );
            }
            else if (mType == GPT_COMPUTE_PROGRAM)
            {
                GLSLProgramPipelineManager::getSingleton().setActiveComputeLinkProgram( NULL );
            }
            else // Its a fragment shader
            {
                GLSLProgramPipelineManager::getSingleton().setActiveFragmentLinkProgram( NULL );
            }
        }
        else
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
            else if (mType == GPT_HULL_PROGRAM)
            {
                GLSLLinkProgramManager::getSingleton().setActiveHullShader( NULL );
            }
            else if (mType == GPT_DOMAIN_PROGRAM)
            {
                GLSLLinkProgramManager::getSingleton().setActiveDomainShader( NULL );
            }
            else if (mType == GPT_COMPUTE_PROGRAM)
            {
                GLSLLinkProgramManager::getSingleton().setActiveComputeShader( NULL );
            }
            else // Its a fragment shader
            {
                GLSLLinkProgramManager::getSingleton().setActiveFragmentShader( NULL );
            }
        }
	}

    //-----------------------------------------------------------------------------
	void GLSLGpuProgram::bindProgramSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask)
	{
		// Link can throw exceptions, ignore them at this point
		try
		{
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Activate the program pipeline object
                GLSLProgramPipeline* programPipeline = GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                // Pass on parameters from params to program object uniforms
                programPipeline->updateUniformBlocks(params, mask, mType);
            }
            else
            {
                // Activate the link program object
                GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
                // Pass on parameters from params to program object uniforms
                linkProgram->updateUniformBlocks(params, mask, mType);
            }
		}
		catch (Exception& e) {}
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
	{
		// Link can throw exceptions, ignore them at this point
		try
		{
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Activate the program pipeline object
                GLSLProgramPipeline* programPipeline = GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                // Pass on parameters from params to program object uniforms
                programPipeline->updateUniforms(params, mask, mType);
            }
            else
            {
                // Activate the link program object
                GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
                // Pass on parameters from params to program object uniforms
                linkProgram->updateUniforms(params, mask, mType);
            }
		}
		catch (Exception& e) {}
	}

	//-----------------------------------------------------------------------------
	void GLSLGpuProgram::bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params)
	{
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Activate the program pipeline object
            GLSLProgramPipeline* programPipeline = GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
            // Pass on parameters from params to program object uniforms
            programPipeline->updatePassIterationUniforms( params );
        }
        else
        {
            // Activate the link program object
            GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            // Pass on parameters from params to program object uniforms
            linkProgram->updatePassIterationUniforms( params );
		}
	}
}

