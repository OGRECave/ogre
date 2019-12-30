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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
namespace RTShader {

String GLSLProgramProcessor::TargetLanguage = "glsl";

//-----------------------------------------------------------------------------
GLSLProgramProcessor::GLSLProgramProcessor()
{

}

//-----------------------------------------------------------------------------
GLSLProgramProcessor::~GLSLProgramProcessor()
{
    StringVector::iterator it = mLibraryPrograms.begin();
    StringVector::iterator itEnd = mLibraryPrograms.end();
    
    for (; it != itEnd; ++it)
    {
        HighLevelGpuProgramManager::getSingleton().remove(
            *it, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    mLibraryPrograms.clear();
}

//-----------------------------------------------------------------------------
bool GLSLProgramProcessor::preCreateGpuPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* fsProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* fsMain   = fsProgram->getEntryPointFunction();    
    bool success;

    // Compact vertex shader outputs.
    success = ProgramProcessor::compactVsOutputs(vsMain, fsMain);
    if (success == false)   
        return false;   

    return true;
}

//-----------------------------------------------------------------------------
bool GLSLProgramProcessor::postCreateGpuPrograms(ProgramSet* programSet)
{
    // Bind vertex auto parameters.
    for(auto type : {GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM})
    {
        Program* cpuProgram = programSet->getCpuProgram(type);
        GpuProgramPtr gpuProgram = programSet->getGpuProgram(type);
        bindAutoParameters(cpuProgram, gpuProgram);
        bindTextureSamplers(cpuProgram, gpuProgram);
    }

    return true;
}
//-----------------------------------------------------------------------------
void GLSLProgramProcessor::bindTextureSamplers(Program* pCpuProgram, GpuProgramPtr pGpuProgram)
{
    GpuProgramParametersSharedPtr pGpuParams = pGpuProgram->getDefaultParameters();
    const UniformParameterList& progParams = pCpuProgram->getParameters();
    UniformParameterConstIterator itParams;

    // Bind the samplers.
    for (itParams=progParams.begin(); itParams != progParams.end(); ++itParams)
    {
        const UniformParameterPtr pCurParam = *itParams;
        
        if (pCurParam->isSampler())
        {       
            pGpuParams->setNamedConstant(pCurParam->getName(), pCurParam->getIndex());                      
        }       
    }
}

}
}
