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

#include "OgreShaderProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreConfigFile.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderGenerator.h"
#include "OgrePass.h"
#include "OgreLogManager.h"
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#include "OgreShaderCGProgramWriter.h"
#include "OgreShaderHLSLProgramWriter.h"
#include "OgreShaderGLSLProgramWriter.h"
#endif
#include "OgreShaderGLSLESProgramWriter.h"
#include "OgreShaderProgramProcessor.h"
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#include "OgreShaderCGProgramProcessor.h"
#include "OgreShaderHLSLProgramProcessor.h"
#include "OgreShaderGLSLProgramProcessor.h"
#endif
#include "OgreShaderGLSLESProgramProcessor.h"
#include "OgreGpuProgramManager.h"


namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::ProgramManager* Singleton<RTShader::ProgramManager>::msSingleton = 0;

namespace RTShader {


//-----------------------------------------------------------------------
ProgramManager* ProgramManager::getSingletonPtr()
{
	return msSingleton;
}

//-----------------------------------------------------------------------
ProgramManager& ProgramManager::getSingleton()
{
	assert( msSingleton );  
	return ( *msSingleton );
}

//-----------------------------------------------------------------------------
ProgramManager::ProgramManager()
{
	createDefaultProgramProcessors();
	createDefaultProgramWriterFactories();
}

//-----------------------------------------------------------------------------
ProgramManager::~ProgramManager()
{
	flushGpuProgramsCache();
	destroyDefaultProgramWriterFactories();
	destroyDefaultProgramProcessors();	
	destroyProgramWriters();
}

//-----------------------------------------------------------------------------
void ProgramManager::acquirePrograms(Pass* pass, TargetRenderState* renderState)
{
	// Create the CPU programs.
	if (false == renderState->createCpuPrograms())
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Could not apply render state ", 
			"ProgramManager::acquireGpuPrograms" );	
	}	

	ProgramSet* programSet = renderState->getProgramSet();

	// Create the GPU programs.
	if (false == createGpuPrograms(programSet))
	{
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
			"Could not create gpu programs from render state ", 
			"ProgramManager::acquireGpuPrograms" );
	}	

	// Bind the created GPU programs to the target pass.
	pass->setVertexProgram(programSet->getGpuVertexProgram()->getName());
	pass->setFragmentProgram(programSet->getGpuFragmentProgram()->getName());

	// Bind uniform parameters to pass parameters.
	bindUniformParameters(programSet->getCpuVertexProgram(), pass->getVertexProgramParameters());
	bindUniformParameters(programSet->getCpuFragmentProgram(), pass->getFragmentProgramParameters());

}

//-----------------------------------------------------------------------------
void ProgramManager::releasePrograms(Pass* pass, TargetRenderState* renderState)
{
	ProgramSet* programSet = renderState->getProgramSet();

	pass->setVertexProgram(StringUtil::BLANK);
	pass->setFragmentProgram(StringUtil::BLANK);

	GpuProgramsMapIterator itVsGpuProgram = mVertexShaderMap.find(programSet->getGpuVertexProgram()->getName());
	GpuProgramsMapIterator itFsGpuProgram = mFragmentShaderMap.find(programSet->getGpuFragmentProgram()->getName());

	renderState->destroyProgramSet();

	if (itVsGpuProgram != mVertexShaderMap.end())
	{
		if (itVsGpuProgram->second.useCount() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
		{
			destroyGpuProgram(itVsGpuProgram->second);
			mVertexShaderMap.erase(itVsGpuProgram);
		}
	}

	if (itFsGpuProgram != mFragmentShaderMap.end())
	{
		if (itFsGpuProgram->second.useCount() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
		{
			destroyGpuProgram(itFsGpuProgram->second);
			mFragmentShaderMap.erase(itFsGpuProgram);
		}
	}
}
//-----------------------------------------------------------------------------
void ProgramManager::flushGpuProgramsCache()
{
	flushGpuProgramsCache(mVertexShaderMap);
	flushGpuProgramsCache(mFragmentShaderMap);
}

//-----------------------------------------------------------------------------
void ProgramManager::flushGpuProgramsCache(GpuProgramsMap& gpuProgramsMap)
{
	while (gpuProgramsMap.size() > 0)
	{
		GpuProgramsMapIterator it = gpuProgramsMap.begin();

		destroyGpuProgram(it->second);
		gpuProgramsMap.erase(it);
	}
}

//-----------------------------------------------------------------------------
void ProgramManager::createDefaultProgramWriterFactories()
{
	// Add standard shader writer factories 
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
	mProgramWriterFactories.push_back(OGRE_NEW ShaderProgramWriterCGFactory());
	mProgramWriterFactories.push_back(OGRE_NEW ShaderProgramWriterGLSLFactory());
	mProgramWriterFactories.push_back(OGRE_NEW ShaderProgramWriterHLSLFactory());
#endif
	mProgramWriterFactories.push_back(OGRE_NEW ShaderProgramWriterGLSLESFactory());
	
	for (unsigned int i=0; i < mProgramWriterFactories.size(); ++i)
	{
		ProgramWriterManager::getSingletonPtr()->addFactory(mProgramWriterFactories[i]);
	}
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyDefaultProgramWriterFactories()
{ 
	for (unsigned int i=0; i<mProgramWriterFactories.size(); i++)
	{
		ProgramWriterManager::getSingletonPtr()->removeFactory(mProgramWriterFactories[i]);
		OGRE_DELETE mProgramWriterFactories[i];
	}
	mProgramWriterFactories.clear();
}

//-----------------------------------------------------------------------------
void ProgramManager::createDefaultProgramProcessors()
{
	// Add standard shader processors
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
	mDefaultProgramProcessors.push_back(OGRE_NEW CGProgramProcessor);
	mDefaultProgramProcessors.push_back(OGRE_NEW GLSLProgramProcessor);
	mDefaultProgramProcessors.push_back(OGRE_NEW HLSLProgramProcessor);
#endif
	mDefaultProgramProcessors.push_back(OGRE_NEW GLSLESProgramProcessor);

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

	mCpuProgramsList.insert(shaderProgram);

	return shaderProgram;
}


//-----------------------------------------------------------------------------
void ProgramManager::destroyCpuProgram(Program* shaderProgram)
{
	ProgramListIterator it    = mCpuProgramsList.find(shaderProgram);
	
	if (it != mCpuProgramsList.end())
	{			
		OGRE_DELETE *it;			
		mCpuProgramsList.erase(it);	
	}			
}

//-----------------------------------------------------------------------------
bool ProgramManager::createGpuPrograms(ProgramSet* programSet)
{
	// Before we start we need to make sure that the pixel shader input
	//  parameters are the same as the vertex output, this required by 
	//  shader models 4 and 5.
	// This change may incrase the number of register used in older shader
	//  models - this is why the check is present here.
	bool isShaderModel4 = GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");
	if (isShaderModel4)
	{
		synchronizePixelnToBeVertexOut(programSet);
	}

	// Grab the matching writer.
	const String& language = ShaderGenerator::getSingleton().getTargetLanguage();
	ProgramWriterIterator itWriter = mProgramWritersMap.find(language);
	ProgramWriter* programWriter = NULL;

	// No writer found -> create new one.
	if (itWriter == mProgramWritersMap.end())
	{
		programWriter = ProgramWriterManager::getSingletonPtr()->createProgramWriter(language);
		mProgramWritersMap[language] = programWriter;
	}
	else
	{
		programWriter = itWriter->second;
	}

	ProgramProcessorIterator itProcessor = mProgramProcessorsMap.find(language);
	ProgramProcessor* programProcessor = NULL;

	if (itProcessor == mProgramProcessorsMap.end())
	{
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
			"Could not find processor for language '" + language,
			"ProgramManager::createGpuPrograms");		
	}

	programProcessor = itProcessor->second;

	bool success;
	
	// Call the pre creation of GPU programs method.
	success = programProcessor->preCreateGpuPrograms(programSet);
	if (success == false)	
		return false;	
	
	// Create the vertex shader program.
	GpuProgramPtr vsGpuProgram;
	
	vsGpuProgram = createGpuProgram(programSet->getCpuVertexProgram(), 
		programWriter,
		language, 
		ShaderGenerator::getSingleton().getVertexShaderProfiles(),
		ShaderGenerator::getSingleton().getVertexShaderProfilesList(),
		ShaderGenerator::getSingleton().getShaderCachePath());

	if (vsGpuProgram.isNull())	
		return false;

	programSet->setGpuVertexProgram(vsGpuProgram);

	//update flags
	programSet->getGpuVertexProgram()->setSkeletalAnimationIncluded(
		programSet->getCpuVertexProgram()->getSkeletalAnimationIncluded());
	// Create the fragment shader program.
	GpuProgramPtr psGpuProgram;

	psGpuProgram = createGpuProgram(programSet->getCpuFragmentProgram(), 
		programWriter,
		language, 
		ShaderGenerator::getSingleton().getFragmentShaderProfiles(),
		ShaderGenerator::getSingleton().getFragmentShaderProfilesList(),
		ShaderGenerator::getSingleton().getShaderCachePath());

	if (psGpuProgram.isNull())	
		return false;

	programSet->setGpuFragmentProgram(psGpuProgram);

	// Call the post creation of GPU programs method.
	success = programProcessor->postCreateGpuPrograms(programSet);
	if (success == false)	
		return false;	

	
	return true;
	
}


//-----------------------------------------------------------------------------
void ProgramManager::bindUniformParameters(Program* pCpuProgram, const GpuProgramParametersSharedPtr& passParams)
{
	const UniformParameterList& progParams = pCpuProgram->getParameters();
	UniformParameterConstIterator itParams = progParams.begin();
	UniformParameterConstIterator itParamsEnd = progParams.end();

	// Bind each uniform parameter to its GPU parameter.
	for (; itParams != itParamsEnd; ++itParams)
	{			
		(*itParams)->bind(passParams);					
	}
}

//-----------------------------------------------------------------------------
GpuProgramPtr ProgramManager::createGpuProgram(Program* shaderProgram, 
											   ProgramWriter* programWriter,
											   const String& language,
											   const String& profiles,
											   const StringVector& profilesList,
											   const String& cachePath)
{
    stringstream sourceCodeStringStream;
	String programName;

	// Generate source code.
	programWriter->writeSourceCode(sourceCodeStringStream, shaderProgram);
    String source = sourceCodeStringStream.str();

#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
	
	// Generate program name.
	programName = generateGUID(source);

#else // Disable caching on android devices 

    // Generate program name.
    static int gpuProgramID = 0;
    programName = "RTSS_"  + StringConverter::toString(++gpuProgramID);
   
#endif
	
	if (shaderProgram->getType() == GPT_VERTEX_PROGRAM)
	{
		programName += "_VS";
	}
	else if (shaderProgram->getType() == GPT_FRAGMENT_PROGRAM)
	{
		programName += "_FS";
	}

	// Try to get program by name.
	HighLevelGpuProgramPtr pGpuProgram = HighLevelGpuProgramManager::getSingleton().getByName(programName);

	// Case the program doesn't exist yet.
	if (pGpuProgram.isNull())
	{
		// Create new GPU program.
		pGpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(programName,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, language, shaderProgram->getType());

		// Case cache directory specified -> create program from file.
		if (cachePath.empty() == false)
		{
			const String  programFullName = programName + "." + language;
			const String  programFileName = cachePath + programFullName;	
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

				outFile << source;
				outFile.close();
			}

			pGpuProgram->setSourceFile(programFullName);
		}

		// No cache directory specified -> create program from system memory.
		else
		{
			pGpuProgram->setSource(source);
		}
		
		
		pGpuProgram->setParameter("entry_point", shaderProgram->getEntryPointFunction()->getName());

		if (language == "hlsl")
		{
			// HLSL program requires specific target profile settings - we have to split the profile string.
			StringVector::const_iterator it = profilesList.begin();
			StringVector::const_iterator itEnd = profilesList.end();
			
			for (; it != itEnd; ++it)
			{
				if (GpuProgramManager::getSingleton().isSyntaxSupported(*it))
				{
					pGpuProgram->setParameter("target", *it);
					break;
				}
			}

			pGpuProgram->setParameter("enable_backwards_compatibility", "true");
			pGpuProgram->setParameter("column_major_matrices", StringConverter::toString(shaderProgram->getUseColumnMajorMatrices()));
		}
		
		pGpuProgram->setParameter("profiles", profiles);
		pGpuProgram->load();
	
		// Case an error occurred.
		if (pGpuProgram->hasCompileError())
		{
			pGpuProgram.setNull();
			return GpuProgramPtr(pGpuProgram);
		}

		// Add the created GPU program to local cache.
		if (pGpuProgram->getType() == GPT_VERTEX_PROGRAM)
		{
			mVertexShaderMap[programName] = pGpuProgram;			
		}
		else if (pGpuProgram->getType() == GPT_FRAGMENT_PROGRAM)
		{
			mFragmentShaderMap[programName] = pGpuProgram;	
		}				
	}
	
	return GpuProgramPtr(pGpuProgram);
}


//-----------------------------------------------------------------------------
String ProgramManager::generateGUID(const String& programString)
{
	//To generate a unique value this component used to use _StringHash class.
	//However when this generates a hash value it selects a maximum of 10 places within the string
	//and bases the hash value on those position. This is not good enough for this situation.
	//
	//Different programs must have unique hash values. Some programs only differ in the size of array parameters.
	//This means that only 1 or 2 letters will be changed. Using the _StringHash class in these case will, in all 
	//likelihood, produce the same values.

	unsigned int val1 = 0x9e3779b9;
	unsigned int val2 = 0x61C88646;
	unsigned int val3 = 0x9e3779b9;
	unsigned int val4 = 0x61C88646;

	//instead of generating the hash from the individual characters we will treat this string as a long 
	//integer value for faster processing. We dismiss the last non-full int.
	size_t sizeInInts = (programString.size() - 3) / 4;
	const uint32* intBuffer = (const uint32*)programString.c_str();

	size_t i = 0;
	for( ; i < sizeInInts - 2 ; i += 3) 
	{
		uint32 bufVal0 = *(intBuffer + i);
		uint32 bufVal1 = *(intBuffer + i + 1);
		uint32 bufVal2 = *(intBuffer + i + 2);
		val1 ^= (val1<<6) + (val1>>2) + bufVal0;
		val2 ^= (val2<<6) + (val2>>2) + bufVal1;
		val3 ^= (val3<<6) + (val3>>2) + bufVal2;
		//ensure greater uniqueness by having the forth int value dependent on the entire string
		val4 ^= (val4<<6) + (val4>>2) + bufVal0 + bufVal1 + bufVal2;
	}
	//read the end of the string we missed
	if (i < sizeInInts - 1)
		val1 ^= (val1<<6) + (val1>>2) + *(intBuffer + i - 1);
	if (i < sizeInInts)
		val2 ^= (val2<<6) + (val2>>2) + *(intBuffer + i);

	//Generate the guid string
	stringstream stream;
	stream.fill('0');
	stream.setf(std::ios::fixed);
	stream.setf(std::ios::hex, std::ios::basefield);
	stream.width(8); stream << val1 << "-";
	stream.width(4); stream << (uint16)(val2 >> 16) << "-";
	stream.width(4); stream << (uint16)(val2) << "-";
	stream.width(4); stream << (uint16)(val3 >> 16) << "-";
	stream.width(4); stream << (uint16)(val3);
	stream.width(8); stream << val4;
	return stream.str();
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
void ProgramManager::destroyGpuProgram(GpuProgramPtr& gpuProgram)
{		
	const String& programName = gpuProgram->getName();
	ResourcePtr res			  = HighLevelGpuProgramManager::getSingleton().getByName(programName);	

	if (res.isNull() == false)
	{		
		HighLevelGpuProgramManager::getSingleton().remove(programName);
	}
}

//-----------------------------------------------------------------------
void ProgramManager::synchronizePixelnToBeVertexOut( ProgramSet* programSet )
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	// first find the vertex shader
	ShaderFunctionConstIterator itFunction ;
	Function* vertexMain = NULL;
	Function* pixelMain = NULL;

	// find vertex shader main
	{
		const ShaderFunctionList& functionList = vsProgram->getFunctions();
		for (itFunction=functionList.begin(); itFunction != functionList.end(); ++itFunction)
		{
			Function* curFunction = *itFunction;
			if (curFunction->getFunctionType() == Function::FFT_VS_MAIN)
			{
				vertexMain = curFunction;
				break;
			}
		}
	}

	// find pixel shader main
	{
		const ShaderFunctionList& functionList = psProgram->getFunctions();
		for (itFunction=functionList.begin(); itFunction != functionList.end(); ++itFunction)
		{
			Function* curFunction = *itFunction;
			if (curFunction->getFunctionType() == Function::FFT_PS_MAIN)
			{
				pixelMain = curFunction;
				break;
			}
		}
	}

    if(pixelMain)
    {
        // save the pixel program original input parameters
        const ShaderParameterList pixelOriginalInParams = pixelMain->getInputParameters();

        // set the pixel Input to be the same as the vertex prog output
        pixelMain->deleteAllInputParameters();

        // Loop the vertex shader output parameters and make sure that
        //   all of them exist in the pixel shader input.
        // If the parameter type exist in the original output - use it
        // If the parameter doesn't exist - use the parameter from the 
        //   vertex shader input.
        // The order will be based on the vertex shader parameters order 
        // Write output parameters.
        ShaderParameterConstIterator it;
        if(vertexMain)
        {
            const ShaderParameterList& outParams = vertexMain->getOutputParameters();
            for (it=outParams.begin(); it != outParams.end(); ++it)
            {
                ParameterPtr curOutParemter = *it;
                ParameterPtr paramToAdd = Function::getParameterBySemantic(
                    pixelOriginalInParams, 
                    curOutParemter->getSemantic(), 
                    curOutParemter->getIndex());

                if (paramToAdd.isNull())
                {
                    // param not found - we will add the one from the vertex shader
                    paramToAdd = curOutParemter; 
                }

                pixelMain->addInputParameter(paramToAdd);
            }
        }
    }
}

/** @} */
/** @} */
}
}
