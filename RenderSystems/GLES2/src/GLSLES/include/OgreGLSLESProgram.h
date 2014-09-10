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
#ifndef __GLSLESProgram_H__
#define __GLSLESProgram_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreGLES2ManagedResource.h"

namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to provide support for OpenGL 
        Shader Language (GLSL ES) for OpenGL ES 2.0.
    @remarks
        GLSL ES has no target assembler or entry point specification like DirectX 9 HLSL.
        Vertex and Fragment shaders only have one entry point called "main".  
        When a shader is compiled, microcode is generated but can not be accessed by
        the application.
        GLSL ES also does not provide assembler low level output after compiling.  The GL ES Render
        system assumes that the Gpu program is a GL Gpu program so GLSLESProgram will create a 
        GLSLESGpuProgram that is subclassed from GLES2GpuProgram for the low level implementation.
        The GLES2Program class will create a shader object and compile the source but will
        not create a program object.  It's up to GLES2GpuProgram class to request a program object
        to link the shader object to.
    */
    class _OgreGLES2Export GLSLESProgram : public HighLevelGpuProgram MANAGED_RESOURCE
    {
    public:
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        /// Command object for running the GLSL optimiser 
        class CmdOptimisation : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
#endif
        /// Command object for setting macro defines
        class CmdPreprocessorDefines : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        
        GLSLESProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~GLSLESProgram();

        /// GL Shader Handle
        GLuint getGLShaderHandle() const { return mGLShaderHandle; }
        void attachToProgramObject( const GLuint programObject );
        void detachFromProgramObject( const GLuint programObject );
        GLuint getGLProgramHandle() const { return mGLProgramHandle; }

        /// Overridden
        bool getPassTransformStates(void) const;
        bool getPassSurfaceAndLightStates(void) const;
        bool getPassFogStates(void) const;

        /// Sets the preprocessor defines use to compile the program.
        void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
        /// Sets the preprocessor defines use to compile the program.
        const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }

#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        /// Sets if the GLSL optimiser is enabled.
        void setOptimiserEnabled(bool enabled);
        /// Gets if the GLSL optimiser is enabled.
        bool getOptimiserEnabled(void) const { return mOptimiserEnabled; }
        
        /// Sets if the GLSL source has been optimised successfully
        void setIsOptimised(bool flag) { mIsOptimised = flag; }
        /// Gets if the GLSL source has been optimised successfully
        bool getIsOptimised(void) { return mIsOptimised; }

        /// Sets the optimised GLSL source 
        void setOptimisedSource(const String& src) { mOptimisedSource = src; }
        /// Gets he optimised GLSL source 
        String getOptimisedSource(void) { return mOptimisedSource; }
#endif

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);

        /// compile source into shader object
        bool compile( const bool checkErrors = false);

    protected:
        static CmdPreprocessorDefines msCmdPreprocessorDefines;
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        static CmdOptimisation msCmdOptimisation;
#endif

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating a dummy low-level program for this
        high-level program. GLSL ES does not give access to the low level implementation of the
        shader so this method creates an object sub-classed from GLES2GpuProgram just to be
        compatible with GLES2RenderSystem.
        */
        void createLowLevelImpl(void);
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void);
        /// Overridden from HighLevelGpuProgram
        void unloadImpl(void);

        /// Populate the passed parameters with name->index map
        void populateParameterNames(GpuProgramParametersSharedPtr params);
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() const;
        /** check the compile result for an error with default precision - and recompile if needed.
            some glsl compilers return an error default precision is set to types other then
            int or float, this function test a failed compile result for the error,
            delete the needed lines from the source if needed then try to re-compile.
        */
        void checkAndFixInvalidDefaultPrecisionError( String &message );
        
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        /** See AndroidResource. */
        virtual void notifyOnContextLost();
#endif
        
    private:
        /// GL handle for shader object
        GLuint mGLShaderHandle;
        GLuint mGLProgramHandle;
        /// Flag indicating if shader object successfully compiled
        GLint mCompiled;
        /// Preprocessor options
        String mPreprocessorDefines;
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        /// Flag indicating if shader has been successfully optimised
        bool mIsOptimised;
        bool mOptimiserEnabled;
        /// The optmised source of the program (may be blank until the shader is optmisied)
        String mOptimisedSource;
#endif
    };
}

#endif // __GLSLESProgram_H__
