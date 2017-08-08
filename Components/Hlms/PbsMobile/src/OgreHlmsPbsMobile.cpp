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

#include "OgreHlmsPbsMobile.h"
#include "OgreHlmsPbsMobileDatablock.h"
#include "OgrePbsMobileShaderCreationData.h"

#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreCamera.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreCamera.h"

namespace Ogre
{
    const IdString PbsMobileProperty::HwGammaRead       = IdString( "hw_gamma_read" );
    const IdString PbsMobileProperty::HwGammaWrite      = IdString( "hw_gamma_write" );
    const IdString PbsMobileProperty::SignedIntTex      = IdString( "signed_int_textures" );

    const IdString PbsMobileProperty::DiffuseMap        = IdString( "diffuse_map" );
    const IdString PbsMobileProperty::NormalMapTex      = IdString( "normal_map_tex" );
    const IdString PbsMobileProperty::SpecularMap       = IdString( "specular_map" );
    const IdString PbsMobileProperty::RoughnessMap      = IdString( "roughness_map" );
    const IdString PbsMobileProperty::EnvProbeMap       = IdString( "envprobe_map" );
    const IdString PbsMobileProperty::DetailWeightMap   = IdString( "detail_weight_map" );

    const IdString PbsMobileProperty::NormalMap         = IdString( "normal_map" );

    const IdString PbsMobileProperty::UvAtlas           = IdString( "uv_atlas" );
    const IdString PbsMobileProperty::FresnelScalar     = IdString( "fresnel_scalar" );

    const IdString PbsMobileProperty::NormalWeight          = IdString( "normal_weight" );
    const IdString PbsMobileProperty::NormalWeightTex       = IdString( "normal_weight_tex" );
    const IdString PbsMobileProperty::NormalWeightDetail0   = IdString( "normal_weight_detail0" );
    const IdString PbsMobileProperty::NormalWeightDetail1   = IdString( "normal_weight_detail1" );
    const IdString PbsMobileProperty::NormalWeightDetail2   = IdString( "normal_weight_detail2" );
    const IdString PbsMobileProperty::NormalWeightDetail3   = IdString( "normal_weight_detail3" );

    const IdString PbsMobileProperty::DetailWeights     = IdString( "detail_weights" );
    const IdString PbsMobileProperty::DetailOffsetsD    = IdString( "detail_offsetsD" );
    const IdString PbsMobileProperty::DetailOffsetsD0   = IdString( "detail_offsetsD0" );
    const IdString PbsMobileProperty::DetailOffsetsD1   = IdString( "detail_offsetsD1" );
    const IdString PbsMobileProperty::DetailOffsetsD2   = IdString( "detail_offsetsD2" );
    const IdString PbsMobileProperty::DetailOffsetsD3   = IdString( "detail_offsetsD3" );
    const IdString PbsMobileProperty::DetailOffsetsN    = IdString( "detail_offsetsN" );
    const IdString PbsMobileProperty::DetailOffsetsN0   = IdString( "detail_offsetsN0" );
    const IdString PbsMobileProperty::DetailOffsetsN1   = IdString( "detail_offsetsN1" );
    const IdString PbsMobileProperty::DetailOffsetsN2   = IdString( "detail_offsetsN2" );
    const IdString PbsMobileProperty::DetailOffsetsN3   = IdString( "detail_offsetsN3" );

    const IdString PbsMobileProperty::UvDiffuse         = IdString( "uv_diffuse" );
    const IdString PbsMobileProperty::UvNormal          = IdString( "uv_normal" );
    const IdString PbsMobileProperty::UvSpecular        = IdString( "uv_specular" );
    const IdString PbsMobileProperty::UvRoughness       = IdString( "uv_roughness" );
    const IdString PbsMobileProperty::UvDetailWeight    = IdString( "uv_detail_weight" );

    const IdString PbsMobileProperty::UvDetail0         = IdString( "uv_detail0" );
    const IdString PbsMobileProperty::UvDetail1         = IdString( "uv_detail1" );
    const IdString PbsMobileProperty::UvDetail2         = IdString( "uv_detail2" );
    const IdString PbsMobileProperty::UvDetail3         = IdString( "uv_detail3" );

    const IdString PbsMobileProperty::UvDetailNm0       = IdString( "uv_detail_nm0" );
    const IdString PbsMobileProperty::UvDetailNm1       = IdString( "uv_detail_nm1" );
    const IdString PbsMobileProperty::UvDetailNm2       = IdString( "uv_detail_nm2" );
    const IdString PbsMobileProperty::UvDetailNm3       = IdString( "uv_detail_nm3" );

    const IdString PbsMobileProperty::BlendModeIndex0   = IdString( "blend_mode_idx0" );
    const IdString PbsMobileProperty::BlendModeIndex1   = IdString( "blend_mode_idx1" );
    const IdString PbsMobileProperty::BlendModeIndex2   = IdString( "blend_mode_idx2" );
    const IdString PbsMobileProperty::BlendModeIndex3   = IdString( "blend_mode_idx3" );

    const IdString PbsMobileProperty::DetailMapsDiffuse = IdString( "detail_maps_diffuse" );
    const IdString PbsMobileProperty::DetailMapsNormal  = IdString( "detail_maps_normal" );

    const IdString PbsMobileProperty::DetailDiffuseSwizzle0 = IdString( "detail_diffuse_swizzle0" );
    const IdString PbsMobileProperty::DetailDiffuseSwizzle1 = IdString( "detail_diffuse_swizzle1" );
    const IdString PbsMobileProperty::DetailDiffuseSwizzle2 = IdString( "detail_diffuse_swizzle2" );
    const IdString PbsMobileProperty::DetailDiffuseSwizzle3 = IdString( "detail_diffuse_swizzle3" );

    const IdString PbsMobileProperty::DetailNormalSwizzle0  = IdString( "detail_normal_swizzle0" );
    const IdString PbsMobileProperty::DetailNormalSwizzle1  = IdString( "detail_normal_swizzle1" );
    const IdString PbsMobileProperty::DetailNormalSwizzle2  = IdString( "detail_normal_swizzle2" );
    const IdString PbsMobileProperty::DetailNormalSwizzle3  = IdString( "detail_normal_swizzle3" );

    const IdString *PbsMobileProperty::UvSourcePtrs[NUM_PBSM_SOURCES] =
    {
        &PbsMobileProperty::UvDiffuse,
        &PbsMobileProperty::UvNormal,
        &PbsMobileProperty::UvSpecular,
        &PbsMobileProperty::UvRoughness,
        &PbsMobileProperty::UvDetailWeight,
        &PbsMobileProperty::UvDetail0,
        &PbsMobileProperty::UvDetail1,
        &PbsMobileProperty::UvDetail2,
        &PbsMobileProperty::UvDetail3,
        &PbsMobileProperty::UvDetailNm0,
        &PbsMobileProperty::UvDetailNm1,
        &PbsMobileProperty::UvDetailNm2,
        &PbsMobileProperty::UvDetailNm3
    };

    const IdString *PbsMobileProperty::DetailDiffuseSwizzles[4] =
    {
        &PbsMobileProperty::DetailDiffuseSwizzle0,
        &PbsMobileProperty::DetailDiffuseSwizzle1,
        &PbsMobileProperty::DetailDiffuseSwizzle2,
        &PbsMobileProperty::DetailDiffuseSwizzle3
    };

    const IdString *PbsMobileProperty::DetailNormalSwizzles[4] =
    {
        &PbsMobileProperty::DetailNormalSwizzle0,
        &PbsMobileProperty::DetailNormalSwizzle1,
        &PbsMobileProperty::DetailNormalSwizzle2,
        &PbsMobileProperty::DetailNormalSwizzle3
    };

    const IdString *PbsMobileProperty::DetailNormalWeights[4] =
    {
        &PbsMobileProperty::NormalWeightDetail0,
        &PbsMobileProperty::NormalWeightDetail1,
        &PbsMobileProperty::NormalWeightDetail2,
        &PbsMobileProperty::NormalWeightDetail3
    };

    const IdString *PbsMobileProperty::DetailOffsetsDPtrs[4] =
    {
        &PbsMobileProperty::DetailOffsetsD0,
        &PbsMobileProperty::DetailOffsetsD1,
        &PbsMobileProperty::DetailOffsetsD2,
        &PbsMobileProperty::DetailOffsetsD3
    };

    const IdString *PbsMobileProperty::DetailOffsetsNPtrs[4] =
    {
        &PbsMobileProperty::DetailOffsetsN0,
        &PbsMobileProperty::DetailOffsetsN1,
        &PbsMobileProperty::DetailOffsetsN2,
        &PbsMobileProperty::DetailOffsetsN3
    };

    const IdString *PbsMobileProperty::BlendModes[4] =
    {
        &PbsMobileProperty::BlendModeIndex0,
        &PbsMobileProperty::BlendModeIndex1,
        &PbsMobileProperty::BlendModeIndex2,
        &PbsMobileProperty::BlendModeIndex3
    };

    extern const String c_pbsBlendModes[];

    const String c_vsPerObjectUniforms[] =
    {
        "worldView",
        "worldViewProj",
        "worldMat"
    };
    const String c_psPerObjectUniforms[] =
    {
        "roughness",
        "kD",
        "kS",
        "alpha_test_threshold",
        "F0",
        "atlasOffsets",
        "normalWeights",
        "cDetailWeights",
        "detailOffsetScaleD",
        "detailOffsetScaleN",
    };

    HlmsPbsMobile::HlmsPbsMobile( Archive *dataFolder, ArchiveVec *libraryFolders ) :
        Hlms( HLMS_PBS, "pbs", dataFolder, libraryFolders )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsMobile::~HlmsPbsMobile()
    {
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsPbsMobile::createShaderCacheEntry( uint32 renderableHash,
                                                            const HlmsCache &passCache,
                                                            uint32 finalHash,
                                                            const QueuedRenderable &queuedRenderable )
    {
        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        GpuNamedConstants *constantsDef;
        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pso.vertexShader->
                                                       getConstantDefinitions() );
        bool hasSkeleton = getProperty( HlmsBaseProp::Skeleton ) != 0;
        for( size_t i=hasSkeleton ? 2 : 0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_vsPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pso.pixelShader->
                                                       getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_psPerObjectUniforms ) / sizeof( String ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_psPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        //Set samplers.
        GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();

        int texUnit = 0;
        if( !mPreparedPass.shadowMaps.empty() )
        {
            vector<int>::type shadowMaps;
            shadowMaps.reserve( mPreparedPass.shadowMaps.size() );
            for( texUnit=0; texUnit<(int)mPreparedPass.shadowMaps.size(); ++texUnit )
                shadowMaps.push_back( texUnit );

            psParams->setNamedConstant( "texShadowMap", &shadowMaps[0], shadowMaps.size(), 1 );
        }

        assert( dynamic_cast<const HlmsPbsMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsPbsMobileDatablock *datablock = static_cast<const HlmsPbsMobileDatablock*>(
                                                    queuedRenderable.renderable->getDatablock() );

        if( getProperty( PbsMobileProperty::DiffuseMap ) )
        {
            assert( !datablock->mTexture[PBSM_DIFFUSE].isNull() );
            psParams->setNamedConstant( "texDiffuseMap", texUnit++ );
        }
        if( getProperty( PbsMobileProperty::NormalMapTex ) )
        {
            assert( !datablock->mTexture[PBSM_NORMAL].isNull() );
            psParams->setNamedConstant( "texNormalMap", texUnit++ );
        }
        if( getProperty( PbsMobileProperty::SpecularMap ) )
        {
            assert( !datablock->mTexture[PBSM_SPECULAR].isNull() );
            psParams->setNamedConstant( "texSpecularMap", texUnit++ );
        }
        if( getProperty( PbsMobileProperty::RoughnessMap ) )
        {
            assert( !datablock->mTexture[PBSM_ROUGHNESS].isNull() );
            psParams->setNamedConstant( "texRoughnessMap", texUnit++ );
        }
        if( getProperty( PbsMobileProperty::DetailWeightMap ) )
        {
            assert( !datablock->mTexture[PBSM_DETAIL_WEIGHT].isNull() );
            psParams->setNamedConstant( "texDetailWeightMap", texUnit++ );
        }

        if( getProperty( PbsMobileProperty::DetailMapsDiffuse ) )
        {
            size_t validDetailMaps = 0;
            for( size_t i=PBSM_DETAIL0; i<=PBSM_DETAIL3; ++i )
            {
                if( !datablock->mTexture[i].isNull() )
                {
                    assert( !datablock->mTexture[i].isNull() );
                    psParams->setNamedConstant( "texDetailMap[" +
                                                StringConverter::toString( validDetailMaps++ ) + "]",
                                                texUnit++ );
                }
            }
        }

        if( getProperty( PbsMobileProperty::DetailMapsNormal ) )
        {
            size_t validDetailMaps = 0;
            for( size_t i=PBSM_DETAIL0_NM; i<=PBSM_DETAIL3_NM; ++i )
            {
                if( !datablock->mTexture[i].isNull() )
                {
                    psParams->setNamedConstant( "texDetailNormalMap[" +
                                                StringConverter::toString( validDetailMaps++ ) + "]",
                                                texUnit++ );
                }
            }
        }

        if( getProperty( PbsMobileProperty::EnvProbeMap ) )
        {
            assert( !datablock->mTexture[PBSM_REFLECTION].isNull() );
            psParams->setNamedConstant( "texEnvProbeMap", texUnit++ );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobile::setDetailMapProperties( bool diffuseMaps, HlmsPbsMobileDatablock *datablock,
                                                PiecesMap *inOutPieces )
    {
        PbsMobileTextureTypes detailTextureStart    = diffuseMaps ? PBSM_DETAIL0 : PBSM_DETAIL0_NM;
        const IdString **detailSwizzles = diffuseMaps ? PbsMobileProperty::DetailDiffuseSwizzles :
                                                        PbsMobileProperty::DetailNormalSwizzles;

        size_t validDetailMaps = 0;
        for( size_t i=0; i<4; ++i )
        {
            uint8 blendMode = datablock->mShaderCreationData->blendModes[i];

            //If Detail map 0 doesn't exists but Detail map 1 does;
            //then DetailDiffuseSwizzle0 must reference the swizzle 'y'
            //Same happens with the UV sources (the UV sources[1] end up
            //actually as sources[0], etc).
            if( !datablock->mTexture[detailTextureStart + i].isNull() )
            {
                if( diffuseMaps )
                {
                    inOutPieces[PixelShader][*PbsMobileProperty::BlendModes[validDetailMaps]] =
                                                "@insertpiece( " + c_pbsBlendModes[blendMode] + ")";
                }

                const char *swizzles[4] = { "x", "y", "z", "w" };
                IdString swizzleN = *detailSwizzles[validDetailMaps];
                inOutPieces[PixelShader][swizzleN] = swizzles[i];

                uint8 uvSource = datablock->mShaderCreationData->uvSource[detailTextureStart + i];
                setProperty( *PbsMobileProperty::UvSourcePtrs[detailTextureStart + validDetailMaps],
                             uvSource );

                if( getProperty( *HlmsBaseProp::UvCountPtrs[uvSource] ) < 2 )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                                 "Renderable needs at least 2 coordinates in UV set #" +
                                 StringConverter::toString( uvSource ) +
                                 ". Either change the mesh, or change the UV source settings",
                                 "HlmsPbsMobile::setDetailMapProperties" );
                }

                ++validDetailMaps;
            }
        }

        setProperty( diffuseMaps ? PbsMobileProperty::DetailMapsDiffuse :
                                   PbsMobileProperty::DetailMapsNormal, validDetailMaps );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobile::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces )
    {
        assert( dynamic_cast<HlmsPbsMobileDatablock*>( renderable->getDatablock() ) );
        HlmsPbsMobileDatablock *datablock = static_cast<HlmsPbsMobileDatablock*>(
                                                        renderable->getDatablock() );
        setProperty( PbsMobileProperty::UvAtlas, datablock->_calculateNumUvAtlas( false ) );
        setProperty( PbsMobileProperty::FresnelScalar, datablock->hasSeparateFresnel() );

        for( size_t i=0; i<PBSM_DETAIL0; ++i )
        {
            uint8 uvSource = datablock->mShaderCreationData->uvSource[i];
            setProperty( *PbsMobileProperty::UvSourcePtrs[i], uvSource );

            if( !datablock->mTexture[i].isNull() &&
                getProperty( *HlmsBaseProp::UvCountPtrs[uvSource] ) < 2 )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Renderable needs at least 2 coordinates in UV set #" +
                             StringConverter::toString( uvSource ) +
                             ". Either change the mesh, or change the UV source settings",
                             "HlmsPbsMobile::calculateHashForPreCreate" );
            }
        }

        int numNormalWeights = 0;
        if( datablock->getNormalMapWeight() != 1.0f && !datablock->mTexture[PBSM_NORMAL].isNull() )
        {
            setProperty( PbsMobileProperty::NormalWeightTex, 1 );
            ++numNormalWeights;
        }

        {
            size_t validDetailMaps = 0;
            for( size_t i=0; i<4; ++i )
            {
                if( !datablock->mTexture[PBSM_DETAIL0_NM + i].isNull() )
                {
                    if( datablock->getDetailNormalWeight( i ) != 1.0f )
                    {
                        setProperty( *PbsMobileProperty::DetailNormalWeights[validDetailMaps], 1 );
                        ++numNormalWeights;
                    }

                    ++validDetailMaps;
                }
            }
        }

        setProperty( PbsMobileProperty::NormalWeight, numNormalWeights );

        setDetailMapProperties( true, datablock, inOutPieces );
        setDetailMapProperties( false, datablock, inOutPieces );

        {
            bool anyDetailWeight = false;
            for( size_t i=0; i<4 && !anyDetailWeight; ++i )
            {
                if( datablock->mShaderCreationData->mDetailWeight[i] != 1.0f &&
                    (!datablock->mTexture[PBSM_DETAIL0 + i].isNull() ||
                     !datablock->mTexture[PBSM_DETAIL0_NM + i].isNull()) )
                {
                    anyDetailWeight = true;
                }
            }

            if( anyDetailWeight )
                setProperty( PbsMobileProperty::DetailWeights, 1 );
        }

        {
            int numOffsets = 0;
            size_t validDetailMaps = 0;
            for( size_t i=0; i<4; ++i )
            {
                if( datablock->mShaderCreationData->mDetailsOffsetScale[i] != Vector4( 0, 0, 1, 1 ) )
                {
                    setProperty( *PbsMobileProperty::DetailOffsetsDPtrs[validDetailMaps], 1 );
                    ++numOffsets;
                }

                if( !datablock->mTexture[PBSM_DETAIL0 + i].isNull() )
                    ++validDetailMaps;
            }

            setProperty( PbsMobileProperty::DetailOffsetsD, numOffsets );

            numOffsets = 0;
            validDetailMaps = 0;
            for( size_t i=0; i<4; ++i )
            {
                if( datablock->mShaderCreationData->mDetailsOffsetScale[i+4] != Vector4( 0, 0, 1, 1 ) )
                {
                    setProperty( *PbsMobileProperty::DetailOffsetsNPtrs[validDetailMaps], 1 );
                    ++numOffsets;
                }

                if( !datablock->mTexture[PBSM_DETAIL0_NM + i].isNull() )
                    ++validDetailMaps;
            }

            setProperty( PbsMobileProperty::DetailOffsetsN, numOffsets );
        }

        setProperty( PbsMobileProperty::DiffuseMap,     !datablock->mTexture[PBSM_DIFFUSE].isNull() );
        setProperty( PbsMobileProperty::NormalMapTex,   !datablock->mTexture[PBSM_NORMAL].isNull() );
        setProperty( PbsMobileProperty::SpecularMap,    !datablock->mTexture[PBSM_SPECULAR].isNull() );
        setProperty( PbsMobileProperty::RoughnessMap,   !datablock->mTexture[PBSM_ROUGHNESS].isNull() );
        setProperty( PbsMobileProperty::EnvProbeMap,    !datablock->mTexture[PBSM_REFLECTION].isNull() );
        setProperty( PbsMobileProperty::DetailWeightMap,!datablock->mTexture[PBSM_DETAIL_WEIGHT].isNull() );

        bool usesNormalMap = !datablock->mTexture[PBSM_NORMAL].isNull();
        for( size_t i=PBSM_DETAIL0_NM; i<=PBSM_DETAIL3_NM; ++i )
            usesNormalMap |= !datablock->mTexture[i].isNull();
        setProperty( PbsMobileProperty::NormalMap, usesNormalMap );

        /*setProperty( HlmsBaseProp::, !datablock->mTexture[PBSM_DETAIL0].isNull() );
        setProperty( HlmsBaseProp::DiffuseMap, !datablock->mTexture[PBSM_DETAIL1].isNull() );*/
        bool normalMapCanBeSupported = (getProperty( HlmsBaseProp::Normal ) &&
                                        getProperty( HlmsBaseProp::Tangent )) ||
                                        getProperty( HlmsBaseProp::QTangent );

        if( !normalMapCanBeSupported && usesNormalMap )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Renderable can't use normal maps but datablock wants normal maps. "
                         "Generate Tangents for this mesh to fix the problem or use a "
                         "datablock without normal maps.", "HlmsPbsMobile::calculateHashForPreCreate" );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobile::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces )
    {
        HlmsPbsMobileDatablock *datablock = static_cast<HlmsPbsMobileDatablock*>(
                                                        renderable->getDatablock() );
        setProperty( PbsMobileProperty::UvAtlas, datablock->_calculateNumUvAtlas( true ) );

        HlmsPropertyVec::iterator itor = mSetProperties.begin();
        HlmsPropertyVec::iterator end  = mSetProperties.end();

        while( itor != end )
        {
            if( itor->keyName != PbsMobileProperty::UvAtlas &&
                itor->keyName != PbsMobileProperty::HwGammaRead &&
                itor->keyName != PbsMobileProperty::UvDiffuse &&
                itor->keyName != HlmsBaseProp::Skeleton &&
                itor->keyName != HlmsBaseProp::BonesPerVertex &&
                itor->keyName != HlmsBaseProp::DualParaboloidMapping &&
                itor->keyName != HlmsBaseProp::AlphaTest )
            {
                itor->value = 0;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsPbsMobile::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                              bool dualParaboloid, SceneManager *sceneManager )
    {
        HlmsCache retVal = Hlms::preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();

        const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();
        setProperty( PbsMobileProperty::HwGammaRead, capabilities->hasCapability( RSC_HW_GAMMA ) );
        setProperty( PbsMobileProperty::HwGammaWrite, capabilities->hasCapability( RSC_HW_GAMMA ) &&
                                                        renderTarget->isHardwareGammaEnabled() );
        setProperty( PbsMobileProperty::SignedIntTex, capabilities->hasCapability(
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

        mPreparedPass.viewProjMatrix    = projectionMatrix * viewMatrix;
        mPreparedPass.viewMatrix        = viewMatrix;

        mPreparedPass.shadowMaps.clear();

        if( !casterPass )
        {
            int32 numShadowMaps = getProperty( HlmsBaseProp::NumShadowMapLights );
            mPreparedPass.vertexShaderSharedBuffer.clear();
            mPreparedPass.vertexShaderSharedBuffer.reserve( (16 + 2) * numShadowMaps + 16 * 2 );

            //---------------------------------------------------------------------------
            //                          ---- VERTEX SHADER ----
            //---------------------------------------------------------------------------

            //mat4 texWorldViewProj[numShadowMaps]
            for( int32 i=0; i<numShadowMaps; ++i )
            {
                Matrix4 viewProjTex = shadowNode->getViewProjectionMatrix( i );
                for( size_t j=0; j<16; ++j )
                    mPreparedPass.vertexShaderSharedBuffer.push_back( (float)viewProjTex[0][j] );
            }
            //vec2 shadowDepthRange[numShadowMaps]
            for( int32 i=0; i<numShadowMaps; ++i )
            {
                Real fNear, fFar;
                shadowNode->getMinMaxDepthRange( i, fNear, fFar );
                const Real depthRange = fFar - fNear;
                mPreparedPass.vertexShaderSharedBuffer.push_back( fNear );
                mPreparedPass.vertexShaderSharedBuffer.push_back( 1.0f / depthRange );
            }

#ifdef OGRE_GLES2_WORKAROUND_1
            Matrix4 tmp = mPreparedPass.viewProjMatrix.transpose();
#endif
            //mat4 worldView (it's actually view)
            for( size_t i=0; i<16; ++i )
            {
#ifdef OGRE_GLES2_WORKAROUND_1
                mPreparedPass.vertexShaderSharedBuffer.push_back( (float)tmp[0][i] );
#else
                mPreparedPass.vertexShaderSharedBuffer.push_back( (float)mPreparedPass.
                                                                    viewProjMatrix[0][i] );
#endif
            }
#ifdef OGRE_GLES2_WORKAROUND_1
            tmp = viewMatrix.transpose();
            //mat4 worldViewProj (it's actually viewProj)
            for( size_t i=0; i<16; ++i )
                mPreparedPass.vertexShaderSharedBuffer.push_back( (float)tmp[0][i] );
#else
            //mat4 worldViewProj (it's actually viewProj)
            for( size_t i=0; i<16; ++i )
                mPreparedPass.vertexShaderSharedBuffer.push_back( (float)viewMatrix[0][i] );
#endif

            //---------------------------------------------------------------------------
            //                          ---- PIXEL SHADER ----
            //---------------------------------------------------------------------------
            int32 numPssmSplits     = getProperty( HlmsBaseProp::PssmSplits );
            int32 numLights         = getProperty( HlmsBaseProp::LightsSpot );
            int32 numAttenLights    = getProperty( HlmsBaseProp::LightsAttenuation );
            int32 numSpotlights     = getProperty( HlmsBaseProp::LightsSpotParams );
            mPreparedPass.pixelShaderSharedBuffer.clear();
            mPreparedPass.pixelShaderSharedBuffer.reserve( 2 * numShadowMaps + numPssmSplits +
                                                           9 * numLights + 3 * numAttenLights +
                                                           6 * numSpotlights + 9 );

            Matrix3 viewMatrix3, invViewMatrix3;
            viewMatrix.extract3x3Matrix( viewMatrix3 );
            invViewMatrix3 = viewMatrix3.Inverse();

            //vec2 invShadowMapSize
            for( int32 i=0; i<numShadowMaps; ++i )
            {
                //TODO: textures[0] is out of bounds when using shadow atlas. Also see how what
                //changes need to be done so that UV calculations land on the right place
                uint32 texWidth  = shadowNode->getLocalTextures()[i].textures[0]->getWidth();
                uint32 texHeight = shadowNode->getLocalTextures()[i].textures[0]->getHeight();
                mPreparedPass.pixelShaderSharedBuffer.push_back( 1.0f / texWidth );
                mPreparedPass.pixelShaderSharedBuffer.push_back( 1.0f / texHeight );
            }
            //float pssmSplitPoints
            for( int32 i=0; i<numPssmSplits; ++i )
                mPreparedPass.pixelShaderSharedBuffer.push_back( (*shadowNode->getPssmSplits(0))[i+1] );

            if( shadowNode )
            {
                const LightClosestArray &lights = shadowNode->getShadowCastingLights();
                //vec3 lightPosition[numLights]
                for( int32 i=0; i<numLights; ++i )
                {
                    Vector4 lightPos4 = lights[i].light->getAs4DVector();
                    Vector3 lightPos;

                    if( lights[i].light->getType() == Light::LT_DIRECTIONAL )
                        lightPos = viewMatrix3 * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );
                    else
                        lightPos = viewMatrix * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( lightPos.x );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( lightPos.y );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( lightPos.z );
                }
                //vec3 lightDiffuse[numLights]
                for( int32 i=0; i<numLights; ++i )
                {
                    ColourValue colour = lights[i].light->getDiffuseColour() *
                                         lights[i].light->getPowerScale();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.r );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.g );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.b );
                }
                //vec3 lightSpecular[numLights]
                for( int32 i=0; i<numLights; ++i )
                {
                    ColourValue colour = lights[i].light->getSpecularColour() *
                                         lights[i].light->getPowerScale();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.r );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.g );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.b );
                }
                //vec3 attenuation[numAttenLights]
                for( int32 i=numLights - numAttenLights; i<numLights; ++i )
                {
                    Real attenRange     = lights[i].light->getAttenuationRange();
                    Real attenLinear    = lights[i].light->getAttenuationLinear();
                    Real attenQuadratic = lights[i].light->getAttenuationQuadric();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( attenRange );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( attenLinear );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( attenQuadratic );
                }
                //vec3 spotDirection[numSpotlights]
                for( int32 i=numLights - numSpotlights; i<numLights; ++i )
                {
                    Vector3 spotDir = viewMatrix3 * lights[i].light->getDerivedDirection();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( spotDir.x );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( spotDir.y );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( spotDir.z );
                }
                //vec3 spotParams[numSpotlights]
                for( int32 i=numLights - numSpotlights; i<numLights; ++i )
                {
                    Radian innerAngle = lights[i].light->getSpotlightInnerAngle();
                    Radian outerAngle = lights[i].light->getSpotlightOuterAngle();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( 1.0f /
                                        ( cosf( innerAngle.valueRadians() * 0.5f ) -
                                          cosf( outerAngle.valueRadians() * 0.5f ) ) );
                    mPreparedPass.pixelShaderSharedBuffer.push_back(
                                        cosf( outerAngle.valueRadians() * 0.5f ) );
                    mPreparedPass.pixelShaderSharedBuffer.push_back(
                                        lights[i].light->getSpotlightFalloff() );
                }
            }
            else
            {
                int32 numDirectionalLights = getProperty( HlmsBaseProp::LightsDirectional );

                //No shadow maps, only pass directional lights
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();

                //vec3 lightPosition[numLights]
                for( int32 i=0; i<numLights; ++i )
                {
                    Vector4 lightPos4 = globalLightList.lights[i]->getAs4DVector();
                    Vector3 lightPos;
                    if( globalLightList.lights[i]->getType() == Light::LT_DIRECTIONAL )
                        lightPos = viewMatrix3 * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );
                    else
                        lightPos = viewMatrix * Vector3( lightPos4.x, lightPos4.y, lightPos4.z );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( lightPos.x );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( lightPos.y );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( lightPos.z );
                }
                //vec3 lightDiffuse[numLights]
                for( int32 i=0; i<numLights; ++i )
                {
                    ColourValue colour = globalLightList.lights[i]->getDiffuseColour() *
                                         globalLightList.lights[i]->getPowerScale();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.r );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.g );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.b );
                }
                //vec3 lightSpecular[numLights]
                for( int32 i=0; i<numLights; ++i )
                {
                    ColourValue colour = globalLightList.lights[i]->getSpecularColour() *
                                         globalLightList.lights[i]->getPowerScale();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.r );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.g );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( colour.b );
                }
                //vec3 attenuation[numAttenLights]
                for( int32 i=numLights - numAttenLights; i<numLights; ++i )
                {
                    assert( globalLightList.lights[i]->getType() != Light::LT_DIRECTIONAL );
                    Real attenRange     = globalLightList.lights[i]->getAttenuationRange();
                    Real attenLinear    = globalLightList.lights[i]->getAttenuationLinear();
                    Real attenQuadratic = globalLightList.lights[i]->getAttenuationQuadric();
                    mPreparedPass.pixelShaderSharedBuffer.push_back( attenRange );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( attenLinear );
                    mPreparedPass.pixelShaderSharedBuffer.push_back( attenQuadratic );
                }
                //vec3 spotDirection[numSpotlights]
                for( int32 i=numDirectionalLights; i<numLights; ++i )
                {
                    if( globalLightList.lights[i]->getType() == Light::LT_SPOTLIGHT )
                    {
                        Vector3 spotDir = viewMatrix3 * globalLightList.lights[i]->getDerivedDirection();
                        mPreparedPass.pixelShaderSharedBuffer.push_back( spotDir.x );
                        mPreparedPass.pixelShaderSharedBuffer.push_back( spotDir.y );
                        mPreparedPass.pixelShaderSharedBuffer.push_back( spotDir.z );
                    }
                }
                //vec3 spotParams[numSpotlights]
                for( int32 i=numDirectionalLights; i<numLights; ++i )
                {
                    if( globalLightList.lights[i]->getType() == Light::LT_SPOTLIGHT )
                    {
                        Radian innerAngle = globalLightList.lights[i]->getSpotlightInnerAngle();
                        Radian outerAngle = globalLightList.lights[i]->getSpotlightOuterAngle();
                        mPreparedPass.pixelShaderSharedBuffer.push_back( 1.0f /
                                            ( cosf( innerAngle.valueRadians() * 0.5f ) -
                                              cosf( outerAngle.valueRadians() * 0.5f ) ) );
                        mPreparedPass.pixelShaderSharedBuffer.push_back(
                                            cosf( outerAngle.valueRadians() * 0.5f ) );
                        mPreparedPass.pixelShaderSharedBuffer.push_back(
                                            globalLightList.lights[i]->getSpotlightFalloff() );
                    }
                }
            }

            //mat3 invViewMatCubemap
            for( size_t i=0; i<9; ++i )
            {
#ifdef OGRE_GLES2_WORKAROUND_2
                Matrix3 xRot( 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, -1.0f,
                              0.0f, 1.0f, 0.0f );
                xRot = xRot * invViewMatrix3;
                mPreparedPass.pixelShaderSharedBuffer.push_back( (float)xRot[0][i] );
#else
                mPreparedPass.pixelShaderSharedBuffer.push_back( (float)invViewMatrix3[0][i] );
#endif
            }

            mPreparedPass.shadowMaps.reserve( numShadowMaps );
            for( int32 i=0; i<numShadowMaps; ++i )
                mPreparedPass.shadowMaps.push_back( shadowNode->getLocalTextures()[i].textures[0] );
        }
        else
        {
            mPreparedPass.vertexShaderSharedBuffer.clear();
            mPreparedPass.vertexShaderSharedBuffer.reserve( 2 + 16 );
            mPreparedPass.pixelShaderSharedBuffer.clear();

            //vec2 depthRange;
            Real fNear, fFar;
            shadowNode->getMinMaxDepthRange( camera, fNear, fFar );
            const Real depthRange = fFar - fNear;
            mPreparedPass.vertexShaderSharedBuffer.push_back( fNear );
            mPreparedPass.vertexShaderSharedBuffer.push_back( 1.0f / depthRange );

            //mat4 worldViewProj (it's actually viewProj)
            for( size_t i=0; i<16; ++i )
            {
                mPreparedPass.vertexShaderSharedBuffer.push_back( (float)mPreparedPass.
                                                                    viewProjMatrix[0][i] );
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbsMobile::fillBuffersFor( const HlmsCache *cache,
                                          const QueuedRenderable &queuedRenderable,
                                          bool casterPass, uint32 lastCacheHash,
                                          uint32 lastTextureHash )
    {
        GpuProgramParametersSharedPtr vpParams = cache->pso.vertexShader->getDefaultParameters();
        GpuProgramParametersSharedPtr psParams = cache->pso.pixelShader->getDefaultParameters();
        float *vsUniformBuffer = vpParams->getFloatPointer( 0 );
#if _SECURE_SCL
        float *psUniformBuffer = 0;
        if( !psParams->getFloatConstantList().empty() )
            psUniformBuffer = psParams->getFloatPointer( 0 );
#else
        float *psUniformBuffer = psParams->getFloatPointer( 0 );
#endif

        assert( dynamic_cast<const HlmsPbsMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsPbsMobileDatablock *datablock = static_cast<const HlmsPbsMobileDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        if( OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH( lastCacheHash ) != HLMS_PBS )
        {
            //We changed HlmsType, rebind the shared textures.
            FastArray<TexturePtr>::const_iterator itor = mPreparedPass.shadowMaps.begin();
            FastArray<TexturePtr>::const_iterator end  = mPreparedPass.shadowMaps.end();

            if( !casterPass )
            {
                size_t texUnit = 0;
                while( itor != end )
                {
                    mRenderSystem->_setTexture( texUnit, true, (*itor).get() );
                    ++texUnit;
                    ++itor;
                }
            }
        }

        uint16 variabilityMask = GPV_PER_OBJECT;
        size_t psBufferElements = mPreparedPass.pixelShaderSharedBuffer.size() -
                                    ((!casterPass && datablock->mTexture[PBSM_REFLECTION].isNull()) ?
                                                                                            9 : 0);

        bool hasSkeletonAnimation = queuedRenderable.renderable->hasSkeletonAnimation();
        size_t sharedViewTransfElem = hasSkeletonAnimation ? 0 : (16 * (2 - casterPass));

        //Sizes can't be equal (we also add more data)
        assert( mPreparedPass.vertexShaderSharedBuffer.size() - sharedViewTransfElem <
                vpParams->getFloatConstantList().size() );
        assert( ( mPreparedPass.pixelShaderSharedBuffer.size() -
                  ((!casterPass && datablock->mTexture[PBSM_REFLECTION].isNull()) ? 9u : 0u) ) <=
                psParams->getFloatConstantList().size() );

        if( cache->hash != lastCacheHash )
        {
            variabilityMask = GPV_ALL;
            memcpy( vsUniformBuffer, mPreparedPass.vertexShaderSharedBuffer.begin(),
                    sizeof(float) * (mPreparedPass.vertexShaderSharedBuffer.size() - sharedViewTransfElem) );

            memcpy( psUniformBuffer, mPreparedPass.pixelShaderSharedBuffer.begin(),
                    sizeof(float) * psBufferElements );
        }

        vsUniformBuffer += mPreparedPass.vertexShaderSharedBuffer.size() - sharedViewTransfElem;
        psUniformBuffer += psBufferElements;

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------
        if( !hasSkeletonAnimation )
        {
            const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();

            //mat4 worldViewProj
            Matrix4 tmp = mPreparedPass.viewProjMatrix * worldMat;
    #ifdef OGRE_GLES2_WORKAROUND_1
            tmp = tmp.transpose();
#endif
#if !OGRE_DOUBLE_PRECISION
            memcpy( vsUniformBuffer, &tmp, sizeof( Matrix4 ) );
            vsUniformBuffer += 16;
#else
            for( int y = 0; y < 4; ++y )
            {
                for( int x = 0; x < 4; ++x )
                {
                    *vsUniformBuffer++ = tmp[ y ][ x ];
                }
            }
#endif
            //mat4 worldView
            tmp = mPreparedPass.viewMatrix.concatenateAffine( worldMat );
    #ifdef OGRE_GLES2_WORKAROUND_1
            //On GLES2, there is a bug in PowerVR SGX 540 where glProgramUniformMatrix4fvEXT doesn't
            tmp = tmp.transpose();
#endif
#if !OGRE_DOUBLE_PRECISION
            memcpy( vsUniformBuffer, &tmp, sizeof( Matrix4 ) * !casterPass );
            vsUniformBuffer += 16 * !casterPass;
#else
            if( !casterPass )
            {
                for( int y = 0; y < 4; ++y )
                {
                    for( int x = 0; x < 4; ++x )
                    {
                        *vsUniformBuffer++ = tmp[ y ][ x ];
                    }
                }
            }
#endif
        }
        else
        {
            uint16 numWorldTransforms = queuedRenderable.renderable->getNumWorldTransforms();
            assert( numWorldTransforms <= 60u );

            //TODO: Don't rely on a virtual function + make a direct 4x3 copy
            Matrix4 tmp[60];
            queuedRenderable.renderable->getWorldTransforms( tmp );
            for( size_t i=0; i<numWorldTransforms; ++i )
            {
#if !OGRE_DOUBLE_PRECISION
                memcpy( vsUniformBuffer, &tmp[ i ], 12 * sizeof( float ) );
                vsUniformBuffer += 12;
#else
                for( int y = 0; y < 3; ++y )
                {
                    for( int x = 0; x < 4; ++x )
                    {
                        *vsUniformBuffer++ = tmp[ i ][ y ][ x ];
                    }
                }
#endif
            }

            vsUniformBuffer += (60 - numWorldTransforms) * 12;
        }

        if( casterPass )
            *vsUniformBuffer++ = datablock->mShadowConstantBias;

        uint32 retVal;

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------
        //See HlmsPbsMobileDatablock::bakeVariableParameters
        memcpy( psUniformBuffer, &datablock->mRoughness, datablock->mFullParametersBytes[casterPass] );
        psUniformBuffer += datablock->mFullParametersBytes[casterPass] >> 2;

        if( !casterPass )
        {
            if( datablock->mTextureHash != lastTextureHash )
            {
                //Rebind textures
                size_t texUnit = mPreparedPass.shadowMaps.size();
                for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
                {
                    if( !datablock->mTexture[i].isNull() )
                    {
                        mRenderSystem->_setTexture( texUnit, true, datablock->mTexture[i].get() );
                        mRenderSystem->_setHlmsSamplerblock( texUnit, datablock->mSamplerblocks[i] );
                        ++texUnit;
                    }
                }

                mRenderSystem->_disableTextureUnitsFrom( texUnit );
            }

            retVal = datablock->mTextureHash;
        }
        else
        {
            if( lastTextureHash )
                mRenderSystem->_disableTextureUnitsFrom( 0 );
            retVal = 0;
        }

        assert( vsUniformBuffer - vpParams->getFloatPointer( 0 ) == vpParams->getFloatConstantList().size() );
        assert( (!psParams->getFloatConstantList().size() && !psUniformBuffer) ||
                (psUniformBuffer - psParams->getFloatPointer( 0 ) == psParams->getFloatConstantList().size()) );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vpParams, variabilityMask );
        mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, variabilityMask );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbsMobile::fillBuffersFor( const HlmsCache *cache,
                                          const QueuedRenderable &queuedRenderable,
                                          bool casterPass, uint32 lastCacheHash,
                                          CommandBuffer *commandBuffer )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Trying to use fast-path on a mobile implementation. "
                     "Change the RenderQueue settings.",
                     "HlmsPbsMobile::fillBuffersFor" );
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsPbsMobile::createDatablockImpl( IdString datablockName,
                                                       const HlmsMacroblock *macroblock,
                                                       const HlmsBlendblock *blendblock,
                                                       const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsPbsMobileDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
