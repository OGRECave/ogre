// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreGLSLangProgramManager.h"
#include "OgreRoot.h"

#include "OgreLogManager.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include "gl_types.h"

namespace
{
TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT =  1,*/
    /* .limits = memset below*/
    };

    /// punch through the private glslang API to get layoutLocation
    struct TQualifier
    {
        // DANGER!! this must be kept in sync with the layout of glslang::TQualifier
        const char*         semanticName;
        int  storage      : 6;
        int  builtIn      : 9;
        int  declaredBuiltIn : 9;
        int  precision    : 3;
        bool invariant    : 1;
        bool centroid     : 1;
        bool smooth       : 1;
        bool flat         : 1;
        bool specConstant : 1;
        bool nonUniform   : 1;
        bool explicitOffset   : 1;
        bool defaultBlock : 1;
        bool noContraction: 1;
        bool nopersp      : 1;
        bool explicitInterp : 1;
        bool pervertexNV  : 1;
        bool perPrimitiveNV : 1;
        bool perViewNV : 1;
        bool perTaskNV : 1;
        bool patch        : 1;
        bool sample       : 1;
        bool restrict     : 1;
        bool readonly     : 1;
        bool writeonly    : 1;
        bool coherent     : 1;
        bool volatil      : 1;
        bool devicecoherent : 1;
        bool queuefamilycoherent : 1;
        bool workgroupcoherent : 1;
        bool subgroupcoherent  : 1;
        bool shadercallcoherent : 1;
        bool nonprivate   : 1;
        bool nullInit : 1;
        bool spirvByReference : 1;
        bool spirvLiteral : 1;

        int layoutMatrix  : 3;
        int layoutPacking : 4;
        int layoutOffset;
        int layoutAlign;

        unsigned int layoutLocation             : 12;
    };
    struct TType
    {
        virtual ~TType() {}

        // DANGER!! this must be kept in sync with the layout of glslang::TType
        int basicType        : 8;
        int vectorSize       : 4;
        int matrixCols       : 4;
        int matrixRows       : 4;
        bool vector1         : 1;
        bool coopmat         : 1;
        TQualifier qualifier;

        const TQualifier& getQualifier() const { return qualifier; }
    };
}

namespace Ogre
{
/// Convert GL uniform size and type to OGRE constant types
static GpuConstantType mapToGCT(int gltype)
{
    switch (gltype)
    {
    case GL_FLOAT:
        return GCT_FLOAT1;
    case GL_FLOAT_VEC2:
        return GCT_FLOAT2;
    case GL_FLOAT_VEC3:
        return GCT_FLOAT3;
    case GL_FLOAT_VEC4:
        return GCT_FLOAT4;
    case GL_IMAGE_1D: //TODO should be its own type?
    case GL_SAMPLER_1D:
    case GL_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        // need to record samplers for GLSL
        return GCT_SAMPLER1D;
    case GL_IMAGE_2D: //TODO should be its own type?
    case GL_IMAGE_2D_RECT:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_2D_RECT:    // TODO: Move these to a new type??
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        return GCT_SAMPLER2D;
    case GL_IMAGE_3D: //TODO should be its own type?
    case GL_SAMPLER_3D:
    case GL_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
        return GCT_SAMPLER3D;
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
        return GCT_SAMPLERCUBE;
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
        return GCT_SAMPLER1DSHADOW;
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
        return GCT_SAMPLER2DSHADOW;
    case GL_INT:
        return GCT_INT1;
    case GL_INT_VEC2:
        return GCT_INT2;
    case GL_INT_VEC3:
        return GCT_INT3;
    case GL_INT_VEC4:
        return GCT_INT4;
    case GL_FLOAT_MAT2:
        return GCT_MATRIX_2X2;
    case GL_FLOAT_MAT3:
        return GCT_MATRIX_3X3;
    case GL_FLOAT_MAT4:
        return GCT_MATRIX_4X4;
    case GL_FLOAT_MAT2x3:
        return GCT_MATRIX_2X3;
    case GL_FLOAT_MAT3x2:
        return GCT_MATRIX_3X2;
    case GL_FLOAT_MAT2x4:
        return GCT_MATRIX_2X4;
    case GL_FLOAT_MAT4x2:
        return GCT_MATRIX_4X2;
    case GL_FLOAT_MAT3x4:
        return GCT_MATRIX_3X4;
    case GL_FLOAT_MAT4x3:
        return GCT_MATRIX_4X3;
    case GL_DOUBLE:
        return GCT_DOUBLE1;
    case GL_DOUBLE_VEC2:
        return GCT_DOUBLE2;
    case GL_DOUBLE_VEC3:
        return GCT_DOUBLE3;
    case GL_DOUBLE_VEC4:
        return GCT_DOUBLE4;
    case GL_DOUBLE_MAT2:
        return GCT_MATRIX_DOUBLE_2X2;
    case GL_DOUBLE_MAT3:
        return GCT_MATRIX_DOUBLE_3X3;
    case GL_DOUBLE_MAT4:
        return GCT_MATRIX_DOUBLE_4X4;
    case GL_DOUBLE_MAT2x3:
        return GCT_MATRIX_DOUBLE_2X3;
    case GL_DOUBLE_MAT3x2:
        return GCT_MATRIX_DOUBLE_3X2;
    case GL_DOUBLE_MAT2x4:
        return GCT_MATRIX_DOUBLE_2X4;
    case GL_DOUBLE_MAT4x2:
        return GCT_MATRIX_DOUBLE_4X2;
    case GL_DOUBLE_MAT3x4:
        return GCT_MATRIX_DOUBLE_3X4;
    case GL_DOUBLE_MAT4x3:
        return GCT_MATRIX_DOUBLE_4X3;
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_ATOMIC_COUNTER: //TODO should be its own type?
        return GCT_UINT1;
    case GL_UNSIGNED_INT_VEC2:
        return GCT_UINT2;
    case GL_UNSIGNED_INT_VEC3:
        return GCT_UINT3;
    case GL_UNSIGNED_INT_VEC4:
        return GCT_UINT4;
    case GL_BOOL:
        return GCT_BOOL1;
    case GL_BOOL_VEC2:
        return GCT_BOOL2;
    case GL_BOOL_VEC3:
        return GCT_BOOL3;
    case GL_BOOL_VEC4:
        return GCT_BOOL4;
    default:
        return GCT_UNKNOWN;
    }
}

//-----------------------------------------------------------------------
static String sLanguageName = "glslang";

static EShLanguage getShLanguage(GpuProgramType type)
{
    switch (type)
    {
    // clang-format off
    case GPT_VERTEX_PROGRAM:    return EShLangVertex;
    case GPT_FRAGMENT_PROGRAM:  return EShLangFragment;
    case GPT_GEOMETRY_PROGRAM:  return EShLangGeometry;
    case GPT_HULL_PROGRAM:      return EShLangTessControl;
    case GPT_DOMAIN_PROGRAM:    return EShLangTessEvaluation;
    case GPT_COMPUTE_PROGRAM:   return EShLangCompute;
    case GPT_MESH_PROGRAM:      return EShLangMeshNV;
    case GPT_TASK_PROGRAM:      return EShLangTaskNV;
        // clang-format on
    }

    return EShLangFragment;
}

GLSLangProgram::GLSLangProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
                               const String& group, bool isManual, ManualResourceLoader* loader)
    : HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
{
    if (createParamDictionary("glslangProgram"))
    {
        setupBaseParamDictionary();
        memset(&DefaultTBuiltInResource.limits, 1, sizeof(TLimits));

        if(sizeof(TBuiltInResource) == 420) // replace with build_info.h, when it is universally available
        {
            // copy VK_NV_mesh_shader limits to VK_EXT_mesh_shader
            memcpy(&DefaultTBuiltInResource.maxMeshViewCountNV + 1, &DefaultTBuiltInResource.maxMeshOutputVerticesNV,
                   9 * sizeof(int));
        }
    }
}

GLSLangProgram::~GLSLangProgram()
{
    unload();
}

const String& GLSLangProgram::getLanguage(void) const
{
    return sLanguageName;
}

bool GLSLangProgram::isSupported() const
{   bool ret = !mCompileError && isRequiredCapabilitiesSupported();
    if(mSyntaxCode != "glslang") // in case this is provided by user
        ret = ret && GpuProgramManager::isSyntaxSupported(mSyntaxCode);
    return ret;
}

void GLSLangProgram::createLowLevelImpl()
{
    if (mCompileError)
        return;

    mAssemblerProgram =
        GpuProgramManager::getSingleton().createProgram(mName + "/Delegate", mGroup, mSyntaxCode, mType);
    String assemblyStr((char*)mAssembly.data(), mAssembly.size() * sizeof(uint32));
    mAssemblerProgram->setSource(assemblyStr);
    mAssembly.clear(); // delegate stores the data now
}
void GLSLangProgram::unloadHighLevelImpl() {}

void GLSLangProgram::prepareImpl()
{
    HighLevelGpuProgram::prepareImpl();

    if (mSyntaxCode == "glslang")
    {
        // find actual syntax code
        for(auto lang : {"gl_spirv", "spirv"})
        {
            if(GpuProgramManager::isSyntaxSupported(lang))
            {
                mSyntaxCode = lang;
                break;
            }
        }
    }

    auto stage = getShLanguage(mType);
    glslang::TShader shader(stage);

    String defines = appendBuiltinDefines(mPreprocessorDefines);
    String preamble = "#extension GL_GOOGLE_cpp_style_line_directive: enable\n";

    if(mSyntaxCode == "spirv")
        preamble += "#define VULKAN 100\n";

    for (auto def : parseDefines(defines))
    {
        preamble += StringUtil::format("#define %s %s\n", def.first, def.second);
    }
    shader.setPreamble(preamble.c_str());

    size_t versionPos = mSource.find("OGRE_NATIVE_GLSL_VERSION_DIRECTIVE");
    if(versionPos != String::npos)
    {
        mSource.replace(versionPos, 34, "#version 460\n");
    }

    mSource = _resolveIncludes(mSource, this, mFilename, true);
    const char* source = mSource.c_str();
    const char* name = mFilename.empty() ? NULL : mFilename.c_str();

    shader.setStringsWithLengthsAndNames(&source, NULL, &name, 1);
    shader.setEntryPoint("main");
    shader.setSourceEntryPoint(mEntryPoint.c_str());
    if(mSyntaxCode == "gl_spirv")
        shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    else if(mSyntaxCode == "spirv")
    {
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);

        if(mType == GPT_MESH_PROGRAM || mType == GPT_TASK_PROGRAM)
        {
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
        }
    }

    // minimal version is 430 for explicit uniform location, but we use latest to get all features
    if (!shader.parse(&DefaultTBuiltInResource, 460, false, EShMsgSpvRules))
    {
        LogManager::getSingleton().logError("GLSLang compilation failed for " + mName + ":\n" +
                                            shader.getInfoLog());
        mCompileError = true;
        return;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgSpvRules))
    {
        LogManager::getSingleton().logError("GLSLang linking failed for " + mName + ":\n" +
                                            program.getInfoLog());
        mCompileError = true;
        return;
    }

    auto intermediate = program.getIntermediate(stage);
    glslang::SpvOptions opts;
    glslang::GlslangToSpv(*intermediate, mAssembly, &opts);

    getConstantDefinitions();
    program.buildReflection();

    int nuniforms = program.getNumLiveUniformVariables();
    int blockIdx = -1;
    for(int i = 0; i < nuniforms; i++)
    {
        auto constType = mapToGCT(program.getUniformType(i));
        if(GpuConstantDefinition::isSampler(constType) || constType == GCT_UNKNOWN)
            continue;

        blockIdx = program.getUniformBlockIndex(i);
        auto isUBO = blockIdx != -1;
        auto uoffset = program.getUniformBufferOffset(i);

        if(isUBO)
        {
            auto uboName = String(program.getUniformBlockName(blockIdx));
            if(uboName != "OgreUniforms")
            {
                if(mType == GPT_MESH_PROGRAM || mType == GPT_TASK_PROGRAM)
                    continue; // SSBOs in mesh shaders are used for vertex buffers

                GpuProgramManager::getSingleton().getSharedParameters(uboName);
                // TODO: there is no public API to set the binding point and create the correct buffer yet
            }
        }

        auto utype = (const TType*)program.getUniformTType(i);
        GpuConstantDefinition def;
        def.logicalIndex = isUBO ? uoffset : utype->getQualifier().layoutLocation;
        def.arraySize = program.getUniformArraySize(i);
        def.physicalIndex = isUBO ? uoffset : mConstantDefs->bufferSize * 4;
        def.constType = constType;
        bool doPadding = isUBO && GpuConstantDefinition::getElementSize(def.constType, false) > 1;
        def.elementSize = GpuConstantDefinition::getElementSize(def.constType, doPadding);

        mConstantDefs->bufferSize += def.arraySize * def.elementSize;
        mConstantDefs->map.emplace(program.getUniformName(i), def);

        if(!isUBO)
        {
            // also allow index based referencing
            GpuLogicalIndexUse use;
            use.physicalIndex = def.physicalIndex;
            use.currentSize = def.arraySize * def.elementSize;
            mLogicalToPhysical->map.emplace(def.logicalIndex, use);
        }
    }

    if (blockIdx != -1)
    {
        mConstantDefs->bufferSize = program.getUniformBlockSize(blockIdx) / 4;
        mLogicalToPhysical.reset();
    }
}

void GLSLangProgram::loadFromSource() {}
void GLSLangProgram::buildConstantDefinitions() {}

//-----------------------------------------------------------------------
GLSLangProgramFactory::GLSLangProgramFactory()
{
    glslang::InitializeProcess();
}
//-----------------------------------------------------------------------
GLSLangProgramFactory::~GLSLangProgramFactory()
{
    glslang::FinalizeProcess();
}
//-----------------------------------------------------------------------
const String& GLSLangProgramFactory::getLanguage(void) const
{
    return sLanguageName;
}
//-----------------------------------------------------------------------
GpuProgram* GLSLangProgramFactory::create(ResourceManager* creator, const String& name,
                                          ResourceHandle handle, const String& group, bool isManual,
                                          ManualResourceLoader* loader)
{
    return OGRE_NEW GLSLangProgram(creator, name, handle, group, isManual, loader);
}
const String& GLSLangPlugin::getName() const
{
    static String sPluginName = "GLSLang Program Manager";
    return sPluginName;
}
//---------------------------------------------------------------------
void GLSLangPlugin::install() {}
//---------------------------------------------------------------------
void GLSLangPlugin::initialise()
{
    if (!GpuProgramManager::isSyntaxSupported("gl_spirv") && !GpuProgramManager::isSyntaxSupported("spirv"))
        return;

    // Create new factory
    mProgramFactory.reset(new GLSLangProgramFactory());

    // Register
    GpuProgramManager::getSingleton().addFactory(mProgramFactory.get());
}
//---------------------------------------------------------------------
void GLSLangPlugin::shutdown()
{
    // nothing to do
}
//---------------------------------------------------------------------
void GLSLangPlugin::uninstall()
{
    if (mProgramFactory)
    {

        // Remove from manager safely
        if (GpuProgramManager::getSingletonPtr())
            GpuProgramManager::getSingleton().removeFactory(mProgramFactory.get());
        mProgramFactory.reset();
    }
}

#ifndef OGRE_STATIC_LIB
extern "C" void _OgreGLSLangProgramManagerExport dllStartPlugin(void);
extern "C" void _OgreGLSLangProgramManagerExport dllStopPlugin(void);

static Plugin* glslangPlugin;
//-----------------------------------------------------------------------
extern "C" void _OgreGLSLangProgramManagerExport dllStartPlugin(void)
{
    // Create new plugin
    glslangPlugin = OGRE_NEW GLSLangPlugin();

    // Register
    Root::getSingleton().installPlugin(glslangPlugin);
}
extern "C" void _OgreGLSLangProgramManagerExport dllStopPlugin(void)
{
    Root::getSingleton().uninstallPlugin(glslangPlugin);
    OGRE_DELETE glslangPlugin;
}
#endif

} // namespace Ogre
