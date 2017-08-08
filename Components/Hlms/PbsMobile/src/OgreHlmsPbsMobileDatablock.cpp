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

#include "OgreHlmsPbsMobileDatablock.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgrePbsMobileShaderCreationData.h"
#include "OgreStringConverter.h"
#include "OgreTexture.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

namespace Ogre
{
    extern const String c_pbsBlendModes[];
    const String c_pbsBlendModes[] =
    {
        "NormalNonPremul", "NormalPremul", "Add", "Subtract", "Multiply",
        "Multiply2x", "Screen", "Overlay", "Lighten", "Darken", "GrainExtract",
        "GrainMerge", "Difference"
    };

    //-----------------------------------------------------------------------------------
    HlmsPbsMobileDatablock::HlmsPbsMobileDatablock( IdString name, Hlms *creator,
                                              const HlmsMacroblock *macroblock,
                                              const HlmsBlendblock *blendblock,
                                              const HlmsParamVec &params ) :
        HlmsDatablock( name, creator, macroblock, blendblock, params ),
        mRoughness( 0.1f ),
        mkDr( 0.318309886f ), mkDg( 0.318309886f ), mkDb( 0.318309886f ), //Max Diffuse = 1 / PI
        mkSr( 1 ), mkSg( 1 ), mkSb( 1 ),
        mShaderCreationData( 0 )
    {
        mFullParametersBytes[0] = mFullParametersBytes[1] = 0;
        mShaderCreationData = new PbsMobileShaderCreationData();
        memset( mVariableParameters, 0, sizeof( mVariableParameters ) );
        memset( mSamplerblocks, 0, sizeof( mSamplerblocks ) );

        String paramVal;

        if( Hlms::findParamInVec( params, "diffuse", paramVal ) )
        {
            Vector3 val = StringConverter::parseVector3( paramVal, Vector3::UNIT_SCALE );
            setDiffuse( val );
        }

        if( Hlms::findParamInVec( params, "specular", paramVal ) )
        {
            Vector3 val = StringConverter::parseVector3( paramVal, Vector3::UNIT_SCALE );
            mkSr = val.x;
            mkSg = val.y;
            mkSb = val.z;
        }

        if( Hlms::findParamInVec( params, "roughness", paramVal ) )
        {
            mRoughness = StringConverter::parseReal( paramVal, 0.1f );

            if( mRoughness <= 1e-6f )
            {
                LogManager::getSingleton().logMessage( "WARNING: PBS Datablock '" +
                            name.getFriendlyText() + "' Very low roughness values can "
                                                       "cause NaNs in the pixel shader!" );
            }
        }

        if( Hlms::findParamInVec( params, "fresnel", paramVal ) )
        {
            Vector3 val( Vector3::UNIT_SCALE );
            vector<String>::type vec = StringUtil::split( paramVal );

            if( vec.size() > 0 )
            {
                val.x = StringConverter::parseReal( vec[0], 0.818f );

                if( vec.size() == 3 )
                {
                    val.y = StringConverter::parseReal( vec[1], 0.818f );
                    val.z = StringConverter::parseReal( vec[2], 0.818f );
                }

                setIndexOfRefraction( val, vec.size() == 3 );
            }
        }

        if( Hlms::findParamInVec( params, "fresnel_coeff", paramVal ) )
        {
            vector<String>::type vec = StringUtil::split( paramVal );

            if( vec.size() > 0 )
            {
                mShaderCreationData->mFresnelR = StringConverter::parseReal( vec[0], 1.0f );

                if( vec.size() == 3 )
                {
                    mShaderCreationData->mFresnelG = StringConverter::parseReal( vec[1], 1.0f );
                    mShaderCreationData->mFresnelB = StringConverter::parseReal( vec[2], 1.0f );
                    mShaderCreationData->mFresnelTypeSizeBytes = 12;
                }
            }
        }

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        if( Hlms::findParamInVec( params, "diffuse_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DIFFUSE, PBSM_DIFFUSE );

            mSamplerblocks[PBSM_DIFFUSE] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "normal_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_NORMALS, PBSM_NORMAL );
            mSamplerblocks[PBSM_NORMAL] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "specular_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DIFFUSE, PBSM_SPECULAR );
            mSamplerblocks[PBSM_SPECULAR] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "roughness_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_MONOCHROME, PBSM_ROUGHNESS );
            mSamplerblocks[PBSM_ROUGHNESS] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "detail_weight_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL, PBSM_DETAIL_WEIGHT );
            mSamplerblocks[PBSM_DETAIL_WEIGHT] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "reflection_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_ENV_MAP, PBSM_REFLECTION );
            mSamplerblocks[PBSM_REFLECTION] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }

        if( Hlms::findParamInVec( params, "uv_diffuse_map", paramVal ) )
            setTextureUvSource( PBSM_DIFFUSE, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_normal_map", paramVal ) )
            setTextureUvSource( PBSM_NORMAL, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_specular_map", paramVal ) )
            setTextureUvSource( PBSM_SPECULAR, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_roughness_map", paramVal ) )
            setTextureUvSource( PBSM_ROUGHNESS, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_detail_weight_map", paramVal ) )
        {
            setTextureUvSource( PBSM_DETAIL_WEIGHT,
                                StringConverter::parseUnsignedInt( paramVal ) );
        }

        //Detail maps default to wrap mode.
        HlmsSamplerblock detailSamplerRef;
        detailSamplerRef.mU = TAM_WRAP;
        detailSamplerRef.mV = TAM_WRAP;
        detailSamplerRef.mW = TAM_WRAP;

        for( size_t i=0; i<4; ++i )
        {
            String key = "detail_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL,
                            static_cast<PbsMobileTextureTypes>( PBSM_DETAIL0 + i ) );
                mSamplerblocks[PBSM_DETAIL0 + i] = hlmsManager->getSamplerblock( detailSamplerRef );
            }

            key = "detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
                            static_cast<PbsMobileTextureTypes>( PBSM_DETAIL0_NM + i ) );
                mSamplerblocks[PBSM_DETAIL0_NM + i] = hlmsManager->getSamplerblock( detailSamplerRef );
            }

            key = "detail_blend_mode" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                for( size_t j=0; j<NUM_PBSM_BLEND_MODES; ++j )
                {
                    String blendModeLowerCase;
                    blendModeLowerCase.resize( c_pbsBlendModes[j].size() );
                    std::transform( c_pbsBlendModes[j].begin(), c_pbsBlendModes[j].end(),
                                    blendModeLowerCase.begin(), ::tolower );
                    StringUtil::toLowerCase( paramVal );

                    if( blendModeLowerCase == paramVal )
                        setDetailMapBlendMode( i, static_cast<PbsMobileBlendModes>( j ) );
                }
            }

            key = "detail_offset_scale" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                mShaderCreationData->mDetailsOffsetScale[i] = StringConverter::parseVector4(
                            paramVal, mShaderCreationData->mDetailsOffsetScale[i] );
            }

            key = "detail_normal_offset_scale" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                mShaderCreationData->mDetailsOffsetScale[i+4] = StringConverter::parseVector4(
                            paramVal, mShaderCreationData->mDetailsOffsetScale[i+4] );
            }

            key = "uv_detail_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTextureUvSource( static_cast<PbsMobileTextureTypes>( PBSM_DETAIL0 + i ),
                                    StringConverter::parseUnsignedInt( paramVal ) );
            }

            key = "uv_detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTextureUvSource( static_cast<PbsMobileTextureTypes>( PBSM_DETAIL0_NM + i ),
                                    StringConverter::parseUnsignedInt( paramVal ) );
            }
        }

        calculateHash();
        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsMobileDatablock::~HlmsPbsMobileDatablock()
    {
        delete mShaderCreationData;
        mShaderCreationData = 0;

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        if( hlmsManager )
        {
            for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
            {
                if( mSamplerblocks[i] )
                    hlmsManager->destroySamplerblock( mSamplerblocks[i] );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::calculateHash()
    {
        IdString hash;
        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
        {
            if( !mTexture[i].isNull() )
            {
                hash += IdString( mTexture[i]->getName() );
                hash += IdString( mSamplerblocks[i]->mId );
            }
        }

        mTextureHash = hash.mHash;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::bakeVariableParameters(void)
    {
        size_t param = 0;
        //float alpha_test_threshold
        if( mAlphaTestCmp != CMPF_ALWAYS_PASS )
            mVariableParameters[param++] = mAlphaTestThreshold;

        //float roughness
        //vec3 kD;
        //vec3 kS;
        //vec3 F0; or float F0;
        memcpy( &mVariableParameters[param], &mShaderCreationData->mFresnelR,
                mShaderCreationData->mFresnelTypeSizeBytes );
        param += mShaderCreationData->mFresnelTypeSizeBytes >> 2;

        //vec3 atlasOffsets[4]; (up to four, can be zero)
        for( size_t i=0; i<=PBSM_ROUGHNESS; ++i )
        {
            if( !mTexture[i].isNull() )
            {
                memcpy( &mVariableParameters[param], &mShaderCreationData->mUvAtlasParams[i],
                        sizeof( PbsUvAtlasParams ) );
                param += sizeof( PbsUvAtlasParams ) >> 2;
            }
        }

        //float normalWeights[5]; (up to five, can be zero)
        if( mShaderCreationData->mNormalMapWeight != 1.0f && !mTexture[PBSM_NORMAL].isNull() )
            mVariableParameters[param++] = mShaderCreationData->mNormalMapWeight;

        for( size_t i=0; i<4; ++i )
        {
            if( mShaderCreationData->mDetailNormalWeight[i] != 1.0f &&
                !mTexture[PBSM_DETAIL0_NM + i].isNull() )
            {
                mVariableParameters[param++] = mShaderCreationData->mDetailNormalWeight[i];
            }
        }

        bool anyDetailWeight = false;
        for( size_t i=0; i<4 && !anyDetailWeight; ++i )
        {
            if( mShaderCreationData->mDetailWeight[i] != 1.0f &&
                (!mTexture[PBSM_DETAIL0 + i].isNull() || !mTexture[PBSM_DETAIL0_NM + i].isNull()) )
            {
                anyDetailWeight = true;
            }
        }

        //vec4 cDetailWeights;
        if( anyDetailWeight )
        {
            memcpy( &mVariableParameters[param], &mShaderCreationData->mDetailWeight,
                    4 * sizeof(float) );
            param += 4;
        }

        //vec4 detailOffsetScaleD[4];  (up to four, can be zero)
        //vec4 detailOffsetScaleN[4];  (up to four, can be zero)
        for( size_t i=0; i<8; ++i )
        {
            if( mShaderCreationData->mDetailsOffsetScale[i] != Vector4( 0, 0, 1, 1 ) )
            {
                memcpy( &mVariableParameters[param],
                        &mShaderCreationData->mDetailsOffsetScale[i],
                        4 * sizeof(float) );
                param += 4;
            }
        }

        //float roughness
        //vec3 kD;
        //vec3 kS;
        //float alpha_test_threshold;
        //vec3 F0; or float F0;
        //vec3 atlasOffsets[4]; (up to four, can be zero)
        //float normalWeights[5]; (up to five, can be zero)
        //vec4 cDetailWeights;
        //vec4 detailOffsetScaleD[4];  (up to four, can be zero)
        //vec4 detailOffsetScaleN[4];  (up to four, can be zero)
        mFullParametersBytes[0] = 7 * sizeof(float) + (param << 2);
        //float alpha_test_threshold
        mFullParametersBytes[1] = (mAlphaTestCmp != CMPF_ALWAYS_PASS) * sizeof(float);
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setTexture( const String &name,
                                             HlmsTextureManager::TextureMapType textureMapType,
                                             PbsMobileTextureTypes textureType )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                                    createOrRetrieveTexture( name, textureMapType );

        assert( !texLocation.texture->isTextureTypeArray() );

        mTexture[textureType] = texLocation.texture;

        if( textureType <= PBSM_ROUGHNESS )
        {
            mShaderCreationData->mUvAtlasParams[textureType] = textureLocationToAtlasParams(
                                                                                        texLocation );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setDiffuse( const Vector3 &diffuseColour )
    {
        const float invPI = 0.318309886f;
        mkDr = diffuseColour.x * invPI;
        mkDg = diffuseColour.y * invPI;
        mkDb = diffuseColour.z * invPI;
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsMobileDatablock::getDiffuse(void) const
    {
        return Vector3( mkDr, mkDg, mkDb );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setSpecular( const Vector3 &specularColour )
    {
        mkSr = specularColour.x;
        mkSg = specularColour.y;
        mkSb = specularColour.z;
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsMobileDatablock::getSpecular(void) const
    {
        return Vector3( mkSr, mkSg, mkSb );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setRoughness( float roughness )
    {
        mRoughness = roughness;
    }
    //-----------------------------------------------------------------------------------
    float HlmsPbsMobileDatablock::getRoughness(void) const
    {
        return mRoughness;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setIndexOfRefraction( const Vector3 &refractionIdx,
                                                       bool separateFresnel )
    {
        Vector3 fresnel = (1.0f - refractionIdx) / (1.0f + refractionIdx);
        fresnel = fresnel * fresnel;
        setFresnel( fresnel, separateFresnel );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setFresnel( const Vector3 &fresnel, bool separateFresnel )
    {
        uint8 fresnelBytes = 4;
        mShaderCreationData->mFresnelR = fresnel.x;

        if( separateFresnel )
        {
            mShaderCreationData->mFresnelG = fresnel.y;
            mShaderCreationData->mFresnelB = fresnel.z;

            fresnelBytes = 12;
        }

        if( fresnelBytes != mShaderCreationData->mFresnelTypeSizeBytes )
        {
            mShaderCreationData->mFresnelTypeSizeBytes = fresnelBytes;
            flushRenderables();
        }

        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsMobileDatablock::getFresnel(void) const
    {
        return Vector3( mShaderCreationData->mFresnelR, mShaderCreationData->mFresnelG,
                        mShaderCreationData->mFresnelB );
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbsMobileDatablock::hasSeparateFresnel(void) const
    {
        return mShaderCreationData->mFresnelTypeSizeBytes != 4;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setTexture( PbsMobileTextureTypes texType, TexturePtr &newTexture,
                                             const PbsUvAtlasParams &atlasParams )
    {
        bool oldWasNull = mTexture[texType].isNull();
        mTexture[texType] = newTexture;

        if( texType <= PBSM_ROUGHNESS )
            mShaderCreationData->mUvAtlasParams[texType] = atlasParams;

        if( oldWasNull != newTexture.isNull() )
        {
            if( !mSamplerblocks[texType] )
            {
                HlmsSamplerblock samplerBlockRef;
                if( texType >= PBSM_DETAIL0 && texType <= PBSM_DETAIL3_NM )
                {
                    //Detail maps default to wrap mode.
                    samplerBlockRef.mU = TAM_WRAP;
                    samplerBlockRef.mV = TAM_WRAP;
                    samplerBlockRef.mW = TAM_WRAP;
                }

                HlmsManager *hlmsManager = mCreator->getHlmsManager();
                mSamplerblocks[texType] = hlmsManager->getSamplerblock( samplerBlockRef );
            }

            flushRenderables();
        }

        calculateHash();
        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setSamplerblock( PbsMobileTextureTypes texType,
                                                  const HlmsSamplerblock &params )
    {
        const HlmsSamplerblock *oldBlock = mSamplerblocks[texType];

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        mSamplerblocks[texType] = hlmsManager->getSamplerblock( params );

        if( oldBlock )
            hlmsManager->destroySamplerblock( oldBlock );
    }
    //-----------------------------------------------------------------------------------
    const HlmsSamplerblock* HlmsPbsMobileDatablock::getSamplerblock(
                                                                PbsMobileTextureTypes texType ) const
    {
        return mSamplerblocks[texType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setTextureUvSource( PbsMobileTextureTypes sourceType, uint8 uvSet )
    {
        if( uvSet >= 8 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "UV set must be in rage in range [0; 8)",
                         "HlmsPbsMobileDatablock::setTextureUvSource" );
        }

        if( sourceType >= NUM_PBSM_SOURCES )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Invalid sourceType",
                         "HlmsPbsMobileDatablock::setTextureUvSource" );
        }

        if( mShaderCreationData->uvSource[sourceType] != uvSet )
        {
            mShaderCreationData->uvSource[sourceType] = uvSet;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setDetailMapBlendMode( uint8 detailMapIdx,
                                                        PbsMobileBlendModes blendMode )
    {
        assert( detailMapIdx < 4 );

        if( mShaderCreationData->blendModes[detailMapIdx] != blendMode )
        {
            mShaderCreationData->blendModes[detailMapIdx] = blendMode;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setDetailNormalWeight( uint8 detailNormalMapIdx, Real weight )
    {
        assert( detailNormalMapIdx < 4 );

        bool wasOne = mShaderCreationData->mDetailNormalWeight[detailNormalMapIdx] == 1.0f;
        mShaderCreationData->mDetailNormalWeight[detailNormalMapIdx] = weight;

        if( wasOne != (mShaderCreationData->mDetailNormalWeight[detailNormalMapIdx] == 1.0f) )
        {
            flushRenderables();
            bakeVariableParameters();
        }
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsMobileDatablock::getDetailNormalWeight( uint8 detailNormalMapIdx ) const
    {
        assert( detailNormalMapIdx < 4 );
        return mShaderCreationData->mDetailNormalWeight[detailNormalMapIdx];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setNormalMapWeight( Real weight )
    {
        bool wasDisabled = mShaderCreationData->mNormalMapWeight == 1.0f;
        mShaderCreationData->mNormalMapWeight = weight;

        if( wasDisabled != (mShaderCreationData->mNormalMapWeight == 1.0f) )
        {
            flushRenderables();
            bakeVariableParameters();
        }
    }
    //-----------------------------------------------------------------------------------
    int HlmsPbsMobileDatablock::_calculateNumUvAtlas( bool casterPass ) const
    {
        int retVal = 0;

        if( !casterPass )
        {
            for( size_t i=0; i<=PBSM_ROUGHNESS; ++i )
                retVal += !mTexture[i].isNull();
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsMobileDatablock::getNormalMapWeight(void) const
    {
        return mShaderCreationData->mNormalMapWeight;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setDetailMapWeight( uint8 detailMap, Real weight )
    {
        assert( detailMap < 4 );
        bool wasDisabled = mShaderCreationData->mDetailWeight[detailMap] == 1.0f;

        mShaderCreationData->mDetailWeight[detailMap] = weight;

        if( wasDisabled != (mShaderCreationData->mDetailWeight[detailMap] == 1.0f) )
            flushRenderables();

        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsMobileDatablock::getDetailMapWeight( uint8 detailMap ) const
    {
        assert( detailMap < 4 );
        return mShaderCreationData->mDetailWeight[detailMap];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setDetailMapOffsetScale( uint8 detailMap, const Vector4 &offsetScale )
    {
        assert( detailMap < 8 );
        bool wasDisabled = mShaderCreationData->mDetailsOffsetScale[detailMap] == Vector4( 0, 0, 1, 1 );

        mShaderCreationData->mDetailsOffsetScale[detailMap] = offsetScale;

        if( wasDisabled !=
                (mShaderCreationData->mDetailsOffsetScale[detailMap] == Vector4( 0, 0, 1, 1 )) )
        {
            flushRenderables();
        }

        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    const Vector4& HlmsPbsMobileDatablock::getDetailMapOffsetScale( uint8 detailMap ) const
    {
        assert( detailMap < 8 );
        return mShaderCreationData->mDetailsOffsetScale[detailMap];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setAlphaTest( CompareFunction compareFunction )
    {
        HlmsDatablock::setAlphaTest( compareFunction );
        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setAlphaTestThreshold( float threshold )
    {
        HlmsDatablock::setAlphaTestThreshold( threshold );
        bakeVariableParameters();
    }
    //-----------------------------------------------------------------------------------
    PbsUvAtlasParams HlmsPbsMobileDatablock::textureLocationToAtlasParams(
                                                const HlmsTextureManager::TextureLocation &texLocation )
    {
        PbsUvAtlasParams retVal;
        retVal.uOffset   = texLocation.xIdx / (float)texLocation.divisor;
        retVal.vOffset   = texLocation.yIdx / (float)texLocation.divisor;
        retVal.invDivisor= 1.0f / texLocation.divisor;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureMapType HlmsPbsMobileDatablock::suggestMapTypeBasedOnTextureType(
                                                                        PbsMobileTextureTypes type )
    {
        HlmsTextureManager::TextureMapType retVal;
        switch( type )
        {
        default:
        case PBSM_DIFFUSE:
        case PBSM_SPECULAR:
        case PBSM_DETAIL_WEIGHT:
            retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
            break;

        case PBSM_NORMAL:
            retVal = HlmsTextureManager::TEXTURE_TYPE_NORMALS;
            break;

        case PBSM_ROUGHNESS:
            retVal = HlmsTextureManager::TEXTURE_TYPE_MONOCHROME;
            break;

        case PBSM_DETAIL0:
        case PBSM_DETAIL1:
        case PBSM_DETAIL2:
        case PBSM_DETAIL3:
            retVal = HlmsTextureManager::TEXTURE_TYPE_DETAIL;
            break;

        case PBSM_DETAIL0_NM:
        case PBSM_DETAIL1_NM:
        case PBSM_DETAIL2_NM:
        case PBSM_DETAIL3_NM:
            retVal = HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP;
            break;

        case PBSM_REFLECTION:
            retVal = HlmsTextureManager::TEXTURE_TYPE_ENV_MAP;
            break;
        }

        return retVal;
    }
}
