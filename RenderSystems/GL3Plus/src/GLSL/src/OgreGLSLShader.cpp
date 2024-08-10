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
#include "OgreGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"
#include "OgreGpuProgramManager.h"

#include "OgreGLSLShader.h"

#include "OgreGLSLPreprocessor.h"
#include "OgreGLSLProgramManager.h"
#include "OgreGLUtil.h"
#include "OgreGLUniformCache.h"

#include "OgreGL3PlusHardwareBufferManager.h"

namespace Ogre {

    /// Command object for setting the maximum output vertices (geometry shader only)
    class CmdHasSamplersBinding : public ParamCommand
    {
    public:
        String doGet(const void* target) const override
        {
            return StringConverter::toString(static_cast<const GLSLShader*>(target)->getSamplerBinding());
        }
        void doSet(void* target, const String& val) override
        {
            static_cast<GLSLShader*>(target)->setSamplerBinding(StringConverter::parseBool(val));
        }
    };
    static CmdHasSamplersBinding msCmdHasSamplerBinding;

    /**  Convert GL uniform size and type to OGRE constant types
         and associate uniform definitions together. */
    static void convertGLUniformtoOgreType(GLenum gltype, GpuConstantDefinition& defToUpdate)
    {
        // Note GLSL never packs rows into float4's (from an API perspective anyway)
        // therefore all values are tight in the buffer.
        //TODO Should the rest of the above enum types be included here?
        switch (gltype)
        {
        case GL_FLOAT:
            defToUpdate.constType = GCT_FLOAT1;
            break;
        case GL_FLOAT_VEC2:
            defToUpdate.constType = GCT_FLOAT2;
            break;
        case GL_FLOAT_VEC3:
            defToUpdate.constType = GCT_FLOAT3;
            break;
        case GL_FLOAT_VEC4:
            defToUpdate.constType = GCT_FLOAT4;
            break;
        case GL_IMAGE_1D: //TODO should be its own type?
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
            // need to record samplers for GLSL
            defToUpdate.constType = GCT_SAMPLER1D;
            break;
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
            defToUpdate.constType = GCT_SAMPLER2D;
            break;
        case GL_IMAGE_3D: //TODO should be its own type?
        case GL_SAMPLER_3D:
        case GL_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
            defToUpdate.constType = GCT_SAMPLER3D;
            break;
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
            defToUpdate.constType = GCT_SAMPLERCUBE;
            break;
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
            defToUpdate.constType = GCT_SAMPLER1DSHADOW;
            break;
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
            defToUpdate.constType = GCT_SAMPLER2DSHADOW;
            break;
        case GL_INT:
            defToUpdate.constType = GCT_INT1;
            break;
        case GL_INT_VEC2:
            defToUpdate.constType = GCT_INT2;
            break;
        case GL_INT_VEC3:
            defToUpdate.constType = GCT_INT3;
            break;
        case GL_INT_VEC4:
            defToUpdate.constType = GCT_INT4;
            break;
        case GL_FLOAT_MAT2:
            defToUpdate.constType = GCT_MATRIX_2X2;
            break;
        case GL_FLOAT_MAT3:
            defToUpdate.constType = GCT_MATRIX_3X3;
            break;
        case GL_FLOAT_MAT4:
            defToUpdate.constType = GCT_MATRIX_4X4;
            break;
        case GL_FLOAT_MAT2x3:
            defToUpdate.constType = GCT_MATRIX_2X3;
            break;
        case GL_FLOAT_MAT3x2:
            defToUpdate.constType = GCT_MATRIX_3X2;
            break;
        case GL_FLOAT_MAT2x4:
            defToUpdate.constType = GCT_MATRIX_2X4;
            break;
        case GL_FLOAT_MAT4x2:
            defToUpdate.constType = GCT_MATRIX_4X2;
            break;
        case GL_FLOAT_MAT3x4:
            defToUpdate.constType = GCT_MATRIX_3X4;
            break;
        case GL_FLOAT_MAT4x3:
            defToUpdate.constType = GCT_MATRIX_4X3;
            break;
        case GL_DOUBLE:
            defToUpdate.constType = GCT_DOUBLE1;
            break;
        case GL_DOUBLE_VEC2:
            defToUpdate.constType = GCT_DOUBLE2;
            break;
        case GL_DOUBLE_VEC3:
            defToUpdate.constType = GCT_DOUBLE3;
            break;
        case GL_DOUBLE_VEC4:
            defToUpdate.constType = GCT_DOUBLE4;
            break;
        case GL_DOUBLE_MAT2:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_2X2;
            break;
        case GL_DOUBLE_MAT3:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_3X3;
            break;
        case GL_DOUBLE_MAT4:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_4X4;
            break;
        case GL_DOUBLE_MAT2x3:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_2X3;
            break;
        case GL_DOUBLE_MAT3x2:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_3X2;
            break;
        case GL_DOUBLE_MAT2x4:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_2X4;
            break;
        case GL_DOUBLE_MAT4x2:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_4X2;
            break;
        case GL_DOUBLE_MAT3x4:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_3X4;
            break;
        case GL_DOUBLE_MAT4x3:
            defToUpdate.constType = GCT_MATRIX_DOUBLE_4X3;
            break;
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_ATOMIC_COUNTER: //TODO should be its own type?
            defToUpdate.constType = GCT_UINT1;
            break;
        case GL_UNSIGNED_INT_VEC2:
            defToUpdate.constType = GCT_UINT2;
            break;
        case GL_UNSIGNED_INT_VEC3:
            defToUpdate.constType = GCT_UINT3;
            break;
        case GL_UNSIGNED_INT_VEC4:
            defToUpdate.constType = GCT_UINT4;
            break;
        case GL_BOOL:
            defToUpdate.constType = GCT_BOOL1;
            break;
        case GL_BOOL_VEC2:
            defToUpdate.constType = GCT_BOOL2;
            break;
        case GL_BOOL_VEC3:
            defToUpdate.constType = GCT_BOOL3;
            break;
        case GL_BOOL_VEC4:
            defToUpdate.constType = GCT_BOOL4;
            break;
        default:
            defToUpdate.constType = GCT_UNKNOWN;
            break;
        }
    }

    /// Get OpenGL GLSL shader type from OGRE GPU program type.
    static GLenum getGLShaderType(GpuProgramType programType)
    {
        switch (programType)
        {
        case GPT_VERTEX_PROGRAM:
            return GL_VERTEX_SHADER;
        case GPT_HULL_PROGRAM:
            return GL_TESS_CONTROL_SHADER;
        case GPT_DOMAIN_PROGRAM:
            return GL_TESS_EVALUATION_SHADER;
        case GPT_GEOMETRY_PROGRAM:
            return GL_GEOMETRY_SHADER;
        case GPT_FRAGMENT_PROGRAM:
            return GL_FRAGMENT_SHADER;
        case GPT_COMPUTE_PROGRAM:
            return GL_COMPUTE_SHADER;
        }

        return 0;
    }

    GLSLShader::GLSLShader(
        ResourceManager* creator,
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : GLSLShaderCommon(creator, name, handle, group, isManual, loader)
    {
        if (createParamDictionary("GLSLShader"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter("attach", &msCmdAttach);
            dict->addParameter("column_major_matrices", &msCmdColumnMajorMatrices);
            dict->addParameter("has_sampler_binding", &msCmdHasSamplerBinding);
        }

        mHasSamplerBinding = false;
        // There is nothing to load
        mLoadFromFile = false;
    }

    GLSLShader::~GLSLShader()
    {
        // Have to call this here rather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }

    void GLSLShader::compileSource()
    {
        if (mSource.empty())
            return;

        const RenderSystemCapabilities* rsc = Root::getSingleton().getRenderSystem()->getCapabilities();

        bool clipDistBug =
            ((OGRE_PLATFORM == OGRE_PLATFORM_WIN32) || (OGRE_PLATFORM == OGRE_PLATFORM_WINRT)) &&
            rsc->getVendor() == GPU_INTEL;

        size_t versionPos = mSource.find("#version");
        size_t belowVersionPos = 0;

        if(versionPos != String::npos)
        {
            belowVersionPos = mSource.find('\n', versionPos) + 1;
        }

        // OSX driver only supports glsl150+ in core profile
        bool shouldUpgradeToVersion150 = !rsc->isShaderProfileSupported("glsl130") && mShaderVersion < 150;

        // Add standard shader input and output blocks, if missing.
        // Assume blocks are missing if gl_Position is missing.
        if (rsc->hasCapability(RSC_GLSL_SSO_REDECLARE) && (mSource.find("vec4 gl_Position") == String::npos))
        {
            size_t insertPos = belowVersionPos;
            size_t extensionPos = mSource.rfind("#extension");
            if(extensionPos != String::npos)
            {
                insertPos = mSource.find('\n', extensionPos) + 1;
            }

            size_t mainPos = mSource.find("void main");
            // Only add blocks if shader is not a child
            // shader, i.e. has a main function.
            if (mainPos != String::npos)
            {
                String clipDistDecl = clipDistBug ? "float gl_ClipDistance[1];" : "float gl_ClipDistance[];";
                String inBlock = "in gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\n"+clipDistDecl+"\n} gl_in[];\n\n";
                String outBlock = "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\n"+clipDistDecl+"\n};\n\n";

                if (mShaderVersion >= 150 || shouldUpgradeToVersion150)
                {
                    switch (mType)
                    {
                    case GPT_VERTEX_PROGRAM:
                        mSource.insert(insertPos, outBlock);
                        break;
                    case GPT_GEOMETRY_PROGRAM:
                        mSource.insert(insertPos, outBlock);
                        mSource.insert(insertPos, inBlock);
                        break;
                    case GPT_DOMAIN_PROGRAM:
                        mSource.insert(insertPos, outBlock);
                        mSource.insert(insertPos, inBlock);
                        break;
                    case GPT_HULL_PROGRAM:
                        mSource.insert(insertPos, outBlock.substr(0, outBlock.size() - 3) + " gl_out[];\n\n");
                        mSource.insert(insertPos, inBlock);
                        break;
                    case GPT_FRAGMENT_PROGRAM:
                    case GPT_COMPUTE_PROGRAM:
                        // Fragment and compute shaders do
                        // not have standard blocks.
                        break;
                    }
                }
                else if(mType == GPT_VERTEX_PROGRAM && mShaderVersion >= 130) // shaderVersion < 150, means we only have vertex shaders
                {
                	// TODO: can we have SSO with GLSL < 130?
                    mSource.insert(insertPos, "out vec4 gl_Position;\nout float gl_PointSize;\nout "+clipDistDecl+"\n\n");
                }
            }
        }

        if(shouldUpgradeToVersion150)
        {
            if(belowVersionPos != 0)
                mSource = mSource.erase(0, belowVersionPos); // drop old definition

            // automatically upgrade to glsl150. thank you apple.
            const char* prefixFp =
                    "#version 150\n"
                    "#define varying in\n";
            const char* prefixVp =
                    "#version 150\n"
                    "#define attribute in\n"
                    "#define varying out\n";

            mSource.insert(0, mType == GPT_FRAGMENT_PROGRAM ? prefixFp : prefixVp);
        }

        // Submit shader source.
        const char *source = mSource.c_str();
        OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));
        OGRE_CHECK_GL_ERROR(glCompileShader(mGLShaderHandle));
    }

    bool GLSLShader::linkSeparable()
    {
        if(mCompileError)
            return false;

        if(mLinked)
            return true;

        OGRE_CHECK_GL_ERROR(glProgramParameteri(mGLProgramHandle, GL_PROGRAM_SEPARABLE, GL_TRUE));
        OGRE_CHECK_GL_ERROR(glProgramParameteri(mGLProgramHandle, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE));

        uint32 hash = _getHash();

        // Use precompiled program if possible.
        mLinked = GLSLProgram::getMicrocodeFromCache(hash, mGLProgramHandle);

        // Compilation needed if precompiled program is
        // unavailable or failed to link.
        if (!mLinked)
        {
            if( mType == GPT_VERTEX_PROGRAM )
                GLSLProgram::bindFixedAttributes( mGLProgramHandle );

            attachToProgramObject(mGLProgramHandle);
            OGRE_CHECK_GL_ERROR(glLinkProgram(mGLProgramHandle));
            OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramHandle, GL_LINK_STATUS, &mLinked));

            // Binary cache needs an update.
            GLSLProgram::writeMicrocodeToCache(hash, mGLProgramHandle);
        }

        if(!mLinked)
        {
            logObjectInfo( mName + String(" - GLSL program result : "), mGLProgramHandle );
            return false;
        }

        return true;
    }

    void GLSLShader::loadFromSource()
    {
        // Create shader object.
        GLenum GLShaderType = getGLShaderType(mType);
        OGRE_CHECK_GL_ERROR(mGLShaderHandle = glCreateShader(GLShaderType));

        auto caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        if (caps->hasCapability(RSC_DEBUG))
            OGRE_CHECK_GL_ERROR(glObjectLabel(GL_SHADER, mGLShaderHandle, -1, mName.c_str()));

        compileSource();

        // Check for compile errors
        int compiled = 0;
        OGRE_CHECK_GL_ERROR(glGetShaderiv(mGLShaderHandle, GL_COMPILE_STATUS, &compiled));

        String compileInfo = getObjectInfo(mGLShaderHandle);

        // also create program object
        if (compiled && caps->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
            // do not attempt to link attach only shaders
            if(mSyntaxCode == "gl_spirv" || (mSource.find("void main") != String::npos))
                compiled = linkSeparable();
        }

        if (!compiled)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, getResourceLogName() + " " + compileInfo, "compile");

        // probably we have warnings
        if (!compileInfo.empty())
            LogManager::getSingleton().stream(LML_WARNING) << getResourceLogName() << " " << compileInfo;
    }

    void GLSLShader::unloadHighLevelImpl(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteShader(mGLShaderHandle));

        if (mGLProgramHandle)
        {
            OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
        }

        // destroy all programs using this shader
        GLSLProgramManager::getSingletonPtr()->destroyAllByShader(this);

        mGLShaderHandle = 0;
        mGLProgramHandle = 0;
        mLinked = 0;
    }

    void GLSLShader::extractUniforms(int block) const
    {
        GLint numUniforms = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramInterfaceiv(mGLProgramHandle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms));

        const GLenum properties[6] = {GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION, GL_ARRAY_SIZE, GL_OFFSET};
        for (int unif = 0; unif < numUniforms; ++unif)
        {
            GLint values[6];
            OGRE_CHECK_GL_ERROR(
                glGetProgramResourceiv(mGLProgramHandle, GL_UNIFORM, unif, 6, properties, 6, NULL, values));

            // Skip any uniforms that are in a different block or atomic_uints
            if (values[0] != block || (block == -1 && values[3] == -1))
                continue;

            GpuConstantDefinition def;
            def.logicalIndex = values[3];
            def.arraySize = values[4];

            std::vector<char> nameData(values[2]);
            OGRE_CHECK_GL_ERROR(glGetProgramResourceName(mGLProgramHandle, GL_UNIFORM, unif, values[2],
                                                         NULL, &nameData[0]));
            String name(nameData.begin(), nameData.end() - 1);

            // If the uniform name ends with "]" then its an array element uniform
            if (name.back() == ']')
            {
                name.resize(name.size() - 3);
            }

            // Complete def and add
            // increment physical buffer location

            convertGLUniformtoOgreType(values[1], def);
            // GL doesn't pad
            def.elementSize = GpuConstantDefinition::getElementSize(def.constType, false);

            // also allow index based referencing
            GpuLogicalIndexUse use;

            if (def.isFloat() || def.isDouble() || def.isInt() || def.isUnsignedInt() || def.isBool())
            {
                def.physicalIndex = block > -1 ? values[5] : mConstantDefs->bufferSize * 4;
                mConstantDefs->bufferSize += def.arraySize * def.elementSize;

                if (values[3] != -1)
                {
                    use.physicalIndex = def.physicalIndex;
                    use.currentSize = def.arraySize * def.elementSize;
                    mLogicalToPhysical->map.emplace(def.logicalIndex, use);

                    // warn if there is a default value, that we would overwrite
                    std::vector<int> val(use.currentSize);
                    OGRE_CHECK_GL_ERROR(glGetUniformiv(mGLProgramHandle, def.logicalIndex, val.data()));
                    if (val != std::vector<int>(use.currentSize))
                        LogManager::getSingleton().logWarning("Default value of uniform '" + name +
                                                              "' is ignored in " + getResourceLogName());
                }
            }
            else if(def.isSampler())
            {
                if(mHasSamplerBinding)
                    continue;
                def.physicalIndex = mConstantDefs->registerCount;
                mConstantDefs->registerCount += def.arraySize * def.elementSize;
                // no index based referencing
            }
            else
            {
                LogManager::getSingleton().logError("Could not parse type of GLSL Uniform: '" + name +
                                                    "' in file " + getResourceLogName());
            }
            mConstantDefs->map.emplace(name, def);
        }
    }

    void GLSLShader::extractBufferBlocks(GLenum type) const
    {
        GLint numBlocks = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramInterfaceiv(mGLProgramHandle, type, GL_ACTIVE_RESOURCES, &numBlocks));

        auto& hbm = static_cast<GL3PlusHardwareBufferManager&>(HardwareBufferManager::getSingleton());

        const GLenum blockProperties[3] = {GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH, GL_BUFFER_DATA_SIZE};
        for(int blockIdx = 0; blockIdx < numBlocks; ++blockIdx)
        {
            GLint values[3];
            OGRE_CHECK_GL_ERROR(glGetProgramResourceiv(mGLProgramHandle, type, blockIdx, 3, blockProperties,
                                                       3, NULL, values));
            if(values[0] == 0) continue;

            std::vector<char> nameData(values[1]);
            OGRE_CHECK_GL_ERROR(glGetProgramResourceName(mGLProgramHandle, type, blockIdx,
                                                         values[1], NULL, &nameData[0]));
            String name(nameData.begin(), nameData.end() - 1);

            if (name == "OgreUniforms") // default buffer
            {
                extractUniforms(blockIdx);
                int binding = mType == GPT_COMPUTE_PROGRAM ? 0 : int(mType);
                if (binding > 1)
                    LogManager::getSingleton().logWarning(
                        getResourceLogName() +
                        " - using 'OgreUniforms' in this shader type does alias with shared_params");

                mDefaultBuffer = hbm.createUniformBuffer(values[2]);
                static_cast<GL3PlusHardwareBuffer*>(mDefaultBuffer.get())->setGLBufferBinding(binding);
                OGRE_CHECK_GL_ERROR(glUniformBlockBinding(mGLProgramHandle, blockIdx, binding));
                continue;
            }

            auto blockSharedParams = GpuProgramManager::getSingleton().getSharedParameters(name);

            HardwareBufferPtr hwGlBuffer = blockSharedParams->_getHardwareBuffer();
            if(!hwGlBuffer)
            {
                size_t binding = 0;
                if(type == GL_UNIFORM_BLOCK)
                {
                    binding = hbm.getUniformBufferCount() + 2; // slots 0 & 1 are reserved for defaultbuffer
                    hwGlBuffer = hbm.createUniformBuffer(values[2]);
                }
                else
                {
                    binding = hbm.getShaderStorageBufferCount();
                    hwGlBuffer = hbm.createShaderStorageBuffer(values[2]);
                }

                static_cast<GL3PlusHardwareBuffer*>(hwGlBuffer.get())->setGLBufferBinding(int(binding));
                blockSharedParams->_setHardwareBuffer(hwGlBuffer);
            }

            int binding = static_cast<GL3PlusHardwareBuffer*>(hwGlBuffer.get())->getGLBufferBinding();

            if(type == GL_UNIFORM_BLOCK)
            {
                OGRE_CHECK_GL_ERROR(glUniformBlockBinding(mGLProgramHandle, blockIdx, binding));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glShaderStorageBlockBinding(mGLProgramHandle, blockIdx, binding));
            }
        }
    }

    void GLSLShader::buildConstantDefinitions()
    {
        createParameterMappingStructures(true);
        auto caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        if(caps->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            extractUniforms();
            try
            {
                extractBufferBlocks(GL_UNIFORM_BLOCK);
                extractBufferBlocks(GL_SHADER_STORAGE_BLOCK);
            }
            catch (const InvalidParametersException& e)
            {
                LogManager::getSingleton().stream(LML_CRITICAL)
                    << "Program '" << mName << "' is not supported: " << e.getDescription();
                mCompileError = true;
            }
            return;
        }

        mLogicalToPhysical.reset();

        // We need an accurate list of all the uniforms in the shader, but we
        // can't get at them until we link all the shaders into a program object.
        // Therefore instead parse the source code manually and extract the uniforms.
        GLSLProgramManager::getSingleton().extractUniformsFromGLSL(mSource, *mConstantDefs, getResourceLogName());


        // Also parse any attached sources.
        for (auto childShader : mAttachedGLSLPrograms)
        {
            GLSLProgramManager::getSingleton().extractUniformsFromGLSL(
                childShader->getSource(), *mConstantDefs, childShader->getName());
        }

        if(!mHasSamplerBinding)
            return;

        // drop samplers from constant definitions
        for(auto it = mConstantDefs->map.begin(); it != mConstantDefs->map.end();)
        {
            if(it->second.isSampler())
                it = mConstantDefs->map.erase(it);
            else
                ++it;
        }
    }

    void GLSLShader::attachToProgramObject(const GLuint programObject)
    {
        // attach child objects
        for (auto childShader : mAttachedGLSLPrograms)
        {
            childShader->attachToProgramObject(programObject);
        }
        OGRE_CHECK_GL_ERROR(glAttachShader(programObject, mGLShaderHandle));
    }


    void GLSLShader::detachFromProgramObject(const GLuint programObject)
    {
        OGRE_CHECK_GL_ERROR(glDetachShader(programObject, mGLShaderHandle));
        logObjectInfo( "Error detaching " + mName + " shader object from GLSL Program Object", programObject);
        // attach child objects
        GLSLProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
        GLSLProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

        while (childprogramcurrent != childprogramend)
        {
            GLSLShaderCommon* childShader = *childprogramcurrent;
            childShader->detachFromProgramObject(programObject);
            ++childprogramcurrent;
        }
    }

    static const String language = "glsl";

    const String& GLSLShader::getLanguage(void) const
    {
        return language;
    }

    const String& GLSLShaderFactory::getLanguage(void) const
    {
        return language;
    }

    GpuProgram* GLSLShaderFactory::create(
        ResourceManager* creator,
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
    {
        return OGRE_NEW GLSLShader(creator, name, handle, group, isManual, loader);
    }
}
