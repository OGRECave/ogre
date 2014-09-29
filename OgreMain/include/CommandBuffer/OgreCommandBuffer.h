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

#ifndef _OgreCommandBuffer_H_
#define _OgreCommandBuffer_H_

#include "CommandBuffer/OgreCbCommon.h"

namespace Ogre
{
    /** Command Buffer implementation.
        This interface is very sensitive to performance. For this reason:
    @par
        1. Each command has a fixed size (@see COMMAND_FIXED_SIZE)
           Whether a command uses all of the bytes is up to them.
    @par
        2. The command is stored as a flat contiguous array.
    @par
        3. Instead of implementing each command as a virtual function
           to the base class 'CbBase', we use C-style function pointers
           and manually maintain the table. This allows nifty optimizations
           (i.e. a vtable in a 64-bit arch needs 8 bytes!) and storing
           the commands as POD in the flat array (instead of having an
           array of pointers, and having to call virtual destructors).
    */
    class _OgreExport CommandBuffer
    {
        static const size_t COMMAND_FIXED_SIZE;

        RenderSystem    *mRenderSystem;

        FastArray<unsigned char>    mCommandBuffer;
    public:
        void execute_setVao( const CbBase * RESTRICT_ALIAS cmd );
        void execute_drawCallIndexedEmulated( const CbBase * RESTRICT_ALIAS cmd );
        void execute_drawCallIndexed( const CbBase * RESTRICT_ALIAS cmd );
        void execute_drawCallStripEmulated( const CbBase * RESTRICT_ALIAS cmd );
        void execute_drawCallStrip( const CbBase * RESTRICT_ALIAS cmd );
        void execute_setConstantBuffer( const CbBase * RESTRICT_ALIAS cmd );
        void execute_setTextureBuffer( const CbBase * RESTRICT_ALIAS cmd );
        void execute_setMacroblock( const CbBase * RESTRICT_ALIAS cmd );
        void execute_setBlendblock( const CbBase * RESTRICT_ALIAS cmd );
        void execute_setHlmsCache( const CbBase * RESTRICT_ALIAS cmd );
        void execute_setTexture( const CbBase * RESTRICT_ALIAS cmd );
        void execute_disableTextureUnitsFrom( const CbBase * RESTRICT_ALIAS cmd );
        void execute_invalidCommand( const CbBase * RESTRICT_ALIAS cmd );

        /// Executes all the commands in the command buffer. Clears the cmd buffer afterwards
        void execute(void);

        /// Creates/Records a command already casted to the typename.
        /// May invalidate returned pointers from previous calls.
        template <typename T>
        T* addCommand(void)
        {
            assert( sizeof(T) <= COMMAND_FIXED_SIZE );
            mCommandBuffer.insert( mCommandBuffer.end(), COMMAND_FIXED_SIZE );

            T *retVal = reinterpret_cast<T*>( mCommandBuffer.end() - COMMAND_FIXED_SIZE );
            retVal->commandType = MAX_COMMAND_BUFFER;

            return retVal;
        }

        /// Returns a pointer to the last created command
        CbBase* getLastCommand(void);

        /// Returns the offset of the given command, in case you want to retrieve
        /// the command later (addCommand may invalidate the pointer.
        /// @see getCommandFromOffset.
        size_t getCommandOffset( CbBase *cmd ) const;

        /// Retrieves the command from the given offset.
        /// Returns null if no such command at that offset (out of bounds).
        /// @see getCommandOffset.
        CbBase* getCommandFromOffset( size_t offset );
    };

    typedef void (CommandBuffer::*CommandBufferExecuteFunc)( const CbBase * RESTRICT_ALIAS cmd );
}

#endif
