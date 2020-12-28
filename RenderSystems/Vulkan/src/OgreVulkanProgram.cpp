/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

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
#include "OgreVulkanProgram.h"

#include "OgreLogManager.h"
#include "OgreProfiler.h"
#include "OgreVulkanDevice.h"
#include "OgreVulkanGpuProgramManager.h"
#include "OgreVulkanMappings.h"
#include "OgreVulkanRootLayout.h"
#include "Vao/OgreVulkanVaoManager.h"

#include "OgreStringConverter.h"
#include "OgreVulkanDelayedFuncs.h"
#include "OgreVulkanUtils.h"
#include "SPIRV-Reflect/spirv_reflect.h"

#include "OgreRenderSystemCapabilities.h"
#include "OgreRoot.h"
#include "OgreVulkanGlslangHeader.h"

#include "glslang/SPIRV/Logger.h"

// Inclusion of SPIRV headers triggers lots of C++11 errors we don't care
namespace glslang
{
    struct SpvOptions
    {
        SpvOptions() :
            generateDebugInfo( false ),
            stripDebugInfo( false ),
            disableOptimizer( true ),
            optimizeSize( false ),
            disassemble( false ),
            validate( false )
        {
        }
        bool generateDebugInfo;
        bool stripDebugInfo;
        bool disableOptimizer;
        bool optimizeSize;
        bool disassemble;
        bool validate;
    };

    void GetSpirvVersion( std::string & );
    int GetSpirvGeneratorVersion();
    void GlslangToSpv( const glslang::TIntermediate &intermediate, std::vector<unsigned int> &spirv,
                       SpvOptions *options = 0 );
    void GlslangToSpv( const glslang::TIntermediate &intermediate, std::vector<unsigned int> &spirv,
                       spv::SpvBuildLogger *logger, SpvOptions *options = 0 );
    void OutputSpvBin( const std::vector<unsigned int> &spirv, const char *baseName );
    void OutputSpvHex( const std::vector<unsigned int> &spirv, const char *baseName,
                       const char *varName );

}  // namespace glslang

namespace Ogre
{
    struct FreeModuleOnDestructor
    {
        SpvReflectShaderModule *module;

        FreeModuleOnDestructor( SpvReflectShaderModule *_module ) : module( _module ) {}
        ~FreeModuleOnDestructor()
        {
            if( module )
            {
                spvReflectDestroyShaderModule( module );
                module = 0;
            }
        }

    private:
        // Prevent being able to copy this object
        FreeModuleOnDestructor( const FreeModuleOnDestructor & );
        FreeModuleOnDestructor &operator=( const FreeModuleOnDestructor & );
    };

    //-----------------------------------------------------------------------
    VulkanProgram::CmdPreprocessorDefines VulkanProgram::msCmdPreprocessorDefines;
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    VulkanProgram::VulkanProgram( ResourceManager *creator, const String &name, ResourceHandle handle,
                                  const String &group, bool isManual, ManualResourceLoader *loader,
                                  VulkanDevice *device, String &languageName ) :
        HighLevelGpuProgram( creator, name, handle, group, isManual, loader ),
        mDevice( device ),
        mRootLayout( 0 ),
        mShaderModule( 0 ),
        mNumSystemGenVertexInputs( 0u ),
        mCustomRootLayout( false ),
        mReplaceVersionMacro( false ),
        mCompiled( false ),
        mConstantsBytesToWrite( 0 )
    {
        if( createParamDictionary( "VulkanProgram" ) )
        {
            setupBaseParamDictionary();
            ParamDictionary *dict = getParamDictionary();

            dict->addParameter(
                ParameterDef( "preprocessor_defines", "Preprocessor defines use to compile the program.",
                              PT_STRING ),
                &msCmdPreprocessorDefines );
        }

        // Manually assign language now since we use it immediately
        mSyntaxCode = languageName;
        mShaderSyntax = ( languageName.find( "hlsl" ) != String::npos ) ? HLSL : GLSL;
        mDrawIdLocation = ( mShaderSyntax == GLSL ) ? 15 : 0;
    }
    //---------------------------------------------------------------------------
    VulkanProgram::~VulkanProgram()
    {
        // Have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if( isLoaded() )
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }
    //-----------------------------------------------------------------------
    uint32 VulkanProgram::getEshLanguage( void ) const
    {
        switch( mType )
        {
        // clang-format off
        case GPT_VERTEX_PROGRAM:    return EShLangVertex;
        case GPT_FRAGMENT_PROGRAM:  return EShLangFragment;
        case GPT_GEOMETRY_PROGRAM:  return EShLangGeometry;
        case GPT_HULL_PROGRAM:      return EShLangTessControl;
        case GPT_DOMAIN_PROGRAM:    return EShLangTessEvaluation;
        case GPT_COMPUTE_PROGRAM:   return EShLangCompute;
            // clang-format on
        }

        return EShLangFragment;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::extractRootLayoutFromSource( void )
    {
        if( mRootLayout )
            return;  // Programmatically specified

        const String rootLayoutHeader = "## ROOT LAYOUT BEGIN";
        const String rootLayoutFooter = "## ROOT LAYOUT END";

        size_t posStart = mSource.find( rootLayoutHeader );
        if( posStart == String::npos )
        {
            LogManager::getSingleton().logMessage( "Error " + mName + ": failed to find required '" +
                                                   rootLayoutHeader + "'" );
            mCompileError = true;
            return;
        }
        posStart += rootLayoutHeader.size();
        const size_t posEnd = mSource.find( "## ROOT LAYOUT END", posStart );
        if( posEnd == String::npos )
        {
            LogManager::getSingleton().logMessage( "Error " + mName + ": failed to find required '" +
                                                   rootLayoutFooter + "'" );
            mCompileError = true;
            return;
        }

        VulkanGpuProgramManager *vulkanProgramManager =
            static_cast<VulkanGpuProgramManager *>( VulkanGpuProgramManager::getSingletonPtr() );
        mRootLayout = vulkanProgramManager->getRootLayout(
            mSource.substr( posStart, posEnd - posStart ).c_str(), mType == GPT_COMPUTE_PROGRAM, mName );
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::initGlslResources( TBuiltInResource &resources )
    {
        const RenderSystem *rs = Root::getSingleton().getRenderSystem();
        const RenderSystemCapabilities *caps = rs->getCapabilities();

        const uint32 *maxThreadsPerThreadGroupAxis = caps->getMaxThreadsPerThreadgroupAxis();

        resources.maxLights = 32;
        resources.maxClipPlanes = 6;
        resources.maxTextureUnits = 32;
        resources.maxTextureCoords = 32;
        resources.maxVertexAttribs = 64;
        resources.maxVertexUniformComponents = 4096;
        resources.maxVaryingFloats = 64;
        resources.maxVertexTextureImageUnits = 32;
        resources.maxCombinedTextureImageUnits = 80;
        resources.maxTextureImageUnits = 32;
        resources.maxFragmentUniformComponents = 4096;
        resources.maxDrawBuffers = 32;
        resources.maxVertexUniformVectors = 128;
        resources.maxVaryingVectors = 8;
        resources.maxFragmentUniformVectors = 16;
        resources.maxVertexOutputVectors = 16;
        resources.maxFragmentInputVectors = 15;
        resources.minProgramTexelOffset = -8;
        resources.maxProgramTexelOffset = 7;
        resources.maxClipDistances = 8;
        resources.maxComputeWorkGroupCountX = 65535;
        resources.maxComputeWorkGroupCountY = 65535;
        resources.maxComputeWorkGroupCountZ = 65535;
        resources.maxComputeWorkGroupSizeX = static_cast<int>( maxThreadsPerThreadGroupAxis[0] );
        resources.maxComputeWorkGroupSizeY = static_cast<int>( maxThreadsPerThreadGroupAxis[1] );
        resources.maxComputeWorkGroupSizeZ = static_cast<int>( maxThreadsPerThreadGroupAxis[2] );
        resources.maxComputeUniformComponents = 1024;
        resources.maxComputeTextureImageUnits = 16;
        resources.maxComputeImageUniforms = 8;
        resources.maxComputeAtomicCounters = 8;
        resources.maxComputeAtomicCounterBuffers = 1;
        resources.maxVaryingComponents = 60;
        resources.maxVertexOutputComponents = 64;
        resources.maxGeometryInputComponents = 64;
        resources.maxGeometryOutputComponents = 128;
        resources.maxFragmentInputComponents = 128;
        resources.maxImageUnits = 8;
        resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        resources.maxCombinedShaderOutputResources = 8;
        resources.maxImageSamples = 0;
        resources.maxVertexImageUniforms = 0;
        resources.maxTessControlImageUniforms = 0;
        resources.maxTessEvaluationImageUniforms = 0;
        resources.maxGeometryImageUniforms = 0;
        resources.maxFragmentImageUniforms = 8;
        resources.maxCombinedImageUniforms = 8;
        resources.maxGeometryTextureImageUnits = 16;
        resources.maxGeometryOutputVertices = 256;
        resources.maxGeometryTotalOutputComponents = 1024;
        resources.maxGeometryUniformComponents = 1024;
        resources.maxGeometryVaryingComponents = 64;
        resources.maxTessControlInputComponents = 128;
        resources.maxTessControlOutputComponents = 128;
        resources.maxTessControlTextureImageUnits = 16;
        resources.maxTessControlUniformComponents = 1024;
        resources.maxTessControlTotalOutputComponents = 4096;
        resources.maxTessEvaluationInputComponents = 128;
        resources.maxTessEvaluationOutputComponents = 128;
        resources.maxTessEvaluationTextureImageUnits = 16;
        resources.maxTessEvaluationUniformComponents = 1024;
        resources.maxTessPatchComponents = 120;
        resources.maxPatchVertices = 32;
        resources.maxTessGenLevel = 64;
        resources.maxViewports = 16;
        resources.maxVertexAtomicCounters = 0;
        resources.maxTessControlAtomicCounters = 0;
        resources.maxTessEvaluationAtomicCounters = 0;
        resources.maxGeometryAtomicCounters = 0;
        resources.maxFragmentAtomicCounters = 8;
        resources.maxCombinedAtomicCounters = 8;
        resources.maxAtomicCounterBindings = 1;
        resources.maxVertexAtomicCounterBuffers = 0;
        resources.maxTessControlAtomicCounterBuffers = 0;
        resources.maxTessEvaluationAtomicCounterBuffers = 0;
        resources.maxGeometryAtomicCounterBuffers = 0;
        resources.maxFragmentAtomicCounterBuffers = 1;
        resources.maxCombinedAtomicCounterBuffers = 1;
        resources.maxAtomicCounterBufferSize = 16384;
        resources.maxTransformFeedbackBuffers = 4;
        resources.maxTransformFeedbackInterleavedComponents = 64;
        resources.maxCullDistances = 8;
        resources.maxCombinedClipAndCullDistances = 8;
        resources.maxSamples = 4;
        resources.maxMeshOutputVerticesNV = 256;
        resources.maxMeshOutputPrimitivesNV = 512;
        resources.maxMeshWorkGroupSizeX_NV = 32;
        resources.maxMeshWorkGroupSizeY_NV = 1;
        resources.maxMeshWorkGroupSizeZ_NV = 1;
        resources.maxTaskWorkGroupSizeX_NV = 32;
        resources.maxTaskWorkGroupSizeY_NV = 1;
        resources.maxTaskWorkGroupSizeZ_NV = 1;
        resources.maxMeshViewCountNV = 4;
        resources.limits.nonInductiveForLoops = 1;
        resources.limits.whileLoops = 1;
        resources.limits.doWhileLoops = 1;
        resources.limits.generalUniformIndexing = 1;
        resources.limits.generalAttributeMatrixVectorIndexing = 1;
        resources.limits.generalVaryingIndexing = 1;
        resources.limits.generalSamplerIndexing = 1;
        resources.limits.generalVariableIndexing = 1;
        resources.limits.generalConstantMatrixVectorIndexing = 1;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::loadFromSource( void ) { compile( true ); }
    //-----------------------------------------------------------------------
    /**
    @brief VulkanProgram::replaceVersionMacros
        Finds the first occurrence of "ogre_glsl_ver_xxx" and replaces it with "450"
    */
    void VulkanProgram::replaceVersionMacros( void )
    {
        const String matchStr = "ogre_glsl_ver_";
        const size_t pos = mSource.find( matchStr );
        if( pos != String::npos && mSource.size() - pos >= 3u )
        {
            mSource.erase( pos, matchStr.size() );
            mSource[pos + 0] = '4';
            mSource[pos + 1] = '5';
            mSource[pos + 2] = '0';
        }
    }
    //-----------------------------------------------------------------------
    struct SemanticMacro
    {
        const char *nameStr;
        VertexElementSemantic semantic;
        uint8 count;
        SemanticMacro( const char *_nameStr, VertexElementSemantic _semantic, uint8 _count = 1u ) :
            nameStr( _nameStr ),
            semantic( _semantic ),
            count( _count )
        {
        }
    };
    // The names match our HLSL bindings
    static const SemanticMacro c_semanticMacros[] = {
        SemanticMacro( "OGRE_POSITION", VES_POSITION ),
        SemanticMacro( "OGRE_BLENDWEIGHT", VES_BLEND_WEIGHTS ),
        SemanticMacro( "OGRE_BLENDINDICES", VES_BLEND_INDICES ),
        SemanticMacro( "OGRE_BLENDWEIGHT2", VES_BLEND_WEIGHTS2 ),
        SemanticMacro( "OGRE_BLENDINDICES2", VES_BLEND_INDICES2 ),
        SemanticMacro( "OGRE_NORMAL", VES_NORMAL ),
        SemanticMacro( "OGRE_DIFFUSE", VES_DIFFUSE ),
        SemanticMacro( "OGRE_SPECULAR", VES_SPECULAR ),
        SemanticMacro( "OGRE_TEXCOORD", VES_TEXTURE_COORDINATES, 8u ),
        SemanticMacro( "OGRE_BINORMAL", VES_BINORMAL ),
        SemanticMacro( "OGRE_TANGENT", VES_TANGENT ),
    };
    void VulkanProgram::addVertexSemanticsToPreamble( String &inOutPreamble ) const
    {
        // This code could be baked at compile time...
        char tmpBuffer[768];
        Ogre::LwString preamble( Ogre::LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );

        for( size_t i = 0u; i < sizeof( c_semanticMacros ) / sizeof( c_semanticMacros[0] ); ++i )
        {
            uint32 attrIdx = VulkanVaoManager::getAttributeIndexFor( c_semanticMacros[i].semantic );
            if( c_semanticMacros[i].count == 1u )
            {
                preamble.a( "#define ", c_semanticMacros[i].nameStr, " ", "location = ", attrIdx, "\n" );
            }
            else
            {
                for( uint8 j = 0u; j < c_semanticMacros[i].count; ++j )
                {
                    preamble.a( "#define ", c_semanticMacros[i].nameStr, j, " ",
                                "location = ", attrIdx + j, "\n" );
                }
            }
        }

        preamble.a( "#define OGRE_DRAWID location = 15u\n" );

        inOutPreamble += preamble.c_str();
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::addPreprocessorToPreamble( String &inOutPreamble ) const
    {
        String preamble;
        preamble.swap( inOutPreamble );
        // Pass all user-defined macros to preprocessor
        if( !mPreprocessorDefines.empty() )
        {
            String::size_type pos = 0u;
            while( pos != String::npos )
            {
                // Find delims
                String::size_type endPos = mPreprocessorDefines.find_first_of( ";,=", pos );
                if( endPos != String::npos )
                {
                    String::size_type macro_name_start = pos;
                    size_t macro_name_len = endPos - pos;
                    pos = endPos;

                    // Check definition part
                    if( mPreprocessorDefines[pos] == '=' )
                    {
                        // Set up a definition, skip delim
                        ++pos;
                        String::size_type macro_val_start = pos;
                        size_t macro_val_len;

                        endPos = mPreprocessorDefines.find_first_of( ";,", pos );
                        if( endPos == String::npos )
                        {
                            macro_val_len = mPreprocessorDefines.size() - pos;
                            pos = endPos;
                        }
                        else
                        {
                            macro_val_len = endPos - pos;
                            pos = endPos + 1u;
                        }
                        preamble += "#define " +
                                    mPreprocessorDefines.substr( macro_name_start, macro_name_len ) +
                                    " " + mPreprocessorDefines.substr( macro_val_start, macro_val_len ) +
                                    "\n";
                    }
                    else
                    {
                        // No definition part, define as "1"
                        ++pos;
                        preamble += "#define " +
                                    mPreprocessorDefines.substr( macro_name_start, macro_name_len ) +
                                    " 1\n";
                    }
                }
                else
                {
                    if( pos < mPreprocessorDefines.size() )
                    {
                        preamble +=
                            "#define " +
                            mPreprocessorDefines.substr( pos, mPreprocessorDefines.size() - pos ) +
                            " 1\n";
                    }
                    pos = endPos;
                }
            }
        }
        preamble.swap( inOutPreamble );
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::setRootLayout( GpuProgramType type, const RootLayout &rootLayout )
    {
        mCustomRootLayout = true;
        HighLevelGpuProgram::setRootLayout( type, rootLayout );

        VulkanGpuProgramManager *vulkanProgramManager =
            static_cast<VulkanGpuProgramManager *>( VulkanGpuProgramManager::getSingletonPtr() );
        mRootLayout = vulkanProgramManager->getRootLayout( rootLayout );
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::unsetRootLayout( void )
    {
        mRootLayout = 0;
        mCustomRootLayout = false;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::setReplaceVersionMacro( bool bReplace ) { mReplaceVersionMacro = bReplace; }
    //-----------------------------------------------------------------------
    void VulkanProgram::getPreamble( String &preamble ) const
    {
        if( mShaderSyntax == GLSL )
        {
            preamble +=
                "#extension GL_EXT_samplerless_texture_functions : require\n"
                "#define vulkan_layout layout\n"
                "#define vulkan( x ) x\n"
                "#define vk_comma ,\n"
                "#define vkSampler1D sampler1D\n"
                "#define vkSampler2D sampler2D\n"
                "#define vkSampler2DArray sampler2DArray\n"
                "#define vkSampler3D sampler3D\n"
                "#define vkSamplerCube samplerCube\n";
        }
        else
        {
            preamble += "#define vulkan( x ) x\n";
        }

        mRootLayout->generateRootLayoutMacros( mType, mShaderSyntax, preamble );
        if( mType == GPT_VERTEX_PROGRAM )
            addVertexSemanticsToPreamble( preamble );
        addPreprocessorToPreamble( preamble );
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::debugDump( String &outString )
    {
        outString += mName;
        outString += "\n";
        mRootLayout->dump( outString );
        outString += "\n";
        getPreamble( outString );
        outString += "\n";
        outString += mSource;
    }
    //-----------------------------------------------------------------------
    bool VulkanProgram::compile( const bool checkErrors )
    {
        mCompiled = false;
        mCompileError = false;

        extractRootLayoutFromSource();

        const EShLanguage stage = static_cast<EShLanguage>( getEshLanguage() );
        glslang::TShader shader( stage );

        TBuiltInResource resources;
        memset( &resources, 0, sizeof( resources ) );
        initGlslResources( resources );

        // Enable SPIR-V and Vulkan rules when parsing GLSL
        EShMessages messages = ( EShMessages )( EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules );
        if( mShaderSyntax == HLSL )
        {
            // glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
            // glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;
            messages = ( EShMessages )( EShMsgDefault | EShMsgSpvRules | EShMsgReadHlsl );
            shader.setEnvInput( glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 100 );
            // shader.setEnvClient( glslang::EShClientVulkan, VulkanClientVersion );
            // shader.setEnvTarget( glslang::EShTargetSpv, TargetVersion );
            shader.setEntryPoint( "main" );
        }

#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH
        messages = ( EShMessages )( messages | EShMsgDebugInfo );
#endif

        const char *sourceCString = mSource.c_str();
        shader.setStrings( &sourceCString, 1 );

        if( !mCompileError )
        {
            if( mReplaceVersionMacro )
                replaceVersionMacros();

            String preamble;
            getPreamble( preamble );
            shader.setPreamble( preamble.c_str() );

            if( !shader.parse( &resources, 450, false, messages ) )
            {
                LogManager::getSingleton().logMessage( "Vulkan GLSL compiler error in " + mName + ":\n" +
                                                       shader.getInfoLog() + "\nDEBUG LOG:\n" +
                                                       shader.getInfoDebugLog() );
                mCompileError = true;
            }
        }

        // Add shader to new program object.
        glslang::TProgram program;
        if( !mCompileError )
        {
            program.addShader( &shader );

            // Link program.
            if( !program.link( messages ) )
            {
                LogManager::getSingleton().logMessage( "Vulkan GLSL linker error in " + mName + ":\n" +
                                                       program.getInfoLog() + "\nDEBUG LOG:\n" +
                                                       program.getInfoDebugLog() );
                mCompileError = true;
            }
        }

        glslang::TIntermediate *intermediate = 0;
        if( !mCompileError )
        {
            // Save any info log that was generated.
            if( shader.getInfoLog() )
            {
                LogManager::getSingleton().logMessage(
                    "Vulkan GLSL shader messages " + mName + ":\n" + shader.getInfoLog(), LML_TRIVIAL );
            }
            if( program.getInfoLog() )
            {
                LogManager::getSingleton().logMessage(
                    "Vulkan GLSL linker messages " + mName + ":\n" + program.getInfoLog(), LML_TRIVIAL );
            }

            intermediate = program.getIntermediate( stage );

            // Translate to SPIRV.
            if( !intermediate )
            {
                LogManager::getSingleton().logMessage( "Vulkan GLSL failed to get intermediate code " +
                                                       mName );
                mCompileError = true;
            }
        }

        mSpirv.clear();

        if( !mCompileError )
        {
            spv::SpvBuildLogger logger;
            glslang::SpvOptions opts;
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_HIGH && OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
            opts.disableOptimizer = true;
            opts.generateDebugInfo = true;
#else
            opts.disableOptimizer = false;
            opts.optimizeSize = true;
            opts.generateDebugInfo = false;
#endif
            glslang::GlslangToSpv( *intermediate, mSpirv, &logger, &opts );

            LogManager::getSingleton().logMessage(
                "Vulkan GLSL to SPIRV " + mName + ":\n" + logger.getAllMessages(), LML_TRIVIAL );
        }

        mCompiled = !mCompileError;

        if( !mCompileError )
            LogManager::getSingleton().logMessage( "Shader " + mName + " compiled successfully." );

        if( !mCompiled && checkErrors )
        {
            String dumpStr;
            mRootLayout->dump( dumpStr );
            dumpStr += "\n";
            getPreamble( dumpStr );

            LogManager::getSingleton().logMessage( dumpStr, LML_CRITICAL );

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         ( ( mType == GPT_VERTEX_PROGRAM ) ? "Vertex Program " : "Fragment Program " ) +
                             mName + " failed to compile. See compile log above for details.",
                         "VulkanProgram::compile" );
        }

        if( mCompiled && !mSpirv.empty() )
        {
            VkShaderModuleCreateInfo moduleCi;
            makeVkStruct( moduleCi, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO );
            moduleCi.codeSize = mSpirv.size() * sizeof( uint32 );
            moduleCi.pCode = &mSpirv[0];
            VkResult result = vkCreateShaderModule( mDevice->mDevice, &moduleCi, 0, &mShaderModule );
            checkVkResult( result, "vkCreateShaderModule" );

            setObjectName( mDevice->mDevice, (uint64_t)mShaderModule,
                           VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, mName.c_str() );
        }

        if( !mSpirv.empty() && mType == GPT_VERTEX_PROGRAM )
        {
            OgreProfileExhaustive( "VulkanProgram::compile::SpvReflectShaderModule" );
            SpvReflectShaderModule module;
            memset( &module, 0, sizeof( module ) );
            SpvReflectResult result =
                spvReflectCreateShaderModule( mSpirv.size() * sizeof( uint32 ), &mSpirv[0], &module );
            if( result != SPV_REFLECT_RESULT_SUCCESS )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "spvReflectCreateShaderModule failed on shader " + mName +
                                 " error code: " + getSpirvReflectError( result ),
                             "VulkanProgram::compile" );
            }

            FreeModuleOnDestructor modulePtr( &module );
            gatherVertexInputs( module );
        }

        return mCompiled;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::createLowLevelImpl( void )
    {
        mAssemblerProgram = GpuProgramPtr( this, SPFM_NONE );
        if( !mCompiled )
            compile( true );
    }
    //---------------------------------------------------------------------------
    void VulkanProgram::unloadImpl()
    {
        // We didn't create mAssemblerProgram through a manager, so override this
        // implementation so that we don't try to remove it from one. Since getCreator()
        // is used, it might target a different matching handle!
        mAssemblerProgram.setNull();

        unloadHighLevel();

        if( !mCustomRootLayout )
            mRootLayout = 0;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::unloadHighLevelImpl( void )
    {
        // Release everything
        mCompiled = false;

        mSpirv.clear();
        if( mShaderModule )
        {
            delayed_vkDestroyShaderModule( mDevice->mVaoManager, mDevice->mDevice, mShaderModule, 0 );
            mShaderModule = 0;
        }
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::populateParameterNames( GpuProgramParametersSharedPtr params )
    {
        getConstantDefinitions();
        params->_setNamedConstants( mConstantDefs );
    }
    //-----------------------------------------------------------------------
    const SpvReflectDescriptorBinding *VulkanProgram::findBinding(
        const FastArray<SpvReflectDescriptorSet *> &sets, size_t setIdx, size_t bindingIdx )
    {
        FastArray<SpvReflectDescriptorSet *>::const_iterator itor = sets.begin();
        FastArray<SpvReflectDescriptorSet *>::const_iterator endt = sets.end();

        while( itor != endt && ( *itor )->set != setIdx )
            ++itor;

        SpvReflectDescriptorBinding const *binding = 0;

        if( itor != sets.end() )
        {
            const SpvReflectDescriptorSet *descSet = *itor;
            const size_t numBindings = descSet->binding_count;
            for( size_t i = 0u; i < numBindings && !binding; ++i )
            {
                if( descSet->bindings[i]->binding == bindingIdx )
                    binding = descSet->bindings[i];
            }
        }

        return binding;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::buildConstantDefinitions( void ) const
    {
        OgreProfileExhaustive( "VulkanProgram::buildConstantDefinitions" );

        // if( !mBuildParametersFromReflection )
        //     return;
        if( mCompileError )
            return;

        if( mSpirv.empty() )
            return;

        size_t paramSetIdx = 0u;
        size_t paramBindingIdx = 0u;
        if( !mRootLayout->findParamsBuffer( mType, paramSetIdx, paramBindingIdx ) )
        {
            // Root layout does not specify a params buffer. Nothing to do here.
            return;
        }

        SpvReflectShaderModule module;
        memset( &module, 0, sizeof( module ) );
        SpvReflectResult result =
            spvReflectCreateShaderModule( mSpirv.size() * sizeof( uint32 ), &mSpirv[0], &module );
        if( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "spvReflectCreateShaderModule failed on shader " + mName +
                             " error code: " + getSpirvReflectError( result ),
                         "VulkanDescriptors::generateDescriptorSet" );
        }

        // Ensure module gets freed if we throw
        FreeModuleOnDestructor modulePtr( &module );

        uint32 numDescSets = 0;
        result = spvReflectEnumerateDescriptorSets( &module, &numDescSets, 0 );
        if( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "spvReflectEnumerateDescriptorSets failed on shader " + mName +
                             " error code: " + getSpirvReflectError( result ),
                         "VulkanDescriptors::generateDescriptorSet" );
        }

        FastArray<SpvReflectDescriptorSet *> sets;
        sets.resize( numDescSets );
        result = spvReflectEnumerateDescriptorSets( &module, &numDescSets, sets.begin() );
        if( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "spvReflectEnumerateDescriptorSets failed on shader " + mName +
                             " error code: " + getSpirvReflectError( result ),
                         "VulkanDescriptors::generateDescriptorSet" );
        }

        // const_cast to get around the fact that buildConstantDefinitions() is const.
        VulkanProgram *vp = const_cast<VulkanProgram *>( this );

        const SpvReflectDescriptorBinding *descBinding =
            findBinding( sets, paramSetIdx, paramBindingIdx );

        if( !descBinding )
        {
            // It's fine. The root layout declared a slot but the shader is not forced to use it
            return;
        }

        // VulkanConstantDefinitionBindingParam prevBindingParam;
        // prevBindingParam.offset = 0;
        // prevBindingParam.size = 0;

        const SpvReflectDescriptorBinding &reflBinding = *descBinding;

        const VkDescriptorType type = static_cast<VkDescriptorType>( reflBinding.descriptor_type );

        if( type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Error on shader " + mName + ": params buffer slot must be a uniform buffer",
                         "VulkanProgram::buildConstantDefinitions" );
        }

        const size_t numMembers = reflBinding.block.member_count;
        for( size_t memberPos = 0u; memberPos < numMembers; ++memberPos )
        {
            const SpvReflectBlockVariable &blockVariable = reflBinding.block.members[memberPos];
            GpuConstantType constantType = VulkanMappings::get( blockVariable.type_description->op );
            if( constantType == GCT_MATRIX_4X4 )
            {
                const uint32_t rowCount = blockVariable.numeric.matrix.row_count;
                const uint32_t columnCount = blockVariable.numeric.matrix.column_count;

                if( rowCount == 2 && columnCount == 2 )
                    constantType = GCT_MATRIX_2X2;
                else if( rowCount == 2 && columnCount == 3 )
                    constantType = GCT_MATRIX_2X3;
                else if( rowCount == 2 && columnCount == 4 )
                    constantType = GCT_MATRIX_2X4;
                else if( rowCount == 3 && columnCount == 2 )
                    constantType = GCT_MATRIX_3X2;
                else if( rowCount == 3 && columnCount == 3 )
                    constantType = GCT_MATRIX_3X3;
                else if( rowCount == 3 && columnCount == 4 )
                    constantType = GCT_MATRIX_3X4;
                else if( rowCount == 4 && columnCount == 2 )
                    constantType = GCT_MATRIX_4X2;
                else if( rowCount == 4 && columnCount == 3 )
                    constantType = GCT_MATRIX_4X3;
                else if( rowCount == 4 && columnCount == 4 )
                    constantType = GCT_MATRIX_4X4;
            }
            else if( ( blockVariable.type_description->type_flags &
                       ( SPV_REFLECT_TYPE_FLAG_VECTOR | SPV_REFLECT_TYPE_FLAG_ARRAY ) ) ||
                     blockVariable.type_description->op == SpvOpTypeInt ||
                     blockVariable.type_description->op == SpvOpTypeFloat )
            {
                const uint32 componentCount = blockVariable.numeric.vector.component_count;
                if( blockVariable.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT )
                {
                    switch( componentCount )
                    {
                    case 0:  // float myArray[5] returns componentCount = 0; vecN myArray[5] is fine
                    case 1:
                        constantType = GCT_FLOAT1;
                        break;
                    case 2:
                        constantType = GCT_FLOAT2;
                        break;
                    case 3:
                        constantType = GCT_FLOAT3;
                        break;
                    case 4:
                        constantType = GCT_FLOAT4;
                        break;
                    default:
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "invalid component count for float vector",
                                     "VulkanProgram::buildConstantDefinitions" );
                    }
                }
                else if( blockVariable.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_INT &&
                         blockVariable.numeric.scalar.signedness )
                {
                    switch( componentCount )
                    {
                    case 0:
                    case 1:
                        constantType = GCT_INT1;
                        break;
                    case 2:
                        constantType = GCT_INT2;
                        break;
                    case 3:
                        constantType = GCT_INT3;
                        break;
                    case 4:
                        constantType = GCT_INT4;
                        break;
                    default:
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "invalid component count for int vector",
                                     "VulkanProgram::buildConstantDefinitions" );
                    }
                }
                else if( blockVariable.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_INT &&
                         !blockVariable.numeric.scalar.signedness )
                {
                    switch( componentCount )
                    {
                    case 0:
                    case 1:
                        constantType = GCT_UINT1;
                        break;
                    case 2:
                        constantType = GCT_UINT2;
                        break;
                    case 3:
                        constantType = GCT_UINT3;
                        break;
                    case 4:
                        constantType = GCT_UINT4;
                        break;
                    default:
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "invalid component count for uint vector",
                                     "VulkanProgram::buildConstantDefinitions" );
                    }
                }
            }
            else if( blockVariable.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT )
            {
                continue;
            }

            GpuConstantDefinition def;
            def.constType = constantType;
            def.logicalIndex = blockVariable.offset;
            if( blockVariable.type_description->type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY )
            {
                def.elementSize = blockVariable.array.stride / sizeof( float );
                // I have no idea why blockVariable.array.dims_count != 1u
                OGRE_ASSERT_LOW( blockVariable.array.dims_count == 1u );
                def.arraySize = blockVariable.array.dims[0];
            }
            else
            {
                // def.elementSize = GpuConstantDefinition::getElementSize( def.constType, false );
                def.elementSize = blockVariable.padded_size / sizeof( float );
                def.arraySize = 1;
            }
            def.variability = GPV_GLOBAL;

            if( def.isFloat() )
            {
                def.physicalIndex = mFloatLogicalToPhysical->bufferSize;
                OGRE_LOCK_MUTEX( mFloatLogicalToPhysical->mutex );
                mFloatLogicalToPhysical->map.insert( GpuLogicalIndexUseMap::value_type(
                    def.logicalIndex,
                    GpuLogicalIndexUse( def.physicalIndex, def.arraySize * def.elementSize,
                                        GPV_GLOBAL ) ) );
                mFloatLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
                mConstantDefs->floatBufferSize = mFloatLogicalToPhysical->bufferSize;
            }
            else if( def.isUnsignedInt() )
            {
                def.physicalIndex = mUIntLogicalToPhysical->bufferSize;
                OGRE_LOCK_MUTEX( mUIntLogicalToPhysical->mutex );
                mUIntLogicalToPhysical->map.insert( GpuLogicalIndexUseMap::value_type(
                    def.logicalIndex,
                    GpuLogicalIndexUse( def.physicalIndex, def.arraySize * def.elementSize,
                                        GPV_GLOBAL ) ) );
                mUIntLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
                mConstantDefs->uintBufferSize = mUIntLogicalToPhysical->bufferSize;
            }
            else
            {
                def.physicalIndex = mIntLogicalToPhysical->bufferSize;
                OGRE_LOCK_MUTEX( mIntLogicalToPhysical->mutex );
                mIntLogicalToPhysical->map.insert( GpuLogicalIndexUseMap::value_type(
                    def.logicalIndex,
                    GpuLogicalIndexUse( def.physicalIndex, def.arraySize * def.elementSize,
                                        GPV_GLOBAL ) ) );
                mIntLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
                mConstantDefs->intBufferSize = mIntLogicalToPhysical->bufferSize;
            }

            String varName = blockVariable.name;
            if( blockVariable.array.dims_count )
                vp->mConstantDefs->generateConstantDefinitionArrayEntries( varName, def );

            mConstantDefs->map.insert( GpuConstantDefinitionMap::value_type( varName, def ) );
            vp->mConstantDefsSorted.push_back( def );

            vp->mConstantsBytesToWrite =
                std::max<uint32>( vp->mConstantsBytesToWrite,
                                  def.logicalIndex + def.arraySize * def.elementSize * sizeof( float ) );
        }

        VulkanConstantDefinitionBindingParam bindingParam;
        bindingParam.offset = reflBinding.block.offset;
        bindingParam.size = reflBinding.block.size;

        vp->mConstantDefsBindingParams.insert(
            map<uint32, VulkanConstantDefinitionBindingParam>::type::value_type( reflBinding.binding,
                                                                                 bindingParam ) );

        // prevBindingParam.offset = bindingParam.offset;
        // prevBindingParam.size = bindingParam.size;
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::gatherVertexInputs( SpvReflectShaderModule &module )
    {
        OgreProfileExhaustive( "VulkanProgram::gatherVertexInputs" );

        mNumSystemGenVertexInputs = 0u;
        mVertexInputs.clear();

        uint32_t count = 0u;

        SpvReflectResult result = spvReflectEnumerateInputVariables( &module, &count, NULL );
        if( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "spvReflectEnumerateInputVariables failed on shader " + mName +
                             " error code: " + getSpirvReflectError( result ),
                         "VulkanProgram::gatherVertexInputs" );
        }

        if( count == 0u )
            return;

        FastArray<SpvReflectInterfaceVariable *> inputVars;
        inputVars.resize( count );

        result = spvReflectEnumerateInputVariables( &module, &count, &inputVars[0] );
        if( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "spvReflectEnumerateInputVariables failed on shader " + mName +
                             " error code: " + getSpirvReflectError( result ),
                         "VulkanProgram::gatherVertexInputs" );
        }

        FastArray<SpvReflectInterfaceVariable *>::const_iterator itor = inputVars.begin();
        FastArray<SpvReflectInterfaceVariable *>::const_iterator endt = inputVars.end();

        while( itor != endt )
        {
            const SpvReflectInterfaceVariable *reflVar = *itor;
            VkVertexInputAttributeDescription attrDesc;

            attrDesc.location = reflVar->location;
            uint32 attrIdx;
            if( mShaderSyntax == HLSL )
            {
                if( strcmp( reflVar->name, "input.drawId" ) == 0 )
                {
                    mDrawIdLocation = attrDesc.location;
                    attrIdx = attrDesc.location;
                }
                else
                {
                    VertexElementSemantic sem = VulkanMappings::getHlslSemantic( reflVar->name );
                    attrIdx = VulkanVaoManager::getAttributeIndexFor( sem );
                }
            }
            else
                attrIdx = attrDesc.location;  // In GLSL the key and attrDesc.location are identical

            attrDesc.binding = 0u;
            attrDesc.format = static_cast<VkFormat>( reflVar->format );
            attrDesc.offset = 0u;

            if( attrDesc.location == std::numeric_limits<uint32_t>::max() )
                ++mNumSystemGenVertexInputs;
            else
                mVertexInputs.insert( VertexInputByLocationIdxMap::value_type( attrIdx, attrDesc ) );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------
    static VkShaderStageFlagBits get( GpuProgramType programType )
    {
        switch( programType )
        {
        // clang-format off
        case GPT_VERTEX_PROGRAM:    return VK_SHADER_STAGE_VERTEX_BIT;
        case GPT_FRAGMENT_PROGRAM:  return VK_SHADER_STAGE_FRAGMENT_BIT;
        case GPT_GEOMETRY_PROGRAM:  return VK_SHADER_STAGE_GEOMETRY_BIT;
        case GPT_HULL_PROGRAM:      return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case GPT_DOMAIN_PROGRAM:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case GPT_COMPUTE_PROGRAM:   return VK_SHADER_STAGE_COMPUTE_BIT;
            // clang-format on
        }
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    void VulkanProgram::fillPipelineShaderStageCi( VkPipelineShaderStageCreateInfo &pssCi )
    {
        makeVkStruct( pssCi, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO );
        pssCi.stage = get( mType );
        pssCi.module = mShaderModule;
        pssCi.pName = "main";
    }
    //-----------------------------------------------------------------------
    uint32 VulkanProgram::getBufferRequiredSize( void ) const { return mConstantsBytesToWrite; }
    //-----------------------------------------------------------------------
    void VulkanProgram::updateBuffers( const GpuProgramParametersSharedPtr &params,
                                       uint8 *RESTRICT_ALIAS dstData )
    {
        vector<GpuConstantDefinition>::type::const_iterator itor = mConstantDefsSorted.begin();
        vector<GpuConstantDefinition>::type::const_iterator endt = mConstantDefsSorted.end();

        while( itor != endt )
        {
            const GpuConstantDefinition &def = *itor;

            void *RESTRICT_ALIAS src;
            if( def.isFloat() )
                src = (void *)&( *( params->getFloatConstantList().begin() + def.physicalIndex ) );
            else if( def.isUnsignedInt() )
                src = (void *)&( *( params->getUnsignedIntConstantList().begin() + def.physicalIndex ) );
            else
                src = (void *)&( *( params->getIntConstantList().begin() + def.physicalIndex ) );

            memcpy( &dstData[def.logicalIndex], src, def.elementSize * def.arraySize * sizeof( float ) );

            ++itor;
        }
    }
    //---------------------------------------------------------------------
    void VulkanProgram::getLayoutForPso(
        const VertexElement2VecVec &vertexElements,
        FastArray<VkVertexInputBindingDescription> &outBufferBindingDescs,
        FastArray<VkVertexInputAttributeDescription> &outVertexInputs )
    {
        OgreProfileExhaustive( "VulkanProgram::getLayoutForPso" );

        outBufferBindingDescs.reserve( vertexElements.size() + 1u );  // +1 due to DRAWID
        outVertexInputs.reserve( mVertexInputs.size() );

        const size_t numShaderInputs = mVertexInputs.size();
        size_t numShaderInputsFound = mNumSystemGenVertexInputs;

        size_t uvCount = 0;

        // Iterate through the vertexElements and see what is actually used by the shader
        const size_t vertexElementsSize = vertexElements.size();
        for( size_t bufferIdx = 0; bufferIdx < vertexElementsSize; ++bufferIdx )
        {
            VertexElement2Vec::const_iterator it = vertexElements[bufferIdx].begin();
            VertexElement2Vec::const_iterator en = vertexElements[bufferIdx].end();

            VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_MAX_ENUM;

            uint32 bindAccumOffset = 0u;

            while( it != en )
            {
                size_t locationIdx = VulkanVaoManager::getAttributeIndexFor( it->mSemantic );

                if( it->mSemantic == VES_TEXTURE_COORDINATES )
                    locationIdx += uvCount++;

                // In GLSL locationIdx == itor->second.location as they're hardcoded
                // However in HLSL these values are generated by the shader and may not match
                VertexInputByLocationIdxMap::const_iterator itor =
                    mVertexInputs.find( static_cast<uint32>( locationIdx ) );

                if( itor != mVertexInputs.end() )
                {
                    if( it->mInstancingStepRate > 1u )
                    {
                        OGRE_EXCEPT(
                            Exception::ERR_RENDERINGAPI_ERROR,
                            "Shader: '" + mName + "' Vulkan only supports mInstancingStepRate = 0 or 1 ",
                            "VulkanProgram::getLayoutForPso" );
                    }
                    else if( inputRate == VK_VERTEX_INPUT_RATE_MAX_ENUM )
                    {
                        if( it->mInstancingStepRate == 0u )
                            inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                        else
                            inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                    }
                    else if( ( it->mInstancingStepRate == 0u &&
                               inputRate != VK_VERTEX_INPUT_RATE_VERTEX ) ||
                             ( it->mInstancingStepRate == 1u &&
                               inputRate != VK_VERTEX_INPUT_RATE_INSTANCE ) )
                    {
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "Shader: '" + mName +
                                         "' can only have all-instancing or all-vertex rate semantics "
                                         "for the same vertex buffer, but it is mixing vertex and "
                                         "instancing semantics for the same buffer idx",
                                     "VulkanProgram::getLayoutForPso" );
                    }

                    outVertexInputs.push_back( itor->second );
                    VkVertexInputAttributeDescription &inputDesc = outVertexInputs.back();
                    inputDesc.format = VulkanMappings::get( it->mType );
                    inputDesc.binding = static_cast<uint32_t>( bufferIdx );
                    inputDesc.offset = bindAccumOffset;

                    ++numShaderInputsFound;
                }

                bindAccumOffset += v1::VertexElement::getTypeSize( it->mType );
                ++it;
            }

            if( inputRate != VK_VERTEX_INPUT_RATE_MAX_ENUM )
            {
                // Only bind this buffer's entry if it's actually used by the shader
                VkVertexInputBindingDescription bindingDesc;
                bindingDesc.binding = static_cast<uint32_t>( bufferIdx );
                bindingDesc.stride = bindAccumOffset;
                bindingDesc.inputRate = inputRate;
                outBufferBindingDescs.push_back( bindingDesc );
            }
        }

        // Check if DRAWID is being used
        {
            const size_t locationIdx = 15u;
            VertexInputByLocationIdxMap::const_iterator itor = mVertexInputs.find( locationIdx );

            if( itor != mVertexInputs.end() )
            {
                outVertexInputs.push_back( itor->second );
                VkVertexInputAttributeDescription &inputDesc = outVertexInputs.back();
                inputDesc.format = VK_FORMAT_R32_UINT;
                inputDesc.binding = static_cast<uint32>( locationIdx );
                inputDesc.offset = 0u;

                ++numShaderInputsFound;

                VkVertexInputBindingDescription bindingDesc;
                bindingDesc.binding = static_cast<uint32>( locationIdx );
                bindingDesc.stride = 4u;
                bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                outBufferBindingDescs.push_back( bindingDesc );
            }
        }

        if( numShaderInputsFound < numShaderInputs )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "The shader requires more input attributes/semantics than what the "
                         "VertexArrayObject / v1::VertexDeclaration has to offer. You're "
                         "missing a component",
                         "VulkanProgram::getLayoutForPso" );
        }
    }
    //---------------------------------------------------------------------
    inline bool VulkanProgram::getPassSurfaceAndLightStates( void ) const
    {
        // Scenemanager should pass on light & material state to the rendersystem
        return true;
    }
    //---------------------------------------------------------------------
    inline bool VulkanProgram::getPassTransformStates( void ) const
    {
        // Scenemanager should pass on transform state to the rendersystem
        return true;
    }
    //---------------------------------------------------------------------
    inline bool VulkanProgram::getPassFogStates( void ) const
    {
        // Scenemanager should pass on fog state to the rendersystem
        return true;
    }
    //-----------------------------------------------------------------------
    String VulkanProgram::CmdPreprocessorDefines::doGet( const void *target ) const
    {
        return static_cast<const VulkanProgram *>( target )->getPreprocessorDefines();
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::CmdPreprocessorDefines::doSet( void *target, const String &val )
    {
        static_cast<VulkanProgram *>( target )->setPreprocessorDefines( val );
    }
    //-----------------------------------------------------------------------
    const String &VulkanProgram::getLanguage( void ) const
    {
        static const String language = "glsl";
        return language;
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr VulkanProgram::createParameters( void )
    {
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();
        params->setTransposeMatrices( true );
        return params;
    }
    //-----------------------------------------------------------------------
}  // namespace Ogre
