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
	
}

//-----------------------------------------------------------------------------
ProgramManager::~ProgramManager()
{
	destroyProgramSets();

	destroyPrograms();

	destroyProgramsWriters();
}

//-----------------------------------------------------------------------------
void ProgramManager::acquireGpuPrograms(Pass* pass, RenderState* renderState)
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
void ProgramManager::destroyPrograms()
{
	NameToProgramIterator it;

	for (it=mNameToProgramMap.begin(); it != mNameToProgramMap.end(); ++it)
	{
		OGRE_DELETE it->second;
	}
	mNameToProgramMap.clear();
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyProgramsWriters()
{
	NameToProgramWriterIterator it;

	for (it=mNameToProgramWritersMap.begin(); it != mNameToProgramWritersMap.end(); ++it)
	{
		if (it->second != NULL)
		{
			OGRE_DELETE it->second;
			it->second = NULL;
		}					
	}
	mNameToProgramWritersMap.clear();
}

//-----------------------------------------------------------------------------
Program* ProgramManager::createCpuProgram(const String& name, const String& desc, GpuProgramType type)
{
	Program* shaderProgram; 

	shaderProgram = getCpuProgram(name);
	if (shaderProgram != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Shader program named " + name + " already exists !!", 
			"ProgramManager::createCpuProgram" );		
	}
	
	shaderProgram = OGRE_NEW Program(name, desc, type);
	mNameToProgramMap[name] = shaderProgram;

	return shaderProgram;
}

//-----------------------------------------------------------------------------
Program* ProgramManager::getCpuProgram(const String& name)
{
	NameToProgramIterator itFind = mNameToProgramMap.find(name);

	if (itFind != mNameToProgramMap.end())
	{
		return itFind->second;
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
bool ProgramManager::destroyCpuProgram(const String& name)
{
	NameToProgramIterator itFind = mNameToProgramMap.find(name);

	if (itFind != mNameToProgramMap.end())
	{
		OGRE_DELETE itFind->second;
		mNameToProgramMap.erase(itFind);

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ProgramManager::createGpuPrograms(ProgramSet* programSet)
{
	GpuProgramPtr vsGpuProgram;
	
	vsGpuProgram = createGpuProgram(programSet->getCpuVertexProgram(), 
		ShaderGenerator::getSingleton().getShaderLanguage(), 
		ShaderGenerator::getSingleton().getVertexShaderProfiles(),
		ShaderGenerator::getSingleton().getShaderCachePath());

	if (vsGpuProgram.isNull())	
		return false;

	programSet->setGpuVertexProgram(vsGpuProgram);


	GpuProgramPtr psGpuProgram;

	psGpuProgram = createGpuProgram(programSet->getCpuFragmentProgram(), 
		ShaderGenerator::getSingleton().getShaderLanguage(), 
		ShaderGenerator::getSingleton().getFragmentShaderProfiles(),
		ShaderGenerator::getSingleton().getShaderCachePath());

	if (psGpuProgram.isNull())	
		return false;

	programSet->setGpuFragmentProgram(psGpuProgram);

	return true;
	
}

//-----------------------------------------------------------------------------
GpuProgramPtr ProgramManager::createGpuProgram(Program* shaderProgram, 
											   const String& language,
											   const String& profiles,
											   const String& cachePath)
{
	const String  programFileName = cachePath + shaderProgram->getName() + "." + language;	
	std::ifstream programFile;
	bool		  writeFile = true;

	
	// Check if program file already exists.
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
		// Generate source code.
		NameToProgramWriterIterator itWriter = mNameToProgramWritersMap.find(language);
		ProgramWriter* programWriter = NULL;


		// No writer found -> create new one.
		if (itWriter == mNameToProgramWritersMap.end())
		{
			programWriter = OGRE_NEW ProgramWriter(language);
			mNameToProgramWritersMap[language] = programWriter;
		}
		else
		{
			programWriter = itWriter->second;
		}

		std::ofstream outFile(programFileName.c_str());

		if (!outFile)
			return GpuProgramPtr();

		programWriter->writeSourceCode(outFile, shaderProgram);		
		outFile.close();
	}



	// Create new GPU program.
	HighLevelGpuProgramPtr pGpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(shaderProgram->getName(),
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
						pCurParam->getName() << " to program named " << shaderProgram->getName();
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
						pCurParam->getName() << " to program named " << shaderProgram->getName();
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

	return GpuProgramPtr(pGpuProgram);
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyGpuProgram(const String& name)
{	
	HighLevelGpuProgramManager::getSingleton().remove(name);
}

}
}