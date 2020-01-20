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
void ProgramManager::releasePrograms(const ProgramSet* programSet)
{
    GpuProgramPtr vsProgram(programSet->getGpuProgram(GPT_VERTEX_PROGRAM));
    GpuProgramPtr psProgram(programSet->getGpuProgram(GPT_FRAGMENT_PROGRAM));

    GpuProgramsMapIterator itVsGpuProgram = !vsProgram ? mVertexShaderMap.end() : mVertexShaderMap.find(vsProgram->getName());
    GpuProgramsMapIterator itFsGpuProgram = !psProgram ? mFragmentShaderMap.end() : mFragmentShaderMap.find(psProgram->getName());

    if (itVsGpuProgram != mVertexShaderMap.end())
    {
        if (itVsGpuProgram->second.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
        {
            destroyGpuProgram(itVsGpuProgram->second);
            mVertexShaderMap.erase(itVsGpuProgram);
        }
    }

    if (itFsGpuProgram != mFragmentShaderMap.end())
    {
        if (itFsGpuProgram->second.use_count() == ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1)
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

size_t ProgramManager::getShaderCount(GpuProgramType type) const
{
    switch(type)
    {
    case GPT_VERTEX_PROGRAM:
        return mVertexShaderMap.size();
    case GPT_FRAGMENT_PROGRAM:
        return mFragmentShaderMap.size();
    default:
        return 0;
    }
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
void ProgramManager::createGpuPrograms(ProgramSet* programSet)
{
    // Before we start we need to make sure that the pixel shader input
    //  parameters are the same as the vertex output, this required by 
    //  shader models 4 and 5.
    // This change may incrase the number of register used in older shader
    //  models - this is why the check is present here.
    bool isVs4 = GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");
    if (isVs4)
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
    
    // Call the pre creation of GPU programs method.
    if (!programProcessor->preCreateGpuPrograms(programSet))
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "preCreateGpuPrograms failed");
    
    // Create the shader programs
    for(auto type : {GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM})
    {
        auto gpuProgram = createGpuProgram(programSet->getCpuProgram(type), programWriter, language,
                                           ShaderGenerator::getSingleton().getShaderProfiles(type),
                                           ShaderGenerator::getSingleton().getShaderProfilesList(type),
                                           ShaderGenerator::getSingleton().getShaderCachePath());

        OgreAssert(gpuProgram, "gpu program could not be created");
        programSet->setGpuProgram(gpuProgram);
    }

    //update flags
    programSet->getGpuProgram(GPT_VERTEX_PROGRAM)->setSkeletalAnimationIncluded(
        programSet->getCpuProgram(GPT_VERTEX_PROGRAM)->getSkeletalAnimationIncluded());

    // Call the post creation of GPU programs method.
    if(!programProcessor->postCreateGpuPrograms(programSet))
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "postCreateGpuPrograms failed");
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

    // Generate source code.
    programWriter->writeSourceCode(sourceCodeStringStream, shaderProgram);
    String source = sourceCodeStringStream.str();

    // Generate program name.
    String programName = generateHash(source, shaderProgram->getPreprocessorDefines());

    if (shaderProgram->getType() == GPT_VERTEX_PROGRAM)
    {
        programName += "_VS";
    }
    else if (shaderProgram->getType() == GPT_FRAGMENT_PROGRAM)
    {
        programName += "_FS";
    }

    // Try to get program by name.
    HighLevelGpuProgramPtr pGpuProgram =
        HighLevelGpuProgramManager::getSingleton().getByName(
            programName, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);

    if(pGpuProgram) {
        return static_pointer_cast<GpuProgram>(pGpuProgram);
    }

    // Case the program doesn't exist yet.
    // Create new GPU program.
    pGpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(programName,
        ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, language, shaderProgram->getType());

    // Case cache directory specified -> create program from file.
    if (!cachePath.empty())
    {
        const String  programFullName = programName + "." + language;
        const String  programFileName = cachePath + programFullName;
        std::ifstream programFile;

        // Check if program file already exist.
        programFile.open(programFileName.c_str());

        // Case we have to write the program to a file.
        if (!programFile)
        {
            std::ofstream outFile(programFileName.c_str());

            if (!outFile)
                return GpuProgramPtr();

            outFile << source;
            outFile.close();
        }
        else
        {
            // use program file version
            StringStream buffer;
            programFile >> buffer.rdbuf();
            source = buffer.str();
        }
    }

    pGpuProgram->setSource(source);
    pGpuProgram->setPreprocessorDefines(shaderProgram->getPreprocessorDefines());
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
        //! [debug_break]
        pGpuProgram.reset();
        //! [debug_break]
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
    
    return static_pointer_cast<GpuProgram>(pGpuProgram);
}


//-----------------------------------------------------------------------------
String ProgramManager::generateHash(const String& programString, const String& defines)
{
    //Different programs must have unique hash values.
    uint32_t hash[4];
    uint32_t seed = FastHash(defines.c_str(), defines.size());
    MurmurHash3_128(programString.c_str(), programString.size(), seed, hash);

    //Generate the string
    return StringUtil::format("%08x%08x%08x%08x", hash[0], hash[1], hash[2], hash[3]);
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
    HighLevelGpuProgramPtr res           = dynamic_pointer_cast<HighLevelGpuProgram>(gpuProgram);

    if (res)
    {
        HighLevelGpuProgramManager::getSingleton().remove(res);
    }
}

//-----------------------------------------------------------------------
void ProgramManager::synchronizePixelnToBeVertexOut( ProgramSet* programSet )
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    // first find the vertex shader
    ShaderFunctionConstIterator itFunction ;
    Function* vertexMain = vsProgram->getEntryPointFunction();
    Function* pixelMain = psProgram->getEntryPointFunction();;

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
                ParameterPtr paramToAdd = Function::_getParameterBySemantic(
                    pixelOriginalInParams, 
                    curOutParemter->getSemantic(), 
                    curOutParemter->getIndex());

                if (!paramToAdd)
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
