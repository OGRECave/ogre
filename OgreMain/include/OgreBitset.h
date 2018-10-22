/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2018 Torus Knot Software Ltd

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

#ifndef _OgreBitset_
#define _OgreBitset_

#include "OgrePrerequisites.h"

namespace Ogre
{
	/** @ingroup Core
	@class cbitsetN
	*/
	template<size_t _N, typename _internalDataType, size_t _bits, size_t _mask> class cbitsetN
	{
		_internalDataType mValues[_N >> _bits];
	public:
		cbitsetN()
		{
			clear();
		}

		void clear()
		{
			memset( mValues, 0, sizeof(mValues) );
		}

		void setValue( size_t position, bool bValue )
		{
			OGRE_ASSERT_MEDIUM( position < _N );
			const size_t idx  = position >> _bits;
			const size_t mask = 1u << (position & _mask);
			if( bValue )
				mValues[idx] |= mask;
			else
				mValues[idx] &= ~mask;
		}

		void set( size_t position )
		{
			OGRE_ASSERT_MEDIUM( position < _N );
			const size_t idx  = position >> _bits;
			const size_t mask = 1u << (position & _mask);
			mValues[idx] |= mask;
		}
		void unset( size_t position )
		{
			OGRE_ASSERT_MEDIUM( position < _N );
			const size_t idx  = position >> _bits;
			const size_t mask = 1u << (position & _mask);
			mValues[idx] &= ~mask;
		}
		bool test( size_t position ) const
		{
			OGRE_ASSERT_MEDIUM( position < _N );
			const size_t idx  = position >> _bits;
			const size_t mask = 1u << (position & _mask);
			return (mValues[idx] & mask) != 0u;
		}
	};

	/** @ingroup Core
	@class cbitset32
		This is similar to std::bitset, except waaay less bloat.
		cbitset32 stands for constant/compile-time bitset with an internal representation of 32-bits
	*/
	template<size_t _N> class cbitset32 : public cbitsetN<_N, uint32, 5u, 0x1Fu>
	{
	};
}

#endif
