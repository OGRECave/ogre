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
#include "OgreHlms.h"
#include "OgreTexture.h"

namespace Ogre
{
    extern CompareFunction convertCompareFunction(const String& param);

    HlmsMacroblock::HlmsMacroblock() :
        mId( 0 ),
        mDepthCheck( true ),
        mDepthWrite( true ),
        mDepthFunc( CMPF_LESS_EQUAL ),
        mDepthBiasConstant( 0 ),
        mDepthBiasSlopeScale( 0 ),
        mAlphaToCoverageEnabled( false ),
        mScissorTestEnabled( false ),
        mCullMode( CULL_CLOCKWISE ),
        mPolygonMode( PM_SOLID ),
        mRsData( 0 )
    {
    }
    HlmsBlendblock::HlmsBlendblock() :
        mId( 0 ),
        mSeparateBlend( false ),
        mSourceBlendFactor( SBF_ONE ),
        mDestBlendFactor( SBF_ZERO ),
        mSourceBlendFactorAlpha( SBF_ONE ),
        mDestBlendFactorAlpha( SBF_ZERO ),
        mBlendOperation( SBO_ADD ),
        mBlendOperationAlpha( SBO_ADD ),
        mRsData( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    HlmsDatablock::HlmsDatablock( IdString name, Hlms *creator, const HlmsMacroblock *macroblock,
                                  const HlmsBlendblock *blendblock,
                                  const HlmsParamVec &params ) :
        mCreator( creator ),
        mName( name ),
        mMacroblockHash( (((macroblock->mId) & 0x1F) << 5) | (blendblock->mId & 0x1F) ),
        mTextureHash( 0 ),
        mType( creator->getType() ),
        mIsOpaque( blendblock->mDestBlendFactor == SBF_ZERO &&
                   blendblock->mSourceBlendFactor != SBF_DEST_COLOUR &&
                   blendblock->mSourceBlendFactor != SBF_ONE_MINUS_DEST_COLOUR &&
                   blendblock->mSourceBlendFactor != SBF_DEST_ALPHA &&
                   blendblock->mSourceBlendFactor != SBF_ONE_MINUS_DEST_ALPHA ),
        mMacroblock( macroblock ),
        mBlendblock( blendblock ),
        mAlphaTest( false ),
        mAlphaTestThreshold( 0.5f ),
        mShadowConstantBias( 0.01f )
    {
    }
    HlmsDatablock::~HlmsDatablock()
    {
        assert( mLinkedRenderables.empty() &&
                "This Datablock is still being used by some Renderables."
                " Change their Datablocks before destroying this." );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setBlendblock( HlmsBlendblock const *blendblock )
    {
        mBlendblock = blendblock;
        mIsOpaque = blendblock->mDestBlendFactor == SBF_ZERO &&
                    blendblock->mSourceBlendFactor != SBF_DEST_COLOUR &&
                    blendblock->mSourceBlendFactor != SBF_ONE_MINUS_DEST_COLOUR &&
                    blendblock->mSourceBlendFactor != SBF_DEST_ALPHA &&
                    blendblock->mSourceBlendFactor != SBF_ONE_MINUS_DEST_ALPHA;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::_linkRenderable( Renderable *renderable )
    {
        assert( renderable->mHlmsGlobalIndex == ~0 &&
                "Renderable must be unlinked before being linked again!" );

        renderable->mHlmsGlobalIndex = mLinkedRenderables.size();
        mLinkedRenderables.push_back( renderable );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::_unlinkRenderable( Renderable *renderable )
    {
        if( renderable->mHlmsGlobalIndex >= mLinkedRenderables.size() ||
            renderable != *(mLinkedRenderables.begin() + renderable->mHlmsGlobalIndex) )
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "A Renderable had it's mHlmsGlobalIndex out of "
                "date!!! (or the Renderable wasn't being tracked by this datablock)",
                "HlmsDatablock::_removeRenderable" );
        }

        vector<Renderable*>::type::iterator itor = mLinkedRenderables.begin() +
                                                    renderable->mHlmsGlobalIndex;
        itor = efficientVectorRemove( mLinkedRenderables, itor );

        //The Renderable that was at the end got swapped and has now a different index
        if( itor != mLinkedRenderables.end() )
            (*itor)->mHlmsGlobalIndex = itor - mLinkedRenderables.begin();

        renderable->mHlmsGlobalIndex = ~0;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setAlphaTest( bool bEnabled )
    {
        if( bEnabled != mAlphaTest )
        {
            mAlphaTest = bEnabled;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::flushRenderables(void)
    {
        vector<Renderable*>::type::const_iterator itor = mLinkedRenderables.begin();
        vector<Renderable*>::type::const_iterator end  = mLinkedRenderables.end();

        while( itor != end )
        {
            uint32 hash, casterHash;
            mCreator->calculateHashFor( *itor, hash, casterHash );
            (*itor)->_setHlmsHashes( hash, casterHash );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
}
