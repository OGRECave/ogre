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
#ifndef __SIMDHelper_H__
#define __SIMDHelper_H__

#include "OgrePrerequisites.h"
#include "OgrePlatformInformation.h"

// Stack-alignment hackery.
//
// If macro __OGRE_SIMD_ALIGN_STACK defined, means there requests
// special code to ensure stack align to a 16-bytes boundary.
//
// Note:
//   This macro can only guarantee callee stack pointer (esp) align
// to a 16-bytes boundary, but not that for frame pointer (ebp).
// Because most compiler might use frame pointer to access to stack
// variables, so you need to wrap those alignment required functions
// with extra function call.
//
#if defined(__INTEL_COMPILER)
// For intel's compiler, simply calling alloca seems to do the right
// thing. The size of the allocated block seems to be irrelevant.
#define __OGRE_SIMD_ALIGN_STACK()   _alloca(16)
#define __OGRE_SIMD_ALIGN_ATTRIBUTE

#elif OGRE_CPU == OGRE_CPU_X86 && (OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG) && (OGRE_ARCH_TYPE != OGRE_ARCHITECTURE_64)
// mark functions with GCC attribute to force stack alignment to 16 bytes
#define __OGRE_SIMD_ALIGN_ATTRIBUTE __attribute__((force_align_arg_pointer))

#elif defined(_MSC_VER)
// Fortunately, MSVC will align the stack automatically
#define __OGRE_SIMD_ALIGN_ATTRIBUTE

#else
#define __OGRE_SIMD_ALIGN_ATTRIBUTE

#endif


// Additional platform-dependent header files and declares.
//
// NOTE: Should be sync with __OGRE_HAVE_SSE macro.
//

#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86

// GCC version 4.0 upwards should be reliable for official SSE now,
// so no longer define SSE macros ourselves
// We don't support gcc 3.x anymore anyway, although that had SSE it was a bit flaky?
#include <xmmintrin.h>


#endif // OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86



//---------------------------------------------------------------------
// SIMD macros and helpers
//---------------------------------------------------------------------


namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/

#if __OGRE_HAVE_SSE

/** Macro __MM_RSQRT_PS calculate square root, which should be used for
    normalise normals only. It might be use NewtonRaphson reciprocal square
    root for high precision, or use SSE rsqrt instruction directly, based
    on profile to pick up perfect one.
@note:
    Prefer to never use NewtonRaphson reciprocal square root at all, since
    speed test indicate performance loss 10% for unrolled version, and loss
    %25 for general version (P4 3.0G HT). A slight loss in precision not
    that important in case of normalise normals.
*/
#if 1
#define __MM_RSQRT_PS(x)    _mm_rsqrt_ps(x)
#else
#define __MM_RSQRT_PS(x)    __mm_rsqrt_nr_ps(x) // Implemented below
#endif

/** Performing the transpose of a 4x4 matrix of single precision floating
    point values.
    Arguments r0, r1, r2, and r3 are __m128 values whose elements
    form the corresponding rows of a 4x4 matrix.
    The matrix transpose is returned in arguments r0, r1, r2, and
    r3 where r0 now holds column 0 of the original matrix, r1 now
    holds column 1 of the original matrix, etc.
*/
#define __MM_TRANSPOSE4x4_PS(r0, r1, r2, r3)                                            \
    {                                                                                   \
        __m128 tmp3, tmp2, tmp1, tmp0;                                                  \
                                                                                        \
                                                            /* r00 r01 r02 r03 */       \
                                                            /* r10 r11 r12 r13 */       \
                                                            /* r20 r21 r22 r23 */       \
                                                            /* r30 r31 r32 r33 */       \
                                                                                        \
        tmp0 = _mm_unpacklo_ps(r0, r1);                       /* r00 r10 r01 r11 */     \
        tmp2 = _mm_unpackhi_ps(r0, r1);                       /* r02 r12 r03 r13 */     \
        tmp1 = _mm_unpacklo_ps(r2, r3);                       /* r20 r30 r21 r31 */     \
        tmp3 = _mm_unpackhi_ps(r2, r3);                       /* r22 r32 r23 r33 */     \
                                                                                        \
        r0 = _mm_movelh_ps(tmp0, tmp1);                         /* r00 r10 r20 r30 */   \
        r1 = _mm_movehl_ps(tmp1, tmp0);                         /* r01 r11 r21 r31 */   \
        r2 = _mm_movelh_ps(tmp2, tmp3);                         /* r02 r12 r22 r32 */   \
        r3 = _mm_movehl_ps(tmp3, tmp2);                         /* r03 r13 r23 r33 */   \
    }

/** Performing the transpose of a continuous stored rows of a 4x3 matrix to
    a 3x4 matrix of single precision floating point values.
    Arguments v0, v1, and v2 are __m128 values whose elements form the
    corresponding continuous stored rows of a 4x3 matrix.
    The matrix transpose is returned in arguments v0, v1, and v2, where
    v0 now holds column 0 of the original matrix, v1 now holds column 1
    of the original matrix, etc.
*/
#define __MM_TRANSPOSE4x3_PS(v0, v1, v2)                                                \
    {                                                                                   \
        __m128 tmp0, tmp1, tmp2;                                                        \
                                                                                        \
                                                            /* r00 r01 r02 r10 */       \
                                                            /* r11 r12 r20 r21 */       \
                                                            /* r22 r30 r31 r32 */       \
                                                                                        \
        tmp0 = _mm_shuffle_ps(v0, v2, _MM_SHUFFLE(3,0,3,0));  /* r00 r10 r22 r32 */     \
        tmp1 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1,0,2,1));  /* r01 r02 r11 r12 */     \
        tmp2 = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(2,1,3,2));  /* r20 r21 r30 r31 */     \
                                                                                        \
        v0 = _mm_shuffle_ps(tmp0, tmp2, _MM_SHUFFLE(2,0,1,0));  /* r00 r10 r20 r30 */   \
        v1 = _mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(3,1,2,0));  /* r01 r11 r21 r31 */   \
        v2 = _mm_shuffle_ps(tmp1, tmp0, _MM_SHUFFLE(3,2,3,1));  /* r02 r12 r22 r32 */   \
    }

/** Performing the transpose of a 3x4 matrix to a continuous stored rows of
    a 4x3 matrix of single precision floating point values.
    Arguments v0, v1, and v2 are __m128 values whose elements form the
    corresponding columns of a 3x4 matrix.
    The matrix transpose is returned in arguments v0, v1, and v2, as a
    continuous stored rows of a 4x3 matrix.
*/
#define __MM_TRANSPOSE3x4_PS(v0, v1, v2)                                            \
    {                                                                               \
        __m128 tmp0, tmp1, tmp2;                                                    \
                                                                                    \
                                                            /* r00 r10 r20 r30 */   \
                                                            /* r01 r11 r21 r31 */   \
                                                            /* r02 r12 r22 r32 */   \
                                                                                    \
        tmp0 = _mm_shuffle_ps(v0, v2, _MM_SHUFFLE(2,0,3,1));  /* r10 r30 r02 r22 */   \
        tmp1 = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(3,1,3,1));  /* r11 r31 r12 r32 */   \
        tmp2 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(2,0,2,0));  /* r00 r20 r01 r21 */   \
                                                                                    \
        v0 = _mm_shuffle_ps(tmp2, tmp0, _MM_SHUFFLE(0,2,2,0));  /* r00 r01 r02 r10 */   \
        v1 = _mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE(3,1,2,0));  /* r11 r12 r20 r21 */   \
        v2 = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(3,1,1,3));  /* r22 r30 r31 r32 */   \
    }

/** Fill vector of single precision floating point with selected value.
    Argument 'fp' is a digit[0123] that represents the fp of argument 'v'.
*/
#define __MM_SELECT(v, fp)                                                          \
    _mm_shuffle_ps((v), (v), _MM_SHUFFLE((fp),(fp),(fp),(fp)))

/// Accumulate four vector of single precision floating point values.
#define __MM_ACCUM4_PS(a, b, c, d)                                                  \
    _mm_add_ps(_mm_add_ps(a, b), _mm_add_ps(c, d))

/** Performing dot-product between two of four vector of single precision
    floating point values.
*/
#define __MM_DOT4x4_PS(a0, a1, a2, a3, b0, b1, b2, b3)                              \
    __MM_ACCUM4_PS(_mm_mul_ps(a0, b0), _mm_mul_ps(a1, b1), _mm_mul_ps(a2, b2), _mm_mul_ps(a3, b3))

/** Performing dot-product between four vector and three vector of single
    precision floating point values.
*/
#define __MM_DOT4x3_PS(r0, r1, r2, r3, v0, v1, v2)                                  \
    __MM_ACCUM4_PS(_mm_mul_ps(r0, v0), _mm_mul_ps(r1, v1), _mm_mul_ps(r2, v2), r3)

/// Accumulate three vector of single precision floating point values.
#define __MM_ACCUM3_PS(a, b, c)                                                     \
    _mm_add_ps(_mm_add_ps(a, b), c)

/** Performing dot-product between two of three vector of single precision
    floating point values.
*/
#define __MM_DOT3x3_PS(r0, r1, r2, v0, v1, v2)                                      \
    __MM_ACCUM3_PS(_mm_mul_ps(r0, v0), _mm_mul_ps(r1, v1), _mm_mul_ps(r2, v2))

/// Calculate multiply of two vector and plus another vector
#define __MM_MADD_PS(a, b, c)                                                       \
    _mm_add_ps(_mm_mul_ps(a, b), c)

/// Linear interpolation
#define __MM_LERP_PS(t, a, b)                                                       \
    __MM_MADD_PS(_mm_sub_ps(b, a), t, a)

/// Calculate multiply of two single floating value and plus another floating value
#define __MM_MADD_SS(a, b, c)                                                       \
    _mm_add_ss(_mm_mul_ss(a, b), c)

/// Linear interpolation
#define __MM_LERP_SS(t, a, b)                                                       \
    __MM_MADD_SS(_mm_sub_ss(b, a), t, a)

/// Same as _mm_load_ps, but can help VC generate more optimised code.
#define __MM_LOAD_PS(p)                                                             \
    (*(const __m128*)(p))

/// Same as _mm_store_ps, but can help VC generate more optimised code.
#define __MM_STORE_PS(p, v)                                                         \
    (*(__m128*)(p) = (v))


    /** Helper to load/store SSE data based on whether or not aligned.
    */
    template <bool aligned = false>
    struct SSEMemoryAccessor
    {
        static FORCEINLINE __m128 load(const float *p)
        {
            return _mm_loadu_ps(p);
        }
        static FORCEINLINE void store(float *p, const __m128& v)
        {
            _mm_storeu_ps(p, v);
        }
    };
    // Special aligned accessor
    template <>
    struct SSEMemoryAccessor<true>
    {
        static FORCEINLINE const __m128& load(const float *p)
        {
            return __MM_LOAD_PS(p);
        }
        static FORCEINLINE void store(float *p, const __m128& v)
        {
            __MM_STORE_PS(p, v);
        }
    };

    /** Check whether or not the given pointer perfect aligned for SSE.
    */
    static FORCEINLINE bool _isAlignedForSSE(const void *p)
    {
        return (((size_t)p) & 15) == 0;
    }

    /** Calculate NewtonRaphson Reciprocal Square Root with formula:
            0.5 * rsqrt(x) * (3 - x * rsqrt(x)^2)
    */
    static FORCEINLINE __m128 __mm_rsqrt_nr_ps(const __m128& x)
    {
        static const __m128 v0pt5 = { 0.5f, 0.5f, 0.5f, 0.5f };
        static const __m128 v3pt0 = { 3.0f, 3.0f, 3.0f, 3.0f };
        __m128 t = _mm_rsqrt_ps(x);
        return _mm_mul_ps(_mm_mul_ps(v0pt5, t),
            _mm_sub_ps(v3pt0, _mm_mul_ps(_mm_mul_ps(x, t), t)));
    }

// Macro to check the stack aligned for SSE
#if OGRE_DEBUG_MODE
#define __OGRE_CHECK_STACK_ALIGNED_FOR_SSE()        \
    {                                               \
        __m128 test;                                \
        assert(_isAlignedForSSE(&test));            \
    }

#else   // !OGRE_DEBUG_MODE
#define __OGRE_CHECK_STACK_ALIGNED_FOR_SSE()

#endif  // OGRE_DEBUG_MODE


#endif  // __OGRE_HAVE_SSE
	/** @} */
	/** @} */

}

#endif // __SIMDHelper_H__
