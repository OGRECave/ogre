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

#ifndef _OgreLwString_H_
#define _OgreLwString_H_

#include "OgreLwConstString.h"

#ifndef _MSC_VER
    #define OGRE_LWSTRING_SNPRINTF_DEFINED
    #define _snprintf snprintf
#else
    #pragma warning( push ) // CRT deprecation
    #pragma warning( disable : 4996 )
#endif

namespace Ogre
{
    /** This is a Light-weight string wrapper around the C string interface.
        This class is heavily influenced by Tom Forsyth's article "A sprintf
        that isn't as ugly"
        https://home.comcast.net/~tom_forsyth/blog.wiki.html#[[A%20sprintf%20that%20isn%27t%20as%20ugly]]

        The goals are:
            * No dynamic allocation.
            * Easier to read and control than sprintf()
            * Type-safe (in as much as C can ever be type-safe - bloody auto-converts).
            * Overflow-safe (i.e. it will refuse to scribble, and will assert in debug mode).

        LwString needs to be fed a pointer and a maximum length size. The pointer's
        preexisting contents will be preserved. For example:
            char buf[1024];
            buf[0] = '\0';
            LwString myStr( buf, 1024 );

            {
                char *buf = new char[1024];
                LwString myStr( buf, 1024 );
                delete buf; //This class will never deallocate the pointer.
            }

            char buf[512];
            buf[0] = 'H'; buf[1] = 'i'; buf[2] = '!'; buf[3] = '\0';
            LwString myStr( buf, 512 );
            printf( myStr.c_str() ) //Hi!

        In order to concatenate strings, you need to use the a() function.
        Concatenations are appended to the current string.
        You can concatenate strings, integers, raw char strings, and floats.
        Examples:
            //Example 1:
            myStr.a( "Hello", " ", "World!" ); //Hello World!

            //Example 2:
            myStr.a( "Hello", " " );
            myStr.a( "World!" ); //Hello World!

            //Example 3:
            char myZero = 0;
            myStr.a( "One", " + ", 1, " equals ", myZero ); //One + 1 equals 0

            //Example 4:
            myStr.a( 1.5f, " + ", 1.5f, " = ", 1.5f + 1.5f ); //1.5 + 1.5 = 3.0

        The += operator only works with strings.

        Additionally, there are a few STL-like interfaces:
            begin (iterator), end (iterator), find, find_first_of
        The input arguments, behavior and return values are the same as std::string.

        Users can specify the precision of floats using LwString::Float
            myStr.a( LwString::Float( 3.5f, precision, minWidth ) );
    @remarks
        The following code will fail to compile:
            myStr.a( 4294967295 );
        The compiler doesn't know whether the literal
        means 4294967295 (unsigned), or -1 (signed).
        To fix the compile error, perform an explicit cast:
            myStr.a( (uint32)4294967295 );  //Prints 4294967295
            myStr.a( (int32)4294967295 );   //Prints -1
    */
    class LwString : public LwConstString
    {
    public:
        LwString( char *inStr, size_t maxLength ) :
            LwConstString( inStr, maxLength )
        {
            mStrPtr[mCapacity-1] = '\0';
        }

        static LwString FromEmptyPointer( char *cStr, size_t maxLength )
        {
            cStr[0] = '\0';
            return LwString( cStr, maxLength );
        }

        /// Takes a subset from source in the range [_start; _start + size),
        /// and copies it to our string.
        void setToSubstr( const LwConstString &source, size_t _start, size_t size )
        {
            assert( (size + 1u) <= this->mCapacity );
            assert( _start <= source.mSize );
            assert( _start + size <= source.mSize );

            _start  = std::min( _start, source.mSize );
            size    = std::min( size, source.mSize - _start );
            size    = std::min( size, this->mCapacity - 1 );

            memmove( this->mStrPtr, source.mStrPtr + _start, size );
            this->mSize = size;
            this->mStrPtr[this->mSize] = '\0';
        }

        /** @see setToSubstr overload. This method takes iterators to the
            source string.
            That is, both _start & _end are in range [source.begin(), source.end()]
        */
        void setToSubstr( const LwConstString &source, char *_start, char *_end )
        {
            assert( _start >= source.begin() && _start <= source.end() );
            this->setToSubstr( source, _start - source.mStrPtr, _end - _start );
        }

        void clear()
        {
            mStrPtr[0] = '\0';
            mSize = 0;
        }

        /// Resizes the string. Unlike std::string, the new
        /// size MUST be lower or equal than current size.
        /// (i.e. we can only shrink)
        void resize( size_t newSize )
        {
            assert( newSize <= this->mSize );

            if( newSize < this->mSize )
            {
                mStrPtr[newSize] = '\0';
                mSize = newSize;
            }
        }

        // Assignment.
        LwString& operator = ( const LwConstString &other )
        {
            assert( other.mSize < this->mCapacity );
            strncpy( this->mStrPtr, other.mStrPtr, this->mCapacity );
            this->mStrPtr[this->mCapacity-1] = '\0';
            this->mSize = std::min( other.mSize, this->mCapacity - 1 );
            return *this;
        }

        // Assignment from a C string.
        LwString& operator = ( const char *pOther )
        {
            size_t otherSize = strlen( pOther );
            assert( otherSize < this->mCapacity );
            strncpy( this->mStrPtr, pOther, this->mCapacity );
            this->mStrPtr[this->mCapacity-1] = '\0';
            this->mSize = std::min( otherSize, this->mCapacity - 1 );
            return *this;
        }

        LwString& operator += ( const LwString &other )
        {
            this->a( other );
            return *this;
        }

        // Append of a C string.
        LwString& operator += ( const char *pOther )
        {
            this->a( pOther );
            return *this;
        }

        LwString& a( const LwString& a0 )
        {
            assert( this->mSize + a0.mSize < this->mCapacity );
            assert( this->mStrPtr != a0.mStrPtr );
            size_t maxAppendCount = this->mCapacity - this->mSize - 1;
            strncat( this->mStrPtr + this->mSize, a0.mStrPtr, maxAppendCount );
            this->mSize += std::min( a0.mSize, maxAppendCount );
            return *this;
        }

        LwString& a( const char *a0 )
        {
            size_t otherSize = strlen( a0 );
            assert( this->mSize + otherSize < this->mCapacity );
            assert( this->mStrPtr != a0 );
            size_t maxAppendCount = this->mCapacity - this->mSize - 1;
            strncat( this->mStrPtr + this->mSize, a0, maxAppendCount );
            this->mSize += std::min( otherSize, maxAppendCount );
            return *this;
        }

        LwString& a( int32 a0 )
        {
            int written = _snprintf( mStrPtr + mSize,
                                     mCapacity - mSize,
                                     "%i", a0 );
            assert( ( written >= 0 ) && ( (size_t)written < mCapacity ) );
            mStrPtr[mCapacity - 1] = '\0';
            mSize = std::min<size_t>( mSize + std::max( written, 0 ), mCapacity - 1 );
            return *this;
        }

        LwString& a( uint32 a0 )
        {
            int written = _snprintf( mStrPtr + mSize,
                                     mCapacity - mSize,
                                     "%u", a0 );
            assert( ( written >= 0 ) && ( (size_t)written < mCapacity ) );
            mStrPtr[mCapacity - 1] = '\0';
            mSize = std::min<size_t>( mSize + std::max( written, 0 ), mCapacity - 1 );
            return *this;
        }

        LwString& a( int64 a0 )
        {
            int written = _snprintf( mStrPtr + mSize,
                                     mCapacity - mSize,
                                     "%lli", a0 );
            assert( ( written >= 0 ) && ( (size_t)written < mCapacity ) );
            mStrPtr[mCapacity - 1] = '\0';
            mSize = std::min<size_t>( mSize + std::max( written, 0 ), mCapacity - 1 );
            return *this;
        }

        LwString& a( uint64 a0 )
        {
            int written = _snprintf( mStrPtr + mSize,
                                     mCapacity - mSize,
                                     "%llu", a0 );
            assert( ( written >= 0 ) && ( (size_t)written < mCapacity ) );
            mStrPtr[mCapacity - 1] = '\0';
            mSize = std::min<size_t>( mSize + std::max( written, 0 ), mCapacity - 1 );
            return *this;
        }

        struct Float
        {
            float   mValue;
            int     mPrecision;
            int     mMinWidth;

            /**
            @param value
                float value to convert.
            @param precision
                Controls truncation/rounding. Example:
                    value       = 1.56
                    precision   = 1
                    prints "1.6"
            @param minWidth
                Controls the minimum width of the decimals. Example:
                    value       = 1.5
                    minWidth    = 2
                    prints "1.50"
            */
            Float( float value, int precision = -1, int minWidth = -1 ) :
                mValue( value ),
                mPrecision( precision ),
                mMinWidth( minWidth )
            {
            }
        };

        LwString& a( float a0 )
        {
            this->a( Float( a0 ) );
            return *this;
        }

        LwString& a( Float a0 )
        {
            int written = -1;
            if( a0.mMinWidth < 0 )
            {
                if( a0.mPrecision < 0 )
                {
                    written = _snprintf( mStrPtr + mSize, mCapacity - mSize,
                                         "%f", a0.mValue );
                }
                else
                {
                    written = _snprintf( mStrPtr + mSize, mCapacity - mSize,
                                         "%.*f", a0.mPrecision, a0.mValue );
                }
            }
            else
            {
                if( a0.mPrecision < 0 )
                {
                    written = _snprintf( mStrPtr + mSize, mCapacity - mSize,
                                         "%*f", a0.mMinWidth, a0.mValue );
                }
                else
                {
                    written = _snprintf( mStrPtr + mSize, mCapacity - mSize,
                                         "%*.*f", a0.mMinWidth, a0.mPrecision, a0.mValue );
                }
            }

            mStrPtr[mCapacity - 1] = '\0';
            assert( ( written >= 0 ) && ( (unsigned)written < mCapacity ) );
            mSize = std::min<size_t>( mSize + std::max( written, 0 ), mCapacity - 1 );
            return *this;
        }

        size_t size() const         { return mSize; }
        size_t capacity() const     { return mCapacity; }

        char* begin()               { return mStrPtr; }
        char* end()                 { return mStrPtr + mSize; }

        /// Make every begin & end method from LwConstString available.
        /// @see https://isocpp.org/wiki/faq/strange-inheritance#overload-derived
        using LwConstString::begin;
        using LwConstString::end;

        template<typename M, typename N>
        LwString & a( const M &a0, const N &a1 )
        {
            this->a( a0 );
            this->a( a1 );
            return *this;
        }

        template<typename M, typename N, typename O>
        LwString & a( const M &a0, const N &a1, const O &a2 )
        {
            this->a( a0 );
            this->a( a1 );
            this->a( a2 );
            return *this;
        }

        template<typename M, typename N, typename O, typename P>
        LwString & a( const M &a0, const N &a1, const O &a2, const P &a3 )
        {
            this->a( a0, a1 );
            this->a( a2, a3 );
            return *this;
        }

        template<typename M, typename N, typename O, typename P, typename Q>
        LwString & a( const M &a0, const N &a1, const O &a2, const P &a3, const Q &a4 )
        {
            this->a( a0, a1, a2, a3 );
            this->a( a4 );
            return *this;
        }

        template<typename M, typename N, typename O, typename P, typename Q, typename R>
        LwString & a( const M &a0, const N &a1, const O &a2,
                     const P &a3, const Q &a4, const R &a5 )
        {
            this->a( a0, a1, a2 );
            this->a( a3, a4, a5 );
            return *this;
        }

        template<typename M, typename N, typename O, typename P, typename Q, typename R, typename S>
        LwString & a( const M &a0, const N &a1, const O &a2, const P &a3,
                     const Q &a4, const R &a5, const S &a6 )
        {
            this->a( a0, a1, a2, a3 );
            this->a( a4, a5, a6 );
            return *this;
        }

        template<typename M, typename N, typename O, typename P,
                 typename Q, typename R, typename S, typename T>
        LwString & a( const M &a0, const N &a1, const O &a2, const P &a3,
                     const Q &a4, const R &a5, const S &a6, const S &a7 )
        {
            this->a( a0, a1, a2, a3 );
            this->a( a4, a5, a6, a7 );
            return *this;
        }
    };
}

#ifdef OGRE_LWSTRING_SNPRINTF_DEFINED
    #undef _snprintf
#else
    #pragma warning( pop )
#endif

#endif
