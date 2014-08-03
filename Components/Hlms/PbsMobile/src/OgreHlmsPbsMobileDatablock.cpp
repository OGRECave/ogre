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
#include "OgreTexture.h"
#include "OgreLogManager.h"

namespace Ogre
{
    extern const String c_blendModes[];
    const String c_blendModes[] =
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
        mFresnelTypeSizeBytes( 4 ),
        mNumUvAtlas( 0 ),
        mNumUvAtlasCaster( 0 ),
        mRoughness( 0.1f ),
        mkDr( 0.318309886f ), mkDg( 0.318309886f ), mkDb( 0.318309886f ), //Max Diffuse = 1 / PI
        mkSr( 1 ), mkSg( 1 ), mkSb( 1 ),
        mFresnelR( 0.818f ), mFresnelG( 0.818f ), mFresnelB( 0.818f ),
        mShaderCreationData( 0 )
    {
        mShaderCreationData = new PbsMobileShaderCreationData();

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
                mFresnelR = StringConverter::parseReal( vec[0], 1.0f );

                if( vec.size() == 3 )
                {
                    mFresnelG = StringConverter::parseReal( vec[1], 1.0f );
                    mFresnelB = StringConverter::parseReal( vec[2], 1.0f );
                    mFresnelTypeSizeBytes = 12;
                }
            }
        }

        if( Hlms::findParamInVec( params, "diffuse_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
                        mTexture[PBSM_DIFFUSE], &mUvAtlasParams[mNumUvAtlas++] );
        }
        if( Hlms::findParamInVec( params, "normal_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_NORMALS,
                        mTexture[PBSM_NORMAL], &mUvAtlasParams[mNumUvAtlas++] );
        }
        if( Hlms::findParamInVec( params, "specular_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
                        mTexture[PBSM_SPECULAR], &mUvAtlasParams[mNumUvAtlas++] );
        }
        if( Hlms::findParamInVec( params, "roughness_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_MONOCHROME,
                        mTexture[PBSM_ROUGHNESS], &mUvAtlasParams[mNumUvAtlas++] );
        }
        if( Hlms::findParamInVec( params, "detail_weight_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL,
                        mTexture[PBSM_DETAIL_WEIGHT], 0 );
        }
        if( Hlms::findParamInVec( params, "reflection_map", paramVal ) )
        {
            setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_ENV_MAP,
                        mTexture[PBSM_REFLECTION], 0 );
        }

        if( Hlms::findParamInVec( params, "uv_diffuse_map", paramVal ) )
            setTextureUvSource( PBSM_SOURCE_DIFFUSE, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_normal_map", paramVal ) )
            setTextureUvSource( PBSM_SOURCE_NORMAL, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_specular_map", paramVal ) )
            setTextureUvSource( PBSM_SOURCE_SPECULAR, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_roughness_map", paramVal ) )
            setTextureUvSource( PBSM_SOURCE_ROUGHNESS, StringConverter::parseUnsignedInt( paramVal ) );
        if( Hlms::findParamInVec( params, "uv_detail_weight_map", paramVal ) )
        {
            setTextureUvSource( PBSM_SOURCE_DETAIL_WEIGHT,
                                StringConverter::parseUnsignedInt( paramVal ) );
        }

        for( size_t i=0; i<4; ++i )
        {
            String key = "detail_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL,
                            mTexture[PBSM_DETAIL0 + i], 0 );
            }

            key = "detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTexture( paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
                            mTexture[PBSM_DETAIL0_NM + i], 0 );
            }

            key = "detail_blend_mode" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                for( size_t j=0; j<NUM_PBSM_BLEND_MODES; ++j )
                {
                    String blendModeLowerCase;
                    blendModeLowerCase.resize( c_blendModes[j].size() );
                    std::transform( c_blendModes[j].begin(), c_blendModes[j].end(),
                                    blendModeLowerCase.begin(), ::tolower );
                    StringUtil::toLowerCase( paramVal );

                    if( blendModeLowerCase == paramVal )
                        setDetailMapBlendMode( i, static_cast<PbsMobileBlendModes>( j ) );
                }
            }

            key = "uv_detail_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTextureUvSource( static_cast<PbsMobileUvSourceType>( PBSM_SOURCE_DETAIL0 + i ),
                                    StringConverter::parseUnsignedInt( paramVal ) );
            }

            key = "uv_detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTextureUvSource( static_cast<PbsMobileUvSourceType>( PBSM_SOURCE_DETAIL0_NM + i ),
                                    StringConverter::parseUnsignedInt( paramVal ) );
            }
        }

        calculateHash();
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsMobileDatablock::~HlmsPbsMobileDatablock()
    {
        delete mShaderCreationData;
        mShaderCreationData = 0;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::calculateHash()
    {
        IdString hash;
        for( size_t i=0; i<PBSM_MAX_TEXTURE_TYPES; ++i )
        {
            if( !mTexture[i].isNull() )
                hash += IdString( mTexture[i]->getName() );
        }

        mTextureHash = hash.mHash;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setTexture( const String &name,
                                             HlmsTextureManager::TextureMapType textureMapType,
                                             TexturePtr &outTexture, UvAtlasParams *outAtlasParams )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManger();
        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                                    createOrRetrieveTexture( name, textureMapType );

        assert( !texLocation.texture->isTextureTypeArray() );

        outTexture = texLocation.texture;

        if( outAtlasParams )
        {
            outAtlasParams->uOffset   = texLocation.xIdx / (float)texLocation.divisor;
            outAtlasParams->vOffset   = texLocation.yIdx / (float)texLocation.divisor;
            outAtlasParams->invDivisor= 1.0f / texLocation.divisor;
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
        mFresnelR = fresnel.x;

        if( separateFresnel )
        {
            mFresnelG = fresnel.y;
            mFresnelB = fresnel.z;

            fresnelBytes = 12;
        }

        if( fresnelBytes != mFresnelTypeSizeBytes )
        {
            mFresnelTypeSizeBytes = fresnelBytes;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsMobileDatablock::getFresnel(void) const
    {
        return Vector3( mFresnelR, mFresnelG, mFresnelB );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setTexture( PbsMobileTextureTypes texType, TexturePtr &newTexture,
                                             const UvAtlasParams &atlasParams )
    {
        bool oldTexExisted = mTexture[texType].isNull();
        mTexture[texType] = newTexture;

        if( texType <= PBSM_ROUGHNESS )
        {
            size_t uvAtlasIdx = 0;

            for( size_t i=0; i<texType; ++i )
                uvAtlasIdx += mTexture[i].isNull();

            if( oldTexExisted != newTexture.isNull() )
            {
                if( !oldTexExisted )
                {
                    //We need to make room for our params
                    memmove( mUvAtlasParams + uvAtlasIdx + 1, mUvAtlasParams + uvAtlasIdx,
                             sizeof(UvAtlasParams) * (mNumUvAtlas - uvAtlasIdx - 1) );
                    ++mNumUvAtlas;
                }
                else
                {
                    //We're out, we need to keep everything contiguous
                    memmove( mUvAtlasParams + uvAtlasIdx, mUvAtlasParams + uvAtlasIdx + 1,
                             sizeof(UvAtlasParams) * (mNumUvAtlas - uvAtlasIdx - 1) );
                    --mNumUvAtlas;
                }
            }

            if( !newTexture.isNull() )
            {
                mUvAtlasParams[uvAtlasIdx] = atlasParams;
            }
        }

        if( oldTexExisted != newTexture.isNull() )
            flushRenderables();

        calculateHash();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setTextureUvSource( PbsMobileUvSourceType sourceType, uint8 uvSet )
    {
        if( uvSet >= 8 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "UV set must be in rage in range [0; 8)",
                         "HlmsPbsMobileDatablock::setTextureUvSource" );
        }

        if( mShaderCreationData->uvSource[sourceType] != uvSet )
        {
            mShaderCreationData->uvSource[sourceType] = uvSet;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsMobileDatablock::setDetailMapBlendMode( uint8 detailMap, PbsMobileBlendModes blendMode )
    {
        if( detailMap >= 4 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Details maps are in range [0; 4)",
                         "HlmsPbsMobileDatablock::setDetailMapBlendMode" );
        }

        if( mShaderCreationData->blendModes[detailMap] != blendMode )
        {
            mShaderCreationData->blendModes[detailMap] = blendMode;
            flushRenderables();
        }
    }
}
