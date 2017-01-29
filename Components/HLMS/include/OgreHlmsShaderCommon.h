/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#pragma once


#include "Ogre.h"			
#include "OgreHlmsPrerequisites.h"
#include "OgreIdString.h"
#include "OgreMurmurHash3.h"
#include <cstddef>

namespace Ogre
{
	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
    class _OgreHlmsExport SubStringRef
    {
		String const *mOriginal;
        size_t  mStart;
        size_t  mEnd;

    public:
		SubStringRef(const String *original, size_t start) :
            mOriginal( original ),
            mStart( start ),
            mEnd( original->size() )
        {
            assert( start <= original->size() );
        }

		SubStringRef(const String *original, size_t _start, size_t _end) :
            mOriginal( original ),
            mStart( _start ),
            mEnd( _end )
        {
            assert( _start <= _end );
            assert( _end <= original->size() );
        }

		SubStringRef(const String *original, String::const_iterator _start) :
            mOriginal( original ),
            mStart( _start - original->begin() ),
            mEnd( original->size() )
        {
        }

        size_t find( const char *value, size_t pos=0 ) const
        {
            size_t retVal = mOriginal->find( value, mStart + pos );
            if( retVal >= mEnd )
				retVal = String::npos;
			else if (retVal != String::npos)
                retVal -= mStart;

            return retVal;
        }

		size_t find(const String &value) const
        {
            size_t retVal = mOriginal->find( value, mStart );
            if( retVal >= mEnd )
				retVal = String::npos;
			else if (retVal != String::npos)
                retVal -= mStart;

            return retVal;
        }

        size_t findFirstOf( const char *c, size_t pos ) const
        {
            size_t retVal = mOriginal->find_first_of( c, mStart + pos );
            if( retVal >= mEnd )
				retVal = String::npos;
			else if (retVal != String::npos)
                retVal -= mStart;

            return retVal;
        }

        bool matchEqual( const char *stringCompare ) const
        {
            const char *origStr = mOriginal->c_str() + mStart;
            ptrdiff_t length = mEnd - mStart;
            while( *origStr == *stringCompare && *origStr && --length )
                ++origStr, ++stringCompare;

            return length == 0 && *origStr == *stringCompare;
        }

        void setStart( size_t newStart )            { mStart = std::min( newStart, mOriginal->size() ); }
        void setEnd( size_t newEnd )                { mEnd = std::min( newEnd, mOriginal->size() ); }
        size_t getStart(void) const                 { return mStart; }
        size_t getEnd(void) const                   { return mEnd; }
        size_t getSize(void) const                  { return mEnd - mStart; }
		String::const_iterator begin() const        { return mOriginal->begin() + mStart; }
		String::const_iterator end() const          { return mOriginal->begin() + mEnd; }
		const String& getOriginalBuffer() const     { return *mOriginal; }
    };

	const String FilePatterns[] = { "_vs", "_fs", "_gs", "_ds", "_hs" };

	typedef vector< std::pair<IdString, String> >::type HlmsParamVec;

	inline bool OrderParamVecByKey(const std::pair<IdString, String> &_left,
		const std::pair<IdString, String> &_right)
    {
        return _left.first < _right.first;
    }

	inline uint32 calcHash(const void* data, int size)
	{
		uint32 finalHash;
		MurmurHash3_x86_32(data, size, IdString::Seed, &finalHash);
		return finalHash;
	}

	inline uint32 calcHash(String str)
	{
		const char* chars = str.c_str();
		return calcHash(chars, str.length() * sizeof(char));
	}

	inline uint32 calcHash(const StringVector& vec)
	{
		StringStream stream;
		for (unsigned int i = 0; i < vec.size(); i++)
		{
			stream << vec[i];
		}
		
		String str = stream.str();
		const char* chars = str.c_str();
		return calcHash(chars, str.length() * sizeof(char));
	}
}
