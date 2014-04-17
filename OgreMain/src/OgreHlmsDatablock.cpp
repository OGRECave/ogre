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

#include "OgreHlmsDatablock.h"
#include "OgreTexture.h"

namespace Ogre
{
    extern CompareFunction convertCompareFunction(const String& param);

    HlmsMacroblock::HlmsMacroblock() :
        mDepthCheck( true ),
        mDepthWrite( true ),
        mDepthFunc( CMPF_LESS_EQUAL ),
        mDepthBiasConstant( 0 ),
        mDepthBiasSlopeScale( 0 ),
        mAlphaToCoverageEnabled( false ),
        mCullMode( CULL_CLOCKWISE ),
        mPolygonMode( PM_SOLID )
    {

    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    HlmsDatablock::HlmsDatablock( HlmsTypes type, const HlmsMacroblock *macroblock, uint8 macroblockId,
                                  const HlmsBlendblock *blendblock, uint8 blendblockId,
                                  const HlmsParamVec &params ) :
        mMacroblockHash( (((macroblockId) & 0x1F) << 5) | (blendblockId & 0x1F) ),
        mTextureHash( ~0 ),
        mType( type ),
        mIsOpaque( true ),
        mMacroblock( macroblock ),
        mBlendblock( blendblock ),
        mOriginalParams( params )
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    HlmsPbsEs2Datablock::HlmsPbsEs2Datablock( const HlmsMacroblock *macroblock, uint8 macroblockId,
                                              const HlmsBlendblock *blendblock, uint8 blendblockId,
                                              const HlmsParamVec &params ) :
        HlmsDatablock( HLMS_PBS, macroblock, macroblockId, blendblock, blendblockId, params )
    {
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsEs2Datablock::calculateHash()
    {
        IdString hash;
        if( !mDiffuseTex.isNull() )
            hash += IdString( mDiffuseTex->getName() );
        if( !mNormalmapTex.isNull() )
            hash += IdString( mNormalmapTex->getName() );
        if( !mSpecularTex.isNull() )
            hash += IdString( mSpecularTex->getName() );
        if( !mReflectionTex.isNull() )
            hash += IdString( mReflectionTex->getName() );

        mTextureHash = hash.mHash;
    }
}
