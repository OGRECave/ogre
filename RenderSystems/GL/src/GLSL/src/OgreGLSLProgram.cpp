/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreGpuProgram.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreLogManager.h"

#include "OgreGLSLProgram.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLPreprocessor.h"

namespace Ogre {

    //-----------------------------------------------------------------------
	GLSLProgram::CmdPreprocessorDefines GLSLProgram::msCmdPreprocessorDefines;
    GLSLProgram::CmdAttach GLSLProgram::msCmdAttach;
	GLSLProgram::CmdInputOperationType GLSLProgram::msInputOperationTypeCmd;
	GLSLProgram::CmdOutputOperationType GLSLProgram::msOutputOperationTypeCmd;
	GLSLProgram::CmdMaxOutputVertices GLSLProgram::msMaxOutputVerticesCmd;

    //-----------------------------------------------------------------------
    //---------------------------------------------------------------------------
    GLSLProgram::~GLSLProgram()
    {
        // have to call this here reather than in Resource destructor
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
		// only create a shader object if glsl is supported
		if (isSupported())
		{
			checkForGLSLError( "GLSLProgram::GLSLProgram", "GL Errors before creating shader object", 0 );
			// create shader object

			GLenum shaderType = 0x0000;
			switch (mType)
			{
			case GPT_VERTEX_PROGRAM:
				shaderType = GL_VERTEX_SHADER_ARB;
				break;
			case GPT_FRAGMENT_PROGRAM:
				shaderType = GL_FRAGMENT_SHADER_ARB;
				break;
			case GPT_GEOMETRY_PROGRAM:
				shaderType = GL_GEOMETRY_SHADER_EXT;
				break;
			}
			mGLHandle = glCreateShaderObjectARB(shaderType);

			checkForGLSLError( "GLSLProgram::GLSLProgram", "Error creating GLSL shader Object", 0 );
		}

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
						// set up a definition, skip delim
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

		// Add preprocessor extras and main source
		if (!mSource.empty())
		{
			const char *source = mSource.c_str();
			glShaderSourceARB(mGLHandle, 1, &source, NULL);
			// check for load errors
			checkForGLSLError( "GLSLProgram::loadFromSource", "Cannot load GLSL high-level shader source : " + mName, 0 );
		}

		compile();
	}
    
    //---------------------------------------------------------------------------
	bool GLSLProgram::compile(const bool checkErrors)
	{
        if (checkErrors)
        {
            logObjectInfo("GLSL compiling: " + mName, mGLHandle);
        }

		glCompileShaderARB(mGLHandle);
		// check for compile errors
		glGetObjectParameterivARB(mGLHandle, GL_OBJECT_COMPILE_STATUS_ARB, &mCompiled);
		// force exception if not compiled
		if (checkErrors)
		{
			checkForGLSLError( "GLSLProgram::loadFromSource", "Cannot compile GLSL high-level shader : " + mName + " ", mGLHandle, !mCompiled, !mCompiled );
			
			if (mCompiled)
			{
				logObjectInfo("GLSL compiled : " + mName, mGLHandle);
			}
		}
		return (mCompiled == 1);

	}

	//-----------------------------------------------------------------------
	void GLSLProgram::createLowLevelImpl(void)
	{
		mAssemblerProgram = GpuProgramPtr(new GLSLGpuProgram( this ));
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
			glDeleteObjectARB(mGLHandle);
		}
	}

	//-----------------------------------------------------------------------
	void GLSLProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
	{
		params->_setNamedConstants(&getConstantDefinitions());
		// Don't set logical / physical maps here, as we can't access parameters by logical index in GLHL.
	}
	//-----------------------------------------------------------------------
	void GLSLProgram::buildConstantDefinitions() const
	{
		// We need an accurate list of all the uniforms in the shader, but we
		// can't get at them until we link all the shaders into a program object.


		// Therefore instead, parse the source code manually and extract the uniforms
		mConstantDefs.floatBufferSize = 0;
		mConstantDefs.intBufferSize = 0;
		GLSLLinkProgramManager::getSingleton().extractConstantDefs(
			mSource, mConstantDefs, mName);

		// Also parse any attached sources
		for (GLSLProgramContainer::const_iterator i = mAttachedGLSLPrograms.begin();
			i != mAttachedGLSLPrograms.end(); ++i)
		{
			GLSLProgram* childShader = *i;

			GLSLLinkProgramManager::getSingleton().extractConstantDefs(
				childShader->getSource(), mConstantDefs, childShader->getName());

		}
	}

	//-----------------------------------------------------------------------
    GLSLProgram::GLSLProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader),
            mInputOperationType(RenderOperation::OT_TRIANGLE_LIST),
            mOutputOperationType(RenderOperation::OT_TRIANGLE_LIST), mMaxOutputVertices(3)
    {
		// add parameter command "attach" to the material serializer dictionary
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
	//---------------------------------------------------------------------
	bool GLSLProgram::getPassSurfaceAndLightStates(void) const
	{
		// scenemanager should pass on light & material state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	bool GLSLProgram::getPassTransformStates(void) const
	{
		// scenemanager should pass on transform state to the rendersystem
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
		//get all the shader program names: there could be more than one
		StringVector vecShaderNames = StringUtil::split(shaderNames, " \t", 0);

		size_t programNameCount = vecShaderNames.size();
		for ( size_t i = 0; i < programNameCount; ++i )
		{
	        static_cast<GLSLProgram*>(target)->attachChildShader(vecShaderNames[i]);
		}
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
	void GLSLProgram::attachToProgramObject( const GLhandleARB programObject )
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
			childShader->compile(false);

			childShader->attachToProgramObject( programObject );

			++childprogramcurrent;
		}
		glAttachObjectARB( programObject, mGLHandle );
		checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram",
			"Error attaching " + mName + " shader object to GLSL Program Object", programObject );

	}
	//-----------------------------------------------------------------------
	void GLSLProgram::detachFromProgramObject( const GLhandleARB programObject )
	{
		glDetachObjectARB(programObject, mGLHandle);
		checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram",
			"Error detaching " + mName + " shader object from GLSL Program Object", programObject );
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

  
}
