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

#include "OgreHlmsGui2DMobileDatablock.h"
#include "OgreHlms.h"
#include "OgreTexture.h"
#include "OgreTextureManager.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"

namespace Ogre
{
    extern const String c_diffuseMap[15];
    const String c_diffuseMap[15] =
    {
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

    HlmsGui2DMobileDatablock::HlmsGui2DMobileDatablock( IdString name, Hlms *creator,
                                                        const HlmsMacroblock *macroblock,
                                                        const HlmsBlendblock *blendblock,
                                                        const HlmsParamVec &params ) :
        HlmsDatablock( name, creator, macroblock, blendblock, params ),
        mNumTextureMatrices( 0 ),
        mHasColour( false ),
        mIsAlphaTested( false ),
        mNumTextureUnits( 0 ),
        mR( 1.0f ), mG( 1.0f ), mB( 1.0f ), mA( 1.0f ),
        mAlphaTestThreshold( 0.5f )
    {
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

        memset( mTextureMatrixMap, 0xffffffff, sizeof(mTextureMatrixMap) );

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

        if( Hlms::findParamInVec( params, Hlms::PropertyAlphaTest, paramVal ) )
        {
            mIsAlphaTested = true;

            if( !paramVal.empty() )
            {
                StringVector vec = StringUtil::split( paramVal );
                StringVector::const_iterator itor = vec.begin();
                StringVector::const_iterator end  = vec.end();

                Real val = -1.0f;
                while( itor != end && val < 0 )
                    val = StringConverter::parseReal( *itor++, -1.0f );

                if( val >= 0 )
                    mAlphaTestThreshold = val;
            }
        }

        if( Hlms::findParamInVec( params, Hlms::PropertyDiffuseMap, paramVal ) ||
            Hlms::findParamInVec( params, "diffuse_map0", paramVal ) )
        {
            size_t pos = std::min( paramVal.find_first_of( ' ' ), paramVal.size() );
            mDiffuseTextures[mNumTextureUnits] = TextureManager::getSingleton().getByName(
                                                                paramVal.substr( 0, pos ) );

            if( mDiffuseTextures[mNumTextureUnits].isNull() )
            {
                //TODO: Assign blank texture
                String paramValName;
                Hlms::findParamInVec( params, "name", paramVal );
                LogManager::getSingleton().logMessage( paramValName + ": WARNING Texture '" +
                                                       paramVal.substr( 0, pos ) + "' not found" );
                //mDiffuseTextures[mNumTextureUnits]
            }

            ++mNumTextureUnits;
        }

        for( size_t i=0; i<sizeof( c_diffuseMap ) / sizeof( String* ); ++i )
        {
            if( Hlms::findParamInVec( params, c_diffuseMap[i], paramVal ) )
            {
                if( mNumTextureUnits != i + 1 )
                {
                    Hlms::findParamInVec( params, "name", paramVal );
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 paramVal + ": Can't leave gaps between texture units! '" +
                                 c_diffuseMap[i] + "' was specified but the previous diffuse_map "
                                 "is missing.", "HlmsGui2DMobileDatablock::HlmsGui2DMobileDatablock" );
                }

                size_t pos = std::min( paramVal.find_first_of( ' ' ), paramVal.size() );
                mDiffuseTextures[mNumTextureUnits] = TextureManager::getSingleton().getByName(
                                                                    paramVal.substr( 0, pos ) );

                if( mDiffuseTextures[mNumTextureUnits].isNull() )
                {
                    //TODO: Assign blank texture
                    String paramValName;
                    Hlms::findParamInVec( params, "name", paramVal );
                    LogManager::getSingleton().logMessage( paramValName + ": WARNING Texture '" +
                                                           paramVal.substr( 0, pos ) + "' not found" );
                    //mDiffuseTextures[mNumTextureUnits]
                }

                ++mNumTextureUnits;
            }
        }

        size_t maxTextureUnits = mCreator->getRenderSystem()->getCapabilities()->getNumTextureUnits();
        if( mNumTextureUnits > maxTextureUnits )
        {
            Hlms::findParamInVec( params, "name", paramVal );
            LogManager::getSingleton().logMessage( "WARNING: material '" + paramVal +
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
                    Hlms::findParamInVec( params, "name", paramVal );
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 paramVal + ": animate parameters must be in range [0; 8)",
                                 "HlmsGui2DMobileDatablock::HlmsGui2DMobileDatablock" );
                }

                if( mTextureMatrixMap[val] == 1 )
                {
                    Hlms::findParamInVec( params, "name", paramVal );
                    LogManager::getSingleton().logMessage( "WARNING: specified same UV set twice "
                            "in material '" + paramVal + "'; parameter 'animate'. Are you sure this "
                            "is correct?", LML_CRITICAL );
                }

                mTextureMatrixMap[val] = 1;

                pos = paramVal.find_first_of( ' ' );
            }

            for( size_t i=0; i<8; ++i )
            {
                if( mTextureMatrixMap[i] == 1 )
                    mTextureMatrixMap[i] = mNumTextureMatrices++;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobileDatablock::calculateHash()
    {
        IdString hash;
        for( uint i=0; i<mNumTextureUnits; ++i )
            hash += IdString( mDiffuseTextures[i]->getName() );

        mTextureHash = hash.mHash;
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobileDatablock::setColour( const ColourValue &diffuse )
    {
        assert( mHasColour && "Setting colour to a Datablock created w/out diffuse flag will be ignored" );
        mR = diffuse.r;
        mG = diffuse.g;
        mB = diffuse.b;
        mA = diffuse.a;
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobileDatablock::setAlphaTestThreshold( float alphaThreshold )
    {
        assert( mIsAlphaTested && "Setting alpha threshold to a Datablock created w/out alpha test "
                "will be ignored" );

        mAlphaTestThreshold = alphaThreshold;
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobileDatablock::setTexture( uint8 texUnit, TexturePtr &newTexture )
    {
        if( texUnit >= mNumTextureUnits )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Texture unit out of range in datablock '" +
                         mName.getFriendlyText() + "'", "HlmsGui2DMobileDatablock::setTexture" );
        }

        if( newTexture.isNull() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "New texture can't be null in datablock '" +
                         mName.getFriendlyText() + "'", "HlmsGui2DMobileDatablock::setTexture" );
        }

        mDiffuseTextures[texUnit] = newTexture;
    }
}
