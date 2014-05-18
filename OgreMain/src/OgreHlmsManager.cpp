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

namespace Ogre
{
    HlmsManager::HlmsManager() : mRenderSystem( 0 ), mTextureManager( 0 )
    {
        memset( mRegisteredHlms, 0, sizeof( mRegisteredHlms ) );
        memset( mDeleteRegisteredOnExit, 0, sizeof( mDeleteRegisteredOnExit ) );

        mTextureManager = OGRE_NEW HlmsTextureManager();

        mActiveMacroblocks.reserve( OGRE_HLMS_NUM_MACROBLOCKS );
        mFreeMacroblockIds.reserve( OGRE_HLMS_NUM_MACROBLOCKS );
        for( uint8 i=0; i<OGRE_HLMS_NUM_MACROBLOCKS; ++i )
        {
            mMacroblocks[i].mId = i;
            mFreeMacroblockIds.push_back( (OGRE_HLMS_NUM_MACROBLOCKS - 1) - i );
        }

        mActiveBlendblocks.reserve( OGRE_HLMS_NUM_BLENDBLOCKS );
        mFreeBlendblockIds.reserve( OGRE_HLMS_NUM_BLENDBLOCKS );
        for( uint8 i=0; i<OGRE_HLMS_NUM_BLENDBLOCKS; ++i )
        {
            mBlendblocks[i].mId = i;
            mFreeBlendblockIds.push_back( (OGRE_HLMS_NUM_BLENDBLOCKS - 1) - i );
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsManager::~HlmsManager()
    {
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
    const HlmsMacroblock* HlmsManager::getMacroblock( const HlmsMacroblock &baseParams )
    {
        assert( mRenderSystem && "A render system must be selected first!" );

        BlockIdxVec::iterator itor = mActiveMacroblocks.begin();
        BlockIdxVec::iterator end  = mActiveMacroblocks.end();

        while( itor != end && mMacroblocks[*itor] != baseParams )
            ++itor;

        HlmsMacroblock const * retVal = 0;
        if( itor != end )
        {
            //Already exists
            retVal = &mMacroblocks[*itor];
        }
        else
        {
            if( mFreeMacroblockIds.empty() )
            {
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                             "Can't have more than 32 active macroblocks! You have too "
                             "many materials with different rasterizer state parameters.",
                             "HlmsManager::getMacroblock" );
            }

            size_t idx = mFreeMacroblockIds.back();
            mFreeMacroblockIds.pop_back();

            mMacroblocks[idx] = baseParams;
            mRenderSystem->_hlmsMacroblockCreated( &mMacroblocks[idx] );
            mActiveMacroblocks.push_back( idx );

            retVal = &mMacroblocks[idx];
        }

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

        mRenderSystem->_hlmsMacroblockDestroyed( &mMacroblocks[macroblock->mId] );
        mMacroblocks[macroblock->mId].mRsData = 0;

        BlockIdxVec::iterator itor = std::find( mActiveMacroblocks.begin(), mActiveMacroblocks.end(),
                                                macroblock->mId );
        assert( itor != mActiveMacroblocks.end() );
        mActiveMacroblocks.erase( itor );

        mFreeMacroblockIds.push_back( macroblock->mId );
    }
    //-----------------------------------------------------------------------------------
    const HlmsBlendblock* HlmsManager::getBlendblock( const HlmsBlendblock &baseParams )
    {
        assert( mRenderSystem && "A render system must be selected first!" );

        BlockIdxVec::iterator itor = mActiveBlendblocks.begin();
        BlockIdxVec::iterator end  = mActiveBlendblocks.end();

        while( itor != end && mBlendblocks[*itor] != baseParams )
            ++itor;

        HlmsBlendblock const * retVal = 0;
        if( itor != end )
        {
            //Already exists
            retVal = &mBlendblocks[*itor];
        }
        else
        {
            if( mFreeBlendblockIds.empty() )
            {
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                             "Can't have more than 32 active Blendblocks! You have too "
                             "many materials with different blend state parameters.",
                             "HlmsManager::getBlendblock" );
            }

            size_t idx = mFreeBlendblockIds.back();
            mFreeBlendblockIds.pop_back();

            mBlendblocks[idx] = baseParams;
            mRenderSystem->_hlmsBlendblockCreated( &mBlendblocks[idx] );
            mActiveBlendblocks.push_back( idx );


            retVal = &mBlendblocks[idx];
        }

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

        mRenderSystem->_hlmsBlendblockDestroyed( &mBlendblocks[blendblock->mId] );
        mBlendblocks[blendblock->mId].mRsData = 0;

        BlockIdxVec::iterator itor = std::find( mActiveBlendblocks.begin(), mActiveBlendblocks.end(),
                                                blendblock->mId );
        assert( itor != mActiveBlendblocks.end() );
        mActiveBlendblocks.erase( itor );

        mFreeBlendblockIds.push_back( blendblock->mId );
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
        HlmsDatablock *retVal = getDatablockNoThrow( name );

        if( !retVal )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Can't find HLMS datablock material '" +
                         name.getFriendlyText() + "'. It may not be visible to this manager, try "
                         "finding it by retrieving getHlms()->getDatablock()",
                         "HlmsManager::getDatablock" );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsManager::getDatablockNoThrow( IdString name ) const
    {
        HlmsDatablock *retVal = 0;

        HlmsDatablockMap::const_iterator itor = mRegisteredDatablocks.find( name );
        if( itor != mRegisteredDatablocks.end() )
            retVal = itor->second;

        return retVal;
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
    void HlmsManager::renderSystemDestroyAllBlocks(void)
    {
        if( mRenderSystem )
        {
            BlockIdxVec::const_iterator itor = mActiveMacroblocks.begin();
            BlockIdxVec::const_iterator end  = mActiveMacroblocks.end();
            while( itor != end )
                mRenderSystem->_hlmsMacroblockDestroyed( &mMacroblocks[*itor++] );

            itor = mActiveBlendblocks.begin();
            end  = mActiveBlendblocks.end();
            while( itor != end )
                mRenderSystem->_hlmsBlendblockDestroyed( &mBlendblocks[*itor++] );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsManager::_changeRenderSystem( RenderSystem *newRs )
    {
        renderSystemDestroyAllBlocks();
        mRenderSystem = newRs;

        if( mRenderSystem )
        {
            BlockIdxVec::const_iterator itor = mActiveMacroblocks.begin();
            BlockIdxVec::const_iterator end  = mActiveMacroblocks.end();
            while( itor != end )
                mRenderSystem->_hlmsMacroblockCreated( &mMacroblocks[*itor++] );

            itor = mActiveBlendblocks.begin();
            end  = mActiveBlendblocks.end();
            while( itor != end )
                mRenderSystem->_hlmsBlendblockCreated( &mBlendblocks[*itor++] );
        }

        mTextureManager->_changeRenderSystem( newRs );

        for( size_t i=0; i<HLMS_MAX; ++i )
        {
            if( mRegisteredHlms[i] )
                mRegisteredHlms[i]->_changeRenderSystem( newRs );
        }
    }
    //-----------------------------------------------------------------------------------
}
