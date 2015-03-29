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

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreTexture.h"
#include "OgreLogManager.h"

namespace Ogre
{
    extern const String c_pbsBlendModes[];
    const String c_pbsBlendModes[] =
    {
        "NormalNonPremul", "NormalPremul", "Add", "Subtract", "Multiply",
        "Multiply2x", "Screen", "Overlay", "Lighten", "Darken", "GrainExtract",
        "GrainMerge", "Difference"
    };

    const size_t HlmsPbsDatablock::MaterialSizeInGpu          = 52 * 4 + NUM_PBSM_TEXTURE_TYPES * 2;
    const size_t HlmsPbsDatablock::MaterialSizeInGpuAligned   = alignToNextMultiple(
                                                                    HlmsPbsDatablock::MaterialSizeInGpu,
                                                                    4 * 4 );

    //-----------------------------------------------------------------------------------
    HlmsPbsDatablock::HlmsPbsDatablock( IdString name, HlmsPbs *creator,
                                        const HlmsMacroblock *macroblock,
                                        const HlmsBlendblock *blendblock,
                                        const HlmsParamVec &params ) :
        HlmsDatablock( name, creator, macroblock, blendblock, params ),
        mFresnelTypeSizeBytes( 4 ),
        mkDr( 0.318309886f ), mkDg( 0.318309886f ), mkDb( 0.318309886f ), //Max Diffuse = 1 / PI
        _padding0( 0 ),
        mkSr( 1 ), mkSg( 1 ), mkSb( 1 ),
        mRoughness( 0.1f ),
        mFresnelR( 0.818f ), mFresnelG( 0.818f ), mFresnelB( 0.818f ),
        mNormalMapWeight( 1.0f ),
        mBrdf( PbsBrdf::Default )
    {
        memset( mUvSource, 0, sizeof( mUvSource ) );
        memset( mBlendModes, 0, sizeof( mBlendModes ) );

        mDetailNormalWeight[0] = mDetailNormalWeight[1] = 1.0f;
        mDetailNormalWeight[2] = mDetailNormalWeight[3] = 1.0f;
        mDetailWeight[0] = mDetailWeight[1] = mDetailWeight[2] = mDetailWeight[3] = 1.0f;
        for( size_t i=0; i<8; ++i )
            mDetailsOffsetScale[i] = Vector4( 0, 0, 1, 1 );

        memset( mTexIndices, 0, sizeof( mTexIndices ) );
        memset( mSamplerblocks, 0, sizeof( mSamplerblocks ) );

        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
            mTexToBakedTextureIdx[i] = NUM_PBSM_TEXTURE_TYPES;

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

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        PbsBakedTexture textures[NUM_PBSM_TEXTURE_TYPES];

        if( Hlms::findParamInVec( params, "diffuse_map", paramVal ) )
        {
            textures[PBSM_DIFFUSE].texture = setTexture( paramVal,
                                                         HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
            mSamplerblocks[PBSM_DIFFUSE] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "normal_map", paramVal ) )
        {
            textures[PBSM_NORMAL].texture = setTexture( paramVal,
                                                        HlmsTextureManager::TEXTURE_TYPE_NORMALS );
            mSamplerblocks[PBSM_NORMAL] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "specular_map", paramVal ) )
        {
            textures[PBSM_SPECULAR].texture = setTexture( paramVal,
                                                          HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
            mSamplerblocks[PBSM_SPECULAR] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "roughness_map", paramVal ) )
        {
            textures[PBSM_ROUGHNESS].texture = setTexture( paramVal,
                                                           HlmsTextureManager::TEXTURE_TYPE_MONOCHROME );
            mSamplerblocks[PBSM_ROUGHNESS] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "detail_weight_map", paramVal ) )
        {
            textures[PBSM_DETAIL_WEIGHT].texture = setTexture( paramVal,
                                                               HlmsTextureManager::TEXTURE_TYPE_DETAIL );
            mSamplerblocks[PBSM_DETAIL_WEIGHT] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "reflection_map", paramVal ) )
        {
            textures[PBSM_REFLECTION].texture = setTexture( paramVal,
                                                            HlmsTextureManager::TEXTURE_TYPE_ENV_MAP );
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
                textures[PBSM_DETAIL0 + i].texture = setTexture(
                            paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL );
                mSamplerblocks[PBSM_DETAIL0 + i] = hlmsManager->getSamplerblock( detailSamplerRef );
            }

            key = "detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                textures[PBSM_DETAIL0_NM + i].texture = setTexture(
                            paramVal, HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP );
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
                        setDetailMapBlendMode( i, static_cast<PbsBlendModes>( j ) );
                }
            }

            key = "detail_offset_scale" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                mDetailsOffsetScale[i] = StringConverter::parseVector4( paramVal,
                                                                        mDetailsOffsetScale[i] );
            }

            key = "detail_normal_offset_scale" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                mDetailsOffsetScale[i+4] = StringConverter::parseVector4( paramVal,
                                                                          mDetailsOffsetScale[i+4] );
            }

            key = "uv_detail_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTextureUvSource( static_cast<PbsTextureTypes>( PBSM_DETAIL0 + i ),
                                    StringConverter::parseUnsignedInt( paramVal ) );
            }

            key = "uv_detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                setTextureUvSource( static_cast<PbsTextureTypes>( PBSM_DETAIL0_NM + i ),
                                    StringConverter::parseUnsignedInt( paramVal ) );
            }
        }

        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
            textures[i].samplerBlock = mSamplerblocks[i];

        bakeTextures( textures );

        calculateHash();

        creator->requestSlot( /*mTextureHash*/0, this, false );
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsDatablock::~HlmsPbsDatablock()
    {
        if( mAssignedPool )
            static_cast<HlmsPbs*>(mCreator)->releaseSlot( this );

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
    void HlmsPbsDatablock::calculateHash()
    {
        IdString hash;

        PbsBakedTextureArray::const_iterator itor = mBakedTextures.begin();
        PbsBakedTextureArray::const_iterator end  = mBakedTextures.end();

        while( itor != end )
        {
            hash += IdString( itor->texture->getName() );
            hash += IdString( itor->samplerBlock->mId );

            ++itor;
        }

        if( mTextureHash != hash.mHash )
        {
            mTextureHash = hash.mHash;
            static_cast<HlmsPbs*>(mCreator)->requestSlot( /*mTextureHash*/0, this, false );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::scheduleConstBufferUpdate(void)
    {
        static_cast<HlmsPbs*>(mCreator)->scheduleForUpdate( this );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::uploadToConstBuffer( char *dstPtr )
    {
        _padding0 = mAlphaTestThreshold;
        memcpy( dstPtr, &mkDr, MaterialSizeInGpu );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::decompileBakedTextures( PbsBakedTexture outTextures[NUM_PBSM_TEXTURE_TYPES] )
    {
        //Decompile the baked textures to know which texture is assigned to each type.
        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
        {
            uint8 idx = mTexToBakedTextureIdx[i];

            if( idx < NUM_PBSM_TEXTURE_TYPES )
            {
                outTextures[i] = PbsBakedTexture( mBakedTextures[idx].texture, mSamplerblocks[i] );
            }
            else
            {
                //The texture may be null, but the samplerblock information may still be there.
                outTextures[i] = PbsBakedTexture( TexturePtr(), mSamplerblocks[i] );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::bakeTextures( const PbsBakedTexture textures[NUM_PBSM_TEXTURE_TYPES] )
    {
        //The shader might need to be recompiled (mTexToBakedTextureIdx changed).
        //We'll need to flush.
        //Most likely mTexIndices also changed, so we need to update the const buffers as well
        mBakedTextures.clear();

        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
        {
            if( !textures[i].texture.isNull() )
            {
                PbsBakedTextureArray::const_iterator itor = std::find( mBakedTextures.begin(),
                                                                       mBakedTextures.end(),
                                                                       textures[i] );

                if( itor == mBakedTextures.end() )
                {
                    mTexToBakedTextureIdx[i] = mBakedTextures.size();
                    mBakedTextures.push_back( textures[i] );
                }
                else
                {
                    mTexToBakedTextureIdx[i] = itor - mBakedTextures.begin();
                }
            }
            else
            {
                mTexToBakedTextureIdx[i] = NUM_PBSM_TEXTURE_TYPES;
            }
        }

        calculateHash();
        flushRenderables();
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    TexturePtr HlmsPbsDatablock::setTexture( const String &name,
                                             HlmsTextureManager::TextureMapType textureMapType )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                                    createOrRetrieveTexture( name, textureMapType );

        assert( texLocation.texture->isTextureTypeArray() );

        mTexIndices[textureMapType] = texLocation.xIdx;

        return texLocation.texture;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDiffuse( const Vector3 &diffuseColour )
    {
        const float invPI = 0.318309886f;
        mkDr = diffuseColour.x * invPI;
        mkDg = diffuseColour.y * invPI;
        mkDb = diffuseColour.z * invPI;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsDatablock::getDiffuse(void) const
    {
        return Vector3( mkDr, mkDg, mkDb );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setSpecular( const Vector3 &specularColour )
    {
        mkSr = specularColour.x;
        mkSg = specularColour.y;
        mkSb = specularColour.z;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsDatablock::getSpecular(void) const
    {
        return Vector3( mkSr, mkSg, mkSb );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setRoughness( float roughness )
    {
        mRoughness = roughness;
        if( mRoughness <= 1e-6f )
        {
            LogManager::getSingleton().logMessage( "WARNING: PBS Datablock '" +
                        mName.getFriendlyText() + "' Very low roughness values can "
                                                  "cause NaNs in the pixel shader!" );
        }
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    float HlmsPbsDatablock::getRoughness(void) const
    {
        return mRoughness;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setIndexOfRefraction( const Vector3 &refractionIdx,
                                                       bool separateFresnel )
    {
        Vector3 fresnel = (1.0f - refractionIdx) / (1.0f + refractionIdx);
        fresnel = fresnel * fresnel;
        setFresnel( fresnel, separateFresnel );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setFresnel( const Vector3 &fresnel, bool separateFresnel )
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

        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsDatablock::getFresnel(void) const
    {
        return Vector3( mFresnelR, mFresnelG, mFresnelB );
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbsDatablock::hasSeparateFresnel(void) const
    {
        return mFresnelTypeSizeBytes != 4;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setTexture( PbsTextureTypes texType, uint16 arrayIndex,
                                       const TexturePtr &newTexture, const HlmsSamplerblock *refParams )
    {
        PbsBakedTexture textures[NUM_PBSM_TEXTURE_TYPES];

        //Decompile the baked textures to know which texture is assigned to each type.
        decompileBakedTextures( textures );

        //Set the new samplerblock
        if( refParams )
        {
            HlmsManager *hlmsManager = mCreator->getHlmsManager();
            const HlmsSamplerblock *oldSamplerblock = mSamplerblocks[texType];
            mSamplerblocks[texType] = hlmsManager->getSamplerblock( *refParams );

            if( oldSamplerblock )
                hlmsManager->destroySamplerblock( oldSamplerblock );
        }
        else if( !newTexture.isNull() && !mSamplerblocks[texType] )
        {
            //Adding a texture, but the samplerblock doesn't exist. Create a default one.
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

        PbsBakedTexture oldTex = textures[texType];

        //Set the texture and make the samplerblock changes to take effect
        textures[texType].texture = newTexture;
        textures[texType].samplerBlock = mSamplerblocks[texType];
        mTexIndices[texType] = arrayIndex;

        if( oldTex == textures[texType] )
        {
            //Only the array index changed. Just update our constant buffer.
            scheduleConstBufferUpdate();
        }
        else
        {
            bakeTextures( textures );
        }
    }
    //-----------------------------------------------------------------------------------
    TexturePtr HlmsPbsDatablock::getTexture( PbsTextureTypes texType ) const
    {
        TexturePtr retVal;

        if( mTexToBakedTextureIdx[texType] < mBakedTextures.size() )
            retVal = mBakedTextures[mTexToBakedTextureIdx[texType]].texture;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    TexturePtr HlmsPbsDatablock::getTexture( size_t texType ) const
    {
        return getTexture( static_cast<PbsTextureTypes>( texType ) );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setSamplerblock( PbsTextureTypes texType, const HlmsSamplerblock &params )
    {
        const HlmsSamplerblock *oldSamplerblock = mSamplerblocks[texType];
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        mSamplerblocks[texType] = hlmsManager->getSamplerblock( params );

        if( oldSamplerblock )
            hlmsManager->destroySamplerblock( oldSamplerblock );

        if( oldSamplerblock != mSamplerblocks[texType] )
        {
            PbsBakedTexture textures[NUM_PBSM_TEXTURE_TYPES];
            decompileBakedTextures( textures );
            bakeTextures( textures );
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsSamplerblock* HlmsPbsDatablock::getSamplerblock( PbsTextureTypes texType ) const
    {
        return mSamplerblocks[texType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setTextureUvSource( PbsTextureTypes sourceType, uint8 uvSet )
    {
        if( uvSet >= 8 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "UV set must be in rage in range [0; 8)",
                         "HlmsPbsDatablock::setTextureUvSource" );
        }

        if( sourceType >= NUM_PBSM_SOURCES )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Invalid sourceType",
                         "HlmsPbsDatablock::setTextureUvSource" );
        }

        if( mUvSource[sourceType] != uvSet )
        {
            mUvSource[sourceType] = uvSet;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDetailMapBlendMode( uint8 detailMap, PbsBlendModes blendMode )
    {
        assert( detailMap < 4 );

        if( mBlendModes[detailMap] != blendMode )
        {
            mBlendModes[detailMap] = blendMode;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDetailNormalWeight( uint8 detailNormalMap, Real weight )
    {
        assert( detailNormalMap < 4 );

        bool wasOne = mDetailNormalWeight[detailNormalMap] == 1.0f;
        mDetailNormalWeight[detailNormalMap] = weight;

        if( wasOne != (mDetailNormalWeight[detailNormalMap] == 1.0f) )
        {
            flushRenderables();
            scheduleConstBufferUpdate();
        }
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsDatablock::getDetailNormalWeight( uint8 detailNormalMap ) const
    {
        assert( detailNormalMap < 4 );
        return mDetailNormalWeight[detailNormalMap];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setNormalMapWeight( Real weight )
    {
        bool wasDisabled = mNormalMapWeight == 1.0f;
        mNormalMapWeight = weight;

        if( wasDisabled != (mNormalMapWeight == 1.0f) )
        {
            flushRenderables();
            scheduleConstBufferUpdate();
        }
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsDatablock::getNormalMapWeight(void) const
    {
        return mNormalMapWeight;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDetailMapWeight( uint8 detailMap, Real weight )
    {
        assert( detailMap < 4 );
        bool wasDisabled = mDetailWeight[detailMap] == 1.0f;

        mDetailWeight[detailMap] = weight;

        if( wasDisabled != (mDetailWeight[detailMap] == 1.0f) )
            flushRenderables();

        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsDatablock::getDetailMapWeight( uint8 detailMap ) const
    {
        assert( detailMap < 4 );
        return mDetailWeight[detailMap];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDetailMapOffsetScale( uint8 detailMap, const Vector4 &offsetScale )
    {
        assert( detailMap < 8 );
        bool wasDisabled = mDetailsOffsetScale[detailMap] == Vector4( 0, 0, 1, 1 );

        mDetailsOffsetScale[detailMap] = offsetScale;

        if( wasDisabled != (mDetailsOffsetScale[detailMap] == Vector4( 0, 0, 1, 1 )) )
        {
            flushRenderables();
        }

        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    const Vector4& HlmsPbsDatablock::getDetailMapOffsetScale( uint8 detailMap ) const
    {
        assert( detailMap < 8 );
        return mDetailsOffsetScale[detailMap];
    }
    //-----------------------------------------------------------------------------------
    uint8 HlmsPbsDatablock::getBakedTextureIdx( PbsTextureTypes texType ) const
    {
        return mTexToBakedTextureIdx[texType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setAlphaTestThreshold( float threshold )
    {
        HlmsDatablock::setAlphaTestThreshold( threshold );
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setBrdf( PbsBrdf::PbsBrdf brdf )
    {
        if( mBrdf != brdf )
        {
            mBrdf = brdf;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbsDatablock::getBrdf(void) const
    {
        return mBrdf;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::importUnity( const Vector3 &diffuse, const Vector3 &specular, Real roughness,
                                        bool changeBrdf )
    {
        if( changeBrdf )
        {
            //Set the BRDF that matches Unity the closest.
            setBrdf( PbsBrdf::DefaultUncorrelated );
        }

        setDiffuse( diffuse );

        //Unity uses coloured fresnel as if it were specular. So we need to
        //set our kS to white and use the fresnel for colouring instead.
        setSpecular( Vector3::UNIT_SCALE );
        bool separateFresnel = false;
        if( Math::Abs( specular.x - specular.y ) <= 1e-5f &&
            Math::Abs( specular.y - specular.z ) <= 1e-5f )
        {
            separateFresnel = true;
        }

        setFresnel( specular, separateFresnel );
        setRoughness( roughness );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::importUnity( const Vector3 &colour, Real metallic, Real roughness, bool changeBrdf )
    {
        Vector3 fresnel = Math::lerp( Vector3( 0.03f ), colour, metallic );
        importUnity( colour, fresnel, roughness, changeBrdf );
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureMapType HlmsPbsDatablock::suggestMapTypeBasedOnTextureType(
                                                                        PbsTextureTypes type )
    {
        HlmsTextureManager::TextureMapType retVal;
        switch( type )
        {
        default:
        case PBSM_DIFFUSE:
        case PBSM_SPECULAR:
        case PBSM_DETAIL_WEIGHT:
        case PBSM_DETAIL0:
        case PBSM_DETAIL1:
        case PBSM_DETAIL2:
        case PBSM_DETAIL3:
            retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
            break;

        case PBSM_NORMAL:
        case PBSM_DETAIL0_NM:
        case PBSM_DETAIL1_NM:
        case PBSM_DETAIL2_NM:
        case PBSM_DETAIL3_NM:
            retVal = HlmsTextureManager::TEXTURE_TYPE_NORMALS;
            break;

        case PBSM_ROUGHNESS:
            retVal = HlmsTextureManager::TEXTURE_TYPE_MONOCHROME;
            break;

        case PBSM_REFLECTION:
            retVal = HlmsTextureManager::TEXTURE_TYPE_ENV_MAP;
            break;
        }

        return retVal;
    }
}
