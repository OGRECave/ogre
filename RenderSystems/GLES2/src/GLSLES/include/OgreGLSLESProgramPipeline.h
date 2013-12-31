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
#ifndef __GLSLESProgramPipeline_H__
#define __GLSLESProgramPipeline_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreGLSLESProgramCommon.h"

namespace Ogre
{
    /** Specialisation of HighLevelGpuProgram to provide support for OpenGL 
     Shader Language (GLSL ES) for OpenGL ES 2.0.
     @remarks
     GLSL ES has no target assembler or entry point specification like DirectX 9 HLSL.
     Vertex and Fragment shaders only have one entry point called "main".  
     When a shader is compiled, microcode is generated but can not be accessed by
     the application.
     GLSL ES also does not provide assembler low level output after compiling.  The GL ES Render
     system assumes that the Gpu program is a GL Gpu program so GLSLESProgramPipeline will create a 
     GLSLESGpuProgram that is subclassed from GLES2GpuProgram for the low level implementation.
     The GLES2Program class will create a shader and program object and compile the source but will
     not create a pipeline object.  It's up to GLES2GpuProgram class to request a program pipeline object
     to link the program object to.

     @note
     GLSL ES supports multiple modular shader objects that can be attached to one program
     object to form a single shader.  This is supported through the "attach" material script
     command.  All the modules to be attached are listed on the same line as the attach command
     separated by white space.
     */
    class _OgreGLES2Export GLSLESProgramPipeline : public GLSLESProgramCommon
    {
    public:
		/// Constructor should only be used by GLSLESProgramPipelineManager
		GLSLESProgramPipeline(GLSLESGpuProgram* vertexProgram, GLSLESGpuProgram* fragmentProgram);
		virtual ~GLSLESProgramPipeline();

        /// GL Program Pipeline Handle
		GLuint getGLProgramPipelineHandle() const { return mGLProgramPipelineHandle; }

        /** Updates program pipeline object uniforms using data from GpuProgramParameters.
         normally called by GLSLESGpuProgram::bindParameters() just before rendering occurs.
         */
		virtual void updateUniforms(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);
		/** Updates program object uniform blocks using data from GpuProgramParameters.
         normally called by GLSLGpuProgram::bindParameters() just before rendering occurs.
         */
		virtual void updateUniformBlocks(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType);
		/** Updates program pipeline object uniforms using data from pass iteration GpuProgramParameters.
         normally called by GLSLESGpuProgram::bindMultiPassParameters() just before multi pass rendering occurs.
         */
		virtual void updatePassIterationUniforms(GpuProgramParametersSharedPtr params);

        /** Makes a program pipeline object active by making sure it is linked and then putting it in use.
         */
		void activate(void);

        /// Get the index of a non-standard attribute bound in the linked code
		virtual GLint getAttributeIndex(VertexElementSemantic semantic, uint index);

	protected:
        enum {
            VERTEX_PROGRAM_LINKED = 1,
            FRAGMENT_PROGRAM_LINKED = 2,
            ALL_PROGRAMS_LINKED = 3
        };
		/// GL handle for pipeline object
        GLuint mGLProgramPipelineHandle;

		/// Compiles and links the separate vertex and fragment programs
        virtual void compileAndLink(void);
        /// Put a program pipeline in use
        virtual void _useProgram(void);
        /// Finds layout qualifiers in the shader source and sets attribute indices appropriately
        virtual void extractLayoutQualifiers(void);
        /// Build uniform references from active named uniforms
		virtual void buildGLUniformReferences(void);
    };
}

#endif
