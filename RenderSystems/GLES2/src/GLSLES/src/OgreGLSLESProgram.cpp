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
#include "OgreGLES2Prerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"
#include "OgreGLUtil.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLNativeSupport.h"

#include "OgreGLSLESProgram.h"
#include "OgreGLSLESProgramManager.h"
#include "OgreGLSLPreprocessor.h"

namespace Ogre {
    //-----------------------------------------------------------------------
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
    GLSLESProgram::CmdOptimisation GLSLESProgram::msCmdOptimisation;
#endif
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    GLSLESProgram::GLSLESProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : GLSLShaderCommon(creator, name, handle, group, isManual, loader)
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        , mIsOptimised(false)
        , mOptimiserEnabled(false)
#endif
    {
        if (createParamDictionary("GLSLESProgram"))
        {
            setupBaseParamDictionary();
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(ParameterDef("use_optimiser", 
                                            "Should the GLSL optimiser be used. Default is false.",
                                            PT_BOOL),&msCmdOptimisation);
#endif
        }
        // Manually assign language now since we use it immediately
        mSyntaxCode = "glsles";

        // There is nothing to load
        mLoadFromFile = false;
    }
    //---------------------------------------------------------------------------
    GLSLESProgram::~GLSLESProgram()
    {
        // Have to call this here reather than in Resource destructor
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
    //---------------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLSLESProgram::notifyOnContextLost()
    {
        unloadHighLevelImpl();
    }

    void GLSLESProgram::notifyOnContextReset()
    {
        try {
            loadFromSource();
        }
        catch(Exception& e)
        {
            // we already compiled this once, this should not happen
            LogManager::getSingleton().stream(LML_WARNING) << e.what();
        }
    }
#endif
    bool GLSLESProgram::linkSeparable()
    {
        if(mLinked)
            return true;

        uint32 hash = _getHash();

        if (GLSLESProgramCommon::getMicrocodeFromCache(hash, mGLProgramHandle))
        {
            mLinked = true;
        }
        else
        {
            if( mType == GPT_VERTEX_PROGRAM )
                GLSLESProgramCommon::bindFixedAttributes( mGLProgramHandle );

            OGRE_CHECK_GL_ERROR(glProgramParameteriEXT(mGLProgramHandle, GL_PROGRAM_SEPARABLE_EXT, GL_TRUE));
            attachToProgramObject(mGLProgramHandle);
            OGRE_CHECK_GL_ERROR(glLinkProgram(mGLProgramHandle));
            OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramHandle, GL_LINK_STATUS, &mLinked));

            GLSLES::logObjectInfo( mName + String("GLSL vertex program result : "), mGLProgramHandle );

            GLSLESProgramCommon::_writeToCache(hash, mGLProgramHandle);
        }

        return mLinked;
    }

    void GLSLESProgram::loadFromSource()
    {
        const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();

        // Only create a shader object if glsl es is supported
        if (isSupported())
        {
            // Create shader object
            GLenum shaderType = 0x0000;
            if (mType == GPT_VERTEX_PROGRAM)
            {
                shaderType = GL_VERTEX_SHADER;
            }
            else if (mType == GPT_FRAGMENT_PROGRAM)
            {
                shaderType = GL_FRAGMENT_SHADER;
            }
            OGRE_CHECK_GL_ERROR(mGLShaderHandle = glCreateShader(shaderType));

            if(caps->hasCapability(RSC_DEBUG))
            {
                glLabelObjectEXT(GL_SHADER_OBJECT_EXT, mGLShaderHandle, 0, mName.c_str());
            }

            // also create program object
            if (caps->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
                if (caps->hasCapability(RSC_DEBUG))
                    OGRE_CHECK_GL_ERROR(
                        glLabelObjectEXT(GL_PROGRAM_OBJECT_EXT, mGLProgramHandle, 0, mName.c_str()));
            }
        }

        // Add preprocessor extras and main source
        if (!mSource.empty())
        {
            size_t versionPos = mSource.find("#version");
            int shaderVersion = 100;
            size_t belowVersionPos = 0;

            if(versionPos != String::npos)
            {
                shaderVersion = StringConverter::parseInt(mSource.substr(versionPos+9, 3));
                belowVersionPos = mSource.find('\n', versionPos) + 1;
            }

            // insert precision qualifier for improved compatibility
            if(mType == GPT_FRAGMENT_PROGRAM && mSource.find("precision ") == String::npos)
                mSource.insert(belowVersionPos, "precision mediump float;\n");

            // Fix up the source in case someone forgot to redeclare gl_Position
            if (caps->hasCapability(RSC_GLSL_SSO_REDECLARE) && mType == GPT_VERTEX_PROGRAM)
            {
                if(shaderVersion >= 300) {
                    // Check that it's missing and that this shader has a main function, ie. not a child shader.
                    if(mSource.find("out highp vec4 gl_Position") == String::npos)
                    {
                        mSource.insert(belowVersionPos, "out highp vec4 gl_Position;\nout highp float gl_PointSize;\n");
                    }
                    if(mSource.find("#extension GL_EXT_separate_shader_objects : require") == String::npos)
                    {
                        mSource.insert(belowVersionPos, "#extension GL_EXT_separate_shader_objects : require\n");
                    }
                }
            }
    
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
            const char *source = (getOptimiserEnabled() && getIsOptimised()) ? mOptimisedSource.c_str() : mSource.c_str();
#else
            const char *source = mSource.c_str();
#endif

            OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));
        }

        OGRE_CHECK_GL_ERROR(glCompileShader(mGLShaderHandle));

        // Check for compile errors
        int compiled;
        OGRE_CHECK_GL_ERROR(glGetShaderiv(mGLShaderHandle, GL_COMPILE_STATUS, &compiled));

        String compileInfo = GLSLES::getObjectInfo(mGLShaderHandle);

        if (!compiled)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, getResourceLogName() + " " + compileInfo, "compile");

        // probably we have warnings
        if (!compileInfo.empty())
            LogManager::getSingleton().stream(LML_WARNING) << getResourceLogName() << " " << compileInfo;
    }

#if !OGRE_NO_GLES2_GLSL_OPTIMISER   
    //-----------------------------------------------------------------------
    void GLSLESProgram::setOptimiserEnabled(bool enabled) 
    { 
        if(mOptimiserEnabled != enabled && mOptimiserEnabled && mCompiled == 1)
        {
            OGRE_CHECK_GL_ERROR(glDeleteShader(mGLShaderHandle));

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
            }
            
            mGLShaderHandle = 0;
            mGLProgramHandle = 0;
            mCompiled = 0;
        }
        mOptimiserEnabled = enabled; 
    }
#endif
    //-----------------------------------------------------------------------
    void GLSLESProgram::unloadHighLevelImpl(void)
    {
        if (isSupported())
        {
//            LogManager::getSingleton().logMessage("Deleting shader " + StringConverter::toString(mGLShaderHandle) +
//                                                  " and program " + StringConverter::toString(mGLProgramHandle));
            OGRE_CHECK_GL_ERROR(glDeleteShader(mGLShaderHandle));

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
            }
            // destroy all programs using this shader
            GLSLESProgramManager::getSingletonPtr()->destroyAllByShader(this);

            
            mGLShaderHandle = 0;
            mGLProgramHandle = 0;
            mLinked = 0;
        }
    }
    //-----------------------------------------------------------------------
    void GLSLESProgram::buildConstantDefinitions() const
    {
        // We need an accurate list of all the uniforms in the shader, but we
        // can't get at them until we link all the shaders into a program object.

        // Therefore instead, parse the source code manually and extract the uniforms
        createParameterMappingStructures(true);
        mFloatLogicalToPhysical.reset();
        mIntLogicalToPhysical.reset();
        GLSLESProgramManager::getSingleton().extractUniformsFromGLSL(mSource, *mConstantDefs, mName);
    }

    //-----------------------------------------------------------------------
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
    String GLSLESProgram::CmdOptimisation::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const GLSLESProgram*>(target)->getOptimiserEnabled());
    }
    void GLSLESProgram::CmdOptimisation::doSet(void *target, const String& val)
    {
        static_cast<GLSLESProgram*>(target)->setOptimiserEnabled(StringConverter::parseBool(val));
    }
#endif
    //-----------------------------------------------------------------------
    void GLSLESProgram::attachToProgramObject( const GLuint programObject )
    {
//        LogManager::getSingleton().logMessage("Attaching shader " + StringConverter::toString(mGLShaderHandle) +
//                                              " to program " + StringConverter::toString(programObject));
        OGRE_CHECK_GL_ERROR(glAttachShader(programObject, mGLShaderHandle));
    }
    //-----------------------------------------------------------------------
    void GLSLESProgram::detachFromProgramObject( const GLuint programObject )
    {
//        LogManager::getSingleton().logMessage("Detaching shader " + StringConverter::toString(mGLShaderHandle) +
//                                              " to program " + StringConverter::toString(programObject));
        OGRE_CHECK_GL_ERROR(glDetachShader(programObject, mGLShaderHandle));
    }

    //-----------------------------------------------------------------------
    const String& GLSLESProgram::getLanguage(void) const
    {
        static const String language = "glsles";

        return language;
    }
    //-----------------------------------------------------------------------
    Ogre::GpuProgramParametersSharedPtr GLSLESProgram::createParameters( void )
    {
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();
        params->setTransposeMatrices(true);
        return params;
    }
}
