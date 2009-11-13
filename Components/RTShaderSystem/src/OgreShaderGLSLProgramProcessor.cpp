/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#include "OgreShaderGLSLProgramProcessor.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderProgram.h"
#include "OgreLogManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

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
		HighLevelGpuProgramManager::getSingleton().remove(*it);
	}
	mLibraryPrograms.clear();
}

//-----------------------------------------------------------------------------
bool GLSLProgramProcessor::preCreateGpuPrograms(ProgramSet* programSet)
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
bool GLSLProgramProcessor::postCreateGpuPrograms(ProgramSet* programSet)
{
	Program* vsCpuProgram = programSet->getCpuVertexProgram();
	Program* fsCpuProgram = programSet->getCpuFragmentProgram();
	GpuProgramPtr vsGpuProgram = programSet->getGpuVertexProgram();
	GpuProgramPtr fsGpuProgram = programSet->getGpuFragmentProgram();
	
	// Bind sub shaders for the vertex shader.
	bindSubShaders(vsCpuProgram, vsGpuProgram);
	
	// Bind sub shaders for the fragment shader.
	bindSubShaders(fsCpuProgram, fsGpuProgram);

	// Bind vertex shader auto parameters.
	bindAutoParameters(programSet->getCpuVertexProgram(), programSet->getGpuVertexProgram());

	// Bind fragment shader auto parameters.
	bindAutoParameters(programSet->getCpuFragmentProgram(), programSet->getGpuFragmentProgram());

	// Bind texture samplers for the vertex shader.
	bindTextureSamplers(vsCpuProgram, vsGpuProgram);

	// Bind texture samplers for the fragment shader.
	bindTextureSamplers(fsCpuProgram, fsGpuProgram);


	return true;
}

//-----------------------------------------------------------------------------
void GLSLProgramProcessor::bindSubShaders(Program* program, GpuProgramPtr pGpuProgram)
{
	if (program->getDependencyCount() > 0)
	{
		String subShaderDef = "";
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

			// Check if the library shader already compiled
			if(!HighLevelGpuProgramManager::getSingleton().resourceExists(subShaderName))
			{
				// Create the library shader
				HighLevelGpuProgramPtr pSubGpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(subShaderName,
					ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TargetLanguage, program->getType());

				// Set the source name
				String sourceName = program->getDependency(i) + "." + TargetLanguage;
				pSubGpuProgram->setSourceFile(sourceName);

				// If we have compile errors than stop processing
				if (pSubGpuProgram->hasCompileError())
				{
					OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
						"Could not compile shader library from the source file: " + sourceName, 
						"GLSLProgramProcessor::bindSubShaders" );	
				}

				mLibraryPrograms.push_back(subShaderName);
			}

			// Append the shader name to subShaders
			subShaderDef += subShaderName + " ";
		}
		pGpuProgram->setParameter("attach", subShaderDef);
	}
	
}
//-----------------------------------------------------------------------------
void GLSLProgramProcessor::bindTextureSamplers(Program* pCpuProgram, GpuProgramPtr pGpuProgram)
{
	GpuProgramParametersSharedPtr pGpuParams = pGpuProgram->getDefaultParameters();
	const ShaderParameterList& progParams = pCpuProgram->getParameters();
	ShaderParameterConstIterator itParams;

	// Bind the samplers.
	for (itParams=progParams.begin(); itParams != progParams.end(); ++itParams)
	{
		const ParameterPtr pCurParam = *itParams;
		const GpuConstantDefinition* gpuConstDef = pGpuParams->_findNamedConstantDefinition(pCurParam->getName());
		
		if (pCurParam->isSampler())
		{		
			pGpuParams->setNamedConstant(pCurParam->getName(), pCurParam->getIndex());						
		}		
	}
}

}
}