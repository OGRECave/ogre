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

#include "Vao/OgreGL3PlusDynamicBuffer.h"
#include "Vao/OgreGL3PlusVaoManager.h"
#include "Vao/OgreGL3PlusStagingBuffer.h"

namespace Ogre
{
    GL3PlusDynamicBuffer::GL3PlusDynamicBuffer( GLuint vboName, GLuint vboSize,
                                                GL3PlusVaoManager *vaoManager,
                                                BufferType persistentMethod ) :
        mVboName( vboName ),
        mVboSize( vboSize ),
        mMappedPtr( 0 ),
        mPersistentMethod( persistentMethod )
    {
        if( !vaoManager->supportsArbBufferStorage() )
            mPersistentMethod = BT_DYNAMIC_DEFAULT;
    }
    //-----------------------------------------------------------------------------------
    GL3PlusDynamicBuffer::~GL3PlusDynamicBuffer()
    {
    }
    //-----------------------------------------------------------------------------------
    size_t GL3PlusDynamicBuffer::addMappedRange(size_t start, size_t count )
    {
        size_t ticket;

        if( !mFreeRanges.empty() )
        {
            ticket = mFreeRanges.back();
            mMappedRanges[ticket] = MappedRange( start, count );
            mFreeRanges.pop_back();
        }
        else
        {
            ticket = mMappedRanges.size();
            mMappedRanges.push_back( MappedRange( start, count ) );
        }

        return ticket;
    }
    //-----------------------------------------------------------------------------------
    void* RESTRICT_ALIAS_RETURN GL3PlusDynamicBuffer::map( size_t start, size_t count,
                                                           size_t &outTicket )
    {
        assert( start <= mVboSize && start + count <= mVboSize );

        if( mMappedRanges.size() == mFreeRanges.size() )
        {
            GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

            if( mPersistentMethod >= BT_DYNAMIC_PERSISTENT )
            {
                flags |= GL_MAP_PERSISTENT_BIT;

                if( mPersistentMethod == BT_DYNAMIC_PERSISTENT_COHERENT )
                    flags |= GL_MAP_COHERENT_BIT;
            }
            else
            {
                flags |= GL_MAP_UNSYNCHRONIZED_BIT;
            }

            OCGE( mMappedPtr = glMapBufferRange( GL_COPY_WRITE_BUFFER, 0, mVboSize, flags ) );
        }

        outTicket = addMappedRange( start, count );

        return static_cast<uint8*>(mMappedPtr) + start;
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDynamicBuffer::flush( size_t ticket, size_t start, size_t count )
    {
        assert( start <= mMappedRanges[ticket].count && start + count <= mMappedRanges[ticket].count );

        OCGE( glFlushMappedBufferRange( GL_COPY_WRITE_BUFFER,
                                        mMappedRanges[ticket].start + start,
                                        count ) );
    }
    //-----------------------------------------------------------------------------------
    void GL3PlusDynamicBuffer::unmap( size_t ticket )
    {
        assert( ticket < mMappedRanges.size() && "Invalid unmap ticket!" );
        assert( mMappedRanges.size() != mFreeRanges.size() &&
                "Unmapping an already unmapped buffer! Did you call unmap with the same ticket twice?" );

        mFreeRanges.push_back( ticket );

        if( mMappedRanges.size() == mFreeRanges.size() )
        {
            OCGE( glUnmapBuffer( GL_COPY_WRITE_BUFFER ) );
            mMappedPtr = 0;
        }
    }
}
