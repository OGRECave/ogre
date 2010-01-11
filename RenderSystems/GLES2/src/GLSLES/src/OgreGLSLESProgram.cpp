/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreGLSLESProgram.h"
#include "OgreGLSLESGpuProgram.h"
#include "OgreGLSLESLinkProgramManager.h"
#include "OgreGLSLESPreprocessor.h"

namespace Ogre {

    //-----------------------------------------------------------------------
	GLSLESProgram::CmdPreprocessorDefines GLSLESProgram::msCmdPreprocessorDefines;
    GLSLESProgram::CmdAttach GLSLESProgram::msCmdAttach;

    //-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
    GLSLESProgram::GLSLESProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
    {
		// Add parameter command "attach" to the material serializer dictionary
        if (createParamDictionary("GLSLESProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("preprocessor_defines", 
				"Preprocessor defines use to compile the program.",
				PT_STRING),&msCmdPreprocessorDefines);
            dict->addParameter(ParameterDef("attach", 
                "name of another GLSL ES program needed by this program",
                PT_STRING),&msCmdAttach);
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
    //-----------------------------------------------------------------------
	void GLSLESProgram::loadFromSource(void)
	{
		// Only create a shader object if glsl es is supported
		if (isSupported())
		{
            GL_CHECK_ERROR

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
			mGLHandle = glCreateShader(shaderType);
            GL_CHECK_ERROR
		}

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

		// Add preprocessor extras and main source
		if (!mSource.empty())
		{
			const char *source = mSource.c_str();
			glShaderSource(mGLHandle, 1, &source, NULL);
			// Check for load errors
            GL_CHECK_ERROR
		}

		compile();
	}
    
    //---------------------------------------------------------------------------
	bool GLSLESProgram::compile(const bool checkErrors)
	{
        if (checkErrors)
            logObjectInfo("GLSL ES compiling: " + mName, mGLHandle);

		glCompileShader(mGLHandle);
        GL_CHECK_ERROR

		// Check for compile errors
		glGetShaderiv(mGLHandle, GL_COMPILE_STATUS, &mCompiled);
        if(checkErrors)
            logObjectInfo("GLSL ES compile log: " + mName, mGLHandle);

		// Force exception if not compiled
		if (checkErrors)
		{
			if (mCompiled)
				logObjectInfo("GLSL ES compiled: " + mName, mGLHandle);
		}
		return (mCompiled == 1);
	}

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
			glDeleteProgram(mGLHandle);
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
		GLSLESLinkProgramManager::getSingleton().extractConstantDefs(
			mSource, *mConstantDefs.get(), mName);

		// Also parse any attached sources
		for (GLSLESProgramContainer::const_iterator i = mAttachedGLSLPrograms.begin();
			i != mAttachedGLSLPrograms.end(); ++i)
		{
			GLSLESProgram* childShader = *i;

			GLSLESLinkProgramManager::getSingleton().extractConstantDefs(
				childShader->getSource(), *mConstantDefs.get(), childShader->getName());
		}
	}

	//---------------------------------------------------------------------
	bool GLSLESProgram::getPassSurfaceAndLightStates(void) const
	{
		// Scenemanager should pass on light & material state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	bool GLSLESProgram::getPassTransformStates(void) const
	{
		// Scenemanager should pass on transform state to the rendersystem
		return true;
	}
	//---------------------------------------------------------------------
	bool GLSLESProgram::getPassFogStates(void) const
	{
		// Scenemanager should pass on fog state to the rendersystem
		return true;
	}
	//-----------------------------------------------------------------------
    String GLSLESProgram::CmdAttach::doGet(const void *target) const
    {
        return (static_cast<const GLSLESProgram*>(target))->getAttachedShaderNames();
    }
	//-----------------------------------------------------------------------
    void GLSLESProgram::CmdAttach::doSet(void *target, const String& shaderNames)
    {
		// Get all the shader program names: there could be more than one
		StringVector vecShaderNames = StringUtil::split(shaderNames, " \t", 0);

		size_t programNameCount = vecShaderNames.size();
		for ( size_t i = 0; i < programNameCount; ++i )
		{
	        static_cast<GLSLESProgram*>(target)->attachChildShader(vecShaderNames[i]);
		}
    }
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
    void GLSLESProgram::attachChildShader(const String& name)
	{
		// Is the name valid and already loaded?
		// check with the high level program manager to see if it was loaded
		HighLevelGpuProgramPtr hlProgram = HighLevelGpuProgramManager::getSingleton().getByName(name);
		if (!hlProgram.isNull())
		{
			if (hlProgram->getSyntaxCode() == "glsles")
			{
				// Make sure attached program source gets loaded and compiled
				// don't need a low level implementation for attached shader objects
				// loadHighLevelImpl will only load the source and compile once
				// so don't worry about calling it several times
				GLSLESProgram* childShader = static_cast<GLSLESProgram*>(hlProgram.getPointer());
				// Load the source and attach the child shader only if supported
				if (isSupported())
				{
					childShader->loadHighLevelImpl();
					// Add to the container
					mAttachedGLSLPrograms.push_back( childShader );
					mAttachedShaderNames += name + " ";
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	void GLSLESProgram::attachToProgramObject( const GLuint programObject )
	{
		// Attach child objects
		GLSLESProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
		GLSLESProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

 		while (childprogramcurrent != childprogramend)
		{
			GLSLESProgram* childShader = *childprogramcurrent;
			childShader->attachToProgramObject(programObject);
			++childprogramcurrent;
		}

		glAttachShader(programObject, mGLHandle);
        GL_CHECK_ERROR
	}
	//-----------------------------------------------------------------------
	void GLSLESProgram::detachFromProgramObject( const GLuint programObject )
	{
		glDetachShader(programObject, mGLHandle);
        GL_CHECK_ERROR

		// Attach child objects
		GLSLESProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
		GLSLESProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

		while (childprogramcurrent != childprogramend)
		{
			GLSLESProgram* childShader = *childprogramcurrent;
			childShader->detachFromProgramObject( programObject );
			++childprogramcurrent;
		}

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
