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
#include "OgreMetalProgram.h"
#include "OgreLogManager.h"
#include "OgreMetalDevice.h"
#include "OgreMetalMappings.h"
#include "OgreGpuProgramManager.h"

#import <Metal/MTLDevice.h>
#import <Metal/MTLVertexDescriptor.h>
#import <Metal/MTLComputePipeline.h>

namespace Ogre {
    static const uint32 VERTEX_ATTRIBUTE_INDEX[VES_COUNT] =
    {
        0,  // VES_POSITION - 1
        1,  // VES_BLEND_WEIGHTS - 1
        7,  // VES_BLEND_INDICES - 1
        2,  // VES_NORMAL - 1
        3,  // VES_DIFFUSE - 1
        4,  // VES_SPECULAR - 1
        8,  // VES_TEXTURE_COORDINATES - 1
        //There are up to 8 VES_TEXTURE_COORDINATES. Occupy range [8; 16)
        //Range [14; 16) overlaps with VES_TANGENT & VES_BINORMAL
        //(slot 16 is where const buffers start)
        15,// VES_BINORMAL - 1
        14,  // VES_TANGENT - 1
    };

    uint32 MetalProgram::getAttributeIndex(VertexElementSemantic semantic)
    {
        return VERTEX_ATTRIBUTE_INDEX[semantic - 1];
    }

    //-----------------------------------------------------------------------
    MetalProgram::CmdShaderReflectionPairHint MetalProgram::msCmdShaderReflectionPairHint;
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MetalProgram::MetalProgram( ResourceManager* creator, const String& name,
                                ResourceHandle handle, const String& group,
                                bool isManual, ManualResourceLoader* loader,
                                MetalDevice *device ) :
        HighLevelGpuProgram(creator, name, handle, group, isManual, loader),
        mLibrary( nil ),
        mFunction( nil ),
        mDevice( device ),
        mCompiled( false )
    {
        if (createParamDictionary("MetalProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("shader_reflection_pair_hint",
                                            "Metal requires Pixel Shaders to be paired with a valid "
                                            "vertex shader to obtain reflection data (i.e. program "
                                            "parameters). Pixel Shaders without parameters don't need "
                                            "this. Pass the name of an already defined vertex shader.",
                                            PT_STRING),&msCmdShaderReflectionPairHint);
        }
        mTargetBufferName = "";
        mEntryPoint = "mainfunc"; // 1.4.4: A Metal function cannot be called main.
    }
    //---------------------------------------------------------------------------
    MetalProgram::~MetalProgram()
    {
        mLibrary = nil;
        mFunction = nil;

        // Have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }
    //-----------------------------------------------------------------------
    void MetalProgram::loadFromSource(void)
    {
        compile( true );
    }
    //-----------------------------------------------------------------------
    void MetalProgram::parsePreprocessorDefinitions( NSMutableDictionary<NSString*,
                                                     NSObject*> *inOutMacros )
    {
        String defines = appendBuiltinDefines(mPreprocessorDefines);
        for(auto def : parseDefines(defines))
        {
            NSString *key = [NSString stringWithUTF8String:def.first];
            NSString *value = [NSString stringWithUTF8String:def.second];
            inOutMacros[key] = value;
        }
    }
    //-----------------------------------------------------------------------
    bool MetalProgram::compile(const bool checkErrors)
    {
        mCompileError = true; //Set to true until we've confirmed otherwise.

        //Send fixed vertex attributes as macros/definitions.
        MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
        NSMutableDictionary<NSString *, NSObject *> *preprocessorMacros =
                [NSMutableDictionary dictionary];

        parsePreprocessorDefinitions( preprocessorMacros );

        options.preprocessorMacros = preprocessorMacros;

        // metal does not support runtime #includes. Also we want to use our Resource system
        mSource = _resolveIncludes(mSource, this, mFilename, true);

        NSError *error;
        mLibrary = [mDevice->mDevice newLibraryWithSource:[NSString stringWithUTF8String:mSource.c_str()]
                                                  options:options
                                                    error:&error];

        if( !mLibrary && checkErrors )
        {
            String errorDesc;
            if( error )
                errorDesc = [error localizedDescription].UTF8String;

            LogManager::getSingleton().logError(
                        "Metal Compiler in " + mName + ":\n" + errorDesc );
        }
        else
        {
            mCompiled = true;

            if( error )
            {
                String errorDesc;
                if( error )
                    errorDesc = [error localizedDescription].UTF8String;
                LogManager::getSingleton().logWarning(
                            "Metal SL Compiler in " + mName + ":\n" + errorDesc );
            }
        }

        mLibrary.label = [NSString stringWithUTF8String:mName.c_str()];

        mFunction = [mLibrary newFunctionWithName:[NSString stringWithUTF8String:mEntryPoint.c_str()]];
        if( !mFunction )
        {
            mCompiled = false;
            LogManager::getSingleton().logError(
                        "retriving entry point '" + mEntryPoint + "' in shader " + mName );
        }

        // Log a message that the shader compiled successfully.
        if( mCompiled && checkErrors )
            LogManager::getSingleton().logMessage( "Shader " + mName + " compiled successfully." );

        mCompileError = !mCompiled;

        if( !mCompiled )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         ((mType == GPT_VERTEX_PROGRAM) ? "Vertex Program " : "Fragment Program ") +
                         mName + " failed to compile. See compile log above for details.",
                         "MetalProgram::compile" );
        }

        return mCompiled;
    }

    //-----------------------------------------------------------------------
    void MetalProgram::autoFillDummyVertexAttributesForShader( id<MTLFunction> vertexFunction,
                                                               MTLRenderPipelineDescriptor *psd )
    {
        if( [vertexFunction.vertexAttributes count] )
        {
            size_t maxSize = 0;
            MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];

            for( MTLVertexAttribute *vertexAttribute in vertexFunction.vertexAttributes )
            {
                const size_t elementIdx = vertexAttribute.attributeIndex;
                vertexDescriptor.attributes[elementIdx].format =
                        MetalMappings::dataTypeToVertexFormat( vertexAttribute.attributeType );
                vertexDescriptor.attributes[elementIdx].bufferIndex = 0;
                vertexDescriptor.attributes[elementIdx].offset = elementIdx * 16u;

                maxSize = std::max( maxSize, elementIdx * 16u );
            }

            vertexDescriptor.layouts[0].stride = maxSize + 16u;
            vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

            [psd setVertexDescriptor:vertexDescriptor];
        }
    }
    //-----------------------------------------------------------------------
    void MetalProgram::analyzeComputeParameters(void)
    {
        MTLAutoreleasedComputePipelineReflection reflection = 0;
        NSError* error = 0;
        id<MTLFunction> metalFunction = this->getMetalFunction();
        id<MTLComputePipelineState> pso =
                [mDevice->mDevice newComputePipelineStateWithFunction:metalFunction
                                                              options:MTLPipelineOptionBufferTypeInfo
                                                           reflection:&reflection
                                                                error:&error];
        if( !pso || error )
        {
            String errorDesc;
            if( error )
                errorDesc = [error localizedDescription].UTF8String;

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create pipeline state for reflection, error " +
                         errorDesc, "MetalProgram::analyzeComputeParameters" );
        }
        else
        {
            createParameterMappingStructures( true );
            NSArray<MTLArgument*> *arguments = reflection.arguments;

            for( MTLArgument *arg in arguments )
            {
                if( arg.type == MTLArgumentTypeBuffer && arg.index >= UNIFORM_INDEX_START )
                    analyzeParameterBuffer( arg );
            }
        }
    }
    //-----------------------------------------------------------------------
    void MetalProgram::analyzeRenderParameters(void)
    {
        MTLRenderPipelineDescriptor *psd = [[MTLRenderPipelineDescriptor alloc] init];
        //[psd setSampleCount: 1];

        switch( mType )
        {
        case GPT_VERTEX_PROGRAM:
        {
            id<MTLFunction> metalFunction = this->getMetalFunction();
            [psd setVertexFunction:metalFunction];
            autoFillDummyVertexAttributesForShader( metalFunction, psd );
            break;
        }
        case GPT_FRAGMENT_PROGRAM:
        {
            GpuProgramPtr shader = GpuProgramManager::getSingleton().
                    getByName( mShaderReflectionPairHint );
            if( !shader )
            {
                OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                             "Shader reflection hint '" + mShaderReflectionPairHint +
                             "' not found for pixel shader '" + mName + "'",
                             "MetalProgram::analyzeRenderParameters" );
            }
            if( shader->getType() != GPT_VERTEX_PROGRAM )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Shader reflection hint '" + mShaderReflectionPairHint +
                             "' for pixel shader '" + mName + "' must be a vertex shader.",
                             "MetalProgram::analyzeRenderParameters" );
            }
            shader->load();

            if(shader->hasCompileError())
                return;

            assert( dynamic_cast<MetalProgram*>( shader->_getBindingDelegate() ) );
            MetalProgram *vertexShader = static_cast<MetalProgram*>( shader->_getBindingDelegate() );
            autoFillDummyVertexAttributesForShader( vertexShader->getMetalFunction(), psd );
            [psd setVertexFunction:vertexShader->getMetalFunction()];
            [psd setFragmentFunction:this->getMetalFunction()];
            break;
        }
        default:
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, "", "MetalProgram::analyzeRenderParameters" );
        }

        psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        MTLAutoreleasedRenderPipelineReflection reflection = 0;
        NSError* error = 0;
        id <MTLRenderPipelineState> pso =
                [mDevice->mDevice newRenderPipelineStateWithDescriptor:psd
                options:MTLPipelineOptionBufferTypeInfo reflection:&reflection error:&error];

        if( !pso || error )
        {
            String errorDesc;
            if( error )
                errorDesc = [error localizedDescription].UTF8String;

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create pipeline state for reflection, error " +
                         errorDesc, "MetalProgram::analyzeRenderParameters" );
        }
        else
        {
            createParameterMappingStructures( true );
            NSArray<MTLArgument*> *arguments =
                    mType == GPT_VERTEX_PROGRAM ? reflection.vertexArguments :
                                                  reflection.fragmentArguments;

            for( MTLArgument *arg in arguments )
            {
                if( arg.type == MTLArgumentTypeBuffer && arg.index >= UNIFORM_INDEX_START )
                    analyzeParameterBuffer( arg );
            }
        }
    }
    //-----------------------------------------------------------------------
    void MetalProgram::analyzeParameterBuffer( MTLArgument *arg )
    {
        if( arg.bufferDataSize == 0 )
            return;

        {
            //Check if not a struct (i.e. a pointer to a basic type, like a float4x4*)
            GpuConstantType ogreType = MetalMappings::get( arg.bufferDataType );
            if( ogreType != GCT_UNKNOWN )
            {
                GpuConstantDefinition def;
                def.constType = ogreType;
                def.logicalIndex = 0;
                def.physicalIndex = 0;
                def.elementSize = GpuConstantDefinition::getElementSize(def.constType, false);
                def.arraySize   = 1;
                def.variability = GPV_GLOBAL;

                def.physicalIndex = mLogicalToPhysical->bufferSize*4;
                OGRE_LOCK_MUTEX(mLogicalToPhysical->mutex);
                mLogicalToPhysical->map.insert(
                GpuLogicalIndexUseMap::value_type(def.logicalIndex,
                GpuLogicalIndexUse(def.physicalIndex,
                                    def.arraySize * def.elementSize, GPV_GLOBAL, BCT_UNKNOWN)));
                mLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
                mConstantDefs->bufferSize = mLogicalToPhysical->bufferSize;

                String varName = arg.name.UTF8String;

                mConstantDefs->map.insert( GpuConstantDefinitionMap::value_type( varName, def ) );
            }
        }

        for( MTLStructMember *member in arg.bufferStructType.members )
        {
            GpuConstantType ogreType;

            if( member.dataType == MTLDataTypeArray )
                ogreType = MetalMappings::get( member.arrayType.elementType );
            else
                ogreType = MetalMappings::get( member.dataType );

            if( ogreType != GCT_UNKNOWN )
            {
                GpuConstantDefinition def;
                def.constType = ogreType;
                def.logicalIndex = member.offset;
                def.physicalIndex = member.offset;
                if( member.dataType == MTLDataTypeArray )
                {
                    def.elementSize = member.arrayType.stride / sizeof(float);
                    def.arraySize   = member.arrayType.arrayLength;
                }
                else
                {
                    def.elementSize = GpuConstantDefinition::getElementSize(def.constType, false);
                    def.arraySize   = 1;
                }
                def.variability = GPV_GLOBAL;

                OGRE_LOCK_MUTEX(mLogicalToPhysical->mutex);
                mLogicalToPhysical->map.insert(
                GpuLogicalIndexUseMap::value_type(def.logicalIndex,
                GpuLogicalIndexUse(def.physicalIndex,
                                    def.arraySize * def.elementSize, GPV_GLOBAL, BCT_UNKNOWN)));
                mLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
                mConstantDefs->bufferSize = mLogicalToPhysical->bufferSize;

                String varName = member.name.UTF8String;

                mConstantDefs->map.insert( GpuConstantDefinitionMap::value_type( varName, def ) );
            }
        }
    }
    //-----------------------------------------------------------------------
    void MetalProgram::unloadHighLevelImpl(void)
    {
        // Release everything
        mLibrary = nil;
        mFunction = nil;
        mCompiled = false;
    }
    //-----------------------------------------------------------------------
    void MetalProgram::buildConstantDefinitions(void)
    {
        if( mType == GPT_FRAGMENT_PROGRAM && mShaderReflectionPairHint.empty() )
        {
            LogManager::getSingleton().logMessage(
                        "WARNING: Pixel Shader '" + mName + "' without shader_reflection_pair_hint. "
                        "If this is intentional, use build_parameters_from_reflection false to hide "
                        "this warning.");
            return;
        }

        if( mCompileError )
            return;

        if( !mLibrary )
            return;

        if( mType != GPT_COMPUTE_PROGRAM )
        {
            analyzeRenderParameters();
        }
        else
        {
            analyzeComputeParameters();
        }

        mLogicalToPhysical.reset(); // disallow access by index for now
    }
    //-----------------------------------------------------------------------
    String MetalProgram::CmdShaderReflectionPairHint::doGet(const void *target) const
    {
        return static_cast<const MetalProgram*>(target)->getShaderReflectionPairHint();
    }
    //-----------------------------------------------------------------------
    void MetalProgram::CmdShaderReflectionPairHint::doSet(void *target, const String& val)
    {
        static_cast<MetalProgram*>(target)->setShaderReflectionPairHint(val);
    }
    //-----------------------------------------------------------------------
    const String& MetalProgram::getLanguage(void) const
    {
        static const String language = "metal";

        return language;
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr MetalProgram::createParameters( void )
    {
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();
        params->setTransposeMatrices(true);
        return params;
    }
}
