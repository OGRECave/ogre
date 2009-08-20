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

#include "OgreShaderProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreConfigFile.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderGenerator.h"
#include "OgrePass.h"
#include "OgreLogManager.h"


// Uncomment to use cached files from previous runs.
//#define _USE_CACHED_FILES_

namespace Ogre {

//-----------------------------------------------------------------------
template<> 
CRTShader::ProgramManager* Singleton<CRTShader::ProgramManager>::ms_Singleton = 0;

namespace CRTShader {


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
		programSet = new ProgramSet;

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
ProgramSet* ProgramManager::getProgramSet(RenderState* renderState)
{
	uint32 renderStateHashCode    = renderState->getHashCode();
	ProgramSetIterator itPrograms = mHashToProgramSetMap.find(renderStateHashCode);
	
	// Cached programs for the given render state found.
	if (itPrograms != mHashToProgramSetMap.end())
	{
		return itPrograms->second;		
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyProgramSets()
{
	ProgramSetIterator it;

	for (it=mHashToProgramSetMap.begin(); it != mHashToProgramSetMap.end(); ++it)
	{
		delete it->second;
	}
	mHashToProgramSetMap.clear();
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyPrograms()
{
	NameToProgramIterator it;

	for (it=mNameToProgramMap.begin(); it != mNameToProgramMap.end(); ++it)
	{
		delete it->second;
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
			delete it->second;
			it->second = NULL;
		}					
	}
	mNameToProgramWritersMap.clear();
}

//-----------------------------------------------------------------------------
Program* ProgramManager::createProgram(const String& name, const String& desc, GpuProgramType type)
{
	Program* shaderProgram; 

	shaderProgram = getProgram(name);
	if (shaderProgram != NULL)
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Shader program named " + name + " already exists !!", 
			"ProgramManager::createShaderProgram" );		
	}
	
	shaderProgram = new Program(name, desc, type);
	mNameToProgramMap[name] = shaderProgram;

	return shaderProgram;
}

//-----------------------------------------------------------------------------
Program* ProgramManager::getProgram(const String& name)
{
	NameToProgramIterator itFind = mNameToProgramMap.find(name);

	if (itFind != mNameToProgramMap.end())
	{
		return itFind->second;
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
bool ProgramManager::destroyProgram(const String& name)
{
	NameToProgramIterator itFind = mNameToProgramMap.find(name);

	if (itFind != mNameToProgramMap.end())
	{
		delete itFind->second;
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
	// Generate source code.
	NameToProgramWriterIterator itWriter = mNameToProgramWritersMap.find(language);
	ProgramWriter* programWriter = NULL;
	
	
	// No writer found -> create new one.
	if (itWriter == mNameToProgramWritersMap.end())
	{
		programWriter = new ProgramWriter(language);
		mNameToProgramWritersMap[language] = programWriter;
	}
	else
	{
		programWriter = itWriter->second;
	}
	
	// Write the program to a file.


	const String programFileName = cachePath + shaderProgram->getName() + "." + language;	
	bool writeFile = true;

#ifdef _USE_CACHED_FILES_

	std::ofstream programFile;
	programFile.open(programFileName.c_str(), std::ios_base::in | std::ios_base::_Nocreate);
	if (!programFile)
	{
		writeFile = true;
	}
	else
	{
		writeFile = false;
		programFile.close();
	}

#endif

	if (writeFile)
	{
		std::ofstream outFile(programFileName.c_str());

		if (!outFile)
			return GpuProgramPtr(HighLevelGpuProgramPtr());

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


	GpuProgramParametersSharedPtr pParams = pGpuProgram->getDefaultParameters();
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
		const Parameter* pCurParam = *itParams;

		if (pCurParam->isAutoConstantParameter())
		{
			if (pCurParam->isAutoConstantRealParameter())
			{
				if (pParams->_findNamedConstantDefinition(pCurParam->getName()) != NULL)
				{
					pParams->setNamedAutoConstantReal(pCurParam->getName(), 
						pCurParam->getAutoConstantType(), 
						pCurParam->getAutoConstantRealData());
				}	
				else
				{
					LogManager::getSingleton().stream() << "CRTShader::ProgramManager: Can not bind auto param named " << 
						pCurParam->getName() << " to program named " << shaderProgram->getName();
				}
			}
			else if (pCurParam->isAutoConstantIntParameter())
			{
				if (pParams->_findNamedConstantDefinition(pCurParam->getName()) != NULL)
				{
					pParams->setNamedAutoConstant(pCurParam->getName(), 
						pCurParam->getAutoConstantType(), 
						pCurParam->getAutoConstantIntData());
				}
				else
				{
					LogManager::getSingleton().stream() << "CRTShader::ProgramManager: Can not bind auto param named " << 
						pCurParam->getName() << " to program named " << shaderProgram->getName();
				}
			}						
		}
	}

	return GpuProgramPtr(pGpuProgram);
}

}
}
