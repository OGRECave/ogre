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

#include "Vao/OgreGL3PlusTexBufferEmulatedPacked.h"
#include "Vao/OgreGL3PlusBufferInterface.h"

#include "OgreGL3PlusPixelFormat.h"

namespace Ogre
{
    GL3PlusTexBufferEmulatedPacked::GL3PlusTexBufferEmulatedPacked(
            size_t internalBufStartBytes, size_t numElements, uint32 bytesPerElement,
            uint32 numElementsPadding, BufferType bufferType, void *initialData, bool keepAsShadow,
            VaoManager *vaoManager, GL3PlusBufferInterface *bufferInterface, PixelFormat pf ) :
        TexBufferPacked( internalBufStartBytes, numElements, bytesPerElement, numElementsPadding,
                         bufferType, initialData, keepAsShadow, vaoManager, bufferInterface, pf ),
        mTexName( 0 ),
        mOriginFormat( 0 ),
        mOriginDataType( 0 ),
        mMaxTexSize( 0 ),
        mInternalNumElemBytes( 0 ),
        mInternalNumElements( 0 )
    {
        OCGE( glGenTextures( 1, &mTexName ) );
        
        mInternalFormat = GL3PlusPixelUtil::getGLImageInternalFormat( pf );

        OCGE( glBindTexture( GL_TEXTURE_2D, mTexName ) );

        mMaxTexSize = 2048;

        mOriginFormat = GL3PlusPixelUtil::getGLOriginFormat( pf );
        mOriginDataType = GL3PlusPixelUtil::getGLOriginDataType( pf );

        mInternalNumElemBytes = PixelUtil::getNumElemBytes( pf );

        mInternalNumElements = numElements / mInternalNumElemBytes;

        size_t width = std::min( mMaxTexSize,  mInternalNumElements);

        size_t texHeight = (mInternalNumElements + mMaxTexSize - 1) / mMaxTexSize;

        OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 ) );
        OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 ) );
        OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) );
        OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) );
        OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
        OCGE( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );
        OCGE( glTexImage2D( GL_TEXTURE_2D, 0, mInternalFormat, width, texHeight,
                            0, mOriginFormat, mOriginDataType, NULL ) );
    }
    //-----------------------------------------------------------------------------------
    GL3PlusTexBufferEmulatedPacked::~GL3PlusTexBufferEmulatedPacked()
    {
        OCGE( glDeleteTextures( 1, &mTexName ) );
    }
    //-----------------------------------------------------------------------------------
    inline void GL3PlusTexBufferEmulatedPacked::bindBuffer( uint16 slot, size_t offset, size_t sizeBytes )
    {
        assert( dynamic_cast<GL3PlusBufferInterface*>( mBufferInterface ) );
        assert( offset < (mNumElements * mBytesPerElement - 1) );
        assert( (offset + sizeBytes) <= mNumElements * mBytesPerElement );

        sizeBytes = !sizeBytes ? (mNumElements * mBytesPerElement - offset) : sizeBytes;

        GL3PlusBufferInterface *bufferInterface =
                static_cast<GL3PlusBufferInterface*>( mBufferInterface );

        size_t numModifiedElements = sizeBytes / mInternalNumElemBytes;
        assert( sizeBytes % mInternalNumElemBytes == 0 );
        size_t texWidth = std::min( numModifiedElements, std::min( mMaxTexSize, mInternalNumElements ) );
        size_t texHeight = ( numModifiedElements + mMaxTexSize - 1 ) / mMaxTexSize;

        if( (mBytesPerElement & 4) != 4 )
        {
            // Standard alignment of 4 is not right for some formats.
            OCGE( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );
        }

        OCGE( glBindBuffer( GL_PIXEL_UNPACK_BUFFER, bufferInterface->getVboName() ) );
        OCGE( glActiveTexture( GL_TEXTURE0 + slot ) );
        OCGE( glBindTexture( GL_TEXTURE_2D, mTexName ) );
        OCGE( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight,
                               mOriginFormat, mOriginDataType, reinterpret_cast<void*>(
                                   mFinalBufferStart * mBytesPerElement + offset ) ) );

        // Restore alignment.
        if ((mBytesPerElement & 4) != 4)
        {
            OCGE( glPixelStorei( GL_UNPACK_ALIGNMENT, 4 ) );
        }

        //TODO: Get rid of this nonsense of restoring the active texture.
        //RenderSystem is always restores to 0 after using,
        //plus activateGLTextureUnit won't see our changes otherwise.
        OCGE( glActiveTexture( GL_TEXTURE0 ) );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferEmulatedPacked::bindBufferVS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes);
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferEmulatedPacked::bindBufferPS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferEmulatedPacked::bindBufferGS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferEmulatedPacked::bindBufferHS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferEmulatedPacked::bindBufferDS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusTexBufferEmulatedPacked::bindBufferCS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        bindBuffer( slot, offset, sizeBytes );
    }
    //-----------------------------------------------------------------------------------
}
