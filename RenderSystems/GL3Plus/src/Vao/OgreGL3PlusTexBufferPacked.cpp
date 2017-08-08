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

#include "Vao/OgreGL3PlusTexBufferPacked.h"
#include "Vao/OgreGL3PlusBufferInterface.h"

#include "OgreGL3PlusPixelFormat.h"

namespace Ogre
{
    GL3PlusTexBufferPacked::GL3PlusTexBufferPacked(
                size_t internalBufStartBytes, size_t numElements, uint32 bytesPerElement,
                uint32 numElementsPadding, BufferType bufferType, void *initialData, bool keepAsShadow,
                VaoManager *vaoManager, GL3PlusBufferInterface *bufferInterface, PixelFormat pf ) :
        TexBufferPacked( internalBufStartBytes, numElements, bytesPerElement, numElementsPadding,
                         bufferType, initialData, keepAsShadow, vaoManager, bufferInterface, pf ),
        mTexName( 0 )
    {
        OCGE( glGenTextures( 1, &mTexName ) );
        OCGE( glBindTexture( GL_TEXTURE_BUFFER, mTexName ) );

        mInternalFormat = GL3PlusPixelUtil::getGLImageInternalFormat( pf );
    }
    //-----------------------------------------------------------------------------------
    GL3PlusTexBufferPacked::~GL3PlusTexBufferPacked()
    {
        OCGE( glDeleteTextures( 1, &mTexName ) );
    }
    //-----------------------------------------------------------------------------------
    inline void GL3PlusTexBufferPacked::bindBuffer( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( dynamic_cast<GL3PlusBufferInterface*>( mBufferInterface ) );
        assert( offset < (mNumElements * mBytesPerElement - 1) );
        assert( (offset + sizeBytes) <= mNumElements * mBytesPerElement );

        sizeBytes = !sizeBytes ? (mNumElements * mBytesPerElement - offset) : sizeBytes;

        GL3PlusBufferInterface *bufferInterface = static_cast<GL3PlusBufferInterface*>(
                                                                      mBufferInterface );

        OCGE( glActiveTexture( GL_TEXTURE0 + slot ) );
        OCGE( glBindTexture( GL_TEXTURE_BUFFER, mTexName ) );
        OCGE(
          glTexBufferRange( GL_TEXTURE_BUFFER, mInternalFormat, bufferInterface->getVboName(),
                            mFinalBufferStart * mBytesPerElement + offset, sizeBytes ) );

        //TODO: Get rid of this nonsense of restoring the active texture.
        //RenderSystem is always restores to 0 after using,
        //plus activateGLTextureUnit won't see our changes otherwise.
        OCGE( glActiveTexture( GL_TEXTURE0 ) );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferPacked::bindBufferVS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes);
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferPacked::bindBufferPS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferPacked::bindBufferGS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferPacked::bindBufferHS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferPacked::bindBufferDS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferPacked::bindBufferCS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
}
