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

#include "Vao/OgreGL3PlusConstBufferPacked.h"
#include "Vao/OgreGL3PlusBufferInterface.h"

namespace Ogre
{
    GL3PlusConstBufferPacked::GL3PlusConstBufferPacked(
                size_t internalBufferStartBytes, size_t numElements, uint32 bytesPerElement,
                uint32 numElementsPadding, BufferType bufferType, void *initialData, bool keepAsShadow,
                VaoManager *vaoManager, BufferInterface *bufferInterface ) :
        ConstBufferPacked( internalBufferStartBytes, numElements, bytesPerElement, numElementsPadding,
                           bufferType, initialData, keepAsShadow, vaoManager, bufferInterface )
    {
    }
    //-----------------------------------------------------------------------------------
    GL3PlusConstBufferPacked::~GL3PlusConstBufferPacked()
    {
    }
    //-----------------------------------------------------------------------------------
    inline void GL3PlusConstBufferPacked::bindBuffer( uint16 slot )
    {
        assert( dynamic_cast<GL3PlusBufferInterface*>( mBufferInterface ) );

        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                                      mBufferInterface );

        OCGE(
          glBindBufferRange( GL_UNIFORM_BUFFER, slot, bufferInterface->getVboName(),
                             mFinalBufferStart * mBytesPerElement, mNumElements * mBytesPerElement ) );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusConstBufferPacked::bindBufferVS( uint16 slot )
    {
        bindBuffer( slot );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusConstBufferPacked::bindBufferPS( uint16 slot )
    {
        bindBuffer( slot );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusConstBufferPacked::bindBufferGS( uint16 slot )
    {
        bindBuffer( slot );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusConstBufferPacked::bindBufferHS( uint16 slot )
    {
        bindBuffer( slot );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusConstBufferPacked::bindBufferDS( uint16 slot )
    {
        bindBuffer( slot );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusConstBufferPacked::bindBufferCS( uint16 slot )
    {
        bindBuffer( slot );
    }
    //-----------------------------------------------------------------------------------
}
