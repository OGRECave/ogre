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
#include "OgreGLES2Util.h"
#include "OgreGLES2RenderSystem.h"

#include "OgreGLSLESProgram.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreGLSLESLinkProgramManager.h"
#include "OgreGLSLESProgramPipelineManager.h"
#include "OgreGLSLESPreprocessor.h"

namespace Ogre {

    String operationTypeToString(RenderOperation::OperationType val);
    RenderOperation::OperationType parseOperationType(const String& val);

    //-----------------------------------------------------------------------
	GLSLESProgram::CmdPreprocessorDefines GLSLESProgram::msCmdPreprocessorDefines;
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
    GLSLESProgram::CmdOptimisation GLSLESProgram::msCmdOptimisation;
#endif
    //-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
    GLSLESProgram::GLSLESProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader) 
		, mGLShaderHandle(0)
        , mGLProgramHandle(0)
        , mCompiled(0)
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        , mIsOptimised(false)
        , mOptimiserEnabled(false)
#endif
    {
        if (createParamDictionary("GLSLESProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("preprocessor_defines", 
                                            "Preprocessor defines use to compile the program.",
                                            PT_STRING),&msCmdPreprocessorDefines);
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
			dict->addParameter(ParameterDef("use_optimiser", 
                                            "Should the GLSL optimiser be used. Default is false.",
                                            PT_BOOL),&msCmdOptimisation);
#endif
        }
        // Manually assign language now since we use it immediately
        mSyntaxCode = "glsles";
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
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    void GLSLESProgram::notifyOnContextLost()
    {
        unloadHighLevelImpl();
    }
#endif
    //-----------------------------------------------------------------------
	void GLSLESProgram::loadFromSource(void)
	{
		// Preprocess the GLSL ES shader in order to get a clean source
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
					pos = endPos;
			}
		}

		size_t out_size = 0;
		const char *src = mSource.c_str ();
		size_t src_len = mSource.size ();
		char *out = cpp.Parse (src, src_len, out_size);
		if (!out || !out_size)
			// Failed to preprocess, break out
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "Failed to preprocess shader " + mName,
						 __FUNCTION__);

		mSource = String (out, out_size);
		if (out < src || out > src + src_len)
			free (out);
    }

	bool GLSLESProgram::compile(const bool checkErrors)
	{
		if (mCompiled == 1)
		{
			return true;
		}
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

#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
            if(getGLES2SupportRef()->checkExtension("GL_EXT_debug_label"))
            {
               OGRE_IF_IOS_VERSION_IS_GREATER_THAN(5.0)
                    glLabelObjectEXT(GL_SHADER_OBJECT_EXT, mGLShaderHandle, 0, mName.c_str());
            }
#endif

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
                if(getGLES2SupportRef()->checkExtension("GL_EXT_debug_label"))
                {
                    OGRE_IF_IOS_VERSION_IS_GREATER_THAN(5.0)
                        glLabelObjectEXT(GL_PROGRAM_OBJECT_EXT, mGLProgramHandle, 0, mName.c_str());
                }
#endif
            }
		}

		// Add preprocessor extras and main source
		if (!mSource.empty())
		{
            // Fix up the source in case someone forgot to redeclare gl_Position
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS) &&
                mType == GPT_VERTEX_PROGRAM)
            {
                size_t versionPos = mSource.find("#version");
                int shaderVersion = StringConverter::parseInt(mSource.substr(versionPos+9, 3));

                // Check that it's missing and that this shader has a main function, ie. not a child shader.
                if(mSource.find("out highp vec4 gl_Position") == String::npos)
                {
                    if(shaderVersion >= 300)
                        mSource.insert(versionPos+16, "out highp vec4 gl_Position;\nout highp float gl_PointSize;\n");
                }
                if(mSource.find("#extension GL_EXT_separate_shader_objects : require") == String::npos)
                {
                    if(shaderVersion >= 300)
                        mSource.insert(versionPos+16, "#extension GL_EXT_separate_shader_objects : require\n");
                }
            }
    
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
			const char *source = (getOptimiserEnabled() && getIsOptimised()) ? mOptimisedSource.c_str() : mSource.c_str();
#else
			const char *source = mSource.c_str();
#endif

			OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));
		}

        if (checkErrors)
            logObjectInfo("GLSL ES compiling: " + mName, mGLShaderHandle);

		OGRE_CHECK_GL_ERROR(glCompileShader(mGLShaderHandle));

		// Check for compile errors
		OGRE_CHECK_GL_ERROR(glGetShaderiv(mGLShaderHandle, GL_COMPILE_STATUS, &mCompiled));
        if(!mCompiled && checkErrors)
		{
            String message = logObjectInfo("GLSL ES compile log: " + mName, mGLShaderHandle);
			checkAndFixInvalidDefaultPrecisionError(message);
		}

		// Log a message that the shader compiled successfully.
        if (mCompiled && checkErrors)
            logObjectInfo("GLSL ES compiled: " + mName, mGLShaderHandle);

        if(!mCompiled)
		{
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        ((mType == GPT_VERTEX_PROGRAM) ? "Vertex Program " : "Fragment Program ") + mName +
                        " failed to compile. See compile log above for details.",
                        "GLSLESProgram::compile");
		}

		return (mCompiled == 1);
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
	void GLSLESProgram::createLowLevelImpl(void)
	{
		mAssemblerProgram = GpuProgramPtr(OGRE_NEW GLSLESGpuProgram( this ));
	}
	//---------------------------------------------------------------------------
	void GLSLESProgram::unloadImpl()
	{   
		// We didn't create mAssemblerProgram through a manager, so override this
		// implementation so that we don't try to remove it from one. Since getCreator()
		// is used, it might target a different matching handle!
		mAssemblerProgram.setNull();

		unloadHighLevel();
	}
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
            
            mGLShaderHandle = 0;
            mGLProgramHandle = 0;
            mCompiled = 0;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLESProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
	{
		getConstantDefinitions();
		params->_setNamedConstants(mConstantDefs);
		// Don't set logical / physical maps here, as we can't access parameters by logical index in GLHL.
	}
	//-----------------------------------------------------------------------
	void GLSLESProgram::buildConstantDefinitions() const
	{
		// We need an accurate list of all the uniforms in the shader, but we
		// can't get at them until we link all the shaders into a program object.

		// Therefore instead, parse the source code manually and extract the uniforms
		createParameterMappingStructures(true);
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLESProgramPipelineManager::getSingleton().extractConstantDefs(mSource, *mConstantDefs.get(), mName);
        }
        else
        {
            GLSLESLinkProgramManager::getSingleton().extractConstantDefs(mSource, *mConstantDefs.get(), mName);
        }
	}

	//---------------------------------------------------------------------
	inline bool GLSLESProgram::getPassSurfaceAndLightStates(void) const
	{
		// Scenemanager should pass on light & material state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	inline bool GLSLESProgram::getPassTransformStates(void) const
	{
		// Scenemanager should pass on transform state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	inline bool GLSLESProgram::getPassFogStates(void) const
	{
		// Scenemanager should pass on fog state to the rendersystem
		return true;
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
	String GLSLESProgram::CmdPreprocessorDefines::doGet(const void *target) const
	{
		return static_cast<const GLSLESProgram*>(target)->getPreprocessorDefines();
	}
	void GLSLESProgram::CmdPreprocessorDefines::doSet(void *target, const String& val)
	{
		static_cast<GLSLESProgram*>(target)->setPreprocessorDefines(val);
	}

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
	//-----------------------------------------------------------------------
	void GLSLESProgram::checkAndFixInvalidDefaultPrecisionError( String &message )
	{
		String precisionQualifierErrorString = ": 'Default Precision Qualifier' : invalid type Type for default precision qualifier can be only float or int";
		vector< String >::type linesOfSource = StringUtil::split(mSource, "\n");
		if( message.find(precisionQualifierErrorString) != String::npos )
		{
			LogManager::getSingleton().logMessage("Fixing invalid type Type for default precision qualifier by deleting bad lines the re-compiling");

			// remove relevant lines from source
			vector< String >::type errors = StringUtil::split(message, "\n");

			// going from the end so when we delete a line the numbers of the lines before will not change
			for(int i = static_cast<int>(errors.size()) - 1 ; i != -1 ; i--)
			{
				String & curError = errors[i];
				size_t foundPos = curError.find(precisionQualifierErrorString);
				if(foundPos != String::npos)
				{
					String lineNumber = curError.substr(0, foundPos);
					size_t posOfStartOfNumber = lineNumber.find_last_of(':');
					if (posOfStartOfNumber != String::npos)
					{
						lineNumber = lineNumber.substr(posOfStartOfNumber +	1, lineNumber.size() - (posOfStartOfNumber + 1));
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
			for(size_t i = 0; i < linesOfSource.size()  ; i++)
			{
				newSource << linesOfSource[i] << "\n";
			}
			mSource = newSource.str();

			const char *source = mSource.c_str();
			OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));

            if (compile())
            {
                LogManager::getSingleton().logMessage("The removing of the lines fixed the invalid type Type for default precision qualifier error.");
            }
            else
            {
                LogManager::getSingleton().logMessage("The removing of the lines didn't help.");
            }
		}
	}
	//-----------------------------------------------------------------------
	RenderOperation::OperationType parseOperationType(const String& val)
	{
		if (val == "point_list")
		{
			return RenderOperation::OT_POINT_LIST;
		}
		else if (val == "line_list")
		{
			return RenderOperation::OT_LINE_LIST;
		}
		else if (val == "line_strip")
		{
			return RenderOperation::OT_LINE_STRIP;
		}
		else if (val == "triangle_strip")
		{
			return RenderOperation::OT_TRIANGLE_STRIP;
		}
		else if (val == "triangle_fan")
		{
			return RenderOperation::OT_TRIANGLE_FAN;
		}
		else 
		{
			//Triangle list is the default fallback. Keep it this way?
			return RenderOperation::OT_TRIANGLE_LIST;
		}
	}
	//-----------------------------------------------------------------------
	String operationTypeToString(RenderOperation::OperationType val)
	{
		switch (val)
		{
		case RenderOperation::OT_POINT_LIST:
			return "point_list";
			break;
		case RenderOperation::OT_LINE_LIST:
			return "line_list";
			break;
		case RenderOperation::OT_LINE_STRIP:
			return "line_strip";
			break;
		case RenderOperation::OT_TRIANGLE_STRIP:
			return "triangle_strip";
			break;
		case RenderOperation::OT_TRIANGLE_FAN:
			return "triangle_fan";
			break;
		case RenderOperation::OT_TRIANGLE_LIST:
		default:
			return "triangle_list";
			break;
		}
	}
}
