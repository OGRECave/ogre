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

#ifndef _Ogre_D3D11VertexArrayObject_H_
#define _Ogre_D3D11VertexArrayObject_H_

#include "OgreD3D11Prerequisites.h"

#include "Vao/OgreVertexArrayObject.h"

namespace Ogre
{
    struct _OgreD3D11Export D3D11VertexArrayObjectShared
    {
        ID3D11Buffer    *mVertexBuffers[16];
        UINT            mStrides[16];
        UINT            mOffsets[16];
        ID3D11Buffer    *mIndexBuffer;
        DXGI_FORMAT     mIndexFormat;

        D3D11VertexArrayObjectShared( const VertexBufferPackedVec &vertexBuffers,
                                      IndexBufferPacked *indexBuffer,
                                      OperationType opType,
                                      VertexBufferPacked *drawId );
    };

    struct _OgreD3D11Export D3D11VertexArrayObject : public VertexArrayObject
    {
        D3D11VertexArrayObjectShared    *mSharedData;

        D3D11VertexArrayObject( uint32 vaoName, uint32 renderQueueId, uint16 inputLayoutId,
                                const VertexBufferPackedVec &vertexBuffers,
                                IndexBufferPacked *indexBuffer,
                                OperationType opType,
                                D3D11VertexArrayObjectShared *sharedData ) :
            VertexArrayObject( vaoName, renderQueueId, inputLayoutId,
                               vertexBuffers, indexBuffer, opType ),
            mSharedData( sharedData )
        {
        }

        void _updateImmutableResource( uint32 vaoName, uint32 renderQueueId,
                                       D3D11VertexArrayObjectShared *sharedData );
    };
}

#endif
