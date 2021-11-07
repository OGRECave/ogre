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
#ifndef _OgreMetalProgram_H_
#define _OgreMetalProgram_H_

#include "OgreMetalPrerequisites.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHardwareVertexBuffer.h"
#import <Metal/MTLLibrary.h>
#import <Metal/MTLRenderPipeline.h>
#import <Metal/MTLRenderCommandEncoder.h>

@class MTLArgument;

namespace Ogre
{
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
        /// Command object for setting vertex shader pair
        class CmdShaderReflectionPairHint : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        MetalProgram( ResourceManager* creator, const String& name, ResourceHandle handle,
                      const String& group, bool isManual, ManualResourceLoader* loader,
                      MetalDevice *device );
        virtual ~MetalProgram();

        /** Sets the entry point for this program ie the first method called. */
        void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
        /** Gets the entry point defined for this program. */
        const String& getEntryPoint(void) const { return mEntryPoint; }

        /// If this shader is a pixel shader, sets a vertex shader that can be paired with us
        /// for properly getting reflection data for GPU program parameters.
        void setShaderReflectionPairHint(const String& shaderName)
                                                    { mShaderReflectionPairHint = shaderName; }
        /// Gets the paired shader. See setShaderReflectionPairHint.
        const String& getShaderReflectionPairHint(void) const       { return mShaderReflectionPairHint; }

        /// Overridden from GpuProgram
        const String& getLanguage(void) const;
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);

        /// Retrieve the Metal function object
        id<MTLFunction> getMetalFunction(void) const    { return mFunction; }

        /// Compile source into shader object
        bool compile(const bool checkErrors = false);
        void analyzeComputeParameters(void);
        void analyzeRenderParameters(void);
        static void autoFillDummyVertexAttributesForShader( id<MTLFunction> inVertexFunction,
                                                            MTLRenderPipelineDescriptor *outPsd );
        void analyzeParameterBuffer( MTLArgument *arg );

        static uint32 getAttributeIndex(VertexElementSemantic semantic);

        // first 15 slots are reserved for the vertex attribute buffers
        constexpr static int UNIFORM_INDEX_START = 16;
    protected:
        static CmdShaderReflectionPairHint msCmdShaderReflectionPairHint;

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /// noop
        void createLowLevelImpl(void) {}
        /// shortcut as we there is no low-level separation here
        GpuProgram* _getBindingDelegate(void) { return this; }
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void);

        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions(void) override;

        void parsePreprocessorDefinitions( NSMutableDictionary<NSString*, NSObject*> *inOutMacros );

    private:
        id <MTLLibrary> mLibrary;
        id <MTLFunction> mFunction;

        MetalDevice *mDevice;

        /// Flag indicating if shader object successfully compiled
        bool mCompiled;
        /// Preprocessor options
        String mTargetBufferName;
        String mShaderReflectionPairHint;
    };
}

#endif // __MetalProgram_H__
