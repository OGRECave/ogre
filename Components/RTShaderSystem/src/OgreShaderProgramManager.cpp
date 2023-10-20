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
    for(auto t : {GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM})
    {
        const auto& prg = programSet->getGpuProgram(t);
        if(prg.use_count() > ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 2)
            continue;

	const auto it = std::find(mShaderList.begin(), mShaderList.end(), prg);
        // TODO: this check should not be necessary, but we observed strange prg.use_count() in the wild
        if(it != mShaderList.end())
            mShaderList.erase(it);
        GpuProgramManager::getSingleton().remove(prg);
    }
}
size_t ProgramManager::getShaderCount(GpuProgramType type) const
{
    size_t count = 0;

    for(const auto& s : mShaderList)
    {
        count += size_t(s->getType() == type);
    }

    return count;
}
//-----------------------------------------------------------------------------
void ProgramManager::flushGpuProgramsCache()
{
    for(auto& s : mShaderList)
    {
        GpuProgramManager::getSingleton().remove(s);
    }
    mShaderList.clear();
}
//-----------------------------------------------------------------------------
void ProgramManager::createDefaultProgramProcessors()
{
    // Add standard shader processors
    mDefaultProgramProcessors.push_back(OGRE_NEW ProgramProcessor);
}

//-----------------------------------------------------------------------------
void ProgramManager::destroyDefaultProgramProcessors()
{
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
    matchVStoPSInterface(programSet);

    // Grab the matching writer.
    const String& language = ShaderGenerator::getSingleton().getTargetLanguage();

    auto programWriter = ProgramWriterManager::getSingleton().getProgramWriter(language);

    ProgramProcessor* programProcessor = mDefaultProgramProcessors.front();
    
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

    // update VS flags
    auto gpuVs = programSet->getGpuProgram(GPT_VERTEX_PROGRAM);
    auto cpuVs = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    gpuVs->setSkeletalAnimationIncluded(cpuVs->getSkeletalAnimationIncluded());
    gpuVs->setInstancingIncluded(cpuVs->getInstancingIncluded());

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
    auto pGpuProgram = GpuProgramManager::getSingleton().getByName(programName, RGN_INTERNAL);

    if(pGpuProgram) {
        return pGpuProgram;
    }

    // Case the program doesn't exist yet.
    // Create new GPU program.
    pGpuProgram =
        GpuProgramManager::getSingleton().createProgram(programName, RGN_INTERNAL, language, shaderProgram->getType());

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

    // Add the created GPU program to local index
    mShaderList.push_back(pGpuProgram);
    
    return pGpuProgram;
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
void ProgramManager::addProgramProcessor(const String& lang, ProgramProcessor* processor) {}
void ProgramManager::removeProgramProcessor(const String& lang) {}

//-----------------------------------------------------------------------
void ProgramManager::matchVStoPSInterface( ProgramSet* programSet )
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
