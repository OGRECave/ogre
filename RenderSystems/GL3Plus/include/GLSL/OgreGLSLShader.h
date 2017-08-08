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
#ifndef __GLSLShader_H__
#define __GLSLShader_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreRenderOperation.h"

namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to encapsulate shader
        objects obtained from compiled shaders written in the OpenGL
        Shader Language (GLSL).
        @remarks
        GLSL has no target assembler or entry point specification
        like DirectX 9 HLSL.  Vertex and
        Fragment shaders only have one entry point called "main".
        When a shader is compiled, microcode is generated but can not
        be accessed by the application.  GLSL also does not provide
        assembler low level output after compiling.  The GL Render
        system assumes that the Gpu program is a GL GPU program so
        GLSLShader will create a GL3PlusShader
        for the low level implementation.  The
        GLSLShader class will create a shader object and compile the
        source but will not create a program object.  It's up to
        GL3PlusShader class to request a program object to link the
        shader object to.

        @note
        GLSL supports multiple modular shader objects that can
        be attached to one program object to form a single shader.
        This is supported through the "attach" material script
        command.  All the modules to be attached are listed on the
        same line as the attach command separated by white space.
    */
    class _OgreGL3PlusExport GLSLShader : public HighLevelGpuProgram
    {
    public:
        /// Command object for attaching another GLSL Program
        class CmdAttach : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& shaderNames);
        };

        /// Command object for setting macro defines
        class CmdPreprocessorDefines : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting the input operation type (geometry shader only)
        class _OgreGL3PlusExport CmdInputOperationType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting the output operation type (geometry shader only)
        class _OgreGL3PlusExport CmdOutputOperationType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting the maximum output vertices (geometry shader only)
        class _OgreGL3PlusExport CmdMaxOutputVertices : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting matrix packing in column-major order
        class CmdColumnMajorMatrices : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Returns the operation type that this geometry program expects to
            receive as input
        */
        virtual OperationType getInputOperationType(void) const
        { return mInputOperationType; }
        /** Returns the operation type that this geometry program will emit
         */
        virtual OperationType getOutputOperationType(void) const
        { return mOutputOperationType; }
        /** Returns the maximum number of vertices that this geometry program can
            output in a single run
        */
        virtual int getMaxOutputVertices(void) const { return mMaxOutputVertices; }

        /** Sets the operation type that this geometry program expects to receive
         */
        virtual void setInputOperationType(OperationType operationType)
        { mInputOperationType = operationType; }
        /** Set the operation type that this geometry program will emit
         */
        virtual void setOutputOperationType(OperationType operationType)
        { mOutputOperationType = operationType; }
        /** Set the maximum number of vertices that a single run of this geometry program
            can emit.
        */
        virtual void setMaxOutputVertices(int maxOutputVertices)
        { mMaxOutputVertices = maxOutputVertices; }

        GLSLShader(ResourceManager* creator,
                   const String& name, ResourceHandle handle,
                   const String& group, bool isManual, ManualResourceLoader* loader);
        // GL3PlusShader(
        //     ResourceManager* creator, const String& name, 
        //     ResourceHandle handle,
        //     const String& group, bool isManual = false, 
        //     ManualResourceLoader* loader = 0);
        ~GLSLShader();

        GLuint getGLShaderHandle() const { return mGLShaderHandle; }
        GLuint getGLProgramHandle();
        void attachToProgramObject(const GLuint programObject);
        void detachFromProgramObject(const GLuint programObject);
        String getAttachedShaderNames() const { return mAttachedShaderNames; }
        /// Get OpenGL GLSL shader type from OGRE GPU program type.
        GLenum getGLShaderType(GpuProgramType programType);
        /// Get a string containing the name of the GLSL shader type
        /// correspondening to the OGRE GPU program type.
        String getShaderTypeLabel(GpuProgramType programType);

        /// Overridden
        bool getPassTransformStates(void) const;
        bool getPassSurfaceAndLightStates(void) const;
        bool getPassFogStates(void) const;

        /** Attach another GLSL Shader to this one. */
        void attachChildShader(const String& name);

        /// Sets the preprocessor defines use to compile the program.
        void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
        /// Sets the preprocessor defines use to compile the program.
        const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;
        /** Sets whether matrix packing in column-major order. */
        void setColumnMajorMatrices(bool columnMajor) { mColumnMajorMatrices = columnMajor; }
        /** Gets whether matrix packed in column-major order. */
        bool getColumnMajorMatrices(void) const { return mColumnMajorMatrices; }

        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);

        /// Compile source into shader object
        bool compile( const bool checkErrors = false);


        /// Bind the shader in OpenGL.
        void bind(void);
        /// Unbind the shader in OpenGL.
        void unbind(void);
        static void unbindAll(void);
        /// Execute the param binding functions for this shader.
        void bindParameters(GpuProgramParametersSharedPtr params, uint16 mask);
        /// Execute the pass iteration param binding functions for this shader.
        void bindPassIterationParameters(GpuProgramParametersSharedPtr params);
        /// Execute the shared param binding functions for this shader.
        void bindSharedParameters(GpuProgramParametersSharedPtr params, uint16 mask);


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
        GLuint getShaderID(void) const { return mShaderID; }

        /// Since GLSL has no assembly, use this shader for binding.
        GpuProgram* _getBindingDelegate(void) { return this; }

    protected:
        static CmdPreprocessorDefines msCmdPreprocessorDefines;
        static CmdAttach msCmdAttach;
        static CmdColumnMajorMatrices msCmdColumnMajorMatrices;
        static CmdInputOperationType msInputOperationTypeCmd;
        static CmdOutputOperationType msOutputOperationTypeCmd;
        static CmdMaxOutputVertices msMaxOutputVerticesCmd;

        /** Internal load implementation, must be implemented by subclasses.
         */
        void loadFromSource(void);
        /** Internal method for creating a dummy low-level program for
            this high-level program.  GLSL does not give access to the
            low level implementation of the shader so this method
            creates an object sub-classed from GL3PlusShader just to
            be compatible with GL3PlusRenderSystem.
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
        /** Check the compile result for an error with default
            precision - and recompile if needed.  some glsl compilers
            return an error default precision is set to types other
            then int or float, this function test a failed compile
            result for the error, delete the needed lines from the
            source if needed then try to re-compile.
        */
        void checkAndFixInvalidDefaultPrecisionError( String &message );

        virtual void setUniformBlockBinding( const char *blockName, uint32 bindingSlot );


        // /// @copydoc Resource::loadImpl
        // void loadImpl(void) {}

        /// OGRE assigned shader ID.
        GLuint mShaderID;

    private:
        /// GL handle for shader object.
        GLuint mGLShaderHandle;

        /// GL handle for program object the shader is bound to.
        GLuint mGLProgramHandle;


        /// Flag indicating if shader object successfully compiled.
        GLint mCompiled;
        /// The input operation type for this (geometry) program.
        OperationType mInputOperationType;
        /// The output operation type for this (geometry) program.
        OperationType mOutputOperationType;
        /// The maximum amount of vertices that this (geometry) program can output.
        int mMaxOutputVertices;
        /// Attached shader names.
        String mAttachedShaderNames;
        /// Preprocessor options.
        String mPreprocessorDefines;
        /// Matrix in column major pack format?
        bool mColumnMajorMatrices;

        typedef vector< GLSLShader* >::type GLSLShaderContainer;
        typedef GLSLShaderContainer::iterator GLSLShaderContainerIterator;
        /// Container of attached shaders.
        GLSLShaderContainer mAttachedGLSLShaders;

        /// Keep track of the number of shaders created.
        static GLuint mShaderCount;

        /** Flag indicating that the shader has been successfully
            linked.
            Only used for separable programs. */
        GLint mLinked;
    };
}

#endif // __GLSLShader_H__
