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
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreGLSLShaderCommon.h"
#include "OgreGLES2ManagedResource.h"

namespace Ogre {
    class _OgreGLES2Export GLSLESProgram : public GLSLShaderCommon MANAGED_RESOURCE
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
        GLSLESProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~GLSLESProgram();

        void attachToProgramObject( const GLuint programObject ) override;
        void detachFromProgramObject( const GLuint programObject ) override;

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
        const String& getLanguage(void) const override;
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void) override;

        bool linkSeparable() override;

    protected:
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        static CmdOptimisation msCmdOptimisation;
#endif

        void loadFromSource() override;
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void) override;

        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() override;
        /** check the compile result for an error with default precision - and recompile if needed.
            some glsl compilers return an error default precision is set to types other then
            int or float, this function test a failed compile result for the error,
            delete the needed lines from the source if needed then try to re-compile.
        */
        void checkAndFixInvalidDefaultPrecisionError( String &message );
        
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        void notifyOnContextLost() override;
        void notifyOnContextReset() override;
#endif
        
    private:
#if !OGRE_NO_GLES2_GLSL_OPTIMISER
        /// Flag indicating if shader has been successfully optimised
        bool mIsOptimised;
        bool mOptimiserEnabled;
        /// The optmised source of the program (may be blank until the shader is optmisied)
        String mOptimisedSource;
#endif
    };

    /** Factory class for GLSL ES programs. */
    class GLSLESProgramFactory : public HighLevelGpuProgramFactory
    {
    public:
        /// Get the name of the language this factory creates programs for
        const String& getLanguage(void) const override;
        /// Create an instance of GLSLESProgram
        GpuProgram* create(ResourceManager* creator,
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader) override;
    };
}

#endif // __GLSLESProgram_H__
