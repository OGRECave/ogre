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
#ifndef __GLSLLinkProgram_H__
#define __GLSLLinkProgram_H__

#include "OgreGLPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreGLUniformCache.h"
#include "OgreGLSLProgramCommon.h"

namespace Ogre {
    namespace GLSL {

    /** C++ encapsulation of GLSL Program Object

    */

    class _OgreGLExport GLSLLinkProgram : public GLSLProgramCommon
    {
    private:
        GLUniformCache *mUniformCache;

        /// Build uniform references from active named uniforms
        void buildGLUniformReferences(void);
        /// Extract attributes
        void extractAttributes(void);

        typedef std::set<GLuint> AttributeSet;
        /// Custom attribute bindings
        AttributeSet mValidAttributes;

        /// Compiles and links the the vertex and fragment programs
        void compileAndLink();
        /// Get the the binary data of a program from the microcode cache
        void getMicrocodeFromCache(uint32 id);
    public:
        /// Constructor should only be used by GLSLLinkProgramManager
        explicit GLSLLinkProgram(const GLShaderList& shaders);
        ~GLSLLinkProgram(void);

        /** Makes a program object active by making sure it is linked and then putting it in use.

        */
        void activate(void);

        bool isAttributeValid(VertexElementSemantic semantic, uint index);
        
        /** Updates program object uniforms using data from GpuProgramParameters.
        normally called by GLSLGpuProgram::bindParameters() just before rendering occurs.
        */
        void updateUniforms(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);

        /// Get the GL Handle for the program object
        uint getGLHandle(void) const { return mGLProgramHandle; }
    };

    }
}

#endif // __GLSLLinkProgram_H__
