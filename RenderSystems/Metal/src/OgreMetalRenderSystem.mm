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
#include "OgreMetalTexture.h"
#include "OgreMetalMultiRenderTarget.h"

#include "OgreMetalHardwareBufferManager.h"
#include "OgreMetalHardwareIndexBuffer.h"
#include "OgreMetalHardwareVertexBuffer.h"

#include "Vao/OgreMetalConstBufferPacked.h"
#include "Vao/OgreMetalUavBufferPacked.h"
#include "Vao/OgreIndirectBufferPacked.h"
#include "Vao/OgreVertexArrayObject.h"
#include "CommandBuffer/OgreCbDrawCall.h"

#include "OgreFrustum.h"
#include "OgreViewport.h"
#include "Compositor/OgreCompositorManager2.h"

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
        mComputePso( 0 ),
        mCurrentIndexBuffer( 0 ),
        mCurrentVertexBuffer( 0 ),
        mCurrentPrimType( MTLPrimitiveTypePoint ),
        mAutoParamsBufferIdx( 0 ),
        mCurrentAutoParamsBufferPtr( 0 ),
        mCurrentAutoParamsBufferSpaceLeft( 0 ),
        mMaxModifiedUavPlusOne( 0 ),
        mUavsDirty( false ),
        mNumMRTs( 0 ),
        mCurrentDepthBuffer( 0 ),
        mActiveDevice( 0 ),
        mActiveRenderEncoder( 0 ),
        mDevice( this ),
        mMainGpuSyncSemaphore( 0 ),
        mMainSemaphoreAlreadyWaited( false ),
        mBeginFrameOnceStarted( false )
    {
        memset( mHistoricalAutoParamsSize, 0, sizeof(mHistoricalAutoParamsSize) );
        for( size_t i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
            mCurrentColourRTs[i] = 0;
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
        for( size_t i=0; i<mAutoParamsBuffer.size(); ++i )
        {
            if( mAutoParamsBuffer[i]->getMappingState() != MS_UNMAPPED )
                mAutoParamsBuffer[i]->unmap( UO_UNMAP_ALL );
            mVaoManager->destroyConstBuffer( mAutoParamsBuffer[i] );
        }
        mAutoParamsBuffer.clear();
        mAutoParamsBufferIdx = 0;
        mCurrentAutoParamsBufferPtr = 0;
        mCurrentAutoParamsBufferSpaceLeft = 0;

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
        rsc->setNumVertexTextureUnits(16);
        rsc->setCapability(RSC_ANISOTROPY);
        rsc->setCapability(RSC_AUTOMIPMAP);
        rsc->setCapability(RSC_BLENDING);
        rsc->setCapability(RSC_DOT3);
        rsc->setCapability(RSC_CUBEMAPPING);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
#if TARGET_OS_TV
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_ASTC);
#endif
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5);
        //rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
#else
        //Actually the limit is not the count but rather how many bytes are in the
        //GPU's internal TBDR cache (16 bytes for Family 1, 32 bytes for the rest)
        //Consult Metal's manual for more info.
        if( [mActiveDevice->mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1] )
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_ASTC);
        }
#endif
        rsc->setCapability(RSC_VBO);
        rsc->setCapability(RSC_32BIT_INDEX);
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_USER_CLIP_PLANES);
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);
        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        rsc->setNonPOW2TexturesLimited(false);
        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        rsc->setCapability(RSC_TEXTURE_FLOAT);
        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
        rsc->setCapability(RSC_TEXTURE_1D);
        rsc->setCapability(RSC_TEXTURE_3D);
        rsc->setCapability(RSC_TEXTURE_SIGNED_INT);
        rsc->setCapability(RSC_VERTEX_PROGRAM);
        rsc->setCapability(RSC_FRAGMENT_PROGRAM);
        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);
        rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
        rsc->setMaxPointSize(256);

        rsc->setCapability(RSC_COMPUTE_PROGRAM);
        rsc->setCapability(RSC_HW_GAMMA);
        rsc->setCapability(RSC_TEXTURE_GATHER);
        rsc->setCapability(RSC_TEXTURE_2D_ARRAY);
        rsc->setCapability(RSC_CONST_BUFFER_SLOTS_IN_SHADER);

        //These don't make sense on Metal, so just use flexible defaults.
        rsc->setVertexProgramConstantFloatCount( 16384 );
        rsc->setVertexProgramConstantBoolCount( 16384 );
        rsc->setVertexProgramConstantIntCount( 16384 );
        rsc->setFragmentProgramConstantFloatCount( 16384 );
        rsc->setFragmentProgramConstantBoolCount( 16384 );
        rsc->setFragmentProgramConstantIntCount( 16384 );
        rsc->setComputeProgramConstantFloatCount( 16384 );
        rsc->setComputeProgramConstantBoolCount( 16384 );
        rsc->setComputeProgramConstantIntCount( 16384 );

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
        rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        uint16 max2DResolution = 16384;
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
        rsc->setMaximumResolutions( max2DResolution, 2048, max2DResolution );

        //TODO: UAVs
        //rsc->setCapability(RSC_UAV);
        //rsc->setCapability(RSC_ATOMIC_COUNTERS);

        rsc->addShaderProfile( "metal" );

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
            mMainSemaphoreAlreadyWaited = false;
            mBeginFrameOnceStarted = false;
            mRealCapabilities = createRenderSystemCapabilities();
            mDriverVersion = mRealCapabilities->getDriverVersion();

            if (!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities( mCurrentCapabilities, 0 );

            mTextureManager = new MetalTextureManager( &mDevice );
            mVaoManager = OGRE_NEW MetalVaoManager( c_inFlightCommandBuffers, &mDevice );
            mHardwareBufferManager = new v1::MetalHardwareBufferManager( &mDevice, mVaoManager );

            mInitialized = true;
        }

        RenderWindow *win = OGRE_NEW MetalRenderWindow( &mDevice, this );
        win->create( name, width, height, fullScreen, miscParams );
        return win;
    }
    //-------------------------------------------------------------------------
    MultiRenderTarget* MetalRenderSystem::createMultiRenderTarget( const String & name )
    {
        MetalMultiRenderTarget *retVal = OGRE_NEW MetalMultiRenderTarget( name );
        attachRenderTarget( *retVal );
        return retVal;
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
        assert( slot < 64 );

        if( !mUavs[slot].buffer && mUavs[slot].texture.isNull() && texture.isNull() )
            return;

        mUavs[slot].texture = texture;
        mUavs[slot].buffer  = 0;

        if( !texture.isNull() )
        {
            if( !(texture->getUsage() & TU_UAV) )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Texture " + texture->getName() +
                             " must have been created with TU_UAV to be bound as UAV",
                             "GL3PlusRenderSystem::queueBindUAV" );
            }

            mUavs[slot].textureName = static_cast<MetalTexture*>( texture.get() )->
                    getTextureForSampling( this );
        }

        mMaxModifiedUavPlusOne = std::max( mMaxModifiedUavPlusOne, static_cast<uint8>( slot + 1 ) );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::queueBindUAV( uint32 slot, UavBufferPacked *buffer,
                                          ResourceAccess::ResourceAccess access,
                                          size_t offset, size_t sizeBytes )
    {
        assert( slot < 64 );

        if( mUavs[slot].texture.isNull() && !mUavs[slot].buffer && !buffer )
            return;

        mUavs[slot].buffer = buffer;
        mUavs[slot].texture.setNull();

        if( buffer )
        {
            mUavs[slot].offset      = offset;
            //mUavs[slot].sizeBytes   = sizeBytes;
        }

        mMaxModifiedUavPlusOne = std::max( mMaxModifiedUavPlusOne, static_cast<uint8>( slot + 1 ) );
        mUavsDirty = true;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::clearUAVs(void)
    {
        for( size_t i=0; i<mMaxModifiedUavPlusOne; ++i )
        {
            mUavs[i].texture.setNull();
            mUavs[i].buffer = 0;
        }
        mUavsDirty = mMaxModifiedUavPlusOne != 0;
        mMaxModifiedUavPlusOne = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::flushUAVs(void)
    {
        for( uint32 i=0; i<mMaxModifiedUavPlusOne; ++i )
        {
            if( !mUavs[i].texture.isNull() )
            {
                //TODO: Replace OGRE_METAL_UAV_SLOT_START w/ something
                //more dynamic, possibly sync with Hlms.
                [mActiveRenderEncoder setVertexTexture:mUavs[i].textureName
                                               atIndex:i + OGRE_METAL_UAV_SLOT_START];
                [mActiveRenderEncoder setFragmentTexture:mUavs[i].textureName
                                                 atIndex:i + OGRE_METAL_UAV_SLOT_START];
            }
            else if( mUavs[i].buffer )
            {
                MetalUavBufferPacked *metalUav = static_cast<MetalUavBufferPacked*>( mUavs[i].buffer );
                metalUav->bindBufferAllRenderStages( i, mUavs[i].offset );
            }
            else
            {
                [mActiveRenderEncoder setVertexTexture:0 atIndex:i + OGRE_METAL_UAV_SLOT_START];
                [mActiveRenderEncoder setFragmentTexture:0 atIndex:i + OGRE_METAL_UAV_SLOT_START];
                [mActiveRenderEncoder setVertexBuffer:0
                                               offset:0
                                              atIndex:i + OGRE_METAL_UAV_SLOT_START];
                [mActiveRenderEncoder setFragmentBuffer:0
                                                 offset:0
                                                atIndex:i + OGRE_METAL_UAV_SLOT_START];
            }
        }

        mUavsDirty = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_bindTextureUavCS( uint32 slot, Texture *texture,
                                               ResourceAccess::ResourceAccess access,
                                               int32 mipmapLevel, int32 textureArrayIndex,
                                               PixelFormat pixelFormat )
    {
        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mActiveDevice->getComputeEncoder();

        __unsafe_unretained id<MTLTexture> metalTexture = 0;

        if( texture )
        {
            MetalTexture *metalTex = static_cast<MetalTexture*>( texture );
            metalTexture = metalTex->getTextureForSampling( this );
        }

        [computeEncoder setTexture:metalTexture atIndex:slot + OGRE_METAL_CS_UAV_SLOT_START];
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTextureCS( uint32 slot, bool enabled, Texture *texPtr )
    {
        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mActiveDevice->getComputeEncoder();

        __unsafe_unretained id<MTLTexture> metalTexture = 0;

        if( texPtr && enabled )
        {
            MetalTexture *metalTex = static_cast<MetalTexture*>( texPtr );
            metalTexture = metalTex->getTextureForSampling( this );
        }

        [computeEncoder setTexture:metalTexture atIndex:slot];
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setHlmsSamplerblockCS( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
        assert( (!samplerblock || samplerblock->mRsData) &&
                "The block must have been created via HlmsManager::getSamplerblock!" );

        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mActiveDevice->getComputeEncoder();

        if( !samplerblock )
        {
            [computeEncoder setSamplerState:0 atIndex:texUnit];
        }
        else
        {
            __unsafe_unretained id<MTLSamplerState> sampler =
                    (__bridge id<MTLSamplerState>)samplerblock->mRsData;
            [computeEncoder setSamplerState:sampler atIndex:texUnit];
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setTexture(size_t unit, bool enabled,  Texture *texPtr)
    {
        __unsafe_unretained id<MTLTexture> metalTexture = 0;

        if( texPtr && enabled )
        {
            MetalTexture *metalTex = static_cast<MetalTexture*>( texPtr );
            metalTexture = metalTex->getTextureForSampling( this );
        }

        [mActiveRenderEncoder setVertexTexture:metalTexture atIndex:unit];
        [mActiveRenderEncoder setFragmentTexture:metalTexture atIndex:unit];
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
        if( renderTarget->prefersDepthTexture() )
            desc.usage |= MTLTextureUsageShaderRead;

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        desc.storageMode = MTLStorageModePrivate;
#endif

        PixelFormat desiredDepthBufferFormat = renderTarget->getDesiredDepthBufferFormat();

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
                                                    renderTarget->getFSAA(), 0,
                                                    desiredDepthBufferFormat,
                                                    renderTarget->prefersDepthTexture(), false,
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
    void MetalRenderSystem::_beginFrameOnce(void)
    {
        assert( !mBeginFrameOnceStarted &&
                "Calling MetalRenderSystem::_beginFrameOnce more than once "
                "without matching call to _endFrameOnce!!!" );

        //Allow the renderer to preflight 3 frames on the CPU (using a semapore as a guard) and
        //commit them to the GPU. This semaphore will get signaled once the GPU completes a
        //frame's work via addCompletedHandler callback below, signifying the CPU can go ahead
        //and prepare another frame.
        _waitForTailFrameToFinish();

        mBeginFrameOnceStarted = true;

        mActiveRenderTarget = 0;
        mActiveViewport = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_endFrameOnce(void)
    {
        //TODO: We shouldn't tidy up JUST the active device. But all of them.

        cleanAutoParamsBuffers();

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
        mActiveViewport = 0;
        mActiveDevice->mFrameAborted = false;
        mMainSemaphoreAlreadyWaited = false;
        mBeginFrameOnceStarted = false;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::cleanAutoParamsBuffers(void)
    {
        const size_t numUsedBuffers = mAutoParamsBufferIdx;
        size_t usedBytes = 0;
        for( size_t i=0; i<numUsedBuffers; ++i )
        {
            mAutoParamsBuffer[i]->unmap( (i == 0u && numUsedBuffers == 1u) ? UO_KEEP_PERSISTENT :
                                                                             UO_UNMAP_ALL );
            usedBytes += mAutoParamsBuffer[i]->getTotalSizeBytes();
        }

        //Get the max amount of bytes consumed in the last N frames.
        const int numHistoricSamples = sizeof(mHistoricalAutoParamsSize) /
                                       sizeof(mHistoricalAutoParamsSize[0]);
        mHistoricalAutoParamsSize[numHistoricSamples-1] = usedBytes;
        for( int i=0; i<numHistoricSamples - 1; ++i )
        {
            usedBytes = std::max( usedBytes, mHistoricalAutoParamsSize[i + 1] );
            mHistoricalAutoParamsSize[i] = mHistoricalAutoParamsSize[i + 1];
        }

        if( //Merge all buffers into one for the next frame.
            numUsedBuffers > 1u ||
            //Historic data shows we've been using less memory. Shrink this record.
            (!mAutoParamsBuffer.empty() && mAutoParamsBuffer[0]->getTotalSizeBytes() > usedBytes) )
        {
            if( mAutoParamsBuffer[0]->getMappingState() != MS_UNMAPPED )
                mAutoParamsBuffer[0]->unmap( UO_UNMAP_ALL );

            for( size_t i=0; i<mAutoParamsBuffer.size(); ++i )
                mVaoManager->destroyConstBuffer( mAutoParamsBuffer[i] );
            mAutoParamsBuffer.clear();

            if( usedBytes > 0 )
            {
                ConstBufferPacked *constBuffer = mVaoManager->createConstBuffer( usedBytes,
                                                                                 BT_DYNAMIC_PERSISTENT,
                                                                                 0, false );
                mAutoParamsBuffer.push_back( constBuffer );
            }
        }

        mCurrentAutoParamsBufferPtr = 0;
        mCurrentAutoParamsBufferSpaceLeft = 0;
        mAutoParamsBufferIdx = 0;

    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_hlmsComputePipelineStateObjectCreated( HlmsComputePso *newPso )
    {
        MetalProgram *computeShader = static_cast<MetalProgram*>(
                    newPso->computeShader->_getBindingDelegate() );

        //Btw. HlmsCompute guarantees mNumThreadGroups won't have zeroes.
        assert( newPso->mNumThreadGroups[0] != 0 &&
                newPso->mNumThreadGroups[1] != 0 &&
                newPso->mNumThreadGroups[2] != 0 &&
                "Invalid parameters. Will also cause div. by zero!" );

        NSError* error = 0;
        MTLComputePipelineDescriptor *psd = [[MTLComputePipelineDescriptor alloc] init];
        psd.computeFunction = computeShader->getMetalFunction();
        psd.threadGroupSizeIsMultipleOfThreadExecutionWidth =
                (newPso->mThreadsPerGroup[0] % newPso->mNumThreadGroups[0] == 0) &&
                (newPso->mThreadsPerGroup[1] % newPso->mNumThreadGroups[1] == 0) &&
                (newPso->mThreadsPerGroup[2] % newPso->mNumThreadGroups[2] == 0);

        id<MTLComputePipelineState> pso =
                [mActiveDevice->mDevice newComputePipelineStateWithDescriptor:psd
                                                                      options:MTLPipelineOptionNone
                                                                   reflection:nil
                                                                        error:&error];
        if( !pso || error )
        {
            String errorDesc;
            if( error )
                errorDesc = [error localizedDescription].UTF8String;

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to create pipeline state for compute, error " +
                         errorDesc, "MetalProgram::_hlmsComputePipelineStateObjectCreated" );
        }

        newPso->rsData = const_cast<void*>( CFBridgingRetain( pso ) );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_hlmsComputePipelineStateObjectDestroyed( HlmsComputePso *pso )
    {
        id<MTLComputePipelineState> metalPso = reinterpret_cast< id<MTLComputePipelineState> >(
                    CFBridgingRelease( pso->rsData ) );
        pso->rsData = 0;
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
        if( mActiveViewport != vp )
        {
            mActiveViewport = vp;

            if( vp )
            {
                const bool activeHasColourWrites = mNumMRTs != 0;

                if( vp->getTarget() != mActiveRenderTarget ||
                    vp->getColourWrite() != activeHasColourWrites )
                {
                    _setRenderTarget( vp->getTarget(), vp->getViewportRenderTargetFlags() );
                }

                if( mActiveRenderEncoder || ( !mActiveRenderEncoder &&
                                              (!vp->coversEntireTarget() ||
                                               !vp->scissorsMatchViewport()) ) )
                {
                    if( !mActiveRenderEncoder )
                        createRenderEncoder();

                    if( !vp->coversEntireTarget() )
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

                    if( !vp->scissorsMatchViewport() )
                    {
                        MTLScissorRect scissorRect;
                        scissorRect.x       = vp->getScissorActualLeft();
                        scissorRect.y       = vp->getScissorActualTop();
                        scissorRect.width   = vp->getScissorActualWidth();
                        scissorRect.height  = vp->getScissorActualHeight();
                        [mActiveRenderEncoder setScissorRect:scissorRect];
                    }
                }
            }
        }

        if( mActiveRenderEncoder && mUavsDirty )
            flushUAVs();
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
            if( prevViewport && (!prevViewport->coversEntireTarget() ||
                                 !prevViewport->scissorsMatchViewport()) )
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

        static_cast<MetalVaoManager*>( mVaoManager )->bindDrawId();
        [mActiveRenderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        flushUAVs();

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
        mPso = 0;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_notifyActiveComputeEnded(void)
    {
        mComputePso = 0;
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

            if( pso->pass.stencilParams.enabled )
            {
                if( pso->pass.stencilParams.stencilFront != StencilStateOp() )
                {
                    const StencilStateOp &stencilOp = pso->pass.stencilParams.stencilFront;

                    MTLStencilDescriptor *stencilDesc = [MTLStencilDescriptor alloc];
                    stencilDesc.stencilCompareFunction = MetalMappings::get( stencilOp.compareOp );
                    stencilDesc.stencilFailureOperation = MetalMappings::get( stencilOp.stencilFailOp );
                    stencilDesc.depthFailureOperation =
                            MetalMappings::get( stencilOp.stencilDepthFailOp );
                    stencilDesc.depthStencilPassOperation =
                            MetalMappings::get( stencilOp.stencilPassOp );

                    stencilDesc.readMask = pso->pass.stencilParams.readMask;
                    stencilDesc.writeMask = pso->pass.stencilParams.writeMask;

                    depthStateDesc.frontFaceStencil = stencilDesc;
                }

                if( pso->pass.stencilParams.stencilBack != StencilStateOp() )
                {
                    const StencilStateOp &stencilOp = pso->pass.stencilParams.stencilBack;

                    MTLStencilDescriptor *stencilDesc = [MTLStencilDescriptor alloc];
                    stencilDesc.stencilCompareFunction = MetalMappings::get( stencilOp.compareOp );
                    stencilDesc.stencilFailureOperation = MetalMappings::get( stencilOp.stencilFailOp );
                    stencilDesc.depthFailureOperation =
                            MetalMappings::get( stencilOp.stencilDepthFailOp );
                    stencilDesc.depthStencilPassOperation =
                            MetalMappings::get( stencilOp.stencilPassOp );

                    stencilDesc.readMask = pso->pass.stencilParams.readMask;
                    stencilDesc.writeMask = pso->pass.stencilParams.writeMask;

                    depthStateDesc.backFaceStencil = stencilDesc;
                }
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

        MetalProgram *vertexShader = 0;
        MetalProgram *pixelShader = 0;

        if( !newPso->vertexShader.isNull() )
        {
            vertexShader = static_cast<MetalProgram*>( newPso->vertexShader->_getBindingDelegate() );
            [psd setVertexFunction:vertexShader->getMetalFunction()];
        }

        if( !newPso->pixelShader.isNull() )
        {
            pixelShader = static_cast<MetalProgram*>( newPso->pixelShader->_getBindingDelegate() );
            [psd setFragmentFunction:pixelShader->getMetalFunction()];
        }
        //Ducttape shaders
//        id<MTLLibrary> library = [mActiveDevice->mDevice newDefaultLibrary];
//        [psd setVertexFunction:[library newFunctionWithName:@"vertex_vs"]];
//        [psd setFragmentFunction:[library newFunctionWithName:@"pixel_ps"]];

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // On iOS we can skip the Vertex Desc.
        // On OS X we always need to set it for the baseInstance / draw id
        if( !newPso->vertexElements.empty() )
#endif
        {
            size_t numUVs = 0;
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
                    size_t elementIdx = MetalVaoManager::getAttributeIndexFor( it->mSemantic );
                    if( it->mSemantic == VES_TEXTURE_COORDINATES )
                    {
                        elementIdx += numUVs;
                        ++numUVs;
                    }
                    vertexDescriptor.attributes[elementIdx].format = MetalMappings::get( it->mType );
                    vertexDescriptor.attributes[elementIdx].bufferIndex = bufferIdx;
                    vertexDescriptor.attributes[elementIdx].offset = accumOffset;

                    accumOffset += v1::VertexElement::getTypeSize( it->mType );
                    ++it;
                }

                vertexDescriptor.layouts[bufferIdx].stride = accumOffset;
                vertexDescriptor.layouts[bufferIdx].stepFunction = MTLVertexStepFunctionPerVertex;

                ++itor;
            }

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            vertexDescriptor.attributes[15].format      = MTLVertexFormatUInt;
            vertexDescriptor.attributes[15].bufferIndex = 15;
            vertexDescriptor.attributes[15].offset      = 0;

            vertexDescriptor.layouts[15].stride = 4;
            vertexDescriptor.layouts[15].stepFunction = MTLVertexStepFunctionPerInstance;
#endif
            [psd setVertexDescriptor:vertexDescriptor];
        }

        uint8 mrtCount = 0;
        for( int i=0; i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            if( newPso->pass.colourFormat[i] != PF_NULL )
                mrtCount = i + 1u;
        }

        for( int i=0; i<mrtCount; ++i )
        {
            HlmsBlendblock const *blendblock = newPso->blendblock;
            psd.colorAttachments[i].pixelFormat = MetalMappings::getPixelFormat(
                        newPso->pass.colourFormat[i], newPso->pass.hwGamma[i] );

            if( psd.colorAttachments[i].pixelFormat == MTLPixelFormatInvalid ||
                (blendblock->mBlendOperation == SBO_ADD &&
                 blendblock->mSourceBlendFactor == SBF_ONE &&
                 blendblock->mDestBlendFactor == SBF_ZERO &&
                (!blendblock->mSeparateBlend ||
                 (blendblock->mBlendOperation == blendblock->mBlendOperationAlpha &&
                 blendblock->mSourceBlendFactor == blendblock->mSourceBlendFactorAlpha &&
                 blendblock->mDestBlendFactor == blendblock->mDestBlendFactorAlpha))) )
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

        if( newPso->pass.depthFormat != PF_NULL )
        {
            MTLPixelFormat depthFormat = MTLPixelFormatInvalid;
            MTLPixelFormat stencilFormat = MTLPixelFormatInvalid;
            MetalMappings::getDepthStencilFormat( mActiveDevice, newPso->pass.depthFormat,
                                                  depthFormat, stencilFormat );
            psd.depthAttachmentPixelFormat = depthFormat;
            psd.stencilAttachmentPixelFormat = stencilFormat;
        }

        NSError* error = NULL;
        id <MTLRenderPipelineState> pso =
                [mActiveDevice->mDevice newRenderPipelineStateWithDescriptor:psd error:&error];

        if( !pso || error )
        {
            String errorDesc;
            if( error )
                errorDesc = [error localizedDescription].UTF8String;

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Failed to created pipeline state for rendering, error " + errorDesc,
                         "MetalRenderSystem::_hlmsPipelineStateObjectCreated" );
        }

        id <MTLDepthStencilState> depthStencilState = getDepthStencilState( newPso );

        MetalHlmsPso *metalPso = new MetalHlmsPso();
        metalPso->pso = pso;
        metalPso->depthStencilState = depthStencilState;
        metalPso->vertexShader      = vertexShader;
        metalPso->pixelShader       = pixelShader;

        switch( newPso->macroblock->mCullMode )
        {
        case CULL_NONE:             metalPso->cullMode = MTLCullModeNone; break;
        case CULL_CLOCKWISE:        metalPso->cullMode = MTLCullModeBack; break;
        case CULL_ANTICLOCKWISE:    metalPso->cullMode = MTLCullModeFront; break;
        }

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
            __unsafe_unretained id <MTLSamplerState> sampler =
                    (__bridge id<MTLSamplerState>)samplerblock->mRsData;
            [mActiveRenderEncoder setVertexSamplerState:sampler atIndex: texUnit];
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
        [mActiveRenderEncoder setCullMode:metalPso->cullMode];

        if( mPso != metalPso )
        {
            [mActiveRenderEncoder setRenderPipelineState:metalPso->pso];
            mPso = metalPso;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setComputePso( const HlmsComputePso *pso )
    {
        __unsafe_unretained id<MTLComputePipelineState> metalPso =
                (__bridge id<MTLComputePipelineState>)pso->rsData;

        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mActiveDevice->getComputeEncoder();

        if( mComputePso != pso )
        {
            [computeEncoder setComputePipelineState:metalPso];
            mComputePso = pso;
        }
    }
    //-------------------------------------------------------------------------
    VertexElementType MetalRenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ARGB;
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
    Real MetalRenderSystem::getRSDepthRange(void) const
    {
         return 1.0f;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_makeProjectionMatrix( Real left, Real right, Real bottom, Real top,
                                                   Real nearPlane, Real farPlane, Matrix4 &dest,
                                                   bool forGpuProgram )
    {
        // Correct position for off-axis projection matrix
        if (!forGpuProgram)
        {
            Real offsetX = left + right;
            Real offsetY = top + bottom;

            left -= offsetX;
            right -= offsetX;
            top -= offsetY;
            bottom -= offsetY;
        }

        Real width = right - left;
        Real height = top - bottom;
        Real q, qn;
        if (farPlane == 0)
        {
            q = 1 - Frustum::INFINITE_FAR_PLANE_ADJUST;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 1);
        }
        else
        {
            q = farPlane / ( farPlane - nearPlane );
            qn = -q * nearPlane;
        }
        dest = Matrix4::ZERO;
        dest[0][0] = 2 * nearPlane / width;
        dest[0][2] = (right+left) / width;
        dest[1][1] = 2 * nearPlane / height;
        dest[1][2] = (top+bottom) / height;
        dest[2][2] = -q;
        dest[3][2] = -1.0f;
        dest[2][3] = qn;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_makeProjectionMatrix( const Radian& fovy, Real aspect, Real nearPlane,
                                                   Real farPlane, Matrix4& dest, bool forGpuProgram )
    {
        Radian theta ( fovy * 0.5 );
        Real h = 1 / Math::Tan(theta);
        Real w = h / aspect;
        Real q, qn;
        if (farPlane == 0)
        {
            q = 1 - Frustum::INFINITE_FAR_PLANE_ADJUST;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 1);
        }
        else
        {
            q = farPlane / ( farPlane - nearPlane );
            qn = -q * nearPlane;
        }

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;

        dest[2][2] = -q;
        dest[3][2] = -1.0f;

        dest[2][3] = qn;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_makeOrthoMatrix( const Radian& fovy, Real aspect, Real nearPlane,
                                              Real farPlane, Matrix4& dest, bool forGpuProgram )
    {
        Radian thetaY (fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        //Real thetaX = thetaY * aspect;
        Real tanThetaX = tanThetaY * aspect; //Math::Tan(thetaX);
        Real half_w = tanThetaX * nearPlane;
        Real half_h = tanThetaY * nearPlane;
        Real iw = 1.0f / half_w;
        Real ih = 1.0f / half_h;
        Real q;
        if (farPlane == 0)
        {
            q = 0;
        }
        else
        {
            q = 1.0f / (farPlane - nearPlane);
        }

        dest = Matrix4::ZERO;
        dest[0][0] = iw;
        dest[1][1] = ih;
        dest[2][2] = -q;
        dest[2][3] = -nearPlane / (farPlane - nearPlane);
        dest[3][3] = 1;
    }
    //---------------------------------------------------------------------
    void MetalRenderSystem::_applyObliqueDepthProjection( Matrix4& matrix, const Plane& plane,
                                                          bool forGpuProgram )
    {
        // Thanks to Eric Lenyel for posting this calculation at www.terathon.com

        // Calculate the clip-space corner point opposite the clipping plane
        // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
        // transform it into camera space by multiplying it
        // by the inverse of the projection matrix

        Vector4 q;
        q.x = (Math::Sign(plane.normal.x) + matrix[0][2]) / matrix[0][0];
        q.y = (Math::Sign(plane.normal.y) + matrix[1][2]) / matrix[1][1];
        q.z = -1.0F;
        q.w = (1.0F + matrix[2][2]) / matrix[2][3];

        // Calculate the scaled plane vector
        Vector4 clipPlane4d(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
        Vector4 c = clipPlane4d * (2.0F / (clipPlane4d.dotProduct(q)));

        // Replace the third row of the projection matrix
        matrix[2][0] = c.x;
        matrix[2][1] = c.y;
        matrix[2][2] = c.z + 1.0F;
        matrix[2][3] = c.w;
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_dispatch( const HlmsComputePso &pso )
    {
        __unsafe_unretained id<MTLComputeCommandEncoder> computeEncoder =
                mActiveDevice->getComputeEncoder();

        MTLSize numThreadGroups = MTLSizeMake( pso.mNumThreadGroups[0],
                                               pso.mNumThreadGroups[1],
                                               pso.mNumThreadGroups[2] );
        MTLSize threadsPerGroup = MTLSizeMake( pso.mThreadsPerGroup[0],
                                               pso.mThreadsPerGroup[1],
                                               pso.mThreadsPerGroup[2] );

        [computeEncoder dispatchThreadgroups:numThreadGroups
                       threadsPerThreadgroup:threadsPerGroup];
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setVertexArrayObject( const VertexArrayObject *vao )
    {
        __unsafe_unretained id<MTLBuffer> metalVertexBuffers[15];
        NSUInteger offsets[15];
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
        assert( numVertexBuffers < 15u );
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
        uint32 bytesPerVertexBuffer[15];
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
#if OGRE_DEBUG_MODE
            assert( ((drawCmd->firstVertexIndex * bytesPerIndexElement) & 0x03) == 0
                    && "Index Buffer must be aligned to 4 bytes. If you're messing with "
                    "VertexArrayObject::setPrimitiveRange, you've entered an invalid "
                    "primStart; not supported by the Metal API." );
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            for( size_t j=0; j<numVertexBuffers; ++j )
            {
                //Manually set vertex buffer offsets since in iOS baseVertex is not supported
                //until iOS GPU Family 3 v1 & OS X (we just use indirect rendering there).
                [mActiveRenderEncoder setVertexBufferOffset:drawCmd->baseVertex * bytesPerVertexBuffer[j]
                                                            atIndex:j];
            }

            //Setup baseInstance.
            [mActiveRenderEncoder setVertexBufferOffset:drawCmd->baseInstance * sizeof(uint32)
                                                atIndex:15];

            [mActiveRenderEncoder drawIndexedPrimitives:primType
                       indexCount:drawCmd->primCount
                        indexType:indexType
                      indexBuffer:indexBuffer
                indexBufferOffset:drawCmd->firstVertexIndex * bytesPerIndexElement
                    instanceCount:drawCmd->instanceCount];
#else
            [mActiveRenderEncoder drawIndexedPrimitives:primType
                       indexCount:drawCmd->primCount
                        indexType:indexType
                      indexBuffer:indexBuffer
                indexBufferOffset:drawCmd->firstVertexIndex * bytesPerIndexElement
                    instanceCount:drawCmd->instanceCount
                       baseVertex:drawCmd->baseVertex
                     baseInstance:drawCmd->baseInstance];
#endif
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
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            //Setup baseInstance.
            [mActiveRenderEncoder setVertexBufferOffset:drawCmd->baseInstance * sizeof(uint32)
                                                atIndex:15];
            [mActiveRenderEncoder drawPrimitives:primType
                      vertexStart:drawCmd->firstVertexIndex
                      vertexCount:drawCmd->primCount
                    instanceCount:drawCmd->instanceCount];
#else
            [mActiveRenderEncoder drawPrimitives:primType
                      vertexStart:drawCmd->firstVertexIndex
                      vertexCount:drawCmd->primCount
                    instanceCount:drawCmd->instanceCount
                     baseInstance:drawCmd->baseInstance];
#endif
            ++drawCmd;
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_setRenderOperation( const v1::CbRenderOp *cmd )
    {
        __unsafe_unretained id<MTLBuffer> metalVertexBuffers[15];
        NSUInteger offsets[15];
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
            assert( slot < 15u );
#endif
            size_t offsetStart;
            metalVertexBuffers[slot] = metalBuffer->getBufferName( offsetStart );
            offsets[slot] = offsetStart;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            offsets[slot] += cmd->vertexData->vertexStart * metalBuffer->getVertexSize();
#endif
            ++itor;
            maxUsedSlot = std::max( maxUsedSlot, slot + 1u );
        }

        [mActiveRenderEncoder setVertexBuffers:metalVertexBuffers offsets:offsets
                                               withRange:NSMakeRange( 0, maxUsedSlot )];

        mCurrentIndexBuffer = cmd->indexData;
        mCurrentVertexBuffer= cmd->vertexData;
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

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    #if OGRE_DEBUG_MODE
            assert( ((cmd->firstVertexIndex * bytesPerIndexElement) & 0x03) == 0
                    && "Index Buffer must be aligned to 4 bytes. If you're messing with "
                    "IndexBuffer::indexStart, you've entered an invalid "
                    "indexStart; not supported by the Metal API." );
    #endif

        //Setup baseInstance.
        [mActiveRenderEncoder setVertexBufferOffset:cmd->baseInstance * sizeof(uint32)
                                            atIndex:15];

        [mActiveRenderEncoder drawIndexedPrimitives:mCurrentPrimType
                   indexCount:cmd->primCount
                    indexType:indexType
                  indexBuffer:indexBuffer
            indexBufferOffset:cmd->firstVertexIndex * bytesPerIndexElement + offsetStart
            instanceCount:cmd->instanceCount];
#else
        [mActiveRenderEncoder drawIndexedPrimitives:mCurrentPrimType
                   indexCount:cmd->primCount
                    indexType:indexType
                  indexBuffer:indexBuffer
            indexBufferOffset:cmd->firstVertexIndex * bytesPerIndexElement + offsetStart
                instanceCount:cmd->instanceCount
                   baseVertex:mCurrentVertexBuffer->vertexStart
                 baseInstance:cmd->baseInstance];
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const v1::CbDrawCallStrip *cmd )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        //Setup baseInstance.
        [mActiveRenderEncoder setVertexBufferOffset:cmd->baseInstance * sizeof(uint32)
                                            atIndex:15];
        [mActiveRenderEncoder drawPrimitives:mCurrentPrimType
                  vertexStart:0 /*cmd->firstVertexIndex already handled in _setRenderOperation*/
                  vertexCount:cmd->primCount
                instanceCount:cmd->instanceCount];
#else
        [mActiveRenderEncoder drawPrimitives:mCurrentPrimType
                  vertexStart:cmd->firstVertexIndex
                  vertexCount:cmd->primCount
                instanceCount:cmd->instanceCount
                 baseInstance:cmd->baseInstance];
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::_render( const v1::RenderOperation &op )
    {
        // Call super class.
        RenderSystem::_render(op);

        const size_t numberOfInstances = op.numberOfInstances;
        const bool hasInstanceData = mCurrentVertexBuffer->vertexBufferBinding->getHasInstanceData();

        // Render to screen!
        if( op.useIndexes )
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

                const MTLIndexType indexType = static_cast<MTLIndexType>(
                            mCurrentIndexBuffer->indexBuffer->getType() );

                //Get index buffer stuff which is the same for all draws in this cmd
                const size_t bytesPerIndexElement = mCurrentIndexBuffer->indexBuffer->getIndexSize();

                size_t offsetStart;
                v1::MetalHardwareIndexBuffer *metalBuffer =
                    static_cast<v1::MetalHardwareIndexBuffer*>( mCurrentIndexBuffer->indexBuffer.get() );
                __unsafe_unretained id<MTLBuffer> indexBuffer = metalBuffer->getBufferName( offsetStart );

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    #if OGRE_DEBUG_MODE
                    assert( ((mCurrentIndexBuffer->indexStart * bytesPerIndexElement) & 0x03) == 0
                            && "Index Buffer must be aligned to 4 bytes. If you're messing with "
                            "IndexBuffer::indexStart, you've entered an invalid "
                            "indexStart; not supported by the Metal API." );
    #endif

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

                if (hasInstanceData)
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
    void MetalRenderSystem::bindGpuProgramParameters( GpuProgramType gptype,
                                                      GpuProgramParametersSharedPtr params,
                                                      uint16 variabilityMask )
    {
        MetalProgram *shader = 0;
        switch (gptype)
        {
        case GPT_VERTEX_PROGRAM:
            mActiveVertexGpuProgramParameters = params;
            shader = mPso->vertexShader;
            break;
        case GPT_FRAGMENT_PROGRAM:
            mActiveFragmentGpuProgramParameters = params;
            shader = mPso->pixelShader;
            break;
        case GPT_GEOMETRY_PROGRAM:
            mActiveGeometryGpuProgramParameters = params;
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Geometry Shaders are not supported in Metal.",
                         "MetalRenderSystem::bindGpuProgramParameters" );
            break;
        case GPT_HULL_PROGRAM:
            mActiveTessellationHullGpuProgramParameters = params;
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Tesselation is different in Metal.",
                         "MetalRenderSystem::bindGpuProgramParameters" );
            break;
        case GPT_DOMAIN_PROGRAM:
            mActiveTessellationDomainGpuProgramParameters = params;
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Tesselation is different in Metal.",
                         "MetalRenderSystem::bindGpuProgramParameters" );
            break;
        case GPT_COMPUTE_PROGRAM:
            mActiveComputeGpuProgramParameters = params;
            shader = static_cast<MetalProgram*>( mComputePso->computeShader->_getBindingDelegate() );
            break;
        default:
            break;
        }

        size_t bytesToWrite = shader->getBufferRequiredSize();
        if( shader && bytesToWrite > 0 )
        {
            if( mCurrentAutoParamsBufferSpaceLeft < bytesToWrite )
            {
                if( mAutoParamsBufferIdx >= mAutoParamsBuffer.size() )
                {
                    ConstBufferPacked *constBuffer =  mVaoManager->createConstBuffer(
                                std::max<size_t>( 512u * 1024u, bytesToWrite ),
                                BT_DYNAMIC_PERSISTENT, 0, false );
                    mAutoParamsBuffer.push_back( constBuffer );
                }

                ConstBufferPacked *constBuffer = mAutoParamsBuffer[mAutoParamsBufferIdx];
                mCurrentAutoParamsBufferPtr = reinterpret_cast<uint8*>(
                            constBuffer->map( 0, constBuffer->getNumElements() ) );
                mCurrentAutoParamsBufferSpaceLeft = constBuffer->getTotalSizeBytes();

                ++mAutoParamsBufferIdx;
            }

            shader->updateBuffers( params, mCurrentAutoParamsBufferPtr );

            assert( dynamic_cast<MetalConstBufferPacked*>(
                    mAutoParamsBuffer[mAutoParamsBufferIdx-1u] ) );

            MetalConstBufferPacked *constBuffer = static_cast<MetalConstBufferPacked*>(
                        mAutoParamsBuffer[mAutoParamsBufferIdx-1u] );
            const size_t bindOffset = constBuffer->getTotalSizeBytes() -
                                        mCurrentAutoParamsBufferSpaceLeft;
            switch (gptype)
            {
            case GPT_VERTEX_PROGRAM:
                constBuffer->bindBufferVS( OGRE_METAL_PARAMETER_SLOT - OGRE_METAL_CONST_SLOT_START,
                                           bindOffset );
                break;
            case GPT_FRAGMENT_PROGRAM:
                constBuffer->bindBufferPS( OGRE_METAL_PARAMETER_SLOT - OGRE_METAL_CONST_SLOT_START,
                                           bindOffset );
                break;
            case GPT_COMPUTE_PROGRAM:
                constBuffer->bindBufferCS( OGRE_METAL_CS_PARAMETER_SLOT - OGRE_METAL_CS_CONST_SLOT_START,
                                           bindOffset );
                break;
            case GPT_GEOMETRY_PROGRAM:
            case GPT_HULL_PROGRAM:
            case GPT_DOMAIN_PROGRAM:
                break;
            }

            mCurrentAutoParamsBufferPtr += bytesToWrite;

            const uint8 *oldBufferPos = mCurrentAutoParamsBufferPtr;
            mCurrentAutoParamsBufferPtr = reinterpret_cast<uint8*>(
                    alignToNextMultiple( reinterpret_cast<uintptr_t>( mCurrentAutoParamsBufferPtr ),
                                         mVaoManager->getConstBufferAlignment() ) );
            bytesToWrite += mCurrentAutoParamsBufferPtr - oldBufferPos;

            //We know that bytesToWrite <= mCurrentAutoParamsBufferSpaceLeft, but that was
            //before padding. After padding this may no longer hold true.
            mCurrentAutoParamsBufferSpaceLeft -= std::min( mCurrentAutoParamsBufferSpaceLeft,
                                                           bytesToWrite );
        }
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
    void MetalRenderSystem::_setRenderTarget(RenderTarget *target, uint8 viewportRenderTargetFlags)
    {
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

        if( mActiveDevice )
            mActiveDevice->endRenderEncoder();

        mActiveRenderTarget = target;

        if( target )
        {
            if( target->getForceDisableColourWrites() )
                viewportRenderTargetFlags &= ~VP_RTT_COLOUR_WRITE;

            mCurrentColourRTs[0] = 0;
            //We need to set mCurrentColourRTs[0] to grab the active device,
            //even if we won't be drawing to colour target.
            target->getCustomAttribute( "mNumMRTs", &mNumMRTs );
            target->getCustomAttribute( "MetalRenderTargetCommon", &mCurrentColourRTs[0] );

            MetalDevice *ownerDevice = 0;

            if( viewportRenderTargetFlags & VP_RTT_COLOUR_WRITE )
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
            else
            {
                mNumMRTs = 0;
            }

            MetalDepthBuffer *depthBuffer = static_cast<MetalDepthBuffer*>( target->getDepthBuffer() );

            if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH && !depthBuffer )
            {
                // Depth is automatically managed and there is no depth buffer attached to this RT
                setDepthBufferFor( target, true );
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
    void MetalRenderSystem::_notifyCompositorNodeSwitchedRenderTarget( RenderTarget *previousTarget )
    {
        if( previousTarget )
        {
            bool mustClear = false;
            uint8 numMRTs = 0;
            MetalRenderTargetCommon *currentColourRTs[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            previousTarget->getCustomAttribute( "mNumMRTs", &numMRTs );
            previousTarget->getCustomAttribute( "MetalRenderTargetCommon", &currentColourRTs[0] );

            for( size_t i=0; i<numMRTs; ++i )
            {
                if( currentColourRTs[i] )
                {
                    if( currentColourRTs[i]->mColourAttachmentDesc.loadAction == MTLLoadActionClear )
                        mustClear = true;
                }
            }

            MetalDepthBuffer *depthBuffer = static_cast<MetalDepthBuffer*>(
                        previousTarget->getDepthBuffer() );

            if( depthBuffer )
            {
                if( depthBuffer->mDepthAttachmentDesc &&
                    depthBuffer->mDepthAttachmentDesc.loadAction == MTLLoadActionClear )
                {
                    mustClear = true;
                }
                if( depthBuffer->mStencilAttachmentDesc &&
                    depthBuffer->mStencilAttachmentDesc.loadAction == MTLLoadActionClear )
                {
                    mustClear = true;
                }
            }

            if( mustClear )
            {
                _setRenderTarget( previousTarget, true );
                createRenderEncoder();
            }
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
    void MetalRenderSystem::initGPUProfiling(void)
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
//        _rmt_BindMetal( mActiveDevice->mCurrentCommandBuffer );
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::deinitGPUProfiling(void)
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
        _rmt_UnbindMetal();
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::beginGPUSampleProfile( const String &name, uint32 *hashCache )
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
        _rmt_BeginMetalSample( name.c_str(), hashCache );
#endif
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::endGPUSampleProfile( const String &name )
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
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
    void MetalRenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
    {
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        DepthBuffer::DefaultDepthBufferFormat = PF_D32_FLOAT_X24_S8_UINT;
        mShaderManager = OGRE_NEW MetalGpuProgramManager( &mDevice );
        mMetalProgramFactory = new MetalProgramFactory( &mDevice );
        HighLevelGpuProgramManager::getSingleton().addFactory( mMetalProgramFactory );
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::updateCompositorManager( CompositorManager2 *compositorManager,
                                                     SceneManagerEnumerator &sceneManagers,
                                                     HlmsManager *hlmsManager )
    {
        // Metal requires that a frame's worth of rendering be invoked inside an autorelease pool.
        // This is true for both iOS and macOS.
        @autoreleasepool
        {
            compositorManager->_updateImplementation( sceneManagers, hlmsManager );
        }
    }
    //-------------------------------------------------------------------------
    void MetalRenderSystem::setStencilBufferParams( uint32 refValue, const StencilParams &stencilParams )
    {
        RenderSystem::setStencilBufferParams( refValue, stencilParams );

        // There are two main cases:
        // 1. The active render encoder is valid and will be subsequently used for drawing.
        //      We need to set the stencil reference value on this encoder. We do this below.
        // 2. The active render is invalid or is about to go away.
        //      In this case, we need to set the stencil reference value on the new encoder when it is created
        //      (see createRenderEncoder). (In this case, the setStencilReferenceValue below in this wasted, but it is inexpensive).

        // Save this info so we can transfer it into a new encoder if necessary
        mStencilEnabled = stencilParams.enabled;
        if (mStencilEnabled)
        {
            mStencilRefValue = refValue;

            if( mActiveRenderEncoder )
                [mActiveRenderEncoder setStencilReferenceValue:refValue];
        }
    }
 }
