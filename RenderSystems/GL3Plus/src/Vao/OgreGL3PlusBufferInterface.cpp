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

#include "Vao/OgreGL3PlusBufferInterface.h"
#include "Vao/OgreGL3PlusVaoManager.h"
#include "Vao/OgreGL3PlusStagingBuffer.h"

namespace Ogre
{
    GL3PlusBufferInterface::GL3PlusBufferInterface( size_t vboPoolIdx, GLenum target, GLuint vboName ) :
        mVboPoolIdx( vboPoolIdx ),
        mTarget( target ),
        mVboName( vboName ),
        mMappedPtr( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    GL3PlusBufferInterface::~GL3PlusBufferInterface()
    {
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusBufferInterface::_firstUpload( void *data, size_t elementStart, size_t elementCount )
    {
        //In OpenGL; immutable buffers are a charade. They're mostly there to satisfy D3D11's needs.
        //However we emulate the behavior and trying to upload to an immutable buffer will result
        //in an exception or an assert, thus we temporarily change the type.
        BufferType originalBufferType = mBuffer->mBufferType;
        if( mBuffer->mBufferType == BT_IMMUTABLE )
            mBuffer->mBufferType = BT_DEFAULT;

        upload( data, elementStart, elementCount );

        mBuffer->mBufferType = originalBufferType;
    }
    //-----------------------------------------------------------------------------------
    void* GL3PlusBufferInterface::map( size_t elementStart, size_t elementCount,
                                       MappingState prevMappingState )
    {
        size_t bytesPerElement = mBuffer->mBytesPerElement;

        GL3PlusVaoManager *vaoManager = static_cast<GL3PlusVaoManager*>( mBuffer->mVaoManager );
        bool canPersistentMap = vaoManager->supportsArbBufferStorage();

        uint8 dynamicCurrentFrame = vaoManager->getDynamicBufferCurrentFrame();

        if( prevMappingState == MS_UNMAPPED || !canPersistentMap )
        {
            GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                                GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

            //Non-persistent buffers just map the small region they'll need.
            size_t offset = mBuffer->mInternalBufferStart + elementStart +
                            mBuffer->mNumElements * dynamicCurrentFrame;
            size_t length = elementCount;

            if( mBuffer->mMappingState >= MS_PERSISTENT_INCOHERENT && canPersistentMap )
            {
                //Persistent buffers map the *whole* assigned buffer,
                //we later care for the offsets and lengths
                offset = mBuffer->mInternalBufferStart;
                length = mBuffer->mNumElements * vaoManager->getDynamicBufferMultiplier();

                flags |= GL_MAP_PERSISTENT_BIT;

                if( mBuffer->mMappingState == MS_PERSISTENT_COHERENT )
                    flags |= GL_MAP_COHERENT_BIT;
            }

            mBuffer->mMappingStart = offset;
            mBuffer->mMappingCount = length;

            glBindBuffer( mTarget, mVboName );
            OCGLE(
                mMappedPtr = glMapBufferRange( mTarget,
                                               offset * bytesPerElement,
                                               length * bytesPerElement,
                                               flags ) );
        }

        mBuffer->mLastMappingStart = elementStart + mBuffer->mInternalBufferStart +
                                        mBuffer->mNumElements * dynamicCurrentFrame;
        mBuffer->mLastMappingCount = elementCount;

        char *retVal = (char*)mMappedPtr;

        if( mBuffer->mMappingState >= MS_PERSISTENT_INCOHERENT && canPersistentMap )
            retVal += (elementStart + mBuffer->mNumElements * dynamicCurrentFrame) * bytesPerElement;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusBufferInterface::unmap( UnmapOptions unmapOption )
    {
        bool canPersistentMap = static_cast<GL3PlusVaoManager*>( mBuffer->mVaoManager )->
                                                                supportsArbBufferStorage();

        if( mBuffer->mMappingState <= MS_PERSISTENT_INCOHERENT ||
            unmapOption == UO_UNMAP_ALL || !canPersistentMap )
        {
            OCGLE( glBindBuffer( mTarget, mVboName ) );
            OCGLE( glFlushMappedBufferRange( mTarget,
                                             (mBuffer->mLastMappingStart - mBuffer->mMappingStart) *
                                                                            mBuffer->mBytesPerElement,
                                             mBuffer->mLastMappingCount * mBuffer->mBytesPerElement ) );

            if( unmapOption == UO_UNMAP_ALL || !canPersistentMap || mBuffer->mMappingState == MS_MAPPED )
            {
                OCGLE( glUnmapBuffer( mTarget ) );
                mMappedPtr = 0;
            }
        }
    }
}
