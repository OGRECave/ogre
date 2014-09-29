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
#include "OgreHlmsManager.h"
#include "OgreTexture.h"
#include "OgreLogManager.h"

namespace Ogre
{
    extern CompareFunction convertCompareFunction(const String& param);

    BasicBlock::BasicBlock( uint8 blockType ) :
        mRsData( 0 ),
        mRefCount( 0 ),
        mId( 0 ),
        mBlockType( blockType )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsMacroblock::HlmsMacroblock() :
        BasicBlock( BLOCK_MACRO ),
        mDepthCheck( true ),
        mDepthWrite( true ),
        mDepthFunc( CMPF_LESS_EQUAL ),
        mDepthBiasConstant( 0 ),
        mDepthBiasSlopeScale( 0 ),
        mAlphaToCoverageEnabled( false ),
        mScissorTestEnabled( false ),
        mCullMode( CULL_CLOCKWISE ),
        mPolygonMode( PM_SOLID )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsBlendblock::HlmsBlendblock() :
        BasicBlock( BLOCK_BLEND ),
        mIsTransparent( false ),
        mSeparateBlend( false ),
        mSourceBlendFactor( SBF_ONE ),
        mDestBlendFactor( SBF_ZERO ),
        mSourceBlendFactorAlpha( SBF_ONE ),
        mDestBlendFactorAlpha( SBF_ZERO ),
        mBlendOperation( SBO_ADD ),
        mBlendOperationAlpha( SBO_ADD )
    {
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    HlmsDatablock::HlmsDatablock( IdString name, Hlms *creator, const HlmsMacroblock *macroblock,
                                  const HlmsBlendblock *blendblock,
                                  const HlmsParamVec &params ) :
        mCreator( creator ),
        mName( name ),
        mTextureHash( 0 ),
        mMacroblockHash( (((macroblock->mId) & 0x1F) << 5) | (blendblock->mId & 0x1F) ),
        mType( creator->getType() ),
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

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        if( hlmsManager )
        {
            hlmsManager->destroyMacroblock( mMacroblock );
            hlmsManager->destroyBlendblock( mBlendblock );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setMacroblock( const HlmsMacroblock &macroblock )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        const HlmsMacroblock *oldBlock = mMacroblock;
        mMacroblock = hlmsManager->getMacroblock( macroblock );

        hlmsManager->destroyMacroblock( oldBlock );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setMacroblock( const HlmsMacroblock *macroblock )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        hlmsManager->addReference( macroblock );
        if( mMacroblock )
            hlmsManager->destroyMacroblock( mMacroblock );
        mMacroblock = macroblock;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setBlendblock( const HlmsBlendblock &blendblock )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        const HlmsBlendblock *oldBlock = mBlendblock;
        mBlendblock = hlmsManager->getBlendblock( blendblock );

        hlmsManager->destroyBlendblock( oldBlock );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setBlendblock( const HlmsBlendblock *blendblock )
    {
        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        hlmsManager->addReference( blendblock );
        if( mBlendblock )
            hlmsManager->destroyBlendblock( mBlendblock );
        mBlendblock = blendblock;
    }
    //-----------------------------------------------------------------------------------
    const String* HlmsDatablock::getFullName(void) const
    {
        return mCreator->getFullNameString( mName );
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::_linkRenderable( Renderable *renderable )
    {
        assert( renderable->mHlmsGlobalIndex == (uint32)~0 &&
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
            try
            {
                uint32 hash, casterHash;
                mCreator->calculateHashFor( *itor, hash, casterHash );
                (*itor)->_setHlmsHashes( hash, casterHash );
                ++itor;
            }
            catch( Exception &e )
            {
                size_t currentIdx = itor - mLinkedRenderables.begin();
                LogManager::getSingleton().logMessage( e.getFullDescription() );
                LogManager::getSingleton().logMessage( "Couldn't apply change to datablock '" +
                                                       mName.getFriendlyText() + "' for "
                                                       "this renderable. Using default one. Check "
                                                       "previous log messages to see if there's more "
                                                       "information.", LML_CRITICAL );


                if( mType == HLMS_LOW_LEVEL )
                {
                    HlmsManager *hlmsManager = mCreator->getHlmsManager();
                    (*itor)->setDatablock( hlmsManager->getDefaultDatablock() );
                }
                else
                {
                    //Try to use the default datablock from the same
                    //HLMS as the one the user wanted us to apply
                    (*itor)->setDatablock( mCreator->getDefaultDatablock() );
                }

                //The container was changed with setDatablock change,
                //the iterators may have been invalidated.
                itor = mLinkedRenderables.begin() + currentIdx;
                end  = mLinkedRenderables.end();
            }
        }
    }
    //-----------------------------------------------------------------------------------
}
