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

#include "OgreRenderSystem.h"

namespace Ogre
{
    CbShaderBuffer::CbShaderBuffer( ShaderType shaderType, uint16 _slot,
                                    ConstBufferPacked *_bufferPacked,
                                    uint32 _bindOffset, uint32 _bindSizeBytes ) :
        CbBase( CB_SET_CONSTANT_BUFFER_VS + shaderType ),
        slot( _slot ),
        bufferPacked( _bufferPacked ),
        bindOffset( _bindOffset ),
        bindSizeBytes( _bindSizeBytes )
    {
        assert( shaderType != NumShaderTypes );
    }

    void CommandBuffer::execute_setConstantBufferVS( CommandBuffer *_this,
                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );
        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );
        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBufferVS( cmd->slot );
    }
    void CommandBuffer::execute_setConstantBufferPS( CommandBuffer *_this,
                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );
        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );
        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBufferPS( cmd->slot );
    }
    void CommandBuffer::execute_setConstantBufferGS( CommandBuffer *_this,
                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );
        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );
        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBufferGS( cmd->slot );
    }
    void CommandBuffer::execute_setConstantBufferHS( CommandBuffer *_this,
                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );
        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );
        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBufferHS( cmd->slot );
    }
    void CommandBuffer::execute_setConstantBufferDS( CommandBuffer *_this,
                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );
        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );
        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBufferDS( cmd->slot );
    }
    void CommandBuffer::execute_setConstantBufferCS( CommandBuffer *_this,
                                                     const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );
        assert( dynamic_cast<ConstBufferPacked*>( cmd->bufferPacked ) );
        static_cast<ConstBufferPacked*>( cmd->bufferPacked )->bindBufferCS( cmd->slot );
    }
    void CommandBuffer::execute_setConstantBufferInvalid( CommandBuffer *_this,
                                                          const CbBase * RESTRICT_ALIAS _cmd )
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                     "'NumShaderTypes' is not a valid parameter for this command!!!",
                     "CommandBuffer::execute_setConstantBufferInvalid" );
    }

    CbShaderBuffer::CbShaderBuffer( ShaderType shaderType, uint16 _slot,
                                    TexBufferPacked *_bufferPacked,
                                    uint32 _bindOffset, uint32 _bindSizeBytes ) :
        CbBase( CB_SET_TEXTURE_BUFFER_VS + shaderType ),
        slot( _slot ),
        bufferPacked( _bufferPacked ),
        bindOffset( _bindOffset ),
        bindSizeBytes( _bindSizeBytes )
    {
        assert( shaderType != NumShaderTypes );
        assert( !_bufferPacked || _bindOffset < _bufferPacked->getTotalSizeBytes() );
    }

    void CommandBuffer::execute_setTextureBufferVS( CommandBuffer *_this,
                                                    const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBufferVS( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
    void CommandBuffer::execute_setTextureBufferPS( CommandBuffer *_this,
                                                    const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBufferPS( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
    void CommandBuffer::execute_setTextureBufferGS( CommandBuffer *_this,
                                                    const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBufferGS( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
    void CommandBuffer::execute_setTextureBufferHS( CommandBuffer *_this,
                                                    const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBufferHS( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
    void CommandBuffer::execute_setTextureBufferDS( CommandBuffer *_this,
                                                    const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBufferDS( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
    void CommandBuffer::execute_setTextureBufferCS( CommandBuffer *_this,
                                                    const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbShaderBuffer *cmd = static_cast<const CbShaderBuffer*>( _cmd );

        assert( dynamic_cast<TexBufferPacked*>( cmd->bufferPacked ) );

        TexBufferPacked *texBuffer = static_cast<TexBufferPacked*>( cmd->bufferPacked );
        texBuffer->bindBufferCS( cmd->slot, cmd->bindOffset, cmd->bindSizeBytes );
    }
    void CommandBuffer::execute_setTextureBufferInvalid( CommandBuffer *_this,
                                                         const CbBase * RESTRICT_ALIAS _cmd )
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                     "'NumShaderTypes' is not a valid parameter for this command!!!",
                     "CommandBuffer::execute_setTextureBufferInvalid" );
    }

    CbIndirectBuffer::CbIndirectBuffer( IndirectBufferPacked *_indirectBuffer ) :
        CbBase( CB_SET_INDIRECT_BUFFER ),
        indirectBuffer( _indirectBuffer )
    {
    }

    void CommandBuffer::execute_setIndirectBuffer( CommandBuffer *_this,
                                                   const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbIndirectBuffer *cmd = static_cast<const CbIndirectBuffer*>( _cmd );
        _this->mRenderSystem->_setIndirectBuffer( cmd->indirectBuffer );
    }
}
