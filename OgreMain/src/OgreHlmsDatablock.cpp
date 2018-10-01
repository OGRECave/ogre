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
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

#include "OgrePass.h"
#include "OgreProfiler.h"

namespace Ogre
{
    extern CompareFunction convertCompareFunction(const String& param);

    BasicBlock::BasicBlock( uint8 blockType ) :
        mRsData( 0 ),
        mRefCount( 0 ),
        mId( 0 ),
        mBlockType( blockType ),
        mAllowGlobalDefaults( 1 )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsMacroblock::HlmsMacroblock() :
        BasicBlock( BLOCK_MACRO ),
        mScissorTestEnabled( false ),
        mDepthCheck( true ),
        mDepthWrite( true ),
        mDepthFunc( CMPF_LESS_EQUAL ),
        mDepthBiasConstant( 0 ),
        mDepthBiasSlopeScale( 0 ),
        mCullMode( CULL_CLOCKWISE ),
        mPolygonMode( PM_SOLID )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsBlendblock::HlmsBlendblock() :
        BasicBlock( BLOCK_BLEND ),
        mAlphaToCoverageEnabled( false ),
        mBlendChannelMask( BlendChannelAll ),
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
    void HlmsBlendblock::setBlendType( SceneBlendType blendType )
    {
        mSeparateBlend = false;
        Pass::_getBlendFlags( blendType, mSourceBlendFactor, mDestBlendFactor );
        mSourceBlendFactorAlpha = mSourceBlendFactor;
        mDestBlendFactorAlpha   = mDestBlendFactor;
    }
    //-----------------------------------------------------------------------------------
    void HlmsBlendblock::setBlendType( SceneBlendType colour, SceneBlendType alpha )
    {
        mSeparateBlend = true;
        Pass::_getBlendFlags( colour, mSourceBlendFactor, mDestBlendFactor );
        Pass::_getBlendFlags( alpha, mSourceBlendFactorAlpha, mDestBlendFactorAlpha );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    HlmsDatablock::HlmsDatablock( IdString name, Hlms *creator, const HlmsMacroblock *macroblock,
                                  const HlmsBlendblock *blendblock,
                                  const HlmsParamVec &params ) :
        mCreator( creator ),
        mName( name ),
        mTextureHash( 0 ),
        mType( creator->getType() ),
        mIgnoreFlushRenderables( false ),
        mAlphaTestCmp( CMPF_ALWAYS_PASS ),
        mAlphaTestShadowCasterOnly( false ),
        mAlphaTestThreshold( 0.5f ),
        mShadowConstantBias( 0.01f )
    {
        mMacroblockHash[0] = mMacroblockHash[1] = 0;
        mMacroblock[0] = mMacroblock[1] = 0;
        mBlendblock[0] = mBlendblock[1] = 0;
        setMacroblock( macroblock, false );
        setBlendblock( blendblock, false );

        //The two previous calls increased the ref. counts by one more than what we need. Correct.
        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        hlmsManager->destroyMacroblock( macroblock );
        hlmsManager->destroyBlendblock( blendblock );

        String paramVal;
        if( Hlms::findParamInVec( params, HlmsBaseProp::AlphaTest, paramVal ) )
        {
            mAlphaTestCmp = CMPF_LESS;

            if( !paramVal.empty() )
            {
                StringVector vec = StringUtil::split( paramVal );

                StringVector::const_iterator itor = vec.begin();
                StringVector::const_iterator end  = vec.end();

                while( itor != end )
                {
                    if( *itor == "less" )
                        mAlphaTestCmp = CMPF_LESS;
                    else if( *itor == "less_equal" )
                        mAlphaTestCmp = CMPF_LESS_EQUAL;
                    else if( *itor == "equal" )
                        mAlphaTestCmp = CMPF_EQUAL;
                    else if( *itor == "greater" )
                        mAlphaTestCmp = CMPF_GREATER;
                    else if( *itor == "greater_equal" )
                        mAlphaTestCmp = CMPF_GREATER_EQUAL;
                    else if( *itor == "not_equal" )
                        mAlphaTestCmp = CMPF_NOT_EQUAL;
                    else
                    {
                        Real val = -1.0f;
                        val = StringConverter::parseReal( *itor, -1.0f );

                        bool error;
                        bool shadowCasterOnly = StringConverter::parseBool( *itor, false, &error );

                        if( val >= 0 )
                        {
                            mAlphaTestThreshold = val;
                        }
                        else if( !error )
                        {
                            mAlphaTestShadowCasterOnly = shadowCasterOnly;
                        }
                        else
                        {
                            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                         mName.getFriendlyText() + ": unknown alpha_test cmp function "
                                         "'" + *itor + "'",
                                         "HlmsDatablock::HlmsDatablock" );
                        }
                    }

                    ++itor;
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock::~HlmsDatablock()
    {
        assert( mLinkedRenderables.empty() &&
                "This Datablock is still being used by some Renderables."
                " Change their Datablocks before destroying this." );

        HlmsManager *hlmsManager = mCreator->getHlmsManager();
        if( hlmsManager )
        {
            for( int i=0; i<2; ++i )
            {
                hlmsManager->destroyMacroblock( mMacroblock[i] );
                hlmsManager->destroyBlendblock( mBlendblock[i] );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsDatablock::clone( String name ) const
    {
        OgreProfileExhaustive( "HlmsDatablock::clone" );

        HlmsDatablock *datablock = mCreator->createDatablock( name, name,
                                                              HlmsMacroblock(), HlmsBlendblock(),
                                                              HlmsParamVec() );

        // Directly const cast macroblocks to keep their mRefCount consistent
        datablock->setMacroblock( const_cast<HlmsMacroblock*>( mMacroblock[0] ), false );
        datablock->setMacroblock( const_cast<HlmsMacroblock*>( mMacroblock[1] ), true );

        // Directly const cast blendblocks to keep their mRefCount consistent
        datablock->setBlendblock( const_cast<HlmsBlendblock*>( mBlendblock[0] ), false );
        datablock->setBlendblock( const_cast<HlmsBlendblock*>( mBlendblock[1] ), true );

        datablock->mAlphaTestCmp = mAlphaTestCmp;
        datablock->mAlphaTestShadowCasterOnly = mAlphaTestShadowCasterOnly;
        datablock->mAlphaTestThreshold = mAlphaTestThreshold;

        datablock->mShadowConstantBias = mShadowConstantBias;

        cloneImpl( datablock );

        datablock->calculateHash();

        return datablock;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setMacroblock( const HlmsMacroblock &macroblock, bool casterBlock )
    {
        OgreProfileExhaustive( "HlmsDatablock::setMacroblockRef" );

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        const HlmsMacroblock *oldBlock = mMacroblock[casterBlock];
        mMacroblock[casterBlock] = hlmsManager->getMacroblock( macroblock );

        if( oldBlock )
            hlmsManager->destroyMacroblock( oldBlock );
        updateMacroblockHash( casterBlock );

        if( !casterBlock )
        {
            mIgnoreFlushRenderables = true;
            bool useBackFaces = mCreator->getHlmsManager()->getShadowMappingUseBackFaces();

            if( useBackFaces && macroblock.mCullMode != CULL_NONE )
            {
                HlmsMacroblock casterblock = macroblock;
                casterblock.mCullMode = macroblock.mCullMode == CULL_CLOCKWISE ? CULL_ANTICLOCKWISE :
                                                                                 CULL_CLOCKWISE;
                setMacroblock( casterblock, true );
            }
            else
            {
                setMacroblock( mMacroblock[0], true );
            }
            mIgnoreFlushRenderables = false;
        }

        if( !mIgnoreFlushRenderables )
        {
            //Force the flush. It's tricky to check if we need this because the new block may
            //be different but be assigned a different ID (old one's API construct was already
            //destroyed) or be equal but have a different ID. It's not random or chaotic and
            //there are guarantees, but it's tricky to get it right and not worth it.
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setMacroblock( const HlmsMacroblock *macroblock, bool casterBlock )
    {
        OgreProfileExhaustive( "HlmsDatablock::setMacroblockPtr" );

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        hlmsManager->addReference( macroblock );
        if( mMacroblock[casterBlock] )
            hlmsManager->destroyMacroblock( mMacroblock[casterBlock] );
        mMacroblock[casterBlock] = macroblock;

        updateMacroblockHash( casterBlock );

        if( !casterBlock )
        {
            mIgnoreFlushRenderables = true;
            bool useBackFaces = mCreator->getHlmsManager()->getShadowMappingUseBackFaces();

            if( useBackFaces && macroblock->mCullMode != CULL_NONE )
            {
                HlmsMacroblock casterblock = *macroblock;
                casterblock.mCullMode = macroblock->mCullMode == CULL_CLOCKWISE ? CULL_ANTICLOCKWISE :
                                                                                  CULL_CLOCKWISE;
                setMacroblock( casterblock, true );
            }
            else
            {
                setMacroblock( mMacroblock[0], true );
            }
            mIgnoreFlushRenderables = false;
        }

        if( !mIgnoreFlushRenderables )
        {
            //Force the flush. It's tricky to check if we need this because the new block may
            //be different but be assigned a different ID (old one's API construct was already
            //destroyed) or be equal but have a different ID. It's not random or chaotic and
            //there are guarantees, but it's tricky to get it right and not worth it.
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setBlendblock( const HlmsBlendblock &blendblock, bool casterBlock )
    {
        OgreProfileExhaustive( "HlmsDatablock::setBlendblockRef" );

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        const HlmsBlendblock *oldBlock = mBlendblock[casterBlock];
        mBlendblock[casterBlock] = hlmsManager->getBlendblock( blendblock );

        if( oldBlock )
            hlmsManager->destroyBlendblock( oldBlock );
        updateMacroblockHash( casterBlock );

        if( !casterBlock )
        {
            mIgnoreFlushRenderables = true;
            setBlendblock( mBlendblock[0], true );
            mIgnoreFlushRenderables = false;
        }

        if( !mIgnoreFlushRenderables )
        {
            //Force the flush. It's tricky to check if we need this because the new block may
            //be different but be assigned a different ID (old one's API construct was already
            //destroyed) or be equal but have a different ID. It's not random or chaotic and
            //there are guarantees, but it's tricky to get it right and not worth it.
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setBlendblock( const HlmsBlendblock *blendblock, bool casterBlock )
    {
        OgreProfileExhaustive( "HlmsDatablock::setBlendblockPtr" );

        HlmsManager *hlmsManager = mCreator->getHlmsManager();

        hlmsManager->addReference( blendblock );
        if( mBlendblock[casterBlock] )
            hlmsManager->destroyBlendblock( mBlendblock[casterBlock] );
        mBlendblock[casterBlock] = blendblock;
        updateMacroblockHash( casterBlock );

        if( !casterBlock )
        {
            mIgnoreFlushRenderables = true;
            setBlendblock( mBlendblock[0], true );
            mIgnoreFlushRenderables = false;
        }

        if( !mIgnoreFlushRenderables )
        {
            //Force the flush. It's tricky to check if we need this because the new block may
            //be different but be assigned a different ID (old one's API construct was already
            //destroyed) or be equal but have a different ID. It's not random or chaotic and
            //there are guarantees, but it's tricky to get it right and not worth it.
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setAlphaTest( CompareFunction compareFunction, bool shadowCasterOnly )
    {
        if( mAlphaTestCmp != compareFunction || mAlphaTestShadowCasterOnly != shadowCasterOnly )
        {
            mAlphaTestCmp = static_cast<CompareFunction>( compareFunction );
            mAlphaTestShadowCasterOnly = shadowCasterOnly;
            flushRenderables();
        }
    }
    //-----------------------------------------------------------------------------------
    CompareFunction HlmsDatablock::getAlphaTest(void) const
    {
        return static_cast<CompareFunction>( mAlphaTestCmp );
    }
    //-----------------------------------------------------------------------------------
    bool HlmsDatablock::getAlphaTestShadowCasterOnly(void) const
    {
        return mAlphaTestShadowCasterOnly;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::setAlphaTestThreshold( float threshold )
    {
        mAlphaTestThreshold = threshold;
    }
    //-----------------------------------------------------------------------------------
    const String* HlmsDatablock::getNameStr(void) const
    {
        return mCreator->getNameStr(mName);
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::getFilenameAndResourceGroup( String const * *outFilename,
                                                     String const * *outResourceGroup ) const
    {
        return mCreator->getFilenameAndResourceGroup( mName, outFilename, outResourceGroup );
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
    void HlmsDatablock::updateMacroblockHash( bool casterPass )
    {
        uint16 macroId = 0;
        uint16 blendId = 0;

        if( mMacroblock[casterPass] )
            macroId = mMacroblock[casterPass]->mId;
        if( mBlendblock[casterPass] )
            blendId = mBlendblock[casterPass]->mId;
        mMacroblockHash[casterPass] = ((macroId & 0x1F) << 5) | (blendId & 0x1F);
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::flushRenderables(void)
    {
        OgreProfileExhaustiveAggr( "HlmsDatablock::flushRenderables" );

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
    bool HlmsDatablock::hasCustomShadowMacroblock(void) const
    {
        const HlmsMacroblock *macroblock0 = mMacroblock[0];
        //Hard copy
        HlmsMacroblock macroblock1 = *mMacroblock[1];

        const bool useBackFaces = mCreator->getHlmsManager()->getShadowMappingUseBackFaces();

        //Revert the flipping
        if( useBackFaces && macroblock0->mCullMode != CULL_NONE && macroblock1.mCullMode != CULL_NONE )
        {
            macroblock1.mCullMode = macroblock1.mCullMode == CULL_CLOCKWISE ? CULL_ANTICLOCKWISE :
                                                                              CULL_CLOCKWISE;
        }

        //Now compare if they're equal
        return *macroblock0 != macroblock1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsDatablock::saveTextures( const String &folderPath, set<String>::type &savedTextures,
                                      bool saveOitd, bool saveOriginal,
                                      HlmsTextureExportListener *listener )
    {
    }
    //-----------------------------------------------------------------------------------
    static const char *c_cmpStrings[NUM_COMPARE_FUNCTIONS+1] =
    {
        "==",   //CMPF_ALWAYS_FAIL (dummy)
        "==",   //CMPF_ALWAYS_PASS (dummy)
        "<",    //CMPF_LESS
        "<=",   //CMPF_LESS_EQUAL
        "==",   //CMPF_EQUAL
        "!=",   //CMPF_NOT_EQUAL
        ">=",   //CMPF_GREATER_EQUAL
        ">",    //CMPF_GREATER
        "==",   //NUM_COMPARE_FUNCTIONS (dummy)
    };
    const char* HlmsDatablock::getCmpString( CompareFunction compareFunction )
    {
        return c_cmpStrings[compareFunction];
    }
    //-----------------------------------------------------------------------------------
}
