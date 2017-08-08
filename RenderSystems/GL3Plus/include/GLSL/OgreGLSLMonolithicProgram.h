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
#ifndef __GLSLMonolithicProgram_H__
#define __GLSLMonolithicProgram_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreGLSLProgram.h"

namespace Ogre {

    class GLSLShader;

    /** Model of OpenGL program object created using the glLinkProgram
        method of linking.
        
        Linking using glLinkProgram is supported by OpenGL 2.0 and up,
        but does not allow hot-swapping shaders without recompiling
        the program object like GLSLSeparableProgram can. Hence the name
        'monolithic'.
    */
    class _OgreGL3PlusExport GLSLMonolithicProgram : public GLSLProgram
    {
    protected:
        /// Compiles and links the vertex and fragment programs
        void compileAndLink(void);
        /// Put a program in use
        void _useProgram(void);

        void buildGLUniformReferences(void);

    public:
        /// Constructor should only be used by GLSLMonolithicProgramManager
        GLSLMonolithicProgram(GLSLShader* vertexProgram,
                              GLSLShader* hullProgram,
                              GLSLShader* domainProgram,
                              GLSLShader* geometryProgram,
                              GLSLShader* fragmentProgram,
                              GLSLShader* computeProgram);
        ~GLSLMonolithicProgram(void);

        /** Makes a program object active by making sure it is linked
            and then putting it in use.
        */
        void activate(void);

        /** Updates program object uniforms using data from
            GpuProgramParameters.  normally called by
            GLSLShader::bindParameters() just before rendering
            occurs.
        */
        void updateUniforms(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);

        /** Updates program object uniform blocks using data from
            GpuProgramParameters.  normally called by
            GLSLShader::bindParameters() just before rendering
            occurs.
        */
        void updateUniformBlocks(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);
        /** Updates program object uniforms using data from pass
            iteration GpuProgramParameters.  normally called by
            GLSLShader::bindMultiPassParameters() just before multi
            pass rendering occurs.
        */
        void updatePassIterationUniforms(GpuProgramParametersSharedPtr params);
    };

}

#endif // __GLSLMonolithicProgram_H__
