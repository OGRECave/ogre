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

#include "OgreShaderGLSLESProgramProcessor.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre {
namespace RTShader {

String GLSLESProgramProcessor::TargetLanguage = "glsles";

//-----------------------------------------------------------------------------
GLSLESProgramProcessor::GLSLESProgramProcessor()
{

}

//-----------------------------------------------------------------------------
GLSLESProgramProcessor::~GLSLESProgramProcessor()
{
    StringVector::iterator it = mLibraryPrograms.begin();
    StringVector::iterator itEnd = mLibraryPrograms.end();
    
    for (; it != itEnd; ++it)
    {
        HighLevelGpuProgramManager::getSingleton().remove(*it);
    }
    mLibraryPrograms.clear();
}

//-----------------------------------------------------------------------------
bool GLSLESProgramProcessor::preCreateGpuPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuVertexProgram();
    Program* fsProgram = programSet->getCpuFragmentProgram();
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
bool GLSLESProgramProcessor::postCreateGpuPrograms(ProgramSet* programSet)
{
    Program* vsCpuProgram = programSet->getCpuVertexProgram();
    Program* fsCpuProgram = programSet->getCpuFragmentProgram();
    GpuProgramPtr vsGpuProgram = programSet->getGpuVertexProgram();
    GpuProgramPtr fsGpuProgram = programSet->getGpuFragmentProgram();

    // Bind vertex shader auto parameters.
    bindAutoParameters(programSet->getCpuVertexProgram(), programSet->getGpuVertexProgram());

    // Bind fragment shader auto parameters.
    bindAutoParameters(programSet->getCpuFragmentProgram(), programSet->getGpuFragmentProgram());

    // Bind texture samplers for the vertex shader.
    bindTextureSamplers(vsCpuProgram, vsGpuProgram);

    // Bind texture samplers for the fragment shader.
    bindTextureSamplers(fsCpuProgram, fsGpuProgram);

#if !OGRE_NO_GLES2_GLSL_OPTIMISER
    vsGpuProgram->setParameter("use_optimiser", "true");
    fsGpuProgram->setParameter("use_optimiser", "true");
#endif

    return true;
}

//-----------------------------------------------------------------------------
void GLSLESProgramProcessor::bindTextureSamplers(Program* pCpuProgram, GpuProgramPtr pGpuProgram)
{
    GpuProgramParametersSharedPtr pGpuParams = pGpuProgram->getDefaultParameters();
    const UniformParameterList& progParams = pCpuProgram->getParameters();
    UniformParameterConstIterator itParams;

    // Bind the samplers.
    for (itParams = progParams.begin(); itParams != progParams.end(); ++itParams)
    {
        const UniformParameterPtr pCurParam = *itParams;
        
        if (pCurParam->isSampler())
        {
            // The optimizer may remove some unnecessary parameters, so we should ignore them
            pGpuParams->setIgnoreMissingParams(true);

            pGpuParams->setNamedConstant(pCurParam->getName(), pCurParam->getIndex());
        }
    }
}

}
}
