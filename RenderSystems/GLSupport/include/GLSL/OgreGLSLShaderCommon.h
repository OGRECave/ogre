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
#ifndef __GLSLShaderCommon_H__
#define __GLSLShaderCommon_H__

#include "OgreGLSupportPrerequisites.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreRenderOperation.h"
#include "OgreGLUniformCache.h"

namespace Ogre {
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
    class GLSLShaderCommon : public HighLevelGpuProgram
    {
    public:
        /// Command object for attaching another GLSL Program 
        class CmdAttach : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& shaderNames);
        };
        /// Command object for setting matrix packing in column-major order
        class CmdColumnMajorMatrices : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        GLSLShaderCommon(ResourceManager* creator,
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);

        virtual bool compile( bool checkErrors = false) = 0;
        virtual void attachToProgramObject(const uint programObject) = 0;
        virtual void detachFromProgramObject(const uint programObject) = 0;

        String getAttachedShaderNames() const { return mAttachedShaderNames; }

        /// Overridden
        bool getPassTransformStates(void) const {
            return true;
        }
        bool getPassSurfaceAndLightStates(void) const {
            return true;
        }
        bool getPassFogStates(void) const {
            return true;
        }

        /** Attach another GLSL Shader to this one. */
        void attachChildShader(const String& name);

        /** Sets whether matrix packing in column-major order. */ 
        void setColumnMajorMatrices(bool columnMajor) { mColumnMajorMatrices = columnMajor; }
        /** Gets whether matrix packed in column-major order. */
        bool getColumnMajorMatrices(void) const { return mColumnMajorMatrices; }

        /** Returns the operation type that this geometry program expects to
            receive as input
        */
        RenderOperation::OperationType getInputOperationType(void) const
        { return mInputOperationType; }
        /** Returns the operation type that this geometry program will emit
        */
        RenderOperation::OperationType getOutputOperationType(void) const
        { return mOutputOperationType; }
        /** Returns the maximum number of vertices that this geometry program can
            output in a single run
        */
        int getMaxOutputVertices(void) const { return mMaxOutputVertices; }

        /** Sets the operation type that this geometry program expects to receive
        */
        void setInputOperationType(RenderOperation::OperationType operationType)
        { mInputOperationType = operationType; }
        /** Set the operation type that this geometry program will emit
        */
        void setOutputOperationType(RenderOperation::OperationType operationType)
        { mOutputOperationType = operationType; }
        /** Set the maximum number of vertices that a single run of this geometry program
            can emit.
        */
        void setMaxOutputVertices(int maxOutputVertices)
        { mMaxOutputVertices = maxOutputVertices; }

        /** Return the shader link status.
            Only used for separable programs.
        */
        int isLinked(void) { return mLinked; }

        /** Set the shader link status.
            Only used for separable programs.
        */
        void setLinked(int flag) { mLinked = flag; }

        /// Get the OGRE assigned shader ID.
        uint getShaderID(void) const { return mShaderID; }

        /// Get the uniform cache for this shader
        GLUniformCache*    getUniformCache(){return &mUniformCache;}

        /// Command object for setting macro defines
        class CmdPreprocessorDefines : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting the input operation type (geometry shader only)
        class CmdInputOperationType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting the output operation type (geometry shader only)
        class CmdOutputOperationType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting the maximum output vertices (geometry shader only)
        class CmdMaxOutputVertices : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
    protected:
        static CmdPreprocessorDefines msCmdPreprocessorDefines;
        static CmdAttach msCmdAttach;
        static CmdColumnMajorMatrices msCmdColumnMajorMatrices;
        static CmdInputOperationType msInputOperationTypeCmd;
        static CmdOutputOperationType msOutputOperationTypeCmd;
        static CmdMaxOutputVertices msMaxOutputVerticesCmd;

        String getResourceLogName() const;

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);

        /// Overridden from HighLevelGpuProgram
        void unloadImpl(void);

        /// Populate the passed parameters with name->index map
        void populateParameterNames(GpuProgramParametersSharedPtr params);

        /// The input operation type for this (geometry) program
        RenderOperation::OperationType mInputOperationType;
        /// The output operation type for this (geometry) program
        RenderOperation::OperationType mOutputOperationType;
        /// The maximum amount of vertices that this (geometry) program can output
        int mMaxOutputVertices;
        /// Attached Shader names
        String mAttachedShaderNames;
        /// Container of attached programs
        typedef std::vector< GLSLShaderCommon* > GLSLProgramContainer;
        typedef GLSLProgramContainer::iterator GLSLProgramContainerIterator;
        GLSLProgramContainer mAttachedGLSLPrograms;
        /// Matrix in column major pack format?
        bool mColumnMajorMatrices;

        /** Flag indicating that the shader has been successfully
            linked.
            Only used for separable programs. */
        int mLinked;

        /// OGRE assigned shader ID.
        uint mShaderID;

        /// Pointer to the uniform cache for this shader
        GLUniformCache    mUniformCache;

        /// Keep track of the number of shaders created.
        static uint mShaderCount;
    };
}

#endif // __GLSLProgramCommon_H__
