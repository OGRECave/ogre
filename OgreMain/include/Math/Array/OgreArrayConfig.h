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
#ifndef __OgreArrayConfig_H__
#define __OgreArrayConfig_H__

#include "OgreConfig.h"
#include "OgrePlatformInformation.h"

#if OGRE_USE_SIMD == 1
	#if OGRE_CPU == OGRE_CPU_X86
		//x86/x64 - SSE2
		#if OGRE_DOUBLE_PRECISION == 1
			#include <emmintrin.h>
			#define ARRAY_PACKED_REALS 2
			namespace Ogre {
				typedef __m128d ArrayReal;
			}
		#else
			#include <xmmintrin.h>
			#include <emmintrin.h>	//SSE Math library still needs SSE2
			#define ARRAY_PACKED_REALS 4
			namespace Ogre {
				typedef __m128 ArrayReal;
				typedef __m128 ArrayMaskR;

				#define ARRAY_REAL_ZERO _mm_setzero_ps()
				#define ARRAY_INT_ZERO _mm_setzero_si128()

				class ArrayRadian;
			}

			#define OGRE_PREFETCH_T0( x ) _mm_prefetch( x, _MM_HINT_T0 )
			#define OGRE_PREFETCH_T1( x ) _mm_prefetch( x, _MM_HINT_T1 )
			#define OGRE_PREFETCH_T2( x ) _mm_prefetch( x, _MM_HINT_T2 )
			#define OGRE_PREFETCH_NTA( x ) _mm_prefetch( x, _MM_HINT_NTA )

			//Distance (in ArrayMemoryManager's slots) used to keep fetching data. This also
			//means the memory manager needs to allocate extra memory for them.
			#define OGRE_PREFETCH_SLOT_DISTANCE		4*ARRAY_PACKED_REALS //Must be multiple of ARRAY_PACKED_REALS
		#endif

		namespace Ogre {
			typedef __m128i ArrayInt;
			typedef __m128i ArrayMaskI;
		}

        ///r = (a * b) + c
        #define _mm_madd_ps( a, b, c )		_mm_add_ps( c, _mm_mul_ps( a, b ) )
        ///r = -(a * b) + c
        #define _mm_nmsub_ps( a, b, c )		_mm_sub_ps( c, _mm_mul_ps( a, b ) )

		/// Does not convert, just cast ArrayReal to ArrayInt
		#define CastRealToInt( x )			_mm_castps_si128( x )
		#define CastIntToReal( x )			_mm_castsi128_ps( x )
		/// Input must be 16-byte aligned
		#define CastArrayToReal( outFloatPtr, arraySimd )		_mm_store_ps( outFloatPtr, arraySimd )

    #elif OGRE_CPU == OGRE_CPU_ARM
        // ARM - NEON
        #include <arm_neon.h>
//        #if OGRE_DOUBLE_PRECISION == 1
//            #define ARRAY_PACKED_REALS 2
//            namespace Ogre {
//                typedef __m128d ArrayReal;
//            }
//        #else
            #define ARRAY_PACKED_REALS 4
            namespace Ogre {
                typedef float32x4_t ArrayReal;

                class ArrayRadian;
            }
//        #endif

	#else
		//Unsupported architecture, tell user to reconfigure. We could silently fallback to C,
		//but this is very green code, and architecture may be x86 with a rare compiler.
		#error "Unknown platform or platform not supported for SIMD. Build Ogre without OGRE_USE_SIMD"
	#endif
#else
	//No SIMD, use C implementation
	#define ARRAY_PACKED_REALS 1
	namespace Ogre {
		typedef Real ArrayReal;
		typedef Radian ArrayRadian;
		typedef Radian ArrayRadian;
		typedef uint32 ArrayInt;
		typedef bool ArrayMaskR;
		typedef bool ArrayMaskI;

		#define CastIntToReal( x ) x
		#define CastRealToInt( x ) x

		#define ogre_madd( a, b, c )		( (c) + ( (a) * (b) ) )

		#define OGRE_PREFETCH_T0( x ) ((void)0)
		#define OGRE_PREFETCH_T1( x ) ((void)0)
		#define OGRE_PREFETCH_T2( x ) ((void)0)
		#define OGRE_PREFETCH_NTA( x ) ((void)0)

		#define ARRAY_INT_ZERO 0

		/// Input must be 16-byte aligned
		#define CastArrayToReal( outFloatPtr, arraySimd )		(*(outFloatPtr) = arraySimd)

		//Distance (in ArrayMemoryManager's slots) used to keep fetching data. This also
		//means the memory manager needs to allocate extra memory for them.
		#define OGRE_PREFETCH_SLOT_DISTANCE		0 //Must be multiple of ARRAY_PACKED_REALS
	}
#endif

#endif
