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

    void CommandBuffer::execute_setVao( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbVao *cmd = static_cast<const CbVao*>( _cmd );
        mRenderSystem->_setVertexArrayObject( cmd->vao );
    }

    CbDrawCall::CbDrawCall( uint16 cmdType, VertexArrayObject *_vao,
                            void *_indirectBufferOffset ) :
        CbBase( cmdType ),
        vao( _vao ),
        numDraws( 0 ),
        indirectBufferOffset( _indirectBufferOffset )
    {
    }

    CbDrawCallIndexed::CbDrawCallIndexed( bool supportsIndirectBuffers, VertexArrayObject *_vao,
                                          void *_indirectBufferOffset ) :
        CbDrawCall( CB_DRAW_CALL_INDEXED_EMULATED + supportsIndirectBuffers,
                    _vao, _indirectBufferOffset )
    {
    }
    void CommandBuffer::execute_drawCallIndexedEmulated( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallIndexed *cmd = static_cast<const CbDrawCallIndexed*>( _cmd );
        mRenderSystem->_renderEmulated( cmd );
    }

    void CommandBuffer::execute_drawCallIndexed( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallIndexed *cmd = static_cast<const CbDrawCallIndexed*>( _cmd );
        mRenderSystem->_render( cmd );
    }

    CbDrawCallStrip::CbDrawCallStrip( bool supportsIndirectBuffers, VertexArrayObject *_vao,
                                      void *_indirectBufferOffset ) :
        CbDrawCall( CB_DRAW_CALL_STRIP_EMULATED + supportsIndirectBuffers, _vao, _indirectBufferOffset )
    {
    }

    void CommandBuffer::execute_drawCallStripEmulated( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallStrip *cmd = static_cast<const CbDrawCallStrip*>( _cmd );
        mRenderSystem->_renderEmulated( cmd );
    }

    void CommandBuffer::execute_drawCallStrip( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbDrawCallStrip *cmd = static_cast<const CbDrawCallStrip*>( _cmd );
        mRenderSystem->_render( cmd );
    }
}
