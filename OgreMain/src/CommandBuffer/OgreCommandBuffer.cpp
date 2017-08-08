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

#include "OgreException.h"

namespace Ogre
{
    const size_t CommandBuffer::COMMAND_FIXED_SIZE = 32;

    CommandBuffer::CommandBufferExecuteFunc* CbExecutionTable[MAX_COMMAND_BUFFER+1] =
    {
        &CommandBuffer::execute_invalidCommand,
        &CommandBuffer::execute_setVao,
        &CommandBuffer::execute_setIndirectBuffer,
        &CommandBuffer::execute_drawCallIndexedEmulatedNoBaseInstance,
        &CommandBuffer::execute_drawCallIndexedEmulated,
        &CommandBuffer::execute_drawCallIndexed,
        &CommandBuffer::execute_drawCallStripEmulatedNoBaseInstance,
        &CommandBuffer::execute_drawCallStripEmulated,
        &CommandBuffer::execute_drawCallStrip,
        &CommandBuffer::execute_setConstantBufferVS,
        &CommandBuffer::execute_setConstantBufferPS,
        &CommandBuffer::execute_setConstantBufferGS,
        &CommandBuffer::execute_setConstantBufferHS,
        &CommandBuffer::execute_setConstantBufferDS,
        &CommandBuffer::execute_setConstantBufferCS,
        &CommandBuffer::execute_setConstantBufferInvalid,
        &CommandBuffer::execute_setTextureBufferVS,
        &CommandBuffer::execute_setTextureBufferPS,
        &CommandBuffer::execute_setTextureBufferGS,
        &CommandBuffer::execute_setTextureBufferHS,
        &CommandBuffer::execute_setTextureBufferDS,
        &CommandBuffer::execute_setTextureBufferCS,
        &CommandBuffer::execute_setTextureBufferInvalid,
        &CommandBuffer::execute_setPso,
        &CommandBuffer::execute_setTexture,
        &CommandBuffer::execute_disableTextureUnitsFrom,
        &CommandBuffer::execute_startV1LegacyRendering,
        &CommandBuffer::execute_setV1RenderOp,
        &CommandBuffer::execute_drawV1IndexedNoBaseInstance,
        &CommandBuffer::execute_drawV1Indexed,
        &CommandBuffer::execute_drawV1StripNoBaseInstance,
        &CommandBuffer::execute_drawV1Strip,
        &CommandBuffer::execute_lowLevelMaterial,
        &CommandBuffer::execute_invalidCommand
    };
    //-----------------------------------------------------------------------------------
    CommandBuffer::CommandBuffer() : mRenderSystem( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void CommandBuffer::setCurrentRenderSystem( RenderSystem *renderSystem )
    {
        mRenderSystem = renderSystem;
    }
    //-----------------------------------------------------------------------------------
    void CommandBuffer::execute_invalidCommand( CommandBuffer *_this, const CbBase *cmd )
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
                     "Invalid CommandBuffer command. "
                     "Either uninitialized memory, bug or buffer corruption.",
                     "CommandBuffer::execute_setInvalidCommand" );
    }
    //-----------------------------------------------------------------------------------
    void CommandBuffer::execute(void)
    {
        unsigned char const * RESTRICT_ALIAS cmdBase = mCommandBuffer.begin();

        size_t cmdBufferCount = mCommandBuffer.size() / CommandBuffer::COMMAND_FIXED_SIZE;
        for( size_t i=cmdBufferCount; i--; )
        {
            CbBase const * RESTRICT_ALIAS cmd = reinterpret_cast<const CbBase*RESTRICT_ALIAS>( cmdBase );
            (*CbExecutionTable[cmd->commandType])( this, cmd );
            cmdBase += CommandBuffer::COMMAND_FIXED_SIZE;
        }

        mCommandBuffer.clear();
    }
    //-----------------------------------------------------------------------------------
    CbBase* CommandBuffer::getLastCommand(void)
    {
        return reinterpret_cast<CbBase*>( mCommandBuffer.end() - COMMAND_FIXED_SIZE );
    }
    //-----------------------------------------------------------------------------------
    size_t CommandBuffer::getCommandOffset( CbBase *cmd ) const
    {
        return reinterpret_cast<FastArray<unsigned char>::const_iterator>(cmd) - mCommandBuffer.begin();
    }
    //-----------------------------------------------------------------------------------
    CbBase* CommandBuffer::getCommandFromOffset( size_t offset )
    {
        CbBase *retVal = 0;

        if( offset < mCommandBuffer.size() )
        {
            assert( !(offset % COMMAND_FIXED_SIZE) );
            retVal = reinterpret_cast<CbBase*>( mCommandBuffer.begin() + offset );
        }

        return retVal;
    }
}
