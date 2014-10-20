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

#ifndef __RawPtr_H__
#define __RawPtr_H__

#include "OgrePlatform.h"

//#define OGRE_RAW_PTR_PROFILE

#ifdef OGRE_RAW_PTR_PROFILE
    #include "OgreException.h"
#endif

namespace Ogre
{
    /** Similar to std::unique_ptr, but:
            * Uses a custom allocator (OGRE_MALLOC_SIMD)
            * Pointers must be really unique (RESTRICT_ALIAS modifier is used!)
            * To access the pointer, use get(); instead of using this container directly
        The purpose of this container is to enclose a raw pointer while avoiding breaking
        the rule of 3 when copying.
        When defining the macro "OGRE_RAW_PTR_PROFILE", this container will raise an exception
        if the copy constructor or the assignment operator are used.
    */
    template <typename T, MemoryCategory M_CATEGORY> class RawSimdUniquePtr
    {
        T * RESTRICT_ALIAS  mPtr;
        size_t              mNumElements;

    public:
        RawSimdUniquePtr() : mPtr( 0 ), mNumElements( 0 ) {}

        RawSimdUniquePtr( size_t numElements ) :
            mPtr( static_cast<T * RESTRICT_ALIAS>(
                    OGRE_MALLOC_SIMD( numElements * sizeof( T ), M_CATEGORY ) ) ),
            mNumElements( numElements )
        {
        }

        RawSimdUniquePtr( const RawSimdUniquePtr &copy ) :
            mPtr( 0 ),
            mNumElements( copy.mNumElements )
        {
            if( copy.mPtr )
            {
#ifdef OGRE_RAW_PTR_PROFILE
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Called copy constructor (performance warning)! Consider using swap "
                             "or undefine OGRE_RAW_PTR_PROFILE", "RawSimdUniquePtr::RawSimdUniquePtr" );
#endif
                mPtr = static_cast<T * RESTRICT_ALIAS>(
                            OGRE_MALLOC_SIMD( copy.mNumElements * sizeof( T ), M_CATEGORY ) );
                memcpy( mPtr, copy.mPtr, copy.mNumElements * sizeof( T ) );
            }
        }

        ~RawSimdUniquePtr()
        {
            OGRE_FREE_SIMD( mPtr, M_CATEGORY );
            mPtr = 0;
        }

        void swap( RawSimdUniquePtr &copy )
        {
            std::swap( mPtr, copy.mPtr );
            std::swap( mNumElements, copy.mNumElements );
        }

        void operator = ( const RawSimdUniquePtr &copy )
        {
            if( this != &copy )
            {
                OGRE_FREE_SIMD( mPtr, M_CATEGORY );
                mPtr = 0;
                mNumElements = copy.mNumElements;

                if( copy.mPtr )
                {
#ifdef OGRE_RAW_PTR_PROFILE
                    OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Called assignment operator (performance warning)! Consider using swap "
                             "or undefine OGRE_RAW_PTR_PROFILE", "RawSimdUniquePtr::RawSimdUniquePtr" );
#endif
                    mPtr = static_cast<T * RESTRICT_ALIAS>(
                                    OGRE_MALLOC_SIMD( copy.mNumElements * sizeof( T ), M_CATEGORY ) );
                    memcpy( mPtr, copy.mPtr, copy.mNumElements * sizeof( T ) );
                }
            }
        }

        T * RESTRICT_ALIAS_RETURN get()                               { return mPtr; }
        const T * RESTRICT_ALIAS_RETURN get() const                   { return mPtr; }
        size_t size() const                     { return mNumElements; }
    };
}

#endif
