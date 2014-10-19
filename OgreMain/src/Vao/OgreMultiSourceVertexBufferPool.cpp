/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "Vao/OgreMultiSourceVertexBufferPool.h"
#include "Vao/OgreVaoManager.h"

#include "OgreStringConverter.h"

namespace Ogre
{
    MultiSourceVertexBufferPool::MultiSourceVertexBufferPool(
                            const VertexElement2VecVec &vertexElementsBySource,
                            size_t maxVertices, BufferType bufferType,
                            size_t internalBufferStart, VaoManager *vaoManager ) :
        mVertexElementsBySource( vertexElementsBySource ),
        mMaxVertices( maxVertices ),
        mBufferType( bufferType ),
        mInternalBufferStart( internalBufferStart ),
        mVaoManager( vaoManager )
    {
        mBytesPerVertexPerSource.reserve( mVertexElementsBySource.size() );
        mSourceOffset.reserve( mVertexElementsBySource.size() );

        uint8 dynamicBufferMultiplier = 1;
        if( mBufferType >= BT_DYNAMIC_DEFAULT )
            dynamicBufferMultiplier = mVaoManager->getDynamicBufferMultiplier();

        size_t accumulatedOffset = 0;

        for( size_t i=0; i<mVertexElementsBySource.size(); ++i )
        {
            uint32 bytesPerVertex = VaoManager::calculateVertexSize( mVertexElementsBySource[i] );

            //Add padding when needed (needed if the buffer is rendered solo)
            //Round to next multiple of bytesPerElement
            accumulatedOffset = ( (accumulatedOffset + bytesPerVertex - 1) /
                                 bytesPerVertex ) * bytesPerVertex;

            mBytesPerVertexPerSource.push_back( bytesPerVertex );
            mSourceOffset.push_back( accumulatedOffset );

            accumulatedOffset += bytesPerVertex * mMaxVertices * dynamicBufferMultiplier;
        }
    }
    //-----------------------------------------------------------------------------------
    void MultiSourceVertexBufferPool::destroyVertexBuffers( VertexBufferPackedVec &inOutVertexBuffers )
    {
        //First check that all of the buffers have been handed to us
        if( inOutVertexBuffers.size() != mVertexElementsBySource.size() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "We were expecting a list of " +
                         StringConverter::toString( mVertexElementsBySource.size() ) +
                         " buffers, user provided " + StringConverter::toString( inOutVertexBuffers.size() ),
                         "MultiSourceVertexBufferPool::destroyVertexBuffers" );
        }

        size_t multiSourceId = inOutVertexBuffers[0]->getMultiSourceId();

        VertexBufferPackedVec::const_iterator itor = inOutVertexBuffers.begin();
        VertexBufferPackedVec::const_iterator end  = inOutVertexBuffers.end();

        while( itor != end )
        {
            if( (*itor)->getMultiSourcePool() != this )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Vertex buffer doesn't belong to this pool!",
                             "MultiSourceVertexBufferPool::destroyVertexBuffers" );
            }

            if( multiSourceId != (*itor)->getMultiSourceId() )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "You must provide all the vertex buffers that "
                             "were returned by createVertexBuffers!",
                             "MultiSourceVertexBufferPool::destroyVertexBuffers" );
            }

            ++itor;
        }

        destroyVertexBuffersImpl( inOutVertexBuffers );

        itor = inOutVertexBuffers.begin();

        while( itor != end )
            delete *itor++;

        inOutVertexBuffers.clear();
    }
    //-----------------------------------------------------------------------------------
    size_t MultiSourceVertexBufferPool::getBytesOffsetToSource( uint8 sourceIdx ) const
    {
        return mSourceOffset[sourceIdx] * mBytesPerVertexPerSource[sourceIdx];
    }
}

