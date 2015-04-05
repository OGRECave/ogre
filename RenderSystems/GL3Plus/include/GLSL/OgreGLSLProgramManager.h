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
#ifndef __GLSLProgramManager_H__
#define __GLSLProgramManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreGLSLShader.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLExtSupport.h"

namespace Ogre {

    /** Ogre assumes that there are separate programs to deal with but
        GLSL has one program object that represents the active shader
        objects during a rendering state.  GLSL shader objects are
        compiled separately and then attached to a program object and
        then the program object is linked.  Since Ogre can only handle
        one program being active in a pass, the GLSL Link Program
        Manager does the same.  The GLSL Link program manager acts as
        a state machine and activates a program object based on the
        active programs.  Previously created program objects are
        stored along with a unique key in a hash_map for quick
        retrieval the next time the program object is required.
    */

    class _OgreGL3PlusExport GLSLProgramManager
    {
    protected:
        /// Active shader objects defining the active program object.
        GLSLShader* mActiveVertexShader;
        GLSLShader* mActiveHullShader;
        GLSLShader* mActiveDomainShader;
        GLSLShader* mActiveGeometryShader;
        GLSLShader* mActiveFragmentShader;
        GLSLShader* mActiveComputeShader;

        const GL3PlusSupport& mGLSupport;

        typedef map<String, GLenum>::type StringToEnumMap;
        /// 
        StringToEnumMap mTypeEnumMap;

        /**  Convert GL uniform size and type to OGRE constant types
             and associate uniform definitions together. */
        void convertGLUniformtoOgreType(GLenum gltype, 
                                        GpuConstantDefinition& defToUpdate);
        /** Find the data source definition for a given uniform name
            and reference. Return true if found and pair the reference
            with its data source. */
        bool findUniformDataSource(
            const String& paramName,
            const GpuConstantDefinitionMap* vertexConstantDefs,
            const GpuConstantDefinitionMap* hullConstantDefs,
            const GpuConstantDefinitionMap* domainConstantDefs,
            const GpuConstantDefinitionMap* geometryConstantDefs,
            const GpuConstantDefinitionMap* fragmentConstantDefs,
            const GpuConstantDefinitionMap* computeConstantDefs,
            GLUniformReference& refToUpdate);
        /** Find the data source definition for a given atomic counter
            uniform name and reference. Return true if found and pair
            the reference with its data source. */
        bool findAtomicCounterDataSource(
            const String& paramName,
            const GpuConstantDefinitionMap* vertexConstantDefs,
            const GpuConstantDefinitionMap* hullConstantDefs,
            const GpuConstantDefinitionMap* domainConstantDefs,
            const GpuConstantDefinitionMap* geometryConstantDefs,
            const GpuConstantDefinitionMap* fragmentConstantDefs,
            const GpuConstantDefinitionMap* computeConstantDefs,
            GLAtomicCounterReference& refToUpdate);
        /** Parse an individual uniform from a GLSL source file and
            store it in a GpuNamedConstant. */
        void parseGLSLUniform(
            const String& src, GpuNamedConstants& defs,
            String::size_type currPos,
            const String& filename, GpuSharedParametersPtr sharedParams);

    public:

        GLSLProgramManager(const GL3PlusSupport& support);

        /** Populate a list of uniforms based on an OpenGL program object.
            @param programObject Handle to the program object to query.
            @param vertexConstantDefs Definition of the uniforms extracted from the
            vertex program, used to match up physical buffer indexes with program
            uniforms. May be null if there is no vertex program.
            @param fragmentConstantDefs Definition of the uniforms extracted from the
            fragment program, used to match up physical buffer indexes with program
            uniforms. May be null if there is no fragment program.
            @param list The list to populate (will not be cleared before adding, clear
            it yourself before calling this if that's what you want).
        */
        void extractUniformsFromProgram(
            GLuint programObject,
            const GpuConstantDefinitionMap* vertexConstantDefs,
            const GpuConstantDefinitionMap* hullConstantDefs,
            const GpuConstantDefinitionMap* domainConstantDefs,
            const GpuConstantDefinitionMap* geometryConstantDefs,
            const GpuConstantDefinitionMap* fragmentConstantDefs,
            const GpuConstantDefinitionMap* computeConstantDefs,
            GLUniformReferenceList& uniformList,
            GLAtomicCounterReferenceList& counterList,
            GLUniformBufferList& uniformBufferList,
            SharedParamsBufferMap& sharedParamsBufferMap,
            //GLShaderStorageBufferList& shaderStorageBufferList,
            GLCounterBufferList& counterBufferList);
        /** Populate a list of uniforms based on GLSL source and store
            them in GpuNamedConstants.  
            @param src Reference to the source code.
            @param constantDefs The defs to populate (will
            not be cleared before adding, clear it yourself before
            calling this if that's what you want).  
            @param filename The file name this came from, for logging errors.
        */
        void extractUniformsFromGLSL(
            const String& src, GpuNamedConstants& constantDefs, const String& filename);
    };

}

#endif // __GLSLProgramManager_H__
