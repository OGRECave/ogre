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

#include "OgreGLSLESProgramPipelineManager.h"
#include "OgreGLSLESProgram.h"
#include "OgreGLSLESProgram.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    template<> GLSLESProgramPipelineManager* Singleton<GLSLESProgramPipelineManager>::msSingleton = 0;
    
    //-----------------------------------------------------------------------
    GLSLESProgramPipelineManager* GLSLESProgramPipelineManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    
    //-----------------------------------------------------------------------
    GLSLESProgramPipelineManager& GLSLESProgramPipelineManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }

    GLSLESProgramPipelineManager::GLSLESProgramPipelineManager(void) :
        GLSLESProgramManagerCommon(), mActiveProgramPipeline(NULL) { }

    GLSLESProgramPipelineManager::~GLSLESProgramPipelineManager(void) {}

    //-----------------------------------------------------------------------
    void GLSLESProgramPipelineManager::setActiveFragmentLinkProgram(GLSLESProgram* fragmentGpuProgram)
    {
        if (fragmentGpuProgram != mActiveFragmentGpuProgram)
        {
            mActiveFragmentGpuProgram = fragmentGpuProgram;
            // ActiveProgramPipeline is no longer valid
            mActiveProgramPipeline = NULL;
        }
    }
    
    //-----------------------------------------------------------------------
    void GLSLESProgramPipelineManager::setActiveVertexLinkProgram(GLSLESProgram* vertexGpuProgram)
    {
        if (vertexGpuProgram != mActiveVertexGpuProgram)
        {
            mActiveVertexGpuProgram = vertexGpuProgram;
            // ActiveProgramPipeline is no longer valid
            mActiveProgramPipeline = NULL;
        }
    }

    //-----------------------------------------------------------------------
    GLSLESProgramPipeline* GLSLESProgramPipelineManager::getActiveProgramPipeline(void)
    {
        // If there is an active link program then return it
        if (mActiveProgramPipeline)
            return mActiveProgramPipeline;

        // No active link program so find one or make a new one
        // Is there an active key?
        uint32 activeKey = 0;
        if (mActiveVertexGpuProgram)
        {
            activeKey = HashCombine(mActiveVertexGpuProgram->getShaderID(), activeKey);
        }
        if (mActiveFragmentGpuProgram)
        {
            activeKey = HashCombine(mActiveFragmentGpuProgram->getShaderID(), activeKey);
        }

        // Only return a program pipeline object if a vertex or fragment stage exist
        if (activeKey > 0)
        {
            // Find the key in the hash map
            ProgramIterator programFound = mPrograms.find(activeKey);
            // Program object not found for key so need to create it
            if (programFound == mPrograms.end())
            {
                mActiveProgramPipeline = new GLSLESProgramPipeline(mActiveVertexGpuProgram, mActiveFragmentGpuProgram);
                mPrograms[activeKey] = mActiveProgramPipeline;
            }
            else
            {
                // Found a link program in map container so make it active
                mActiveProgramPipeline = static_cast<GLSLESProgramPipeline*>(programFound->second);
            }
            
        }
        // Make the program object active
        if (mActiveProgramPipeline)
            mActiveProgramPipeline->activate();
        
        return mActiveProgramPipeline;
    }

}
