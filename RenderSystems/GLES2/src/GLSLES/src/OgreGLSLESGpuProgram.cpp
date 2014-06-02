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

#include "OgreGLSLESGpuProgram.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLSLESLinkProgramManager.h"
#include "OgreGLSLESProgramPipelineManager.h"
#include "OgreRoot.h"

namespace Ogre {

    GLuint GLSLESGpuProgram::mVertexShaderCount = 0;
    GLuint GLSLESGpuProgram::mFragmentShaderCount = 0;
    //-----------------------------------------------------------------------------
    GLSLESGpuProgram::GLSLESGpuProgram(GLSLESProgram* parent) : 
        GLES2GpuProgram(parent->getCreator(), parent->getName(), parent->getHandle(), 
            parent->getGroup(), false, 0), mGLSLProgram(parent)
    {
        mType = parent->getType();
        mSyntaxCode = "glsles";

        mLinked = 0;

        if (parent->getType() == GPT_VERTEX_PROGRAM)
        {
            mProgramID = ++mVertexShaderCount;
        }
        else if (parent->getType() == GPT_FRAGMENT_PROGRAM)
        {
            mProgramID = ++mFragmentShaderCount;
        }

        // Transfer skeletal animation status from parent
        mSkeletalAnimation = mGLSLProgram->isSkeletalAnimationIncluded();
        // There is nothing to load
        mLoadFromFile = false;
    }
    //-----------------------------------------------------------------------
    GLSLESGpuProgram::~GLSLESGpuProgram()
    {
		// This workaround is needed otherwise we carry on some dangling pointers
		GLSLESLinkProgramManager::getSingletonPtr()->destroyAllByProgram(this);

        // Have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::loadImpl(void)
    {
        // nothing to load
    }

    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::unloadImpl(void)
    {
        // nothing to unload
    }

    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::loadFromSource(void)
    {
        // nothing to load
    }

    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::bindProgram(void)
    {
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Tell the Program Pipeline Manager what pipeline is to become active
            switch (mType)
            {
                case GPT_VERTEX_PROGRAM:
                    GLSLESProgramPipelineManager::getSingleton().setActiveVertexLinkProgram( this );
                    break;
                case GPT_FRAGMENT_PROGRAM:
                    GLSLESProgramPipelineManager::getSingleton().setActiveFragmentLinkProgram( this );
                    break;
                case GPT_GEOMETRY_PROGRAM:
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
                    GLSLESLinkProgramManager::getSingleton().setActiveVertexShader( this );
                    break;
                case GPT_FRAGMENT_PROGRAM:
                    GLSLESLinkProgramManager::getSingleton().setActiveFragmentShader( this );
                    break;
                case GPT_GEOMETRY_PROGRAM:
                default:
                    break;
            }
        }
    }

    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::unbindProgram(void)
    {
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Tell the Program Pipeline Manager what pipeline is to become inactive
            if (mType == GPT_VERTEX_PROGRAM)
            {
                GLSLESProgramPipelineManager::getSingleton().setActiveVertexLinkProgram(NULL);
            }
            else if (mType == GPT_FRAGMENT_PROGRAM)
            {
                GLSLESProgramPipelineManager::getSingleton().setActiveFragmentLinkProgram(NULL);
            }
        }
        else
        {
            // Tell the Link Program Manager what shader is to become inactive
            if (mType == GPT_VERTEX_PROGRAM)
            {
                GLSLESLinkProgramManager::getSingleton().setActiveVertexShader( NULL );
            }
            else if (mType == GPT_FRAGMENT_PROGRAM)
            {
                GLSLESLinkProgramManager::getSingleton().setActiveFragmentShader( NULL );
            }
        }
    }
    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::unbindAll(void)
    {
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLESProgramPipelineManager &glslManager = GLSLESProgramPipelineManager::getSingleton();
            glslManager.setActiveVertexLinkProgram(NULL);
            glslManager.setActiveFragmentLinkProgram(NULL);
        }
        else
        {
            GLSLESLinkProgramManager &glslManager = GLSLESLinkProgramManager::getSingleton();
            glslManager.setActiveVertexShader( NULL );
            glslManager.setActiveFragmentShader( NULL );
        }
    }

    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::bindProgramParameters(GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // Link can throw exceptions, ignore them at this point
        try
        {
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Activate the program pipeline object
                GLSLESProgramPipeline* programPipeline = GLSLESProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                // Pass on parameters from params to program object uniforms
                programPipeline->updateUniforms(params, mask, mType);
            }
            else
            {
                // Activate the link program object
                GLSLESLinkProgram* linkProgram = GLSLESLinkProgramManager::getSingleton().getActiveLinkProgram();
                // Pass on parameters from params to program object uniforms
                linkProgram->updateUniforms(params, mask, mType);
            }
        }
        catch (Exception& e) {}
    }
    
    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::bindProgramSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // Link can throw exceptions, ignore them at this point
        try
        {
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Activate the program pipeline object
                GLSLESProgramPipeline* programPipeline = GLSLESProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                // Pass on parameters from params to program object uniforms
                programPipeline->updateUniformBlocks(params, mask, mType);
            }
            else
            {
                // Activate the link program object
                GLSLESLinkProgram* linkProgram = GLSLESLinkProgramManager::getSingleton().getActiveLinkProgram();
                // Pass on parameters from params to program object uniforms
                linkProgram->updateUniformBlocks(params, mask, mType);
            }
        }
        catch (Exception& e) {}
    }

    //-----------------------------------------------------------------------------
    void GLSLESGpuProgram::bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params)
    {
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Activate the program pipeline object
            GLSLESProgramPipeline* programPipeline = GLSLESProgramPipelineManager::getSingleton().getActiveProgramPipeline();
            // Pass on parameters from params to program object uniforms
            programPipeline->updatePassIterationUniforms( params );
        }
        else
        {
            // Activate the link program object
            GLSLESLinkProgram* linkProgram = GLSLESLinkProgramManager::getSingleton().getActiveLinkProgram();
            // Pass on parameters from params to program object uniforms
            linkProgram->updatePassIterationUniforms( params );
        }
    }
}
