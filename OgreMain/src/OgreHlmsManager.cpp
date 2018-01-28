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

#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreHlmsTextureManager.h"
#include "OgreRenderSystem.h"
#include "OgreHlmsCompute.h"
#include "OgreLogManager.h"
#if !OGRE_NO_JSON
    #include "OgreResourceGroupManager.h"
#endif

namespace Ogre
{
    HlmsManager::HlmsManager() :
        mComputeHlms( 0 ),
        mRenderSystem( 0 ),
        mShadowMappingUseBackFaces( true ),
        mTextureManager( 0 ),
        mDefaultHlmsType( HLMS_PBS )
  #if !OGRE_NO_JSON
    ,   mJsonListener( 0 )
  #endif
    {
        memset( mRegisteredHlms, 0, sizeof( mRegisteredHlms ) );
        memset( mDeleteRegisteredOnExit, 0, sizeof( mDeleteRegisteredOnExit ) );
        memset( mBlocks, 0, sizeof( mBlocks ) );

        mTextureManager = OGRE_NEW HlmsTextureManager();

        mActiveBlocks[BLOCK_MACRO].reserve( OGRE_HLMS_NUM_MACROBLOCKS );
        mFreeBlockIds[BLOCK_MACRO].reserve( OGRE_HLMS_NUM_MACROBLOCKS );
        for( uint8 i=0; i<OGRE_HLMS_NUM_MACROBLOCKS; ++i )
        {
            mMacroblocks[i].mId = i;
            mBlocks[BLOCK_MACRO][i] = &mMacroblocks[i];
            mFreeBlockIds[BLOCK_MACRO].push_back( (OGRE_HLMS_NUM_MACROBLOCKS - 1) - i );
        }

        mActiveBlocks[BLOCK_BLEND].reserve( OGRE_HLMS_NUM_BLENDBLOCKS );
        mFreeBlockIds[BLOCK_BLEND].reserve( OGRE_HLMS_NUM_BLENDBLOCKS );
        for( uint8 i=0; i<OGRE_HLMS_NUM_BLENDBLOCKS; ++i )
        {
            mBlendblocks[i].mId = i;
            mBlocks[BLOCK_BLEND][i] = &mBlendblocks[i];
            mFreeBlockIds[BLOCK_BLEND].push_back( (OGRE_HLMS_NUM_BLENDBLOCKS - 1) - i );
        }

        mActiveBlocks[BLOCK_SAMPLER].reserve( OGRE_HLMS_NUM_SAMPLERBLOCKS );
        mFreeBlockIds[BLOCK_SAMPLER].reserve( OGRE_HLMS_NUM_SAMPLERBLOCKS );
        for( uint8 i=0; i<OGRE_HLMS_NUM_SAMPLERBLOCKS; ++i )
        {
            mSamplerblocks[i].mId = i;
            mBlocks[BLOCK_SAMPLER][i] = &mSamplerblocks[i];
            mFreeBlockIds[BLOCK_SAMPLER].push_back( (OGRE_HLMS_NUM_SAMPLERBLOCKS - 1) - i );
        }

        mFreeInputLayouts.reserve( OGRE_HLMS_NUM_INPUT_LAYOUTS );
        for( uint8 i=0; i<OGRE_HLMS_NUM_INPUT_LAYOUTS; ++i )
        {
            mInputLayouts[i].opType = OT_POINT_LIST;
            mInputLayouts[i].refCount = 0;
            mFreeInputLayouts.push_back( (OGRE_HLMS_NUM_INPUT_LAYOUTS - 1) - i );
        }

#if !OGRE_NO_JSON
        mScriptPatterns.push_back( "*.material.json" );
        ResourceGroupManager::getSingleton()._registerScriptLoader(this);
#endif
    }
    //-----------------------------------------------------------------------------------
    HlmsManager::~HlmsManager()
    {
#if !OGRE_NO_JSON
        ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
#endif

        renderSystemDestroyAllBlocks();

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        for( size_t i=0; i<HLMS_MAX; ++i )
        {
            if( mRegisteredHlms[i] )
            {
                mRegisteredHlms[i]->_notifyManager( 0 );
                if( mDeleteRegisteredOnExit[i] )
                {
                    OGRE_DELETE mRegisteredHlms[i];
                    mRegisteredHlms[i] = 0;
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    Hlms* HlmsManager::getHlms( IdString name )
    {
        Hlms* retVal = NULL;

        for( size_t i=0; i<HLMS_MAX && !retVal; ++i )
        {
            if( mRegisteredHlms[i] && mRegisteredHlms[i]->getTypeName() == name )
            {
                retVal = mRegisteredHlms[i];
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::addReference( const BasicBlock *block )
    {
        BasicBlock *realBlock = mBlocks[block->mBlockType][block->mId];
        if( realBlock != block )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "The block wasn't created with this manager!",
                         "HlmsManager::addReference" );
        }

        ++realBlock->mRefCount;
    }
    //-----------------------------------------------------------------------------------
    uint16 HlmsManager::getFreeBasicBlock( uint8 type )
    {
        if( mFreeBlockIds[type].empty() )
        {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                         "Can't have more than " +
                         StringConverter::toString( mActiveBlocks[type].size() ) +
                         " active blocks! You have too "
                         "many materials with different rasterizer state, "
                         "blending state, or sampler state parameters.",
                         "HlmsManager::getFreeBasicBlock" );
        }

        size_t idx = mFreeBlockIds[type].back();
        mFreeBlockIds[type].pop_back();

        mActiveBlocks[type].push_back( idx );

        return idx;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::destroyBasicBlock( BasicBlock *block )
    {
        block->mRsData = 0;

        BlockIdxVec::iterator itor = std::find( mActiveBlocks[block->mBlockType].begin(),
                                                mActiveBlocks[block->mBlockType].end(),
                                                block->mId );
        assert( itor != mActiveBlocks[block->mBlockType].end() );
        mActiveBlocks[block->mBlockType].erase( itor );

        mFreeBlockIds[block->mBlockType].push_back( block->mId );
    }
    //-----------------------------------------------------------------------------------
    const HlmsMacroblock* HlmsManager::getMacroblock( const HlmsMacroblock &baseParams )
    {
        assert( mRenderSystem && "A render system must be selected first!" );
        assert( baseParams.mBlockType == BLOCK_MACRO &&
                "baseParams.mBlockType should always be BLOCK_MACRO! You can ignore this assert,"
                " but it usually indicates memory corruption (or you created the block without "
                "its default constructor)." );

        BlockIdxVec::iterator itor = mActiveBlocks[BLOCK_MACRO].begin();
        BlockIdxVec::iterator end  = mActiveBlocks[BLOCK_MACRO].end();

        while( itor != end && mMacroblocks[*itor] != baseParams )
            ++itor;

        HlmsMacroblock *retVal = 0;
        if( itor != end )
        {
            //Already exists
            retVal = &mMacroblocks[*itor];
        }
        else
        {
            size_t idx = getFreeBasicBlock( BLOCK_MACRO );

            mMacroblocks[idx] = baseParams;
            //Restore the values which has just been overwritten and we need properly set.
            mMacroblocks[idx].mRefCount = 0;
            mMacroblocks[idx].mId       = idx;
            mMacroblocks[idx].mBlockType= BLOCK_MACRO;
            mRenderSystem->_hlmsMacroblockCreated( &mMacroblocks[idx] );

            retVal = &mMacroblocks[idx];
        }

        assert( retVal->mRefCount < 0xFFFF && "Reference count overflow!" );
        ++retVal->mRefCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::destroyMacroblock( const HlmsMacroblock *macroblock )
    {
        if( &mMacroblocks[macroblock->mId] != macroblock )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "The macroblock wasn't created with this manager!",
                         "HlmsManager::destroyMacroblock" );
        }

        --mMacroblocks[macroblock->mId].mRefCount;

        if( !mMacroblocks[macroblock->mId].mRefCount )
        {
            for( size_t i=0; i<HLMS_MAX; ++i )
            {
                if( mRegisteredHlms[i] )
                    mRegisteredHlms[i]->_notifyMacroblockDestroyed( macroblock->mId );
            }

            mRenderSystem->_hlmsMacroblockDestroyed( &mMacroblocks[macroblock->mId] );
            destroyBasicBlock( &mMacroblocks[macroblock->mId] );
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsBlendblock* HlmsManager::getBlendblock( const HlmsBlendblock &baseParams )
    {
        assert( mRenderSystem && "A render system must be selected first!" );
        assert( baseParams.mBlockType == BLOCK_BLEND &&
                "baseParams.mBlockType should always be BLOCK_BLEND! You can ignore this assert,"
                " but it usually indicates memory corruption (or you created the block without "
                "its default constructor)." );

        BlockIdxVec::iterator itor = mActiveBlocks[BLOCK_BLEND].begin();
        BlockIdxVec::iterator end  = mActiveBlocks[BLOCK_BLEND].end();

        while( itor != end && mBlendblocks[*itor] != baseParams )
            ++itor;

        HlmsBlendblock *retVal = 0;
        if( itor != end )
        {
            //Already exists
            retVal = &mBlendblocks[*itor];
        }
        else
        {
            size_t idx = getFreeBasicBlock( BLOCK_BLEND );

            mBlendblocks[idx] = baseParams;

            //Restore the values which has just been overwritten and we need properly set.
            mBlendblocks[idx].mRefCount = 0;
            mBlendblocks[idx].mId       = idx;
            mBlendblocks[idx].mBlockType= BLOCK_BLEND;

            mBlendblocks[idx].mIsTransparent =
                     !( baseParams.mDestBlendFactor == SBF_ZERO &&
                        baseParams.mSourceBlendFactor != SBF_DEST_COLOUR &&
                        baseParams.mSourceBlendFactor != SBF_ONE_MINUS_DEST_COLOUR &&
                        baseParams.mSourceBlendFactor != SBF_DEST_ALPHA &&
                        baseParams.mSourceBlendFactor != SBF_ONE_MINUS_DEST_ALPHA );
            mRenderSystem->_hlmsBlendblockCreated( &mBlendblocks[idx] );

            retVal = &mBlendblocks[idx];
        }

        assert( retVal->mRefCount < 0xFFFF && "Reference count overflow!" );
        ++retVal->mRefCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::destroyBlendblock( const HlmsBlendblock *blendblock )
    {
        if( &mBlendblocks[blendblock->mId] != blendblock )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "The Blendblock wasn't created with this manager!",
                         "HlmsManager::destroyBlendblock" );
        }

        --mBlendblocks[blendblock->mId].mRefCount;

        if( !mBlendblocks[blendblock->mId].mRefCount )
        {
            for( size_t i=0; i<HLMS_MAX; ++i )
            {
                if( mRegisteredHlms[i] )
                    mRegisteredHlms[i]->_notifyBlendblockDestroyed( blendblock->mId );
            }

            mRenderSystem->_hlmsBlendblockDestroyed( &mBlendblocks[blendblock->mId] );
            destroyBasicBlock( &mBlendblocks[blendblock->mId] );
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsSamplerblock* HlmsManager::getSamplerblock( HlmsSamplerblock baseParams )
    {
        assert( mRenderSystem && "A render system must be selected first!" );
        assert( baseParams.mBlockType == BLOCK_SAMPLER &&
                "baseParams.mBlockType should always be BLOCK_SAMPLER! You can ignore this assert,"
                " but it usually indicates memory corruption (or you created the block without "
                "its default constructor)." );

        bool errorsFixed = false;

        if( baseParams.mMaxAnisotropy < 1.0f )
        {
            baseParams.mMaxAnisotropy = 1.0f;
            LogManager::getSingleton().logMessage( "WARNING: Max anisotropy can't be lower than 1" );
        }

        if( baseParams.mMinFilter != FO_ANISOTROPIC && baseParams.mMagFilter != FO_ANISOTROPIC &&
            baseParams.mMipFilter != FO_ANISOTROPIC && baseParams.mMaxAnisotropy > 1.0f )
        {
            baseParams.mMaxAnisotropy = 1.0f;
            LogManager::getSingleton().logMessage( "WARNING: Max anisotropy must be 1 if no anisotropic "
                                                   "filter is used." );
        }

        if( errorsFixed )
        {
            LogManager::getSingleton().logMessage( "WARNING: Invalid sampler block parameters detected."
                                                   " They've been corrected." );
        }

        BlockIdxVec::iterator itor = mActiveBlocks[BLOCK_SAMPLER].begin();
        BlockIdxVec::iterator end  = mActiveBlocks[BLOCK_SAMPLER].end();

        while( itor != end && mSamplerblocks[*itor] != baseParams )
            ++itor;

        HlmsSamplerblock *retVal = 0;
        if( itor != end )
        {
            //Already exists
            retVal = &mSamplerblocks[*itor];
        }
        else
        {
            size_t idx = getFreeBasicBlock( BLOCK_SAMPLER );

            mSamplerblocks[idx] = baseParams;
            //Restore the values which has just been overwritten and we need properly set.
            mSamplerblocks[idx].mRefCount   = 0;
            mSamplerblocks[idx].mId         = idx;
            mSamplerblocks[idx].mBlockType  = BLOCK_SAMPLER;
            mRenderSystem->_hlmsSamplerblockCreated( &mSamplerblocks[idx] );

            retVal = &mSamplerblocks[idx];
        }

        assert( retVal->mRefCount < 0xFFFF && "Reference count overflow!" );
        ++retVal->mRefCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::destroySamplerblock( const HlmsSamplerblock *samplerblock )
    {
        if( &mSamplerblocks[samplerblock->mId] != samplerblock )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "The Samplerblock wasn't created with this manager!",
                         "HlmsManager::destroySamplerblock" );
        }

        --mSamplerblocks[samplerblock->mId].mRefCount;

        if( !mSamplerblocks[samplerblock->mId].mRefCount )
        {
            mRenderSystem->_hlmsSamplerblockDestroyed( &mSamplerblocks[samplerblock->mId] );
            destroyBasicBlock( &mSamplerblocks[samplerblock->mId] );
        }
    }
    //-----------------------------------------------------------------------------------
    uint8 HlmsManager::_addInputLayoutId( VertexElement2VecVec vertexElements, OperationType opType )
    {
        InputLayoutsIdVec::const_iterator itor = mActiveInputLayouts.begin();
        InputLayoutsIdVec::const_iterator end  = mActiveInputLayouts.end();

        while( itor != end &&
               (mInputLayouts[*itor].vertexElements != vertexElements ||
                mInputLayouts[*itor].opType != opType) )
        {
            ++itor;
        }

        uint8 retVal = 0;
        if( itor != end )
        {
            //Already exists
            retVal = *itor;
        }
        else
        {
            if( mFreeInputLayouts.empty() )
            {
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                             "Can't have more than 256 active input layouts! "
                             "You have too many different vertex formats.",
                             "HlmsManager::_addInputLayoutId" );
            }

            retVal = mFreeInputLayouts.back();
            mFreeInputLayouts.pop_back();

            mActiveInputLayouts.push_back( retVal );

            mInputLayouts[retVal].opType            = opType;
            mInputLayouts[retVal].vertexElements    = vertexElements;
            mInputLayouts[retVal].refCount          = 0;
        }

        ++mInputLayouts[retVal].refCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::_removeInputLayoutIdReference( uint8 layoutId )
    {
        --mInputLayouts[layoutId].refCount;

        if( !mInputLayouts[layoutId].refCount )
        {
            for( int i=0; i<HLMS_MAX; ++i )
            {
                if( mRegisteredHlms[i] )
                    mRegisteredHlms[i]->_notifyInputLayoutDestroyed( layoutId );
            }

            InputLayoutsIdVec::iterator itor = std::find( mActiveInputLayouts.begin(),
                                                          mActiveInputLayouts.end(),
                                                          layoutId );

            assert( itor != mActiveInputLayouts.end() );
            mActiveInputLayouts.erase( itor );

            mFreeInputLayouts.push_back( layoutId );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::_notifyV1InputLayoutDestroyed( uint8 v1LayoutId )
    {
        for( int i=0; i<HLMS_MAX; ++i )
        {
            if( mRegisteredHlms[i] )
                mRegisteredHlms[i]->_notifyV1InputLayoutDestroyed( v1LayoutId );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::_datablockAdded( HlmsDatablock *datablock )
    {
        IdString datablockName = datablock->getName();
        if( mRegisteredDatablocks.find( datablockName ) != mRegisteredDatablocks.end() )
        {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "HLMS Datablock '" +
                         datablockName.getFriendlyText() + "' already exists, probably "
                         "created by another type of HLMS",
                         "HlmsManager::_datablockAdded" );
        }

        mRegisteredDatablocks[datablockName] = datablock;
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::_datablockDestroyed( IdString name )
    {
        HlmsDatablockMap::iterator itor = mRegisteredDatablocks.find( name );

        assert( itor != mRegisteredDatablocks.end() );

        if( itor != mRegisteredDatablocks.end() )
        {
            //We don't delete the pointer, as we don't own it (the Hlms class owns it)
            mRegisteredDatablocks.erase( itor );
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsManager::getDatablock( IdString name ) const
    {
        HlmsDatablock *retVal = getDatablockNoDefault( name );

        if( !retVal )
        {
            LogManager::getSingleton().logMessage( "Can't find HLMS datablock material '" +
                         name.getFriendlyText() + "'. It may not be visible to this manager, try "
                         "finding it by retrieving getHlms()->getDatablock()", LML_CRITICAL );

            retVal = getDefaultDatablock();
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsManager::getDatablockNoDefault( IdString name ) const
    {
        HlmsDatablock *retVal = 0;

        HlmsDatablockMap::const_iterator itor = mRegisteredDatablocks.find( name );
        if( itor != mRegisteredDatablocks.end() )
            retVal = itor->second;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsManager::getDefaultDatablock(void) const
    {
        return mRegisteredHlms[mDefaultHlmsType]->getDefaultDatablock();
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::registerHlms( Hlms *provider, bool deleteOnExit )
    {
        HlmsTypes type = provider->getType();

        if( mRegisteredHlms[type] )
        {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "Provider for HLMS type '" +
                         StringConverter::toString( type ) + "' has already been set!",
                         "HlmsManager::registerHlms" );
        }

        mDeleteRegisteredOnExit[type] = deleteOnExit;
        mRegisteredHlms[type] = provider;
        mRegisteredHlms[type]->_notifyManager( this );
        mRegisteredHlms[type]->_changeRenderSystem( mRenderSystem );
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::unregisterHlms( HlmsTypes type )
    {
        if( mRegisteredHlms[type] )
        {
            //TODO: Go through all the MovableObjects and remove the Hlms?
            mRegisteredHlms[type]->_notifyManager( 0 );
            if( mDeleteRegisteredOnExit[type] )
                OGRE_DELETE mRegisteredHlms[type];
            mRegisteredHlms[type] = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::registerComputeHlms( HlmsCompute *provider )
    {
        if( mComputeHlms )
        {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "Provider for HLMS type 'Compute'"
                         " has already been set!", "HlmsManager::registerComputeHlms" );
        }

        mComputeHlms = provider;
        mComputeHlms->_notifyManager( this );
        mComputeHlms->_changeRenderSystem( mRenderSystem );
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::unregisterComputeHlms(void)
    {
        if( mComputeHlms )
        {
            mComputeHlms->_notifyManager( 0 );
            mComputeHlms = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::setShadowMappingUseBackFaces( bool useBackFaces )
    {
        if( mShadowMappingUseBackFaces != useBackFaces )
        {
            mShadowMappingUseBackFaces = useBackFaces;

            for( int i=0; i<HLMS_MAX; ++i )
            {
                if( mRegisteredHlms[i] )
                    mRegisteredHlms[i]->_notifyShadowMappingBackFaceSetting();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::renderSystemDestroyAllBlocks(void)
    {
        if( mRenderSystem )
        {
            for( size_t i=0; i<HLMS_MAX; ++i )
            {
                if( mRegisteredHlms[i] )
                    mRegisteredHlms[i]->_clearShaderCache();
            }

            BlockIdxVec::const_iterator itor = mActiveBlocks[BLOCK_MACRO].begin();
            BlockIdxVec::const_iterator end  = mActiveBlocks[BLOCK_MACRO].end();
            while( itor != end )
                mRenderSystem->_hlmsMacroblockDestroyed( &mMacroblocks[*itor++] );

            itor = mActiveBlocks[BLOCK_BLEND].begin();
            end  = mActiveBlocks[BLOCK_BLEND].end();
            while( itor != end )
                mRenderSystem->_hlmsBlendblockDestroyed( &mBlendblocks[*itor++] );

            itor = mActiveBlocks[BLOCK_SAMPLER].begin();
            end  = mActiveBlocks[BLOCK_SAMPLER].end();
            while( itor != end )
                mRenderSystem->_hlmsSamplerblockDestroyed( &mSamplerblocks[*itor++] );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::_changeRenderSystem( RenderSystem *newRs )
    {
        renderSystemDestroyAllBlocks();
        mRenderSystem = newRs;

        if( mRenderSystem )
        {
            BlockIdxVec::const_iterator itor = mActiveBlocks[BLOCK_MACRO].begin();
            BlockIdxVec::const_iterator end  = mActiveBlocks[BLOCK_MACRO].end();
            while( itor != end )
                mRenderSystem->_hlmsMacroblockCreated( &mMacroblocks[*itor++] );

            itor = mActiveBlocks[BLOCK_BLEND].begin();
            end  = mActiveBlocks[BLOCK_BLEND].end();
            while( itor != end )
                mRenderSystem->_hlmsBlendblockCreated( &mBlendblocks[*itor++] );

            itor = mActiveBlocks[BLOCK_SAMPLER].begin();
            end  = mActiveBlocks[BLOCK_SAMPLER].end();
            while( itor != end )
                mRenderSystem->_hlmsSamplerblockCreated( &mSamplerblocks[*itor++] );
        }

        mTextureManager->_changeRenderSystem( newRs );

        for( size_t i=0; i<HLMS_MAX; ++i )
        {
            if( mRegisteredHlms[i] )
                mRegisteredHlms[i]->_changeRenderSystem( newRs );
        }
    }
#if !OGRE_NO_JSON
    //-----------------------------------------------------------------------------------
    void HlmsManager::loadMaterials( const String &filename, const String &groupName,
                                     HlmsJsonListener *listener,
                                     const String &additionalTextureExtension )
    {
        DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource( filename, groupName );

        vector<char>::type fileData;
        fileData.resize( stream->size() + 1 );
        if( !fileData.empty() )
        {
            stream->read( &fileData[0], stream->size() );

            //Add null terminator just in case (to prevent bad input)
            fileData.back() = '\0';
            HlmsJson hlmsJson( this, listener );
            hlmsJson.loadMaterials( stream->getName(), groupName, &fileData[0],
                                    additionalTextureExtension );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::saveMaterials( HlmsTypes hlmsType, const String &filename,
                                     HlmsJsonListener *listener,
                                     const String &additionalTextureExtension )
    {
        assert( hlmsType != HLMS_MAX );
        assert( hlmsType != HLMS_LOW_LEVEL );

        String jsonString;
        HlmsJson hlmsJson( this, listener );
        hlmsJson.saveMaterials( mRegisteredHlms[hlmsType], jsonString, additionalTextureExtension );

        std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out );
        if( file.is_open() )
            file.write( jsonString.c_str(), jsonString.size() );
        file.close();
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::saveMaterial( const HlmsDatablock *datablock, const String &filename,
                                    HlmsJsonListener *listener,
                                    const String &additionalTextureExtension )
    {
        String jsonString;
        HlmsJson hlmsJson( this, listener );
        hlmsJson.saveMaterial( datablock, jsonString, additionalTextureExtension );

        std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out );
        if( file.is_open() )
            file.write( jsonString.c_str(), jsonString.size() );
        file.close();
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        vector<char>::type fileData;
        fileData.resize( stream->size() + 1 );
        if( !fileData.empty() )
        {
            stream->read( &fileData[0], stream->size() );

            String additionalTextureExtension;
            ResourceToTexExtensionMap::const_iterator itExt =
                    mAdditionalTextureExtensionsPerGroup.find( groupName );

            if( itExt != mAdditionalTextureExtensionsPerGroup.end() )
                additionalTextureExtension = itExt->second;

            //Add null terminator just in case (to prevent bad input)
            fileData.back() = '\0';
            HlmsJson hlmsJson( this, mJsonListener );
            hlmsJson.loadMaterials( stream->getName(), groupName, &fileData[0],
                                    additionalTextureExtension );
        }
    }
    //-----------------------------------------------------------------------------------
    Real HlmsManager::getLoadingOrder(void) const
    {
        return 100;
    }
#endif
    //-----------------------------------------------------------------------------------
    const HlmsManager::BlockIdxVec& HlmsManager::_getActiveBlocksIndices( const HlmsBasicBlock &blockType ) const
    {
        return mActiveBlocks[blockType];
    }
    //-----------------------------------------------------------------------------------
    BasicBlock const * const * HlmsManager::_getBlocks( const HlmsBasicBlock &blockType ) const
    {
        return mBlocks[blockType];
    }
    //-----------------------------------------------------------------------------------
    const HlmsMacroblock* HlmsManager::_getMacroblock( uint16 idx ) const
    {
        assert( idx < OGRE_HLMS_NUM_MACROBLOCKS );
        return &mMacroblocks[idx];
    }
    //-----------------------------------------------------------------------------------
    const HlmsBlendblock* HlmsManager::_getBlendblock( uint16 idx ) const
    {
        assert( idx < OGRE_HLMS_NUM_BLENDBLOCKS );
        return &mBlendblocks[idx];
    }
    //-----------------------------------------------------------------------------------
    const HlmsSamplerblock* HlmsManager::_getSamplerblock( uint16 idx ) const
    {
        assert( idx < OGRE_HLMS_NUM_SAMPLERBLOCKS );
        return &mSamplerblocks[idx];
    }
}
