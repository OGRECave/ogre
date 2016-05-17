/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#ifndef __OgreMetalProgram_H__
#define __OgreMetalProgram_H__

#import "OgreMetalPrerequisites.h"
#import "OgreHighLevelGpuProgram.h"
#import "OgreHardwareVertexBuffer.h"
#import <Metal/MTLLibrary.h>
#import <Metal/MTLRenderPipeline.h>
#import <Metal/MTLRenderCommandEncoder.h>


namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to provide support for Metal
        Shader Language.
    @remarks
        Metal has no target assembler or entry point specification like DirectX 9 HLSL.
        Vertex and Fragment shaders only have one entry point called "main".  
        When a shader is compiled, microcode is generated but can not be accessed by
        the application.
        Metal also does not provide assembler low level output after compiling.  The Metal Render
        system assumes that the Gpu program is a Metal Gpu program so MetalProgram will create a 
        MetalGpuProgram that is subclassed from MetalGpuProgram for the low level implementation.
        The MetalProgram class will create a shader object and compile the source but will
        not create a program object.  It's up to MetalGpuProgram class to request a program object
        to link the shader object to.
    */
    class _OgreMetalExport MetalProgram : public HighLevelGpuProgram
    {
    public:

        /// Command object for setting macro defines
        class CmdPreprocessorDefines : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        
        /// Command object for setting entry point
        class CmdEntryPoint : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        MetalProgram(ResourceManager* creator,
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~MetalProgram();

        /// Overridden
        bool getPassTransformStates(void) const;
        bool getPassSurfaceAndLightStates(void) const;
        bool getPassFogStates(void) const;

        /// Sets the preprocessor defines use to compile the program.
        void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
        /// Sets the preprocessor defines use to compile the program.
        const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }

        /** Sets the entry point for this program ie the first method called. */
        void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
        /** Gets the entry point defined for this program. */
        const String& getEntryPoint(void) const { return mEntryPoint; }

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);

        /// Retrieve the Metal function object
        id <MTLFunction> getMetalFunction(void) { return mFunction; }

        /// Compile source into shader object
        bool compile(const bool checkErrors = false);

        /// Execute the binding functions for this program
        void bindProgram(void);
        /// Execute the unbinding functions for this program
        void unbindProgram(void);
        /// Execute the param binding functions for this program
        void bindProgramParameters(id <MTLRenderCommandEncoder> &encoder, GpuProgramParametersSharedPtr params, uint16 mask);
        void bindProgramPassIterationParameters(GpuProgramParametersSharedPtr params) {}
        void bindUniformBuffers(id <MTLRenderCommandEncoder> &encoder, const Renderable *rend, const Pass *pass, const GpuProgramParametersSharedPtr params, MTLRenderPipelineDescriptor *pipeline);
        void updateAttributeBuffer(id <MTLRenderCommandEncoder> &encoder, const size_t index, const size_t vertexStart, v1::HardwareVertexBufferSharedPtr hwBuffer);
        NSUInteger getFunctionParamCount(void);
        size_t getSharedParamCount(void);

    protected:
        static CmdPreprocessorDefines msCmdPreprocessorDefines;
        static CmdEntryPoint msCmdEntryPoint;

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating a dummy low-level program for this
        high-level program. Metal does not give access to the low level implementation of the
        shader so this method creates an object sub-classed from MetalGpuProgram just to be
        compatible with MetalRenderSystem.
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

    private:
        id <MTLLibrary> mLibrary;
        id <MTLFunction> mFunction;
        
        /// Flag indicating if shader object successfully compiled
        bool mCompiled;
        /// Preprocessor options
        String mPreprocessorDefines;
        String mEntryPoint;
        String mTargetBufferName;
    };
}

#endif // __MetalProgram_H__
