/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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

#include "OgreMetalRenderSystem.h"
#include "OgreMetalRenderWindow.h"
#include "OgreMetalTextureManager.h"
#include "Vao/OgreMetalVaoManager.h"
#include "Vao/OgreMetalBufferInterface.h"
#include "OgreMetalHlmsPso.h"
#include "OgreMetalRenderTargetCommon.h"
#include "OgreMetalDepthBuffer.h"
#include "OgreMetalDevice.h"
#include "OgreMetalGpuProgramManager.h"
#include "OgreMetalProgram.h"
#include "OgreMetalProgramFactory.h"

#include "OgreMetalHardwareBufferManager.h"
#include "OgreMetalHardwareIndexBuffer.h"
#include "OgreMetalHardwareVertexBuffer.h"

#include "Vao/OgreIndirectBufferPacked.h"
#include "Vao/OgreVertexArrayObject.h"
#include "CommandBuffer/OgreCbDrawCall.h"

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
        mShaderManager( 0 ),
        mMetalProgramFactory( 0 ),
        mIndirectBuffer( 0 ),
        mSwIndirectBufferPtr( 0 ),
        mPso( 0 ),
        mCurrentIndexBuffer( 0 ),
        mCurrentPrimType( MTLPrimitiveTypePoint ),
        mNumMRTs( 0 ),
        mCurrentDepthBuffer( 0 ),
        mActiveDevice( 0 ),
        mActiveRenderEncoder( 0 ),
        mDevice( this ),
        mMainGpuSyncSemaphore( 0 )
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

        if( mMetalProgramFactory )
        {
            // Remove from manager safely
            if( HighLevelGpuProgramManager::getSingletonPtr() )
                HighLevelGpuProgramManager::getSingleton().removeFactory( mMetalProgramFactory );
            OGRE_DELETE mMetalProgramFactory;
            mMetalProgramFactory = 0;
        }

        OGRE_DELETE mShaderManager;
        mShaderManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;
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

        rsc->addShaderProfile( "metal" );

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
        if( !mInitialized )
        {
            mDevice.init();
            setActiveDevice(&mDevice);
            
            const long c_inFlightCommandBuffers = 3;
            mMainGpuSyncSemaphore = dispatch_semaphore_create(c_inFlightCommandBuffers);
            mRealCapabilities = createRenderSystemCapabilities();

            if (!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities( mCurrentCapabilities, 0 );

            mTextureManager = new MetalTextureManager();
            mVaoManager = OGRE_NEW MetalVaoManager( c_inFlightCommandBuffers, &mDevice );
            mHardwareBufferManager = new v1::MetalHardwareBufferManager( &mDevice, mVaoManager );

            mInitialized = true;
        }

        RenderWindow *win = OGRE_NEW MetalRenderWindow( &mDevice, this );
        win->create( name, width, height, fullScreen, miscParams );
        return win;
    }
    //-------------------------------------------------------------------------
    MultiRenderTarget* MetalRenderSystem::createMultiRenderTarget(const String & name)
    {
        return 0;
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
        if( mVaoManager->supportsIndirectBuffers() )
        {
            if( indirectBuffer )
            {
                MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                                                            indirectBuffer->getBufferInterface() );
                mIndirectBuffer = bufferInterface->getVboName();
            }
            else
            {
                mIndirectBuffer = 0;
            }
        }
        else
        {
            if( indirectBuffer )
                mSwIndirectBufferPtr = indirectBuffer->getSwBufferPtr();
            else
                mSwIndirectBufferPtr = 0;
        }
    }
    //-------------------------------------------------------------------------
    DepthBuffer* MetalRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget,
                                                          bool exactMatchFormat )
    {
        return 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_beginFrameOnce(void)
    {
        //Allow the renderer to preflight 3 frames on the CPU (using a semapore as a guard) and
        //commit them to the GPU. This semaphore will get signaled once the GPU completes a
        //frame's work via addCompletedHandler callback below, signifying the CPU can go ahead
        //and prepare another frame.
        dispatch_semaphore_wait( mMainGpuSyncSemaphore, DISPATCH_TIME_FOREVER );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_endFrameOnce(void)
    {
        //TODO: We shouldn't tidy up JUST the active device. But all of them.
        __block dispatch_semaphore_t blockSemaphore = mMainGpuSyncSemaphore;
        [mActiveDevice->mCurrentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
        {
            // GPU has completed rendering the frame and is done using the contents of any buffers
            // previously encoded on the CPU for that frame. Signal the semaphore and allow the CPU
            // to proceed and construct the next frame.
            dispatch_semaphore_signal( blockSemaphore );
        }];

        mActiveDevice->commitAndNextCommandBuffer();
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
    void MetalRenderSystem::setActiveDevice( MetalDevice *device )
    {
        if( mActiveDevice != device )
        {
            mActiveDevice           = device;
            mActiveRenderEncoder    = device->mRenderEncoder;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::createRenderEncoder(void)
    {
        assert( mActiveDevice );

        mActiveDevice->endAllEncoders();
        mActiveRenderEncoder = 0;

        //TODO: With a couple modifications, Compositor should be able to tell us
        //if we can use MTLStoreActionDontCare.

        MTLRenderPassDescriptor *passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        for( uint8 i=0; i<mNumMRTs; ++i )
        {
            passDesc.colorAttachments[i] = [mCurrentColourRTs[i]->mColourAttachmentDesc copy];
            //Next time it will be used it will have to be loaded
            //unless we're later told to clear or discard.
            mCurrentColourRTs[i]->mColourAttachmentDesc.loadAction = MTLLoadActionLoad;
        }
        if( mCurrentDepthBuffer )
        {
            MTLRenderPassDepthAttachmentDescriptor *descDepth =
                    mCurrentDepthBuffer->mDepthAttachmentDesc;
            if( descDepth )
            {
                passDesc.depthAttachment = [descDepth copy];
                descDepth.loadAction = MTLLoadActionLoad;
            }

            MTLRenderPassStencilAttachmentDescriptor *descStencil =
                    mCurrentDepthBuffer->mStencilAttachmentDesc;
            if( descStencil )
            {
                passDesc.stencilAttachment = [descStencil copy];
                descStencil.loadAction = MTLLoadActionLoad;
            }
        }

        mActiveDevice->mRenderEncoder =
                [mActiveDevice->mCurrentCommandBuffer renderCommandEncoderWithDescriptor:passDesc];
        mActiveRenderEncoder = mActiveDevice->mRenderEncoder;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_notifyActiveEncoderEnded(void)
    {
        mActiveRenderEncoder = 0;
        mPso = 0;
    }
	//-------------------------------------------------------------------------
	void MetalRenderSystem::_notifyDeviceStalled(void)
	{
        v1::MetalHardwareBufferManager *hwBufferMgr = static_cast<v1::MetalHardwareBufferManager*>(
                    mHardwareBufferManager );
        MetalVaoManager *vaoManager = static_cast<MetalVaoManager*>( mVaoManager );

        hwBufferMgr->_notifyDeviceStalled();
        vaoManager->_notifyDeviceStalled();
	}
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_clearRenderTargetImmediately( RenderTarget *renderTarget )
    {
        _setRenderTarget( renderTarget, true );
        createRenderEncoder();
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

            depthState.depthStencilState =
                    [mActiveDevice->mDevice newDepthStencilStateWithDescriptor:depthStateDesc];

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

//        if( !newPso->vertexShader.isNull() )
//        {
//            MetalProgram *shader = static_cast<MetalProgram*>( newPso->vertexShader->
//                                                               _getBindingDelegate() );
//            [psd setVertexFunction:shader->getMetalFunction()];
//        }

//        if( !newPso->pixelShader.isNull() )
//        {
//            MetalProgram *shader = static_cast<MetalProgram*>( newPso->pixelShader->
//                                                               _getBindingDelegate() );
//            [psd setFragmentFunction:shader->getMetalFunction()];
//        }
        //Ducttape shaders
        id<MTLLibrary> library = [mActiveDevice->mDevice newDefaultLibrary];
        [psd setVertexFunction:[library newFunctionWithName:@"vertex_vs"]];
        [psd setFragmentFunction:[library newFunctionWithName:@"pixel_ps"]];

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
                [mActiveDevice->mDevice newRenderPipelineStateWithDescriptor:psd error:&error];

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

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        const bool supportsCompareFunction =
                [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
#else
        const bool supportsCompareFunction = true;
#endif

        if( supportsCompareFunction )
        {
            if( newBlock->mCompareFunction != NUM_COMPARE_FUNCTIONS )
                samplerDescriptor.compareFunction = MetalMappings::get( newBlock->mCompareFunction );
        }

        id <MTLSamplerState> sampler =
                [mActiveDevice->mDevice newSamplerStateWithDescriptor:samplerDescriptor];

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
            [mActiveRenderEncoder setFragmentSamplerState:0 atIndex: texUnit];
        }
        else
        {
            id <MTLSamplerState> sampler = (__bridge id<MTLSamplerState>)samplerblock->mRsData;
            [mActiveRenderEncoder setFragmentSamplerState:sampler atIndex: texUnit];
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setPipelineStateObject( const HlmsPso *pso )
    {
        MetalHlmsPso *metalPso = reinterpret_cast<MetalHlmsPso*>(pso->rsData);

        if( pso && !mActiveRenderEncoder )
            createRenderEncoder();
        
        if( !mPso || mPso->depthStencilState != metalPso->depthStencilState )
            [mActiveRenderEncoder setDepthStencilState:metalPso->depthStencilState];
        
        [mActiveRenderEncoder setDepthBias:pso->macroblock->mDepthBiasConstant
                                     slopeScale:pso->macroblock->mDepthBiasSlopeScale
                                     clamp:0.0f];
        // Ogre CullMode starts with 1 and Metals starts with 0
        [mActiveRenderEncoder setCullMode:(MTLCullMode)((int)pso->macroblock->mCullMode - 1)];
        
        if( mPso != metalPso )
        {
            [mActiveRenderEncoder setRenderPipelineState:metalPso->pso];
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
        __unsafe_unretained id<MTLBuffer> metalVertexBuffers[16];
        NSUInteger offsets[16];
        memset( offsets, 0, sizeof(offsets) );

        const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();

        size_t numVertexBuffers = 0;
        VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
        VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

        while( itor != end )
        {
            MetalBufferInterface *bufferInterface = static_cast<MetalBufferInterface*>(
                        (*itor)->getBufferInterface() );
            metalVertexBuffers[numVertexBuffers++] = bufferInterface->getVboName();
            ++itor;
        }

#if OGRE_DEBUG_MODE
        assert( numVertexBuffers < 16u );
#endif

        [mActiveRenderEncoder setVertexBuffers:metalVertexBuffers offsets:offsets
                                               withRange:NSMakeRange( 0, numVertexBuffers )];
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
        const VertexArrayObject *vao = cmd->vao;
        CbDrawIndexed *drawCmd = reinterpret_cast<CbDrawIndexed*>(
                                    mSwIndirectBufferPtr + (size_t)cmd->indirectBufferOffset );

        const MTLIndexType indexType = static_cast<MTLIndexType>( vao->mIndexBuffer->getIndexType() );
        const MTLPrimitiveType primType =  std::min(
                    MTLPrimitiveTypeTriangleStrip,
                    static_cast<MTLPrimitiveType>( vao->getOperationType() - 1u ) );

        //Calculate bytesPerVertexBuffer & numVertexBuffers which is the same for all draws in this cmd
        uint32 bytesPerVertexBuffer[16];
        size_t numVertexBuffers = 0;
        const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
        VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
        VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

        while( itor != end )
        {
            bytesPerVertexBuffer[numVertexBuffers] = (*itor)->getBytesPerElement();
            ++numVertexBuffers;
            ++itor;
        }

        //Get index buffer stuff which is the same for all draws in this cmd
        const size_t bytesPerIndexElement = vao->mIndexBuffer->getBytesPerElement();

        MetalBufferInterface *indexBufferInterface = static_cast<MetalBufferInterface*>(
                    vao->mIndexBuffer->getBufferInterface() );
        __unsafe_unretained id<MTLBuffer> indexBuffer = indexBufferInterface->getVboName();

        for( uint32 i=cmd->numDraws; i--; )
        {
            for( size_t j=0; j<numVertexBuffers; ++j )
            {
                //Manually set vertex buffer offsets since in iOS baseVertex is not supported
                //until iOS GPU Family 3 v1 & OS X (we just use indirect rendering there).
                [mActiveRenderEncoder setVertexBufferOffset:drawCmd->baseVertex * bytesPerVertexBuffer[j]
                                                            atIndex:j];
            }

            //TODO: Setup baseInstance.

#if OGRE_DEBUG_MODE
            assert( ((drawCmd->firstVertexIndex * bytesPerIndexElement) & 0x04) == 0
                    && "Index Buffer must be aligned to 4 bytes. If you're messing with "
                    "VertexArrayObject::setPrimitiveRange, you've entered an invalid "
                    "primStart; not supported by the Metal API." );
#endif

            [mActiveRenderEncoder drawIndexedPrimitives:primType
                       indexCount:drawCmd->primCount
                        indexType:indexType
                      indexBuffer:indexBuffer
                indexBufferOffset:drawCmd->firstVertexIndex * bytesPerIndexElement
                    instanceCount:drawCmd->instanceCount];
            ++drawCmd;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_renderEmulated( const CbDrawCallStrip *cmd )
    {
        const VertexArrayObject *vao = cmd->vao;
        CbDrawStrip *drawCmd = reinterpret_cast<CbDrawStrip*>(
                                    mSwIndirectBufferPtr + (size_t)cmd->indirectBufferOffset );

        const MTLPrimitiveType primType =  std::min(
                    MTLPrimitiveTypeTriangleStrip,
                    static_cast<MTLPrimitiveType>( vao->getOperationType() - 1u ) );

//        const size_t numVertexBuffers = vertexBuffers.size();
//        for( size_t j=0; j<numVertexBuffers; ++j )
//        {
//            //baseVertex is not needed as vertexStart does the same job.
//            [mActiveRenderEncoder setVertexBufferOffset:0 atIndex:j];
//        }

        for( uint32 i=cmd->numDraws; i--; )
        {
            //TODO: Setup baseInstance.
            [mActiveRenderEncoder drawPrimitives:primType
                      vertexStart:drawCmd->firstVertexIndex
                      vertexCount:drawCmd->primCount
                    instanceCount:drawCmd->instanceCount];
            ++drawCmd;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setRenderOperation( const v1::CbRenderOp *cmd )
    {
        __unsafe_unretained id<MTLBuffer> metalVertexBuffers[16];
        NSUInteger offsets[16];
        memset( offsets, 0, sizeof(offsets) );

        size_t maxUsedSlot = 0;
        const v1::VertexBufferBinding::VertexBufferBindingMap& binds =
                cmd->vertexData->vertexBufferBinding->getBindings();
        v1::VertexBufferBinding::VertexBufferBindingMap::const_iterator itor = binds.begin();
        v1::VertexBufferBinding::VertexBufferBindingMap::const_iterator end  = binds.end();

        while( itor != end )
        {
            v1::MetalHardwareVertexBuffer *metalBuffer =
                static_cast<v1::MetalHardwareVertexBuffer*>( itor->second.get() );

            const size_t slot = itor->first;
#if OGRE_DEBUG_MODE
            assert( slot < 16u );
#endif
            size_t offsetStart;
            metalVertexBuffers[slot] = metalBuffer->getBufferName( offsetStart );
            offsets[slot]            = cmd->vertexData->vertexStart * metalBuffer->getVertexSize() +
                                                                                        offsetStart;

            ++itor;
            maxUsedSlot = std::max( maxUsedSlot, slot + 1u );
        }

        [mActiveRenderEncoder setVertexBuffers:metalVertexBuffers offsets:offsets
                                               withRange:NSMakeRange( 0, maxUsedSlot )];

        mCurrentIndexBuffer = cmd->indexData;
        mCurrentPrimType    = std::min(  MTLPrimitiveTypeTriangleStrip,
                                         static_cast<MTLPrimitiveType>( cmd->operationType - 1u ) );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const v1::CbDrawCallIndexed *cmd )
    {
        const MTLIndexType indexType = static_cast<MTLIndexType>(
                    mCurrentIndexBuffer->indexBuffer->getType() );

        //Get index buffer stuff which is the same for all draws in this cmd
        const size_t bytesPerIndexElement = mCurrentIndexBuffer->indexBuffer->getIndexSize();

        size_t offsetStart;
        v1::MetalHardwareIndexBuffer *metalBuffer =
            static_cast<v1::MetalHardwareIndexBuffer*>( mCurrentIndexBuffer->indexBuffer.get() );
        __unsafe_unretained id<MTLBuffer> indexBuffer = metalBuffer->getBufferName( offsetStart );

        //TODO: Setup baseInstance.

#if OGRE_DEBUG_MODE
        assert( ((cmd->firstVertexIndex * bytesPerIndexElement) & 0x04) == 0
                && "Index Buffer must be aligned to 4 bytes. If you're messing with "
                "IndexBuffer::indexStart, you've entered an invalid "
                "indexStart; not supported by the Metal API." );
#endif

        [mActiveRenderEncoder drawIndexedPrimitives:mCurrentPrimType
                   indexCount:cmd->primCount
                    indexType:indexType
                  indexBuffer:indexBuffer
            indexBufferOffset:cmd->firstVertexIndex * bytesPerIndexElement + offsetStart
            instanceCount:cmd->instanceCount];
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const v1::CbDrawCallStrip *cmd )
    {
        //TODO: Setup baseInstance.
        [mActiveRenderEncoder drawPrimitives:mCurrentPrimType
                  vertexStart:0 /*cmd->firstVertexIndex already handled in _setRenderOperation*/
                  vertexCount:cmd->primCount
                instanceCount:cmd->instanceCount];
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
        if( buffers & FBT_COLOUR )
        {
            for( size_t i=0; i<mNumMRTs; ++i )
            {
                if( mCurrentColourRTs[i] )
                {
                    mCurrentColourRTs[i]->mColourAttachmentDesc.loadAction = MTLLoadActionClear;
                    mCurrentColourRTs[i]->mColourAttachmentDesc.clearColor =
                            MTLClearColorMake( colour.r, colour.g, colour.b, colour.a );
                }
            }
        }

        if( mCurrentDepthBuffer )
        {
            if( buffers & FBT_DEPTH && mCurrentDepthBuffer->mDepthAttachmentDesc )
            {
                mCurrentDepthBuffer->mDepthAttachmentDesc.loadAction = MTLLoadActionClear;
                mCurrentDepthBuffer->mDepthAttachmentDesc.clearDepth = depth;
            }

            if( buffers & FBT_STENCIL && mCurrentDepthBuffer->mStencilAttachmentDesc )
            {
                mCurrentDepthBuffer->mStencilAttachmentDesc.loadAction = MTLLoadActionClear;
                mCurrentDepthBuffer->mStencilAttachmentDesc.clearStencil = stencil;
            }
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::discardFrameBuffer( unsigned int buffers )
    {
        if( buffers & FBT_COLOUR )
        {
            for( size_t i=0; i<mNumMRTs; ++i )
            {
                if( mCurrentColourRTs[i] )
                    mCurrentColourRTs[i]->mColourAttachmentDesc.loadAction = MTLLoadActionDontCare;
            }
        }

        if( mCurrentDepthBuffer )
        {
            if( buffers & FBT_DEPTH && mCurrentDepthBuffer->mDepthAttachmentDesc )
                mCurrentDepthBuffer->mDepthAttachmentDesc.loadAction = MTLLoadActionDontCare;

            if( buffers & FBT_STENCIL && mCurrentDepthBuffer->mStencilAttachmentDesc )
                mCurrentDepthBuffer->mStencilAttachmentDesc.loadAction = MTLLoadActionDontCare;
        }
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
        if( target )
        {
            colourWrite &= !target->getForceDisableColourWrites();

            //We need to set mCurrentColourRTs[0] to grab the active device,
            //even if we won't be drawing to colour target.
            target->getCustomAttribute( "MetalRenderTargetCommon", &mCurrentColourRTs[0] );

            if( colourWrite )
            {
                //TODO: Deal with MRT.
                mNumMRTs = 1;
                MTLRenderPassColorAttachmentDescriptor *desc =
                        mCurrentColourRTs[0]->mColourAttachmentDesc;

                //TODO. This information is stored in Texture. Metal needs it now.
                const bool explicitResolve = false;

                //TODO: Compositor should be able to tell us whether to use
                //MTLStoreActionDontCare with some future enhancements.
                if( target->getFSAA() > 1 && !explicitResolve )
                {
                    desc.storeAction = MTLStoreActionMultisampleResolve;
                }
                else
                {
                    desc.storeAction = MTLStoreActionStore;
                }
            }
            else
            {
                mNumMRTs = 0;
            }

            MetalDepthBuffer *depthBuffer = static_cast<MetalDepthBuffer*>( target->getDepthBuffer() );
            mCurrentDepthBuffer = depthBuffer;
            if( depthBuffer )
            {
                if( depthBuffer->mDepthAttachmentDesc )
                    depthBuffer->mDepthAttachmentDesc.storeAction = MTLStoreActionStore;
                if( depthBuffer->mStencilAttachmentDesc )
                    depthBuffer->mStencilAttachmentDesc.storeAction = MTLStoreActionStore;
            }

            setActiveDevice( mCurrentColourRTs[0]->getOwnerDevice() );
        }
        else
        {
            mNumMRTs = 0;
            mCurrentDepthBuffer = 0;
        }
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
        mShaderManager = OGRE_NEW MetalGpuProgramManager( &mDevice );
        mMetalProgramFactory = new MetalProgramFactory( &mDevice );
        HighLevelGpuProgramManager::getSingleton().addFactory( mMetalProgramFactory );
    }
}
