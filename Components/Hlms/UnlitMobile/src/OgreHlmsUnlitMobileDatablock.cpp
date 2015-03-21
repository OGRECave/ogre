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

#include "OgreHlmsUnlitMobileDatablock.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreTexture.h"
#include "OgreTextureManager.h"
#include "OgreRenderSystem.h"
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

    const String c_diffuseMap[16] =
    {
        "diffuse_map",
        "diffuse_map1",
        "diffuse_map2",
        "diffuse_map3",
        "diffuse_map4",
        "diffuse_map5",
        "diffuse_map6",
        "diffuse_map7",
        "diffuse_map8",
        "diffuse_map9",
        "diffuse_map10",
        "diffuse_map11",
        "diffuse_map12",
        "diffuse_map13",
        "diffuse_map14",
        "diffuse_map15"
    };

    HlmsUnlitMobileDatablock::HlmsUnlitMobileDatablock( IdString name, Hlms *creator,
                                                        const HlmsMacroblock *macroblock,
                                                        const HlmsBlendblock *blendblock,
                                                        const HlmsParamVec &params ) :
        HlmsDatablock( name, creator, macroblock, blendblock, params ),
        mNumTextureMatrices( 0 ),
        mHasColour( false ),
        mNumTextureUnits( 0 ),
        mNumUvAtlas( 0 ),
        mR( 1.0f ), mG( 1.0f ), mB( 1.0f ), mA( 1.0f ),
        mShaderCreationData( 0 )
    {
        memset( mBakedSamplerblocks, 0, sizeof(mBakedSamplerblocks) );

        for( size_t i=0; i<sizeof(mTextureMatrices) / sizeof(Matrix4); ++i )
        {
            mTextureMatrices[i*16 +  0] = 1.0f;
            mTextureMatrices[i*16 +  1] = 0.0f;
            mTextureMatrices[i*16 +  2] = 0.0f;
            mTextureMatrices[i*16 +  3] = 0.0f;

            mTextureMatrices[i*16 +  4] = 0.0f;
            mTextureMatrices[i*16 +  5] = 1.0f;
            mTextureMatrices[i*16 +  6] = 0.0f;
            mTextureMatrices[i*16 +  7] = 0.0f;

            mTextureMatrices[i*16 +  8] = 0.0f;
            mTextureMatrices[i*16 +  9] = 0.0f;
            mTextureMatrices[i*16 + 10] = 1.0f;
            mTextureMatrices[i*16 + 11] = 0.0f;

            mTextureMatrices[i*16 + 12] = 0.0f;
            mTextureMatrices[i*16 + 13] = 0.0f;
            mTextureMatrices[i*16 + 14] = 0.0f;
            mTextureMatrices[i*16 + 15] = 1.0f;
        }

        mShaderCreationData = new ShaderCreationData();

        String paramVal;

        if( Hlms::findParamInVec( params, "diffuse", paramVal ) )
        {
            mHasColour = true;

            if( !paramVal.empty() )
            {
                ColourValue val = StringConverter::parseColourValue( paramVal );
                mR = val.r;
                mG = val.g;
                mB = val.b;
                mA = val.a;
            }
        }

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        HlmsTextureManager *hlmsTextureManager = hlmsManager->getTextureManager();

        const HlmsSamplerblock *defaultSamplerblock = hlmsManager->getSamplerblock( HlmsSamplerblock() );

        for( size_t i=0; i<sizeof( c_diffuseMap ) / sizeof( String ); ++i )
        {
            if( Hlms::findParamInVec( params, c_diffuseMap[i], paramVal ) )
            {
                if( mNumTextureUnits != i )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 mName.getFriendlyText() + ": Can't leave gaps between texture units! '"
                                 + c_diffuseMap[i] + "' was specified but the previous diffuse_map "
                                 "is missing.", "HlmsUnlitMobileDatablock::HlmsUnlitMobileDatablock" );
                }

                mShaderCreationData->mDiffuseTextures[i] = hlmsTextureManager->getBlankTexture().texture;
                mShaderCreationData->mSamplerblocks[i] = defaultSamplerblock;
                hlmsManager->addReference( mShaderCreationData->mSamplerblocks[i] );

                StringVector vec = StringUtil::split( paramVal );

                StringVector::const_iterator itor = vec.begin();
                StringVector::const_iterator end  = vec.end();

                while( itor != end )
                {
                    uint val = StringConverter::parseUnsignedInt( *itor, ~0 );

                    if( val != (uint)(~0) )
                    {
                        //It's a number, must be an UV Set
                        setTextureUvSource( i, val );
                    }
                    else if( !itor->empty() )
                    {
                        //Is it a blend mode?
                        const String *it = std::find( c_blendModes, c_blendModes +
                                                      sizeof(c_blendModes) / sizeof( String ),
                                                      *itor );

                        if( it == c_blendModes + sizeof(c_blendModes) / sizeof( String ) )
                        {
                            //Not blend mode, try loading a texture
                            HlmsTextureManager::TextureLocation texLocation = hlmsTextureManager->
                                   createOrRetrieveTexture( *itor,
                                                            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
                            assert( !texLocation.texture->isTextureTypeArray() );
                            mShaderCreationData->mDiffuseTextures[i] = texLocation.texture;

                            if( texLocation.xIdx != 0 || texLocation.yIdx != 0 ||
                                texLocation.divisor != 1 )
                            {
                                mUvAtlasParams[mNumUvAtlas].uOffset   = texLocation.xIdx /
                                                                            (float)texLocation.divisor;
                                mUvAtlasParams[mNumUvAtlas].vOffset   = texLocation.yIdx /
                                                                            (float)texLocation.divisor;
                                mUvAtlasParams[mNumUvAtlas].invDivisor= 1.0f / texLocation.divisor;
                                mShaderCreationData->mTextureIsAtlas[i] = true;
                                ++mNumUvAtlas;
                            }
                        }
                        else
                        {
                            //It's a blend mode
                            mShaderCreationData->mBlendModes[i] = (it - c_blendModes);
                        }
                    }

                    ++itor;
                }

                ++mNumTextureUnits;
            }
        }

        //Remove the reference
        hlmsManager->destroySamplerblock( defaultSamplerblock );

        size_t maxTextureUnits = mCreator->getRenderSystem()->getCapabilities()->getNumTextureUnits();
        if( mNumTextureUnits > maxTextureUnits )
        {
            LogManager::getSingleton().logMessage( "WARNING: material '" + mName.getFriendlyText() +
                    "' exceeds the maximum number of " + StringConverter::toString( maxTextureUnits ) +
                    " texture units supported by this hardware.", LML_CRITICAL );
            mNumTextureUnits = maxTextureUnits;
        }

        if( Hlms::findParamInVec( params, "animate", paramVal ) )
        {
            size_t pos = paramVal.find_first_of( ' ' );
            while( pos != String::npos )
            {
                uint val = StringConverter::parseUnsignedInt( paramVal.substr( pos, 1 ), ~0 );

                if( val >= 8 )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 mName.getFriendlyText() +
                                 ": animate parameters must be in range [0; 8)",
                                 "HlmsUnlitMobileDatablock::HlmsUnlitMobileDatablock" );
                }

                if( mShaderCreationData->mTextureMatrixMap[val] == 1 )
                {
                    LogManager::getSingleton().logMessage( "WARNING: specified same UV set twice "
                            "in material '" + mName.getFriendlyText() +
                            "'; parameter 'animate'. Are you sure this is correct?", LML_CRITICAL );
                }

                mShaderCreationData->mTextureMatrixMap[val] = 1;

                pos = paramVal.find_first_of( ' ' );
            }

            for( size_t i=0; i<8; ++i )
            {
                if( mShaderCreationData->mTextureMatrixMap[i] == 1 )
                    mShaderCreationData->mTextureMatrixMap[i] = mNumTextureMatrices++;
            }
        }

        calculateHash();
    }
    //-----------------------------------------------------------------------------------
    HlmsUnlitMobileDatablock::~HlmsUnlitMobileDatablock()
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        if( hlmsManager )
        {
            for( uint8 i=0; i<16; ++i )
            {
                if( mShaderCreationData->mSamplerblocks[i] )
                {
                    hlmsManager->destroySamplerblock( mShaderCreationData->mSamplerblocks[i] );
                    mShaderCreationData->mSamplerblocks[i] = 0;
                }
            }
        }

        delete mShaderCreationData;
        mShaderCreationData = 0;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobileDatablock::calculateHash()
    {
        //First bake the textures
        uint8 numTextureUnits = 0;
        for( uint8 i=0; i<16; ++i )
        {
            if( !mShaderCreationData->mDiffuseTextures[i].isNull() )
            {
                mBakedDiffuseTextures[numTextureUnits]    = mShaderCreationData->mDiffuseTextures[i];
                mBakedSamplerblocks[numTextureUnits]      = mShaderCreationData->mSamplerblocks[i];
                ++numTextureUnits;
            }
        }

        //Clean the pointers of unused slots if the number of textures is now smaller.
        for( uint8 i=numTextureUnits; i<mNumTextureUnits; ++i )
        {
            mBakedDiffuseTextures[i].setNull();
            mBakedSamplerblocks[i] = 0;
        }

        if( mNumTextureUnits != numTextureUnits )
        {
            mNumTextureUnits = numTextureUnits;
            flushRenderables();
        }

        IdString hash;
        for( uint i=0; i<mNumTextureUnits; ++i )
        {
            hash += IdString( mBakedDiffuseTextures[i]->getName() );
            hash += IdString( mBakedSamplerblocks[i]->mId );
        }

        mTextureHash = hash.mHash;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobileDatablock::setUseColour( bool useColour )
    {
        if( mHasColour != useColour )
        {
            mHasColour = useColour;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobileDatablock::setColour( const ColourValue &diffuse )
    {
        assert( diffuse == ColourValue::White ||
                (mHasColour &&
                 "Setting colour to a Datablock created w/out diffuse flag will be ignored") );
        mR = diffuse.r;
        mG = diffuse.g;
        mB = diffuse.b;
        mA = diffuse.a;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobileDatablock::setTexture( uint8 texUnit, TexturePtr &newTexture,
                                               const UvAtlasParams &atlasParams )
    {
        if( texUnit >= 16 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Texture unit out of range in datablock '" +
                         mName.getFriendlyText() + "'", "HlmsUnlitMobileDatablock::setTexture" );
        }

        mShaderCreationData->mDiffuseTextures[texUnit]   = newTexture;

        if( !mShaderCreationData->mSamplerblocks[texUnit] && !newTexture.isNull() )
        {
            HlmsManager *hlmsManager = mCreator->getHlmsManager();
            mShaderCreationData->mSamplerblocks[texUnit] =
                                            hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }

        size_t uvAtlasIdx = 0;
        for( size_t i=0; i<texUnit; ++i )
            uvAtlasIdx += mShaderCreationData->mTextureIsAtlas[i];

        if( (atlasParams.uOffset != 0 || atlasParams.vOffset != 0 || atlasParams.invDivisor != 1.0f) &&
            !newTexture.isNull() )
        {
            if( mShaderCreationData->mTextureIsAtlas[texUnit] == false )
            {
                //The previous texture wasn't an atlas, we need to make room for the params
                memmove( mUvAtlasParams + uvAtlasIdx + 1, mUvAtlasParams + uvAtlasIdx,
                         sizeof(UvAtlasParams) * (mNumUvAtlas - uvAtlasIdx - 1) );
                mShaderCreationData->mTextureIsAtlas[texUnit] = true;
                ++mNumUvAtlas;
            }

            mUvAtlasParams[uvAtlasIdx] = atlasParams;
        }
        else
        {
            if( mShaderCreationData->mTextureIsAtlas[texUnit] == true )
            {
                //The new texture isn't an atlas, we need to keep everything contiguous
                memmove( mUvAtlasParams + uvAtlasIdx, mUvAtlasParams + uvAtlasIdx + 1,
                         sizeof(UvAtlasParams) * (mNumUvAtlas - uvAtlasIdx - 1) );
                mShaderCreationData->mTextureIsAtlas[texUnit] = false;
                --mNumUvAtlas;
            }
        }

        calculateHash();
    }
    //-----------------------------------------------------------------------------------
    TexturePtr HlmsUnlitMobileDatablock::getTexture( uint8 texUnit ) const
    {
        assert( texUnit < 16 );
        return mShaderCreationData->mDiffuseTextures[texUnit];
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobileDatablock::setSamplerblock( uint8 texUnit, const HlmsSamplerblock &params )
    {
        if( texUnit >= 16 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Texture unit out of range in datablock '" +
                         mName.getFriendlyText() + "'", "HlmsUnlitMobileDatablock::setSamplerblock" );
        }

        const HlmsSamplerblock *oldBlock = mShaderCreationData->mSamplerblocks[texUnit];

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        mShaderCreationData->mSamplerblocks[texUnit] = hlmsManager->getSamplerblock( params );

        if( oldBlock != mShaderCreationData->mSamplerblocks[texUnit] )
            calculateHash();

        if( oldBlock )
            hlmsManager->destroySamplerblock( oldBlock );
    }
    //-----------------------------------------------------------------------------------
    const HlmsSamplerblock* HlmsUnlitMobileDatablock::getSamplerblock( uint8 texUnit ) const
    {
        return mShaderCreationData->mSamplerblocks[texUnit];
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobileDatablock::setTextureUvSource( uint8 texUnit, uint8 uvSet )
    {
        if( texUnit >= 16 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Texture unit out of range in datablock '" +
                         mName.getFriendlyText() + "' attempted value: " +
                         StringConverter::toString( texUnit ),
                         "HlmsUnlitMobileDatablock::setTextureUvSource" );
        }

        if( uvSet >= 8 )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "UV Set must be in range [0; 8) in datablock '" +
                         mName.getFriendlyText() + "'; attempted value: " +
                         StringConverter::toString( uvSet ),
                         "HlmsUnlitMobileDatablock::setTextureUvSource" );
        }

        mShaderCreationData->mUvSetForTexture[texUnit] = uvSet;

        flushRenderables();
    }
    //-----------------------------------------------------------------------------------
    uint8 HlmsUnlitMobileDatablock::getNumUvSets(void) const
    {
        uint8 retVal = 0;

        for( size_t i=0; i<mNumTextureUnits; ++i )
            retVal = std::max<uint8>( mShaderCreationData->mUvSetForTexture[i] + 1, retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsUnlitMobileDatablock::UvAtlasParams HlmsUnlitMobileDatablock::textureLocationToAtlasParams(
                                                const HlmsTextureManager::TextureLocation &texLocation )
    {
        UvAtlasParams retVal;
        retVal.uOffset   = texLocation.xIdx / (float)texLocation.divisor;
        retVal.vOffset   = texLocation.yIdx / (float)texLocation.divisor;
        retVal.invDivisor= 1.0f / texLocation.divisor;

        return retVal;
    }
}
