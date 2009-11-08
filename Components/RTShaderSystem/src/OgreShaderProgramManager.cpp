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

#include "OgreShaderProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreConfigFile.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderGenerator.h"
#include "OgrePass.h"
#include "OgreLogManager.h"
#include "OgreShaderProgramProcessor.h"
#include "OgreShaderCGProgramProcessor.h"


namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::ProgramManager* Singleton<RTShader::ProgramManager>::ms_Singleton = 0;

namespace RTShader {


//-----------------------------------------------------------------------
ProgramManager* ProgramManager::getSingletonPtr()
{
	assert( ms_Singleton );  
	return ms_Singleton;
}

//-----------------------------------------------------------------------
ProgramManager& ProgramManager::getSingleton()
{
	assert( ms_Singleton );  
	return ( *ms_Singleton );
}

//-----------------------------------------------------------------------------
ProgramManager::ProgramManager()
{
	mVertexShaderCount   = 0;
	mFragmentShaderCount = 0;

	createDefaultProgramProcessors();
}

//-----------------------------------------------------------------------------
ProgramManager::~ProgramManager()
{
	destroyDefaultProgramProcessors();

	destroyProgramSets();

	destroyProgramWriters();
}

//-----------------------------------------------------------------------------
void ProgramManager::acquirePrograms(Pass* pass, RenderState* renderState)
{
	uint32 renderStateHashCode    = renderState->getHashCode();
	ProgramSetIterator itPrograms = mHashToProgramSetMap.find(renderStateHashCode);
	ProgramSet* programSet = NULL;

	// Cached programs for the given render state found.
	if (itPrograms != mHashToProgramSetMap.end())
	{
		programSet = itPrograms->second;		
	}

	// Case we have to generate gpu programs for the given render state.
	else
	{
		programSet = OGRE_NEW ProgramSet;

		if (false == renderState->createCpuPrograms(programSet))
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"Could not apply render state ", 
				"ProgramManager::acquireGpuPrograms" );	
		}	


		if (false == createGpuPrograms(programSet))
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"Could not create gpu programs from render state ", 
				"ProgramManager::acquireGpuPrograms" );
		}	

		mHashToProgramSetMap[renderStateHashCode] = programSet;
	}

	pass->setVertexProgram(programSet->getGpuVertexProgram()->getName());
	pass->setFragmentProgram(programSet->getGpuFragmentProgram()->getName());
}

//-----------------------------------------------------------------------------
void ProgramManager::releasePrograms(RenderState* renderState)
{
	uint32 renderStateHashCode    = renderState->getHashCode();
	ProgramSetIterator itPrograms = mHashToProgramSetMap.find(renderStateHashCode);
	
	// Cached programs for the given render state found.
	if (itPrograms != mHashToProgramSetMap.end())
	{
		OGRE_DELETE itPrograms->second;	
		mHashToProgramSetMap.erase(itPrograms);
	}
}

//-----------------------------------------------------------------------------
void ProgramManager::createDefaultProgramProcessors()
{
	mDefaultProgramProcessors.push_back(OGRE_NEW CGProgramProcessor);

	for (unsigned int i=0; i < mDefaultProgramProcessors.size(); ++i)
	{
		addProgramProcessor(mDefaultProgramProcessors[i]);
	}
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyDefaultProgramProcessors()
{
	for (unsigned int i=0; i < mDefaultProgramProcessors.size(); ++i)
	{
		removeProgramProcessor(mDefaultProgramProcessors[i]);
		OGRE_DELETE mDefaultProgramProcessors[i];
	}
	mDefaultProgramProcessors.clear();
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyProgramSets()
{
	ProgramSetIterator it;

	for (it=mHashToProgramSetMap.begin(); it != mHashToProgramSetMap.end(); ++it)
	{
		OGRE_DELETE it->second;
	}
	mHashToProgramSetMap.clear();
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyProgramWriters()
{
	ProgramWriterIterator it    = mProgramWritersMap.begin();
	ProgramWriterIterator itEnd = mProgramWritersMap.end();

	for (; it != itEnd; ++it)
	{
		if (it->second != NULL)
		{
			OGRE_DELETE it->second;
			it->second = NULL;
		}					
	}
	mProgramWritersMap.clear();
}

//-----------------------------------------------------------------------------
Program* ProgramManager::createCpuProgram(GpuProgramType type)
{
	
	Program* shaderProgram = OGRE_NEW Program(type);

	mCpuProgramsList.push_back(shaderProgram);

	return shaderProgram;
}


//-----------------------------------------------------------------------------
bool ProgramManager::destroyCpuProgram(Program* shaderProgram)
{
	ProgramListIterator it    = mCpuProgramsList.begin();
	ProgramListIterator itEnd = mCpuProgramsList.end();

	for (; it != itEnd; ++it)
	{
		if (*it == shaderProgram)
		{			
			OGRE_DELETE *it;			
			mCpuProgramsList.erase(it);
			return true;			
		}		
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ProgramManager::createGpuPrograms(ProgramSet* programSet)
{
	// Grab the matching writer.
	const String& language = ShaderGenerator::getSingleton().getShaderLanguage();
	ProgramWriterIterator itWriter = mProgramWritersMap.find(language);
	ProgramWriter* programWriter = NULL;


	// No writer found -> create new one.
	if (itWriter == mProgramWritersMap.end())
	{
		programWriter = OGRE_NEW ProgramWriter(language);
		mProgramWritersMap[language] = programWriter;
	}
	else
	{
		programWriter = itWriter->second;
	}

	ProgramProcessorIterator itProcessor = mProgramProcessorsMap.find(language);
	ProgramProcessor* programProcessor = NULL;

	if (itProcessor != mProgramProcessorsMap.end())
	{
		programProcessor = itProcessor->second;
	}

	// Case program processor found.
	if (programProcessor != NULL)
	{
		bool success;

		// Call the pre creation of GPU programs method.
		success = programProcessor->preCreateGpuPrograms(programSet);
		if (success == false)	
			return false;	
	}


	// Create the vertex shader program.
	GpuProgramPtr vsGpuProgram;
	
	vsGpuProgram = createGpuProgram(programSet->getCpuVertexProgram(), 
		programWriter,
		language, 
		ShaderGenerator::getSingleton().getVertexShaderProfiles(),
		ShaderGenerator::getSingleton().getShaderCachePath());

	if (vsGpuProgram.isNull())	
		return false;

	programSet->setGpuVertexProgram(vsGpuProgram);


	// Create the fragment shader program.
	GpuProgramPtr psGpuProgram;

	psGpuProgram = createGpuProgram(programSet->getCpuFragmentProgram(), 
		programWriter,
		language, 
		ShaderGenerator::getSingleton().getFragmentShaderProfiles(),
		ShaderGenerator::getSingleton().getShaderCachePath());

	if (psGpuProgram.isNull())	
		return false;

	programSet->setGpuFragmentProgram(psGpuProgram);

	return true;
	
}

//-----------------------------------------------------------------------------
GpuProgramPtr ProgramManager::createGpuProgram(Program* shaderProgram, 
											   ProgramWriter* programWriter,
											   const String& language,
											   const String& profiles,
											   const String& cachePath)
{

	

	std::stringstream sourceCodeStringStream;
	_StringHash stringHash;
	uint32 programHashCode;
	String programName;

	// Generate source code.
	programWriter->writeSourceCode(sourceCodeStringStream, shaderProgram);	

	// Generate program hash code.
	programHashCode = static_cast<uint32>(stringHash(sourceCodeStringStream.str()));

	// Generate program name.
	programName = StringConverter::toString(programHashCode);
	
	if (shaderProgram->getType() == GPT_VERTEX_PROGRAM)
	{
		programName += "_VS";
	}
	else if (shaderProgram->getType() == GPT_FRAGMENT_PROGRAM)
	{
		programName += "_FS";
	}

	HighLevelGpuProgramPtr pGpuProgram;

	// Try to get program by name.
	pGpuProgram = HighLevelGpuProgramManager::getSingleton().getByName(programName);

	// Case the program doesn't exist yet.
	if (pGpuProgram.isNull())
	{
		const String  programFileName = cachePath + programName + "." + language;	
		std::ifstream programFile;
		bool		  writeFile = true;


		// Check if program file already exist.
		programFile.open(programFileName.c_str());

		// Case no matching file found -> we have to write it.
		if (!programFile)
		{
			writeFile = true;
		}
		else
		{
			writeFile = false;
			programFile.close();
		}

		// Case we have to write the program to a file.
		if (writeFile)
		{
			std::ofstream outFile(programFileName.c_str());

			if (!outFile)
				return GpuProgramPtr();

			outFile << sourceCodeStringStream.str();
			outFile.close();
		}



		// Create new GPU program.
		pGpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(programName,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, language, shaderProgram->getType());


		pGpuProgram->setSourceFile(programFileName);
		pGpuProgram->setParameter("entry_point", shaderProgram->getEntryPointFunction()->getName());
		pGpuProgram->setParameter("target", profiles);
		pGpuProgram->setParameter("profiles", profiles);


		GpuProgramParametersSharedPtr pGpuParams = pGpuProgram->getDefaultParameters();
		const ShaderParameterList& progParams = shaderProgram->getParameters();
		ShaderParameterConstIterator itParams;

		// Case an error occurred.
		if (pGpuProgram->hasCompileError())
		{
			pGpuProgram.setNull();
			return GpuProgramPtr(pGpuProgram);
		}

		// Bind auto parameters.
		for (itParams=progParams.begin(); itParams != progParams.end(); ++itParams)
		{
			const ParameterPtr pCurParam = *itParams;
			const GpuConstantDefinition* gpuConstDef = pGpuParams->_findNamedConstantDefinition(pCurParam->getName());


			if (pCurParam->isAutoConstantParameter())
			{
				if (pCurParam->isAutoConstantRealParameter())
				{
					if (gpuConstDef != NULL)
					{
						pGpuParams->setNamedAutoConstantReal(pCurParam->getName(), 
							pCurParam->getAutoConstantType(), 
							pCurParam->getAutoConstantRealData());
					}	
					else
					{
						LogManager::getSingleton().stream() << "RTShader::ProgramManager: Can not bind auto param named " << 
							pCurParam->getName() << " to program named " << pGpuProgram->getName();
					}
				}
				else if (pCurParam->isAutoConstantIntParameter())
				{
					if (gpuConstDef != NULL)
					{
						pGpuParams->setNamedAutoConstant(pCurParam->getName(), 
							pCurParam->getAutoConstantType(), 
							pCurParam->getAutoConstantIntData());
					}
					else
					{
						LogManager::getSingleton().stream() << "RTShader::ProgramManager: Can not bind auto param named " << 
							pCurParam->getName() << " to program named " << pGpuProgram->getName();
					}
				}						
			}
			else
			{
				// No auto constant - we have to update its variability ourself.
				if (gpuConstDef != NULL)
				{
					gpuConstDef->variability |= pCurParam->getVariability();

					// Update variability in the float map.
					if (gpuConstDef->isSampler() == false)
					{
						GpuLogicalBufferStructPtr floatLogical = pGpuParams->getFloatLogicalBufferStruct();
						if (floatLogical.get())
						{
							for (GpuLogicalIndexUseMap::const_iterator i = floatLogical->map.begin(); i != floatLogical->map.end(); ++i)
							{
								if (i->second.physicalIndex == gpuConstDef->physicalIndex)
								{
									i->second.variability |= gpuConstDef->variability;
									break;
								}
							}
						}
					}							
				}
			}
		}

		if (shaderProgram->getType() == GPT_VERTEX_PROGRAM)
		{
			mVertexShaderCount++;
		}
		else if (shaderProgram->getType() == GPT_FRAGMENT_PROGRAM)
		{
			mFragmentShaderCount++;
		}
	}
	

	return GpuProgramPtr(pGpuProgram);
}

//-----------------------------------------------------------------------------
void ProgramManager::addProgramProcessor(ProgramProcessor* processor)
{
	
	ProgramProcessorIterator itFind = mProgramProcessorsMap.find(processor->getTargetLanguage());

	if (itFind != mProgramProcessorsMap.end())
	{
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
			"A processor for language '" + processor->getTargetLanguage() + "' already exists.",
			"ProgramManager::addProgramProcessor");
	}		

	mProgramProcessorsMap[processor->getTargetLanguage()] = processor;
}

//-----------------------------------------------------------------------------
void ProgramManager::removeProgramProcessor(ProgramProcessor* processor)
{
	ProgramProcessorIterator itFind = mProgramProcessorsMap.find(processor->getTargetLanguage());

	if (itFind != mProgramProcessorsMap.end())
		mProgramProcessorsMap.erase(itFind);

}

//-----------------------------------------------------------------------------
void ProgramManager::destroyGpuProgram(const String& name, GpuProgramType type)
{	
	ResourcePtr res = HighLevelGpuProgramManager::getSingleton().getByName(name);
	
	if (res.isNull() == false)
	{
		if (type == GPT_VERTEX_PROGRAM)
		{
			assert(mVertexShaderCount > 0);
			mVertexShaderCount--;
		}
		else if (type == GPT_FRAGMENT_PROGRAM)
		{
			assert(mFragmentShaderCount > 0);
			mFragmentShaderCount--;
		}

		HighLevelGpuProgramManager::getSingleton().remove(name);
	}
}

}
}