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
#include "OgreVertexShadowMapHelper.h"

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreAsyncTicket.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    void VertexShadowMapHelper::useSameVaos( const VertexArrayObjectArray &inVao,
                                             VertexArrayObjectArray &outVao )
    {
        assert( outVao.empty() );

        outVao.reserve( inVao.size() );

        VertexArrayObjectArray::const_iterator itor = inVao.begin();
        VertexArrayObjectArray::const_iterator end  = inVao.end();

        while( itor != end )
        {
            outVao.push_back( *itor );
            ++itor;
        }
    }
    //---------------------------------------------------------------------
    void VertexShadowMapHelper::optimizeForShadowMapping( VaoManager *vaoManager,
                                                          const VertexArrayObjectArray &inVao,
                                                          VertexArrayObjectArray &outVao )
    {
        assert( outVao.empty() );
        outVao.reserve( inVao.size() );

        VertexArrayObjectArray::const_iterator itor = inVao.begin();
        VertexArrayObjectArray::const_iterator end  = inVao.end();

        vector< FastArray<uint32> >::type vertexConversionLuts; //One per Vao
        vertexConversionLuts.resize( inVao.size() );

        while( itor != end )
        {
            VertexArrayObject *vao = *itor;
            const size_t currentVaoIdx = itor - inVao.begin();

            VertexBufferPacked *shadowVertexBuffer = 0;
            const VertexBufferPackedVec &origVertexBuffers = vao->getVertexBuffers();

            size_t bufferIdx[3] = { -1, -1, -1 };
            VertexElement2 const *origElements[3];
            origElements[0] = vao->findBySemantic( VES_POSITION, bufferIdx[0] );

            if( !origElements[0] )
            {
                //This Vao is invalid for shadow mapping (doesn't
                //have a VES_POSITION semantic). Abort.
                useSameVaos( inVao, outVao );
                return;
            }

            size_t sharedVaoIdx = 0, sharedVertexBufferIdx = 0;
            findFirstAppearance( inVao, origVertexBuffers[bufferIdx[0]],
                                 sharedVaoIdx, sharedVertexBufferIdx );

            if( sharedVaoIdx != currentVaoIdx && bufferIdx[0] != sharedVaoIdx )
            {
                //Shared with a vertex buffer we've already created in a previous iteration.
                shadowVertexBuffer = outVao[sharedVaoIdx]->getVertexBuffers()[sharedVertexBufferIdx];
            }
            else
            {
                //Not shared. Create a new one by converting the original.
                origElements[1] = vao->findBySemantic( VES_BLEND_INDICES, bufferIdx[1] );
                origElements[2] = vao->findBySemantic( VES_BLEND_WEIGHTS, bufferIdx[2] );

                AsyncTicketPtr tickets[3];
                uint8 const * srcData[3];
                size_t srcOffset[3];
                size_t srcBytesPerVertex[3];

                memset( srcData, 0, sizeof( srcData ) );
                memset( srcOffset, 0, sizeof( srcOffset ) );
                memset( srcBytesPerVertex, 0, sizeof( srcBytesPerVertex ) );

                //Pack all the requests together
                for( size_t i=0; i<3; ++i )
                {
                    bool sameBuffer = false;
                    for( size_t j=0; j<i && !sameBuffer; ++j )
                        sameBuffer = bufferIdx[j] == bufferIdx[i];

                    if( !sameBuffer && origElements[i] )
                    {
                        tickets[i] = origVertexBuffers[bufferIdx[i]]->readRequest(
                                        0, origVertexBuffers[bufferIdx[i]]->getNumElements() );
                    }
                }

                //Pack all the maps together
                for( size_t i=0; i<3; ++i )
                {
                    if( tickets[i].isNull() )
                    {
                        for( size_t j=0; j<i; ++j )
                        {
                            if( bufferIdx[j] == bufferIdx[i] )
                            {
                                srcData[j] = srcData[i];
                                break;
                            }
                        }
                    }
                    else
                    {
                        srcData[i] = reinterpret_cast<const uint8*>( tickets[i]->map() );
                    }
                }

                for( size_t i=0; i<3; ++i )
                {
                    if( origElements[i] )
                    {
                        const VertexElement2Vec &vertexElements =
                                origVertexBuffers[bufferIdx[i]]->getVertexElements();
                        VertexElement2Vec::const_iterator itElements = vertexElements.begin();
                        VertexElement2Vec::const_iterator enElements = vertexElements.end();

                        while( itElements != enElements )
                        {
                            if( itElements->mSemantic == origElements[i]->mSemantic )
                                srcOffset[i] = srcBytesPerVertex[i];

                            srcBytesPerVertex[i] += v1::VertexElement::getTypeSize( itElements->mType );

                            ++itElements;
                        }
                    }
                }

                VertexElement2Vec vertexElements;
                for( size_t i=0; i<3; ++i )
                {
                    if( origElements[i] )
                        vertexElements.push_back( *origElements[i] );
                }

                uint32 numVertices = origVertexBuffers[0]->getNumElements(); //numVertices may shrink
                const uint32 bytesPerVertex = VaoManager::calculateVertexSize( vertexElements );

                uint8 *finalVertexData = reinterpret_cast<uint8*>(
                            OGRE_MALLOC_SIMD( numVertices * bytesPerVertex, MEMCATEGORY_GEOMETRY ) );

                const size_t newVertexCount = shrinkVertexBuffer( finalVertexData, origElements,
                                                                  vertexConversionLuts[currentVaoIdx],
                                                                  vao->getIndexBuffer() != 0, srcData,
                                                                  srcOffset, srcBytesPerVertex,
                                                                  numVertices );

                for( size_t i=0; i<3; ++i )
                {
                    if( !tickets[i].isNull() )
                        tickets[i]->unmap();
                }

                try
                {
                    const BufferType origBufferType = origVertexBuffers[bufferIdx[0]]->getBufferType();
                    const bool keepAsShadow = origBufferType >= BT_DYNAMIC_DEFAULT ? true : false;

                    shadowVertexBuffer = vaoManager->createVertexBuffer( vertexElements, newVertexCount,
                                                                         origBufferType, finalVertexData,
                                                                         keepAsShadow );
                }
                catch( Exception &e )
                {
                    //During exceptions we become responsible for freeing it.
                    OGRE_FREE_SIMD( finalVertexData, MEMCATEGORY_GEOMETRY );
                    finalVertexData = 0;
                    throw e;
                }
            }

            IndexBufferPacked *indexBuffer = 0;
            IndexBufferPacked *origIndexBuffer = vao->getIndexBuffer();

            if( origIndexBuffer )
            {
                const FastArray<uint32> &vertexConvLut = vertexConversionLuts[currentVaoIdx];

                AsyncTicketPtr ticket =
                        origIndexBuffer->readRequest( 0, origIndexBuffer->getNumElements() );
                const void *indexData = ticket->map();

                void *shadowIndexData = OGRE_MALLOC_SIMD( origIndexBuffer->getTotalSizeBytes(),
                                                          MEMCATEGORY_GEOMETRY );

                if( origIndexBuffer->getIndexType() == IndexBufferPacked::IT_16BIT )
                {
                    uint16 const *srcIndexData = reinterpret_cast<const uint16*>( indexData );
                    uint16 *dstIndexData = reinterpret_cast<uint16*>( shadowIndexData );

                    for( size_t i=0; i<origIndexBuffer->getNumElements(); ++i )
                        *dstIndexData++ = static_cast<uint16>( vertexConvLut[*srcIndexData++] );
                }
                else
                {
                    uint32 const *srcIndexData = reinterpret_cast<const uint32*>( indexData );
                    uint32 *dstIndexData = reinterpret_cast<uint32*>( shadowIndexData );

                    for( size_t i=0; i<origIndexBuffer->getNumElements(); ++i )
                        *dstIndexData++ = vertexConvLut[*srcIndexData++];
                }

                try
                {
                    const BufferType origBufferType = origIndexBuffer->getBufferType();
                    const bool keepAsShadow = origBufferType >= BT_DYNAMIC_DEFAULT ? true : false;

                    indexBuffer = vaoManager->createIndexBuffer( origIndexBuffer->getIndexType(),
                                                                 origIndexBuffer->getNumElements(),
                                                                 origBufferType,
                                                                 shadowIndexData, keepAsShadow );
                }
                catch( Exception &e )
                {
                    //During exceptions we become responsible for freeing it.
                    OGRE_FREE_SIMD( shadowIndexData, MEMCATEGORY_GEOMETRY );
                    shadowIndexData = 0;
                    throw e;
                }

                ticket->unmap();
            }

            VertexBufferPackedVec vertexBuffers( 1, shadowVertexBuffer );
            VertexArrayObject *shadowVao = vaoManager->createVertexArrayObject( vertexBuffers,
                                                                                indexBuffer,
                                                                                vao->getOperationType() );

            outVao.push_back( shadowVao );

            ++itor;
        }
    }
    //---------------------------------------------------------------------
    uint32 VertexShadowMapHelper::shrinkVertexBuffer( uint8 *dstData,
                                                      const VertexElement2 *vertexElements[],
                                                      FastArray<uint32> &vertexConversionLutArg,
                                                      bool hasIndexBuffer,
                                                      const uint8 * srcData[],
                                                      const size_t srcOffset[],
                                                      const size_t srcBytesPerVertex[],
                                                      uint32 numVertices )
    {
        size_t elementOffset[3+1];
        memset( elementOffset, 0, sizeof(elementOffset) );

        size_t bytesPerVertex = 0;

        for( size_t i=0; i<3; ++i )
        {
            if( vertexElements[i] )
                elementOffset[i+1] = v1::VertexElement::getTypeSize( vertexElements[i]->mType );
            bytesPerVertex += elementOffset[i+1];
        }

        for( uint32 i=0; i<numVertices; ++i )
        {
            //Copy while compacting all data
            for( size_t j=0; j<3; ++j )
            {
                memcpy( dstData + elementOffset[j]  + i * bytesPerVertex,
                        srcData[j] + srcOffset[j]   + i * srcBytesPerVertex[j],
                        elementOffset[j+1] );
            }
        }

        //Mark duplicated vertices as such.
        //Swap the internal pointer to a local version of the
        //array to allow compiler optimizations (otherwise the
        //compiler can't know if vertexConversionLutArg.mSize
        //and co. may change after every non-trivial call)
        FastArray<uint32> vertexConversionLut;
        vertexConversionLut.swap( vertexConversionLutArg );

        vertexConversionLut.resize( numVertices );
        for( uint32 i=0; i<numVertices; ++i )
            vertexConversionLut[i] = i;

        for( uint32 i=0; i<numVertices; ++i )
        {
            for( uint32 j=i+1; j<numVertices; ++j )
            {
                if( vertexConversionLut[j] == j )
                {
                    if( memcmp( dstData + i * bytesPerVertex,
                                dstData + j * bytesPerVertex,
                                bytesPerVertex ) == 0 )
                    {
                        vertexConversionLut[j] = i;
                    }
                }
            }
        }

        //We need vertexConversionLut to be filled in case Vao[1][0] doesn't use index buffers
        //but Vao[1][1] does use index buffers. Also knowing which vertices are duplicated
        //won't save memory, but at least will improve the post vertex cache usage.
        if( !hasIndexBuffer )
            return numVertices;

        uint32 newNumVertices = numVertices;

        //Remove the duplicated vertices, iterating in reverse order for lower algorithmic complexity.
        for( uint32 i=numVertices; --i; )
        {
            if( vertexConversionLut[i] != i )
            {
                --newNumVertices;
                memmove( dstData + i * bytesPerVertex,
                         dstData + (i+1) * bytesPerVertex,
                         (newNumVertices - i) * bytesPerVertex );
            }
        }

        //vertexConversionLut.resize( numVertices );
        //The table is outdated because some vertices have shifted. Example:
        //Before:
        //Vertices 0 & 5 were unique, 8 vertices:
        //  0000 5555
        //But now vertex 5 has become vertex 1. We need the table to say:
        //  0000 1111
        uint32 numUniqueVerts = 0;
        for( uint32 i=0; i<numVertices; ++i )
        {
            if( vertexConversionLut[i] == i )
                vertexConversionLut[i] = numUniqueVerts++;
            else
                vertexConversionLut[i] = vertexConversionLut[vertexConversionLut[i]];
        }

        vertexConversionLut.swap( vertexConversionLutArg );

        assert( newNumVertices == numUniqueVerts );

        return newNumVertices;
    }
    //---------------------------------------------------------------------
    bool VertexShadowMapHelper::findFirstAppearance( const VertexArrayObjectArray &vao,
                                                     const VertexBufferPacked *vertexBuffer,
                                                     size_t &outVaoIdx,
                                                     size_t &outVertexBufferIdx )
    {
        bool bFound = false;

        VertexArrayObjectArray::const_iterator itor = vao.begin();
        VertexArrayObjectArray::const_iterator end  = vao.end();

        while( itor != end && !bFound )
        {
            const VertexBufferPackedVec &vertexBuffers = (*itor)->getVertexBuffers();
            VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
            VertexBufferPackedVec::const_iterator enBuffers = vertexBuffers.end();

            while( itBuffers != enBuffers && !bFound )
            {
                if( *itBuffers == vertexBuffer )
                {
                    outVaoIdx           = itor - vao.begin();
                    outVertexBufferIdx  = itBuffers - vertexBuffers.begin();
                    bFound = true;
                }

                ++itBuffers;
            }

            ++itor;
        }

        return bFound;
    }
}
