/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
n
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

#ifndef __GLSLAssembly_H__
#define __GLSLAssembly_H__

// Precompiler options
#include "OgreGLSLExtSupport.h"
#include "OgreGL3PlusShader.h"


namespace Ogre {

    /** GLSL low level compiled shader object - this class is used to
        get at the linked program object and provide an interface for
        GL3PlusRenderSystem calls.  GLSL does not provide access to
        the low level code of the shader so this class is really just
        a dummy place holder.  GLSL uses a program object to represent
        the active shaders used but Ogre materials maintain separate
        instances of the active shaders which creates a small problem
        for GLSL integration.  The GLSLAssembly class provides the
        interface between the GLSLProgramManagers,
        GL3PlusRenderSystem, and the active GLSLShader instances.
    */
    class _OgreGL3PlusExport GLSLAssembly : public GL3PlusShader
    {
    private:
        /// Associated GLSL shader.
        GLSLShader* mGLSLShader;

        /// Keep track of the number of vertex shaders created.
        static GLuint mVertexShaderCount;
        /// Keep track of the number of fragment shaders created.
        static GLuint mFragmentShaderCount;
        /// Keep track of the number of geometry shaders created.
        static GLuint mGeometryShaderCount;
        /// Keep track of the number of tesselation hull (control) shaders created.
        static GLuint mHullShaderCount;
        /// Keep track of the number of tesselation domain (evaluation) shaders created.
        static GLuint mDomainShaderCount;
        /// Keep track of the number of compute shaders created.
        static GLuint mComputeShaderCount;

        /** Flag indicating that the shader has been successfully
            linked. 
            Only used for separable programs. */
        GLint mLinked;

    public:
        GLSLAssembly(GLSLShader* parent);
        ~GLSLAssembly();

        /// Execute the binding functions for this program.
        void bindShader(void);
        /// Execute the unbinding functions for this program.
        void unbindShader(void);
        /// Execute the param binding functions for this program.
        void bindShaderParameters(GpuProgramParametersSharedPtr params, uint16 mask);
        /// Execute the pass iteration param binding functions for this program.
        void bindShaderPassIterationParameters(GpuProgramParametersSharedPtr params);
        /// Execute the shared param binding functions for this program.
        void bindShaderSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask);

        /// Get the GLSLShader for the shader object.
        GLSLShader* getGLSLShader(void) const { return mGLSLShader; }

        /** Return the shader link status.
            Only used for separable programs.
        */
        GLint isLinked(void) { return mLinked; }

        /** Set the shader link status.
            Only used for separable programs.
        */
        void setLinked(GLint flag) { mLinked = flag; }

        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const;

        /// Get the OGRE assigned shader ID.
        GLuint getShaderID(void) const
        { return mShaderID; }

        // /// Get shader type
        // GLenum getShaderType(GpuProgramType shaderType);

    protected:
        /// Overridden from GpuProgram.
        void loadFromSource(void);
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);
        /// @copydoc Resource::loadImpl
        void loadImpl(void);

        /// OGRE assigned shader ID.
        GLuint mShaderID;
    };
}

#endif // __GLSLAssembly_H__
