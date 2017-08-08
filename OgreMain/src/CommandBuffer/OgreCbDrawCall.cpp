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

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbDrawCall.h"

#include "Vao/OgreVertexArrayObject.h"
#include "Vao/OgreIndirectBufferPacked.h"

#include "OgreRenderSystem.h"

namespace Ogre
{
    CbVao::CbVao( VertexArrayObject *_vao ) :
        CbBase( CB_SET_VAO ),
        vao( _vao )
    {
    }

    void CommandBuffer::execute_setVao( CommandBuffer *_this, const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbVao *cmd = static_cast<const CbVao*>( _cmd );
        _this->mRenderSystem->_setVertexArrayObject( cmd->vao );
    }

    CbDrawCall::CbDrawCall( uint16 cmdType, VertexArrayObject *_vao,
                            void *_indirectBufferOffset ) :
        CbBase( cmdType ),
        vao( _vao ),
        numDraws( 0 ),
        indirectBufferOffset( _indirectBufferOffset )
    {
    }

    CbDrawCallIndexed::CbDrawCallIndexed( int baseInstanceAndIndirectBuffers, VertexArrayObject *_vao,
                                          void *_indirectBufferOffset ) :
        CbDrawCall( CB_DRAW_CALL_INDEXED_EMULATED_NO_BASE_INSTANCE + baseInstanceAndIndirectBuffers,
                    _vao, _indirectBufferOffset )
    {
    }

    void CommandBuffer::execute_drawCallIndexedEmulatedNoBaseInstance(
            CommandBuffer *_this, const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallIndexed *cmd = static_cast<const CbDrawCallIndexed*>( _cmd );
        _this->mRenderSystem->_renderEmulatedNoBaseInstance( cmd );
    }

    void CommandBuffer::execute_drawCallIndexedEmulated( CommandBuffer *_this,
                                                         const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallIndexed *cmd = static_cast<const CbDrawCallIndexed*>( _cmd );
        _this->mRenderSystem->_renderEmulated( cmd );
    }

    void CommandBuffer::execute_drawCallIndexed( CommandBuffer *_this,
                                                 const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallIndexed *cmd = static_cast<const CbDrawCallIndexed*>( _cmd );
        _this->mRenderSystem->_render( cmd );
    }

    CbDrawCallStrip::CbDrawCallStrip( int baseInstanceAndIndirectBuffers, VertexArrayObject *_vao,
                                      void *_indirectBufferOffset ) :
        CbDrawCall( CB_DRAW_CALL_STRIP_EMULATED_NO_BASE_INSTANCE + baseInstanceAndIndirectBuffers,
                    _vao, _indirectBufferOffset )
    {
    }

    void CommandBuffer::execute_drawCallStripEmulatedNoBaseInstance( CommandBuffer *_this,
                                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallStrip *cmd = static_cast<const CbDrawCallStrip*>( _cmd );
        _this->mRenderSystem->_renderEmulatedNoBaseInstance( cmd );
    }

    void CommandBuffer::execute_drawCallStripEmulated( CommandBuffer *_this,
                                                       const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallStrip *cmd = static_cast<const CbDrawCallStrip*>( _cmd );
        _this->mRenderSystem->_renderEmulated( cmd );
    }

    void CommandBuffer::execute_drawCallStrip( CommandBuffer *_this,
                                               const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallStrip *cmd = static_cast<const CbDrawCallStrip*>( _cmd );
        _this->mRenderSystem->_render( cmd );
    }

    //-----------------------------------------------------------------------------------

    v1::CbStartV1LegacyRendering::CbStartV1LegacyRendering() :
        CbBase( CB_START_V1_LEGACY_RENDERING )
    {
    }

    void CommandBuffer::execute_startV1LegacyRendering( CommandBuffer *_this,
                                                        const CbBase * RESTRICT_ALIAS _cmd )
    {
        _this->mRenderSystem->_startLegacyV1Rendering();
    }

    v1::CbRenderOp::CbRenderOp( const v1::RenderOperation &renderOp ) :
        CbBase( CB_SET_V1_RENDER_OP )
    {
        vertexData  = renderOp.vertexData;
        indexData   = renderOp.useIndexes ? renderOp.indexData : 0;
        operationType = static_cast<uint8>( renderOp.operationType );
    }

    void CommandBuffer::execute_setV1RenderOp( CommandBuffer *_this, const CbBase * RESTRICT_ALIAS _cmd )
    {
        const v1::CbRenderOp *cmd = static_cast<const v1::CbRenderOp*>( _cmd );
        _this->mRenderSystem->_setRenderOperation( cmd );
    }

    v1::CbDrawCall::CbDrawCall( uint16 cmdType ) :
        CbBase( cmdType )
    {
    }

    v1::CbDrawCallIndexed::CbDrawCallIndexed( bool supportsBaseInstance ) :
        v1::CbDrawCall( CB_DRAW_V1_INDEXED_NO_BASE_INSTANCE + supportsBaseInstance )
    {
    }

    void CommandBuffer::execute_drawV1IndexedNoBaseInstance( CommandBuffer *_this,
                                                             const CbBase * RESTRICT_ALIAS _cmd )
    {
        const v1::CbDrawCallIndexed *cmd = static_cast<const v1::CbDrawCallIndexed*>( _cmd );
        _this->mRenderSystem->_renderNoBaseInstance( cmd );
    }

    void CommandBuffer::execute_drawV1Indexed( CommandBuffer *_this,
                                               const CbBase * RESTRICT_ALIAS _cmd )
    {
        const v1::CbDrawCallIndexed *cmd = static_cast<const v1::CbDrawCallIndexed*>( _cmd );
        _this->mRenderSystem->_render( cmd );
    }

    v1::CbDrawCallStrip::CbDrawCallStrip( bool supportsBaseInstance ) :
        v1::CbDrawCall( CB_DRAW_V1_STRIP_NO_BASE_INSTANCE + supportsBaseInstance )
    {
    }

    void CommandBuffer::execute_drawV1StripNoBaseInstance( CommandBuffer *_this,
                                                           const CbBase * RESTRICT_ALIAS _cmd )
    {
        const v1::CbDrawCallStrip *cmd = static_cast<const v1::CbDrawCallStrip*>( _cmd );
        _this->mRenderSystem->_renderNoBaseInstance( cmd );
    }

    void CommandBuffer::execute_drawV1Strip( CommandBuffer *_this,
                                             const CbBase * RESTRICT_ALIAS _cmd )
    {
        const v1::CbDrawCallStrip *cmd = static_cast<const v1::CbDrawCallStrip*>( _cmd );
        _this->mRenderSystem->_render( cmd );
    }
}
