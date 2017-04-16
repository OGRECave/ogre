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

#include "OgreHlmsBufferManager.h"
#include "OgreRenderSystem.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbShaderBuffer.h"

namespace Ogre
{
    HlmsBufferManager::HlmsBufferManager( HlmsTypes type, const String &typeName, Archive *dataFolder,
                                          ArchiveVec *libraryFolders ) :
        Hlms( type, typeName, dataFolder, libraryFolders ),
        mVaoManager( 0 ),
        mCurrentConstBuffer( 0 ),
        mCurrentTexBuffer( 0 ),
        mStartMappedConstBuffer( 0 ),
        mCurrentMappedConstBuffer( 0 ),
        mCurrentConstBufferSize( 0 ),
        mRealStartMappedTexBuffer( 0 ),
        mStartMappedTexBuffer( 0 ),
        mCurrentMappedTexBuffer( 0 ),
        mCurrentTexBufferSize( 0 ),
        mTexLastOffset( 0 ),
        mLastTexBufferCmdOffset( (size_t)~0 ),
        mTextureBufferDefaultSize( 4 * 1024 * 1024 )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsBufferManager::~HlmsBufferManager()
    {
        destroyAllBuffers();
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::_changeRenderSystem( RenderSystem *newRs )
    {
        if( mVaoManager )
        {
            destroyAllBuffers();
            mVaoManager = 0;
        }

        if( newRs )
            mVaoManager = newRs->getVaoManager();

        Hlms::_changeRenderSystem( newRs );
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsBufferManager::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                        bool dualParaboloid, SceneManager *sceneManager )
    {
        HlmsCache retVal = Hlms::preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager );

        //mTexBuffers must hold at least one buffer to prevent out of bound exceptions.
        if( mTexBuffers.empty() )
        {
            size_t bufferSize = std::min<size_t>( mTextureBufferDefaultSize,
                                                  mVaoManager->getTexBufferMaxSize() );
            TexBufferPacked *newBuffer = mVaoManager->createTexBuffer( PF_FLOAT32_RGBA, bufferSize,
                                                                       BT_DYNAMIC_PERSISTENT, 0, false );
            mTexBuffers.push_back( newBuffer );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::unmapConstBuffer(void)
    {
        if( mStartMappedConstBuffer )
        {
            //Unmap the current buffer
            ConstBufferPacked *constBuffer = mConstBuffers[mCurrentConstBuffer];
            constBuffer->unmap( UO_KEEP_PERSISTENT, 0,
                                (mCurrentMappedConstBuffer - mStartMappedConstBuffer) * sizeof(uint32) );

            ++mCurrentConstBuffer;

            mStartMappedConstBuffer     = 0;
            mCurrentMappedConstBuffer   = 0;
            mCurrentConstBufferSize     = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    uint32* RESTRICT_ALIAS_RETURN HlmsBufferManager::mapNextConstBuffer( CommandBuffer *commandBuffer )
    {
        unmapConstBuffer();

        if( mCurrentConstBuffer >= mConstBuffers.size() )
        {
            size_t bufferSize = std::min<size_t>( 65536, mVaoManager->getConstBufferMaxSize() );
            ConstBufferPacked *newBuffer = mVaoManager->createConstBuffer( bufferSize,
                                                                           BT_DYNAMIC_PERSISTENT,
                                                                           0, false );
            mConstBuffers.push_back( newBuffer );
        }

        ConstBufferPacked *constBuffer = mConstBuffers[mCurrentConstBuffer];

        mStartMappedConstBuffer     = reinterpret_cast<uint32*>(
                                            constBuffer->map( 0, constBuffer->getNumElements() ) );
        mCurrentMappedConstBuffer   = mStartMappedConstBuffer;
        mCurrentConstBufferSize     = constBuffer->getNumElements() >> 2;

        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( VertexShader, 2,
                                                                       constBuffer, 0, 0 );
        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader, 2,
                                                                       constBuffer, 0, 0 );

        return mStartMappedConstBuffer;
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::unmapTexBuffer( CommandBuffer *commandBuffer )
    {
        //Save our progress
        const size_t bytesWritten = (mCurrentMappedTexBuffer - mRealStartMappedTexBuffer) *
                                                                            sizeof(float);
        mTexLastOffset += bytesWritten;

        if( mRealStartMappedTexBuffer )
        {
            //Unmap the current buffer
            TexBufferPacked *texBuffer = mTexBuffers[mCurrentTexBuffer];
            texBuffer->unmap( UO_KEEP_PERSISTENT, 0, bytesWritten );

            CbShaderBuffer *shaderBufferCmd = reinterpret_cast<CbShaderBuffer*>(
                        commandBuffer->getCommandFromOffset( mLastTexBufferCmdOffset ) );
            if( shaderBufferCmd )
            {
                assert( shaderBufferCmd->bufferPacked == texBuffer );
                shaderBufferCmd->bindSizeBytes = mTexLastOffset - shaderBufferCmd->bindOffset;
                mLastTexBufferCmdOffset = (size_t)~0;
            }
        }

        mRealStartMappedTexBuffer = 0;
        mStartMappedTexBuffer   = 0;
        mCurrentMappedTexBuffer = 0;
        mCurrentTexBufferSize   = 0;

        //Ensure the proper alignment
        mTexLastOffset = alignToNextMultiple( mTexLastOffset, mVaoManager->getTexBufferAlignment() );
    }
    //-----------------------------------------------------------------------------------
    float* RESTRICT_ALIAS_RETURN HlmsBufferManager::mapNextTexBuffer( CommandBuffer *commandBuffer,
                                                                      size_t minimumSizeBytes )
    {
        unmapTexBuffer( commandBuffer );

        TexBufferPacked *texBuffer = mTexBuffers[mCurrentTexBuffer];

        mTexLastOffset = alignToNextMultiple( mTexLastOffset, mVaoManager->getTexBufferAlignment() );

        //We'll go out of bounds. This buffer is full. Get a new one and remap from 0.
        if( mTexLastOffset + minimumSizeBytes >= texBuffer->getTotalSizeBytes() )
        {
            mTexLastOffset = 0;
            ++mCurrentTexBuffer;

            if( mCurrentTexBuffer >= mTexBuffers.size() )
            {
                size_t bufferSize = std::min<size_t>( mTextureBufferDefaultSize,
                                                      mVaoManager->getTexBufferMaxSize() );
                TexBufferPacked *newBuffer = mVaoManager->createTexBuffer( PF_FLOAT32_RGBA, bufferSize,
                                                                           BT_DYNAMIC_PERSISTENT,
                                                                           0, false );
                mTexBuffers.push_back( newBuffer );
            }

            texBuffer = mTexBuffers[mCurrentTexBuffer];
        }

        mRealStartMappedTexBuffer   = reinterpret_cast<float*>(
                                            texBuffer->map( mTexLastOffset,
                                                            texBuffer->getNumElements() - mTexLastOffset,
                                                            false ) );
        mStartMappedTexBuffer   = mRealStartMappedTexBuffer;
        mCurrentMappedTexBuffer = mRealStartMappedTexBuffer;
        mCurrentTexBufferSize   = (texBuffer->getNumElements() - mTexLastOffset) >> 2;

        CbShaderBuffer *shaderBufferCmd = commandBuffer->addCommand<CbShaderBuffer>();
        *shaderBufferCmd = CbShaderBuffer( VertexShader, 0, texBuffer, mTexLastOffset, 0 );

        mLastTexBufferCmdOffset = commandBuffer->getCommandOffset( shaderBufferCmd );

        return mStartMappedTexBuffer;
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::rebindTexBuffer( CommandBuffer *commandBuffer, bool resetOffset,
                                   size_t minimumSizeBytes )
    {
        assert( minimumSizeBytes > 0 );

        //Set the binding size of the old binding command (if exists)
        CbShaderBuffer *shaderBufferCmd = reinterpret_cast<CbShaderBuffer*>(
                    commandBuffer->getCommandFromOffset( mLastTexBufferCmdOffset ) );
        if( shaderBufferCmd )
        {
            assert( shaderBufferCmd->bufferPacked == mTexBuffers[mCurrentTexBuffer] );
            shaderBufferCmd->bindSizeBytes = (mCurrentMappedTexBuffer - mStartMappedTexBuffer) *
                                                sizeof(float);
        }

        const size_t bufferSizeBytes = mCurrentTexBufferSize * sizeof(float);
        size_t currentOffset = (mCurrentMappedTexBuffer - mStartMappedTexBuffer) * sizeof(float);
        currentOffset = alignToNextMultiple( currentOffset, mVaoManager->getTexBufferAlignment() );
        currentOffset = std::min( bufferSizeBytes, currentOffset );
        const size_t remainingSize = bufferSizeBytes - currentOffset;

        if( resetOffset && remainingSize < minimumSizeBytes )
        {
            mapNextTexBuffer( commandBuffer, minimumSizeBytes );
        }
        else
        {
            size_t bindOffset = (mStartMappedTexBuffer - mRealStartMappedTexBuffer) * sizeof(float);
            if( resetOffset )
            {
                mStartMappedTexBuffer = reinterpret_cast<float*>(
                            reinterpret_cast<unsigned char*>(mStartMappedTexBuffer) + currentOffset );
                mCurrentMappedTexBuffer = mStartMappedTexBuffer;
                mCurrentTexBufferSize -= currentOffset / sizeof(float);

                bindOffset = (mCurrentMappedTexBuffer - mRealStartMappedTexBuffer) * sizeof(float);
            }

            if( mTexLastOffset + bindOffset >= mTexBuffers[mCurrentTexBuffer]->getTotalSizeBytes() )
            {
                mapNextTexBuffer( commandBuffer, minimumSizeBytes );
            }
            else
            {
                //Add a new binding command.
                shaderBufferCmd = commandBuffer->addCommand<CbShaderBuffer>();
                *shaderBufferCmd = CbShaderBuffer( VertexShader, 0, mTexBuffers[mCurrentTexBuffer],
                                                   mTexLastOffset + bindOffset, 0 );
                mLastTexBufferCmdOffset = commandBuffer->getCommandOffset( shaderBufferCmd );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::destroyAllBuffers(void)
    {
        mCurrentConstBuffer = 0;
        mCurrentTexBuffer   = 0;
        mTexLastOffset      = 0;

        {
            TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
            TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyTexBuffer( *itor );
                ++itor;
            }

            mTexBuffers.clear();
        }

        {
            ConstBufferPackedVec::const_iterator itor = mConstBuffers.begin();
            ConstBufferPackedVec::const_iterator end  = mConstBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyConstBuffer( *itor );
                ++itor;
            }

            mConstBuffers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::preCommandBufferExecution( CommandBuffer *commandBuffer )
    {
        unmapConstBuffer();
        unmapTexBuffer( commandBuffer );

        TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
        TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

        while( itor != end )
        {
            (*itor)->advanceFrame();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::postCommandBufferExecution( CommandBuffer *commandBuffer )
    {
        TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
        TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

        while( itor != end )
        {
            (*itor)->regressFrame();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::frameEnded(void)
    {
        mCurrentConstBuffer = 0;
        mCurrentTexBuffer   = 0;
        mTexLastOffset      = 0;

        TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
        TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

        while( itor != end )
        {
            (*itor)->advanceFrame();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsBufferManager::setTextureBufferDefaultSize( size_t defaultSize )
    {
        mTextureBufferDefaultSize = defaultSize;
    }
}
