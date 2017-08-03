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

#include "OgreGLSLShader.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLMonolithicProgramManager.h"
#include "OgreGLSLSeparableProgramManager.h"
#include "OgreGLSLPreprocessor.h"
#include "OgreGL3PlusUtil.h"

namespace Ogre {

    String operationTypeToString(OperationType val);
    OperationType parseOperationType(const String& val);

    GLSLShader::CmdPreprocessorDefines GLSLShader::msCmdPreprocessorDefines;
    GLSLShader::CmdAttach GLSLShader::msCmdAttach;
    GLSLShader::CmdColumnMajorMatrices GLSLShader::msCmdColumnMajorMatrices;
    GLSLShader::CmdInputOperationType GLSLShader::msInputOperationTypeCmd;
    GLSLShader::CmdOutputOperationType GLSLShader::msOutputOperationTypeCmd;
    GLSLShader::CmdMaxOutputVertices GLSLShader::msMaxOutputVerticesCmd;
    
    GLuint GLSLShader::mShaderCount = 0;

    GLSLShader::GLSLShader(
        ResourceManager* creator,
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
        , mGLShaderHandle(0)
        , mGLProgramHandle(0)
        , mCompiled(0)
        , mColumnMajorMatrices(true)
    {
        if (createParamDictionary("GLSLShader"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef(
                "preprocessor_defines",
                "Preprocessor defines use to compile the program.",
                PT_STRING), &msCmdPreprocessorDefines);
            dict->addParameter(ParameterDef(
                "attach",
                "name of another GLSL program needed by this program",
                PT_STRING), &msCmdAttach);
            dict->addParameter(ParameterDef(
                "column_major_matrices",
                "Whether matrix packing in column-major order.",
                PT_BOOL), &msCmdColumnMajorMatrices);
            dict->addParameter(
                ParameterDef(
                    "input_operation_type",
                    "The input operation type for this geometry program. "
                    "Can be 'point_list', 'line_list', 'line_strip', 'triangle_list', "
                    "'triangle_strip' or 'triangle_fan'",
                    PT_STRING), &msInputOperationTypeCmd);
            dict->addParameter(
                ParameterDef("output_operation_type",
                             "The input operation type for this geometry program. "
                             "Can be 'point_list', 'line_strip' or 'triangle_strip'",
                             PT_STRING), &msOutputOperationTypeCmd);
            dict->addParameter(
                ParameterDef("max_output_vertices",
                             "The maximum number of vertices a single run "
                             "of this geometry program can output",
                             PT_INT), &msMaxOutputVerticesCmd);
        }

        mType = GPT_VERTEX_PROGRAM; // default value, to be corrected after the constructor with GpuProgram::setType()
        mSyntaxCode = "glsl" + StringConverter::toString(Root::getSingleton().getRenderSystem()->getNativeShadingLanguageVersion());

        mLinked = 0;
        // Increase shader counter and use as ID
        mShaderID = ++mShaderCount;        
        
        // Transfer skeletal animation status from parent
        mSkeletalAnimation = isSkeletalAnimationIncluded();
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


    void GLSLShader::loadFromSource(void)
    {
        // Preprocess the GLSL shader in order to get a clean source
        CPreprocessor cpp;

        // Pass all user-defined macros to preprocessor
        if (!mPreprocessorDefines.empty ())
        {
            String::size_type pos = 0;
            while (pos != String::npos)
            {
                // Find delims
                String::size_type endPos = mPreprocessorDefines.find_first_of(";,=", pos);
                if (endPos != String::npos)
                {
                    String::size_type macro_name_start = pos;
                    size_t macro_name_len = endPos - pos;
                    pos = endPos;

                    // Check definition part
                    if (mPreprocessorDefines[pos] == '=')
                    {
                        // Set up a definition, skip delim
                        ++pos;
                        String::size_type macro_val_start = pos;
                        size_t macro_val_len;

                        endPos = mPreprocessorDefines.find_first_of(";,", pos);
                        if (endPos == String::npos)
                        {
                            macro_val_len = mPreprocessorDefines.size () - pos;
                            pos = endPos;
                        }
                        else
                        {
                            macro_val_len = endPos - pos;
                            pos = endPos+1;
                        }
                        cpp.Define (
                            mPreprocessorDefines.c_str () + macro_name_start, macro_name_len,
                            mPreprocessorDefines.c_str () + macro_val_start, macro_val_len);
                    }
                    else
                    {
                        // No definition part, define as "1"
                        ++pos;
                        cpp.Define (
                            mPreprocessorDefines.c_str () + macro_name_start, macro_name_len, 1);
                    }
                }
                else
                {
                    if(pos < mPreprocessorDefines.size())
                         cpp.Define (mPreprocessorDefines.c_str () + pos, mPreprocessorDefines.size() - pos, 1);
 
                    pos = endPos;
                }
            }
        }

        size_t out_size = 0;
        const char *src = mSource.c_str ();
        size_t src_len = mSource.size ();
        char *out = cpp.Parse (src, src_len, out_size);
        if (!out || !out_size)
        {
            mCompileError = true;
            // Failed to preprocess, break out
            OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to preprocess shader " + mName,
                         __FUNCTION__);
        }

        mSource = String (out, out_size);
        if (out < src || out > src + src_len)
            free (out);
    }


    bool GLSLShader::compile(const bool checkErrors)
    {
        if (mCompiled == 1)
        {
            return true;
        }

        // Create shader object.
        GLenum GLShaderType = getGLShaderType(mType);
        OGRE_CHECK_GL_ERROR(mGLShaderHandle = glCreateShader(GLShaderType));

        //TODO GL 4.3 KHR_debug

        // if (getGLSupport()->checkExtension("GL_KHR_debug") || mHasGL43)
        //     glObjectLabel(GL_SHADER, mGLShaderHandle, 0, mName.c_str());

        // if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        // {
        //     OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
        //     if(getGLSupport()->checkExtension("GL_KHR_debug") || mHasGL43)
        //         glObjectLabel(GL_PROGRAM, mGLProgramHandle, 0, mName.c_str());
        // }

        // Add boiler plate code and preprocessor extras, then
        // submit shader source to OpenGL.
        if (!mSource.empty())
        {
            // Add standard shader input and output blocks, if missing.
            if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Assume blocks are missing if gl_Position is missing.
                if (mSource.find("vec4 gl_Position") == String::npos)
                {
                    size_t mainPos = mSource.find("void main");
                    // Only add blocks if shader is not a child
                    // shader, i.e. has a main function.
                    if (mainPos != String::npos)
                    {
                        size_t versionPos = mSource.find("#version");
                        int shaderVersion = StringConverter::parseInt(mSource.substr(versionPos+9, 3));
                        if (shaderVersion >= 150)
                        {
                            size_t belowVersionPos = mSource.find("\n", versionPos) + 1;
                            switch (mType)
                            {
                            case GPT_VERTEX_PROGRAM:
                                mSource.insert(belowVersionPos, "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n};\n\n");
                                break;
                            case GPT_GEOMETRY_PROGRAM:
                                mSource.insert(belowVersionPos, "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n};\n\n");
                                mSource.insert(belowVersionPos, "in gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n} gl_in[];\n\n");
                                break;
                            case GPT_DOMAIN_PROGRAM:
                                mSource.insert(belowVersionPos, "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n};\n\n");
                                mSource.insert(belowVersionPos, "in gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n} gl_in[];\n\n");
                                break;
                            case GPT_HULL_PROGRAM:
                                mSource.insert(belowVersionPos, "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n} gl_out[];\n\n");
                                mSource.insert(belowVersionPos, "in gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n} gl_in[];\n\n");
                                break;
                            case GPT_FRAGMENT_PROGRAM:
                            case GPT_COMPUTE_PROGRAM:
                                // Fragment and compute shaders do
                                // not have standard blocks.
                                break;
                            }
                        }
                    }
                }
            }
            // Submit shader source.
            const char *source = mSource.c_str();
            OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));
        }

        OGRE_CHECK_GL_ERROR(glCompileShader(mGLShaderHandle));

        // Check for compile errors
        OGRE_CHECK_GL_ERROR(glGetShaderiv(mGLShaderHandle, GL_COMPILE_STATUS, &mCompiled));
        if (!mCompiled && checkErrors)
        {
            String message = logObjectInfo("GLSL compile log: " + mName, mGLShaderHandle);
            checkAndFixInvalidDefaultPrecisionError(message);
        }

        // Log a message that the shader compiled successfully.
        if (mCompiled && checkErrors)
            logObjectInfo("GLSL compiled: " + mName, mGLShaderHandle);

        if (!mCompiled)
        {
            mCompileError = true;
            dumpSourceIfHasIncludeEnabled();

            String shaderType = getShaderTypeLabel(mType);
            StringUtil::toTitleCase(shaderType);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        shaderType + " Program " + mName +
                        " failed to compile. See compile log above for details.",
                        "GLSLShader::compile");
        }

        return (mCompiled == 1);
    }


    void GLSLShader::createLowLevelImpl(void)
    {
        // mAssemblerProgram = GpuProgramPtr(OGRE_NEW GLSLShader(this));
        // // Shader params need to be forwarded to low level implementation
        // mAssemblerProgram->setAdjacencyInfoRequired(isAdjacencyInfoRequired());
        // mAssemblerProgram->setComputeGroupDimensions(getComputeGroupDimensions());
    }


    void GLSLShader::unloadImpl()
    {
        // We didn't create mAssemblerProgram through a manager, so override this
        // implementation so that we don't try to remove it from one. Since getCreator()
        // is used, it might target a different matching handle!
        // mAssemblerProgram.setNull();

        unloadHighLevel();
    }


    void GLSLShader::unloadHighLevelImpl(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteShader(mGLShaderHandle));

        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS) && mGLProgramHandle)
        {
            OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
        }

        mGLShaderHandle = 0;
        mGLProgramHandle = 0;
        mCompiled = 0;
    }


    void GLSLShader::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        getConstantDefinitions();
        params->_setNamedConstants(mConstantDefs);
        // Don't set logical / physical maps here, as we can't access parameters by logical index in GLSL.
    }


    void GLSLShader::buildConstantDefinitions() const
    {
        // We need an accurate list of all the uniforms in the shader, but we
        // can't get at them until we link all the shaders into a program object.

        // Therefore instead parse the source code manually and extract the uniforms.
        createParameterMappingStructures(true);
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLSeparableProgramManager::getSingleton().extractUniformsFromGLSL(mSource, *mConstantDefs.get(), mName);
        }
        else
        {
            GLSLMonolithicProgramManager::getSingleton().extractUniformsFromGLSL(mSource, *mConstantDefs.get(), mName);
        }

        // Also parse any attached sources.
        for (GLSLShaderContainer::const_iterator i = mAttachedGLSLShaders.begin();
             i != mAttachedGLSLShaders.end(); ++i)
        {
            GLSLShader* childShader = *i;

            if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                GLSLSeparableProgramManager::getSingleton().extractUniformsFromGLSL(childShader->getSource(),
                                                                                    *mConstantDefs.get(), childShader->getName());
            }
            else
            {
                GLSLMonolithicProgramManager::getSingleton().extractUniformsFromGLSL(childShader->getSource(),
                                                                                     *mConstantDefs.get(), childShader->getName());
            }
        }
    }


    inline bool GLSLShader::getPassSurfaceAndLightStates(void) const
    {
        // Scenemanager should pass on light & material state to the rendersystem.
        return true;
    }

    inline bool GLSLShader::getPassTransformStates(void) const
    {
        // Scenemanager should pass on transform state to the rendersystem.
        return true;
    }

    inline bool GLSLShader::getPassFogStates(void) const
    {
        // Scenemanager should pass on fog state to the rendersystem.
        return true;
    }


    String GLSLShader::CmdAttach::doGet(const void *target) const
    {
        return (static_cast<const GLSLShader*>(target))->getAttachedShaderNames();
    }
    void GLSLShader::CmdAttach::doSet(void *target, const String& shaderNames)
    {
        // Get all the shader program names: there could be more than one.
        StringVector vecShaderNames = StringUtil::split(shaderNames, " \t", 0);

        size_t programNameCount = vecShaderNames.size();
        for ( size_t i = 0; i < programNameCount; ++i)
        {
            static_cast<GLSLShader*>(target)->attachChildShader(vecShaderNames[i]);
        }
    }


    String GLSLShader::CmdColumnMajorMatrices::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const GLSLShader*>(target)->getColumnMajorMatrices());
    }
    void GLSLShader::CmdColumnMajorMatrices::doSet(void *target, const String& val)
    {
        static_cast<GLSLShader*>(target)->setColumnMajorMatrices(StringConverter::parseBool(val));
    }


    String GLSLShader::CmdPreprocessorDefines::doGet(const void *target) const
    {
        return static_cast<const GLSLShader*>(target)->getPreprocessorDefines();
    }
    void GLSLShader::CmdPreprocessorDefines::doSet(void *target, const String& val)
    {
        static_cast<GLSLShader*>(target)->setPreprocessorDefines(val);
    }


    String GLSLShader::CmdInputOperationType::doGet(const void* target) const
    {
        const GLSLShader* t = static_cast<const GLSLShader*>(target);
        return operationTypeToString(t->getInputOperationType());
    }
    void GLSLShader::CmdInputOperationType::doSet(void* target, const String& val)
    {
        GLSLShader* t = static_cast<GLSLShader*>(target);
        t->setInputOperationType(parseOperationType(val));
    }


    String GLSLShader::CmdOutputOperationType::doGet(const void* target) const
    {
        const GLSLShader* t = static_cast<const GLSLShader*>(target);
        return operationTypeToString(t->getOutputOperationType());
    }
    void GLSLShader::CmdOutputOperationType::doSet(void* target, const String& val)
    {
        GLSLShader* t = static_cast<GLSLShader*>(target);
        t->setOutputOperationType(parseOperationType(val));
    }


    String GLSLShader::CmdMaxOutputVertices::doGet(const void* target) const
    {
        const GLSLShader* t = static_cast<const GLSLShader*>(target);
        return StringConverter::toString(t->getMaxOutputVertices());
    }
    void GLSLShader::CmdMaxOutputVertices::doSet(void* target, const String& val)
    {
        GLSLShader* t = static_cast<GLSLShader*>(target);
        t->setMaxOutputVertices(StringConverter::parseInt(val));
    }


    void GLSLShader::attachChildShader(const String& name)
    {
        // Is the name valid and already loaded?
        // Check with the high level program manager to see if it was loaded.
        HighLevelGpuProgramPtr hlProgram = HighLevelGpuProgramManager::getSingleton().getByName(name);
        if (!hlProgram.isNull())
        {
            if (hlProgram->getSyntaxCode() == "glsl")
            {
                // Make sure attached program source gets loaded and compiled
                // don't need a low level implementation for attached shader objects
                // loadHighLevelImpl will only load the source and compile once
                // so don't worry about calling it several times.
                GLSLShader* childShader = static_cast<GLSLShader*>(hlProgram.getPointer());
                // Load the source and attach the child shader.
                childShader->loadHighLevelImpl();
                // Add to the container.
                mAttachedGLSLShaders.push_back(childShader);
                mAttachedShaderNames += name + " ";
            }
        }
    }


    void GLSLShader::attachToProgramObject(const GLuint programObject)
    {
        // attach child objects
        GLSLShaderContainerIterator childProgramCurrent = mAttachedGLSLShaders.begin();
        GLSLShaderContainerIterator childProgramEnd = mAttachedGLSLShaders.end();

        for (; childProgramCurrent != childProgramEnd; ++childProgramCurrent)
        {
            GLSLShader* childShader = *childProgramCurrent;
            childShader->compile(true);
            childShader->attachToProgramObject(programObject);
        }
        OGRE_CHECK_GL_ERROR(glAttachShader(programObject, mGLShaderHandle));
    }


    void GLSLShader::detachFromProgramObject(const GLuint programObject)
    {
        OGRE_CHECK_GL_ERROR(glDetachShader(programObject, mGLShaderHandle));
        logObjectInfo( "Error detaching " + mName + " shader object from GLSL Program Object", programObject);
        // attach child objects
        GLSLShaderContainerIterator childprogramcurrent = mAttachedGLSLShaders.begin();
        GLSLShaderContainerIterator childprogramend = mAttachedGLSLShaders.end();

        while (childprogramcurrent != childprogramend)
        {
            GLSLShader* childShader = *childprogramcurrent;
            childShader->detachFromProgramObject(programObject);
            ++childprogramcurrent;
        }
    }


    const String& GLSLShader::getLanguage(void) const
    {
        static const String language = "glsl";

        return language;
    }


    Ogre::GpuProgramParametersSharedPtr GLSLShader::createParameters(void)
    {
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();
        return params;
    }


    void GLSLShader::checkAndFixInvalidDefaultPrecisionError(String &message)
    {
        String precisionQualifierErrorString = ": 'Default Precision Qualifier' :  invalid type Type for default precision qualifier can be only float or int";
        vector< String >::type linesOfSource = StringUtil::split(mSource, "\n");
        if (message.find(precisionQualifierErrorString) != String::npos)
        {
            LogManager::getSingleton().logMessage("Fixing invalid type Type for default precision qualifier by deleting bad lines the re-compiling", LML_CRITICAL);

            // remove relevant lines from source
            vector< String >::type errors = StringUtil::split(message, "\n");

            // going from the end so when we delete a line the numbers of the lines before will not change
            for (int i = (int)errors.size() - 1 ; i != -1 ; i--)
            {
                String & curError = errors[i];
                size_t foundPos = curError.find(precisionQualifierErrorString);
                if (foundPos != String::npos)
                {
                    String lineNumber = curError.substr(0, foundPos);
                    size_t posOfStartOfNumber = lineNumber.find_last_of(':');
                    if (posOfStartOfNumber != String::npos)
                    {
                        lineNumber = lineNumber.substr(posOfStartOfNumber +     1, lineNumber.size() - (posOfStartOfNumber + 1));
                        if (StringConverter::isNumber(lineNumber))
                        {
                            int iLineNumber = StringConverter::parseInt(lineNumber);
                            linesOfSource.erase(linesOfSource.begin() + iLineNumber - 1);
                        }
                    }
                }
            }
            // rebuild source
            StringStream newSource;
            for (size_t i = 0; i < linesOfSource.size()  ; i++)
            {
                newSource << linesOfSource[i] << "\n";
            }
            mSource = newSource.str();

            const char *source = mSource.c_str();
            OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));
            // Check for load errors
            if (compile(true))
            {
                LogManager::getSingleton().logMessage("The removing of the lines fixed the invalid type Type for default precision qualifier error.", LML_CRITICAL);
            }
            else
            {
                LogManager::getSingleton().logMessage("The removing of the lines didn't help.", LML_CRITICAL);
            }
        }
    }

    void GLSLShader::setUniformBlockBinding( const char *blockName, uint32 bindingSlot )
    {
        GLuint programHandle = 0;

        const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if( caps->hasCapability( RSC_SEPARATE_SHADER_OBJECTS ) )
        {
            GLSLSeparableProgram *activeLinkProgram =
                    GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
            programHandle = activeLinkProgram->getGLProgramHandle();
        }
        else
        {
            GLSLMonolithicProgram *activeLinkProgram =
                    GLSLMonolithicProgramManager::getSingleton().getActiveMonolithicProgram();
            programHandle = activeLinkProgram->getGLProgramHandle();
        }

        GLuint blockIdx = glGetUniformBlockIndex( programHandle, blockName );
        if( blockIdx != GL_INVALID_INDEX )
        {
            OCGE( glUniformBlockBinding( programHandle, blockIdx, bindingSlot ) );
        }
    }


    OperationType parseOperationType(const String& val)
    {
        if (val == "point_list")
        {
            return OT_POINT_LIST;
        }
        else if (val == "line_list")
        {
            return OT_LINE_LIST;
        }
        else if (val == "line_strip")
        {
            return OT_LINE_STRIP;
        }
        else if (val == "triangle_strip")
        {
            return OT_TRIANGLE_STRIP;
        }
        else if (val == "triangle_fan")
        {
            return OT_TRIANGLE_FAN;
        }
        else
        {
            // Triangle list is the default fallback. Keep it this way?
            return OT_TRIANGLE_LIST;
        }
    }


    String operationTypeToString(OperationType val)
    {
        switch (val)
        {
        case OT_POINT_LIST:
            return "point_list";
            break;
        case OT_LINE_LIST:
            return "line_list";
            break;
        case OT_LINE_STRIP:
            return "line_strip";
            break;
        case OT_TRIANGLE_STRIP:
            return "triangle_strip";
            break;
        case OT_TRIANGLE_FAN:
            return "triangle_fan";
            break;
        case OT_TRIANGLE_LIST:
        default:
            return "triangle_list";
            break;
        }
    }


    GLenum GLSLShader::getGLShaderType(GpuProgramType programType)
    {
        //TODO Convert to map, or is speed different negligible?
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

        //TODO add warning or error
        return 0;
    }

    String GLSLShader::getShaderTypeLabel(GpuProgramType programType)
    {
        switch (programType)
        {
        case GPT_VERTEX_PROGRAM:
            return "vertex";
            break;
        case GPT_DOMAIN_PROGRAM:
            return "tessellation evaluation";
            break;
        case GPT_HULL_PROGRAM:
            return "tessellation control";
            break;
        case GPT_GEOMETRY_PROGRAM:
            return "geometry";
            break;
        case GPT_FRAGMENT_PROGRAM:
            return "fragment";
            break;
        case GPT_COMPUTE_PROGRAM:
            return "compute";
            break;
        }

        //TODO add warning or error
        return 0;
    }


    GLuint GLSLShader::getGLProgramHandle() {
        //TODO This should be removed and the compile() function
        // should use glCreateShaderProgramv
        // for separable programs which includes creating a program.
        if (mGLProgramHandle == 0)
        {
            OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
            if (mGLProgramHandle == 0)
            {
                //TODO error handling
            }
        }
        return mGLProgramHandle;
    }


    void GLSLShader::bind(void)
    {
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Tell the Program Pipeline Manager what pipeline is to become active.
            switch (mType)
            {
            case GPT_VERTEX_PROGRAM:
                GLSLSeparableProgramManager::getSingleton().setActiveVertexShader(this);
                break;
            case GPT_FRAGMENT_PROGRAM:
                GLSLSeparableProgramManager::getSingleton().setActiveFragmentShader(this);
                break;
            case GPT_GEOMETRY_PROGRAM:
                GLSLSeparableProgramManager::getSingleton().setActiveGeometryShader(this);
                break;
            case GPT_HULL_PROGRAM:
                GLSLSeparableProgramManager::getSingleton().setActiveTessHullShader(this);
                break;
            case GPT_DOMAIN_PROGRAM:
                GLSLSeparableProgramManager::getSingleton().setActiveTessDomainShader(this);
                break;
            case GPT_COMPUTE_PROGRAM:
                GLSLSeparableProgramManager::getSingleton().setActiveComputeShader(this);
            default:
                break;
            }
        }
        else
        {
            // Tell the Link Program Manager what shader is to become active.
            switch (mType)
            {
            case GPT_VERTEX_PROGRAM:
                GLSLMonolithicProgramManager::getSingleton().setActiveVertexShader(this);
                break;
            case GPT_FRAGMENT_PROGRAM:
                GLSLMonolithicProgramManager::getSingleton().setActiveFragmentShader(this);
                break;
            case GPT_GEOMETRY_PROGRAM:
                GLSLMonolithicProgramManager::getSingleton().setActiveGeometryShader(this);
                break;
            case GPT_HULL_PROGRAM:
                GLSLMonolithicProgramManager::getSingleton().setActiveHullShader(this);
                break;
            case GPT_DOMAIN_PROGRAM:
                GLSLMonolithicProgramManager::getSingleton().setActiveDomainShader(this);
                break;
            case GPT_COMPUTE_PROGRAM:
                GLSLMonolithicProgramManager::getSingleton().setActiveComputeShader(this);
            default:
                break;
            }
        }
    }

    void GLSLShader::unbind(void)
    {
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Tell the Program Pipeline Manager what pipeline is to become inactive.
            if (mType == GPT_VERTEX_PROGRAM)
            {
                GLSLSeparableProgramManager::getSingleton().setActiveVertexShader(NULL);
            }
            else if (mType == GPT_GEOMETRY_PROGRAM)
            {
                GLSLSeparableProgramManager::getSingleton().setActiveGeometryShader(NULL);
            }
            else if (mType == GPT_HULL_PROGRAM)
            {
                GLSLSeparableProgramManager::getSingleton().setActiveTessHullShader(NULL);
            }
            else if (mType == GPT_DOMAIN_PROGRAM)
            {
                GLSLSeparableProgramManager::getSingleton().setActiveTessDomainShader(NULL);
            }
            else if (mType == GPT_COMPUTE_PROGRAM)
            {
                GLSLSeparableProgramManager::getSingleton().setActiveComputeShader(NULL);
            }
            else // It's a fragment shader
            {
                GLSLSeparableProgramManager::getSingleton().setActiveFragmentShader(NULL);
            }
        }
        else
        {
            // Tell the Link Program Manager what shader is to become inactive.
            if (mType == GPT_VERTEX_PROGRAM)
            {
                GLSLMonolithicProgramManager::getSingleton().setActiveVertexShader(NULL);
            }
            else if (mType == GPT_GEOMETRY_PROGRAM)
            {
                GLSLMonolithicProgramManager::getSingleton().setActiveGeometryShader(NULL);
            }
            else if (mType == GPT_HULL_PROGRAM)
            {
                GLSLMonolithicProgramManager::getSingleton().setActiveHullShader(NULL);
            }
            else if (mType == GPT_DOMAIN_PROGRAM)
            {
                GLSLMonolithicProgramManager::getSingleton().setActiveDomainShader(NULL);
            }
            else if (mType == GPT_COMPUTE_PROGRAM)
            {
                GLSLMonolithicProgramManager::getSingleton().setActiveComputeShader(NULL);
            }
            else // It's a fragment shader
            {
                GLSLMonolithicProgramManager::getSingleton().setActiveFragmentShader(NULL);
            }
        }
    }

    void GLSLShader::unbindAll(void)
    {
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLSeparableProgramManager &glslManager = GLSLSeparableProgramManager::getSingleton();
            glslManager.setActiveVertexShader(NULL);
            glslManager.setActiveGeometryShader(NULL);
            glslManager.setActiveTessHullShader(NULL);
            glslManager.setActiveTessDomainShader(NULL);
            glslManager.setActiveComputeShader(NULL);
            glslManager.setActiveFragmentShader(NULL);
        }
        else
        {
            GLSLMonolithicProgramManager &glslManager = GLSLMonolithicProgramManager::getSingleton();
            glslManager.setActiveVertexShader(NULL);
            glslManager.setActiveGeometryShader(NULL);
            glslManager.setActiveHullShader(NULL);
            glslManager.setActiveDomainShader(NULL);
            glslManager.setActiveComputeShader(NULL);
            glslManager.setActiveFragmentShader(NULL);
        }
    }


    void GLSLShader::bindParameters(GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // Link can throw exceptions, ignore them at this point.
        try
        {
            if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Activate the program pipeline object.
                GLSLSeparableProgram* separableProgram = GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
                // Pass on parameters from params to program object uniforms.
                separableProgram->updateUniforms(params, mask, mType);
                separableProgram->updateAtomicCounters(params, mask, mType);
            }
            else
            {
                // Activate the link program object.
                GLSLMonolithicProgram* monolithicProgram = GLSLMonolithicProgramManager::getSingleton().getActiveMonolithicProgram();
                // Pass on parameters from params to program object uniforms.
                monolithicProgram->updateUniforms(params, mask, mType);
                //TODO add atomic counter support
                //monolithicProgram->updateAtomicCounters(params, mask, mType);
            }
        }
        catch (Exception&) {}
    }


    void GLSLShader::bindPassIterationParameters(GpuProgramParametersSharedPtr params)
    {
        if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            // Activate the program pipeline object.
            GLSLSeparableProgram* separableProgram = GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
            // Pass on parameters from params to program object uniforms.
            separableProgram->updatePassIterationUniforms(params);
        }
        else
        {
            // Activate the link program object.
            GLSLMonolithicProgram* monolithicProgram = GLSLMonolithicProgramManager::getSingleton().getActiveMonolithicProgram();
            // Pass on parameters from params to program object uniforms.
            monolithicProgram->updatePassIterationUniforms(params);
        }
    }


    void GLSLShader::bindSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // Link can throw exceptions, ignore them at this point.
        try
        {
            if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                // Activate the program pipeline object.
                GLSLSeparableProgram* separableProgram = GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
                // Pass on parameters from params to program object uniforms.
                separableProgram->updateUniformBlocks(params, mask, mType);
                // separableProgram->updateShaderStorageBlock(params, mask, mType);
            }
            else
            {
                // Activate the link program object.
                GLSLMonolithicProgram* monolithicProgram = GLSLMonolithicProgramManager::getSingleton().getActiveMonolithicProgram();
                // Pass on parameters from params to program object uniforms.
                monolithicProgram->updateUniformBlocks(params, mask, mType);
            }
        }
        catch (Exception&) {}
    }


    size_t GLSLShader::calculateSize(void) const
    {
        size_t memSize = 0;

        // Delegate names.
        memSize += sizeof(GLuint);
        memSize += sizeof(GLenum);
        memSize += GpuProgram::calculateSize();

        return memSize;
    }

}
