/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef __C_BooleanMask_H__
#define __C_BooleanMask_H___

#ifndef __BooleanMask_H__
	#error "Don't include this file directly. include Math/Array/OgreBooleanMask.h"
#endif

namespace Ogre
{
	class BooleanMask4
	{
	public:
		enum
		{
			MASK_NONE			= 0,
			MASK_X				= 1,
			NUM_MASKS			= 2
		};
	public:
		inline static ArrayMaskR getMask( bool x );
		inline static ArrayMaskR getMask( bool booleans[1] );

		/// Returns true if alls bit in mask0[i] and mask1[i] are set.
		inline static bool allBitsSet( bool mask0[1], bool mask1[1] );

		/** Converts a SIMD mask into a mask that fits in 32-bit number
		@remarks
			@See IS_SET_MASK_X & co. to read the mask, since the result may need
			byteswapping in some architectures (i.e. SSE2)
		*/
		inline static uint32 getScalarMask( ArrayMaskR mask );
	};
}

#include "OgreBooleanMask.inl"

#endif
