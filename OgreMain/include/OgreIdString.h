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

#ifndef __OgreIdString__
#define __OgreIdString__

#include "OgreMurmurHash3.h"
#include <stdio.h>  // sprintf
#include <string.h> // strlen
#include <string>

#define OGRE_HASH_FUNC MurmurHash3_x86_32
#define OGRE_HASH_BITS 32

#if OGRE_DEBUG_MODE == 0
    #define OGRE_COPY_DEBUG_STRING( _Expression ) ((void)0)
    #define OGRE_APPEND_DEBUG_STRING( _Expression ) ((void)0)
#else
    #include "assert.h"
#endif

namespace Ogre
{
    /** Hashed string.
        An IdString is meant to be passed by value rather than by reference since in Release
        mode it's just an encapsulated integer. The default implementation uses a 32-bit uint.
        A hash = 0 is the default initialization, and assumed by Ogre to mean an IdString is
        empty or unspecified.
    @par
        Note that IdString("") != IdString(0) != IdString()
        The latter indicates mHash == 0 (the default initialization). While the first
        one will hash the empty string "", and the second will attempt to hash the
        number 0, producing up to three possible outputs.
    @par
        IdStrings convert regular String to hashes AND ARE DESTRUCTIVE.
        This means that you may no longer recover the real string it was constructed from.
        When the original data is not available getFriendlyText returns [Hash 0x0a0100ef]
        (in the example that mHash = 0x0a0100ef)*
        In debug mode (OGRE_DEBUG_MODE is defined), IdStrings try to maintain a copy of
        the original string for two purposes:
            1. Easy debugging: Reading "Texture/diffuse.png" is much nicer than "0x0a0100ef"
            2. Hash collision: If Ogre finds two IdStrings are identical but their original
               strings are not, an assertion will trigger indicating a collision has been
               found. Note that this isn't foolproof.
            * Note: When IdString( uint32 ) was used to hash an integer, getFriendlyText
              will return "[Value 0x1234567]"
    @par
        IdStrings can be concatenated: IdString( "Hello." ) + IdString( " How are you?" )
        produc a new IdString, but please note that it may not be the same as doing
        IdString( "Hello. How are you?" ) depending on the hashing function used
        (by default, we use MurmurHash3, @see OGRE_HASH_FUNC)

    @remarks
        Hash collision detection isn't foolproof because we keep a fixed string copy of
        the original string. If the original string is too long, it will be truncated
        and there is an extremely low possibility that their truncation becomes
        the same string, and their hashes also become equal, when the original strings
        were not. i.e.
        VeryLooooooong/String and UltraaaaaLoong/String could both become "/String"
        and happen to also hash to the same number.
    @par
        In practice we truncate to 32 bytes. If your fear this is too little for you and
        also fear about collisions, increase @see OGRE_DEBUG_STR_SIZE
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    struct IdString
    {
        static const uint32_t Seed = 0x3A8EFA67; //It's a prime number :)

        uint32      mHash;
#if OGRE_DEBUG_MODE
        #define OGRE_DEBUG_STR_SIZE 32
        char        mDebugString[OGRE_DEBUG_STR_SIZE];
#endif

        IdString() : mHash( 0 )
        {
#if OGRE_DEBUG_MODE
            mDebugString[0] = '\0';
#endif
        }

        IdString( const char *string ) : mHash( 0 )
        {
            OGRE_HASH_FUNC( string, static_cast<int>(strlen( string )), Seed, &mHash );
            OGRE_COPY_DEBUG_STRING( string );
        }

        IdString( const std::string &string ) : mHash( 0 )
        {
            OGRE_HASH_FUNC( string.c_str(), static_cast<int>(string.size()), Seed, &mHash );
            OGRE_COPY_DEBUG_STRING( string );
        }

        IdString( uint32 value ) : mHash( 0 )
        {
            OGRE_HASH_FUNC( &value, sizeof( value ), Seed, &mHash );
            OGRE_COPY_DEBUG_STRING( value );
        }

#if OGRE_DEBUG_MODE
        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( push )
            #pragma warning( disable: 4996 ) //Unsecure CRT deprecation warning
        #endif

        void OGRE_COPY_DEBUG_STRING( const char *string )
        {
            size_t strLength = strlen( string );
            if( strLength > OGRE_DEBUG_STR_SIZE-1 )
            {
                //Copy the last characters, not the first ones!
                strncpy( mDebugString, string + strLength - (OGRE_DEBUG_STR_SIZE-1),
                         OGRE_DEBUG_STR_SIZE );
            }
            else
            {
                strncpy( mDebugString, string, OGRE_DEBUG_STR_SIZE );
            }
            mDebugString[OGRE_DEBUG_STR_SIZE-1] = '\0';
        }

        void OGRE_COPY_DEBUG_STRING( const std::string &string )
        {
            size_t strLength = string.size();
            if( strLength > OGRE_DEBUG_STR_SIZE-1 )
            {
                //Copy the last characters, not the first ones!
                strncpy( mDebugString, string.c_str() + strLength - (OGRE_DEBUG_STR_SIZE-1),
                         OGRE_DEBUG_STR_SIZE );
            }
            else
            {
                strncpy( mDebugString, string.c_str(), OGRE_DEBUG_STR_SIZE );
            }
            mDebugString[OGRE_DEBUG_STR_SIZE-1] = '\0';
        }

        void OGRE_COPY_DEBUG_STRING( uint32 value )
        {
            sprintf( mDebugString, "[Value 0x%.8x]", value );
            mDebugString[OGRE_DEBUG_STR_SIZE-1] = '\0';
        }

        void OGRE_APPEND_DEBUG_STRING( const char *string )
        {
            size_t strLen0 = strlen( mDebugString );
            size_t strLen1 = strlen( string );

            if( strLen0 + strLen1 < OGRE_DEBUG_STR_SIZE )
            {
                strcat( mDebugString, string );
                mDebugString[OGRE_DEBUG_STR_SIZE-1] = '\0';
            }
            else
            {
                size_t newStart0    = (strLen0 >> 1);
                size_t newLen0      = strLen0 - newStart0;
                memmove( mDebugString, mDebugString + newStart0, newLen0 );

                size_t newStart1    = 0;
                size_t newLen1      = strLen1;
                if( newLen0 + strLen1 >= OGRE_DEBUG_STR_SIZE )
                {
                    newLen1     = OGRE_DEBUG_STR_SIZE - newLen0 - 1;
                    newStart1   = strLen1 - newLen1;
                }

                memcpy( mDebugString + newLen0, string + newStart1, newLen1 );
                mDebugString[OGRE_DEBUG_STR_SIZE-1] = '\0';
            }
        }

        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( pop )
        #endif
#endif

        void operator += ( IdString idString )
        {
            uint32 doubleHash[2];
            doubleHash[0] = mHash;
            doubleHash[1] = idString.mHash;

            OGRE_HASH_FUNC( &doubleHash, sizeof( doubleHash ), Seed, &mHash );
            OGRE_APPEND_DEBUG_STRING( idString.mDebugString );
        }

        IdString operator + ( IdString idString ) const
        {
            IdString retVal( *this );
            retVal += idString;
            return retVal;
        }

        bool operator < ( IdString idString ) const
        {
#if OGRE_DEBUG_MODE
            //On highly debug builds, check for collisions
            assert( !(mHash == idString.mHash &&
                    strcmp( mDebugString, idString.mDebugString ) != 0) &&
                    "Collision detected!" );
#endif
            return mHash < idString.mHash;
        }

        bool operator == ( IdString idString ) const
        {
#if OGRE_DEBUG_MODE
            assert( !(mHash == idString.mHash &&
                    strcmp( mDebugString, idString.mDebugString ) != 0) &&
                    "Collision detected!" );
#endif
            return mHash == idString.mHash;
        }

        bool operator != ( IdString idString ) const
        {
#if OGRE_DEBUG_MODE
            assert( !(mHash == idString.mHash &&
                    strcmp( mDebugString, idString.mDebugString ) != 0) &&
                    "Collision detected!" );
#endif
            return mHash != idString.mHash;
        }

        /// Returns "[Hash 0x0a0100ef]" strings in Release mode, readable string in debug
        std::string getFriendlyText() const
        {
#if OGRE_DEBUG_MODE
            return std::string( mDebugString );
#else
            return getReleaseText();
#endif
        }

        /// Always returns "[Hash 0x0a0100ef]" strings in any mode
        std::string getReleaseText() const
        {
        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( push )
            #pragma warning( disable: 4996 ) //Unsecure CRT deprecation warning
        #endif

            char tmp[(OGRE_HASH_BITS >> 2)+10];
            sprintf( tmp, "[Hash 0x%.8x]", mHash );
            tmp[(OGRE_HASH_BITS >> 2)+10-1] = '\0';
            return std::string( tmp );

        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( pop )
        #endif
        }
    };
}

#endif

