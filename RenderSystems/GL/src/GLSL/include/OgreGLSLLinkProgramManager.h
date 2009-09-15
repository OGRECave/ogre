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
#ifndef __GLSLLinkProgramManager_H__
#define __GLSLLinkProgramManager_H__

#include "OgreGLPrerequisites.h"
#include "OgreSingleton.h"

#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgram.h"

namespace Ogre {


	/** Ogre assumes that there are seperate vertex and fragment programs to deal with but
		GLSL has one program object that represents the active vertex and fragment shader objects
		during a rendering state.  GLSL Vertex and fragment 
		shader objects are compiled seperately and then attached to a program object and then the
		program object is linked.  Since Ogre can only handle one vertex program and one fragment
		program being active in a pass, the GLSL Link Program Manager does the same.  The GLSL Link
		program manager acts as a state machine and activates a program object based on the active
		vertex and fragment program.  Previously created program objects are stored along with a unique
		key in a hash_map for quick retrieval the next time the program object is required.

	*/

	class _OgrePrivate GLSLLinkProgramManager : public Singleton<GLSLLinkProgramManager>
	{

	private:
	
		typedef map<uint64, GLSLLinkProgram*>::type LinkProgramMap;
		typedef LinkProgramMap::iterator LinkProgramIterator;

		/// container holding previously created program objects 
		LinkProgramMap mLinkPrograms; 

		/// active objects defining the active rendering gpu state
		GLSLGpuProgram* mActiveVertexGpuProgram;
		GLSLGpuProgram* mActiveGeometryGpuProgram;
		GLSLGpuProgram* mActiveFragmentGpuProgram;
		GLSLLinkProgram* mActiveLinkProgram;

		typedef map<String, GLenum>::type StringToEnumMap;
		StringToEnumMap mTypeEnumMap;

		/// Use type to complete other information
		void completeDefInfo(GLenum gltype, GpuConstantDefinition& defToUpdate);
		/// Find where the data for a specific uniform should come from, populate
		bool completeParamSource(const String& paramName,
			const GpuConstantDefinitionMap* vertexConstantDefs, 
			const GpuConstantDefinitionMap* geometryConstantDefs,
			const GpuConstantDefinitionMap* fragmentConstantDefs,
			GLUniformReference& refToUpdate);

	public:

		GLSLLinkProgramManager(void);

		~GLSLLinkProgramManager(void);

		/**
			Get the program object that links the two active shader objects together
			if a program object was not already created and linked a new one is created and linked
		*/
		GLSLLinkProgram* getActiveLinkProgram(void);

		/** Set the active fragment shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveFragmentShader(GLSLGpuProgram* fragmentGpuProgram);
		/** Set the active geometry shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveGeometryShader(GLSLGpuProgram* geometryGpuProgram);
		/** Set the active vertex shader for the next rendering state.
			The active program object will be cleared.
			Normally called from the GLSLGpuProgram::bindProgram and unbindProgram methods
		*/
		void setActiveVertexShader(GLSLGpuProgram* vertexGpuProgram);

		/** Populate a list of uniforms based on a program object.
		@param programObject Handle to the program object to query
		@param vertexConstantDefs Definition of the constants extracted from the
			vertex program, used to match up physical buffer indexes with program
			uniforms. May be null if there is no vertex program.
		@param geometryConstantDefs Definition of the constants extracted from the
			geometry program, used to match up physical buffer indexes with program
			uniforms. May be null if there is no geometry program.
		@param fragmentConstantDefs Definition of the constants extracted from the
			fragment program, used to match up physical buffer indexes with program
			uniforms. May be null if there is no fragment program.
		@param list The list to populate (will not be cleared before adding, clear
		it yourself before calling this if that's what you want).
		*/
		void extractUniforms(GLhandleARB programObject, 
			const GpuConstantDefinitionMap* vertexConstantDefs, 
			const GpuConstantDefinitionMap* geometryConstantDefs,
			const GpuConstantDefinitionMap* fragmentConstantDefs,
			GLUniformReferenceList& list);
		/** Populate a list of uniforms based on GLSL source.
		@param src Reference to the source code
		@param list The defs to populate (will not be cleared before adding, clear
		it yourself before calling this if that's what you want).
		@param filename The file name this came from, for logging errors.
		*/
		void extractConstantDefs(const String& src, GpuNamedConstants& constantDefs, 
			const String& filename);

		static GLSLLinkProgramManager& getSingleton(void);
        static GLSLLinkProgramManager* getSingletonPtr(void);

	};

}

#endif // __GLSLLinkProgramManager_H__
