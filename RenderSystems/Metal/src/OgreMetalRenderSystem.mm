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
#include "OgreMetalRenderTargetCommon.h"
#include "OgreMetalDepthBuffer.h"
#include "OgreMetalDevice.h"
#include "OgreGpuProgramManager.h"
#include "OgreMetalProgram.h"
#include "OgreMetalProgramFactory.h"
#include "OgreMetalTexture.h"
#include "OgreMetalMultiRenderTarget.h"

#include "OgreMetalHardwareBufferManager.h"
#include "OgreMetalHardwareBufferCommon.h"

#include "OgreViewport.h"

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
        mMetalProgramFactory( 0 ),
        mCurrentIndexBuffer( 0 ),
        mCurrentVertexBuffer( 0 ),
        mCurrentPrimType( MTLPrimitiveTypePoint ),
        mNumMRTs( 0 ),
        mCurrentDepthBuffer( 0 ),
        mActiveDevice( 0 ),
        mActiveRenderEncoder( 0 ),
        mDevice( this ),
        mMainGpuSyncSemaphore( 0 ),
        mMainSemaphoreAlreadyWaited( false ),
        mBeginFrameOnceStarted( false )
    {
        for( size_t i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
            mCurrentColourRTs[i] = 0;

        // set config options defaults
        initConfigOptions();
    }
    //-------------------------------------------------------------------------
    MetalRenderSystem::~MetalRenderSystem()
    {
        shutdown();

        // Destroy render windows
        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
            OGRE_DELETE i->second;
        mRenderTargets.clear();
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
    HardwareOcclusionQuery* MetalRenderSystem::createHardwareOcclusionQuery(void)
    {
        return 0; //TODO
    }
    //-------------------------------------------------------------------------
    RenderSystemCapabilities* MetalRenderSystem::createRenderSystemCapabilities(void) const
    {
        RenderSystemCapabilities* rsc = new RenderSystemCapabilities();
        rsc->setRenderSystemName(getName());

        rsc->setDeviceName(mActiveDevice->mDevice.name.UTF8String);

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_CPU == OGRE_CPU_ARM
        rsc->setVendor( GPU_APPLE );
#endif

        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setNumTextureUnits(16);
        rsc->setNumVertexTextureUnits(16);
        rsc->setCapability(RSC_ANISOTROPY);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
#if TARGET_OS_TV
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_ASTC);
#endif
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        // If the device is running macOS older than 11,
        // then they are all x86 systems which all support BCn
        bool supportsBCTextureCompression = true;
        if( @available( macOS 11, * ) )
        {
            supportsBCTextureCompression = mActiveDevice->mDevice.supportsBCTextureCompression;
        }

        if( supportsBCTextureCompression )
        {
            rsc->setCapability( RSC_TEXTURE_COMPRESSION_DXT );
            rsc->setCapability( RSC_TEXTURE_COMPRESSION_BC4_BC5 );
            // rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
        }
#else
        //Actually the limit is not the count but rather how many bytes are in the
        //GPU's internal TBDR cache (16 bytes for Family 1, 32 bytes for the rest)
        //Consult Metal's manual for more info.
        if( [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1] )
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_ASTC);
        }
#endif
        rsc->setCapability(RSC_32BIT_INDEX);
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_USER_CLIP_PLANES);
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        if( [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1] )
#endif
        {
            rsc->setCapability(RSC_DEPTH_CLAMP);
        }

        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        rsc->setNonPOW2TexturesLimited(false);
        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        rsc->setCapability(RSC_TEXTURE_FLOAT);
        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_TEXTURE_1D);
        rsc->setCapability(RSC_TEXTURE_3D);
        // rsc->setCapability(RSC_TEXTURE_SIGNED_INT);
        rsc->setCapability(RSC_VERTEX_PROGRAM);
        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        rsc->setCapability(RSC_VERTEX_FORMAT_INT_10_10_10_2);
        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);
        rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
        rsc->setMaxPointSize(256);

        rsc->setCapability(RSC_COMPUTE_PROGRAM);
        rsc->setCapability(RSC_HW_GAMMA);
        // rsc->setCapability(RSC_TEXTURE_GATHER);
        rsc->setCapability(RSC_TEXTURE_2D_ARRAY);
        // rsc->setCapability(RSC_CONST_BUFFER_SLOTS_IN_SHADER);

        //These don't make sense on Metal, so just use flexible defaults.
        rsc->setVertexProgramConstantFloatCount( 16384 );
        rsc->setFragmentProgramConstantFloatCount( 16384 );
        rsc->setComputeProgramConstantFloatCount( 16384 );

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        uint8 mrtCount = 8u;
#else
        //Actually the limit is not the count but rather how many bytes are in the
        //GPU's internal TBDR cache (16 bytes for Family 1, 32 bytes for the rest)
        //Consult Metal's manual for more info.
        uint8 mrtCount = 4u;
        if( [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1] ||
            [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2] ||
            [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1] )
        {
            mrtCount = 8u;
        }
#endif
        rsc->setNumMultiRenderTargets( std::min<int>(mrtCount, OGRE_MAX_MULTIPLE_RENDER_TARGETS) );

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        //uint16 max2DResolution = 16384;
#else
        uint16 max2DResolution = 4096;
        if( [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2] ||
            [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2] )
        {
            max2DResolution = 8192;
        }
        if( [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1] )
        {
            max2DResolution = 16384;
        }
#endif
        // rsc->setMaximumResolutions( max2DResolution, 2048, max2DResolution );

        //TODO: UAVs
        //rsc->setCapability(RSC_UAV);
        //rsc->setCapability(RSC_ATOMIC_COUNTERS);

        rsc->addShaderProfile( "metal" );

        rsc->setCapability(RSC_FIXED_FUNCTION); // FIXME: hack to get past RTSS

        DriverVersion driverVersion;

        struct FeatureSets
        {
            MTLFeatureSet featureSet;
            const char* name;
            int major;
            int minor;
        };

        FeatureSets featureSets[] =
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            { MTLFeatureSet_iOS_GPUFamily1_v1, "iOS_GPUFamily1_v1", 1, 1 },
            { MTLFeatureSet_iOS_GPUFamily2_v1, "iOS_GPUFamily2_v1", 2, 1 },

            { MTLFeatureSet_iOS_GPUFamily1_v2, "iOS_GPUFamily1_v2", 1, 2 },
            { MTLFeatureSet_iOS_GPUFamily2_v2, "iOS_GPUFamily2_v2", 2, 2 },
            { MTLFeatureSet_iOS_GPUFamily3_v1, "iOS_GPUFamily3_v2", 3, 2 },
#else
            { MTLFeatureSet_OSX_GPUFamily1_v1, "OSX_GPUFamily1_v1", 1, 1 },
#endif
        };

        for( size_t i=0; i<sizeof(featureSets) / sizeof(featureSets[0]); ++i )
        {
            if( [mActiveDevice->mDevice supportsFeatureSet:featureSets[i].featureSet] )
            {
                LogManager::getSingleton().logMessage( "Supports: " + String(featureSets[i].name) );
                driverVersion.major = featureSets[i].major;
                driverVersion.minor = featureSets[i].minor;
            }
        }

        rsc->setDriverVersion( driverVersion );

        return rsc;
    }
    //-------------------------------------------------------------------------
    RenderWindow* MetalRenderSystem::_createRenderWindow( const String &name,
                                                         unsigned int width, unsigned int height,
                                                         bool fullScreen,
                                                         const NameValuePairList *miscParams )
    {
        RenderSystem::_createRenderWindow(name, width, height, fullScreen, miscParams);
        if( !mInitialized )
        {
            // enable debug layer
            setenv("METAL_DEVICE_WRAPPER_TYPE", "1", 0);

            mDevice.init();
            setActiveDevice(&mDevice);

            const long c_inFlightCommandBuffers = 3;
            mMainGpuSyncSemaphore = dispatch_semaphore_create(c_inFlightCommandBuffers);
            mMainSemaphoreAlreadyWaited = false;
            mBeginFrameOnceStarted = false;
            mRealCapabilities = createRenderSystemCapabilities();
            mDriverVersion = mRealCapabilities->getDriverVersion();

            if (!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities( mCurrentCapabilities, 0 );

            mTextureManager = new MetalTextureManager( &mDevice );
            mHardwareBufferManager = new MetalHardwareBufferManager( &mDevice );

            psd = [MTLRenderPipelineDescriptor new];
            MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
            [psd setVertexDescriptor:vertexDescriptor];
            mDepthStencilDesc = [MTLDepthStencilDescriptor new];

            mInitialized = true;

            mDefaultVP = GpuProgramManager::getSingleton().createProgram("MetalDefaultVP", RGN_INTERNAL, "metal", GPT_VERTEX_PROGRAM).get();
            mDefaultFP = GpuProgramManager::getSingleton().createProgram("MetalDefaultFP", RGN_INTERNAL, "metal", GPT_FRAGMENT_PROGRAM).get();
            mDefaultVP->setSourceFile("DefaultShaders.metal");
            mDefaultVP->setParameter("entry_point", "default_vp");
            mDefaultFP->setSourceFile("DefaultShaders.metal");
            mDefaultFP->setParameter("entry_point", "default_fp");
            mDefaultFP->setParameter("shader_reflection_pair_hint", "MetalDefaultVP");
        }

        RenderWindow *win = OGRE_NEW MetalRenderWindow( &mDevice, this );
        win->create( name, width, height, fullScreen, miscParams );
        attachRenderTarget(*win);
        return win;
    }

    const GpuProgramParametersPtr& MetalRenderSystem::getFixedFunctionParams(TrackVertexColourType tracking,
                                                                      FogMode fog)
    {
        mDefaultVP->load();
        mDefaultFP->load();

        if(!mAutoParamsBuffer)
        {
            mDefaultFP->getDefaultParameters(); // also analyse FP params for testing

            auto vpParams = mDefaultVP->getDefaultParameters();
            vpParams->setNamedAutoConstant("texMtx", GpuProgramParameters::ACT_TEXTURE_MATRIX);
            vpParams->setNamedAutoConstant("mvpMtx", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

            // create const buffer to back VP (currently unused)
            if(auto sz = vpParams->getConstantList().size())
            {
                mAutoParamsBuffer.reset(new MetalHardwareBufferCommon(sz, HBU_CPU_TO_GPU,
                                                                      false, 4, &mDevice));
            }
        }

        bindGpuProgram(mDefaultVP);
        bindGpuProgram(mDefaultFP);

        return mDefaultVP->getDefaultParameters();
    }

    void MetalRenderSystem::applyFixedFunctionParams(const GpuProgramParametersPtr& params, uint16 mask)
    {
        bindGpuProgramParameters(GPT_VERTEX_PROGRAM, params, mask);
    }

    //-------------------------------------------------------------------------
    MultiRenderTarget* MetalRenderSystem::createMultiRenderTarget( const String & name )
    {
        MetalMultiRenderTarget *retVal = OGRE_NEW MetalMultiRenderTarget( name );
        attachRenderTarget( *retVal );
        return retVal;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTexture(size_t unit, bool enabled,  const TexturePtr& texPtr)
    {
        __unsafe_unretained id<MTLTexture> metalTexture = 0;

        if( texPtr && enabled )
        {
            MetalTexture *metalTex = static_cast<MetalTexture*>( texPtr.get() );
            metalTexture = metalTex->getTextureForSampling( this );
        }

        [mActiveRenderEncoder setVertexTexture:metalTexture atIndex:unit];
        [mActiveRenderEncoder setFragmentTexture:metalTexture atIndex:unit];
    }
    //-------------------------------------------------------------------------
    DepthBuffer* MetalRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
    {
        MTLTextureDescriptor *desc = [MTLTextureDescriptor new];
        desc.sampleCount = renderTarget->getFSAA();
        desc.textureType = renderTarget->getFSAA() > 1u ? MTLTextureType2DMultisample :
                                                          MTLTextureType2D;
        desc.width              = (NSUInteger)renderTarget->getWidth();
        desc.height             = (NSUInteger)renderTarget->getHeight();
        desc.depth              = (NSUInteger)1u;
        desc.arrayLength        = 1u;
        desc.mipmapLevelCount   = 1u;

        desc.usage = MTLTextureUsageRenderTarget;
        //if( renderTarget->prefersDepthTexture() )
        //    desc.usage |= MTLTextureUsageShaderRead;

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        desc.storageMode = MTLStorageModePrivate;
#endif

        PixelFormat desiredDepthBufferFormat = PF_DEPTH24_STENCIL8;//renderTarget->getDesiredDepthBufferFormat();

        MTLPixelFormat depthFormat = MTLPixelFormatInvalid;
        MTLPixelFormat stencilFormat = MTLPixelFormatInvalid;
        MetalMappings::getDepthStencilFormat( mActiveDevice, desiredDepthBufferFormat,
                                              depthFormat, stencilFormat );

        id<MTLTexture> depthTexture = 0;
        id<MTLTexture> stencilTexture = 0;

        if( depthFormat != MTLPixelFormatInvalid )
        {
            desc.pixelFormat = depthFormat;
            depthTexture = [mActiveDevice->mDevice newTextureWithDescriptor: desc];
        }

        if( stencilFormat != MTLPixelFormatInvalid )
        {
            if( stencilFormat != MTLPixelFormatDepth32Float_Stencil8
   #if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            && stencilFormat != MTLPixelFormatDepth24Unorm_Stencil8
   #endif
            )
            {
                //Separate Stencil & Depth
                desc.pixelFormat = stencilFormat;
                stencilTexture = [mActiveDevice->mDevice newTextureWithDescriptor: desc];
            }
            else
            {
                //Combined Stencil & Depth
                stencilTexture = depthTexture;
            }
        }

        DepthBuffer *retVal = new MetalDepthBuffer( 0, this, renderTarget->getWidth(),
                                                    renderTarget->getHeight(),
                                                    renderTarget->getFSAA(),
                                                    depthFormat, false,
                                                    depthTexture, stencilTexture,
                                                    mActiveDevice );

        return retVal;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_waitForTailFrameToFinish(void)
    {
        if( !mMainSemaphoreAlreadyWaited )
        {
            dispatch_semaphore_wait( mMainGpuSyncSemaphore, DISPATCH_TIME_FOREVER );
            mMainSemaphoreAlreadyWaited = true;
        }
    }
    //-------------------------------------------------------------------------
    bool MetalRenderSystem::_willTailFrameStall(void)
    {
        bool retVal = mMainSemaphoreAlreadyWaited;

        if( !mMainSemaphoreAlreadyWaited )
        {
            const long result = dispatch_semaphore_wait( mMainGpuSyncSemaphore, DISPATCH_TIME_NOW );
            if( result == 0 )
            {
                retVal = true;
                //Semaphore was just grabbed, so ensure we don't grab it twice.
                mMainSemaphoreAlreadyWaited = true;
            }
        }

        return retVal;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_beginFrame(void)
    {
        RenderSystem::_beginFrame();

        assert( !mBeginFrameOnceStarted &&
                "Calling MetalRenderSystem::_beginFrameOnce more than once "
                "without matching call to _endFrameOnce!!!" );

        //Allow the renderer to preflight 3 frames on the CPU (using a semapore as a guard) and
        //commit them to the GPU. This semaphore will get signaled once the GPU completes a
        //frame's work via addCompletedHandler callback below, signifying the CPU can go ahead
        //and prepare another frame.
        _waitForTailFrameToFinish();

        mBeginFrameOnceStarted = true;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_endFrame(void)
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

        mActiveRenderTarget = 0;
        mActiveDevice->mFrameAborted = false;
        mMainSemaphoreAlreadyWaited = false;
        mBeginFrameOnceStarted = false;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::setScissorTest(bool enabled, const Rect& rect)
    {
        if(!enabled)
            return;

        MTLScissorRect scissorRect;
        scissorRect.x       = rect.left;
        scissorRect.y       = rect.top;
        scissorRect.width   = rect.width();
        scissorRect.height  = rect.height();
        [mActiveRenderEncoder setScissorRect:scissorRect];
    }
    void MetalRenderSystem::_setViewport(Viewport *vp)
    {
        if( mActiveViewport != vp )
        {
            mActiveViewport = vp;

            if( vp )
            {
                //const bool activeHasColourWrites = mNumMRTs != 0;

                if( vp->getTarget() != mActiveRenderTarget )
                {
                    _setRenderTarget( vp->getTarget() );
                }

#if 0
                if( mActiveRenderEncoder || ( !mActiveRenderEncoder /*&&
                                              (!vp->coversEntireTarget() ||
                                               !vp->scissorsMatchViewport())*/ ) )
#endif
                {
                    if( !mActiveRenderEncoder )
                        createRenderEncoder();

                    //if( !vp->coversEntireTarget() )
                    {
                        MTLViewport mtlVp;
                        mtlVp.originX   = vp->getActualLeft();
                        mtlVp.originY   = vp->getActualTop();
                        mtlVp.width     = vp->getActualWidth();
                        mtlVp.height    = vp->getActualHeight();
                        mtlVp.znear     = 0;
                        mtlVp.zfar      = 1;
                        [mActiveRenderEncoder setViewport:mtlVp];
                    }
                }
            }
        }

        //if( mActiveRenderEncoder && mUavsDirty )
        //    flushUAVs();
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::setActiveDevice( MetalDevice *device )
    {
        if( mActiveDevice != device )
        {
            mActiveDevice           = device;
            mActiveRenderEncoder    = device ? device->mRenderEncoder : 0;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::createRenderEncoder(void)
    {
        assert( mActiveDevice );

        //We assume the RenderWindow is at MRT#0 (in fact it
        //would be very odd to use the RenderWindow as MRT)
        if( mNumMRTs > 0 )
            mActiveDevice->mFrameAborted |= !mCurrentColourRTs[0]->nextDrawable();

        if( mActiveDevice->mFrameAborted )
        {
            mActiveDevice->endAllEncoders();
            mActiveRenderEncoder = 0;
            return;
        }

        const bool hadActiveRenderEncoder = mActiveRenderEncoder;
        Viewport *prevViewport = 0;
        RenderTarget *prevRenderTarget = 0;
        if( hadActiveRenderEncoder )
        {
            //Ending all encoders will clear those, so we'll have to restore them.
            prevViewport = mActiveViewport;
            prevRenderTarget = mActiveRenderTarget;
        }

        mActiveDevice->endAllEncoders();
        mActiveRenderEncoder = 0;

        if( hadActiveRenderEncoder )
        {
            if( prevViewport /*&& (!prevViewport->coversEntireTarget() ||
                                 !prevViewport->scissorsMatchViewport())*/ )
            {
                _setViewport( prevViewport );
            }
            else
            {
                mActiveViewport = prevViewport;
                mActiveRenderTarget = prevRenderTarget;
            }
        }

        //TODO: With a couple modifications, Compositor should be able to tell us
        //if we can use MTLStoreActionDontCare.

        MTLRenderPassDescriptor *passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        for( uint8 i=0; i<mNumMRTs; ++i )
        {
            passDesc.colorAttachments[i] = [mCurrentColourRTs[i]->mColourAttachmentDesc copy];
            mCurrentColourRTs[i]->mColourAttachmentDesc.loadAction = MTLLoadActionLoad;

            assert( passDesc.colorAttachments[i].texture );
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

        // static_cast<MetalVaoManager*>( mVaoManager )->bindDrawId();
        [mActiveRenderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        // flushUAVs();

        if (mStencilEnabled)
        {
            [mActiveRenderEncoder setStencilReferenceValue:mStencilRefValue];
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_notifyActiveEncoderEnded(void)
    {
        mActiveViewport = 0;
        mActiveRenderTarget = 0;
        mActiveRenderEncoder = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_notifyActiveComputeEnded(void)
    {
        //mComputePso = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_notifyDeviceStalled(void)
    {
        MetalHardwareBufferManager *hwBufferMgr = static_cast<MetalHardwareBufferManager*>(
                    mHardwareBufferManager );
        hwBufferMgr->_notifyDeviceStalled();
    }
    void MetalRenderSystem::setColourBlendState(const ColourBlendState& state)
    {
        mCurrentBlend = state;

        uint8 mrtCount = 1;
        /*for( int i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            if( newPso->pass.colourFormat[i] != PF_NULL )
                mrtCount = i + 1u;
        }*/

        for( int i=0; i<mrtCount; ++i )
        {
            psd.colorAttachments[i].blendingEnabled = state.blendingEnabled();
            psd.colorAttachments[i].rgbBlendOperation           = MetalMappings::get( state.operation );
            psd.colorAttachments[i].alphaBlendOperation         = MetalMappings::get( state.alphaOperation );
            psd.colorAttachments[i].sourceRGBBlendFactor        = MetalMappings::get( state.sourceFactor );
            psd.colorAttachments[i].destinationRGBBlendFactor   = MetalMappings::get( state.destFactor );
            psd.colorAttachments[i].sourceAlphaBlendFactor      = MetalMappings::get( state.sourceFactorAlpha );
            psd.colorAttachments[i].destinationAlphaBlendFactor = MetalMappings::get( state.destFactorAlpha );

            psd.colorAttachments[i].writeMask =
                        state.writeR << 3 | state.writeG << 2 | state.writeB << 1 | state.writeA;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setSampler( size_t texUnit, Sampler& s)
    {
        id <MTLSamplerState> sampler = static_cast<MetalSampler&>(s).getState();
        [mActiveRenderEncoder setVertexSamplerState:sampler atIndex: texUnit];
        [mActiveRenderEncoder setFragmentSamplerState:sampler atIndex: texUnit];
    }

    void MetalRenderSystem::_setDepthClamp(bool enable)
    {
        [mActiveRenderEncoder setDepthClipMode:enable ? MTLDepthClipModeClamp : MTLDepthClipModeClip];
    }

    void MetalRenderSystem::_setDepthBufferParams( bool depthTest, bool depthWrite, CompareFunction depthFunction )
    {
        mDepthStencilDescChanged = true;
        if(!depthTest)
        {
            mDepthStencilDesc.depthCompareFunction = MTLCompareFunctionAlways;
            mDepthStencilDesc.depthWriteEnabled    = false;
            return;
        }

        mDepthStencilDesc.depthCompareFunction = MetalMappings::get( depthFunction );
        mDepthStencilDesc.depthWriteEnabled    = depthWrite;
    }
    void MetalRenderSystem::_setCullingMode(CullingMode mode)
    {
        MTLCullMode cullMode = MTLCullModeNone;

        switch( mode )
        {
        case CULL_NONE:             cullMode = MTLCullModeNone; break;
        case CULL_CLOCKWISE:        cullMode = MTLCullModeBack; break;
        case CULL_ANTICLOCKWISE:    cullMode = MTLCullModeFront; break;
        }

        [mActiveRenderEncoder setCullMode:cullMode];
        }
    void MetalRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        [mActiveRenderEncoder setDepthBias:constantBias
                                     slopeScale:slopeScaleBias
                                     clamp:0.0f];
    }

    void MetalRenderSystem::_setPolygonMode(PolygonMode level)
    {
        MTLTriangleFillMode fillMode =
            level == PM_SOLID ? MTLTriangleFillModeFill : MTLTriangleFillModeLines;
        [mActiveRenderEncoder setTriangleFillMode:fillMode];
    }

    void MetalRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        psd.alphaToCoverageEnabled = (func != CMPF_ALWAYS_PASS) && alphaToCoverage;
    }

    //-------------------------------------------------------------------------
    void MetalRenderSystem::_convertProjectionMatrix( const Matrix4& matrix, Matrix4& dest,
                                                      bool forGpuProgram )
    {
        dest = matrix;

        // Convert depth range from [-1,+1] to [0,1]
        dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
        dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
        dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
        dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
    }
    //-------------------------------------------------------------------------
    id<MTLRenderPipelineState> MetalRenderSystem::getPipelineState()
    {
        NSUInteger psdHash = [psd hash];
        auto it = mPSOCache.find(psdHash);
        if(it != mPSOCache.end())
            return it->second;

        NSError* error = NULL;
        id <MTLRenderPipelineState> pso =
                [mActiveDevice->mDevice newRenderPipelineStateWithDescriptor:psd error:&error];

        if( !pso || error )
        {
            String errorDesc;
            if( error )
                errorDesc = [error localizedDescription].UTF8String;

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to created pipeline state for rendering: " + errorDesc);
        }

        mPSOCache.emplace(psdHash, pso);

        return pso;
    }

    void MetalRenderSystem::_render( const RenderOperation &op )
    {
        if(!mActiveRenderTarget) return;

        // Call super class.
        RenderSystem::_render(op);

        mCurrentIndexBuffer = op.indexData;
        mCurrentVertexBuffer= op.vertexData;
        mCurrentPrimType    = std::min(  MTLPrimitiveTypeTriangleStrip,
                                         static_cast<MTLPrimitiveType>( op.operationType - 1u ) );

        psd.colorAttachments[0].pixelFormat =  MetalMappings::getPixelFormat(
                        mActiveRenderTarget->suggestPixelFormat(), mActiveRenderTarget->isHardwareGammaEnabled() );
        psd.depthAttachmentPixelFormat = static_cast<MetalDepthBuffer*>(mActiveRenderTarget->getDepthBuffer())->getFormat();

        MTLVertexDescriptor *vertexDescriptor = [psd vertexDescriptor];
        [vertexDescriptor reset];

        size_t unused;
        for (auto elem : mCurrentVertexBuffer->vertexDeclaration->getElements())
        {
            auto bufferIdx = elem.getSource();
            if (!mCurrentVertexBuffer->vertexBufferBinding->isBufferBound(bufferIdx))
                continue; // skip unbound elements
            auto elementIdx = MetalProgram::getAttributeIndex(elem.getSemantic());

            auto hwbuf = mCurrentVertexBuffer->vertexBufferBinding->getBuffer(bufferIdx).get();

            vertexDescriptor.attributes[elementIdx].format = MetalMappings::get( elem.getType() );
            vertexDescriptor.attributes[elementIdx].bufferIndex = bufferIdx;
            vertexDescriptor.attributes[elementIdx].offset = elem.getOffset();

            vertexDescriptor.layouts[bufferIdx].stride = hwbuf->getVertexSize();
            vertexDescriptor.layouts[bufferIdx].stepFunction = MTLVertexStepFunctionPerVertex;

            auto mtlbuf = hwbuf->_getImpl<MetalHardwareBufferCommon>();
            [mActiveRenderEncoder setVertexBuffer:mtlbuf->getBufferName(unused) offset:0 atIndex:bufferIdx];
        }

        if(mDepthStencilDescChanged)
        {
            mDepthStencilState =
                    [mActiveDevice->mDevice newDepthStencilStateWithDescriptor:mDepthStencilDesc];
            mDepthStencilDescChanged = false;
        }

        [mActiveRenderEncoder setDepthStencilState:mDepthStencilState];

        [mActiveRenderEncoder setRenderPipelineState:getPipelineState()];

        const size_t numberOfInstances = op.numberOfInstances;

        // Render to screen!
        if( op.useIndexes )
        {
            do
            {
                const MTLIndexType indexType = static_cast<MTLIndexType>(
                            mCurrentIndexBuffer->indexBuffer->getType() );

                //Get index buffer stuff which is the same for all draws in this cmd
                const size_t bytesPerIndexElement = mCurrentIndexBuffer->indexBuffer->getIndexSize();

                size_t offsetStart;
                auto metalBuffer = mCurrentIndexBuffer->indexBuffer->_getImpl<MetalHardwareBufferCommon>();
                __unsafe_unretained id<MTLBuffer> indexBuffer = metalBuffer->getBufferName( offsetStart );

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                OgreAssertDbg( ((mCurrentIndexBuffer->indexStart * bytesPerIndexElement) & 0x03) == 0
                            ,"Index Buffer must be aligned to 4 bytes. If you're messing with "
                            "IndexBuffer::indexStart, you've entered an invalid "
                            "indexStart; not supported by the Metal API." );

                [mActiveRenderEncoder drawIndexedPrimitives:mCurrentPrimType
                           indexCount:mCurrentIndexBuffer->indexCount
                            indexType:indexType
                          indexBuffer:indexBuffer
                    indexBufferOffset:mCurrentIndexBuffer->indexStart * bytesPerIndexElement + offsetStart
                        instanceCount:numberOfInstances];
#else
                [mActiveRenderEncoder drawIndexedPrimitives:mCurrentPrimType
                           indexCount:mCurrentIndexBuffer->indexCount
                            indexType:indexType
                          indexBuffer:indexBuffer
                    indexBufferOffset:mCurrentIndexBuffer->indexStart * bytesPerIndexElement + offsetStart
                        instanceCount:numberOfInstances
                           baseVertex:mCurrentVertexBuffer->vertexStart
                         baseInstance:0];
#endif
            } while (updatePassIterationRenderState());
        }
        else
        {
            do
            {
                // Update derived depth bias.
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    [mActiveRenderEncoder setDepthBias:mDerivedDepthBiasBase +
                                                       mDerivedDepthBiasMultiplier *
                                                       mCurrentPassIterationNum
                                            slopeScale:mDerivedDepthBiasSlopeScale
                                                 clamp:0.0f];
                }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                const uint32 vertexStart = 0;
#else
                const uint32 vertexStart = static_cast<uint32>( mCurrentVertexBuffer->vertexStart );
#endif

                if (numberOfInstances > 1)
                {
                    [mActiveRenderEncoder drawPrimitives:mCurrentPrimType
                                             vertexStart:vertexStart
                                             vertexCount:mCurrentVertexBuffer->vertexCount
                                           instanceCount:numberOfInstances];
                }
                else
                {
                    [mActiveRenderEncoder drawPrimitives:mCurrentPrimType
                                             vertexStart:vertexStart
                                             vertexCount:mCurrentVertexBuffer->vertexCount];
                }
            } while (updatePassIterationRenderState());
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        auto shader = static_cast<MetalProgram*>( prg->_getBindingDelegate() );
        if(prg->getType() == GPT_VERTEX_PROGRAM)
            [psd setVertexFunction:shader->getMetalFunction()];
        else
            [psd setFragmentFunction:shader->getMetalFunction()];
    }

    void MetalRenderSystem::bindGpuProgramParameters( GpuProgramType gptype,
                                                      const GpuProgramParametersPtr& params,
                                                      uint16 variabilityMask )
    {
        mActiveParameters[gptype] = params;

        switch (gptype)
        {
        case GPT_GEOMETRY_PROGRAM:
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Geometry Shaders are not supported in Metal.");
            break;
        case GPT_HULL_PROGRAM:
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Tesselation is different in Metal.");
            break;
        case GPT_DOMAIN_PROGRAM:
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Tesselation is different in Metal.");
            break;
        default:
            break;
        }

        // update const buffer
        #if 1
        [mActiveRenderEncoder setVertexBytes:params->getFloatPointer(0)
            length:params->getConstantList().size() atIndex:MetalProgram::UNIFORM_INDEX_START];
        #else
        // TODO rather use this, but buffer seems to be never updated
        size_t unused;
        mAutoParamsBuffer->writeData(0, params->getConstantList().size(), params->getFloatPointer(0));
        [mActiveRenderEncoder setVertexBuffer:mAutoParamsBuffer->getBufferName(unused) offset:0 atIndex:MetalProgram::UNIFORM_INDEX_START];
        #endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::clearFrameBuffer( unsigned int buffers, const ColourValue& colour,
                                             float depth, unsigned short stencil )
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
    void MetalRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        #if 0
        uint8 viewportRenderTargetFlags = 0;
        {
            const bool activeHasColourWrites = mNumMRTs != 0;
            if( mActiveRenderTarget == target &&
                activeHasColourWrites == (viewportRenderTargetFlags & VP_RTT_COLOUR_WRITE) )
            {
                if( mActiveRenderEncoder && mUavsDirty )
                    flushUAVs();
                return;
            }
        }
        #endif

        if( mActiveDevice )
            mActiveDevice->endRenderEncoder();

        mActiveRenderTarget = target;

        if( auto metalTarget = dynamic_cast<MetalRenderTargetCommon*>(target) )
        {
            //if( target->getForceDisableColourWrites() )
            //    viewportRenderTargetFlags &= ~VP_RTT_COLOUR_WRITE;

            mCurrentColourRTs[0] = metalTarget;
            //We need to set mCurrentColourRTs[0] to grab the active device,
            //even if we won't be drawing to colour target.
            target->getCustomAttribute( "mNumMRTs", &mNumMRTs );

            MetalDevice *ownerDevice = 0;

            //if( viewportRenderTargetFlags & VP_RTT_COLOUR_WRITE )
            {
                for( size_t i=0; i<mNumMRTs; ++i )
                {
                    MTLRenderPassColorAttachmentDescriptor *desc =
                            mCurrentColourRTs[i]->mColourAttachmentDesc;

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

                    ownerDevice = mCurrentColourRTs[i]->getOwnerDevice();
                }
            }
            if(0) //else
            {
                mNumMRTs = 0;
            }

            MetalDepthBuffer *depthBuffer = static_cast<MetalDepthBuffer*>( target->getDepthBuffer() );

            if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH && !depthBuffer )
            {
                // Depth is automatically managed and there is no depth buffer attached to this RT
                setDepthBufferFor( target );
            }

            depthBuffer = static_cast<MetalDepthBuffer*>( target->getDepthBuffer() );
            mCurrentDepthBuffer = depthBuffer;
            if( depthBuffer )
            {
                if( depthBuffer->mDepthAttachmentDesc )
                    depthBuffer->mDepthAttachmentDesc.storeAction = MTLStoreActionStore;
                if( depthBuffer->mStencilAttachmentDesc )
                    depthBuffer->mStencilAttachmentDesc.storeAction = MTLStoreActionStore;

                ownerDevice = depthBuffer->getOwnerDevice();
            }

            setActiveDevice( ownerDevice );
        }
        else
        {
            mNumMRTs = 0;
            mCurrentDepthBuffer = 0;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::initGPUProfiling(void)
    {
#if OGRE_PROFILING
//        _rmt_BindMetal( mActiveDevice->mCurrentCommandBuffer );
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::deinitGPUProfiling(void)
    {
#if OGRE_PROFILING
        _rmt_UnbindMetal();
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::beginGPUSampleProfile( const String &name, uint32 *hashCache )
    {
#if OGRE_PROFILING
        _rmt_BeginMetalSample( name.c_str(), hashCache );
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::endGPUSampleProfile( const String &name )
    {
#if OGRE_PROFILING
        _rmt_EndMetalSample();
#endif
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
    void MetalRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        //DepthBuffer::DefaultDepthBufferFormat = PF_D32_FLOAT_X24_S8_UINT;
        mMetalProgramFactory = new MetalProgramFactory( &mDevice );
        HighLevelGpuProgramManager::getSingleton().addFactory( mMetalProgramFactory );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::setStencilState(const StencilState& state)
    {
        mStencilEnabled = state.enabled;
        // There are two main cases:
        // 1. The active render encoder is valid and will be subsequently used for drawing.
        //      We need to set the stencil reference value on this encoder. We do this below.
        // 2. The active render is invalid or is about to go away.
        //      In this case, we need to set the stencil reference value on the new encoder when it is created
        //      (see createRenderEncoder). (In this case, the setStencilReferenceValue below in this wasted, but it is inexpensive).

        if (mStencilEnabled)
        {
            mStencilRefValue = state.referenceValue;

            if( mActiveRenderEncoder )
                [mActiveRenderEncoder setStencilReferenceValue:mStencilRefValue];
        }
    }
 }
