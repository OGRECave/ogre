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
#ifndef _OgreCbDrawCall_H_
#define _OgreCbDrawCall_H_

#include "CommandBuffer/OgreCbCommon.h"

namespace Ogre
{
    struct CbVao : public CbBase
    {
        VertexArrayObject *vao;
        CbVao( VertexArrayObject *_vao );
    };

#pragma pack( push, 4 )
    struct CbSharedDraw
    {
        uint32 primCount;
        uint32 instanceCount;
        uint32 firstVertexIndex;
    };

    struct CbDrawStrip : CbSharedDraw
    {
        uint32 baseInstance;
    };

    struct CbDrawIndexed : CbSharedDraw
    {
        uint32 baseVertex;
        uint32 baseInstance;
    };
#pragma pack( pop )

    struct _OgreExport CbDrawCall : public CbBase
    {
        VertexArrayObject   *vao;
        uint32              numDraws;
        void                *indirectBufferOffset;
        CbDrawCall( uint16 cmdType, VertexArrayObject *_vao, void *_indirectBufferOffset );
    };

    struct _OgreExport CbDrawCallIndexed : public CbDrawCall
    {
        CbDrawCallIndexed( int baseInstanceAndIndirectBuffers, VertexArrayObject *_vao,
                           void *_indirectBufferOffset );
    };

    struct _OgreExport CbDrawCallStrip : public CbDrawCall
    {
        CbDrawCallStrip( int baseInstanceAndIndirectBuffers, VertexArrayObject *_vao,
                         void *_indirectBufferOffset );
    };

namespace v1
{
    struct CbStartV1LegacyRendering : public CbBase
    {
        CbStartV1LegacyRendering();
    };

    struct _OgreExport CbRenderOp : public CbBase
    {
        v1::VertexData  *vertexData;
        v1::IndexData   *indexData;
        uint8           operationType;
        //uint32          numberOfInstances;
        //uint8           renderToVertexBuffer; //Is this used???
        //uint8           useGlobalInstancingVertexBufferIsAvailable;

        CbRenderOp( const v1::RenderOperation &_renderOp );
    };

    struct _OgreExport CbDrawCall : public CbBase
    {
        //uint8           renderToVertexBuffer; //Is this used???
        //uint8           useGlobalInstancingVertexBufferIsAvailable;
        //void            *indirectBufferOffset;
        uint32          baseInstance;
        uint32          primCount;
        uint32          instanceCount;
        uint32          firstVertexIndex;
        CbDrawCall( uint16 cmdType );
    };

    struct _OgreExport CbDrawCallIndexed : public CbDrawCall
    {
        CbDrawCallIndexed( bool supportsBaseInstance );
    };

    struct _OgreExport CbDrawCallStrip : public CbDrawCall
    {
        CbDrawCallStrip( bool supportsBaseInstance );
    };
}
}

#endif
