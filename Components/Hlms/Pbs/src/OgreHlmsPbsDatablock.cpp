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
#include "OgreTextureManager.h"
#include "OgreLogManager.h"
#include "Cubemaps/OgreCubemapProbe.h"

#include "OgreHlmsPbsDatablock.cpp.inc"

namespace Ogre
{
    extern const String c_pbsBlendModes[];
    const String c_pbsBlendModes[] =
    {
        "NormalNonPremul", "NormalPremul", "Add", "Subtract", "Multiply",
        "Multiply2x", "Screen", "Overlay", "Lighten", "Darken", "GrainExtract",
        "GrainMerge", "Difference"
    };

    const size_t HlmsPbsDatablock::MaterialSizeInGpu          = 56 * 4 + NUM_PBSM_TEXTURE_TYPES * 2;
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
        mTwoSided( false ),
        mUseAlphaFromTextures( true ),
        mWorkflow( SpecularWorkflow ),
        mReceiveShadows( true ),
        mTransparencyMode( None ),
        mkDr( 0.318309886f ), mkDg( 0.318309886f ), mkDb( 0.318309886f ), //Max Diffuse = 1 / PI
        _padding0( 1 ),
        mkSr( 1 ), mkSg( 1 ), mkSb( 1 ),
        mRoughness( 1.0f ),
        mFresnelR( 0.818f ), mFresnelG( 0.818f ), mFresnelB( 0.818f ),
        mTransparencyValue( 1.0f ),
        mNormalMapWeight( 1.0f ),
        mCubemapProbe( 0 ),
        mBrdf( PbsBrdf::Default )
    {
        memset( mUvSource, 0, sizeof( mUvSource ) );
        memset( mBlendModes, 0, sizeof( mBlendModes ) );
        memset( mReserved, 0, sizeof( mReserved ) );

        mBgDiffuse[0] = mBgDiffuse[1] = mBgDiffuse[2] = mBgDiffuse[3] = 1.0f;

        mDetailNormalWeight[0] = mDetailNormalWeight[1] = 1.0f;
        mDetailNormalWeight[2] = mDetailNormalWeight[3] = 1.0f;
        mDetailWeight[0] = mDetailWeight[1] = mDetailWeight[2] = mDetailWeight[3] = 1.0f;
        for( size_t i=0; i<4; ++i )
        {
            mDetailsOffsetScale[i][0] = 0;
            mDetailsOffsetScale[i][1] = 0;
            mDetailsOffsetScale[i][2] = 1;
            mDetailsOffsetScale[i][3] = 1;
        }

        mEmissive[0] = mEmissive[1] = mEmissive[2] = 0.0f;
        memset( mTexIndices, 0, sizeof( mTexIndices ) );
        memset( mSamplerblocks, 0, sizeof( mSamplerblocks ) );

        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
            mTexToBakedTextureIdx[i] = NUM_PBSM_TEXTURE_TYPES;

        String paramVal;

        if( Hlms::findParamInVec( params, "background_diffuse", paramVal ) )
        {
            ColourValue val = StringConverter::parseColourValue( paramVal, ColourValue::White );
            setBackgroundDiffuse( val );
        }

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

        if( Hlms::findParamInVec( params, "emissive", paramVal ) )
        {
            Vector3 val = StringConverter::parseVector3( paramVal, Vector3::UNIT_SCALE );
            setEmissive( val );
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
            textures[PBSM_DIFFUSE].texture = setTexture( paramVal, PBSM_DIFFUSE );
            mSamplerblocks[PBSM_DIFFUSE] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "normal_map", paramVal ) )
        {
            textures[PBSM_NORMAL].texture = setTexture( paramVal, PBSM_NORMAL );
            mSamplerblocks[PBSM_NORMAL] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "specular_map", paramVal ) )
        {
            textures[PBSM_SPECULAR].texture = setTexture( paramVal, PBSM_SPECULAR );
            mSamplerblocks[PBSM_SPECULAR] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "roughness_map", paramVal ) )
        {
            textures[PBSM_ROUGHNESS].texture = setTexture( paramVal, PBSM_ROUGHNESS );
            mSamplerblocks[PBSM_ROUGHNESS] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "emissive_map", paramVal ) )
        {
            textures[PBSM_EMISSIVE].texture = setTexture( paramVal, PBSM_EMISSIVE );
            mSamplerblocks[PBSM_EMISSIVE] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "detail_weight_map", paramVal ) )
        {
            textures[PBSM_DETAIL_WEIGHT].texture = setTexture( paramVal, PBSM_DETAIL_WEIGHT );
            mSamplerblocks[PBSM_DETAIL_WEIGHT] = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
        if( Hlms::findParamInVec( params, "reflection_map", paramVal ) )
        {
            textures[PBSM_REFLECTION].texture = setTexture( paramVal, PBSM_REFLECTION );
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
                            paramVal, static_cast<PbsTextureTypes>( PBSM_DETAIL0 + i ) );
                mSamplerblocks[PBSM_DETAIL0 + i] = hlmsManager->getSamplerblock( detailSamplerRef );
            }

            key = "detail_normal_map" + StringConverter::toString( i );
            if( Hlms::findParamInVec( params, key, paramVal ) )
            {
                textures[PBSM_DETAIL0_NM + i].texture = setTexture(
                            paramVal, static_cast<PbsTextureTypes>( PBSM_DETAIL0_NM + i ) );
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
                Vector4 offsetScale = StringConverter::parseVector4( paramVal,
                                                                     getDetailMapOffsetScale( i ) );
                mDetailsOffsetScale[i][0] = static_cast<float>( offsetScale[0] );
                mDetailsOffsetScale[i][1] = static_cast<float>( offsetScale[1] );
                mDetailsOffsetScale[i][2] = static_cast<float>( offsetScale[2] );
                mDetailsOffsetScale[i][3] = static_cast<float>( offsetScale[3] );
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

        creator->requestSlot( /*mTextureHash*/0, this, false );

        bakeTextures( textures );
        calculateHash();
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

        const uint32 oldTexHash = mTextureHash;

        if( static_cast<HlmsPbs*>(mCreator)->getOptimizationStrategy() == HlmsPbs::LowerGpuOverhead )
        {
            const size_t poolIdx = static_cast<HlmsPbs*>(mCreator)->getPoolIndex( this );
            const uint32 finalHash = (hash.mHash & 0xFFFFFE00) | (poolIdx & 0x000001FF);
            mTextureHash = finalHash;
        }
        else
        {
            const size_t poolIdx = static_cast<HlmsPbs*>(mCreator)->getPoolIndex( this );
            const uint32 finalHash = (hash.mHash & 0xFFFFFFF0) | (poolIdx & 0x0000000F);
            mTextureHash = finalHash;
        }

        //When ParallaxCorrectedCubemaps are used, we set a const buffer with the
        //probe's information. Thus we need to keep them in different pools
        if( oldTexHash != mTextureHash && mCubemapProbe )
        {
            IdString probeHash( mCubemapProbe->getInternalTexture()->getName() );
            static_cast<HlmsPbs*>(mCreator)->requestSlot( probeHash.mHash, this, false );
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
        float oldFresnelR = mFresnelR;
        float oldFresnelG = mFresnelG;
        float oldFresnelB = mFresnelB;

        float oldkDr = mkDr;
        float oldkDg = mkDg;
        float oldkDb = mkDb;

        if( mTransparencyMode == Transparent )
        {
            //Precompute the transparency CPU-side.
            if( mWorkflow != MetallicWorkflow )
            {
                mFresnelR *= mTransparencyValue;
                mFresnelG *= mTransparencyValue;
                mFresnelB *= mTransparencyValue;
            }
            mkDr *= mTransparencyValue * mTransparencyValue;
            mkDg *= mTransparencyValue * mTransparencyValue;
            mkDb *= mTransparencyValue * mTransparencyValue;
        }

        memcpy( dstPtr, &mBgDiffuse[0], MaterialSizeInGpu );

        mkDr = oldkDr;
        mkDg = oldkDg;
        mkDb = oldkDb;

        mFresnelR = oldFresnelR;
        mFresnelG = oldFresnelG;
        mFresnelB = oldFresnelB;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::notifyOptimizationStrategyChanged(void)
    {
        calculateHash();
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
                                             PbsTextureTypes textureType )
    {
        const HlmsTextureManager::TextureMapType texMapTypes[NUM_PBSM_TEXTURE_TYPES] =
        {
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,

            mWorkflow == MetallicWorkflow
                ? HlmsTextureManager::TEXTURE_TYPE_MONOCHROME
                : HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,

            HlmsTextureManager::TEXTURE_TYPE_MONOCHROME,
            HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA,
#ifdef OGRE_TEXTURE_ATLAS
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
#else
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
#endif
            HlmsTextureManager::TEXTURE_TYPE_ENV_MAP
        };

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                                    createOrRetrieveTexture( name,
                                                                             texMapTypes[textureType] );

        assert( texLocation.texture->isTextureTypeArray() || textureType == PBSM_REFLECTION );

        //If HLMS texture manager failed to find a reflection texture, have look int standard texture manager
        //NB we only do this for reflection textures as all other textures must be texture arrays for performance reasons
        if (textureType == PBSM_REFLECTION && texLocation.texture == hlmsTextureManager->getBlankTexture().texture)
        {
            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(name);
            if (tex.isNull() == false)
            {
                texLocation.texture = tex;
                texLocation.xIdx = 0;
                texLocation.yIdx = 0;
                texLocation.divisor = 1;
            }
        }

        mTexIndices[textureType] = texLocation.xIdx;

        return texLocation.texture;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setBackgroundDiffuse( const ColourValue &bgDiffuse )
    {
        mBgDiffuse[0] = bgDiffuse.r;
        mBgDiffuse[1] = bgDiffuse.g;
        mBgDiffuse[2] = bgDiffuse.b;
        mBgDiffuse[3] = bgDiffuse.a;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    ColourValue HlmsPbsDatablock::getBackgroundDiffuse(void) const
    {
        ColourValue retVal( mBgDiffuse[0], mBgDiffuse[1], mBgDiffuse[2], mBgDiffuse[3] );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDiffuse( const Vector3 &diffuseColour )
    {
        const float invPI = getBrdf() == PbsBrdf::BlinnPhongFullLegacy ? 1.0f : 0.318309886f;
        mkDr = diffuseColour.x * invPI;
        mkDg = diffuseColour.y * invPI;
        mkDb = diffuseColour.z * invPI;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsDatablock::getDiffuse(void) const
    {
        const Real pi = getBrdf() == PbsBrdf::BlinnPhongFullLegacy ? 1.0f : Math::PI;
        return Vector3( mkDr, mkDg, mkDb ) * pi;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setSpecular( const Vector3 &specularColour )
    {
        const float invPI = getBrdf() == PbsBrdf::BlinnPhongLegacyMath ? 0.318309886f : 1.0f;

        mkSr = specularColour.x * invPI;
        mkSg = specularColour.y * invPI;
        mkSb = specularColour.z * invPI;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsDatablock::getSpecular(void) const
    {
        const Real pi = getBrdf() == PbsBrdf::BlinnPhongLegacyMath ? Math::PI : 1.0f;
        return Vector3( mkSr, mkSg, mkSb ) * pi;
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
    void HlmsPbsDatablock::setEmissive( const Vector3 &emissiveColour )
    {
        bool hadEmissive = hasEmissive();
        mEmissive[0] = emissiveColour.x;
        mEmissive[1] = emissiveColour.y;
        mEmissive[2] = emissiveColour.z;

        scheduleConstBufferUpdate();

        if( hadEmissive != hasEmissive() )
            flushRenderables();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsPbsDatablock::getEmissive(void) const
    {
        return Vector3( mEmissive[0], mEmissive[1], mEmissive[2] );
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbsDatablock::hasEmissive(void) const
    {
        return  mEmissive[0] != 0 || mEmissive[1] != 0 ||
                mEmissive[2] != 0 || mTexToBakedTextureIdx[PBSM_EMISSIVE] != NUM_PBSM_TEXTURE_TYPES;
    }
    //-----------------------------------------------------------------------------------
    float HlmsPbsDatablock::getRoughness(void) const
    {
        return mRoughness;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setWorkflow( Workflows workflow )
    {
        if( mWorkflow != workflow )
        {
            mWorkflow = static_cast<uint8>( workflow );
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsDatablock::Workflows HlmsPbsDatablock::getWorkflow(void) const
    {
        return static_cast<Workflows>( mWorkflow );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setMetalness( float metalness )
    {
        assert( mWorkflow == MetallicWorkflow );
        mFresnelR = metalness;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    float HlmsPbsDatablock::getMetalness(void) const
    {
        return mFresnelR;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setIndexOfRefraction( const Vector3 &refractionIdx,
                                                       bool separateFresnel )
    {
        assert( mWorkflow != MetallicWorkflow );
        Vector3 fresnel = (1.0f - refractionIdx) / (1.0f + refractionIdx);
        fresnel = fresnel * fresnel;
        setFresnel( fresnel, separateFresnel );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setFresnel( const Vector3 &fresnel, bool separateFresnel )
    {
        assert( mWorkflow != MetallicWorkflow );
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
    void HlmsPbsDatablock::_setTextures( const PackedTexture packedTextures[NUM_PBSM_TEXTURE_TYPES] )
    {
        PbsBakedTexture textures[NUM_PBSM_TEXTURE_TYPES];

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        for( int i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
        {
            if( mSamplerblocks[i] )
                hlmsManager->destroySamplerblock( mSamplerblocks[i] );

            mTexIndices[i] = packedTextures[i].xIdx;
            textures[i] = PbsBakedTexture( packedTextures[i].texture, packedTextures[i].samplerblock );

            if( !textures[i].texture.isNull() && !textures[i].samplerBlock )
            {
                HlmsSamplerblock samplerBlockRef;
                if( i >= PBSM_DETAIL0 && i <= PBSM_DETAIL3_NM )
                {
                    //Detail maps default to wrap mode.
                    samplerBlockRef.mU = TAM_WRAP;
                    samplerBlockRef.mV = TAM_WRAP;
                    samplerBlockRef.mW = TAM_WRAP;
                }

                textures[i].samplerBlock = hlmsManager->getSamplerblock( samplerBlockRef );
            }

            mSamplerblocks[i] = textures[i].samplerBlock;
        }

        bakeTextures( textures );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setTexture( PbsTextureTypes texType, uint16 arrayIndex,
                                       const TexturePtr &newTexture, const HlmsSamplerblock *refParams )
    {
        //PBS can only use texture arrays.
        assert( newTexture.isNull() ||
                newTexture->getTextureType() == TEX_TYPE_2D_ARRAY ||
                texType == PBSM_REFLECTION );

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
    uint8 HlmsPbsDatablock::getTextureUvSource( PbsTextureTypes sourceType ) const
    {
        assert( sourceType < NUM_PBSM_SOURCES );
        return mUvSource[sourceType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDetailMapBlendMode( uint8 detailMapIdx, PbsBlendModes blendMode )
    {
        assert( detailMapIdx < 4 );

        if( mBlendModes[detailMapIdx] != blendMode )
        {
            mBlendModes[detailMapIdx] = blendMode;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    PbsBlendModes HlmsPbsDatablock::getDetailMapBlendMode( uint8 detailMapIdx ) const
    {
        assert( detailMapIdx < 4 );
        return static_cast<PbsBlendModes>( mBlendModes[detailMapIdx] );
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setDetailNormalWeight( uint8 detailNormalMapIdx, Real weight )
    {
        assert( detailNormalMapIdx < 4 );

        bool wasOne = mDetailNormalWeight[detailNormalMapIdx] == 1.0f;
        mDetailNormalWeight[detailNormalMapIdx] = weight;

        if( wasOne != (mDetailNormalWeight[detailNormalMapIdx] == 1.0f) )
        {
            flushRenderables();
            scheduleConstBufferUpdate();
        }
    }
    //-----------------------------------------------------------------------------------
    Real HlmsPbsDatablock::getDetailNormalWeight( uint8 detailNormalMapIdx ) const
    {
        assert( detailNormalMapIdx < 4 );
        return mDetailNormalWeight[detailNormalMapIdx];
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
        assert( detailMap < 4 );
        bool wasDisabled = getDetailMapOffsetScale( detailMap ) == Vector4( 0, 0, 1, 1 );

        mDetailsOffsetScale[detailMap][0] = static_cast<float>( offsetScale[0] );
        mDetailsOffsetScale[detailMap][1] = static_cast<float>( offsetScale[1] );
        mDetailsOffsetScale[detailMap][2] = static_cast<float>( offsetScale[2] );
        mDetailsOffsetScale[detailMap][3] = static_cast<float>( offsetScale[3] );

        if( wasDisabled != (getDetailMapOffsetScale( detailMap ) == Vector4( 0, 0, 1, 1 )) )
        {
            flushRenderables();
        }

        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector4 HlmsPbsDatablock::getDetailMapOffsetScale( uint8 detailMap ) const
    {
        assert( detailMap < 4 );
        return Vector4( mDetailsOffsetScale[detailMap][0], mDetailsOffsetScale[detailMap][1],
                        mDetailsOffsetScale[detailMap][2], mDetailsOffsetScale[detailMap][3] );
    }
    //-----------------------------------------------------------------------------------
    uint8 HlmsPbsDatablock::getBakedTextureIdx( PbsTextureTypes texType ) const
    {
        return mTexToBakedTextureIdx[texType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setTwoSidedLighting( bool twoSided, bool changeMacroblock,
                                                CullingMode oneSidedShadowCast )
    {
        mTwoSided = twoSided;

        if( twoSided && changeMacroblock )
        {
            HlmsMacroblock macroblock = *mMacroblock[0];
            macroblock.mCullMode = CULL_NONE;
            setMacroblock( macroblock );

            if( oneSidedShadowCast != CULL_NONE )
            {
                macroblock.mCullMode = oneSidedShadowCast;
                setMacroblock( macroblock, true );
            }
        }

        flushRenderables();
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbsDatablock::getTwoSidedLighting(void) const
    {
        return mTwoSided;
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbsDatablock::hasCustomShadowMacroblock(void) const
    {
        if( mTwoSided &&
            (mMacroblock[0]->mCullMode != CULL_NONE ||
             mMacroblock[1]->mCullMode != CULL_NONE) )
        {
            //Since we may have ignored what HlmsManager::setShadowMappingUseBackFaces
            //says, we need to treat them as custom.
            return true;
        }

        return HlmsDatablock::hasCustomShadowMacroblock();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setAlphaTestThreshold( float threshold )
    {
        HlmsDatablock::setAlphaTestThreshold( threshold );
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setTransparency( float transparency, TransparencyModes mode,
                                            bool useAlphaFromTextures, bool changeBlendblock )
    {
        bool mustFlush = false;

        mTransparencyValue      = transparency;

        if( mUseAlphaFromTextures != useAlphaFromTextures || mTransparencyMode != mode )
        {
            mUseAlphaFromTextures = useAlphaFromTextures;
            mTransparencyMode = mode;
            mustFlush = true;
        }

        if( changeBlendblock )
        {
            HlmsBlendblock newBlendblock;

            if( mTransparencyMode == None )
            {
                newBlendblock.mSourceBlendFactor    = SBF_ONE;
                newBlendblock.mDestBlendFactor      = SBF_ZERO;
            }
            else if( mTransparencyMode == Transparent )
            {
                newBlendblock.mSourceBlendFactor    = SBF_ONE;
                newBlendblock.mDestBlendFactor      = SBF_ONE_MINUS_SOURCE_ALPHA;
            }
            else if( mTransparencyMode == Fade )
            {
                newBlendblock.mSourceBlendFactor    = SBF_SOURCE_ALPHA;
                newBlendblock.mDestBlendFactor      = SBF_ONE_MINUS_SOURCE_ALPHA;
            }

            if( newBlendblock != *mBlendblock[0] )
                setBlendblock( newBlendblock );
        }
        else
        {
            if( mTransparencyMode == None && mBlendblock[0]->mIsTransparent )
            {
                LogManager::getSingleton().logMessage(
                            "WARNING: PBS Datablock '" + *getNameStr() +
                            "' disabling transparency but forcing a blendblock to"
                            " keep using alpha blending. Performance will be affected." );
            }
            else if( mTransparencyMode != None && !mBlendblock[0]->mIsTransparent )
            {
                LogManager::getSingleton().logMessage(
                            "WARNING: PBS Datablock '" + *getNameStr() +
                            "' enabling transparency but forcing a blendblock to avoid"
                            " alpha blending. Results may not look as expected." );
            }
        }

        scheduleConstBufferUpdate();
        if( mustFlush )
            flushRenderables();
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setReceiveShadows( bool receiveShadows )
    {
        if( mReceiveShadows != receiveShadows )
        {
            mReceiveShadows = receiveShadows;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    bool HlmsPbsDatablock::getReceiveShadows(void) const
    {
        return mReceiveShadows;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setCubemapProbe( CubemapProbe *probe )
    {
        if( mCubemapProbe != probe )
        {
            if( mCubemapProbe )
                mCubemapProbe->_removeReference();

            mCubemapProbe = probe;
            if( mCubemapProbe )
            {
                mCubemapProbe->_addReference();
                setTexture( PBSM_REFLECTION, 0, mCubemapProbe->getInternalTexture() );
            }
            else
            {
                TexturePtr nullTex;
                setTexture( PBSM_REFLECTION, 0, nullTex );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    CubemapProbe* HlmsPbsDatablock::getCubemapProbe(void) const
    {
        return mCubemapProbe;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::setBrdf( PbsBrdf::PbsBrdf brdf )
    {
        if( mBrdf != brdf )
        {
            const PbsBrdf::PbsBrdf oldBrdf = (PbsBrdf::PbsBrdf)getBrdf();
            const PbsBrdf::PbsBrdf newBrdf = brdf;
            mBrdf = brdf;

            //BlinnPhongFullLegacy expects diffuse in range [0; 1]; we have it in range [0; 1 / PI]
            if( oldBrdf == PbsBrdf::BlinnPhongFullLegacy && newBrdf != PbsBrdf::BlinnPhongFullLegacy )
                setDiffuse( Vector3( mkDr, mkDg, mkDb ) ); //Will be divided by PI.
            if( oldBrdf != PbsBrdf::BlinnPhongFullLegacy && newBrdf == PbsBrdf::BlinnPhongFullLegacy )
                setDiffuse( Vector3( mkDr, mkDg, mkDb ) * Math::PI ); //Increase by PI, won't be divided.

            //BlinnPhongLegacyMath expects specular in range [0; 1 / PI]; we have it in range [0; 1]
            if( oldBrdf == PbsBrdf::BlinnPhongLegacyMath && newBrdf != PbsBrdf::BlinnPhongLegacyMath )
                setSpecular( Vector3( mkSr, mkSg, mkSb ) * Math::PI );
            if( oldBrdf != PbsBrdf::BlinnPhongLegacyMath && newBrdf == PbsBrdf::BlinnPhongLegacyMath )
                setSpecular( Vector3( mkSr, mkSg, mkSb ) );

            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbsDatablock::getBrdf(void) const
    {
        return mBrdf;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::importUnity( const Vector3 &diffuse, const Vector3 &specular,
                                        Real roughness, bool changeBrdf )
    {
        if( changeBrdf )
        {
            //Set the BRDF that matches Unity the closest.
            setBrdf( PbsBrdf::DefaultUncorrelated );
        }

        setWorkflow( SpecularAsFresnelWorkflow );

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
    void HlmsPbsDatablock::importUnity( const Vector3 &colour, Real metallic,
                                        Real roughness, bool changeBrdf )
    {
        if( changeBrdf )
        {
            //Set the BRDF that matches Unity the closest.
            setBrdf( PbsBrdf::DefaultUncorrelated );
        }

        setWorkflow( MetallicWorkflow );

        setDiffuse( colour );

        setSpecular( Vector3::UNIT_SCALE );
        setFresnel( Vector3::UNIT_SCALE, false );
        setMetalness( metallic );
        setRoughness( roughness );
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
        case PBSM_EMISSIVE:
            retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
            break;
        case PBSM_SPECULAR:
            if( mWorkflow == MetallicWorkflow )
            {
                retVal = HlmsTextureManager::TEXTURE_TYPE_MONOCHROME;
            }
            else
            {
                retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
            }
            break;
        case PBSM_DETAIL_WEIGHT:
            retVal = HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA;
            break;
        case PBSM_DETAIL0:
        case PBSM_DETAIL1:
        case PBSM_DETAIL2:
        case PBSM_DETAIL3:
#ifdef OGRE_TEXTURE_ATLAS
            retVal = HlmsTextureManager::TEXTURE_TYPE_DETAIL;
#else
            retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
#endif
            break;

        case PBSM_NORMAL:
            retVal = HlmsTextureManager::TEXTURE_TYPE_NORMALS;
            break;
        case PBSM_DETAIL0_NM:
        case PBSM_DETAIL1_NM:
        case PBSM_DETAIL2_NM:
        case PBSM_DETAIL3_NM:
#ifdef OGRE_TEXTURE_ATLAS
            retVal = HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP;
#else
            retVal = HlmsTextureManager::TEXTURE_TYPE_NORMALS;
#endif
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
    //-----------------------------------------------------------------------------------
    void HlmsPbsDatablock::saveTextures( const String &folderPath, set<String>::type &savedTextures,
                                         bool saveOitd, bool saveOriginal,
                                         HlmsTextureExportListener *listener )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

        for( size_t i=0; i<NUM_PBSM_TEXTURE_TYPES; ++i )
        {
            if( mTexToBakedTextureIdx[i] < mBakedTextures.size() )
            {
                TexturePtr texture = mBakedTextures[mTexToBakedTextureIdx[i]].texture;

                HlmsTextureManager::TextureLocation texLocation;
                texLocation.texture = texture;
                texLocation.xIdx    = mTexIndices[i];
                texLocation.yIdx    = 0;
                texLocation.divisor = 1;
                const String *aliasNamePtr = hlmsTextureManager->findAliasName( texLocation );

                const String aliasName = aliasNamePtr ? *aliasNamePtr : texture->getName();

                //Render Targets are... complicated. Let's not, for now.
                if( savedTextures.find( aliasName ) == savedTextures.end() &&
                    (aliasNamePtr || i == PBSM_REFLECTION) &&
                    !(texture->getUsage() & TU_RENDERTARGET) )
                {
                    DataStreamPtr inFile;
                    if( saveOriginal )
                    {
                        String resourceName;
                        if( aliasNamePtr )
                        {
                            const String *resNamePtr =
                                    hlmsTextureManager->findResourceNameFromAlias( aliasName );
                            if( resNamePtr )
                                resourceName = *resNamePtr;
                            else
                                resourceName = aliasName;
                        }
                        else
                            resourceName = aliasName;

                        String savingFilename = aliasName;
                        if( listener )
                        {
                            listener->savingChangeTextureNameOriginal( aliasName, resourceName,
                                                                       savingFilename );
                        }

                        try
                        {
                            inFile = ResourceGroupManager::getSingleton().openResource(
                                         resourceName, texture->getGroup() );
                        }
                        catch( FileNotFoundException &e )
                        {
                            //Try opening as an absolute path
                            std::fstream *ifs = OGRE_NEW_T( std::fstream, MEMCATEGORY_GENERAL )(
                                                    resourceName.c_str(),
                                                    std::ios::binary|std::ios::in );

                            if( ifs->is_open() )
                            {
                                inFile = DataStreamPtr( OGRE_NEW FileStreamDataStream( resourceName,
                                                                                       ifs, true ) );
                            }
                            else
                            {
                                LogManager::getSingleton().logMessage(
                                            "WARNING: Could not find texture file " + aliasName +
                                            " (" + resourceName + ") for copying to export location. "
                                            "Error: " + e.getFullDescription() );
                            }
                        }
                        catch( Exception &e )
                        {
                            LogManager::getSingleton().logMessage(
                                        "WARNING: Could not find texture file " + aliasName +
                                        " (" + resourceName + ") for copying to export location. "
                                        "Error: " + e.getFullDescription() );
                        }

                        if( inFile )
                        {
                            size_t fileSize = inFile->size();
                            vector<uint8>::type fileData;
                            fileData.resize( fileSize );
                            inFile->read( &fileData[0], fileData.size() );
                            std::ofstream outFile( (folderPath + "/" + savingFilename).c_str(),
                                                   std::ios::binary | std::ios::out );
                            outFile.write( (const char*)&fileData[0], fileData.size() );
                            outFile.close();
                        }
                    }

                    if( saveOitd )
                    {
                        String texName = aliasName;
                        if( listener )
                            listener->savingChangeTextureNameOitd( aliasName, texName );
                        const uint32 numSlices = i == PBSM_REFLECTION ? 6u : 1u;

                        Image image;
                        texture->convertToImage( image, true, 0u, texLocation.xIdx, numSlices );

                        image.save( folderPath + "/" + texName + ".oitd" );
                    }

                    savedTextures.insert( aliasName );
                }
            }
        }

    }
}
