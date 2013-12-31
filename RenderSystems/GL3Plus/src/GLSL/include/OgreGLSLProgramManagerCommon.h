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
#ifndef __GLSLProgramManagerCommon_H__
#define __GLSLProgramManagerCommon_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreGLSLProgramCommon.h"
#include "OgreGLSLExtSupport.h"

namespace Ogre {


	/** Ogre assumes that there are separate programs to deal with but
		GLSL has one program object that represents the active shader objects
		during a rendering state.  GLSL shader objects are compiled separately and then attached 
        to a program object and then the program object is linked.  Since Ogre can only handle one program
		being active in a pass, the GLSL Link Program Manager does the same.  The GLSL Link
		program manager acts as a state machine and activates a program object based on the active
        programs.  Previously created program objects are stored along with a unique
		key in a hash_map for quick retrieval the next time the program object is required.
	*/

	class _OgreGL3PlusExport GLSLProgramManagerCommon
	{
	protected:
		/// Active objects defining the active rendering gpu state
		GLSLGpuProgram* mActiveVertexGpuProgram;
		GLSLGpuProgram* mActiveGeometryGpuProgram;
		GLSLGpuProgram* mActiveFragmentGpuProgram;
		GLSLGpuProgram* mActiveHullGpuProgram;
		GLSLGpuProgram* mActiveDomainGpuProgram;
		GLSLGpuProgram* mActiveComputeGpuProgram;

		typedef map<String, GLenum>::type StringToEnumMap;
		StringToEnumMap mTypeEnumMap;
		/// Use type to complete other information
		void completeDefInfo(GLenum gltype, GpuConstantDefinition& defToUpdate);
		/// Find where the data for a specific uniform should come from, populate
		bool completeParamSource(const String& paramName,
             const GpuConstantDefinitionMap* vertexConstantDefs,
             const GpuConstantDefinitionMap* geometryConstantDefs,
             const GpuConstantDefinitionMap* fragmentConstantDefs,
             const GpuConstantDefinitionMap* hullConstantDefs,
             const GpuConstantDefinitionMap* domainConstantDefs,
             const GpuConstantDefinitionMap* computeConstantDefs,
			 GLUniformReference& refToUpdate);
        void parseIndividualConstant(const String& src, GpuNamedConstants& defs,
                                     String::size_type currPos,
                                     const String& filename, GpuSharedParametersPtr sharedParams);

	public:

		GLSLProgramManagerCommon(void);
		~GLSLProgramManagerCommon(void);

		/** Populate a list of uniforms based on a program object.
		@param programObject Handle to the program object to query
		@param vertexConstantDefs Definition of the constants extracted from the
			vertex program, used to match up physical buffer indexes with program
			uniforms. May be null if there is no vertex program.
		@param fragmentConstantDefs Definition of the constants extracted from the
			fragment program, used to match up physical buffer indexes with program
			uniforms. May be null if there is no fragment program.
		@param list The list to populate (will not be cleared before adding, clear
		it yourself before calling this if that's what you want).
		*/
		void extractUniforms(GLuint programObject, 
             const GpuConstantDefinitionMap* vertexConstantDefs,
             const GpuConstantDefinitionMap* geometryConstantDefs,
             const GpuConstantDefinitionMap* fragmentConstantDefs,
             const GpuConstantDefinitionMap* hullConstantDefs,
             const GpuConstantDefinitionMap* domainConstantDefs,
             const GpuConstantDefinitionMap* computeConstantDefs,
			 GLUniformReferenceList& list, GLUniformBufferList& sharedList);
		/** Populate a list of uniforms based on GLSL ES source.
		@param src Reference to the source code
		@param constantDefs The defs to populate (will not be cleared before adding, clear
		it yourself before calling this if that's what you want).
		@param filename The file name this came from, for logging errors.
		*/
		void extractConstantDefs(const String& src, GpuNamedConstants& constantDefs, 
			const String& filename);
	};

}

#endif // __GLSLProgramManagerCommon_H__
