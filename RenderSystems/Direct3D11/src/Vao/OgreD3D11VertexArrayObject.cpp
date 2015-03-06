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

#include "Vao/OgreD3D11VertexArrayObject.h"
#include "Vao/OgreD3D11BufferInterface.h"

#include "Vao/OgreIndexBufferPacked.h"

namespace Ogre
{
    D3D11VertexArrayObjectShared::D3D11VertexArrayObjectShared(
            const VertexBufferPackedVec &vertexBuffers,
            IndexBufferPacked *indexBuffer,
            v1::RenderOperation::OperationType opType,
            VertexBufferPacked *drawId )
    {
        memset( mStrides, 0, sizeof( mStrides ) );
        memset( mOffsets, 0, sizeof( mOffsets ) );

        assert( vertexBuffers.size() + 1 < 16 );
        const size_t numVertexBuffers = vertexBuffers.size();
        for( size_t i=0; i<numVertexBuffers; ++i )
        {
            VertexBufferPacked *vertexBuffer = vertexBuffers[i];
            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        vertexBuffer->getBufferInterface() );
            mVertexBuffers[i]   = bufferInterface->getVboName();
            mStrides[i]         = vertexBuffer->getBytesPerElement();
        }

        {
            VertexBufferPacked *vertexBuffer = drawId;
            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        vertexBuffer->getBufferInterface() );
            mVertexBuffers[numVertexBuffers]    = bufferInterface->getVboName();
            mStrides[numVertexBuffers]          = vertexBuffer->getBytesPerElement();
        }

        if( indexBuffer )
        {
            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        indexBuffer->getBufferInterface() );

            mIndexBuffer = bufferInterface->getVboName();
            mIndexFormat =
                    indexBuffer->getIndexType() == IndexBufferPacked::IT_16BIT ?
                                    DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        }
        else
        {
            mIndexBuffer = 0;
            mIndexFormat = DXGI_FORMAT_UNKNOWN;
        }

        switch( opType )
        {
        case v1::RenderOperation::OT_POINT_LIST:
            mPrimType[0] = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            mPrimType[1] = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            mPrimType[2] = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case v1::RenderOperation::OT_LINE_LIST:
            mPrimType[0] = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            mPrimType[1] = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            mPrimType[2] = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            break;
        case v1::RenderOperation::OT_LINE_STRIP:
            mPrimType[0] = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            mPrimType[1] = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            mPrimType[2] = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            break;
        default:
        case v1::RenderOperation::OT_TRIANGLE_LIST:
            mPrimType[0] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            mPrimType[1] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            mPrimType[2] = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            break;
        case v1::RenderOperation::OT_TRIANGLE_STRIP:
            mPrimType[0] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            mPrimType[1] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            mPrimType[2] = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            break;
        case v1::RenderOperation::OT_TRIANGLE_FAN:
            mPrimType[0] = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            mPrimType[1] = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            mPrimType[2] = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Error - DX11 render - no support for triangle fan (OT_TRIANGLE_FAN)",
                         "D3D11VaoManager::createVertexArrayObjectImpl" );
            break;
        }
    }
    //-----------------------------------------------------------------------------------
}
