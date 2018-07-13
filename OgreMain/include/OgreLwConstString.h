// BSD License. Do what you want. I would like to know if you used this library
// in production though; but you're not forced to.
//
// Copyright (c) 2015, Matías N. Goldberg
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in
//     the documentation and/or other materials provided with the
//     distribution.
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _OgreLwConstString_H_
#define _OgreLwConstString_H_

#include "OgrePrerequisites.h"
#include <assert.h>
#include <string.h>
#include <algorithm>

#if !defined( _MSC_VER ) || (_MSC_VER > 1600)
    #include <stdint.h>
#endif
#include <stdio.h>


#ifdef _MSC_VER
    #pragma warning( push ) // CRT deprecation
    #pragma warning( disable : 4996 )
#endif

namespace Ogre
{
    /** @see LwString. This base class exists so that you can wrap around useful
        functionality around const char pointers, for example:
            const char myText = "Hello World!";
            LwConstString myString( myText, 13 );
        such as comparison, finding, etc.
        The internal pointer is const_cast'ed because derived class LwString needs it so,
        however this base class *never* writes to this pointer or returns a non-const
        pointer.
    */
    class _OgreExport LwConstString
    {
        friend class LwString;
    protected:
#if OGRE_DEBUG_MODE
        char const *WarningHeader;
        static const char *WarningHeaderConst;
#endif
        char *mStrPtr;
        size_t mSize;

        /// Unlike the std lib, mCapacity is ALWAYS going to be mSize < mCapacity
        /// because we need +1 for the null terminator.
        size_t mCapacity;

    public:
        LwConstString( const char *inStr, size_t maxLength ) :
            mStrPtr( const_cast<char*>( inStr ) ),
            mSize( strnlen( inStr, maxLength ) ),
            mCapacity( maxLength )
        {
            //mSize < mCapacity and not mSize <= mCapacity
            //because we need to account the null terminator
            assert( mSize < mCapacity );
#if OGRE_DEBUG_MODE
            WarningHeader = WarningHeaderConst;
#endif
        }

        static LwConstString FromUnsafeCStr( const char *cStr )
        {
            return LwConstString( cStr, strlen( cStr ) + 1 );
        }

        const char *c_str() const
        {
            return mStrPtr;
        }

        size_t size() const         { return mSize; }
        size_t capacity() const     { return mCapacity; }

        const char* begin() const   { return mStrPtr; }
        const char* end() const     { return mStrPtr + mSize; }

        size_t find( const char *val, size_t pos = 0 ) const
        {
            pos = std::min( pos, mSize );
            const char *result = strstr( mStrPtr + pos, val );
            return result ? result - mStrPtr : (size_t)(~0);
        }

        size_t find( const LwConstString *val, size_t pos = 0 ) const
        {
            return find( val->mStrPtr, pos );
        }

        size_t find_first_of( char c, size_t pos = 0 ) const
        {
            pos = std::min( pos, mSize );
            const char *result = strchr( mStrPtr + pos, c );
            return result ? result - mStrPtr : (size_t)(~0);
        }

        size_t find_first_of( const char *val, size_t pos = 0 ) const
        {
            pos = std::min( pos, mSize );
            const char *result = strpbrk( mStrPtr + pos, val );
            return result ? result - mStrPtr : (size_t)(~0);
        }

        size_t find_last_of( char c, size_t pos = ~0 ) const
        {
            size_t retVal = size_t(~0);

            size_t curr = 0;
            const char *s = mStrPtr;
            do
            {
                if( *s == c && curr <= pos )
                    retVal = curr;
                ++curr;
            } while( *s++ );

            return retVal;
        }

        bool operator == ( const LwConstString &other ) const
        {
            return strcmp( mStrPtr, other.mStrPtr ) == 0;
        }

        bool operator != ( const LwConstString &other ) const
        {
            return strcmp( mStrPtr, other.mStrPtr ) != 0;
        }

        bool operator < ( const LwConstString &other ) const
        {
            return strcmp( mStrPtr, other.mStrPtr ) < 0;
        }

        bool operator > ( const LwConstString &other ) const
        {
            return strcmp( mStrPtr, other.mStrPtr ) > 0;
        }

        bool operator == ( const char *other ) const
        {
            return strcmp( mStrPtr, other ) == 0;
        }

        bool operator != ( const char *other ) const
        {
            return strcmp( mStrPtr, other ) != 0;
        }

        bool operator < ( const char *other ) const
        {
            return strcmp( mStrPtr, other ) < 0;
        }

        bool operator > ( const char *other ) const
        {
            return strcmp( mStrPtr, other ) > 0;
        }

    private:
        // Forbidden calls
        // Assignment.
        LwConstString& operator = ( const LwConstString &other )
        {
            assert( false && "Can't call asignment operator!" );
            return *this;
        }
    };
}

#ifdef _MSC_VER
    #pragma warning( pop )
#endif

#endif
