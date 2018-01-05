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

#include "Terra/Hlms/OgreHlmsTerraDatablock.h"
#include "Terra/Hlms/OgreHlmsTerra.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreTexture.h"
#include "OgreTextureManager.h"
#include "OgreLogManager.h"

#include "OgreHlmsTerraDatablock.cpp.inc"

namespace Ogre
{
    const size_t HlmsTerraDatablock::MaterialSizeInGpu          = 4 * 7 * 4;
    const size_t HlmsTerraDatablock::MaterialSizeInGpuAligned   = alignToNextMultiple(
                                                                    HlmsTerraDatablock::MaterialSizeInGpu,
                                                                    4 * 4 );

    //-----------------------------------------------------------------------------------
    HlmsTerraDatablock::HlmsTerraDatablock( IdString name, HlmsTerra *creator,
                                        const HlmsMacroblock *macroblock,
                                        const HlmsBlendblock *blendblock,
                                        const HlmsParamVec &params ) :
        HlmsDatablock( name, creator, macroblock, blendblock, params ),
        mkDr( 0.318309886f ), mkDg( 0.318309886f ), mkDb( 0.318309886f ), //Max Diffuse = 1 / PI
        _padding0( 1 ),
        mBrdf( TerraBrdf::Default )
    {
        mRoughness[0] = mRoughness[1] = 1.0f;
        mRoughness[2] = mRoughness[3] = 1.0f;
        mMetalness[0] = mMetalness[1] = 1.0f;
        mMetalness[2] = mMetalness[3] = 1.0f;

        for( size_t i=0; i<4; ++i )
            mDetailsOffsetScale[i] = Vector4( 0, 0, 1, 1 );

        memset( mTexIndices, 0, sizeof( mTexIndices ) );
        memset( mSamplerblocks, 0, sizeof( mSamplerblocks ) );

        for( size_t i=0; i<NUM_TERRA_TEXTURE_TYPES; ++i )
            mTexToBakedTextureIdx[i] = NUM_TERRA_TEXTURE_TYPES;

        calculateHash();

        creator->requestSlot( /*mTextureHash*/0, this, false );
    }
    //-----------------------------------------------------------------------------------
    HlmsTerraDatablock::~HlmsTerraDatablock()
    {
        if( mAssignedPool )
            static_cast<HlmsTerra*>(mCreator)->releaseSlot( this );

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        if( hlmsManager )
        {
            for( size_t i=0; i<NUM_TERRA_TEXTURE_TYPES; ++i )
            {
                if( mSamplerblocks[i] )
                    hlmsManager->destroySamplerblock( mSamplerblocks[i] );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::calculateHash()
    {
        IdString hash;

        TerraBakedTextureArray::const_iterator itor = mBakedTextures.begin();
        TerraBakedTextureArray::const_iterator end  = mBakedTextures.end();

        while( itor != end )
        {
            hash += IdString( itor->texture->getName() );
            hash += IdString( itor->samplerBlock->mId );

            ++itor;
        }

        if( mTextureHash != hash.mHash )
        {
            mTextureHash = hash.mHash;
            static_cast<HlmsTerra*>(mCreator)->requestSlot( /*mTextureHash*/0, this, false );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::scheduleConstBufferUpdate(void)
    {
        static_cast<HlmsTerra*>(mCreator)->scheduleForUpdate( this );
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::uploadToConstBuffer( char *dstPtr )
    {
        memcpy( dstPtr, &mkDr, MaterialSizeInGpu );
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::decompileBakedTextures( TerraBakedTexture outTextures[NUM_TERRA_TEXTURE_TYPES] )
    {
        //Decompile the baked textures to know which texture is assigned to each type.
        for( size_t i=0; i<NUM_TERRA_TEXTURE_TYPES; ++i )
        {
            uint8 idx = mTexToBakedTextureIdx[i];

            if( idx < NUM_TERRA_TEXTURE_TYPES )
            {
                outTextures[i] = TerraBakedTexture( mBakedTextures[idx].texture, mSamplerblocks[i] );
            }
            else
            {
                //The texture may be null, but the samplerblock information may still be there.
                outTextures[i] = TerraBakedTexture( TexturePtr(), mSamplerblocks[i] );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::bakeTextures( const TerraBakedTexture textures[NUM_TERRA_TEXTURE_TYPES] )
    {
        //The shader might need to be recompiled (mTexToBakedTextureIdx changed).
        //We'll need to flush.
        //Most likely mTexIndices also changed, so we need to update the const buffers as well
        mBakedTextures.clear();

        for( size_t i=0; i<NUM_TERRA_TEXTURE_TYPES; ++i )
        {
            if( !textures[i].texture.isNull() )
            {
                TerraBakedTextureArray::const_iterator itor = std::find( mBakedTextures.begin(),
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
                mTexToBakedTextureIdx[i] = NUM_TERRA_TEXTURE_TYPES;
            }
        }

        calculateHash();
        flushRenderables();
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
//    TexturePtr HlmsTerraDatablock::setTexture( const String &name,
//                                             TerraTextureTypes textureType )
//    {
//        const HlmsTextureManager::TextureMapType texMapTypes[NUM_TERRA_TEXTURE_TYPES] =
//        {
//            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
//            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
//            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
//            HlmsTextureManager::TEXTURE_TYPE_MONOCHROME,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
//            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
//            HlmsTextureManager::TEXTURE_TYPE_ENV_MAP
//        };

//        HlmsManager *hlmsManager = mCreator->getHlmsManager();
//        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();
//        HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
//                                                    createOrRetrieveTexture( name,
//                                                                             texMapTypes[textureType] );

//        assert( texLocation.texture->isTextureTypeArray() || textureType == TERRA_REFLECTION );

//        //If HLMS texture manager failed to find a reflection texture, have look int standard texture manager
//        //NB we only do this for reflection textures as all other textures must be texture arrays for performance reasons
//        if (textureType == TERRA_REFLECTION && texLocation.texture == hlmsTextureManager->getBlankTexture().texture)
//        {
//            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(name);
//            if (tex.isNull() == false)
//            {
//                texLocation.texture = tex;
//                texLocation.xIdx = 0;
//                texLocation.yIdx = 0;
//                texLocation.divisor = 1;
//            }
//        }

//        mTexIndices[textureType] = texLocation.xIdx;

//        return texLocation.texture;
//    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setDiffuse( const Vector3 &diffuseColour )
    {
        const float invPI = 0.318309886f;
        mkDr = diffuseColour.x * invPI;
        mkDg = diffuseColour.y * invPI;
        mkDb = diffuseColour.z * invPI;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    Vector3 HlmsTerraDatablock::getDiffuse(void) const
    {
        return Vector3( mkDr, mkDg, mkDb ) * Ogre::Math::PI;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setRoughness( uint8 detailMapIdx, float roughness )
    {
        mRoughness[detailMapIdx] = roughness;
        if( mRoughness[detailMapIdx] <= 1e-6f )
        {
            LogManager::getSingleton().logMessage( "WARNING: TERRA Datablock '" +
                        mName.getFriendlyText() + "' Very low roughness values can "
                                                  "cause NaNs in the pixel shader!" );
        }
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    float HlmsTerraDatablock::getRoughness( uint8 detailMapIdx ) const
    {
        return mRoughness[detailMapIdx];
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setMetalness( uint8 detailMapIdx, float metalness )
    {
        mMetalness[detailMapIdx] = metalness;
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    float HlmsTerraDatablock::getMetalness( uint8 detailMapIdx ) const
    {
        return mMetalness[detailMapIdx];
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::_setTextures( const TerraPackedTexture packedTextures[NUM_TERRA_TEXTURE_TYPES] )
    {
        TerraBakedTexture textures[NUM_TERRA_TEXTURE_TYPES];

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        for( int i=0; i<NUM_TERRA_TEXTURE_TYPES; ++i )
        {
            if( mSamplerblocks[i] )
                hlmsManager->destroySamplerblock( mSamplerblocks[i] );

            mTexIndices[i] = packedTextures[i].xIdx;
            textures[i] = TerraBakedTexture( packedTextures[i].texture, packedTextures[i].samplerblock );

            if( !textures[i].texture.isNull() && !textures[i].samplerBlock )
            {
                HlmsSamplerblock samplerBlockRef;
                if( i >= TERRA_DETAIL0 && i <= TERRA_DETAIL_METALNESS3 )
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
    void HlmsTerraDatablock::setTexture( TerraTextureTypes texType, uint16 arrayIndex,
                                       const TexturePtr &newTexture, const HlmsSamplerblock *refParams )
    {
        TerraBakedTexture textures[NUM_TERRA_TEXTURE_TYPES];

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
            if( texType >= TERRA_DETAIL0 && texType <= TERRA_DETAIL_METALNESS3 )
            {
                //Detail maps default to wrap mode.
                samplerBlockRef.mU = TAM_WRAP;
                samplerBlockRef.mV = TAM_WRAP;
                samplerBlockRef.mW = TAM_WRAP;
            }

            HlmsManager *hlmsManager = mCreator->getHlmsManager();
            mSamplerblocks[texType] = hlmsManager->getSamplerblock( samplerBlockRef );
        }

        TerraBakedTexture oldTex = textures[texType];

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
    TexturePtr HlmsTerraDatablock::getTexture( TerraTextureTypes texType ) const
    {
        TexturePtr retVal;

        if( mTexToBakedTextureIdx[texType] < mBakedTextures.size() )
            retVal = mBakedTextures[mTexToBakedTextureIdx[texType]].texture;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    TexturePtr HlmsTerraDatablock::getTexture( size_t texType ) const
    {
        return getTexture( static_cast<TerraTextureTypes>( texType ) );
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setSamplerblock( TerraTextureTypes texType, const HlmsSamplerblock &params )
    {
        const HlmsSamplerblock *oldSamplerblock = mSamplerblocks[texType];
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        mSamplerblocks[texType] = hlmsManager->getSamplerblock( params );

        if( oldSamplerblock )
            hlmsManager->destroySamplerblock( oldSamplerblock );

        if( oldSamplerblock != mSamplerblocks[texType] )
        {
            TerraBakedTexture textures[NUM_TERRA_TEXTURE_TYPES];
            decompileBakedTextures( textures );
            bakeTextures( textures );
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsSamplerblock* HlmsTerraDatablock::getSamplerblock( TerraTextureTypes texType ) const
    {
        return mSamplerblocks[texType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setDetailMapOffsetScale( uint8 detailMap, const Vector4 &offsetScale )
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
    const Vector4& HlmsTerraDatablock::getDetailMapOffsetScale( uint8 detailMap ) const
    {
        assert( detailMap < 8 );
        return mDetailsOffsetScale[detailMap];
    }
    //-----------------------------------------------------------------------------------
    uint8 HlmsTerraDatablock::getBakedTextureIdx( TerraTextureTypes texType ) const
    {
        return mTexToBakedTextureIdx[texType];
    }
    //-----------------------------------------------------------------------------------
    uint16 HlmsTerraDatablock::_getTextureSliceArrayIndex( TerraTextureTypes texType ) const
    {
        return mTexIndices[texType];
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setAlphaTestThreshold( float threshold )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Alpha testing not supported on Terra Hlms",
                     "HlmsTerraDatablock::setAlphaTestThreshold" );

        HlmsDatablock::setAlphaTestThreshold( threshold );
        scheduleConstBufferUpdate();
    }
    //-----------------------------------------------------------------------------------
    void HlmsTerraDatablock::setBrdf( TerraBrdf::TerraBrdf brdf )
    {
        if( mBrdf != brdf )
        {
            mBrdf = brdf;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsTerraDatablock::getBrdf(void) const
    {
        return mBrdf;
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureMapType HlmsTerraDatablock::suggestMapTypeBasedOnTextureType(
                                                                        TerraTextureTypes type )
    {
        HlmsTextureManager::TextureMapType retVal;
        switch( type )
        {
        default:
        case TERRA_DIFFUSE:
            retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
            break;
        case TERRA_DETAIL_WEIGHT:
            retVal = HlmsTextureManager::TEXTURE_TYPE_NON_COLOR_DATA;
            break;
        case TERRA_DETAIL0:
        case TERRA_DETAIL1:
        case TERRA_DETAIL2:
        case TERRA_DETAIL3:
#ifdef OGRE_TEXTURE_ATLAS
            retVal = HlmsTextureManager::TEXTURE_TYPE_DETAIL;
#else
            retVal = HlmsTextureManager::TEXTURE_TYPE_DIFFUSE;
#endif
            break;

        case TERRA_DETAIL0_NM:
        case TERRA_DETAIL1_NM:
        case TERRA_DETAIL2_NM:
        case TERRA_DETAIL3_NM:
#ifdef OGRE_TEXTURE_ATLAS
            retVal = HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP;
#else
            retVal = HlmsTextureManager::TEXTURE_TYPE_NORMALS;
#endif
            break;

        case TERRA_DETAIL_ROUGHNESS0:
        case TERRA_DETAIL_ROUGHNESS1:
        case TERRA_DETAIL_ROUGHNESS2:
        case TERRA_DETAIL_ROUGHNESS3:
        case TERRA_DETAIL_METALNESS0:
        case TERRA_DETAIL_METALNESS1:
        case TERRA_DETAIL_METALNESS2:
        case TERRA_DETAIL_METALNESS3:
            retVal = HlmsTextureManager::TEXTURE_TYPE_MONOCHROME;
            break;

        case TERRA_REFLECTION:
            retVal = HlmsTextureManager::TEXTURE_TYPE_ENV_MAP;
            break;
        }

        return retVal;
    }
}
