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
}

//-----------------------------------------------------------------------------
ProgramManager::~ProgramManager()
{
    flushGpuProgramsCache();
    destroyDefaultProgramProcessors();  
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
void ProgramManager::createDefaultProgramProcessors()
{
    // Add standard shader processors
    mDefaultProgramProcessors.push_back(OGRE_NEW GLSLProgramProcessor);
    addProgramProcessor("glsles", mDefaultProgramProcessors.back());
    addProgramProcessor("glslang", mDefaultProgramProcessors.back());
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    addProgramProcessor("glsl", mDefaultProgramProcessors.back());
    mDefaultProgramProcessors.push_back(OGRE_NEW HLSLProgramProcessor);
    addProgramProcessor("hlsl", mDefaultProgramProcessors.back());
#endif
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyDefaultProgramProcessors()
{
    // removing unknown is not an error
    for(auto lang : {"glsl", "glsles", "glslang", "hlsl"})
        removeProgramProcessor(lang);
    for (auto processor : mDefaultProgramProcessors) {
        OGRE_DELETE processor;
    }
    mDefaultProgramProcessors.clear();
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

    auto programWriter = ProgramWriterManager::getSingleton().getProgramWriter(language);

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
                                           ShaderGenerator::getSingleton().getShaderCachePath());
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
                                               const String& cachePath)
{
    std::stringstream sourceCodeStringStream;

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
        const String  programFullName = programName + "." + programWriter->getTargetLanguage();
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
    pGpuProgram->setParameter("preprocessor_defines", shaderProgram->getPreprocessorDefines());
    pGpuProgram->setParameter("entry_point", "main");

    if (language == "hlsl")
    {
        pGpuProgram->setParameter("target", profiles);
        pGpuProgram->setParameter("enable_backwards_compatibility", "true");
        pGpuProgram->setParameter("column_major_matrices", StringConverter::toString(shaderProgram->getUseColumnMajorMatrices()));
    }
    else if (language == "glsl")
    {
        auto* rs = Root::getSingleton().getRenderSystem();
        if( rs && rs->getNativeShadingLanguageVersion() >= 420)
            pGpuProgram->setParameter("has_sampler_binding", "true");
    }

    pGpuProgram->load();

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
void ProgramManager::addProgramProcessor(const String& lang, ProgramProcessor* processor)
{
    
    ProgramProcessorIterator itFind = mProgramProcessorsMap.find(lang);

    if (itFind != mProgramProcessorsMap.end())
    {
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "A processor for language '" + lang + "' already exists.");
    }

    mProgramProcessorsMap[lang] = processor;
}

//-----------------------------------------------------------------------------
void ProgramManager::removeProgramProcessor(const String& lang)
{
    ProgramProcessorIterator itFind = mProgramProcessorsMap.find(lang);

    if (itFind != mProgramProcessorsMap.end())
        mProgramProcessorsMap.erase(itFind);

}

//-----------------------------------------------------------------------------
void ProgramManager::destroyGpuProgram(GpuProgramPtr& gpuProgram)
{       
    GpuProgramManager::getSingleton().remove(gpuProgram);
}

//-----------------------------------------------------------------------
void ProgramManager::synchronizePixelnToBeVertexOut( ProgramSet* programSet )
{
    Function* vertexMain = programSet->getCpuProgram(GPT_VERTEX_PROGRAM)->getMain();
    Function* pixelMain = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM)->getMain();

    // save the pixel program original input parameters
    auto pixelOriginalInParams = pixelMain->getInputParameters();

    // set the pixel Input to be the same as the vertex prog output
    pixelMain->deleteAllInputParameters();

    // Loop the vertex shader output parameters and make sure that
    //   all of them exist in the pixel shader input.
    // If the parameter type exist in the original output - use it
    // If the parameter doesn't exist - use the parameter from the
    //   vertex shader input.
    // The order will be based on the vertex shader parameters order
    // Write output parameters.
    for (const auto& curOutParemter : vertexMain->getOutputParameters())
    {
        ParameterPtr paramToAdd = Function::_getParameterBySemantic(
            pixelOriginalInParams, curOutParemter->getSemantic(), curOutParemter->getIndex());

        pixelOriginalInParams.erase(
            std::remove(pixelOriginalInParams.begin(), pixelOriginalInParams.end(), paramToAdd),
            pixelOriginalInParams.end());

        if (!paramToAdd)
        {
            // param not found - we will add the one from the vertex shader
            paramToAdd = curOutParemter;
        }

        pixelMain->addInputParameter(paramToAdd);
    }

    for(const auto& paramToAdd : pixelOriginalInParams)
    {
        pixelMain->addInputParameter(paramToAdd);
    }
}

/** @} */
/** @} */
}
}
