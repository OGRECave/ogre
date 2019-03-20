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
        bindSubShaders(cpuProgram, gpuProgram);
        bindAutoParameters(cpuProgram, gpuProgram);
        bindTextureSamplers(cpuProgram, gpuProgram);
    }

    return true;
}

//-----------------------------------------------------------------------------
void GLSLProgramProcessor::bindSubShaders(Program* program, GpuProgramPtr pGpuProgram)
{
    const String& group = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;

    if (program->getDependencyCount() > 0)
    {
        // Get all attached shaders so we do not attach shaders twice.
        // maybe GLSLProgram should take care of that ( prevent add duplicate shaders )
        String attachedShaders = pGpuProgram->getParameter("attach");
        String subShaderDef = "";

        auto* rs = Root::getSingleton().getRenderSystem();
        int GLSLVersion = rs ? rs->getNativeShadingLanguageVersion() : 100;

        for (unsigned int i=0; i < program->getDependencyCount(); ++i)
        {
            // Here we append _VS and _FS to the library shaders (so max each lib shader
            // is compiled twice once as vertex and once as fragment shader)
            String subShaderName = program->getDependency(i);
            if (program->getType() == GPT_VERTEX_PROGRAM)
            {
                subShaderName += "_VS";
            }
            else
            {
                subShaderName += "_FS";
            }                   

            const String& defs = program->getPreprocessorDefines();
            if (!defs.empty())
            {
                subShaderName += std::to_string(FastHash(defs.c_str(), defs.size()));
            }

            // Check if the library shader already compiled
            if (!HighLevelGpuProgramManager::getSingleton().resourceExists(subShaderName, group))
            {
                // Create the library shader
                HighLevelGpuProgramPtr pSubGpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(subShaderName,
                    group, TargetLanguage, program->getType());

                // Set the source name
                String sourceName = program->getDependency(i) + ".glsl";

                // load source code
                DataStreamPtr stream =
                    ResourceGroupManager::getSingleton().openResource(sourceName, group);
                String sourceCode = stream->getAsString();

                // Prepend the current GLSL version
                String versionLine = "#version " + StringConverter::toString(GLSLVersion) + "\n";

                if(GLSLVersion > 130) {
                    versionLine += GLSLProgramWriter::getGL3CompatDefines();
                }

                pSubGpuProgram->setSource(versionLine + sourceCode);
                pSubGpuProgram->setPreprocessorDefines(program->getPreprocessorDefines());
                pSubGpuProgram->load();

                // If we have compile errors than stop processing
                if (pSubGpuProgram->hasCompileError())
                {
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                        "Could not compile shader library from the source file: " + sourceName, 
                        "GLSLProgramProcessor::bindSubShaders" );   
                }

                mLibraryPrograms.push_back(subShaderName);
            }

            // Check if the lib shader already attached to this shader
            if (attachedShaders.find(subShaderName) == String::npos)
            {
                // Append the shader name to subShaders
                subShaderDef += subShaderName + " ";
            }
        }

        // Check if we have something to attach
        if (subShaderDef.length() > 0)
        {
            pGpuProgram->setParameter("attach", subShaderDef);
        }
    }
    
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
