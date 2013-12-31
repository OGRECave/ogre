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

#include "OgreGLSLProgram.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLProgramPipelineManager.h"
#include "OgreGLSLPreprocessor.h"
#include "OgreGL3PlusUtil.h"

namespace Ogre {

    String operationTypeToString(RenderOperation::OperationType val);
    RenderOperation::OperationType parseOperationType(const String& val);

    //-----------------------------------------------------------------------
	GLSLProgram::CmdPreprocessorDefines GLSLProgram::msCmdPreprocessorDefines;
    GLSLProgram::CmdAttach GLSLProgram::msCmdAttach;
    GLSLProgram::CmdColumnMajorMatrices GLSLProgram::msCmdColumnMajorMatrices;
	GLSLProgram::CmdInputOperationType GLSLProgram::msInputOperationTypeCmd;
	GLSLProgram::CmdOutputOperationType GLSLProgram::msOutputOperationTypeCmd;
	GLSLProgram::CmdMaxOutputVertices GLSLProgram::msMaxOutputVerticesCmd;

    //-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
    GLSLProgram::GLSLProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader) 
		, mGLShaderHandle(0)
        , mGLProgramHandle(0)
        , mCompiled(0)
        , mColumnMajorMatrices(true)
    {
        if (createParamDictionary("GLSLProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("preprocessor_defines", 
                                            "Preprocessor defines use to compile the program.",
                                            PT_STRING),&msCmdPreprocessorDefines);
            dict->addParameter(ParameterDef("attach", 
                "name of another GLSL program needed by this program",
                PT_STRING),&msCmdAttach);
            dict->addParameter(ParameterDef("column_major_matrices",
                                            "Whether matrix packing in column-major order.",
                                            PT_BOOL),&msCmdColumnMajorMatrices);
			dict->addParameter(
				ParameterDef("input_operation_type",
				"The input operation type for this geometry program. \
				Can be 'point_list', 'line_list', 'line_strip', 'triangle_list', \
				'triangle_strip' or 'triangle_fan'", PT_STRING),
				&msInputOperationTypeCmd);
			dict->addParameter(
				ParameterDef("output_operation_type",
				"The input operation type for this geometry program. \
				Can be 'point_list', 'line_strip' or 'triangle_strip'",
				 PT_STRING),
				 &msOutputOperationTypeCmd);
			dict->addParameter(
				ParameterDef("max_output_vertices", 
				"The maximum number of vertices a single run of this geometry program can output",
				PT_INT),&msMaxOutputVerticesCmd);
        }
        // Manually assign language now since we use it immediately
        mSyntaxCode = "glsl";
    }
    //---------------------------------------------------------------------------
    GLSLProgram::~GLSLProgram()
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
    //-----------------------------------------------------------------------
	void GLSLProgram::loadFromSource(void)
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
    
    //---------------------------------------------------------------------------
	bool GLSLProgram::compile(const bool checkErrors)
	{
		if (mCompiled == 1)
		{
			return true;
		}

		// only create a shader object if glsl is supported
		if (isSupported())
		{
			// create shader object

			GLenum shaderType = 0x0000;
			switch (mType)
			{
			case GPT_VERTEX_PROGRAM:
				shaderType = GL_VERTEX_SHADER;
				break;
			case GPT_FRAGMENT_PROGRAM:
				shaderType = GL_FRAGMENT_SHADER;
				break;
			case GPT_GEOMETRY_PROGRAM:
				shaderType = GL_GEOMETRY_SHADER;
				break;
            case GPT_DOMAIN_PROGRAM:
                shaderType = GL_TESS_EVALUATION_SHADER;
                break;
            case GPT_HULL_PROGRAM:
                shaderType = GL_TESS_CONTROL_SHADER;
                break;
            case GPT_COMPUTE_PROGRAM:
                shaderType = GL_COMPUTE_SHADER;
                break;
			}
			OGRE_CHECK_GL_ERROR(mGLShaderHandle = glCreateShader(shaderType));

//            if(getGLSupport()->checkExtension("GL_KHR_debug") || gl3wIsSupported(4, 3))
//                glObjectLabel(GL_SHADER, mGLShaderHandle, 0, mName.c_str());

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                OGRE_CHECK_GL_ERROR(mGLProgramHandle = glCreateProgram());
//                if(getGLSupport()->checkExtension("GL_KHR_debug") || gl3wIsSupported(4, 3))
//                    glObjectLabel(GL_PROGRAM, mGLProgramHandle, 0, mName.c_str());
            }
		}

		// Add preprocessor extras and main source
		if (!mSource.empty())
		{
            // Fix up the source in case someone forgot to redeclare gl_Position
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS) &&
               mType == GPT_VERTEX_PROGRAM)
            {
                // Check that it's missing and that this shader has a main function, ie. not a child shader.
                if(mSource.find("vec4 gl_Position") == String::npos)
                {
                    size_t mainPos = mSource.find("void main");
                    if(mainPos != String::npos)
                    {
                        size_t versionPos = mSource.find("#version");
                        int shaderVersion = StringConverter::parseInt(mSource.substr(versionPos+9, 3));
                        if(shaderVersion >= 150)
                            mSource.insert(mainPos, "out gl_PerVertex\n{\nvec4 gl_Position;\nfloat gl_PointSize;\nfloat gl_ClipDistance[];\n};\n");
                    }
                }
            }

			const char *source = mSource.c_str();
			OGRE_CHECK_GL_ERROR(glShaderSource(mGLShaderHandle, 1, &source, NULL));
		}

		OGRE_CHECK_GL_ERROR(glCompileShader(mGLShaderHandle));

		// Check for compile errors
		OGRE_CHECK_GL_ERROR(glGetShaderiv(mGLShaderHandle, GL_COMPILE_STATUS, &mCompiled));
        if(!mCompiled && checkErrors)
		{
            String message = logObjectInfo("GLSL compile log: " + mName, mGLShaderHandle);
			checkAndFixInvalidDefaultPrecisionError(message);
		}

		// Log a message that the shader compiled successfully.
        if (mCompiled && checkErrors)
            logObjectInfo("GLSL compiled: " + mName, mGLShaderHandle);

        if(!mCompiled)
        {
			String progType = "Fragment";
			if (mType == GPT_VERTEX_PROGRAM)
			{
				progType = "Vertex";
			}
			else if (mType == GPT_GEOMETRY_PROGRAM)
			{
				progType = "Geometry";
			}
			else if (mType == GPT_DOMAIN_PROGRAM)
			{
				progType = "Tesselation Evaluation";
			}
			else if (mType == GPT_HULL_PROGRAM)
			{
				progType = "Tesselation Control";
			}
			else if (mType == GPT_COMPUTE_PROGRAM)
			{
				progType = "Compute";
			}
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        progType + " Program " + mName +
                        " failed to compile. See compile log above for details.",
                        "GLSLProgram::compile");
        }

		return (mCompiled == 1);
	}

	//-----------------------------------------------------------------------
	void GLSLProgram::createLowLevelImpl(void)
	{
		mAssemblerProgram = GpuProgramPtr(OGRE_NEW GLSLGpuProgram( this ));
        // Shader params need to be forwarded to low level implementation
        mAssemblerProgram->setAdjacencyInfoRequired(isAdjacencyInfoRequired());
	}
	//---------------------------------------------------------------------------
	void GLSLProgram::unloadImpl()
	{   
		// We didn't create mAssemblerProgram through a manager, so override this
		// implementation so that we don't try to remove it from one. Since getCreator()
		// is used, it might target a different matching handle!
		mAssemblerProgram.setNull();

		unloadHighLevel();
	}
	//-----------------------------------------------------------------------
	void GLSLProgram::unloadHighLevelImpl(void)
	{
		if (isSupported())
		{
			OGRE_CHECK_GL_ERROR(glDeleteShader(mGLShaderHandle));

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS) && mGLProgramHandle)
            {
                OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
            }
            
            mGLShaderHandle = 0;
            mGLProgramHandle = 0;
            mCompiled = 0;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
	{
		getConstantDefinitions();
		params->_setNamedConstants(mConstantDefs);
		// Don't set logical / physical maps here, as we can't access parameters by logical index in GLSL.
	}
	//-----------------------------------------------------------------------
	void GLSLProgram::buildConstantDefinitions() const
	{
		// We need an accurate list of all the uniforms in the shader, but we
		// can't get at them until we link all the shaders into a program object.

		// Therefore instead, parse the source code manually and extract the uniforms
		createParameterMappingStructures(true);
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLProgramPipelineManager::getSingleton().extractConstantDefs(mSource, *mConstantDefs.get(), mName);
        }
        else
        {
            GLSLLinkProgramManager::getSingleton().extractConstantDefs(mSource, *mConstantDefs.get(), mName);
        }

		// Also parse any attached sources
		for (GLSLProgramContainer::const_iterator i = mAttachedGLSLPrograms.begin();
			i != mAttachedGLSLPrograms.end(); ++i)
		{
			GLSLProgram* childShader = *i;

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                GLSLProgramPipelineManager::getSingleton().extractConstantDefs(childShader->getSource(),
                                                                               *mConstantDefs.get(), childShader->getName());
            }
            else
            {
                GLSLLinkProgramManager::getSingleton().extractConstantDefs(childShader->getSource(),
                                                                           *mConstantDefs.get(), childShader->getName());
            }
		}
	}

	//---------------------------------------------------------------------
	inline bool GLSLProgram::getPassSurfaceAndLightStates(void) const
	{
		// Scenemanager should pass on light & material state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	inline bool GLSLProgram::getPassTransformStates(void) const
	{
		// Scenemanager should pass on transform state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	inline bool GLSLProgram::getPassFogStates(void) const
	{
		// Scenemanager should pass on fog state to the rendersystem
		return true;
	}
	//-----------------------------------------------------------------------
    String GLSLProgram::CmdAttach::doGet(const void *target) const
    {
        return (static_cast<const GLSLProgram*>(target))->getAttachedShaderNames();
    }
	//-----------------------------------------------------------------------
    void GLSLProgram::CmdAttach::doSet(void *target, const String& shaderNames)
    {
		// Get all the shader program names: there could be more than one
		StringVector vecShaderNames = StringUtil::split(shaderNames, " \t", 0);

		size_t programNameCount = vecShaderNames.size();
		for ( size_t i = 0; i < programNameCount; ++i )
		{
	        static_cast<GLSLProgram*>(target)->attachChildShader(vecShaderNames[i]);
		}
    }
    //-----------------------------------------------------------------------
    String GLSLProgram::CmdColumnMajorMatrices::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const GLSLProgram*>(target)->getColumnMajorMatrices());
    }
    void GLSLProgram::CmdColumnMajorMatrices::doSet(void *target, const String& val)
    {
        static_cast<GLSLProgram*>(target)->setColumnMajorMatrices(StringConverter::parseBool(val));
    }
	//-----------------------------------------------------------------------
	String GLSLProgram::CmdPreprocessorDefines::doGet(const void *target) const
	{
		return static_cast<const GLSLProgram*>(target)->getPreprocessorDefines();
	}
	void GLSLProgram::CmdPreprocessorDefines::doSet(void *target, const String& val)
	{
		static_cast<GLSLProgram*>(target)->setPreprocessorDefines(val);
	}
	//-----------------------------------------------------------------------
    String GLSLProgram::CmdInputOperationType::doGet(const void* target) const
    {
        const GLSLProgram* t = static_cast<const GLSLProgram*>(target);
		return operationTypeToString(t->getInputOperationType());
    }
    void GLSLProgram::CmdInputOperationType::doSet(void* target, const String& val)
    {
        GLSLProgram* t = static_cast<GLSLProgram*>(target);
		t->setInputOperationType(parseOperationType(val));
    }
	//-----------------------------------------------------------------------
	String GLSLProgram::CmdOutputOperationType::doGet(const void* target) const
    {
        const GLSLProgram* t = static_cast<const GLSLProgram*>(target);
		return operationTypeToString(t->getOutputOperationType());
    }
    void GLSLProgram::CmdOutputOperationType::doSet(void* target, const String& val)
    {
        GLSLProgram* t = static_cast<GLSLProgram*>(target);
		t->setOutputOperationType(parseOperationType(val));
    }
	//-----------------------------------------------------------------------
	String GLSLProgram::CmdMaxOutputVertices::doGet(const void* target) const
	{
		const GLSLProgram* t = static_cast<const GLSLProgram*>(target);
		return StringConverter::toString(t->getMaxOutputVertices());
	}
	void GLSLProgram::CmdMaxOutputVertices::doSet(void* target, const String& val)
	{
		GLSLProgram* t = static_cast<GLSLProgram*>(target);
		t->setMaxOutputVertices(StringConverter::parseInt(val));
	}

	//-----------------------------------------------------------------------
    void GLSLProgram::attachChildShader(const String& name)
	{
		// is the name valid and already loaded?
		// check with the high level program manager to see if it was loaded
		HighLevelGpuProgramPtr hlProgram = HighLevelGpuProgramManager::getSingleton().getByName(name);
		if (!hlProgram.isNull())
		{
			if (hlProgram->getSyntaxCode() == "glsl")
			{
				// make sure attached program source gets loaded and compiled
				// don't need a low level implementation for attached shader objects
				// loadHighLevelImpl will only load the source and compile once
				// so don't worry about calling it several times
				GLSLProgram* childShader = static_cast<GLSLProgram*>(hlProgram.getPointer());
				// load the source and attach the child shader only if supported
				if (isSupported())
				{
					childShader->loadHighLevelImpl();
					// add to the container
					mAttachedGLSLPrograms.push_back( childShader );
					mAttachedShaderNames += name + " ";
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	void GLSLProgram::attachToProgramObject( const GLuint programObject )
	{
		// attach child objects
		GLSLProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
		GLSLProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

 		while (childprogramcurrent != childprogramend)
		{
			GLSLProgram* childShader = *childprogramcurrent;
			// bug in ATI GLSL linker : modules without main function must be recompiled each time 
			// they are linked to a different program object
			// don't check for compile errors since there won't be any
			// *** minor inconvenience until ATI fixes there driver
			childShader->compile(true);

			childShader->attachToProgramObject( programObject );

			++childprogramcurrent;
		}
        OGRE_CHECK_GL_ERROR(glAttachShader(programObject, mGLShaderHandle));
		logObjectInfo( "Error attaching " + mName + " shader object to GLSL Program Object", programObject );
    }

	//-----------------------------------------------------------------------
	void GLSLProgram::detachFromProgramObject( const GLuint programObject )
	{
        OGRE_CHECK_GL_ERROR(glDetachShader(programObject, mGLShaderHandle));
		logObjectInfo( "Error detaching " + mName + " shader object from GLSL Program Object", programObject );
		// attach child objects
		GLSLProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
		GLSLProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

		while (childprogramcurrent != childprogramend)
		{
			GLSLProgram* childShader = *childprogramcurrent;
			childShader->detachFromProgramObject( programObject );
			++childprogramcurrent;
		}
	}

    //-----------------------------------------------------------------------
    const String& GLSLProgram::getLanguage(void) const
    {
        static const String language = "glsl";

        return language;
    }
	//-----------------------------------------------------------------------
	Ogre::GpuProgramParametersSharedPtr GLSLProgram::createParameters( void )
	{
		GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();
		return params;
	}
	//-----------------------------------------------------------------------
	void GLSLProgram::checkAndFixInvalidDefaultPrecisionError( String &message )
	{
		String precisionQualifierErrorString = ": 'Default Precision Qualifier' :  invalid type Type for default precision qualifier can be only float or int";
		vector< String >::type linesOfSource = StringUtil::split(mSource, "\n");
		if( message.find(precisionQualifierErrorString) != String::npos )
		{
			LogManager::getSingleton().logMessage("Fixing invalid type Type for default precision qualifier by deleting bad lines the re-compiling", LML_CRITICAL);

			// remove relevant lines from source
			vector< String >::type errors = StringUtil::split(message, "\n");

			// going from the end so when we delete a line the numbers of the lines before will not change
			for(int i = (int)errors.size() - 1 ; i != -1 ; i--)
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
			// Triangle list is the default fallback. Keep it this way?
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
