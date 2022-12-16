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

#include "OgreGLSupportPrerequisites.h"
#include "OgreString.h"
#include "OgreGpuProgramParams.h"
#include "OgreGLSLProgramCommon.h"

namespace Ogre {
    class GLSLShaderCommon;

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
    class GLSLProgramManagerCommon
    {
    protected:
        typedef std::map<String, GpuConstantType> StringToEnumMap;
        StringToEnumMap mTypeEnumMap;

        /** Parse an individual uniform from a GLSL source file and
            store it in a GpuNamedConstant. */
        void parseGLSLUniform(String line, GpuNamedConstants& defs, const String& filename);

        /// checks whether the param with the given name should be added to the uniforms list
        static bool validateParam(String paramName, uint32 numActiveArrayElements,
                                  const GpuConstantDefinitionMap* (&constantDefs)[6], GLUniformReference& refToUpdate);

        typedef std::map<uint32, GLSLProgramCommon*> ProgramMap;
        typedef ProgramMap::iterator ProgramIterator;

        /// container holding previously created program objects
        ProgramMap mPrograms;

        /// Active shader objects defining the active program object.
        GLShaderList mActiveShader;
        /// active objects defining the active rendering gpu state
        GLSLProgramCommon* mActiveProgram;
    public:
        GLSLProgramManagerCommon();
        virtual ~GLSLProgramManagerCommon();

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

        /// Destroy all programs which referencing this shader
        void destroyAllByShader(GLSLShaderCommon* shader);

        /** Set the shader for the next rendering state.
            The active program object will be cleared.
        */
        void setActiveShader(GpuProgramType type, GLSLShaderCommon* shader);
    };

}

#endif // __GLSLProgramManagerCommon_H__
