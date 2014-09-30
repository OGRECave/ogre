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
#include "CommandBuffer/OgreCbShaderBuffer.h"

#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"

namespace Ogre
{
    CbShaderBuffer::CbShaderBuffer( uint16 _slot, ConstBufferPacked *_bufferPacked,
                                    uint32 _bindOffset, uint32 _bindSizeBytes ) :
        CbBase( CB_SET_CONSTANT_BUFFER ),
        slot( _slot ),
        bufferPacked( _bufferPacked ),
        bindOffset( _bindOffset ),
        bindSizeBytes( _bindSizeBytes )
    {
    }

    void CommandBuffer::execute_setConstantBuffer( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );

        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBuffer( cmd->slot );
    }

    CbShaderBuffer::CbShaderBuffer( uint16 _slot, TexBufferPacked *_bufferPacked,
                                    uint32 _bindOffset, uint32 _bindSizeBytes ) :
        CbBase( CB_SET_TEXTURE_BUFFER ),
        slot( _slot ),
        bufferPacked( _bufferPacked ),
        bindOffset( _bindOffset ),
        bindSizeBytes( _bindSizeBytes )
    {
    }

    void CommandBuffer::execute_setTextureBuffer( const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBuffer( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
}
