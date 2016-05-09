/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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

#include "OgreMetalRenderSystem.h"
#include "OgreMetalRenderWindow.h"
#include "OgreMetalTextureManager.h"
#include "Vao/OgreMetalVaoManager.h"
#include "OgreMetalHlmsPso.h"

#include "OgreDefaultHardwareBufferManager.h"

#include "OgreMetalMappings.h"

#import <Metal/Metal.h>
#import <Foundation/NSEnumerator.h>

namespace Ogre
{
    //-------------------------------------------------------------------------
    MetalRenderSystem::MetalRenderSystem() :
        RenderSystem(),
        mInitialized( false ),
        mHardwareBufferManager( 0 ),
        mPso( 0 ),
        mNumMRTs( 0 ),
        mCurrentDepthBuffer( 0 ),
        mRenderEncoder( 0 )
    {
        for( size_t i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
            mCurrentColourRTs[i] = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        mRenderPassAttachmentsMap.clear();
    }
    //-------------------------------------------------------------------------
    const String& MetalRenderSystem::getName(void) const
    {
        static String strName("Metal Rendering Subsystem");
        return strName;
    }
    //-------------------------------------------------------------------------
    const String& MetalRenderSystem::getFriendlyName(void) const
    {
        static String strName("Metal_RS");
        return strName;
    }
    //-------------------------------------------------------------------------
    HardwareOcclusionQuery* MetalRenderSystem::createHardwareOcclusionQuery(void)
    {
        return 0; //TODO
    }
    //-------------------------------------------------------------------------
    RenderSystemCapabilities* MetalRenderSystem::createRenderSystemCapabilities(void) const
    {
        RenderSystemCapabilities* rsc = new RenderSystemCapabilities();
        rsc->setRenderSystemName(getName());

        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setStencilBufferBitDepth(8);
        rsc->setNumTextureUnits(16);
        rsc->setCapability(RSC_ANISOTROPY);
        rsc->setCapability(RSC_AUTOMIPMAP);
        rsc->setCapability(RSC_BLENDING);
        rsc->setCapability(RSC_DOT3);
        rsc->setCapability(RSC_CUBEMAPPING);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
        rsc->setCapability(RSC_VBO);
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_USER_CLIP_PLANES);
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);
        rsc->setCapability(RSC_TEXTURE_3D);
        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        rsc->setNonPOW2TexturesLimited(false);
        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        rsc->setCapability(RSC_TEXTURE_FLOAT);
        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
        rsc->setMaxPointSize(256);

        rsc->setMaximumResolutions( 16384, 4096, 16384 );

        return rsc;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->_initialise(true);
    }
    //-------------------------------------------------------------------------
    RenderWindow* MetalRenderSystem::_initialise( bool autoCreateWindow, const String& windowTitle )
    {
        RenderWindow *autoWindow = 0;
        if( autoCreateWindow )
            autoWindow = _createRenderWindow( windowTitle, 1, 1, false );
        RenderSystem::_initialise(autoCreateWindow, windowTitle);

        return autoWindow;
    }
    //-------------------------------------------------------------------------
    RenderWindow* MetalRenderSystem::_createRenderWindow( const String &name,
                                                         unsigned int width, unsigned int height,
                                                         bool fullScreen,
                                                         const NameValuePairList *miscParams )
    {
        RenderWindow *win = OGRE_NEW MetalRenderWindow();

        if( !mInitialized )
        {
            mRealCapabilities = createRenderSystemCapabilities();
            mCurrentCapabilities = mRealCapabilities;

            mHardwareBufferManager = new v1::DefaultHardwareBufferManager();
            mTextureManager = new MetalTextureManager();
            mVaoManager = OGRE_NEW MetalVaoManager();

            mInitialized = true;
        }

        return win;
    }
    //-------------------------------------------------------------------------
    MultiRenderTarget* MetalRenderSystem::createMultiRenderTarget(const String & name)
    {
        return 0;
    }
    //-------------------------------------------------------------------------
    RenderTarget* MetalRenderSystem::detachRenderTarget( const String &name )
    {
        RenderTargetMap::iterator it = mRenderTargets.find( name );
        if( it != mRenderTargets.end() )
        {
            mRenderPassAttachmentsMap.erase( it->second );
        }

        RenderSystem::detachRenderTarget( name );
    }
    //-------------------------------------------------------------------------
    String MetalRenderSystem::getErrorDescription(long errorNumber) const
    {
        return BLANKSTRING;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setWorldMatrix(const Matrix4 &m)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setViewMatrix(const Matrix4 &m)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setSurfaceParams( const ColourValue &ambient,
                            const ColourValue &diffuse, const ColourValue &specular,
                            const ColourValue &emissive, Real shininess,
                            TrackVertexColourType tracking )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setPointSpritesEnabled(bool enabled)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setPointParameters(Real size, bool attenuationEnabled,
        Real constant, Real linear, Real quadratic, Real minSize, Real maxSize)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::queueBindUAV( uint32 slot, TexturePtr texture,
                                         ResourceAccess::ResourceAccess access,
                                         int32 mipmapLevel, int32 textureArrayIndex,
                                         PixelFormat pixelFormat )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::queueBindUAV( uint32 slot, UavBufferPacked *buffer,
                                         ResourceAccess::ResourceAccess access,
                                         size_t offset, size_t sizeBytes )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::clearUAVs(void)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::flushUAVs(void)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_bindTextureUavCS( uint32 slot, Texture *texture,
                                              ResourceAccess::ResourceAccess access,
                                              int32 mipmapLevel, int32 textureArrayIndex,
                                              PixelFormat pixelFormat )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTextureCS( uint32 slot, bool enabled, Texture *texPtr )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setHlmsSamplerblockCS( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTexture(size_t unit, bool enabled,  Texture *texPtr)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTextureCoordSet(size_t unit, size_t index)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTextureCoordCalculation( size_t unit, TexCoordCalcMethod m,
                                                        const Frustum* frustum )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTextureBlendMode(size_t unit, const LayerBlendModeEx& bm)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTextureMatrix(size_t unit, const Matrix4& xform)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setIndirectBuffer( IndirectBufferPacked *indirectBuffer )
    {
    }
    //-------------------------------------------------------------------------
    DepthBuffer* MetalRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget,
                                                          bool exactMatchFormat )
    {
        return 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_beginFrame(void)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_endFrame(void)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setViewport(Viewport *vp)
    {
    }
    //-------------------------------------------------------------------------
    MTLRenderPassColorAttachmentDescriptor* MetalRenderSystem::getRenderPass( RenderTarget *rtt )
    {
        RenderPassAttachmentsByRttMap::const_iterator itor = mRenderPassAttachmentsMap.find( rtt );
        assert( itor != mRenderPassAttachmentsMap.end() );
        assert( dynamic_cast<MTLRenderPassColorAttachmentDescriptor*>( itor->second ) );
        return static_cast<MTLRenderPassColorAttachmentDescriptor*>( itor->second );
    }
    //-------------------------------------------------------------------------
    MTLRenderPassDepthAttachmentDescriptor* MetalRenderSystem::getDepthRenderPass(
            DepthBuffer *depthBuffer )
    {
        RenderPassAttachmentsByRttMap::const_iterator itor =
                mRenderPassAttachmentsMap.find( depthBuffer );
        assert( itor != mRenderPassAttachmentsMap.end() );
        assert( dynamic_cast<MTLRenderPassDepthAttachmentDescriptor*>( itor->second ) );
        return static_cast<MTLRenderPassDepthAttachmentDescriptor*>( itor->second );
    }
    //-------------------------------------------------------------------------
    MTLRenderPassStencilAttachmentDescriptor* MetalRenderSystem::getStencilRenderPass(
            DepthBuffer *depthBuffer )
    {
        RenderPassAttachmentsByRttMap::const_iterator itor =
                mRenderPassAttachmentsMap.find( depthBuffer );
        assert( itor != mRenderPassAttachmentsMap.end() );
        assert( dynamic_cast<MTLRenderPassStencilAttachmentDescriptor*>( itor->second ) );
        return static_cast<MTLRenderPassStencilAttachmentDescriptor*>( itor->second );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::createRenderEncoder(void)
    {
        if( mRenderEncoder )
        {
            [mRenderEncoder endEncoding];
            mRenderEncoder = 0;
        }

        //TODO: When a DepthBuffer or RenderTarget are destroyed, we need to remove them from
        //mRenderPassAttachmentsMap

        //TODO: With a couple modifications, Compositor should be able to tell us
        //if we can use MTLStoreActionDontCare.

        MTLRenderPassDescriptor *passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        for( uint8 i=0; i<mNumMRTs; ++i )
        {
            MTLRenderPassColorAttachmentDescriptor *desc = getRenderPass( mCurrentColourRTs[i] );
            passDesc.colorAttachments[i] = [desc copy];
            //Next time it will be used it will have to be loaded
            //unless we're later told to clear or discard.
            desc.loadAction = MTLLoadActionLoad;
        }
        if( mCurrentDepthBuffer )
        {
            MTLRenderPassDepthAttachmentDescriptor *descDepth =
                    getDepthRenderPass( mCurrentDepthBuffer );
            passDesc.depthAttachment = [descDepth copy];
            descDepth.loadAction = MTLLoadActionLoad;

            MTLRenderPassStencilAttachmentDescriptor *descStencil =
                    getStencilRenderPass( mCurrentDepthBuffer );
            passDesc.stencilAttachment = [descStencil copy];
            descStencil.loadAction = MTLLoadActionLoad;
        }

        //id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        id<MTLCommandBuffer> commandBuffer = 0; //TODO
        mRenderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    }
    //-------------------------------------------------------------------------
    id <MTLDepthStencilState> MetalRenderSystem::getDepthStencilState( HlmsPso *pso )
    {
        CachedDepthStencilState depthState;
        if( pso->macroblock->mDepthCheck )
        {
            depthState.depthFunc    = pso->macroblock->mDepthFunc;
            depthState.depthWrite   = pso->macroblock->mDepthWrite;
        }
        else
        {
            depthState.depthFunc    = CMPF_ALWAYS_PASS;
            depthState.depthWrite   = false;
        }

        depthState.stencilParams = pso->pass.stencilParams;

        CachedDepthStencilStateVec::iterator itor = std::lower_bound( mDepthStencilStates.begin(),
                                                                      mDepthStencilStates.end(),
                                                                      depthState );

        if( itor == mDepthStencilStates.end() || depthState != *itor )
        {
            //Not cached. Add the entry
            MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
            depthStateDesc.depthCompareFunction = MetalMappings::get( depthState.depthFunc );
            depthStateDesc.depthWriteEnabled    = depthState.depthWrite;

            //TODO: Convert stencil params
            if( pso->pass.stencilParams.enabled )
            {
//                pso->pass.stencilParams.readMask;
//                pso->pass.stencilParams.writeMask;
//                depthStateDesc.frontFaceStencil =;
//                depthStateDesc.backFaceStencil =;
            }

            depthState.depthStencilState = [mDevice newDepthStencilStateWithDescriptor:depthStateDesc];

            itor = mDepthStencilStates.insert( itor, depthState );
        }

        ++itor->refCount;
        return itor->depthStencilState;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::removeDepthStencilState( HlmsPso *pso )
    {
        CachedDepthStencilState depthState;
        if( pso->macroblock->mDepthCheck )
        {
            depthState.depthFunc    = pso->macroblock->mDepthFunc;
            depthState.depthWrite   = pso->macroblock->mDepthWrite;
        }
        else
        {
            depthState.depthFunc    = CMPF_ALWAYS_PASS;
            depthState.depthWrite   = false;
        }

        depthState.stencilParams = pso->pass.stencilParams;

        CachedDepthStencilStateVec::iterator itor = std::lower_bound( mDepthStencilStates.begin(),
                                                                      mDepthStencilStates.end(),
                                                                      depthState );

        if( itor == mDepthStencilStates.end() || !(depthState != *itor) )
        {
            --itor->refCount;
            if( !itor->refCount )
            {
                mDepthStencilStates.erase( itor );
            }
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_hlmsPipelineStateObjectCreated( HlmsPso *newPso )
    {
        MTLRenderPipelineDescriptor *psd = [[MTLRenderPipelineDescriptor alloc] init];
        [psd setSampleCount: newPso->pass.multisampleCount];
//		[psd setVertexFunction:newPso->vertexShader->vsShader];
//		[psd setFragmentFunction:newPso->pixelShader->psShader];

        if( !newPso->vertexElements.empty() )
        {
            size_t elementIdx = 0;
            MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];

            VertexElement2VecVec::const_iterator itor = newPso->vertexElements.begin();
            VertexElement2VecVec::const_iterator end  = newPso->vertexElements.end();

            while( itor != end )
            {
                size_t accumOffset = 0;
                const size_t bufferIdx = itor - newPso->vertexElements.begin();
                VertexElement2Vec::const_iterator it = itor->begin();
                VertexElement2Vec::const_iterator en = itor->end();

                while( it != en )
                {
                    vertexDescriptor.attributes[elementIdx].format = MetalMappings::get( it->mType );
                    vertexDescriptor.attributes[elementIdx].bufferIndex = bufferIdx;
                    vertexDescriptor.attributes[elementIdx].offset = accumOffset;

                    accumOffset += v1::VertexElement::getTypeSize( it->mType );

                    ++elementIdx;
                    ++it;
                }

                vertexDescriptor.layouts[bufferIdx].stride = accumOffset;
                vertexDescriptor.layouts[bufferIdx].stepFunction = MTLVertexStepFunctionPerVertex;

                ++itor;
            }

            [psd setVertexDescriptor:vertexDescriptor];
        }

        //TODO: Ogre should track the number of RTTs instead of assuming
        //OGRE_MAX_MULTIPLE_RENDER_TARGETS (will NOT work on Metal!)
        for( int i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            HlmsBlendblock const *blendblock = newPso->blendblock;
            psd.colorAttachments[i].pixelFormat = MetalMappings::getPixelFormat(
                        newPso->pass.colourFormat[i], newPso->pass.hwGamma[i] );

            if( blendblock->mBlendOperation == SBO_ADD &&
                blendblock->mSourceBlendFactor == SBF_ONE &&
                blendblock->mDestBlendFactor == SBF_ZERO &&
                (!blendblock->mSeparateBlend ||
                 (blendblock->mBlendOperation == blendblock->mBlendOperationAlpha &&
                 blendblock->mSourceBlendFactor == blendblock->mSourceBlendFactorAlpha &&
                 blendblock->mDestBlendFactor == blendblock->mDestBlendFactorAlpha)) )
            {
                //Note: Can NOT use blendblock->mIsTransparent. What Ogre understand
                //as transparent differs from what Metal understands.
                psd.colorAttachments[i].blendingEnabled = NO;
            }
            else
            {
                psd.colorAttachments[i].blendingEnabled = YES;
            }
            psd.colorAttachments[i].rgbBlendOperation           = MetalMappings::get( blendblock->mBlendOperation );
            psd.colorAttachments[i].alphaBlendOperation         = MetalMappings::get( blendblock->mBlendOperationAlpha );
            psd.colorAttachments[i].sourceRGBBlendFactor        = MetalMappings::get( blendblock->mSourceBlendFactor );
            psd.colorAttachments[i].destinationRGBBlendFactor   = MetalMappings::get( blendblock->mDestBlendFactor );
            psd.colorAttachments[i].sourceAlphaBlendFactor      = MetalMappings::get( blendblock->mSourceBlendFactorAlpha );
            psd.colorAttachments[i].destinationAlphaBlendFactor = MetalMappings::get( blendblock->mDestBlendFactorAlpha );

            psd.colorAttachments[i].writeMask = MetalMappings::get( blendblock->mBlendChannelMask );
        }

        psd.depthAttachmentPixelFormat = MetalMappings::getPixelFormat( newPso->pass.depthFormat, false );

        NSError* error = NULL;
        id <MTLRenderPipelineState> pso =
                [mDevice newRenderPipelineStateWithDescriptor:psd error:&error];

        //TODO: Ogre should throw.
        if( !pso ) {
            NSLog(@"Failed to created pipeline state, error %@", error);
        }

        id <MTLDepthStencilState> depthStencilState = getDepthStencilState( newPso );

        MetalHlmsPso *metalPso = new MetalHlmsPso();
        metalPso->pso = pso;
        metalPso->depthStencilState = depthStencilState;
        
        newPso->rsData = metalPso;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_hlmsPipelineStateObjectDestroyed( HlmsPso *pso )
    {
        assert( pso->rsData );

        removeDepthStencilState( pso );

        MetalHlmsPso *metalPso = reinterpret_cast<MetalHlmsPso*>(pso->rsData);
        delete metalPso;
        pso->rsData = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_hlmsSamplerblockCreated( HlmsSamplerblock *newBlock )
    {
        MTLSamplerDescriptor *samplerDescriptor = [MTLSamplerDescriptor new];
        samplerDescriptor.minFilter = MetalMappings::get( newBlock->mMinFilter );
        samplerDescriptor.magFilter = MetalMappings::get( newBlock->mMagFilter );
        samplerDescriptor.mipFilter = MetalMappings::getMipFilter( newBlock->mMipFilter );
        samplerDescriptor.maxAnisotropy = newBlock->mMaxAnisotropy;
        samplerDescriptor.sAddressMode  = MetalMappings::get( newBlock->mU );
        samplerDescriptor.tAddressMode  = MetalMappings::get( newBlock->mV );
        samplerDescriptor.rAddressMode  = MetalMappings::get( newBlock->mW );
        samplerDescriptor.normalizedCoordinates = YES;
        samplerDescriptor.lodMinClamp   = newBlock->mMinLod;
        samplerDescriptor.lodMaxClamp   = newBlock->mMaxLod;

        if( newBlock->mCompareFunction != NUM_COMPARE_FUNCTIONS )
            samplerDescriptor.compareFunction = MetalMappings::get( newBlock->mCompareFunction );

        id <MTLSamplerState> sampler = [mDevice newSamplerStateWithDescriptor:samplerDescriptor];

        newBlock->mRsData = const_cast<void*>( CFBridgingRetain( sampler ) );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_hlmsSamplerblockDestroyed( HlmsSamplerblock *block )
    {
        id <MTLSamplerState> sampler = reinterpret_cast< id <MTLSamplerState> >(
                    CFBridgingRelease( block->mRsData ) );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setHlmsSamplerblock( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
        assert( (!samplerblock || samplerblock->mRsData) &&
                "The block must have been created via HlmsManager::getSamplerblock!" );

        if( !samplerblock )
        {
            [mRenderEncoder setFragmentSamplerState:0 atIndex: texUnit];
        }
        else
        {
            id <MTLSamplerState> sampler =
                    reinterpret_cast< id <MTLSamplerState> >( samplerblock->mRsData );
            [mRenderEncoder setFragmentSamplerState:sampler atIndex: texUnit];
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setPipelineStateObject( const HlmsPso *pso )
    {
        MetalHlmsPso *metalPso = reinterpret_cast<MetalHlmsPso*>(pso->rsData);
        
        if( !mPso || mPso->depthStencilState != metalPso->depthStencilState )
            [mRenderEncoder setDepthStencilState:metalPso->depthStencilState];
        
        [mRenderEncoder setDepthBias:pso->macroblock->mDepthBiasConstant
                                     slopeScale:pso->macroblock->mDepthBiasSlopeScale
                                     clamp:0.0f];
        // Ogre CullMode starts with 1 and Metals starts with 0
        [mRenderEncoder setCullMode:(MTLCullMode)((int)pso->macroblock->mCullMode - 1)];
        
        if( mPso != metalPso )
        {
            [mRenderEncoder setRenderPipelineState:metalPso->pso];
            mPso = metalPso;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setComputePso( const HlmsComputePso *pso )
    {
    }
    //-------------------------------------------------------------------------
    VertexElementType MetalRenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ARGB;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_dispatch( const HlmsComputePso &pso )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setVertexArrayObject( const VertexArrayObject *vao )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const CbDrawCallIndexed *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const CbDrawCallStrip *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_renderEmulated( const CbDrawCallIndexed *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_renderEmulated( const CbDrawCallStrip *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setRenderOperation( const v1::CbRenderOp *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const v1::CbDrawCallIndexed *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const v1::CbDrawCallStrip *cmd )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::bindGpuProgramParameters(GpuProgramType gptype,
        GpuProgramParametersSharedPtr params, uint16 variabilityMask)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::clearFrameBuffer( unsigned int buffers, const ColourValue& colour,
                                             Real depth, unsigned short stencil )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::discardFrameBuffer( unsigned int buffers )
    {
    }
    //-------------------------------------------------------------------------
    Real MetalRenderSystem::getHorizontalTexelOffset(void)
    {
        return 0.0f;
    }
    //-------------------------------------------------------------------------
    Real MetalRenderSystem::getVerticalTexelOffset(void)
    {
        return 0.0f;
    }
    //-------------------------------------------------------------------------
    Real MetalRenderSystem::getMinimumDepthInputValue(void)
    {
        return 0.0f;
    }
    //-------------------------------------------------------------------------
    Real MetalRenderSystem::getMaximumDepthInputValue(void)
    {
        return 1.0f;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setRenderTarget(RenderTarget *target, bool colourWrite)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::preExtraThreadsStarted()
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::postExtraThreadsStarted()
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::registerThread()
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::unregisterThread()
    {
    }
    //-------------------------------------------------------------------------
    const PixelFormatToShaderType* MetalRenderSystem::getPixelFormatToShaderType(void) const
    {
        return &mPixelFormatToShaderType;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::beginProfileEvent( const String &eventName )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::endProfileEvent( void )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::markProfileEvent( const String &event )
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
    }
}
