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

#include "Vao/OgreBufferPacked.h"
#include "Vao/OgreBufferInterface.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre
{
    BufferPacked::BufferPacked( size_t internalBufferStart, size_t numElements, uint32 bytesPerElement,
                                BufferType bufferType, void *initialData, bool keepAsShadow,
                                VaoManager *vaoManager, BufferInterface *bufferInterface ) :
        mInternalBufferStart( internalBufferStart ),
        mNumElements( numElements ),
        mBytesPerElement( bytesPerElement ),
        mBufferType( bufferType ),
        mVaoManager( vaoManager ),
        mMappingState( MS_UNMAPPED ),
        mMappingStart( 0 ),
        mMappingCount( 0 ),
        mBufferInterface( bufferInterface ),
        mShadowCopy( 0 )
    {
        if( !initialData && mBufferType == BT_IMMUTABLE )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Immutable buffers MUST provide initial data!",
                         "BufferPacked::BufferPacked" );
        }

        if( keepAsShadow )
        {
            if( mBufferType == BT_DYNAMIC )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Dynamic buffers can't have a shadow copy!",
                             "BufferPacked::BufferPacked" );
            }

            mShadowCopy = initialData;
        }
    }
    //-----------------------------------------------------------------------------------
    BufferPacked::~BufferPacked()
    {
        if( mShadowCopy )
        {
            OGRE_FREE_SIMD( mShadowCopy, MEMCATEGORY_GEOMETRY );
            mShadowCopy = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::upload( void *data, size_t elementStart, size_t elementCount )
    {
        if( mBufferType == BT_IMMUTABLE )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Cannot use upload on an immutable buffer!",
                         "BufferPacked::upload" );
        }

        if( elementCount + elementStart > mNumElements )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Size of the provided data goes out of bounds!",
                         "BufferPacked::upload" );
        }

        if( mShadowCopy )
        {
            memcpy( (char*)mShadowCopy + elementStart * mBytesPerElement,
                    data, elementCount * mBytesPerElement );
        }
    }
    //-----------------------------------------------------------------------------------
    void* BufferPacked::map(size_t elementStart, size_t elementCount, MappingState persistentMethod )
    {
        if( mBufferType != BT_DYNAMIC )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Only dynamic buffers can be mapped! Use upload instead.",
                         "BufferPacked::map" );
        }

        if( persistentMethod == MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "persistentMethod cannot be MS_UNMAPPED",
                         "BufferPacked::map" );
        }

        //Can't map twice, unless this is a persistent mapping and we're
        //being called with the same persistency flag as before.
        if( mMappingState != MS_UNMAPPED &&
            (mMappingState != persistentMethod || persistentMethod < MS_PERSISTENT_INCOHERENT) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Buffer already mapped! (or missmatching persistency flag between calls)",
                         "BufferPacked::map" );
        }

        if( mMappingState == persistentMethod && persistentMethod >= MS_PERSISTENT_INCOHERENT &&
            (elementStart < mMappingStart  ||
             elementCount > (mMappingStart - elementStart) + mMappingCount) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "The buffer is already persistently mapped, but the requested subregion lies "
                         " outside this buffer. The first map should've been bigger (or unmap "
                         "the buffer and remap)", "BufferPacked::map" );
        }

        MappingState prevMappingState = mMappingState;
        mMappingState = persistentMethod;

        return mBufferInterface->map( elementStart, elementCount, prevMappingState );
    }
    //-----------------------------------------------------------------------------------
    void BufferPacked::unmap( UnmapOptions unmapOption )
    {
        if( mMappingState == MS_UNMAPPED )
        {
            OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                         "Buffer wasn't mapped!",
                         "BufferPacked::unmap" );
        }

        mBufferInterface->unmap( unmapOption );

        if( unmapOption || mMappingState == MS_MAPPED )
            mMappingState = MS_UNMAPPED;
    }
}

