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

namespace Ogre
{
    GL3PlusBufferInterface::GL3PlusBufferInterface( GLenum target, GLuint vboName ) :
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
    void GL3PlusBufferInterface::upload( void *data, size_t elementStart, size_t elementCount )
    {
        if( mBuffer->mBufferType == BT_DYNAMIC )
        {
            void *dstData = this->map( elementStart, elementCount, MS_MAPPED );
            memcpy( dstData, data, elementCount * mBuffer->mBytesPerElement );
            this->unmap( UO_UNMAP_ALL );
        }
        else
        {
            size_t bytesPerElement = mBuffer->mBytesPerElement;

            //Get a staging buffer (will bind to GL_COPY_READ_BUFFER)
            GL3PlusVaoManager *vaoManager = static_cast<GL3PlusVaoManager*>(mBuffer->mVaoManager);
            vaoManager->getStagingBuffer( elementCount * bytesPerElement, true );
            OCGLE( glBindBuffer( GL_COPY_WRITE_BUFFER, mVboName ) );

            //Map and memcpy the data (CPU -> GPU)
            void *dstData = 0;
            OCGLE(
                dstData = glMapBufferRange( GL_COPY_READ_BUFFER,
                                            0,
                                            elementCount * bytesPerElement,
                                            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                                            GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT ) );

            memcpy( dstData, data, elementCount * bytesPerElement );

            OCGLE( glFlushMappedBufferRange( GL_COPY_READ_BUFFER,
                                             0,
                                             elementCount * bytesPerElement ) );
            OCGLE( glUnmapBuffer( GL_COPY_READ_BUFFER ) );

            //Copy data from Staging to real buffer (GPU -> GPU)
            OCGLE( glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
                                        0,
                                        elementStart * bytesPerElement,
                                        elementCount * bytesPerElement ) );
        }
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
