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

namespace Ogre
{
    typedef vector<VertexBufferPacked*>::type VertexBufferPackedVec;

    VertexArrayObject::VertexArrayObject( uint32 vaoName, uint32 renderQueueId,
                                          const VertexBufferPackedVec &vertexBuffers,
                                          IndexBufferPacked *indexBuffer,
                                          v1::RenderOperation::OperationType operationType ) :
            mVaoName( vaoName ),
            mRenderQueueId( renderQueueId ),
            mPrimStart( 0 ),
            mPrimCount( 0 ),
            mVertexBuffers( vertexBuffers ),
            mIndexBuffer( indexBuffer ),
            mOperationType( operationType )
    {
        if( mIndexBuffer )
            mPrimCount = mIndexBuffer->getNumElements();
        else
            mPrimCount = mVertexBuffers[0]->getNumElements();

        /*switch( mOperationType )
        {
        case v1::RenderOperation::OT_TRIANGLE_LIST:
            mFaceCount = (val / 3);
            break;
        case v1::RenderOperation::OT_TRIANGLE_STRIP:
        case v1::RenderOperation::OT_TRIANGLE_FAN:
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
            limit = mVertexBuffers[0]->getNumElements();

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
}
