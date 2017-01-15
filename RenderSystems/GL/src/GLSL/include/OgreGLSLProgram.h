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
#ifndef __GLSLProgram_H__
#define __GLSLProgram_H__

#include "OgreGLPrerequisites.h"
#include "OgreGLSLShaderCommon.h"
#include "OgreRenderOperation.h"

namespace Ogre {
    namespace GLSL {
    /** Specialisation of HighLevelGpuProgram to provide support for OpenGL
        Shader Language (GLSL).
    @remarks
        GLSL has no target assembler or entry point specification like DirectX 9 HLSL.
        Vertex and Fragment shaders only have one entry point called "main".  
        When a shader is compiled, microcode is generated but can not be accessed by
        the application.
        GLSL also does not provide assembler low level output after compiling.  The GL Render
        system assumes that the Gpu program is a GL Gpu program so GLSLProgram will create a 
        GLSLGpuProgram that is subclassed from GLGpuProgram for the low level implementation.
        The GLSLProgram class will create a shader object and compile the source but will
        not create a program object.  It's up to GLSLGpuProgram class to request a program object
        to link the shader object to.

    @note
        GLSL supports multiple modular shader objects that can be attached to one program
        object to form a single shader.  This is supported through the "attach" material script
        command.  All the modules to be attached are listed on the same line as the attach command
        separated by white space.
        
    */
    class _OgreGLExport GLSLProgram : public GLSLShaderCommon
    {
    public:
        GLSLProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~GLSLProgram();

        GLhandleARB getGLHandle() const { return mGLHandle; }
        void attachToProgramObject( const GLhandleARB programObject );
        void detachFromProgramObject( const GLhandleARB programObject );

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;

        /// compile source into shader object
        bool compile( bool checkErrors = true);
    protected:
        /** Internal method for creating a dummy low-level program for this
        high-level program. GLSL does not give access to the low level implementation of the
        shader so this method creates an object sub-classed from GLGpuProgram just to be
        compatible with GLRenderSystem.
        */
        void createLowLevelImpl(void);
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void);

        /// Populate the passed parameters with name->index map
        void populateParameterNames(GpuProgramParametersSharedPtr params);
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() const;

    private:
        /// GL handle for shader object
        GLhandleARB mGLHandle;
    };
    }
}

#endif // __GLSLProgram_H__
