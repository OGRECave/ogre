/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

	@note
		GLSL ES supports multiple modular shader objects that can be attached to one program
		object to form a single shader.  This is supported through the "attach" material script
		command.  All the modules to be attached are listed on the same line as the attach command
		seperated by white space.
        
    */
    class _OgrePrivate GLSLESProgram : public HighLevelGpuProgram
    {
    public:
        /// Command object for attaching another GLSL Program 
        class CmdAttach : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& shaderNames);
        };

        GLSLESProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
		~GLSLESProgram();

		const GLuint getGLHandle() const { return mGLHandle; }
		void attachToProgramObject( const GLuint programObject );
		void detachFromProgramObject( const GLuint programObject );
		String getAttachedShaderNames() const { return mAttachedShaderNames; }

		/// Overridden
		bool getPassTransformStates(void) const;
		bool getPassSurfaceAndLightStates(void) const;
		bool getPassFogStates(void) const;

        /// Attach another GLSL ES Shader to this one.
        void attachChildShader(const String& name);

		/// Sets the preprocessor defines use to compile the program.
		void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
		/// Sets the preprocessor defines use to compile the program.
		const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;
		/// Overridden from GpuProgram
		GpuProgramParametersSharedPtr createParameters(void);


		/// Command object for setting macro defines
		class CmdPreprocessorDefines : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
	protected:
		static CmdPreprocessorDefines msCmdPreprocessorDefines;
        static CmdAttach msCmdAttach;

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating a dummy low-level program for this
        high-level program.	GLSL ES does not give access to the low level implementation of the
		shader so this method creates an object sub-classed from GLES2GpuProgram just to be
		compatible with	GLES2RenderSystem.
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
		/// compile source into shader object
		bool compile( const bool checkErrors = false);

	private:
		/// GL handle for shader object
		GLuint mGLHandle;
		/// Flag indicating if shader object successfully compiled
		GLint mCompiled;
		/// Attached Shader names
		String mAttachedShaderNames;
		/// Preprocessor options
		String mPreprocessorDefines;
		/// Container of attached programs
		typedef vector< GLSLESProgram* >::type GLSLESProgramContainer;
		typedef GLSLESProgramContainer::iterator GLSLESProgramContainerIterator;
		GLSLESProgramContainer mAttachedGLSLPrograms;

    };
}

#endif // __GLSLESProgram_H__
