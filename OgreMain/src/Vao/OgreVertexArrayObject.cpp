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
#include "Vao/OgreVertexArrayObject.h"
#include "Vao/OgreIndexBufferPacked.h"

//Needed by VertexArrayObject::clone
#include "Vao/OgreAsyncTicket.h"
#include "Vao/OgreVaoManager.h"

namespace Ogre
{
    VertexBufferPacked VertexArrayObject::msDummyVertexBuffer( 0, 0, 1, BT_DEFAULT, 0, false, 0,
                                                               0, VertexElement2Vec(), 0, 0, 0 );

    typedef vector<VertexBufferPacked*>::type VertexBufferPackedVec;

    VertexArrayObject::VertexArrayObject( uint32 vaoName, uint32 renderQueueId, uint8 inputLayoutId,
                                          const VertexBufferPackedVec &vertexBuffers,
                                          IndexBufferPacked *indexBuffer,
                                          OperationType operationType ) :
            mVaoName( vaoName ),
            mRenderQueueId( renderQueueId ),
            mInputLayoutId( inputLayoutId ),
            mPrimStart( 0 ),
            mPrimCount( 0 ),
            mVertexBuffers( vertexBuffers ),
            mIndexBuffer( indexBuffer ),
            mBaseVertexBuffer( 0 ),
            mOperationType( operationType )
    {
        if( mVertexBuffers.empty() )
            mBaseVertexBuffer = &msDummyVertexBuffer;
        else
            mBaseVertexBuffer = mVertexBuffers[0];

        if( mIndexBuffer )
            mPrimCount = mIndexBuffer->getNumElements();
        else if( !mVertexBuffers.empty() )
            mPrimCount = mVertexBuffers[0]->getNumElements();

        /*switch( mOperationType )
        {
        case OT_TRIANGLE_LIST:
            mFaceCount = (val / 3);
            break;
        case OT_TRIANGLE_STRIP:
        case OT_TRIANGLE_FAN:
            mFaceCount = (val - 2);
            break;
        default:
            break;
        }*/
    }
    //-----------------------------------------------------------------------------------
    void VertexArrayObject::setPrimitiveRange( uint32 primStart, uint32 primCount )
    {
        size_t limit;
        if( mIndexBuffer )
            limit = mIndexBuffer->getNumElements();
        else
        {
            if( !mVertexBuffers.empty() )
                limit = mVertexBuffers[0]->getNumElements();
            else
                limit = std::numeric_limits<uint32>::max();
        }

        if( primStart > limit )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "primStart must be less or equal than " +
                         StringConverter::toString( limit ),
                         "VertexArrayObject::setPrimitiveRange" );
        }

        if( primStart + primCount > limit )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "primStart + primCount must be less or equal than " +
                         StringConverter::toString( limit ),
                         "VertexArrayObject::setPrimitiveRange" );
        }

        mPrimStart = primStart;
        mPrimCount = primCount;
    }
    //-----------------------------------------------------------------------------------
    const VertexElement2* VertexArrayObject::findBySemantic( VertexElementSemantic semantic,
                                                             size_t &outIndex, size_t &outOffset ) const
    {
        const VertexElement2 *retVal = 0;

        VertexBufferPackedVec::const_iterator itBuffers = mVertexBuffers.begin();
        VertexBufferPackedVec::const_iterator enBuffers = mVertexBuffers.end();

        while( itBuffers != enBuffers && !retVal )
        {
            size_t accumOffset = 0;
            const VertexElement2Vec &elements =  (*itBuffers)->getVertexElements();

            VertexElement2Vec::const_iterator itElements = elements.begin();
            VertexElement2Vec::const_iterator enElements = elements.end();

            while( itElements != enElements && itElements->mSemantic != semantic )
            {
                accumOffset += v1::VertexElement::getTypeSize( itElements->mType );
                ++itElements;
            }

            if( itElements != enElements && itElements->mSemantic == semantic )
            {
                outIndex = itBuffers - mVertexBuffers.begin();
                outOffset = accumOffset;
                retVal = &(*itElements);
            }

            ++itBuffers;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    VertexElement2VecVec VertexArrayObject::getVertexDeclaration(void) const
    {
        VertexElement2VecVec retVal;
        retVal.reserve( mVertexBuffers.size() );
        VertexBufferPackedVec::const_iterator itBuffers = mVertexBuffers.begin();
        VertexBufferPackedVec::const_iterator enBuffers = mVertexBuffers.end();

        while( itBuffers != enBuffers )
        {
            retVal.push_back( (*itBuffers)->getVertexElements() );
            ++itBuffers;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    VertexElement2VecVec VertexArrayObject::getVertexDeclaration( const VertexBufferPackedVec &vertexBuffers )
    {
        VertexElement2VecVec retVal;
        retVal.reserve( vertexBuffers.size() );
        VertexBufferPackedVec::const_iterator itBuffers = vertexBuffers.begin();
        VertexBufferPackedVec::const_iterator enBuffers = vertexBuffers.end();

        while( itBuffers != enBuffers )
        {
            retVal.push_back( (*itBuffers)->getVertexElements() );
            ++itBuffers;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    VertexArrayObject* VertexArrayObject::clone( VaoManager *vaoManager,
                                                 SharedVertexBufferMap *sharedBuffers ) const
    {
        VertexBufferPackedVec newVertexBuffers;
        newVertexBuffers.reserve( mVertexBuffers.size() );

        VertexBufferPackedVec::const_iterator itBuffers = mVertexBuffers.begin();
        VertexBufferPackedVec::const_iterator enBuffers = mVertexBuffers.end();

        while( itBuffers != enBuffers )
        {
            if( sharedBuffers )
            {
                SharedVertexBufferMap::const_iterator itShared = sharedBuffers->find( *itBuffers );
                if( itShared != sharedBuffers->end() )
                {
                    //This buffer is shared. Reuse the one we created in a previous call.
                    newVertexBuffers.push_back( itShared->second );
                    ++itBuffers;
                    continue;
                }
            }

            AsyncTicketPtr asyncTicket = (*itBuffers)->readRequest( 0, (*itBuffers)->getNumElements() );

            void *dstData = OGRE_MALLOC_SIMD( (*itBuffers)->getTotalSizeBytes(), MEMCATEGORY_GEOMETRY );
            FreeOnDestructor dataPtrContainer( dstData );

            const void *srcData = asyncTicket->map();
            memcpy( dstData, srcData, (*itBuffers)->getTotalSizeBytes() );
            asyncTicket->unmap();

            const bool keepAsShadow = (*itBuffers)->getShadowCopy() != 0;
            VertexBufferPacked *newVertexBuffer = vaoManager->createVertexBuffer(
                                                    (*itBuffers)->getVertexElements(),
                                                    (*itBuffers)->getNumElements(),
                                                    (*itBuffers)->getBufferType(),
                                                    dstData, keepAsShadow );

            if( keepAsShadow ) //Don't free the pointer ourselves
                dataPtrContainer.ptr = 0;

            newVertexBuffers.push_back( newVertexBuffer );

            if( sharedBuffers )
                (*sharedBuffers)[*itBuffers] = newVertexBuffer;

            ++itBuffers;
        }

        IndexBufferPacked *newIndexBuffer = 0;
        if( mIndexBuffer )
        {
            AsyncTicketPtr asyncTicket = mIndexBuffer->readRequest( 0, mIndexBuffer->getNumElements() );

            void *dstData = OGRE_MALLOC_SIMD( mIndexBuffer->getTotalSizeBytes(), MEMCATEGORY_GEOMETRY );
            FreeOnDestructor dataPtrContainer( dstData );

            const void *srcData = asyncTicket->map();
            memcpy( dstData, srcData, (*itBuffers)->getTotalSizeBytes() );
            asyncTicket->unmap();

            const bool keepAsShadow = mIndexBuffer->getShadowCopy() != 0;
            newIndexBuffer = vaoManager->createIndexBuffer( mIndexBuffer->getIndexType(),
                                                            mIndexBuffer->getNumElements(),
                                                            mIndexBuffer->getBufferType(),
                                                            dstData, keepAsShadow );

            if( keepAsShadow ) //Don't free the pointer ourselves
                dataPtrContainer.ptr = 0;
        }

        VertexArrayObject *retVal = vaoManager->createVertexArrayObject( newVertexBuffers,
                                                                         newIndexBuffer,
                                                                         mOperationType );
        retVal->mPrimStart = mPrimStart;
        retVal->mPrimCount = mPrimCount;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void VertexArrayObject::readRequests( ReadRequestsArray &requests )
    {
        set<VertexBufferPacked*>::type seenBuffers;

        ReadRequestsArray::iterator itor = requests.begin();
        ReadRequestsArray::iterator end  = requests.end();

        while( itor != end )
        {
            size_t bufferIdx, offset;
            const VertexElement2 *vElement = findBySemantic( itor->semantic, bufferIdx, offset );
            if( !vElement )
            {
                OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Cannot find semantic in VertexArrayObject",
                             "VertexArrayObject::readRequests" );
            }

            VertexBufferPacked *vertexBuffer = mVertexBuffers[bufferIdx];

            itor->type = vElement->mType;
            itor->offset = offset;
            itor->vertexBuffer = vertexBuffer;

            if( seenBuffers.find( vertexBuffer ) == seenBuffers.end() )
            {
                itor->asyncTicket = vertexBuffer->readRequest( 0, vertexBuffer->getNumElements() );
                seenBuffers.insert( vertexBuffer );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void VertexArrayObject::mapAsyncTickets( ReadRequestsArray &tickets )
    {
        map<VertexBufferPacked const *, size_t>::type seenBuffers;

        ReadRequestsArray::iterator itor = tickets.begin();
        ReadRequestsArray::iterator end  = tickets.end();

        while( itor != end )
        {
            if( !itor->asyncTicket.isNull() )
            {
                itor->data = reinterpret_cast<const char*>( itor->asyncTicket->map() );
                itor->data += itor->offset;
                seenBuffers[itor->vertexBuffer] = itor - tickets.begin();
            }
            else
            {
                map<VertexBufferPacked const *, size_t>::type::const_iterator it = seenBuffers.find(
                                                                                itor->vertexBuffer );
                assert( it != seenBuffers.end() &&
                        "These tickets are invalid or already been unmapped" );

                itor->data = tickets[it->second].data - tickets[it->second].offset + itor->offset;
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void VertexArrayObject::unmapAsyncTickets( ReadRequestsArray &tickets )
    {
        ReadRequestsArray::iterator itor = tickets.begin();
        ReadRequestsArray::iterator end  = tickets.end();

        while( itor != end )
        {
            itor->data = 0;
            if( !itor->asyncTicket.isNull() )
            {
                itor->asyncTicket->unmap();
                itor->asyncTicket.setNull();
            }

            ++itor;
        }
    }
}
