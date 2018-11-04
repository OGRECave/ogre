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

#include "OgreStableHeaders.h"

#include "OgreHlmsPbs.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsListener.h"
#include "OgreLwString.h"

#if !OGRE_NO_JSON
    #include "OgreHlmsJsonPbs.h"
#endif

#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreCamera.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreForward3D.h"
#include "Cubemaps/OgreParallaxCorrectedCubemap.h"
#include "OgreIrradianceVolume.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbTexture.h"
#include "CommandBuffer/OgreCbShaderBuffer.h"

#include "Animation/OgreSkeletonInstance.h"

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
    #include "OgrePlanarReflections.h"
#endif

#include "OgreLogManager.h"
#include "OgreProfiler.h"

namespace Ogre
{
    const IdString PbsProperty::HwGammaRead       = IdString( "hw_gamma_read" );
    const IdString PbsProperty::HwGammaWrite      = IdString( "hw_gamma_write" );
    const IdString PbsProperty::SignedIntTex      = IdString( "signed_int_textures" );
    const IdString PbsProperty::MaterialsPerBuffer= IdString( "materials_per_buffer" );
    const IdString PbsProperty::LowerGpuOverhead  = IdString( "lower_gpu_overhead" );
    const IdString PbsProperty::DebugPssmSplits   = IdString( "debug_pssm_splits" );
    const IdString PbsProperty::HasPlanarReflections=IdString( "has_planar_reflections" );

    const IdString PbsProperty::NumTextures     = IdString( "num_textures" );
    const char *PbsProperty::DiffuseMap         = "diffuse_map";
    const char *PbsProperty::NormalMapTex       = "normal_map_tex";
    const char *PbsProperty::SpecularMap        = "specular_map";
    const char *PbsProperty::RoughnessMap       = "roughness_map";
    const char *PbsProperty::EmissiveMap        = "emissive_map";
    const char *PbsProperty::EnvProbeMap        = "envprobe_map";
    const char *PbsProperty::DetailWeightMap    = "detail_weight_map";
    const char *PbsProperty::DetailMapN         = "detail_map";     //detail_map0-4
    const char *PbsProperty::DetailMapNmN       = "detail_map_nm";  //detail_map_nm0-4

    const IdString PbsProperty::DetailMap0      = "detail_map0";
    const IdString PbsProperty::DetailMap1      = "detail_map1";
    const IdString PbsProperty::DetailMap2      = "detail_map2";
    const IdString PbsProperty::DetailMap3      = "detail_map3";

    const IdString PbsProperty::NormalMap         = IdString( "normal_map" );

    const IdString PbsProperty::FresnelScalar     = IdString( "fresnel_scalar" );
    const IdString PbsProperty::UseTextureAlpha   = IdString( "use_texture_alpha" );
    const IdString PbsProperty::TransparentMode   = IdString( "transparent_mode" );
    const IdString PbsProperty::FresnelWorkflow   = IdString( "fresnel_workflow" );
    const IdString PbsProperty::MetallicWorkflow  = IdString( "metallic_workflow" );
    const IdString PbsProperty::TwoSidedLighting  = IdString( "two_sided_lighting" );
    const IdString PbsProperty::ReceiveShadows    = IdString( "receive_shadows" );
    const IdString PbsProperty::UsePlanarReflections=IdString( "use_planar_reflections" );

    const IdString PbsProperty::NormalWeight          = IdString( "normal_weight" );
    const IdString PbsProperty::NormalWeightTex       = IdString( "normal_weight_tex" );
    const IdString PbsProperty::NormalWeightDetail0   = IdString( "normal_weight_detail0" );
    const IdString PbsProperty::NormalWeightDetail1   = IdString( "normal_weight_detail1" );
    const IdString PbsProperty::NormalWeightDetail2   = IdString( "normal_weight_detail2" );
    const IdString PbsProperty::NormalWeightDetail3   = IdString( "normal_weight_detail3" );

    const IdString PbsProperty::DetailWeights     = IdString( "detail_weights" );
    const IdString PbsProperty::DetailOffsets0    = IdString( "detail_offsets0" );
    const IdString PbsProperty::DetailOffsets1    = IdString( "detail_offsets1" );
    const IdString PbsProperty::DetailOffsets2    = IdString( "detail_offsets2" );
    const IdString PbsProperty::DetailOffsets3    = IdString( "detail_offsets3" );

    const IdString PbsProperty::UvDiffuse         = IdString( "uv_diffuse" );
    const IdString PbsProperty::UvNormal          = IdString( "uv_normal" );
    const IdString PbsProperty::UvSpecular        = IdString( "uv_specular" );
    const IdString PbsProperty::UvRoughness       = IdString( "uv_roughness" );
    const IdString PbsProperty::UvDetailWeight    = IdString( "uv_detail_weight" );
    const IdString PbsProperty::UvDetail0         = IdString( "uv_detail0" );
    const IdString PbsProperty::UvDetail1         = IdString( "uv_detail1" );
    const IdString PbsProperty::UvDetail2         = IdString( "uv_detail2" );
    const IdString PbsProperty::UvDetail3         = IdString( "uv_detail3" );
    const IdString PbsProperty::UvDetailNm0       = IdString( "uv_detail_nm0" );
    const IdString PbsProperty::UvDetailNm1       = IdString( "uv_detail_nm1" );
    const IdString PbsProperty::UvDetailNm2       = IdString( "uv_detail_nm2" );
    const IdString PbsProperty::UvDetailNm3       = IdString( "uv_detail_nm3" );
    const IdString PbsProperty::UvEmissive        = IdString( "uv_emissive" );

    const IdString PbsProperty::BlendModeIndex0   = IdString( "blend_mode_idx0" );
    const IdString PbsProperty::BlendModeIndex1   = IdString( "blend_mode_idx1" );
    const IdString PbsProperty::BlendModeIndex2   = IdString( "blend_mode_idx2" );
    const IdString PbsProperty::BlendModeIndex3   = IdString( "blend_mode_idx3" );

    const IdString PbsProperty::DetailMapsDiffuse = IdString( "detail_maps_diffuse" );
    const IdString PbsProperty::DetailMapsNormal  = IdString( "detail_maps_normal" );
    const IdString PbsProperty::FirstValidDetailMapNm= IdString( "first_valid_detail_map_nm" );
    const IdString PbsProperty::EmissiveConstant  = IdString( "emissive_constant" );

    const IdString PbsProperty::Pcf3x3            = IdString( "pcf_3x3" );
    const IdString PbsProperty::Pcf4x4            = IdString( "pcf_4x4" );
    const IdString PbsProperty::PcfIterations     = IdString( "pcf_iterations" );
    const IdString PbsProperty::ExponentialShadowMaps= IdString( "exponential_shadow_maps" );

    const IdString PbsProperty::EnvMapScale       = IdString( "envmap_scale" );
    const IdString PbsProperty::AmbientFixed      = IdString( "ambient_fixed" );
    const IdString PbsProperty::AmbientHemisphere = IdString( "ambient_hemisphere" );
    const IdString PbsProperty::TargetEnvprobeMap = IdString( "target_envprobe_map" );
    const IdString PbsProperty::ParallaxCorrectCubemaps = IdString( "parallax_correct_cubemaps" );
    const IdString PbsProperty::UseParallaxCorrectCubemaps= IdString( "use_parallax_correct_cubemaps" );
    const IdString PbsProperty::IrradianceVolumes = IdString( "irradiance_volumes" );

    const IdString PbsProperty::BrdfDefault       = IdString( "BRDF_Default" );
    const IdString PbsProperty::BrdfCookTorrance  = IdString( "BRDF_CookTorrance" );
    const IdString PbsProperty::BrdfBlinnPhong    = IdString( "BRDF_BlinnPhong" );
    const IdString PbsProperty::FresnelSeparateDiffuse  = IdString( "fresnel_separate_diffuse" );
    const IdString PbsProperty::GgxHeightCorrelated     = IdString( "GGX_height_correlated" );
    const IdString PbsProperty::LegacyMathBrdf          = IdString( "legacy_math_brdf" );
    const IdString PbsProperty::RoughnessIsShininess    = IdString( "roughness_is_shininess" );

    const IdString *PbsProperty::UvSourcePtrs[NUM_PBSM_SOURCES] =
    {
        &PbsProperty::UvDiffuse,
        &PbsProperty::UvNormal,
        &PbsProperty::UvSpecular,
        &PbsProperty::UvRoughness,
        &PbsProperty::UvDetailWeight,
        &PbsProperty::UvDetail0,
        &PbsProperty::UvDetail1,
        &PbsProperty::UvDetail2,
        &PbsProperty::UvDetail3,
        &PbsProperty::UvDetailNm0,
        &PbsProperty::UvDetailNm1,
        &PbsProperty::UvDetailNm2,
        &PbsProperty::UvDetailNm3,
        &PbsProperty::UvEmissive,
    };

    const IdString *PbsProperty::DetailNormalWeights[4] =
    {
        &PbsProperty::NormalWeightDetail0,
        &PbsProperty::NormalWeightDetail1,
        &PbsProperty::NormalWeightDetail2,
        &PbsProperty::NormalWeightDetail3
    };

    const IdString *PbsProperty::DetailOffsetsPtrs[4] =
    {
        &PbsProperty::DetailOffsets0,
        &PbsProperty::DetailOffsets1,
        &PbsProperty::DetailOffsets2,
        &PbsProperty::DetailOffsets3
    };

    const IdString *PbsProperty::BlendModes[4] =
    {
        &PbsProperty::BlendModeIndex0,
        &PbsProperty::BlendModeIndex1,
        &PbsProperty::BlendModeIndex2,
        &PbsProperty::BlendModeIndex3
    };

    extern const String c_pbsBlendModes[];

    HlmsPbs::HlmsPbs( Archive *dataFolder, ArchiveVec *libraryFolders ) :
        HlmsBufferManager( HLMS_PBS, "pbs", dataFolder, libraryFolders ),
        ConstBufferPool( HlmsPbsDatablock::MaterialSizeInGpuAligned,
                         ConstBufferPool::ExtraBufferParams() ),
        mShadowmapSamplerblock( 0 ),
        mShadowmapCmpSamplerblock( 0 ),
        mShadowmapEsmSamplerblock( 0 ),
        mCurrentShadowmapSamplerblock( 0 ),
        mParallaxCorrectedCubemap( 0 ),
        mCurrentPassBuffer( 0 ),
        mGridBuffer( 0 ),
        mGlobalLightListBuffer( 0 ),
        mTexUnitSlotStart( 0 ),
        mPrePassTextures( 0 ),
        mSsrTexture( 0 ),
        mIrradianceVolume( 0 ),
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
        mPlanarReflections( 0 ),
        mPlanarReflectionsSamplerblock( 0 ),
        mHasPlanarReflections( false ),
        mLastBoundPlanarReflection( 0u ),
#endif
        mAreaLightMasksSamplerblock( 0 ),
        mUsingAreaLightMasks( false ),
        mUsingLtcMatrix( false ),
        mDecalsSamplerblock( 0 ),
        mLastBoundPool( 0 ),
        mLastTextureHash( 0 ),
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
        mFineLightMaskGranularity( true ),
#endif
        mDebugPssmSplits( false ),
        mShadowFilter( PCF_3x3 ),
        mEsmK( 600u ),
        mAmbientLightMode( AmbientAuto )
    {
        //Override defaults
        mLightGatheringMode = LightGatherForwardPlus;
    }
    //-----------------------------------------------------------------------------------
    HlmsPbs::~HlmsPbs()
    {
        destroyAllBuffers();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::_changeRenderSystem( RenderSystem *newRs )
    {
        ConstBufferPool::_changeRenderSystem( newRs );
        HlmsBufferManager::_changeRenderSystem( newRs );

        if( newRs )
        {
            HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
            HlmsDatablockMap::const_iterator end  = mDatablocks.end();

            while( itor != end )
            {
                assert( dynamic_cast<HlmsPbsDatablock*>( itor->second.datablock ) );
                HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>( itor->second.datablock );

                requestSlot( datablock->mTextureHash, datablock, false );
                ++itor;
            }

            HlmsSamplerblock samplerblock;
            samplerblock.mU             = TAM_BORDER;
            samplerblock.mV             = TAM_BORDER;
            samplerblock.mW             = TAM_CLAMP;
            samplerblock.mBorderColour  = ColourValue( std::numeric_limits<float>::max(),
                                                       std::numeric_limits<float>::max(),
                                                       std::numeric_limits<float>::max(),
                                                       std::numeric_limits<float>::max() );

            if( mShaderProfile != "hlsl" )
            {
                samplerblock.mMinFilter = FO_POINT;
                samplerblock.mMagFilter = FO_POINT;
                samplerblock.mMipFilter = FO_NONE;

                if( !mShadowmapSamplerblock )
                    mShadowmapSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
            }

            if( !mShadowmapEsmSamplerblock )
            {
                samplerblock.mMinFilter     = FO_LINEAR;
                samplerblock.mMagFilter     = FO_LINEAR;
                samplerblock.mMipFilter     = FO_NONE;

                mShadowmapEsmSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
            }

            samplerblock.mMinFilter     = FO_LINEAR;
            samplerblock.mMagFilter     = FO_LINEAR;
            samplerblock.mMipFilter     = FO_NONE;
            samplerblock.mCompareFunction   = CMPF_LESS_EQUAL;

            if( !mShadowmapCmpSamplerblock )
                mShadowmapCmpSamplerblock = mHlmsManager->getSamplerblock( samplerblock );

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            if( !mPlanarReflectionsSamplerblock )
            {
                samplerblock.mMinFilter     = FO_LINEAR;
                samplerblock.mMagFilter     = FO_LINEAR;
                samplerblock.mMipFilter     = FO_LINEAR;
                samplerblock.mCompareFunction   = NUM_COMPARE_FUNCTIONS;

                samplerblock.mU             = TAM_CLAMP;
                samplerblock.mV             = TAM_CLAMP;
                samplerblock.mW             = TAM_CLAMP;

                mPlanarReflectionsSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
            }
#endif
            if( !mAreaLightMasksSamplerblock )
            {
                samplerblock.mMinFilter     = FO_LINEAR;
                samplerblock.mMagFilter     = FO_LINEAR;
                samplerblock.mMipFilter     = FO_LINEAR;
                samplerblock.mCompareFunction   = NUM_COMPARE_FUNCTIONS;

                samplerblock.mU             = TAM_CLAMP;
                samplerblock.mV             = TAM_CLAMP;
                samplerblock.mW             = TAM_CLAMP;

                mAreaLightMasksSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
            }

            if( !mDecalsSamplerblock )
            {
                samplerblock.mMinFilter     = FO_LINEAR;
                samplerblock.mMagFilter     = FO_LINEAR;
                samplerblock.mMipFilter     = FO_LINEAR;
                samplerblock.mCompareFunction   = NUM_COMPARE_FUNCTIONS;

                samplerblock.mU             = TAM_CLAMP;
                samplerblock.mV             = TAM_CLAMP;
                samplerblock.mW             = TAM_CLAMP;

                mDecalsSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsPbs::createShaderCacheEntry( uint32 renderableHash,
                                                            const HlmsCache &passCache,
                                                            uint32 finalHash,
                                                            const QueuedRenderable &queuedRenderable )
    {
        OgreProfileExhaustive( "HlmsPbs::createShaderCacheEntry" );

        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        if( mShaderProfile == "hlsl" || mShaderProfile == "metal" )
        {
            mListener->shaderCacheEntryCreated( mShaderProfile, retVal, passCache,
                                                mSetProperties, queuedRenderable );
            return retVal; //D3D embeds the texture slots in the shader.
        }

        //Set samplers.
        if( !retVal->pso.pixelShader.isNull() )
        {
            GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();

            int texUnit = 1; //Vertex shader consumes 1 slot with its tbuffer.

            //Forward3D consumes 2 more slots.
            if( mGridBuffer )
            {
                psParams->setNamedConstant( "f3dGrid",      1 );
                psParams->setNamedConstant( "f3dLightList", 2 );
                texUnit += 2;
            }

            if( mPrePassTextures )
            {
                psParams->setNamedConstant( "gBuf_normals",         texUnit++ );
                psParams->setNamedConstant( "gBuf_shadowRoughness", texUnit++ );

                if( mPrePassMsaaDepthTexture )
                    psParams->setNamedConstant( "gBuf_depthTexture", texUnit++ );

                if( mSsrTexture )
                    psParams->setNamedConstant( "ssrTexture", texUnit++ );
            }

            if( mIrradianceVolume && getProperty( HlmsBaseProp::ShadowCaster ) == 0 )
                psParams->setNamedConstant( "irradianceVolume", texUnit++ );

            if( mAreaLightMasks && getProperty( HlmsBaseProp::LightsAreaTexMask ) > 0 )
                psParams->setNamedConstant( "areaLightMasks", texUnit++ );

            if( mLtcMatrixTexture && getProperty( HlmsBaseProp::LightsAreaLtc ) > 0 )
                psParams->setNamedConstant( "ltcMatrix", texUnit++ );

            if( mGridBuffer && getProperty( HlmsBaseProp::EnableDecals ) )
            {
                if( mDecalsTextures[0] )
                    psParams->setNamedConstant( "decalsDiffuseTex", texUnit++ );
                if( mDecalsTextures[1] )
                    psParams->setNamedConstant( "decalsNormalsTex", texUnit++ );
                if( mDecalsTextures[2] && mDecalsTextures[0] != mDecalsTextures[2] )
                    psParams->setNamedConstant( "decalsEmissiveTex", texUnit++ );
            }

            if( !mPreparedPass.shadowMaps.empty() )
            {
                if( getProperty( HlmsBaseProp::NumShadowMapLights ) != 0 )
                {
                    char tmpData[32];
                    LwString texName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );
                    texName = "texShadowMap";
                    const size_t baseTexSize = texName.size();

                    for( size_t i=0; i<mPreparedPass.shadowMaps.size(); ++i )
                    {
                        texName.resize( baseTexSize );
                        texName.a( (uint32)i );   //texShadowMap0
                        psParams->setNamedConstant( texName.c_str(), &texUnit, 1, 1 );
                        ++texUnit;
                    }
                }
                else
                {
                    //No visible lights casting shadow maps.
                    texUnit += mPreparedPass.shadowMaps.size();
                }
            }

            int cubemapTexUnit = 0;
            const int32 parallaxCorrectCubemaps = getProperty( PbsProperty::ParallaxCorrectCubemaps );
            if( parallaxCorrectCubemaps )
                cubemapTexUnit = texUnit++;

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            if( mHasPlanarReflections )
            {
                const bool usesPlanarReflections = getProperty( PbsProperty::UsePlanarReflections ) != 0;
                if( usesPlanarReflections )
                    psParams->setNamedConstant( "planarReflectionTex", texUnit );
                ++texUnit;
            }
#endif
            assert( dynamic_cast<const HlmsPbsDatablock*>( queuedRenderable.renderable->getDatablock() ) );
            const HlmsPbsDatablock *datablock = static_cast<const HlmsPbsDatablock*>(
                                                        queuedRenderable.renderable->getDatablock() );

            int numTextures = getProperty( PbsProperty::NumTextures );
            for( int i=0; i<numTextures; ++i )
            {
                psParams->setNamedConstant( "textureMaps[" + StringConverter::toString( i ) + "]",
                                            texUnit++ );
            }

            const int32 envProbeMap         = getProperty( PbsProperty::EnvProbeMap );
            const int32 targetEnvProbeMap   = getProperty( PbsProperty::TargetEnvprobeMap );
            if( (envProbeMap && envProbeMap != targetEnvProbeMap) || parallaxCorrectCubemaps )
            {
                assert( !datablock->getTexture( PBSM_REFLECTION ).isNull() || parallaxCorrectCubemaps );
                if( !envProbeMap || envProbeMap == targetEnvProbeMap )
                    psParams->setNamedConstant( "texEnvProbeMap", cubemapTexUnit );
                else
                    psParams->setNamedConstant( "texEnvProbeMap", texUnit++ );
            }
        }

        GpuProgramParametersSharedPtr vsParams = retVal->pso.vertexShader->getDefaultParameters();
        vsParams->setNamedConstant( "worldMatBuf", 0 );

        mListener->shaderCacheEntryCreated( mShaderProfile, retVal, passCache,
                                            mSetProperties, queuedRenderable );

        mRenderSystem->_setPipelineStateObject( &retVal->pso );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vsParams, GPV_ALL );
        if( !retVal->pso.pixelShader.isNull() )
        {
            GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();
            mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, GPV_ALL );
        }

        if( !mRenderSystem->getCapabilities()->hasCapability( RSC_CONST_BUFFER_SLOTS_IN_SHADER ) )
        {
            //Setting it to the vertex shader will set it to the PSO actually.
            retVal->pso.vertexShader->setUniformBlockBinding( "PassBuffer", 0 );
            retVal->pso.vertexShader->setUniformBlockBinding( "MaterialBuf", 1 );
            retVal->pso.vertexShader->setUniformBlockBinding( "InstanceBuffer", 2 );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setDetailMapProperties( HlmsPbsDatablock *datablock, PiecesMap *inOutPieces )
    {
        uint32 minNormalMap = 4;
        bool hasDiffuseMaps = false;
        bool hasNormalMaps = false;
        bool anyDetailWeight = false;
        for( size_t i=0; i<4; ++i )
        {
            uint8 blendMode = datablock->mBlendModes[i];

            setDetailTextureProperty( PbsProperty::DetailMapN,   datablock, PBSM_DETAIL0, i );
            setDetailTextureProperty( PbsProperty::DetailMapNmN, datablock, PBSM_DETAIL0_NM, i );

            if( !datablock->getTexture( PBSM_DETAIL0 + i ).isNull() )
            {
                inOutPieces[PixelShader][*PbsProperty::BlendModes[i]] =
                                                "@insertpiece( " + c_pbsBlendModes[blendMode] + ")";
                hasDiffuseMaps = true;
            }

            if( !datablock->getTexture( PBSM_DETAIL0_NM + i ).isNull() )
            {
                minNormalMap = std::min<uint32>( minNormalMap, i );
                hasNormalMaps = true;
            }

            if( datablock->getDetailMapOffsetScale( i ) != Vector4( 0, 0, 1, 1 ) )
                setProperty( *PbsProperty::DetailOffsetsPtrs[i], 1 );

            if( datablock->mDetailWeight[i] != 1.0f &&
                (!datablock->getTexture( PBSM_DETAIL0 + i ).isNull() ||
                 !datablock->getTexture( PBSM_DETAIL0_NM + i ).isNull()) )
            {
                anyDetailWeight = true;
            }
        }

        if( hasDiffuseMaps )
            setProperty( PbsProperty::DetailMapsDiffuse, 4 );

        if( hasNormalMaps )
            setProperty( PbsProperty::DetailMapsNormal, 4 );

        setProperty( PbsProperty::FirstValidDetailMapNm, minNormalMap );

        if( anyDetailWeight )
            setProperty( PbsProperty::DetailWeights, 1 );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setTextureProperty( const char *propertyName, HlmsPbsDatablock *datablock,
                                      PbsTextureTypes texType )
    {
        uint8 idx = datablock->getBakedTextureIdx( texType );
        if( idx != NUM_PBSM_TEXTURE_TYPES )
        {
            char tmpData[64];
            LwString propName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );

            propName = propertyName; //diffuse_map

            //In the template the we subtract the "+1" for the index.
            //We need to increment it now otherwise @property( diffuse_map )
            //can translate to @property( 0 ) which is not what we want.
            setProperty( propertyName, idx + 1 );

            propName.a( "_idx" ); //diffuse_map_idx
            setProperty( propName.c_str(), idx );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setDetailTextureProperty( const char *propertyName, HlmsPbsDatablock *datablock,
                                            PbsTextureTypes baseTexType, uint8 detailIdx )
    {
        const PbsTextureTypes texType = static_cast<PbsTextureTypes>( baseTexType + detailIdx );
        const uint8 idx = datablock->getBakedTextureIdx( texType );
        if( idx != NUM_PBSM_TEXTURE_TYPES )
        {
            char tmpData[64];
            LwString propName = LwString::FromEmptyPointer( tmpData, sizeof(tmpData) );

            propName.a( propertyName, detailIdx ); //detail_map0

            //In the template the we subtract the "+1" for the index.
            //We need to increment it now otherwise @property( diffuse_map )
            //can translate to @property( 0 ) which is not what we want.
            setProperty( propName.c_str(), idx + 1 );

            propName.a( "_idx" ); //detail_map0_idx
            setProperty( propName.c_str(), idx );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces )
    {
        assert( dynamic_cast<HlmsPbsDatablock*>( renderable->getDatablock() ) );
        HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>(
                                                        renderable->getDatablock() );

        const bool metallicWorkflow = datablock->getWorkflow() == HlmsPbsDatablock::MetallicWorkflow;
        const bool fresnelWorkflow = datablock->getWorkflow() ==
                                                        HlmsPbsDatablock::SpecularAsFresnelWorkflow;

        setProperty( PbsProperty::FresnelScalar, datablock->hasSeparateFresnel() || metallicWorkflow );
        setProperty( PbsProperty::FresnelWorkflow, fresnelWorkflow );
        setProperty( PbsProperty::MetallicWorkflow, metallicWorkflow );

        if( datablock->getTwoSidedLighting() )
            setProperty( PbsProperty::TwoSidedLighting, 1 );

        if( datablock->getReceiveShadows() )
            setProperty( PbsProperty::ReceiveShadows, 1 );

        uint32 brdf = datablock->getBrdf();
        if( (brdf & PbsBrdf::BRDF_MASK) == PbsBrdf::Default )
        {
            setProperty( PbsProperty::BrdfDefault, 1 );

            if( !(brdf & PbsBrdf::FLAG_UNCORRELATED) )
                setProperty( PbsProperty::GgxHeightCorrelated, 1 );
        }
        else if( (brdf & PbsBrdf::BRDF_MASK) == PbsBrdf::CookTorrance )
            setProperty( PbsProperty::BrdfCookTorrance, 1 );
        else if( (brdf & PbsBrdf::BRDF_MASK) == PbsBrdf::BlinnPhong )
            setProperty( PbsProperty::BrdfBlinnPhong, 1 );

        if( brdf & PbsBrdf::FLAG_SPERATE_DIFFUSE_FRESNEL )
            setProperty( PbsProperty::FresnelSeparateDiffuse, 1 );

        if( brdf & PbsBrdf::FLAG_LEGACY_MATH )
            setProperty( PbsProperty::LegacyMathBrdf, 1 );
        if( brdf & PbsBrdf::FLAG_FULL_LEGACY )
            setProperty( PbsProperty::RoughnessIsShininess, 1 );

        for( size_t i=0; i<PBSM_REFLECTION; ++i )
        {
            uint8 uvSource = datablock->mUvSource[i];
            setProperty( *PbsProperty::UvSourcePtrs[i], uvSource );

            if( !datablock->getTexture( i ).isNull() &&
                getProperty( *HlmsBaseProp::UvCountPtrs[uvSource] ) < 2 )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Renderable needs at least 2 coordinates in UV set #" +
                             StringConverter::toString( uvSource ) +
                             ". Either change the mesh, or change the UV source settings",
                             "HlmsPbs::calculateHashForPreCreate" );
            }
        }

        int numNormalWeights = 0;
        if( datablock->getNormalMapWeight() != 1.0f && !datablock->getTexture( PBSM_NORMAL ).isNull() )
        {
            setProperty( PbsProperty::NormalWeightTex, 1 );
            ++numNormalWeights;
        }

        {
            size_t validDetailMaps = 0;
            for( size_t i=0; i<4; ++i )
            {
                if( !datablock->getTexture( PBSM_DETAIL0_NM + i ).isNull() )
                {
                    if( datablock->getDetailNormalWeight( i ) != 1.0f )
                    {
                        setProperty( *PbsProperty::DetailNormalWeights[validDetailMaps], 1 );
                        ++numNormalWeights;
                    }

                    ++validDetailMaps;
                }
            }
        }

        setProperty( PbsProperty::NormalWeight, numNormalWeights );

        setDetailMapProperties( datablock, inOutPieces );

        bool envMap = datablock->getBakedTextureIdx( PBSM_REFLECTION ) != NUM_PBSM_TEXTURE_TYPES;

        setProperty( PbsProperty::NumTextures, datablock->mBakedTextures.size() - envMap );

        setTextureProperty( PbsProperty::DiffuseMap,    datablock,  PBSM_DIFFUSE );
        setTextureProperty( PbsProperty::NormalMapTex,  datablock,  PBSM_NORMAL );
        setTextureProperty( PbsProperty::SpecularMap,   datablock,  PBSM_SPECULAR );
        setTextureProperty( PbsProperty::RoughnessMap,  datablock,  PBSM_ROUGHNESS );
        setTextureProperty( PbsProperty::EmissiveMap,   datablock,  PBSM_EMISSIVE );
        setTextureProperty( PbsProperty::EnvProbeMap,   datablock,  PBSM_REFLECTION );
        setTextureProperty( PbsProperty::DetailWeightMap,datablock, PBSM_DETAIL_WEIGHT );

        {
            //Save the name of the cubemap for hazard prevention
            //(don't sample the cubemap and render to it at the same time).
            TexturePtr reflectionTexture = datablock->getTexture( PBSM_REFLECTION );
            if( !reflectionTexture.isNull() )
            {
                //Manual reflection texture
                if( datablock->getCubemapProbe() )
                    setProperty( PbsProperty::UseParallaxCorrectCubemaps, 1 );
                setProperty( PbsProperty::EnvProbeMap, static_cast<int32>(
                             IdString( reflectionTexture->getName() ).mHash ) );
            }
        }

        if( datablock->hasEmissive() )
        {
            if( datablock->getEmissive() != Vector3::ZERO )
                setProperty( PbsProperty::EmissiveConstant, 1 );
        }

        bool usesNormalMap = !datablock->getTexture( PBSM_NORMAL ).isNull();
        for( size_t i=PBSM_DETAIL0_NM; i<=PBSM_DETAIL3_NM; ++i )
            usesNormalMap |= !datablock->getTexture( i ).isNull();

        /*setProperty( HlmsBaseProp::, !datablock->getTexture( PBSM_DETAIL0 ).isNull() );
        setProperty( HlmsBaseProp::DiffuseMap, !datablock->getTexture( PBSM_DETAIL1 ).isNull() );*/
        bool normalMapCanBeSupported = (getProperty( HlmsBaseProp::Normal ) &&
                                        getProperty( HlmsBaseProp::Tangent )) ||
                                        getProperty( HlmsBaseProp::QTangent );

        setProperty( PbsProperty::NormalMap, usesNormalMap );

        if( !normalMapCanBeSupported && usesNormalMap )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Renderable can't use normal maps but datablock wants normal maps. "
                         "Generate Tangents for this mesh to fix the problem or use a "
                         "datablock without normal maps.", "HlmsPbs::calculateHashForPreCreate" );
        }

        if( datablock->mUseAlphaFromTextures && datablock->mBlendblock[0]->mIsTransparent &&
            (getProperty( PbsProperty::DiffuseMap ) || getProperty( PbsProperty::DetailMapsDiffuse )) )
        {
            setProperty( PbsProperty::UseTextureAlpha, 1 );

            //When we don't use the alpha in the texture, transparency still works but
            //only at material level (i.e. what datablock->mTransparency says). The
            //alpha from the texture will be ignored.
            if( datablock->mTransparencyMode == HlmsPbsDatablock::Transparent )
                setProperty( PbsProperty::TransparentMode, 1 );
        }

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
        if( mPlanarReflections && mPlanarReflections->hasPlanarReflections( renderable ) )
        {
            if( !mPlanarReflections->_isUpdatingRenderablesHlms() )
                mPlanarReflections->_notifyRenderableFlushedHlmsDatablock( renderable );
            else
                setProperty( PbsProperty::UsePlanarReflections, 1 );
        }
#endif

        if( mFastShaderBuildHack )
            setProperty( PbsProperty::MaterialsPerBuffer, static_cast<int>( 2 ) );
        else
            setProperty( PbsProperty::MaterialsPerBuffer, static_cast<int>( mSlotsPerPool ) );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces )
    {
        HlmsPbsDatablock *datablock = static_cast<HlmsPbsDatablock*>( renderable->getDatablock() );
        const bool hasAlphaTest = datablock->getAlphaTest() != CMPF_ALWAYS_PASS;

        HlmsPropertyVec::iterator itor = mSetProperties.begin();
        HlmsPropertyVec::iterator end  = mSetProperties.end();

        while( itor != end )
        {
            if( itor->keyName == PbsProperty::FirstValidDetailMapNm )
            {
                itor->value = 0;
                ++itor;
            }
            else if( itor->keyName != PbsProperty::HwGammaRead &&
                     itor->keyName != PbsProperty::UvDiffuse &&
                     itor->keyName != HlmsPsoProp::InputLayoutId &&
                     itor->keyName != HlmsBaseProp::Skeleton &&
                     itor->keyName != HlmsBaseProp::BonesPerVertex &&
                     itor->keyName != HlmsBaseProp::DualParaboloidMapping &&
                     itor->keyName != HlmsBaseProp::AlphaTest &&
                     itor->keyName != HlmsBaseProp::AlphaBlend &&
                     (!hasAlphaTest || !requiredPropertyByAlphaTest( itor->keyName )) )
            {
                itor = mSetProperties.erase( itor );
                end  = mSetProperties.end();
            }
            else
            {
                ++itor;
            }
        }

        if( hasAlphaTest )
        {
            //Keep GLSL happy by not declaring more textures than we'll actually need.
            uint8 numTextures = 0;
            for( int i=0; i<4; ++i )
            {
                if( datablock->mTexToBakedTextureIdx[PBSM_DETAIL0+i] < datablock->mBakedTextures.size() )
                {
                    numTextures = std::max<uint8>(
                                numTextures, datablock->mTexToBakedTextureIdx[PBSM_DETAIL0+i] + 1 );
                }
            }

            if( datablock->mTexToBakedTextureIdx[PBSM_DIFFUSE] < datablock->mBakedTextures.size() )
            {
                numTextures = std::max<uint8>(
                            numTextures, datablock->mTexToBakedTextureIdx[PBSM_DIFFUSE] + 1 );
            }

            setProperty( PbsProperty::NumTextures, numTextures );

            //Set the blending mode as a piece again
            for( size_t i=0; i<4; ++i )
            {
                uint8 blendMode = datablock->mBlendModes[i];
                if( !datablock->getTexture( PBSM_DETAIL0 + i ).isNull() )
                {
                    inOutPieces[PixelShader][*PbsProperty::BlendModes[i]] =
                                                    "@insertpiece( " + c_pbsBlendModes[blendMode] + ")";
                }
            }
        }

        if( mFastShaderBuildHack )
            setProperty( PbsProperty::MaterialsPerBuffer, static_cast<int>( 2 ) );
        else
            setProperty( PbsProperty::MaterialsPerBuffer, static_cast<int>( mSlotsPerPool ) );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::notifyPropertiesMergedPreGenerationStep(void)
    {
        if( getProperty( HlmsBaseProp::DecalsNormals ) )
        {
            //If decals normals are enabled, we need to generate the TBN matrix.
            bool normalMapCanBeSupported = (getProperty( HlmsBaseProp::Normal ) &&
                                            getProperty( HlmsBaseProp::Tangent )) ||
                                            getProperty( HlmsBaseProp::QTangent );

            setProperty( PbsProperty::NormalMap, normalMapCanBeSupported );
        }
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbs::requiredPropertyByAlphaTest( IdString keyName )
    {
        bool retVal =
                keyName == PbsProperty::NumTextures ||
                keyName == PbsProperty::DiffuseMap ||
                keyName == PbsProperty::DetailWeightMap ||
                keyName == PbsProperty::DetailMap0 || keyName == PbsProperty::DetailMap1 ||
                keyName == PbsProperty::DetailMap2 || keyName == PbsProperty::DetailMap3 ||
                keyName == PbsProperty::DetailWeights ||
                keyName == PbsProperty::DetailOffsets0 || keyName == PbsProperty::DetailOffsets1 ||
                keyName == PbsProperty::DetailOffsets2 || keyName == PbsProperty::DetailOffsets3 ||
                keyName == PbsProperty::UvDetailWeight ||
                keyName == PbsProperty::UvDetail0 || keyName == PbsProperty::UvDetail1 ||
                keyName == PbsProperty::UvDetail2 || keyName == PbsProperty::UvDetail3 ||
                keyName == PbsProperty::BlendModeIndex0 || keyName == PbsProperty::BlendModeIndex1 ||
                keyName == PbsProperty::BlendModeIndex2 || keyName == PbsProperty::BlendModeIndex3 ||
                keyName == PbsProperty::DetailMapsDiffuse ||
                keyName == HlmsBaseProp::UvCount;

        for( int i=0; i<8 && !retVal; ++i )
            retVal |= keyName == *HlmsBaseProp::UvCountPtrs[i];

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    static bool SortByTextureLightMaskIdx( const Light *_a, const Light *_b )
    {
        return _a->mTextureLightMaskIdx < _b->mTextureLightMaskIdx;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsPbs::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                        bool dualParaboloid, SceneManager *sceneManager )
    {
        OgreProfileExhaustive( "HlmsPbs::preparePassHash" );

        mSetProperties.clear();

        if( shadowNode && mShadowFilter == ExponentialShadowMaps )
            setProperty( PbsProperty::ExponentialShadowMaps, mEsmK );

        //The properties need to be set before preparePassHash so that
        //they are considered when building the HlmsCache's hash.
        if( shadowNode && !casterPass )
        {
            //Shadow receiving can be improved in performance by using gather sampling.
            //(it's the only feature so far that uses gather)
            const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();
            if( capabilities->hasCapability( RSC_TEXTURE_GATHER ) )
                setProperty( HlmsBaseProp::TexGather, 1 );

            if( mShadowFilter == PCF_3x3 )
            {
                setProperty( PbsProperty::Pcf3x3, 1 );
                setProperty( PbsProperty::PcfIterations, 4 );
            }
            else if( mShadowFilter == PCF_4x4 )
            {
                setProperty( PbsProperty::Pcf4x4, 1 );
                setProperty( PbsProperty::PcfIterations, 9 );
            }
            else if( mShadowFilter == ExponentialShadowMaps )
            {
                //Already set
                //setProperty( PbsProperty::ExponentialShadowMaps, 1 );
            }
            else
            {
                setProperty( PbsProperty::PcfIterations, 1 );
            }

            if( mDebugPssmSplits )
            {
                int32 numPssmSplits = 0;
                const vector<Real>::type *pssmSplits = shadowNode->getPssmSplits( 0 );
                if( pssmSplits )
                {
                    numPssmSplits = static_cast<int32>( pssmSplits->size() - 1 );
                    if( numPssmSplits > 0 )
                        setProperty( PbsProperty::DebugPssmSplits, 1 );
                }
            }
        }

        mTargetEnvMap.setNull();

        mPrePassTextures = sceneManager->getCurrentPrePassTextures();
        assert( !mPrePassTextures || mPrePassTextures->size() == 2u );

        mPrePassMsaaDepthTexture.reset();
        if( mPrePassTextures && (*mPrePassTextures)[0]->getFSAA() > 1u )
        {
            const TextureVec *msaaDepthTexture = sceneManager->getCurrentPrePassDepthTexture();
            if( msaaDepthTexture )
                mPrePassMsaaDepthTexture = (*msaaDepthTexture)[0];
            assert( (!mPrePassTextures ||
                    (mPrePassMsaaDepthTexture &&
                     mPrePassMsaaDepthTexture->getFSAA() == (*mPrePassTextures)[0]->getFSAA())) &&
                    "Prepass + MSAA must specify an MSAA depth texture" );
        }

        mSsrTexture = sceneManager->getCurrentSsrTexture();
        assert( !(!mPrePassTextures && mSsrTexture) && "Using SSR *requires* to be in prepass mode" );
        assert( !mSsrTexture || mSsrTexture->size() == 1u );

        AmbientLightMode ambientMode = mAmbientLightMode;
        ColourValue upperHemisphere = sceneManager->getAmbientLightUpperHemisphere();
        ColourValue lowerHemisphere = sceneManager->getAmbientLightLowerHemisphere();

        const float envMapScale = upperHemisphere.a;
        //Ignore alpha channel
        upperHemisphere.a = lowerHemisphere.a = 1.0;

        if( !casterPass )
        {
            if( mAmbientLightMode == AmbientAuto )
            {
                if( upperHemisphere == lowerHemisphere )
                {
                    if( upperHemisphere == ColourValue::Black )
                        ambientMode = AmbientNone;
                    else
                        ambientMode = AmbientFixed;
                }
                else
                {
                    ambientMode = AmbientHemisphere;
                }
            }

            if( ambientMode == AmbientFixed )
                setProperty( PbsProperty::AmbientFixed, 1 );
            if( ambientMode == AmbientHemisphere )
                setProperty( PbsProperty::AmbientHemisphere, 1 );

            if( envMapScale != 1.0f )
                setProperty( PbsProperty::EnvMapScale, 1 );

            //Save cubemap's name so that we never try to render & sample to/from it at the same time
            const CompositorTexture &compoTarget = sceneManager->getCompositorTarget();
            if( !compoTarget.textures->empty() )
            {
                const TexturePtr &firstTargetTex = (*compoTarget.textures)[0];
                if( firstTargetTex->getTextureType() == TEX_TYPE_CUBE_MAP )
                {
                    setProperty( PbsProperty::TargetEnvprobeMap,
                                 static_cast<int32>( IdString(firstTargetTex->getName()).mHash ) );
                    mTargetEnvMap = firstTargetTex;
                }
            }

            if( mParallaxCorrectedCubemap )
                setProperty( PbsProperty::ParallaxCorrectCubemaps, 1 );

            if( mIrradianceVolume )
                setProperty( PbsProperty::IrradianceVolumes, 1 );

            if( mAreaLightMasks )
            {
                const size_t numComponents =
                        PixelUtil::getComponentCount( mAreaLightMasks->getFormat() );
                if( numComponents > 2u )
                    setProperty( HlmsBaseProp::LightsAreaTexColour, 1 );
            }

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            mHasPlanarReflections = false;
            mLastBoundPlanarReflection = 0u;
            if( mPlanarReflections &&
                mPlanarReflections->cameraMatches( sceneManager->getCameraInProgress() ) )
            {
                mHasPlanarReflections = true;
                setProperty( PbsProperty::HasPlanarReflections,
                             mPlanarReflections->getMaxActiveActors() );
            }
#endif

#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
            if( mFineLightMaskGranularity )
                setProperty( HlmsBaseProp::FineLightMask, 1 );
#endif
        }

        if( mOptimizationStrategy == LowerGpuOverhead )
            setProperty( PbsProperty::LowerGpuOverhead, 1 );

        HlmsCache retVal = Hlms::preparePassHashBase( shadowNode, casterPass,
                                                      dualParaboloid, sceneManager );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();

        const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();
        setProperty( PbsProperty::HwGammaRead, capabilities->hasCapability( RSC_HW_GAMMA ) );
//        setProperty( PbsProperty::HwGammaWrite, capabilities->hasCapability( RSC_HW_GAMMA ) &&
//                                                        renderTarget->isHardwareGammaEnabled() );
        setProperty( PbsProperty::HwGammaWrite, 1 );
        setProperty( PbsProperty::SignedIntTex, capabilities->hasCapability(
                                                            RSC_TEXTURE_SIGNED_INT ) );
        retVal.setProperties = mSetProperties;

        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        Matrix4 projectionMatrix = camera->getProjectionMatrixWithRSDepth();

        if( renderTarget->requiresTextureFlipping() )
        {
            projectionMatrix[1][0] = -projectionMatrix[1][0];
            projectionMatrix[1][1] = -projectionMatrix[1][1];
            projectionMatrix[1][2] = -projectionMatrix[1][2];
            projectionMatrix[1][3] = -projectionMatrix[1][3];
        }

        int32 numLights             = getProperty( HlmsBaseProp::LightsSpot );
        int32 numDirectionalLights  = getProperty( HlmsBaseProp::LightsDirNonCaster );
        int32 numShadowMapLights    = getProperty( HlmsBaseProp::NumShadowMapLights );
        int32 numPssmSplits         = getProperty( HlmsBaseProp::PssmSplits );
        int32 numAreaApproxLights   = getProperty( HlmsBaseProp::LightsAreaApprox );
        int32 numAreaLtcLights      = getProperty( HlmsBaseProp::LightsAreaLtc );
        const size_t realNumAreaApproxLights = mRealNumAreaApproxLights;
        const size_t realNumAreaLtcLights = mRealNumAreaLtcLights;

        bool isPssmBlend = getProperty( HlmsBaseProp::PssmBlend ) != 0;
        bool isPssmFade = getProperty( HlmsBaseProp::PssmFade ) != 0;

        bool isShadowCastingPointLight = false;

        //mat4 viewProj;
        size_t mapSize = 16 * 4;

        mGridBuffer             = 0;
        mGlobalLightListBuffer  = 0;

        mDecalsTextures[0].setNull();
        mDecalsTextures[1].setNull();
        mDecalsTextures[2].setNull();

        if( !casterPass )
        {
            ForwardPlusBase *forwardPlus = sceneManager->_getActivePassForwardPlus();
            if( forwardPlus )
            {
                mapSize += forwardPlus->getConstBufferSize();
                mGridBuffer             = forwardPlus->getGridBuffer( camera );
                mGlobalLightListBuffer  = forwardPlus->getGlobalLightListBuffer( camera );

                if( forwardPlus->getDecalsEnabled() )
                {
                    const PrePassMode prePassMode = sceneManager->getCurrentPrePassMode();
                    if( prePassMode != PrePassCreate )
                        mDecalsTextures[0] = sceneManager->getDecalsDiffuse();
                    if( prePassMode != PrePassUse )
                        mDecalsTextures[1] = sceneManager->getDecalsNormals();
                    if( prePassMode != PrePassCreate )
                        mDecalsTextures[2] = sceneManager->getDecalsEmissive();
                }
            }

            if( mParallaxCorrectedCubemap )
            {
                mParallaxCorrectedCubemap->_notifyPreparePassHash( viewMatrix );
                mapSize += mParallaxCorrectedCubemap->getConstBufferSize();
            }

            //mat4 view + mat4 shadowRcv[numShadowMapLights].texViewProj +
            //              vec2 shadowRcv[numShadowMapLights].shadowDepthRange +
            //              vec2 padding +
            //              vec4 shadowRcv[numShadowMapLights].invShadowMapSize +
            //mat3 invViewMatCubemap (upgraded to three vec4)
            mapSize += ( 16 + (16 + 2 + 2 + 4) * numShadowMapLights + 4 * 3 ) * 4;

            //vec4 shadowRcv[numShadowMapLights].texViewZRow
            if( mShadowFilter == ExponentialShadowMaps )
                mapSize += (4 * 4) * numShadowMapLights;

            //float windowHeight + padding
            if( mPrePassTextures )
                mapSize += 4 * 4;

            //vec3 ambientUpperHemi + float envMapScale
            if( ambientMode == AmbientFixed || ambientMode == AmbientHemisphere || envMapScale != 1.0f )
                mapSize += 4 * 4;

            //vec3 ambientLowerHemi + padding + vec3 ambientHemisphereDir + padding
            if( ambientMode == AmbientHemisphere )
                mapSize += 8 * 4;

            //vec3 irradianceOrigin + float maxPower +
            //vec3 irradianceSize + float invHeight + mat4 invView
            if( mIrradianceVolume )
                mapSize += (4 + 4 + 4*4) * 4;

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            if( mHasPlanarReflections )
                mapSize += mPlanarReflections->getConstBufferSize();
#endif
            //float pssmSplitPoints N times.
            mapSize += numPssmSplits * 4;

            if( isPssmBlend )
            {
                //float pssmBlendPoints N-1 times.
                mapSize += ( numPssmSplits - 1 ) * 4;
            }
            if( isPssmFade )
            {
                //float pssmFadePoint.
                mapSize += 4;
            }

            mapSize = alignToNextMultiple( mapSize, 16 );

            if( numShadowMapLights > 0 )
            {
                //Six variables * 4 (padded vec3) * 4 (bytes) * numLights
                mapSize += ( 6 * 4 * 4 ) * numLights;
            }
            else
            {
                //Three variables * 4 (padded vec3) * 4 (bytes) * numDirectionalLights
                mapSize += ( 3 * 4 * 4 ) * numDirectionalLights;
            }

            mapSize += ( 7 * 4 * 4 ) * numAreaApproxLights;
            mapSize += ( 7 * 4 * 4 ) * numAreaLtcLights;
        }
        else
        {
            isShadowCastingPointLight = getProperty( HlmsBaseProp::ShadowCasterPoint ) != 0;
            //vec4 cameraPosWS
            if( isShadowCastingPointLight )
                mapSize += 4 * 4;
            //vec4 viewZRow
            if( mShadowFilter == ExponentialShadowMaps )
                mapSize += 4 * 4;
            //vec4 depthRange
            mapSize += (2 + 2) * 4;
        }

        const bool isCameraReflected = camera->isReflected();
        //vec4 clipPlane0
        if( isCameraReflected )
            mapSize += 4 * 4;

        mapSize += mListener->getPassBufferSize( shadowNode, casterPass, dualParaboloid,
                                                 sceneManager );

        //Arbitrary 16kb (minimum supported by GL), should be enough.
        const size_t maxBufferSize = 16 * 1024;

        assert( mapSize <= maxBufferSize );

        if( mCurrentPassBuffer >= mPassBuffers.size() )
        {
            mPassBuffers.push_back( mVaoManager->createConstBuffer( maxBufferSize, BT_DYNAMIC_PERSISTENT,
                                                                    0, false ) );
        }

        ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer++];
        float *passBufferPtr = reinterpret_cast<float*>( passBuffer->map( 0, mapSize ) );

#ifndef NDEBUG
        const float *startupPtr = passBufferPtr;
#endif

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------

        //mat4 viewProj;
        Matrix4 viewProjMatrix = projectionMatrix * viewMatrix;
        for( size_t i=0; i<16; ++i )
            *passBufferPtr++ = (float)viewProjMatrix[0][i];

        //vec4 clipPlane0
        if( isCameraReflected )
        {
            const Plane &reflPlane = camera->getReflectionPlane();
            *passBufferPtr++ = (float)reflPlane.normal.x;
            *passBufferPtr++ = (float)reflPlane.normal.y;
            *passBufferPtr++ = (float)reflPlane.normal.z;
            *passBufferPtr++ = (float)reflPlane.d;
        }

        //vec4 cameraPosWS;
        if( isShadowCastingPointLight )
        {
            const Vector3 &camPos = camera->getDerivedPosition();
            *passBufferPtr++ = (float)camPos.x;
            *passBufferPtr++ = (float)camPos.y;
            *passBufferPtr++ = (float)camPos.z;
            *passBufferPtr++ = 1.0f;
        }

        mPreparedPass.viewMatrix        = viewMatrix;

        mPreparedPass.shadowMaps.clear();

        if( !casterPass )
        {
            //mat4 view;
            for( size_t i=0; i<16; ++i )
                *passBufferPtr++ = (float)viewMatrix[0][i];

            size_t shadowMapTexIdx = 0;
            const TextureVec &contiguousShadowMapTex = shadowNode->getContiguousShadowMapTex();

            for( int32 i=0; i<numShadowMapLights; ++i )
            {
                //Skip inactive lights (e.g. no directional lights are available
                //and there's a shadow map that only accepts dir lights)
                while( !shadowNode->isShadowMapIdxActive( shadowMapTexIdx ) )
                    ++shadowMapTexIdx;

                //mat4 shadowRcv[numShadowMapLights].texViewProj
                Matrix4 viewProjTex = shadowNode->getViewProjectionMatrix( shadowMapTexIdx );
                for( size_t j=0; j<16; ++j )
                    *passBufferPtr++ = (float)viewProjTex[0][j];

                //vec4 texViewZRow;
                if( mShadowFilter == ExponentialShadowMaps )
                {
                    const Matrix4 &viewTex = shadowNode->getViewMatrix( shadowMapTexIdx );
                    *passBufferPtr++ = viewTex[2][0];
                    *passBufferPtr++ = viewTex[2][1];
                    *passBufferPtr++ = viewTex[2][2];
                    *passBufferPtr++ = viewTex[2][3];
                }

                //vec2 shadowRcv[numShadowMapLights].shadowDepthRange
                Real fNear, fFar;
                shadowNode->getMinMaxDepthRange( shadowMapTexIdx, fNear, fFar );
                const Real depthRange = fFar - fNear;
                *passBufferPtr++ = fNear;
                *passBufferPtr++ = 1.0f / depthRange;
                ++passBufferPtr; //Padding
                ++passBufferPtr; //Padding


                //vec2 shadowRcv[numShadowMapLights].invShadowMapSize
                size_t shadowMapTexContigIdx =
                        shadowNode->getIndexToContiguousShadowMapTex( (size_t)shadowMapTexIdx );
                uint32 texWidth  = contiguousShadowMapTex[shadowMapTexContigIdx]->getWidth();
                uint32 texHeight = contiguousShadowMapTex[shadowMapTexContigIdx]->getHeight();
                *passBufferPtr++ = 1.0f / texWidth;
                *passBufferPtr++ = 1.0f / texHeight;
                *passBufferPtr++ = static_cast<float>( texWidth );
                *passBufferPtr++ = static_cast<float>( texHeight );

                ++shadowMapTexIdx;
            }

            //---------------------------------------------------------------------------
            //                          ---- PIXEL SHADER ----
            //---------------------------------------------------------------------------

            Matrix3 viewMatrix3, invViewMatrixCubemap;
            viewMatrix.extract3x3Matrix( viewMatrix3 );
            //Cubemaps are left-handed.
            invViewMatrixCubemap = viewMatrix3;
            invViewMatrixCubemap[0][2] = -invViewMatrixCubemap[0][2];
            invViewMatrixCubemap[1][2] = -invViewMatrixCubemap[1][2];
            invViewMatrixCubemap[2][2] = -invViewMatrixCubemap[2][2];
            invViewMatrixCubemap = invViewMatrixCubemap.Inverse();

            //mat3 invViewMatCubemap
            for( size_t i=0; i<9; ++i )
            {
#ifdef OGRE_GLES2_WORKAROUND_2
                Matrix3 xRot( 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, -1.0f,
                              0.0f, 1.0f, 0.0f );
                xRot = xRot * invViewMatrixCubemap;
                *passBufferPtr++ = (float)xRot[0][i];
#else
                *passBufferPtr++ = (float)invViewMatrixCubemap[0][i];
#endif

                //Alignment: each row/column is one vec4, despite being 3x3
                if( !( (i+1) % 3 ) )
                    ++passBufferPtr;
            }

            if( mPrePassTextures )
            {
                //vec4 windowHeight
                const float windowHeight = renderTarget->getHeight();
                *passBufferPtr++ = windowHeight;
                *passBufferPtr++ = windowHeight;
                *passBufferPtr++ = windowHeight;
                *passBufferPtr++ = windowHeight;
            }

            //vec3 ambientUpperHemi + padding
            if( ambientMode == AmbientFixed || ambientMode == AmbientHemisphere || envMapScale != 1.0f )
            {
                *passBufferPtr++ = static_cast<float>( upperHemisphere.r );
                *passBufferPtr++ = static_cast<float>( upperHemisphere.g );
                *passBufferPtr++ = static_cast<float>( upperHemisphere.b );
                *passBufferPtr++ = envMapScale;
            }

            //vec3 ambientLowerHemi + padding + vec3 ambientHemisphereDir + padding
            if( ambientMode == AmbientHemisphere )
            {
                *passBufferPtr++ = static_cast<float>( lowerHemisphere.r );
                *passBufferPtr++ = static_cast<float>( lowerHemisphere.g );
                *passBufferPtr++ = static_cast<float>( lowerHemisphere.b );
                *passBufferPtr++ = 1.0f;

                Vector3 hemisphereDir = viewMatrix3 * sceneManager->getAmbientLightHemisphereDir();
                hemisphereDir.normalise();
                *passBufferPtr++ = static_cast<float>( hemisphereDir.x );
                *passBufferPtr++ = static_cast<float>( hemisphereDir.y );
                *passBufferPtr++ = static_cast<float>( hemisphereDir.z );
                *passBufferPtr++ = 1.0f;
            }

            if( mIrradianceVolume )
            {
                const Vector3 irradianceCellSize = mIrradianceVolume->getIrradianceCellSize();
                const Vector3 irradianceVolumeOrigin = mIrradianceVolume->getIrradianceOrigin() /
                                                       irradianceCellSize;
                const float fTexWidth = static_cast<float>(
                            mIrradianceVolume->getIrradianceVolumeTexture()->getWidth() );
                const float fTexDepth = static_cast<float>(
                            mIrradianceVolume->getIrradianceVolumeTexture()->getDepth() );

                *passBufferPtr++ = static_cast<float>( irradianceVolumeOrigin.x ) / fTexWidth;
                *passBufferPtr++ = static_cast<float>( irradianceVolumeOrigin.y );
                *passBufferPtr++ = static_cast<float>( irradianceVolumeOrigin.z ) / fTexDepth;
                *passBufferPtr++ = mIrradianceVolume->getIrradianceMaxPower() *
                                   mIrradianceVolume->getPowerScale();

                const float fTexHeight = static_cast<float>(
                            mIrradianceVolume->getIrradianceVolumeTexture()->getHeight() );

                *passBufferPtr++ = 1.0f / (fTexWidth * irradianceCellSize.x);
                *passBufferPtr++ = 1.0f / irradianceCellSize.y;
                *passBufferPtr++ = 1.0f / (fTexDepth * irradianceCellSize.z);
                *passBufferPtr++ = 1.0f / fTexHeight;

                //mat4 invView;
                Matrix4 invViewMatrix = viewMatrix.inverse();
                for( size_t i=0; i<16; ++i )
                    *passBufferPtr++ = (float)invViewMatrix[0][i];
            }

            //float pssmSplitPoints
            for( int32 i=0; i<numPssmSplits; ++i )
                *passBufferPtr++ = (*shadowNode->getPssmSplits(0))[i+1];

            int32 numPssmBlendsAndFade = 0;
            if( isPssmBlend )
            {
                numPssmBlendsAndFade += numPssmSplits - 1;

                //float pssmBlendPoints
                for( int32 i=0; i<numPssmSplits-1; ++i )
                    *passBufferPtr++ = (*shadowNode->getPssmBlends(0))[i];
            }
            if( isPssmFade )
            {
                numPssmBlendsAndFade += 1;

                //float pssmFadePoint
                *passBufferPtr++ = *shadowNode->getPssmFade(0);
            }

            passBufferPtr += alignToNextMultiple( numPssmSplits + numPssmBlendsAndFade, 4 ) -
                             ( numPssmSplits + numPssmBlendsAndFade );

            if( numShadowMapLights > 0 )
            {
                //All directional lights (caster and non-caster) are sent.
                //Then non-directional shadow-casting shadow lights are sent.
                size_t shadowLightIdx = 0;
                size_t nonShadowLightIdx = 0;
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                const LightClosestArray &lights = shadowNode->getShadowCastingLights();

                const CompositorShadowNode::LightsBitSet &affectedLights =
                        shadowNode->getAffectedLightsBitSet();

                int32 shadowCastingDirLights = getProperty( HlmsBaseProp::LightsDirectional );

                for( int32 i=0; i<numLights; ++i )
                {
                    Light const *light = 0;

                    if( i >= shadowCastingDirLights && i < numDirectionalLights )
                    {
                        while( affectedLights[nonShadowLightIdx] )
                            ++nonShadowLightIdx;

                        light = globalLightList.lights[nonShadowLightIdx++];

                        assert( light->getType() == Light::LT_DIRECTIONAL );
                    }
                    else
                    {
                        //Skip inactive lights (e.g. no directional lights are available
                        //and there's a shadow map that only accepts dir lights)
                        while( !lights[shadowLightIdx].light )
                            ++shadowLightIdx;
                        light = lights[shadowLightIdx++].light;
                    }

                    Vector4 lightPos4 = light->getAs4DVector();
                    Vector3 lightPos;

                    if( light->getType() == Light::LT_DIRECTIONAL )
                        lightPos = viewMatrix3 * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );
                    else
                        lightPos = viewMatrix * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );

                    //vec3 lights[numLights].position
                    *passBufferPtr++ = lightPos.x;
                    *passBufferPtr++ = lightPos.y;
                    *passBufferPtr++ = lightPos.z;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
                    *reinterpret_cast<uint32 * RESTRICT_ALIAS>( passBufferPtr ) = light->getLightMask();
#endif
                    ++passBufferPtr;

                    //vec3 lights[numLights].diffuse
                    ColourValue colour = light->getDiffuseColour() *
                                         light->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;

                    //vec3 lights[numLights].specular
                    colour = light->getSpecularColour() * light->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;

                    //vec3 lights[numLights].attenuation;
                    Real attenRange     = light->getAttenuationRange();
                    Real attenLinear    = light->getAttenuationLinear();
                    Real attenQuadratic = light->getAttenuationQuadric();
                    *passBufferPtr++ = attenRange;
                    *passBufferPtr++ = attenLinear;
                    *passBufferPtr++ = attenQuadratic;
                    ++passBufferPtr;

                    const Vector2 rectSize = light->getDerivedRectSize();

                    //vec4 lights[numLights].spotDirection;
                    Vector3 spotDir = viewMatrix3 * light->getDerivedDirection();
                    *passBufferPtr++ = spotDir.x;
                    *passBufferPtr++ = spotDir.y;
                    *passBufferPtr++ = spotDir.z;
                    *passBufferPtr++ = 1.0f / rectSize.x;

                    //vec4 lights[numLights].spotParams;
                    if( light->getType() != Light::LT_AREA_APPROX )
                    {
                        Radian innerAngle = light->getSpotlightInnerAngle();
                        Radian outerAngle = light->getSpotlightOuterAngle();
                        *passBufferPtr++ = 1.0f / ( cosf( innerAngle.valueRadians() * 0.5f ) -
                                                    cosf( outerAngle.valueRadians() * 0.5f ) );
                        *passBufferPtr++ = cosf( outerAngle.valueRadians() * 0.5f );
                        *passBufferPtr++ = light->getSpotlightFalloff();
                    }
                    else
                    {
                        Quaternion qRot = light->getParentNode()->_getDerivedOrientation();
                        Vector3 xAxis = viewMatrix3 * qRot.xAxis();
                        *passBufferPtr++ = xAxis.x;
                        *passBufferPtr++ = xAxis.y;
                        *passBufferPtr++ = xAxis.z;
                    }
                    *passBufferPtr++ = 1.0f / rectSize.y;

                }
            }
            else
            {
                //No shadow maps, only send directional lights
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();

                for( int32 i=0; i<numDirectionalLights; ++i )
                {
                    assert( globalLightList.lights[i]->getType() == Light::LT_DIRECTIONAL );

                    Vector4 lightPos4 = globalLightList.lights[i]->getAs4DVector();
                    Vector3 lightPos = viewMatrix3 * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );

                    //vec3 lights[numLights].position
                    *passBufferPtr++ = lightPos.x;
                    *passBufferPtr++ = lightPos.y;
                    *passBufferPtr++ = lightPos.z;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
                    *reinterpret_cast<uint32 * RESTRICT_ALIAS>( passBufferPtr ) =
                            globalLightList.lights[i]->getLightMask();
#endif
                    ++passBufferPtr;

                    //vec3 lights[numLights].diffuse
                    ColourValue colour = globalLightList.lights[i]->getDiffuseColour() *
                                         globalLightList.lights[i]->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;

                    //vec3 lights[numLights].specular
                    colour = globalLightList.lights[i]->getSpecularColour() * globalLightList.lights[i]->getPowerScale();
                    *passBufferPtr++ = colour.r;
                    *passBufferPtr++ = colour.g;
                    *passBufferPtr++ = colour.b;
                    ++passBufferPtr;
                }
            }

            float areaLightNumMipmaps = 0.0f;
            float areaLightNumMipmapsSpecFactor = 0.0f;

            if( mAreaLightMasks )
            {
                //Roughness minimum value is 0.02, so we need to map
                //[0.02; 1.0] -> [0; 1] in the pixel shader that's why we divide by 0.98.
                //The 2.0 is just arbitrary (it looks good)
                areaLightNumMipmaps = mAreaLightMasks->getNumMipmaps();
                areaLightNumMipmapsSpecFactor = areaLightNumMipmaps * 2.0f / 0.98f;
            }

            //Send area lights. We need them sorted so textured ones
            //come first, as that what our shader expects
            mAreaLights.reserve( numAreaApproxLights );
            mAreaLights.clear();
            const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
            size_t areaLightNumber = 0;
            for( size_t idx = mAreaLightsGlobalLightListStart;
                 idx<globalLightList.lights.size() && areaLightNumber < realNumAreaApproxLights; ++idx )
            {                
                if( globalLightList.lights[idx]->getType() == Light::LT_AREA_APPROX )
                {
                    mAreaLights.push_back( globalLightList.lights[idx] );
                    ++areaLightNumber;
                }
            }

            std::sort( mAreaLights.begin(), mAreaLights.end(), SortByTextureLightMaskIdx );

            for( size_t i=0; i<realNumAreaApproxLights; ++i )
            {
                Light const *light = mAreaLights[i];

                Vector4 lightPos4 = light->getAs4DVector();
                Vector3 lightPos = viewMatrix * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );

                //vec3 areaApproxLights[numLights].position
                *passBufferPtr++ = lightPos.x;
                *passBufferPtr++ = lightPos.y;
                *passBufferPtr++ = lightPos.z;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
                *reinterpret_cast<uint32 * RESTRICT_ALIAS>( passBufferPtr ) = light->getLightMask();
#endif
                ++passBufferPtr;

                //vec3 areaApproxLights[numLights].diffuse
                ColourValue colour = light->getDiffuseColour() *
                                     light->getPowerScale();
                *passBufferPtr++ = colour.r;
                *passBufferPtr++ = colour.g;
                *passBufferPtr++ = colour.b;
                *passBufferPtr++ = areaLightNumMipmaps *
                                   (light->mTexLightMaskDiffuseMipStart / 65535.0f);

                //vec3 areaApproxLights[numLights].specular
                colour = light->getSpecularColour() * light->getPowerScale();
                *passBufferPtr++ = colour.r;
                *passBufferPtr++ = colour.g;
                *passBufferPtr++ = colour.b;
                *passBufferPtr++ = areaLightNumMipmapsSpecFactor;

                //vec4 areaApproxLights[numLights].attenuation;
                Real attenRange     = light->getAttenuationRange();
                Real attenLinear    = light->getAttenuationLinear();
                Real attenQuadratic = light->getAttenuationQuadric();
                *passBufferPtr++ = attenRange;
                *passBufferPtr++ = attenLinear;
                *passBufferPtr++ = attenQuadratic;
                *passBufferPtr++ = static_cast<float>( light->mTextureLightMaskIdx );

                const Vector2 rectSize = light->getDerivedRectSize();

                //vec4 areaApproxLights[numLights].direction;
                Vector3 areaDir = viewMatrix3 * light->getDerivedDirection();
                *passBufferPtr++ = areaDir.x;
                *passBufferPtr++ = areaDir.y;
                *passBufferPtr++ = areaDir.z;
                *passBufferPtr++ = 1.0f / rectSize.x;

                //vec4 areaApproxLights[numLights].tangent;
                Quaternion qRot = light->getParentNode()->_getDerivedOrientation();
                Vector3 xAxis = viewMatrix3 * qRot.xAxis();
                *passBufferPtr++ = xAxis.x;
                *passBufferPtr++ = xAxis.y;
                *passBufferPtr++ = xAxis.z;
                *passBufferPtr++ = 1.0f / rectSize.y;

                //vec4 doubleSided;
                *passBufferPtr++ = light->getDoubleSided() ? 1.0f : 0.0f;
                *passBufferPtr++ = 0.0f;
                *passBufferPtr++ = 0.0f;
                *passBufferPtr++ = 0.0f;
            }

            for( int32 i=realNumAreaApproxLights; i<numAreaApproxLights; ++i )
            {
                //vec3 areaApproxLights[numLights].position
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;

                //vec3 areaApproxLights[numLights].diffuse
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 1000.0f;

                //vec3 areaApproxLights[numLights].specular
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 1000.0f;

                //vec4 areaApproxLights[numLights].attenuation;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 1;
                *passBufferPtr++ = 1;
                *passBufferPtr++ = 0;

                //vec4 areaApproxLights[numLights].direction;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 1.0f;

                //vec4 areaApproxLights[numLights].tangent;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 0;
                *passBufferPtr++ = 1.0f;

                //vec4 doubleSided;
                *passBufferPtr++ = 0.0f;
                *passBufferPtr++ = 0.0f;
                *passBufferPtr++ = 0.0f;
                *passBufferPtr++ = 0.0f;
            }

            mAreaLights.reserve( numAreaLtcLights );
            mAreaLights.clear();
            areaLightNumber = 0;
            for( size_t idx = mAreaLightsGlobalLightListStart;
                 idx<globalLightList.lights.size() && areaLightNumber < realNumAreaLtcLights; ++idx )
            {
                if( globalLightList.lights[idx]->getType() == Light::LT_AREA_LTC )
                {
                    mAreaLights.push_back( globalLightList.lights[idx] );
                    ++areaLightNumber;
                }
            }

            //std::sort( mAreaLights.begin(), mAreaLights.end(), SortByTextureLightMaskIdx );

            for( size_t i=0; i<realNumAreaLtcLights; ++i )
            {
                Light const *light = mAreaLights[i];

                Vector4 lightPos4 = light->getAs4DVector();
                Vector3 lightPos = viewMatrix * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );

                //vec3 areaLtcLights[numLights].position
                *passBufferPtr++ = lightPos.x;
                *passBufferPtr++ = lightPos.y;
                *passBufferPtr++ = lightPos.z;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
                *reinterpret_cast<uint32 * RESTRICT_ALIAS>( passBufferPtr ) = light->getLightMask();
#endif
                ++passBufferPtr;

                const Real attenRange   = light->getAttenuationRange();

                //vec3 areaLtcLights[numLights].diffuse
                ColourValue colour = light->getDiffuseColour() *
                                     light->getPowerScale();
                *passBufferPtr++ = colour.r;
                *passBufferPtr++ = colour.g;
                *passBufferPtr++ = colour.b;
                *passBufferPtr++ = attenRange;

                //vec3 areaLtcLights[numLights].specular
                colour = light->getSpecularColour() * light->getPowerScale();
                *passBufferPtr++ = colour.r;
                *passBufferPtr++ = colour.g;
                *passBufferPtr++ = colour.b;
                *passBufferPtr++ = light->getDoubleSided() ? 1.0f : 0.0f;

                const Vector2 rectSize = light->getDerivedRectSize() * 0.5f;
                Quaternion qRot = light->getParentNode()->_getDerivedOrientation();
                Vector3 xAxis = (viewMatrix3 * qRot.xAxis()) * rectSize.x;
                Vector3 yAxis = (viewMatrix3 * qRot.yAxis()) * rectSize.y;

                Vector3 rectPoints[4];
                //vec4 areaLtcLights[numLights].points[4];
                rectPoints[0] = lightPos - xAxis - yAxis;
                rectPoints[1] = lightPos + xAxis - yAxis;
                rectPoints[2] = lightPos + xAxis + yAxis;
                rectPoints[3] = lightPos - xAxis + yAxis;

                for( size_t j=0; j<4u; ++j )
                {
                    *passBufferPtr++ = rectPoints[j].x;
                    *passBufferPtr++ = rectPoints[j].y;
                    *passBufferPtr++ = rectPoints[j].z;
                    *passBufferPtr++ = 1.0f;
                }
            }

            memset( passBufferPtr, 0,
                    (numAreaLtcLights - realNumAreaLtcLights) * sizeof(float) * 4u * 7u );
            passBufferPtr += (numAreaLtcLights - realNumAreaLtcLights) * 4u * 7u;

            if( shadowNode )
            {
                mPreparedPass.shadowMaps.reserve( contiguousShadowMapTex.size() );

                TextureVec::const_iterator itShadowMap = contiguousShadowMapTex.begin();
                TextureVec::const_iterator enShadowMap = contiguousShadowMapTex.end();

                while( itShadowMap != enShadowMap )
                {
                    mPreparedPass.shadowMaps.push_back( itShadowMap->get() );
                    ++itShadowMap;
                }
            }

            ForwardPlusBase *forwardPlus = sceneManager->_getActivePassForwardPlus();
            if( forwardPlus )
            {
                forwardPlus->fillConstBufferData( sceneManager->getCurrentViewport(), renderTarget,
                                                  mShaderSyntax, passBufferPtr );
                passBufferPtr += forwardPlus->getConstBufferSize() >> 2u;
            }

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            if( mHasPlanarReflections )
            {
                mPlanarReflections->fillConstBufferData( renderTarget, camera,
                                                         projectionMatrix, passBufferPtr );
                passBufferPtr += mPlanarReflections->getConstBufferSize() >> 2u;
            }
#endif
            if( mParallaxCorrectedCubemap )
            {
                mParallaxCorrectedCubemap->fillConstBufferData( viewMatrix, passBufferPtr );
                passBufferPtr += mParallaxCorrectedCubemap->getConstBufferSize() >> 2u;
            }
        }
        else
        {
            //vec4 viewZRow
            if( mShadowFilter == ExponentialShadowMaps )
            {
                *passBufferPtr++ = viewMatrix[2][0];
                *passBufferPtr++ = viewMatrix[2][1];
                *passBufferPtr++ = viewMatrix[2][2];
                *passBufferPtr++ = viewMatrix[2][3];
            }

            //vec2 depthRange;
            Real fNear, fFar;
            shadowNode->getMinMaxDepthRange( camera, fNear, fFar );
            const Real depthRange = fFar - fNear;
            *passBufferPtr++ = fNear;
            *passBufferPtr++ = 1.0f / depthRange;
            passBufferPtr += 2;
        }

        passBufferPtr = mListener->preparePassBuffer( shadowNode, casterPass, dualParaboloid,
                                                      sceneManager, passBufferPtr );

        assert( (size_t)(passBufferPtr - startupPtr) * 4u == mapSize );

        passBuffer->unmap( UO_KEEP_PERSISTENT );

        //mTexBuffers must hold at least one buffer to prevent out of bound exceptions.
        if( mTexBuffers.empty() )
        {
            size_t bufferSize = std::min<size_t>( mTextureBufferDefaultSize,
                                                  mVaoManager->getTexBufferMaxSize() );
            TexBufferPacked *newBuffer = mVaoManager->createTexBuffer( PF_FLOAT32_RGBA, bufferSize,
                                                                       BT_DYNAMIC_PERSISTENT, 0, false );
            mTexBuffers.push_back( newBuffer );
        }

        mLastTextureHash = 0;

        mLastBoundPool = 0;

        if( mShadowFilter == ExponentialShadowMaps )
            mCurrentShadowmapSamplerblock = mShadowmapEsmSamplerblock;
        else if( mShadowmapSamplerblock && !getProperty( HlmsBaseProp::ShadowUsesDepthTexture ) )
            mCurrentShadowmapSamplerblock = mShadowmapSamplerblock;
        else
            mCurrentShadowmapSamplerblock = mShadowmapCmpSamplerblock;

        mTexUnitSlotStart = mPreparedPass.shadowMaps.size() + 1;
        if( mGridBuffer )
            mTexUnitSlotStart += 2;
        if( mIrradianceVolume )
            mTexUnitSlotStart += 1;
        if( mParallaxCorrectedCubemap )
            mTexUnitSlotStart += 1;
        if( mPrePassTextures )
            mTexUnitSlotStart += 2;
        if( mPrePassMsaaDepthTexture )
            mTexUnitSlotStart += 1;
        if( mSsrTexture )
            mTexUnitSlotStart += 1;
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
        if( mHasPlanarReflections )
            mTexUnitSlotStart += 1;
#endif
        if( mAreaLightMasks && getProperty( HlmsBaseProp::LightsAreaTexMask ) > 0 )
        {
            mTexUnitSlotStart += 1;
            mUsingAreaLightMasks = true;
        }
        else
        {
            mUsingAreaLightMasks = false;
        }

        if( mLtcMatrixTexture && getProperty( HlmsBaseProp::LightsAreaLtc ) > 0 )
        {
            mTexUnitSlotStart += 1;
            mUsingLtcMatrix = true;
        }
        else
        {
            mUsingLtcMatrix = false;
        }

        for( size_t i=0; i<3u; ++i )
        {
            if( mDecalsTextures[i] &&
                (i != 2u || mDecalsTextures[2] != mDecalsTextures[0]) )
            {
                ++mTexUnitSlotStart;
            }
        }

        uploadDirtyDatablocks();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbs::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                    bool casterPass, uint32 lastCacheHash,
                                    uint32 lastTextureHash )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Trying to use slow-path on a desktop implementation. "
                     "Change the RenderQueue settings.",
                     "HlmsPbs::fillBuffersFor" );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbs::fillBuffersForV1( const HlmsCache *cache,
                                      const QueuedRenderable &queuedRenderable,
                                      bool casterPass, uint32 lastCacheHash,
                                      CommandBuffer *commandBuffer )
    {
        return fillBuffersFor( cache, queuedRenderable, casterPass,
                               lastCacheHash, commandBuffer, true );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbs::fillBuffersForV2( const HlmsCache *cache,
                                      const QueuedRenderable &queuedRenderable,
                                      bool casterPass, uint32 lastCacheHash,
                                      CommandBuffer *commandBuffer )
    {
        return fillBuffersFor( cache, queuedRenderable, casterPass,
                               lastCacheHash, commandBuffer, false );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbs::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                    bool casterPass, uint32 lastCacheHash,
                                    CommandBuffer *commandBuffer, bool isV1 )
    {
        assert( dynamic_cast<const HlmsPbsDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsPbsDatablock *datablock = static_cast<const HlmsPbsDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        if( OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH( lastCacheHash ) != mType )
        {
            //layout(binding = 0) uniform PassBuffer {} pass
            ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer-1];
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( VertexShader,
                                                                           0, passBuffer, 0,
                                                                           passBuffer->
                                                                           getTotalSizeBytes() );
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                           0, passBuffer, 0,
                                                                           passBuffer->
                                                                           getTotalSizeBytes() );

            if( !casterPass )
            {
                size_t texUnit = 1;

                if( mGridBuffer )
                {
                    texUnit = 3;
                    *commandBuffer->addCommand<CbShaderBuffer>() =
                            CbShaderBuffer( PixelShader, 1, mGridBuffer, 0, 0 );
                    *commandBuffer->addCommand<CbShaderBuffer>() =
                            CbShaderBuffer( PixelShader, 2, mGlobalLightListBuffer, 0, 0 );
                }

                if( mPrePassTextures )
                {
                    *commandBuffer->addCommand<CbTexture>() =
                            CbTexture( texUnit++, true, (*mPrePassTextures)[0].get(), 0 );
                    *commandBuffer->addCommand<CbTexture>() =
                            CbTexture( texUnit++, true, (*mPrePassTextures)[1].get(), 0 );
                }

                if( mPrePassMsaaDepthTexture )
                {
                    *commandBuffer->addCommand<CbTexture>() =
                            CbTexture( texUnit++, true, mPrePassMsaaDepthTexture.get(), 0 );
                }

                if( mSsrTexture )
                {
                    *commandBuffer->addCommand<CbTexture>() =
                            CbTexture( texUnit++, true, (*mSsrTexture)[0].get(), 0 );
                }

                if( mIrradianceVolume )
                {
                    const TexturePtr &irradianceTex = mIrradianceVolume->getIrradianceVolumeTexture();
                    const HlmsSamplerblock *samplerblock = mIrradianceVolume->getIrradSamplerblock();

                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true,
                                                                         irradianceTex.get(),
                                                                         samplerblock );
                    ++texUnit;
                }

                if( mUsingAreaLightMasks )
                {
                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true,
                                                                         mAreaLightMasks.get(),
                                                                         mAreaLightMasksSamplerblock );
                    ++texUnit;
                }

                if( mUsingLtcMatrix )
                {
                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true,
                                                                         mLtcMatrixTexture.get(),
                                                                         mAreaLightMasksSamplerblock );
                    ++texUnit;
                }

                for( size_t i=0; i<3u; ++i )
                {
                    if( mDecalsTextures[i] &&
                        (i != 2u || mDecalsTextures[2] != mDecalsTextures[0]) )
                    {
                        *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true,
                                                                             mDecalsTextures[i].get(),
                                                                             mDecalsSamplerblock );
                        ++texUnit;
                    }
                }

                //We changed HlmsType, rebind the shared textures.
                FastArray<Texture*>::const_iterator itor = mPreparedPass.shadowMaps.begin();
                FastArray<Texture*>::const_iterator end  = mPreparedPass.shadowMaps.end();
                while( itor != end )
                {
                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true, *itor,
                                                                         mCurrentShadowmapSamplerblock );
                    ++texUnit;
                    ++itor;
                }

                if( mParallaxCorrectedCubemap )
                {
                    Texture *pccTexture = mParallaxCorrectedCubemap->getBlendCubemap().get();
                    const HlmsSamplerblock *samplerblock =
                            mParallaxCorrectedCubemap->getBlendCubemapTrilinearSamplerblock();
                    *commandBuffer->addCommand<CbTexture>() = CbTexture( texUnit, true, pccTexture,
                                                                         samplerblock );
                    ++texUnit;
                }
            }
            else
            {
                *commandBuffer->addCommand<CbTextureDisableFrom>() = CbTextureDisableFrom( 1 );
            }

            mLastTextureHash = 0;
            mLastBoundPool = 0;

            //layout(binding = 2) uniform InstanceBuffer {} instance
            if( mCurrentConstBuffer < mConstBuffers.size() &&
                (size_t)((mCurrentMappedConstBuffer - mStartMappedConstBuffer) + 4) <=
                    mCurrentConstBufferSize )
            {
                *commandBuffer->addCommand<CbShaderBuffer>() =
                        CbShaderBuffer( VertexShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
                *commandBuffer->addCommand<CbShaderBuffer>() =
                        CbShaderBuffer( PixelShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
            }

            rebindTexBuffer( commandBuffer );

#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            mLastBoundPlanarReflection = 0u;
#endif
            mListener->hlmsTypeChanged( casterPass, commandBuffer, datablock );
        }

        //Don't bind the material buffer on caster passes (important to keep
        //MDI & auto-instancing running on shadow map passes)
        if( mLastBoundPool != datablock->getAssignedPool() &&
            (!casterPass || datablock->getAlphaTest() != CMPF_ALWAYS_PASS) )
        {
            //layout(binding = 1) uniform MaterialBuf {} materialArray
            const ConstBufferPool::BufferPool *newPool = datablock->getAssignedPool();
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                           1, newPool->materialBuffer, 0,
                                                                           newPool->materialBuffer->
                                                                           getTotalSizeBytes() );
            CubemapProbe *manualProbe = datablock->getCubemapProbe();
            if( manualProbe )
            {
                ConstBufferPacked *probeConstBuf = manualProbe->getConstBufferForManualProbes();
                *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                               3, probeConstBuf,
                                                                               0, 0 );
            }
            mLastBoundPool = newPool;
        }

        uint32 * RESTRICT_ALIAS currentMappedConstBuffer    = mCurrentMappedConstBuffer;
        float * RESTRICT_ALIAS currentMappedTexBuffer       = mCurrentMappedTexBuffer;

        bool hasSkeletonAnimation = queuedRenderable.renderable->hasSkeletonAnimation();

        const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------

        if( !hasSkeletonAnimation )
        {
            //We need to correct currentMappedConstBuffer to point to the right texture buffer's
            //offset, which may not be in sync if the previous draw had skeletal animation.
            const size_t currentConstOffset = (currentMappedTexBuffer - mStartMappedTexBuffer) >>
                                                (2 + !casterPass);
            currentMappedConstBuffer =  currentConstOffset + mStartMappedConstBuffer;
            bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4)
                                        > mCurrentConstBufferSize;

            const size_t minimumTexBufferSize = 16 * (1 + !casterPass);
            bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                                         minimumTexBufferSize >= mCurrentTexBufferSize;

            if( exceedsConstBuffer || exceedsTexBuffer )
            {
                currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                if( exceedsTexBuffer )
                    mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                else
                    rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                currentMappedTexBuffer = mCurrentMappedTexBuffer;
            }

            //uint worldMaterialIdx[]
            *currentMappedConstBuffer = datablock->getAssignedSlot() & 0x1FF;

            //mat4x3 world
#if !OGRE_DOUBLE_PRECISION
            memcpy( currentMappedTexBuffer, &worldMat, 4 * 3 * sizeof( float ) );
            currentMappedTexBuffer += 16;
#else
            for( int y = 0; y < 3; ++y )
            {
                for( int x = 0; x < 4; ++x )
                {
                    *currentMappedTexBuffer++ = worldMat[ y ][ x ];
                }
            }
            currentMappedTexBuffer += 4;
#endif

            //mat4 worldView
            Matrix4 tmp = mPreparedPass.viewMatrix.concatenateAffine( worldMat );
    #ifdef OGRE_GLES2_WORKAROUND_1
            tmp = tmp.transpose();
#endif
#if !OGRE_DOUBLE_PRECISION
            memcpy( currentMappedTexBuffer, &tmp, sizeof( Matrix4 ) * !casterPass );
            currentMappedTexBuffer += 16 * !casterPass;
#else
            if( !casterPass )
            {
                for( int y = 0; y < 4; ++y )
                {
                    for( int x = 0; x < 4; ++x )
                    {
                        *currentMappedTexBuffer++ = tmp[ y ][ x ];
                    }
                }
            }
#endif
        }
        else
        {
            bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4)
                                        > mCurrentConstBufferSize;

            if( isV1 )
            {
                uint16 numWorldTransforms = queuedRenderable.renderable->getNumWorldTransforms();
                assert( numWorldTransforms <= 256u );

                const size_t minimumTexBufferSize = 12 * numWorldTransforms;
                bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                        minimumTexBufferSize >= mCurrentTexBufferSize;

                if( exceedsConstBuffer || exceedsTexBuffer )
                {
                    currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                    if( exceedsTexBuffer )
                        mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                    else
                        rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                    currentMappedTexBuffer = mCurrentMappedTexBuffer;
                }

                //uint worldMaterialIdx[]
                size_t distToWorldMatStart = mCurrentMappedTexBuffer - mStartMappedTexBuffer;
                distToWorldMatStart >>= 2;
                *currentMappedConstBuffer = (distToWorldMatStart << 9 ) |
                        (datablock->getAssignedSlot() & 0x1FF);

                //vec4 worldMat[][3]
                //TODO: Don't rely on a virtual function + make a direct 4x3 copy
                Matrix4 tmp[256];
                queuedRenderable.renderable->getWorldTransforms( tmp );
                for( size_t i=0; i<numWorldTransforms; ++i )
                {
#if !OGRE_DOUBLE_PRECISION
                    memcpy( currentMappedTexBuffer, &tmp[ i ], 12 * sizeof( float ) );
                    currentMappedTexBuffer += 12;
#else
                    for( int y = 0; y < 3; ++y )
                    {
                        for( int x = 0; x < 4; ++x )
                        {
                            *currentMappedTexBuffer++ = tmp[ i ][ y ][ x ];
                        }
                    }
#endif
                }
            }
            else
            {
                SkeletonInstance *skeleton = queuedRenderable.movableObject->getSkeletonInstance();

#if OGRE_DEBUG_MODE
                assert( dynamic_cast<const RenderableAnimated*>( queuedRenderable.renderable ) );
#endif

                const RenderableAnimated *renderableAnimated = static_cast<const RenderableAnimated*>(
                                                                        queuedRenderable.renderable );

                const RenderableAnimated::IndexMap *indexMap = renderableAnimated->getBlendIndexToBoneIndexMap();

                const size_t minimumTexBufferSize = 12 * indexMap->size();
                bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                                            minimumTexBufferSize >= mCurrentTexBufferSize;

                if( exceedsConstBuffer || exceedsTexBuffer )
                {
                    currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

                    if( exceedsTexBuffer )
                        mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
                    else
                        rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

                    currentMappedTexBuffer = mCurrentMappedTexBuffer;
                }

                //uint worldMaterialIdx[]
                size_t distToWorldMatStart = mCurrentMappedTexBuffer - mStartMappedTexBuffer;
                distToWorldMatStart >>= 2;
                *currentMappedConstBuffer = (distToWorldMatStart << 9 ) |
                        (datablock->getAssignedSlot() & 0x1FF);

                RenderableAnimated::IndexMap::const_iterator itBone = indexMap->begin();
                RenderableAnimated::IndexMap::const_iterator enBone = indexMap->end();

                while( itBone != enBone )
                {
                    const SimpleMatrixAf4x3 &mat4x3 = skeleton->_getBoneFullTransform( *itBone );
                    mat4x3.streamTo4x3( currentMappedTexBuffer );
                    currentMappedTexBuffer += 12;

                    ++itBone;
                }
            }

            //If the next entity will not be skeletally animated, we'll need
            //currentMappedTexBuffer to be 16/32-byte aligned.
            //Non-skeletally animated objects are far more common than skeletal ones,
            //so we do this here instead of doing it before rendering the non-skeletal ones.
            size_t currentConstOffset = (size_t)(currentMappedTexBuffer - mStartMappedTexBuffer);
            currentConstOffset = alignToNextMultiple( currentConstOffset, 16 + 16 * !casterPass );
            currentConstOffset = std::min( currentConstOffset, mCurrentTexBufferSize );
            currentMappedTexBuffer = mStartMappedTexBuffer + currentConstOffset;
        }

        *reinterpret_cast<float * RESTRICT_ALIAS>( currentMappedConstBuffer+1 ) = datablock->
                                                                                    mShadowConstantBias;
#if !OGRE_NO_FINE_LIGHT_MASK_GRANULARITY
        *( currentMappedConstBuffer+2u ) = queuedRenderable.movableObject->getLightMask();
#endif
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
        *( currentMappedConstBuffer+3u ) = queuedRenderable.renderable->mCustomParameter & 0x7F;
#endif
        currentMappedConstBuffer += 4;

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------

        if( !casterPass || datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
            if( mHasPlanarReflections &&
                (queuedRenderable.renderable->mCustomParameter & 0x80) &&
                mLastBoundPlanarReflection != queuedRenderable.renderable->mCustomParameter )
            {
                const uint8 activeActorIdx = queuedRenderable.renderable->mCustomParameter & 0x7F;
                TexturePtr planarReflTex = mPlanarReflections->getTexture( activeActorIdx );
                *commandBuffer->addCommand<CbTexture>() =
                        CbTexture( mTexUnitSlotStart - 1u, true, planarReflTex.get(),
                                   mPlanarReflectionsSamplerblock );
                mLastBoundPlanarReflection = queuedRenderable.renderable->mCustomParameter;
            }
#endif
            if( datablock->mTextureHash != mLastTextureHash )
            {
                //Rebind textures
                size_t texUnit = mTexUnitSlotStart;

                PbsBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
                PbsBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

                while( itor != end )
                {
                    if( itor->texture != mTargetEnvMap )
                    {
                        *commandBuffer->addCommand<CbTexture>() =
                                CbTexture( texUnit++, true, itor->texture.get(), itor->samplerBlock );
                    }
                    ++itor;
                }

                *commandBuffer->addCommand<CbTextureDisableFrom>() = CbTextureDisableFrom( texUnit );

                mLastTextureHash = datablock->mTextureHash;
            }
        }

        mCurrentMappedConstBuffer   = currentMappedConstBuffer;
        mCurrentMappedTexBuffer     = currentMappedTexBuffer;

        return ((mCurrentMappedConstBuffer - mStartMappedConstBuffer) >> 2) - 1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::destroyAllBuffers(void)
    {
        HlmsBufferManager::destroyAllBuffers();

        mCurrentPassBuffer  = 0;

        {
            ConstBufferPackedVec::const_iterator itor = mPassBuffers.begin();
            ConstBufferPackedVec::const_iterator end  = mPassBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyConstBuffer( *itor );
                ++itor;
            }

            mPassBuffers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::postCommandBufferExecution( CommandBuffer *commandBuffer )
    {
        HlmsBufferManager::postCommandBufferExecution( commandBuffer );

        if( mPrePassMsaaDepthTexture )
        {
            //We need to unbind the depth texture, it may be used as a depth buffer later.
            size_t texUnit = mGridBuffer ? 3 : 1;
            if( mPrePassTextures )
                texUnit += 2;

            mRenderSystem->_setTexture( texUnit, false, 0 );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::frameEnded(void)
    {
        HlmsBufferManager::frameEnded();
        mCurrentPassBuffer  = 0;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::loadLtcMatrix(void)
    {
        HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();

        const uint32 poolId = 992044u;

        if( !hlmsTextureManager->hasPoolId( poolId, HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA ) )
        {
            hlmsTextureManager->reservePoolId( poolId, HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA,
                                               64u, 64u, 2u, 0u, PF_FLOAT16_RGBA, false, false );
        }
        HlmsTextureManager::TextureLocation texLocation0, texLocation1;
        texLocation0 = hlmsTextureManager->createOrRetrieveTexture(
                           "ltcMatrix0.dds", "ltcMatrix0.dds",
                           HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA, poolId );
        texLocation1 = hlmsTextureManager->createOrRetrieveTexture(
                           "ltcMatrix1.dds", "ltcMatrix1.dds",
                           HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA, poolId );

        OGRE_ASSERT_LOW( texLocation0.texture == texLocation1.texture );
        OGRE_ASSERT_LOW( texLocation0.xIdx == 0u );
        OGRE_ASSERT_LOW( texLocation1.xIdx == 1u );

        mLtcMatrixTexture = texLocation0.texture;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::getDefaultPaths( String &outDataFolderPath, StringVector &outLibraryFoldersPaths )
    {
        //We need to know what RenderSystem is currently in use, as the
        //name of the compatible shading language is part of the path
        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingleton().getRenderSystem();
        Ogre::String shaderSyntax = "GLSL";
        if (renderSystem->getName() == "Direct3D11 Rendering Subsystem")
            shaderSyntax = "HLSL";
        else if (renderSystem->getName() == "Metal Rendering Subsystem")
            shaderSyntax = "Metal";

        //Fill the library folder paths with the relevant folders
        outLibraryFoldersPaths.clear();
        outLibraryFoldersPaths.push_back( "Hlms/Common/" + shaderSyntax );
        outLibraryFoldersPaths.push_back( "Hlms/Common/Any" );
        outLibraryFoldersPaths.push_back( "Hlms/Pbs/Any" );

        //Fill the data folder path
        outDataFolderPath = "Hlms/Pbs/" + shaderSyntax;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setDebugPssmSplits( bool bDebug )
    {
        mDebugPssmSplits = bDebug;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setShadowSettings( ShadowFilter filter )
    {
        mShadowFilter = filter;

        if( mShadowFilter == ExponentialShadowMaps && mHlmsManager->getShadowMappingUseBackFaces() )
        {
            LogManager::getSingleton().logMessage(
                        "QUALITY WARNING: It is highly recommended that you call "
                        "mHlmsManager->setShadowMappingUseBackFaces( false ) when using Exponential "
                        "Shadow Maps (HlmsPbs::setShadowSettings)" );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setEsmK( uint16 K )
    {
        assert( K != 0 && "A value of K = 0 is invalid!" );
        mEsmK = K;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setAmbientLightMode( AmbientLightMode mode )
    {
        mAmbientLightMode = mode;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setAreaLightMasks( const TexturePtr &areaLightMask )
    {
        mAreaLightMasks = areaLightMask;
    }
#ifdef OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS
    //-----------------------------------------------------------------------------------
    void HlmsPbs::setPlanarReflections( PlanarReflections *planarReflections )
    {
        mPlanarReflections = planarReflections;
    }
    //-----------------------------------------------------------------------------------
    PlanarReflections* HlmsPbs::getPlanarReflections(void) const
    {
        return mPlanarReflections;
    }
#endif
#if !OGRE_NO_JSON
    //-----------------------------------------------------------------------------------
    void HlmsPbs::_loadJson( const rapidjson::Value &jsonValue, const HlmsJson::NamedBlocks &blocks,
                             HlmsDatablock *datablock, HlmsJsonListener *listener,
                             const String &additionalTextureExtension ) const
    {
        HlmsJsonPbs jsonPbs( mHlmsManager, listener, additionalTextureExtension );
        jsonPbs.loadMaterial( jsonValue, blocks, datablock );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::_saveJson( const HlmsDatablock *datablock, String &outString,
                             HlmsJsonListener *listener,
                             const String &additionalTextureExtension ) const
    {
        HlmsJsonPbs jsonPbs( mHlmsManager, listener, additionalTextureExtension );
        jsonPbs.saveMaterial( datablock, outString );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbs::_collectSamplerblocks( set<const HlmsSamplerblock*>::type &outSamplerblocks,
                                         const HlmsDatablock *datablock ) const
    {
        HlmsJsonPbs::collectSamplerblocks( datablock, outSamplerblocks );
    }
#endif
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsPbs::createDatablockImpl( IdString datablockName,
                                                       const HlmsMacroblock *macroblock,
                                                       const HlmsBlendblock *blendblock,
                                                       const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsPbsDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
