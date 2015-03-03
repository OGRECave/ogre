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

#include "OgreStableHeaders.h"

#include "OgreOptimisedUtil.h"
#include "OgrePlatformInformation.h"

#if __OGRE_HAVE_DIRECTXMATH

#include "OgreVector3.h"
#include "OgreMatrix4.h"

#include <directxmath.h>
using namespace DirectX;

namespace DirectX
{
	//------------------------------------------------------------------------------
	// 4D Vector; 8 bit char components
	__declspec(align(4)) struct XMCHAR4
	{
		char c[4];

		XMCHAR4() {}
		XMCHAR4(int32_t c0, int32_t c1, int32_t c2, int32_t c3) { c[0] = c0; c[1] = c1; c[2] = c2; c[3] = c3; }
		explicit XMCHAR4(_In_reads_(4) const char *pArray) { c[0] = pArray[0]; c[1] = pArray[1]; c[2] = pArray[2]; c[3] = pArray[3]; }

		XMCHAR4& operator= (const XMCHAR4& Char4) { c[0] = Char4.c[0]; c[1] = Char4.c[1]; c[2] = Char4.c[2]; c[3] = Char4.c[3]; return *this; }
	};

	inline XMCHAR4 __fastcall XMVector4GreaterByteMask
	(
		FXMVECTOR V1,
		FXMVECTOR V2
	)
	{
#if defined(_XM_NO_INTRINSICS_)
		XMCHAR4 byteMask( 
			(V1.vector4_f32[0] > V2.vector4_f32[0] ? 0xFF : 0),
			(V1.vector4_f32[1] > V2.vector4_f32[1] ? 0xFF : 0),
			(V1.vector4_f32[2] > V2.vector4_f32[2] ? 0xFF : 0),
			(V1.vector4_f32[3] > V2.vector4_f32[3] ? 0xFF : 0));
		return byteMask;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		uint32x4_t vResult = vcgtq_f32(V1, V2);
		int8x8x2_t vTemp = vzip_u8(vget_low_u8(vResult), vget_high_u8(vResult));
		vTemp = vzip_u8(vTemp.val[0], vTemp.val[1]);
		uint32_t r = vget_lane_u32(vTemp.val[1], 1);
		return *(XMCHAR4*)&r;	// ARM is little endian

#elif defined(_XM_SSE_INTRINSICS_)
		static const uint32_t bmLE[16] = {
			0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
			0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
			0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
			0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
		};

		XMVECTOR vTemp = _mm_cmpgt_ps(V1, V2);
		int iTest = _mm_movemask_ps(vTemp);
		uint32_t r = bmLE[iTest];
		return *(XMCHAR4*)&r;	// SSE is little endian

#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

}

// Use unrolled version when vertices exceed this limit
#define OGRE_DIRECTXMATH_SKINNING_UNROLL_VERTICES 16

namespace Ogre {

//-------------------------------------------------------------------------
// Local classes
//-------------------------------------------------------------------------

    /** General implementation of OptimisedUtil.
    @note
        Don't use this class directly, use OptimisedUtil instead.
    */
    class _OgrePrivate OptimisedUtilDirectXMath : public OptimisedUtil
    {
    public:
        /// @copydoc OptimisedUtil::softwareVertexSkinning
        virtual void softwareVertexSkinning(
            const float *srcPosPtr, float *destPosPtr,
            const float *srcNormPtr, float *destNormPtr,
            const float *blendWeightPtr, const unsigned char* blendIndexPtr,
            const Matrix4* const* blendMatrices,
            size_t srcPosStride, size_t destPosStride,
            size_t srcNormStride, size_t destNormStride,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numVertices);

        /// @copydoc OptimisedUtil::softwareVertexMorph
        virtual void softwareVertexMorph(
            Real t,
            const float *srcPos1, const float *srcPos2,
            float *dstPos,
            size_t pos1VSize, size_t pos2VSize, size_t dstVSize, 
            size_t numVertices,
            bool morphNormals);

        /// @copydoc OptimisedUtil::concatenateAffineMatrices
        virtual void concatenateAffineMatrices(
            const Matrix4& baseMatrix,
            const Matrix4* srcMatrices,
            Matrix4* dstMatrices,
            size_t numMatrices);

        /// @copydoc OptimisedUtil::calculateFaceNormals
        virtual void calculateFaceNormals(
            const float *positions,
            const v1::EdgeData::Triangle *triangles,
            Vector4 *faceNormals,
            size_t numTriangles);

        /// @copydoc OptimisedUtil::calculateLightFacing
        virtual void calculateLightFacing(
            const Vector4& lightPos,
            const Vector4* faceNormals,
            char* lightFacings,
            size_t numFaces);

        /// @copydoc OptimisedUtil::extrudeVertices
        virtual void extrudeVertices(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* srcPositions,
            float* destPositions,
            size_t numVertices);
    };

//---------------------------------------------------------------------
// DirectXMath helpers.
//---------------------------------------------------------------------
    
/** Check whether or not the given pointer perfect aligned for DirectXMath.
*/
static FORCEINLINE bool _isAlignedForDirectXMath(const void *p)
{
    return (((size_t)p) & 15) == 0;
}

/// Linear interpolation
#define __DX_LERP_PS(t, a, b)                                                       \
    XMVectorLerpV(a, b, t)

/// Linear interpolation.  Single value lerp is not supported in DirectXMath, fallback to __DX_LERP_PS.
#define __DX_LERP_SS(t, a, b)                                                       \
    __DX_LERP_PS(t, a, b)

#define __DX_LOAD_PS(p)                                                             \
    (*(XMVECTOR*)(p))

#define __DX_STORE_PS(p, v)                                                         \
    (*(XMVECTOR*)(p) = (v))

/// Accumulate three vector of single precision floating point values.
#define __DX_ACCUM3_PS(a, b, c)                                                     \
    XMVectorAdd(XMVectorAdd(a, b), c)

/// Accumulate four vector of single precision floating point values.
#define __DX_ACCUM4_PS(a, b, c, d)                                                  \
    XMVectorAdd(XMVectorAdd(a, b), XMVectorAdd(c, d))

/** Performing dot-product between two of three vector of single precision
    floating point values.
*/
#define __DX_DOT3x3_PS(r0, r1, r2, v0, v1, v2)                                      \
    __DX_ACCUM3_PS(XMVectorMultiply(r0, v0), XMVectorMultiply(r1, v1), XMVectorMultiply(r2, v2))

/** Performing dot-product between four vector and three vector of single
    precision floating point values.
*/
#define __DX_DOT4x3_PS(r0, r1, r2, r3, v0, v1, v2)                                  \
    __DX_ACCUM4_PS(XMVectorMultiply(r0, v0), XMVectorMultiply(r1, v1), XMVectorMultiply(r2, v2), r3)

/** Performing the transpose of a 4x4 matrix of single precision floating
    point values.
    Arguments r0, r1, r2, and r3 are XMVECTOR values whose elements
    form the corresponding rows of a 4x4 matrix.
    The matrix transpose is returned in arguments r0, r1, r2, and
    r3 where r0 now holds column 0 of the original matrix, r1 now
    holds column 1 of the original matrix, etc.
*/
#define __DX_TRANSPOSE4x4_PS(r0, r1, r2, r3)                                            \
    {                                                                                   \
        XMVECTOR tmp3, tmp2, tmp1, tmp0;                                                \
                                                                                        \
                                                            /* r00 r01 r02 r03 */       \
                                                            /* r10 r11 r12 r13 */       \
                                                            /* r20 r21 r22 r23 */       \
                                                            /* r30 r31 r32 r33 */       \
                                                                                        \
        tmp0 = XMVectorMergeXY(r0, r1);                       /* r00 r10 r01 r11 */     \
        tmp2 = XMVectorMergeZW(r0, r1);                       /* r02 r12 r03 r13 */     \
        tmp1 = XMVectorMergeXY(r2, r3);                       /* r20 r30 r21 r31 */     \
        tmp3 = XMVectorMergeZW(r2, r3);                       /* r22 r32 r23 r33 */     \
                                                                                        \
        r0 = XMVectorPermute<0, 1, 4, 5>(tmp0, tmp1);           /* r00 r10 r20 r30 */   \
        r1 = XMVectorPermute<6, 7, 2, 3>(tmp1, tmp0);           /* r01 r11 r21 r31 */   \
        r2 = XMVectorPermute<0, 1, 4, 5>(tmp2, tmp3);           /* r02 r12 r22 r32 */   \
        r3 = XMVectorPermute<6, 7, 2, 3>(tmp3, tmp2);           /* r03 r13 r23 r33 */   \
    }

/** Performing the transpose of a continuous stored rows of a 4x3 matrix to
    a 3x4 matrix of single precision floating point values.
    Arguments v0, v1, and v2 are XMVECTOR values whose elements form the
    corresponding continuous stored rows of a 4x3 matrix.
    The matrix transpose is returned in arguments v0, v1, and v2, where
    v0 now holds column 0 of the original matrix, v1 now holds column 1
    of the original matrix, etc.
*/
#define __DX_TRANSPOSE4x3_PS(v0, v1, v2)                                                \
    {                                                                                   \
        XMVECTOR tmp0, tmp1, tmp2;                                                      \
                                                                                        \
                                                            /* r00 r01 r02 r10 */       \
                                                            /* r11 r12 r20 r21 */       \
                                                            /* r22 r30 r31 r32 */       \
                                                                                        \
        tmp0 = XMVectorPermute<0, 3, 4, 7>(v0, v2);         /* r00 r10 r22 r32 */       \
        tmp1 = XMVectorPermute<1, 2, 4, 5>(v0, v1);         /* r01 r02 r11 r12 */       \
        tmp2 = XMVectorPermute<2, 3, 5, 6>(v1, v2);         /* r20 r21 r30 r31 */       \
                                                                                        \
        v0 = XMVectorPermute<0, 1, 4, 6>(tmp0, tmp2);       /* r00 r10 r20 r30 */       \
        v1 = XMVectorPermute<0, 2, 5, 7>(tmp1, tmp2);       /* r01 r11 r21 r31 */       \
        v2 = XMVectorPermute<1, 3, 6, 7>(tmp1, tmp0);       /* r02 r12 r22 r32 */       \
    }

/** Performing the transpose of a 3x4 matrix to a continuous stored rows of
    a 4x3 matrix of single precision floating point values.
    Arguments v0, v1, and v2 are XMVECTOR values whose elements form the
    corresponding columns of a 3x4 matrix.
    The matrix transpose is returned in arguments v0, v1, and v2, as a
    continuous stored rows of a 4x3 matrix.
*/
#define __DX_TRANSPOSE3x4_PS(v0, v1, v2)                                            \
    {                                                                               \
        XMVECTOR tmp0, tmp1, tmp2;                                                  \
                                                                                    \
                                                            /* r00 r10 r20 r30 */   \
                                                            /* r01 r11 r21 r31 */   \
                                                            /* r02 r12 r22 r32 */   \
                                                                                    \
        tmp0 = XMVectorPermute<1, 3, 4, 6>(v0, v2);         /* r10 r30 r02 r22 */   \
        tmp1 = XMVectorPermute<1, 3, 5, 7>(v1, v2);         /* r11 r31 r12 r32 */   \
        tmp2 = XMVectorPermute<0, 2, 4, 6>(v0, v1);         /* r00 r20 r01 r21 */   \
                                                                                    \
        v0 = XMVectorPermute<0, 2, 6, 4>(tmp2, tmp0);       /* r00 r01 r02 r10 */   \
        v1 = XMVectorPermute<0, 2, 5, 7>(tmp1, tmp2);       /* r11 r12 r20 r21 */   \
        v2 = XMVectorPermute<3, 1, 5, 7>(tmp0, tmp1);       /* r22 r30 r31 r32 */   \
    }

    /** Helper to load/store DirectXMath data based on whether or not aligned.
    */
    template <bool aligned = false>
    struct DirectXMathMemoryAccessor
    {
        static FORCEINLINE XMVECTOR load(const float *p)
        {
            return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(p));
        }
        static FORCEINLINE void store(float *p, const XMVECTOR& v)
        {
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(p), v);
        }
    };
    // Special aligned accessor
    template <>
    struct DirectXMathMemoryAccessor<true>
    {
        static FORCEINLINE const XMVECTOR load(const float *p)
        {
            return __DX_LOAD_PS(p);
        }
        static FORCEINLINE void store(float *p, const XMVECTOR& v)
        {
            __DX_STORE_PS(p, v);
        }
    };

//---------------------------------------------------------------------
// Some useful macro for collapse matrices.
//---------------------------------------------------------------------

#define __LOAD_MATRIX(row0, row1, row2, pMatrix)                        \
    {                                                                   \
        row0 = __DX_LOAD_PS((*pMatrix)[0]);                             \
        row1 = __DX_LOAD_PS((*pMatrix)[1]);                              \
        row2 = __DX_LOAD_PS((*pMatrix)[2]);                              \
    }

#define __LERP_MATRIX(row0, row1, row2, weight, pMatrix)                \
    {                                                                   \
        row0 = XMVectorLerpV(row0, __DX_LOAD_PS((*pMatrix)[0]), weight);\
        row1 = XMVectorLerpV(row1, __DX_LOAD_PS((*pMatrix)[1]), weight);\
        row2 = XMVectorLerpV(row2, __DX_LOAD_PS((*pMatrix)[2]), weight);\
    }

#define __LOAD_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix)       \
    {                                                                   \
        row0 = XMVectorMultiply(__DX_LOAD_PS((*pMatrix)[0]), weight);   \
        row1 = XMVectorMultiply(__DX_LOAD_PS((*pMatrix)[1]), weight);   \
        row2 = XMVectorMultiply(__DX_LOAD_PS((*pMatrix)[2]), weight);   \
    }

#define __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix)      \
    {                                                                   \
        row0 = XMVectorMultiplyAdd(__DX_LOAD_PS((*pMatrix)[0]), weight, row0); \
        row1 = XMVectorMultiplyAdd(__DX_LOAD_PS((*pMatrix)[1]), weight, row1); \
        row2 = XMVectorMultiplyAdd(__DX_LOAD_PS((*pMatrix)[2]), weight, row2); \
    }

//---------------------------------------------------------------------
// The following macros request variables declared by caller.
//
// :) Thank row-major matrix used in Ogre, it make we accessing affine matrix easy.
//---------------------------------------------------------------------

/** Collapse one-weighted matrix.
    Eliminated multiply by weight since the weight should be equal to one always
*/
#define __COLLAPSE_MATRIX_W1(row0, row1, row2, ppMatrices, pIndices, pWeights)  \
    {                                                                           \
        pMatrix0 = blendMatrices[pIndices[0]];                                  \
        __LOAD_MATRIX(row0, row1, row2, pMatrix0);                              \
    }

/** Collapse two-weighted matrix.
    Based on the fact that accumulated weights are equal to one, by use lerp,
    replaced two multiplies and one additive with one multiplie and two additives.
*/
#define __COLLAPSE_MATRIX_W2(row0, row1, row2, ppMatrices, pIndices, pWeights)  \
    {                                                                           \
        weight = XMVectorReplicatePtr(pWeights + 1);                            \
        pMatrix0 = ppMatrices[pIndices[0]];                                     \
        __LOAD_MATRIX(row0, row1, row2, pMatrix0);                              \
        pMatrix1 = ppMatrices[pIndices[1]];                                     \
        __LERP_MATRIX(row0, row1, row2, weight, pMatrix1);                      \
    }

/** Collapse three-weighted matrix.
*/
#define __COLLAPSE_MATRIX_W3(row0, row1, row2, ppMatrices, pIndices, pWeights)  \
    {                                                                           \
        weight = XMVectorReplicatePtr(pWeights + 0);                            \
        pMatrix0 = ppMatrices[pIndices[0]];                                     \
        __LOAD_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix0);             \
        weight = XMVectorReplicatePtr(pWeights + 1);                            \
        pMatrix1 = ppMatrices[pIndices[1]];                                     \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix1);            \
        weight = XMVectorReplicatePtr(pWeights + 2);                            \
        pMatrix2 = ppMatrices[pIndices[2]];                                     \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix2);            \
    }

/** Collapse four-weighted matrix.
*/
#define __COLLAPSE_MATRIX_W4(row0, row1, row2, ppMatrices, pIndices, pWeights)  \
    {                                                                           \
        /* Load four blend weights at one time, they will be shuffled later */  \
        weights = __DX_LOAD_PS(pWeights);                                       \
                                                                                \
        pMatrix0 = ppMatrices[pIndices[0]];                                     \
        weight = XMVectorSplatX(weights);                                       \
        __LOAD_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix0);             \
        pMatrix1 = ppMatrices[pIndices[1]];                                     \
        weight = XMVectorSplatY(weights);                                       \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix1);            \
        pMatrix2 = ppMatrices[pIndices[2]];                                     \
        weight = XMVectorSplatZ(weights);                                       \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix2);            \
        pMatrix3 = ppMatrices[pIndices[3]];                                     \
        weight = XMVectorSplatW(weights);                                       \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix3);            \
    }



    //---------------------------------------------------------------------
    // Collapse a matrix at one time. The collapsed matrix are weighted by
    // blend-weights, and then can use to transform corresponding vertex directly.
    //
    // I'd like use inline function instead of macro here, but I also want to
    // ensure compiler integrate this code into its callers (release build at
    // least), doesn't matter about specific compile options. Inline function
    // work fine for VC, but looks like gcc (3.4.4 here) generate function-call
    // when implemented as inline function, even if compile with "-O3" option.
    //
#define _collapseOneMatrix(                                                     \
        m00, m01, m02,                                                          \
        pBlendWeight, pBlendIndex,                                              \
        blendMatrices,                                                          \
        blendWeightStride, blendIndexStride,                                    \
        numWeightsPerVertex)                                                    \
    {                                                                           \
        /* Important Note: If reuse pMatrixXXX frequently, M$ VC7.1 will */     \
        /* generate wrong code here!!!                                   */     \
        const Matrix4* pMatrix0, *pMatrix1, *pMatrix2, *pMatrix3;               \
        XMVECTOR weight, weights;                                               \
                                                                                \
        switch (numWeightsPerVertex)                                            \
        {                                                                       \
        default:    /* Just in case and make compiler happy */                  \
        case 1:                                                                 \
            __COLLAPSE_MATRIX_W1(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            break;                                                              \
                                                                                \
        case 2:                                                                 \
            __COLLAPSE_MATRIX_W2(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            break;                                                              \
                                                                                \
        case 3:                                                                 \
            __COLLAPSE_MATRIX_W3(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            break;                                                              \
                                                                                \
        case 4:                                                                 \
            __COLLAPSE_MATRIX_W4(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            break;                                                              \
        }                                                                       \
    }

    //---------------------------------------------------------------------
    // Collapse four matrices at one time. The collapsed matrix are weighted by
    // blend-weights, and then can use to transform corresponding vertex directly.
    //
    // I'd like use inline function instead of macro here, but I also want to
    // ensure compiler integrate this code into its callers (release build at
    // least), doesn't matter about specific compile options. Inline function
    // work fine for VC, but looks like gcc (3.4.4 here) generate function-call
    // when implemented as inline function, even if compile with "-O3" option.
    //
#define _collapseFourMatrices(                                                  \
        m00, m01, m02,                                                          \
        m10, m11, m12,                                                          \
        m20, m21, m22,                                                          \
        m30, m31, m32,                                                          \
        pBlendWeight, pBlendIndex,                                              \
        blendMatrices,                                                          \
        blendWeightStride, blendIndexStride,                                    \
        numWeightsPerVertex)                                                    \
    {                                                                           \
        /* Important Note: If reuse pMatrixXXX frequently, M$ VC7.1 will */     \
        /* generate wrong code here!!!                                   */     \
        const Matrix4* pMatrix0, *pMatrix1, *pMatrix2, *pMatrix3;               \
        XMVECTOR weight, weights;                                                 \
                                                                                \
        switch (numWeightsPerVertex)                                            \
        {                                                                       \
        default:    /* Just in case and make compiler happy */                  \
        case 1:                                                                 \
            __COLLAPSE_MATRIX_W1(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W1(m10, m11, m12, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 1 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 1 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W1(m20, m21, m22, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 2 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 2 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W1(m30, m31, m32, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 3 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 3 * blendWeightStride));         \
            break;                                                              \
                                                                                \
        case 2:                                                                 \
            __COLLAPSE_MATRIX_W2(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W2(m10, m11, m12, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 1 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 1 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W2(m20, m21, m22, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 2 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 2 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W2(m30, m31, m32, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 3 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 3 * blendWeightStride));         \
            break;                                                              \
                                                                                \
        case 3:                                                                 \
            __COLLAPSE_MATRIX_W3(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W3(m10, m11, m12, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 1 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 1 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W3(m20, m21, m22, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 2 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 2 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W3(m30, m31, m32, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 3 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 3 * blendWeightStride));         \
            break;                                                              \
                                                                                \
        case 4:                                                                 \
            __COLLAPSE_MATRIX_W4(m00, m01, m02, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 0 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 0 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W4(m10, m11, m12, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 1 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 1 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W4(m20, m21, m22, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 2 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 2 * blendWeightStride));         \
            __COLLAPSE_MATRIX_W4(m30, m31, m32, blendMatrices,                  \
                rawOffsetPointer(pBlendIndex, 3 * blendIndexStride),            \
                rawOffsetPointer(pBlendWeight, 3 * blendWeightStride));         \
            break;                                                              \
        }                                                                       \
    }



    //---------------------------------------------------------------------
    // General DirectXMath version skinning positions, and optional skinning normals.
    static void softwareVertexSkinning_DirectXMath_General(
        const float *pSrcPos, float *pDestPos,
        const float *pSrcNorm, float *pDestNorm,
        const float *pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t srcPosStride, size_t destPosStride,
        size_t srcNormStride, size_t destNormStride,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numVertices)
    {
        for (size_t i = 0; i < numVertices; ++i)
        {
            // Collapse matrices
            XMVECTOR m00, m01, m02;
            _collapseOneMatrix(
                m00, m01, m02,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex);

            // Advance blend weight and index pointers
            advanceRawPointer(pBlendWeight, blendWeightStride);
            advanceRawPointer(pBlendIndex, blendIndexStride);

            //------------------------------------------------------------------

            XMVECTOR m03 = g_XMZero;
            __DX_TRANSPOSE4x4_PS(m00, m01, m02, m03);

            //------------------------------------------------------------------
            // Transform position
            //------------------------------------------------------------------

            XMVECTOR s0, s1, s2;

            // Load source position
            s0 = XMVectorReplicatePtr(pSrcPos + 0);
            s1 = XMVectorReplicatePtr(pSrcPos + 1);
            s2 = XMVectorReplicatePtr(pSrcPos + 2);

            // Transform by collapsed matrix
            XMVECTOR accumPos = __DX_DOT4x3_PS(m00, m01, m02, m03, s0, s1, s2);   // x y z 0

            // Store blended position, no aligned requirement
            XMStoreFloat3((XMFLOAT3*)(pDestPos + 0), accumPos);

            // Advance source and target position pointers
            advanceRawPointer(pSrcPos, srcPosStride);
            advanceRawPointer(pDestPos, destPosStride);

            //------------------------------------------------------------------
            // Optional blend normal
            //------------------------------------------------------------------

            if (pSrcNorm)
            {
                // Load source normal
                s0 = XMVectorReplicatePtr(pSrcNorm + 0);
                s1 = XMVectorReplicatePtr(pSrcNorm + 1);
                s2 = XMVectorReplicatePtr(pSrcNorm + 2);

                // Transform by collapsed matrix
                XMVECTOR accumNorm = __DX_DOT3x3_PS(m00, m01, m02, s0, s1, s2);   // x y z 0

                // Normalise normal
                accumNorm = XMVector3Normalize(accumNorm);

                // Store blended normal, no aligned requirement
                XMStoreFloat3((XMFLOAT3*)(pDestNorm + 0), accumNorm);

                // Advance source and target normal pointers
                advanceRawPointer(pSrcNorm, srcNormStride);
                advanceRawPointer(pDestNorm, destNormStride);
            }
        }
    }
    //---------------------------------------------------------------------
    // Special DirectXMath version skinning shared buffers of position and normal,
    // and the buffer are packed.
    template <bool srcAligned, bool destAligned>
    struct SoftwareVertexSkinning_DirectXMath_PosNorm_Shared_Packed
    {
        static void apply(
            const float* pSrc, float* pDest,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef DirectXMathMemoryAccessor<srcAligned> SrcAccessor;
            typedef DirectXMathMemoryAccessor<destAligned> DestAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                XMVECTOR m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
                _collapseFourMatrices(
                    m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22,
                    m30, m31, m32,
                    pBlendWeight, pBlendIndex,
                    blendMatrices,
                    blendWeightStride, blendIndexStride,
                    numWeightsPerVertex);

                // Advance 4 vertices
                advanceRawPointer(pBlendWeight, 4 * blendWeightStride);
                advanceRawPointer(pBlendIndex, 4 * blendIndexStride);

                //------------------------------------------------------------------
                // Transform position/normals
                //------------------------------------------------------------------

                XMVECTOR s0, s1, s2, s3, s4, s5, d0, d1, d2, d3, d4, d5;
                XMVECTOR t0, t1, t2, t3, t4, t5;

                // Load source position/normals
                s0 = SrcAccessor::load(pSrc + 0);                       // px0 py0 pz0 nx0
                s1 = SrcAccessor::load(pSrc + 4);                       // ny0 nz0 px1 py1
                s2 = SrcAccessor::load(pSrc + 8);                       // pz1 nx1 ny1 nz1
                s3 = SrcAccessor::load(pSrc + 12);                      // px2 py2 pz2 nx2
                s4 = SrcAccessor::load(pSrc + 16);                      // ny2 nz2 px3 py3
                s5 = SrcAccessor::load(pSrc + 20);                      // pz3 nx3 ny3 nz3

                // Rearrange to component-major for batches calculate.
                t0 = XMVectorMergeXY(s0, s3);                           // px0 px2 py0 py2
                t1 = XMVectorMergeZW(s0, s3);                           // pz0 pz2 nx0 nx2
                t2 = XMVectorMergeXY(s1, s4);                           // ny0 ny2 nz0 nz2
                t3 = XMVectorMergeZW(s1, s4);                           // px1 px3 py1 py3
                t4 = XMVectorMergeXY(s2, s5);                           // pz1 pz3 nx1 nx3
                t5 = XMVectorMergeZW(s2, s5);                           // ny1 ny3 nz1 nz3

                s0 = XMVectorMergeXY(t0, t3);                           // px0 px1 px2 px3
                s1 = XMVectorMergeZW(t0, t3);                           // py0 py1 py2 py3
                s2 = XMVectorMergeXY(t1, t4);                           // pz0 pz1 pz2 pz3
                s3 = XMVectorMergeZW(t1, t4);                           // nx0 nx1 nx2 nx3
                s4 = XMVectorMergeXY(t2, t5);                           // ny0 ny1 ny2 ny3
                s5 = XMVectorMergeZW(t2, t5);                           // nz0 nz1 nz2 nz3

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __DX_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __DX_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // PX0 PX1 PX2 PX3
                d3 = __DX_DOT3x3_PS(m00, m10, m20, s3, s4, s5);         // NX0 NX1 NX2 NX3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __DX_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __DX_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // PY0 PY1 PY2 PY3
                d4 = __DX_DOT3x3_PS(m01, m11, m21, s3, s4, s5);         // NY0 NY1 NY2 NY3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __DX_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __DX_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // PZ0 PZ1 PZ2 PZ3
                d5 = __DX_DOT3x3_PS(m02, m12, m22, s3, s4, s5);         // NZ0 NZ1 NZ2 NZ3

                // Normalise normals
                XMVECTOR tmp = __DX_DOT3x3_PS(d3, d4, d5, d3, d4, d5);
                tmp = XMVectorReciprocalSqrtEst(tmp);
                d3 = XMVectorMultiply(d3, tmp);
                d4 = XMVectorMultiply(d4, tmp);
                d5 = XMVectorMultiply(d5, tmp);

                // Arrange back to continuous format for store results

                t0 = XMVectorMergeXY(d0, d1); // PX0 PY0 PX1 PY1
                t1 = XMVectorMergeZW(d0, d1); // PX2 PY2 PX3 PY3
                t2 = XMVectorMergeXY(d2, d3); // PZ0 NX0 PZ1 NX1
                t3 = XMVectorMergeZW(d2, d3); // PZ2 NX2 PZ3 NX3
                t4 = XMVectorMergeXY(d4, d5); // NY0 NZ0 NY1 NZ1
                t5 = XMVectorMergeZW(d4, d5); // NY2 NZ2 NY3 NZ3

                d0 = XMVectorPermute<0, 1, 4, 5>(t0, t2); // PX0 PY0 PZ0 NX0
                d1 = XMVectorPermute<0, 1, 6, 7>(t4, t0); // NY0 NZ0 PX1 PY1
                d2 = XMVectorPermute<6, 7, 2, 3>(t4, t2); // PZ1 NX1 NY1 NZ1
                d3 = XMVectorPermute<0, 1, 4, 5>(t1, t3); // PX2 PY2 PZ2 NX2
                d4 = XMVectorPermute<0, 1, 6, 7>(t5, t1); // NY2 NZ2 PX3 PY3
                d5 = XMVectorPermute<6, 7, 2, 3>(t5, t3); // PZ3 NX3 NY3 NZ3

                // Store blended position/normals
                DestAccessor::store(pDest + 0, d0);
                DestAccessor::store(pDest + 4, d1);
                DestAccessor::store(pDest + 8, d2);
                DestAccessor::store(pDest + 12, d3);
                DestAccessor::store(pDest + 16, d4);
                DestAccessor::store(pDest + 20, d5);

                // Advance 4 vertices
                pSrc += 4 * (3 + 3);
                pDest += 4 * (3 + 3);
            }
        }
    };
    static FORCEINLINE void softwareVertexSkinning_DirectXMath_PosNorm_Shared_Packed(
            const float* pSrcPos, float* pDestPos,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
    {
        // pSrcPos might can't align to 16 bytes because 8 bytes alignment shift per-vertex

        // Instantiating two version only, since other alignment combinations are not that important.
        if (_isAlignedForDirectXMath(pSrcPos) && _isAlignedForDirectXMath(pDestPos))
        {
            SoftwareVertexSkinning_DirectXMath_PosNorm_Shared_Packed<true, true>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_DirectXMath_PosNorm_Shared_Packed<false, false>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    // Special DirectXMath version skinning separated buffers of position and normal,
    // both of position and normal buffer are packed.
    template <bool srcPosAligned, bool destPosAligned, bool srcNormAligned, bool destNormAligned>
    struct SoftwareVertexSkinning_DirectXMath_PosNorm_Separated_Packed
    {
        static void apply(
            const float* pSrcPos, float* pDestPos,
            const float* pSrcNorm, float* pDestNorm,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef DirectXMathMemoryAccessor<srcPosAligned> SrcPosAccessor;
            typedef DirectXMathMemoryAccessor<destPosAligned> DestPosAccessor;
            typedef DirectXMathMemoryAccessor<srcNormAligned> SrcNormAccessor;
            typedef DirectXMathMemoryAccessor<destNormAligned> DestNormAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                XMVECTOR m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
                _collapseFourMatrices(
                    m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22,
                    m30, m31, m32,
                    pBlendWeight, pBlendIndex,
                    blendMatrices,
                    blendWeightStride, blendIndexStride,
                    numWeightsPerVertex);

                // Advance 4 vertices
                advanceRawPointer(pBlendWeight, 4 * blendWeightStride);
                advanceRawPointer(pBlendIndex, 4 * blendIndexStride);

                //------------------------------------------------------------------
                // Transform positions
                //------------------------------------------------------------------

                XMVECTOR s0, s1, s2, d0, d1, d2;

                // Load source positions
                s0 = SrcPosAccessor::load(pSrcPos + 0);                 // x0 y0 z0 x1
                s1 = SrcPosAccessor::load(pSrcPos + 4);                 // y1 z1 x2 y2
                s2 = SrcPosAccessor::load(pSrcPos + 8);                 // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __DX_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __DX_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __DX_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // X0 X1 X2 X3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __DX_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __DX_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // Y0 Y1 Y2 Y3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __DX_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __DX_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // Z0 Z1 Z2 Z3

                // Arrange back to 4x3 continuous format for store results
                __DX_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store blended positions
                DestPosAccessor::store(pDestPos + 0, d0);
                DestPosAccessor::store(pDestPos + 4, d1);
                DestPosAccessor::store(pDestPos + 8, d2);

                // Advance 4 vertices
                pSrcPos += 4 * 3;
                pDestPos += 4 * 3;

                //------------------------------------------------------------------
                // Transform normals
                //------------------------------------------------------------------

                // Load source normals
                s0 = SrcNormAccessor::load(pSrcNorm + 0);               // x0 y0 z0 x1
                s1 = SrcNormAccessor::load(pSrcNorm + 4);               // y1 z1 x2 y2
                s2 = SrcNormAccessor::load(pSrcNorm + 8);               // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __DX_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed and shuffled matrices
                d0 = __DX_DOT3x3_PS(m00, m10, m20, s0, s1, s2);         // X0 X1 X2 X3
                d1 = __DX_DOT3x3_PS(m01, m11, m21, s0, s1, s2);         // Y0 Y1 Y2 Y3
                d2 = __DX_DOT3x3_PS(m02, m12, m22, s0, s1, s2);         // Z0 Z1 Z2 Z3

                // Normalise normals
                XMVECTOR tmp = __DX_DOT3x3_PS(d0, d1, d2, d0, d1, d2);
                tmp = XMVectorReciprocalSqrtEst(tmp);
                d0 = XMVectorMultiply(d0, tmp);
                d1 = XMVectorMultiply(d1, tmp);
                d2 = XMVectorMultiply(d2, tmp);

                // Arrange back to 4x3 continuous format for store results
                __DX_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store blended normals
                DestNormAccessor::store(pDestNorm + 0, d0);
                DestNormAccessor::store(pDestNorm + 4, d1);
                DestNormAccessor::store(pDestNorm + 8, d2);

                // Advance 4 vertices
                pSrcNorm += 4 * 3;
                pDestNorm += 4 * 3;
            }
        }
    };
    static FORCEINLINE void softwareVertexSkinning_DirectXMath_PosNorm_Separated_Packed(
        const float* pSrcPos, float* pDestPos,
        const float* pSrcNorm, float* pDestNorm,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numIterations)
    {
        assert(_isAlignedForDirectXMath(pSrcPos));

        // Instantiating two version only, since other alignement combination not that important.
        if (_isAlignedForDirectXMath(pSrcNorm) && _isAlignedForDirectXMath(pDestPos) && _isAlignedForDirectXMath(pDestNorm))
        {
            SoftwareVertexSkinning_DirectXMath_PosNorm_Separated_Packed<true, true, true, true>::apply(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_DirectXMath_PosNorm_Separated_Packed<true, false, false, false>::apply(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    // Special DirectXMath version skinning position only, the position buffer are
    // packed.
    template <bool srcPosAligned, bool destPosAligned>
    struct SoftwareVertexSkinning_DirectXMath_PosOnly_Packed
    {
        static void apply(
            const float* pSrcPos, float* pDestPos,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef DirectXMathMemoryAccessor<srcPosAligned> SrcPosAccessor;
            typedef DirectXMathMemoryAccessor<destPosAligned> DestPosAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                XMVECTOR m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
                _collapseFourMatrices(
                    m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22,
                    m30, m31, m32,
                    pBlendWeight, pBlendIndex,
                    blendMatrices,
                    blendWeightStride, blendIndexStride,
                    numWeightsPerVertex);

                // Advance 4 vertices
                advanceRawPointer(pBlendWeight, 4 * blendWeightStride);
                advanceRawPointer(pBlendIndex, 4 * blendIndexStride);

                //------------------------------------------------------------------
                // Transform positions
                //------------------------------------------------------------------

                XMVECTOR s0, s1, s2, d0, d1, d2;

                // Load source positions
                s0 = SrcPosAccessor::load(pSrcPos + 0);                 // x0 y0 z0 x1
                s1 = SrcPosAccessor::load(pSrcPos + 4);                 // y1 z1 x2 y2
                s2 = SrcPosAccessor::load(pSrcPos + 8);                 // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __DX_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __DX_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __DX_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // X0 X1 X2 X3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __DX_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __DX_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // Y0 Y1 Y2 Y3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __DX_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __DX_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // Z0 Z1 Z2 Z3

                // Arrange back to 4x3 continuous format for store results
                __DX_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store blended positions
                DestPosAccessor::store(pDestPos + 0, d0);
                DestPosAccessor::store(pDestPos + 4, d1);
                DestPosAccessor::store(pDestPos + 8, d2);

                // Advance 4 vertices
                pSrcPos += 4 * 3;
                pDestPos += 4 * 3;
            }
        }
    };
    static FORCEINLINE void softwareVertexSkinning_DirectXMath_PosOnly_Packed(
        const float* pSrcPos, float* pDestPos,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numIterations)
    {
        assert(_isAlignedForDirectXMath(pSrcPos));

        // Instantiating two version only, since other alignement combination not that important.
        if (_isAlignedForDirectXMath(pDestPos))
        {
            SoftwareVertexSkinning_DirectXMath_PosOnly_Packed<true, true>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_DirectXMath_PosOnly_Packed<true, false>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilDirectXMath::softwareVertexSkinning(
        const float *pSrcPos, float *pDestPos,
        const float *pSrcNorm, float *pDestNorm,
        const float *pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t srcPosStride, size_t destPosStride,
        size_t srcNormStride, size_t destNormStride,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numVertices)
    {
        // All position/normal pointers should be perfect aligned, but still check here
        // for avoid hardware buffer which allocated by potential buggy driver doesn't
        // support alignment properly.
        // Because we are used meta-function technique here, the code is easy to maintenance
        // and still provides all possible alignment combination.
        //

        // Use unrolled routines only if there a lot of vertices
        if (numVertices > OGRE_DIRECTXMATH_SKINNING_UNROLL_VERTICES)
        {
            if (pSrcNorm)
            {
                // Blend position and normal

                if (srcPosStride == sizeof(float) * (3 + 3) && destPosStride == sizeof(float) * (3 + 3) &&
                    pSrcNorm == pSrcPos + 3 && pDestNorm == pDestPos + 3)
                {
                    // Position and normal are sharing with packed buffer

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign == 8)   // Because 8 bytes alignment shift per-vertex
                    {
                        size_t count = srcPosAlign / 8;
                        numVertices -= count;
                        softwareVertexSkinning_DirectXMath_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * (3 + 3);
                        pDestPos += count * (3 + 3);
                        pSrcNorm += count * (3 + 3);
                        pDestNorm += count * (3 + 3);
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_DirectXMath_PosNorm_Shared_Packed(
                        pSrcPos, pDestPos,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * (3 + 3);
                        pDestPos += numIterations * 4 * (3 + 3);
                        pSrcNorm += numIterations * 4 * (3 + 3);
                        pDestNorm += numIterations * 4 * (3 + 3);
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else if (srcPosStride == sizeof(float) * 3 && destPosStride == sizeof(float) * 3 &&
                         srcNormStride == sizeof(float) * 3 && destNormStride == sizeof(float) * 3)
                {
                    // Position and normal are separate buffers, and all of them are packed

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign)
                    {
                        size_t count = srcPosAlign / 4;
                        numVertices -= count;
                        softwareVertexSkinning_DirectXMath_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * 3;
                        pDestPos += count * 3;
                        pSrcNorm += count * 3;
                        pDestNorm += count * 3;
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_DirectXMath_PosNorm_Separated_Packed(
                        pSrcPos, pDestPos,
                        pSrcNorm, pDestNorm,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * 3;
                        pDestPos += numIterations * 4 * 3;
                        pSrcNorm += numIterations * 4 * 3;
                        pDestNorm += numIterations * 4 * 3;
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else    // Not 'packed' form or wrong order between position and normal
                {
                    // Should never occur, do nothing here just in case
                }
            }
            else    // !pSrcNorm
            {
                // Blend position only

                if (srcPosStride == sizeof(float) * 3 && destPosStride == sizeof(float) * 3)
                {
                    // All buffers are packed

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign)
                    {
                        size_t count = srcPosAlign / 4;
                        numVertices -= count;
                        softwareVertexSkinning_DirectXMath_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * 3;
                        pDestPos += count * 3;
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_DirectXMath_PosOnly_Packed(
                        pSrcPos, pDestPos,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * 3;
                        pDestPos += numIterations * 4 * 3;
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else    // Not 'packed' form
                {
                    // Might occur only if user forced software blending position only
                }
            }
        }

        // Blend remaining vertices, need to do it with SIMD for identical result,
        // since mixing general floating-point and SIMD algorithm will causing
        // floating-point error.
        if (numVertices)
        {
            softwareVertexSkinning_DirectXMath_General(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                srcPosStride, destPosStride,
                srcNormStride, destNormStride,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numVertices);
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilDirectXMath::softwareVertexMorph(
        Real t,
        const float *pSrc1, const float *pSrc2,
        float *pDst,
        size_t pos1VSize, size_t pos2VSize, size_t dstVSize, 
        size_t numVertices,
        bool morphNormals)
    {   
        XMVECTOR src01, src02, src11, src12, src21, src22;
        XMVECTOR dst0, dst1, dst2;

        XMVECTOR t4 = XMVectorReplicate(t);


        // If we're morphing normals, we have twice the number of floats to process
        // Positions are interleaved with normals, so we'll have to separately
        // normalise just the normals later; we'll just lerp in the first pass
        // We can't normalise as we go because normals & positions are only 3 floats
        // each so are not aligned for DirectXMath, we'd mix the data up
        size_t normalsMultiplier = morphNormals ? 2 : 1;
        size_t numIterations = (numVertices*normalsMultiplier) / 4;
        size_t numVerticesRemainder = (numVertices*normalsMultiplier) & 3;
        
        // Save for later
        float *pStartDst = pDst;
                        
        // Never use meta-function technique to accessing memory because looks like
        // VC7.1 generate a bit inefficient binary code when put following code into
        // inline function.

        if (_isAlignedForDirectXMath(pSrc1) && _isAlignedForDirectXMath(pSrc2) && _isAlignedForDirectXMath(pDst))
        {
            // All data aligned

            // Morph 4 vertices per-iteration. Special designed for use all
            // available CPU registers as possible (7 registers used here),
            // and avoid temporary values allocated in stack for suppress
            // extra memory access.
            for (size_t i = 0; i < numIterations; ++i)
            {
                // 12 floating-point values
                src01 = __DX_LOAD_PS(pSrc1 + 0);
                src02 = __DX_LOAD_PS(pSrc2 + 0);
                src11 = __DX_LOAD_PS(pSrc1 + 4);
                src12 = __DX_LOAD_PS(pSrc2 + 4);
                src21 = __DX_LOAD_PS(pSrc1 + 8);
                src22 = __DX_LOAD_PS(pSrc2 + 8);
                pSrc1 += 12; pSrc2 += 12;

                dst0 = __DX_LERP_PS(t4, src01, src02);
                dst1 = __DX_LERP_PS(t4, src11, src12);
                dst2 = __DX_LERP_PS(t4, src21, src22);

                __DX_STORE_PS(pDst + 0, dst0);
                __DX_STORE_PS(pDst + 4, dst1);
                __DX_STORE_PS(pDst + 8, dst2);
                pDst += 12;
            }

            // Morph remaining vertices
            switch (numVerticesRemainder)
            {
            case 3:
                // 9 floating-point values
                src01 = __DX_LOAD_PS(pSrc1 + 0);
                src02 = __DX_LOAD_PS(pSrc2 + 0);
                src11 = __DX_LOAD_PS(pSrc1 + 4);
                src12 = __DX_LOAD_PS(pSrc2 + 4);
                src21 = XMLoadFloat(pSrc1 + 8);
                src22 = XMLoadFloat(pSrc2 + 8);

                dst0 = __DX_LERP_PS(t4, src01, src02);
                dst1 = __DX_LERP_PS(t4, src11, src12);
                dst2 = __DX_LERP_SS(t4, src21, src22);

                __DX_STORE_PS(pDst + 0, dst0);
                __DX_STORE_PS(pDst + 4, dst1);
                XMStoreFloat(pDst + 8, dst2);
                break;

            case 2:
                // 6 floating-point values
                src01 = __DX_LOAD_PS(pSrc1 + 0);
                src02 = __DX_LOAD_PS(pSrc2 + 0);
                src11 = XMLoadFloat2((XMFLOAT2*)(pSrc1 + 4));
                src12 = XMLoadFloat2((XMFLOAT2*)(pSrc2 + 4));

                dst0 = __DX_LERP_PS(t4, src01, src02);
                dst1 = __DX_LERP_PS(t4, src11, src12);

                __DX_STORE_PS(pDst + 0, dst0);
                XMStoreFloat2((XMFLOAT2*)(pDst + 4), dst1);
                break;

            case 1:
                // 3 floating-point values
                src01 = XMLoadFloat3((XMFLOAT3*)(pSrc1 + 0));
                src02 = XMLoadFloat3((XMFLOAT3*)(pSrc2 + 0));

                dst0 = __DX_LERP_PS(t4, src01, src02);

                XMStoreFloat3((XMFLOAT3*)(pDst + 0), dst0);
                break;
            }
        }
        else    // Should never occur, just in case of buggy drivers
        {
            // Assume all data unaligned

            // Morph 4 vertices per-iteration. Special designed for use all
            // available CPU registers as possible (7 registers used here),
            // and avoid temporary values allocated in stack for suppress
            // extra memory access.
            for (size_t i = 0; i < numIterations; ++i)
            {
                // 12 floating-point values
                src01 = XMLoadFloat4((XMFLOAT4*)(pSrc1 + 0));
                src02 = XMLoadFloat4((XMFLOAT4*)(pSrc2 + 0));
                src11 = XMLoadFloat4((XMFLOAT4*)(pSrc1 + 4));
                src12 = XMLoadFloat4((XMFLOAT4*)(pSrc2 + 4));
                src21 = XMLoadFloat4((XMFLOAT4*)(pSrc1 + 8));
                src22 = XMLoadFloat4((XMFLOAT4*)(pSrc2 + 8));
                pSrc1 += 12; pSrc2 += 12;

                dst0 = __DX_LERP_PS(t4, src01, src02);
                dst1 = __DX_LERP_PS(t4, src11, src12);
                dst2 = __DX_LERP_PS(t4, src21, src22);

                XMStoreFloat4((XMFLOAT4*)(pDst + 0), dst0);
                XMStoreFloat4((XMFLOAT4*)(pDst + 4), dst1);
                XMStoreFloat4((XMFLOAT4*)(pDst + 8), dst2);
                pDst += 12;
            }

            // Morph remaining vertices
            switch (numVerticesRemainder)
            {
            case 3:
                // 9 floating-point values
                src01 = XMLoadFloat4((XMFLOAT4*)(pSrc1 + 0));
                src02 = XMLoadFloat4((XMFLOAT4*)(pSrc2 + 0));
                src11 = XMLoadFloat4((XMFLOAT4*)(pSrc1 + 4));
                src12 = XMLoadFloat4((XMFLOAT4*)(pSrc2 + 4));
                src21 = XMLoadFloat(pSrc1 + 8);
                src22 = XMLoadFloat(pSrc2 + 8);

                dst0 = __DX_LERP_PS(t4, src01, src02);
                dst1 = __DX_LERP_PS(t4, src11, src12);
                dst2 = __DX_LERP_SS(t4, src21, src22);

                XMStoreFloat4((XMFLOAT4*)(pDst + 0), dst0);
                XMStoreFloat4((XMFLOAT4*)(pDst + 4), dst1);
                XMStoreFloat(pDst + 8, dst2);
                break;

            case 2:
                // 6 floating-point values
                src01 = XMLoadFloat4((XMFLOAT4*)(pSrc1 + 0));
                src02 = XMLoadFloat4((XMFLOAT4*)(pSrc2 + 0));
                src11 = XMLoadFloat2((XMFLOAT2*)(pSrc1 + 4));
                src12 = XMLoadFloat2((XMFLOAT2*)(pSrc2 + 4));

                dst0 = __DX_LERP_PS(t4, src01, src02);
                dst1 = __DX_LERP_PS(t4, src11, src12);

                XMStoreFloat4((XMFLOAT4*)(pDst + 0), dst0);
                XMStoreFloat2((XMFLOAT2*)(pDst + 4), dst1);
                break;

            case 1:
                // 3 floating-point values
                src01 = XMLoadFloat3((XMFLOAT3*)(pSrc1 + 0));
                src02 = XMLoadFloat3((XMFLOAT3*)(pSrc2 + 0));

                dst0 = __DX_LERP_PS(t4, src01, src02);

                XMStoreFloat3((XMFLOAT3*)(pDst + 0), dst0);
                break;
            }
            
        }
        
        if (morphNormals)
        {
            
            // Now we need to do and unaligned normalise on the normals data we just
            // lerped; because normals are 3 elements each they're always unaligned
            float *pNorm = pStartDst;
            
            // Offset past first position
            pNorm += 3;
            
            // We'll do one normal each iteration, but still use DirectXMath
            for (size_t n = 0; n < numVertices; ++n)
            {
                // normalise function
                XMVECTOR norm;
                
                // load 3 floating-point normal values
                norm = XMLoadFloat3((XMFLOAT3*)pNorm);
                norm = XMVector3Normalize(norm);
                
                // Store back in the same place
                XMStoreFloat3((XMFLOAT3*)pNorm, norm);
                
                // Skip to next vertex (3x normal components, 3x position components)
                pNorm += 6;
            }
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilDirectXMath::concatenateAffineMatrices(
        const Matrix4& baseMatrix,
        const Matrix4* pSrcMat,
        Matrix4* pDstMat,
        size_t numMatrices)
    {
        assert(_isAlignedForDirectXMath(pSrcMat));
        assert(_isAlignedForDirectXMath(pDstMat));

        // Load base matrix, unaligned
        XMVECTOR m0 = XMLoadFloat4((XMFLOAT4*)baseMatrix[0]);
        XMVECTOR m1 = XMLoadFloat4((XMFLOAT4*)baseMatrix[1]);
        XMVECTOR m2 = XMLoadFloat4((XMFLOAT4*)baseMatrix[2]);
        XMVECTOR m3 = XMLoadFloat4((XMFLOAT4*)baseMatrix[3]);        // m3 should be equal to (0, 0, 0, 1)

        for (size_t i = 0; i < numMatrices; ++i)
        {
            // Load source matrix, aligned
            XMVECTOR s0 = __DX_LOAD_PS((*pSrcMat)[0]);
            XMVECTOR s1 = __DX_LOAD_PS((*pSrcMat)[1]);
            XMVECTOR s2 = __DX_LOAD_PS((*pSrcMat)[2]);

            ++pSrcMat;

            XMVECTOR t0, t1, t2, t3;

            // Concatenate matrix, and store results

            // Row 0
            t0 = XMVectorMultiply(XMVectorSplatX(m0), s0);
            t1 = XMVectorMultiply(XMVectorSplatY(m0), s1);
            t2 = XMVectorMultiply(XMVectorSplatZ(m0), s2);
            t3 = XMVectorMultiply(m0, m3);    // Compiler should optimise this out of the loop
            __DX_STORE_PS((*pDstMat)[0], __DX_ACCUM4_PS(t0,t1,t2,t3));

            // Row 1
            t0 = XMVectorMultiply(XMVectorSplatX(m1), s0);
            t1 = XMVectorMultiply(XMVectorSplatY(m1), s1);
            t2 = XMVectorMultiply(XMVectorSplatZ(m1), s2);
            t3 = XMVectorMultiply(m1, m3);    // Compiler should optimise this out of the loop
            __DX_STORE_PS((*pDstMat)[1], __DX_ACCUM4_PS(t0,t1,t2,t3));

            // Row 2
            t0 = XMVectorMultiply(XMVectorSplatX(m2), s0);
            t1 = XMVectorMultiply(XMVectorSplatY(m2), s1);
            t2 = XMVectorMultiply(XMVectorSplatZ(m2), s2);
            t3 = XMVectorMultiply(m2, m3);    // Compiler should optimise this out of the loop
            __DX_STORE_PS((*pDstMat)[2], __DX_ACCUM4_PS(t0,t1,t2,t3));

            // Row 3
            __DX_STORE_PS((*pDstMat)[3], m3);

            ++pDstMat;
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilDirectXMath::calculateFaceNormals(
        const float *positions,
        const v1::EdgeData::Triangle *triangles,
        Vector4 *faceNormals,
        size_t numTriangles)
    {
        assert(_isAlignedForDirectXMath(faceNormals));

        size_t numIterations = numTriangles / 4;
        numTriangles &= 3;

        // Four triangles per-iteration
        for (size_t i = 0; i < numIterations; ++i)
        {

// Load four Vector3 as: (x0, x1, x2, x3), (y0, y1, y2, y3), (z0, z1, z2, z3)
#define __LOAD_FOUR_VECTOR3(x, y, z, p0, p1, p2, p3)                            \
            {                                                                   \
                XMVECTOR v0 = XMLoadFloat3((XMFLOAT3*)(p0)); /* x0 y0 z0 -- */  \
                XMVECTOR v1 = XMLoadFloat3((XMFLOAT3*)(p1)); /* x1 y1 z1 -- */  \
                XMVECTOR v2 = XMLoadFloat3((XMFLOAT3*)(p2)); /* x2 y2 z2 -- */  \
                XMVECTOR v3 = XMLoadFloat3((XMFLOAT3*)(p3)); /* x3 y3 z3 -- */  \
                XMVECTOR t0, t1;                                                \
                                                                                \
                t0 = XMVectorMergeXY(v0, v2);       /* x0 x2 y0 y2 */           \
                t1 = XMVectorMergeXY(v1, v3);       /* x1 x3 y1 y3 */           \
                x = XMVectorMergeXY(t0, t1);        /* x0 x1 x2 x3 */           \
                y = XMVectorMergeZW(t0, t1);        /* y0 y1 y2 y3 */           \
                                                                                \
                t0 = XMVectorMergeZW(v0, v2);       /* z0 z2 -- -- */           \
                t1 = XMVectorMergeZW(v1, v2);       /* z1 z3 -- -- */           \
                z = XMVectorMergeXY(t0, t1);        /* z0 z1 z2 z3 */           \
            }

            XMVECTOR x0, x1, x2, y0, y1, y2, z0, z1, z2;

            // Load vertex 0 of four triangles, packed as component-major format: xxxx yyyy zzzz
            __LOAD_FOUR_VECTOR3(x0, y0, z0,
                positions + triangles[0].vertIndex[0] * 3,
                positions + triangles[1].vertIndex[0] * 3,
                positions + triangles[2].vertIndex[0] * 3,
                positions + triangles[3].vertIndex[0] * 3);

            // Load vertex 1 of four triangles, packed as component-major format: xxxx yyyy zzzz
            __LOAD_FOUR_VECTOR3(x1, y1, z1,
                positions + triangles[0].vertIndex[1] * 3,
                positions + triangles[1].vertIndex[1] * 3,
                positions + triangles[2].vertIndex[1] * 3,
                positions + triangles[3].vertIndex[1] * 3);

            // Load vertex 2 of four triangles, packed as component-major format: xxxx yyyy zzzz
            __LOAD_FOUR_VECTOR3(x2, y2, z2,
                positions + triangles[0].vertIndex[2] * 3,
                positions + triangles[1].vertIndex[2] * 3,
                positions + triangles[2].vertIndex[2] * 3,
                positions + triangles[3].vertIndex[2] * 3);

            triangles += 4;

            // Calculate triangle face normals

            // a = v1 - v0
            XMVECTOR ax = XMVectorSubtract(x1, x0);
            XMVECTOR ay = XMVectorSubtract(y1, y0);
            XMVECTOR az = XMVectorSubtract(z1, z0);

            // b = v2 - v0
            XMVECTOR bx = XMVectorSubtract(x2, x0);
            XMVECTOR by = XMVectorSubtract(y2, y0);
            XMVECTOR bz = XMVectorSubtract(z2, z0);

            // n = a cross b
            XMVECTOR nx = XMVectorSubtract(XMVectorMultiply(ay, bz), XMVectorMultiply(az, by));
            XMVECTOR ny = XMVectorSubtract(XMVectorMultiply(az, bx), XMVectorMultiply(ax, bz));
            XMVECTOR nz = XMVectorSubtract(XMVectorMultiply(ax, by), XMVectorMultiply(ay, bx));

            // w = - (n dot v0)
            XMVECTOR nw = XMVectorNegate(__DX_DOT3x3_PS(nx, ny, nz, x0, y0, z0));

            // Arrange to per-triangle face normal major format
            __DX_TRANSPOSE4x4_PS(nx, ny, nz, nw);

            // Store results
            __DX_STORE_PS(&faceNormals[0].x, nx);
            __DX_STORE_PS(&faceNormals[1].x, ny);
            __DX_STORE_PS(&faceNormals[2].x, nz);
            __DX_STORE_PS(&faceNormals[3].x, nw);
            faceNormals += 4;
        }

        // Dealing with remaining triangles
        for (size_t j = 0; j < numTriangles; ++j)
        {
            // Load vertices of the triangle
            XMVECTOR v0 = XMLoadFloat3((XMFLOAT3*)(positions + triangles->vertIndex[0] * 3));
            XMVECTOR v1 = XMLoadFloat3((XMFLOAT3*)(positions + triangles->vertIndex[1] * 3));
            XMVECTOR v2 = XMLoadFloat3((XMFLOAT3*)(positions + triangles->vertIndex[2] * 3));
            ++triangles;

            // Calculate face normal
            XMVECTOR plane = XMPlaneFromPoints(v0, v1, v2);

            // Store result
            __DX_STORE_PS(&faceNormals->x, plane);
            ++faceNormals;
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilDirectXMath::calculateLightFacing(
        const Vector4& lightPos,
        const Vector4* faceNormals,
        char* lightFacings,
        size_t numFaces)
    {
        assert(_isAlignedForDirectXMath(faceNormals));

        XMVECTOR n0, n1, n2, n3;
        XMVECTOR t0, t1;
        XMVECTOR dp;
        XMCHAR4 bytemask;

        // Load light vector, unaligned
        XMVECTOR lp = XMLoadFloat4((XMFLOAT4*)(&lightPos.x));

        size_t numIterations = numFaces / 4;
        numFaces &= 3;

        // Four faces per-iteration
        for (size_t i = 0; i < numIterations; ++i)
        {
            // Load face normals, aligned
            n0 = __DX_LOAD_PS(&faceNormals[0].x);
            n1 = __DX_LOAD_PS(&faceNormals[1].x);
            n2 = __DX_LOAD_PS(&faceNormals[2].x);
            n3 = __DX_LOAD_PS(&faceNormals[3].x);
            faceNormals += 4;

            // Multiply by light vector
            n0 = XMVectorMultiply(n0, lp);        // x0 y0 z0 w0
            n1 = XMVectorMultiply(n1, lp);        // x1 y1 z1 w1
            n2 = XMVectorMultiply(n2, lp);        // x2 y2 z2 w2
            n3 = XMVectorMultiply(n3, lp);        // x3 y3 z3 w3

            // Horizontal add four vector values.
            t0 = XMVectorAdd(                                           // x0+z0 x1+z1 y0+w0 y1+w1
                XMVectorMergeXY(n0, n1),    // x0 x1 y0 y1
                XMVectorMergeZW(n0, n1));   // z0 z1 w0 w1
            t1 = XMVectorAdd(                                           // x2+z2 x3+z3 y2+w2 y3+w3
                XMVectorMergeXY(n2, n3),    // x2 x3 y2 y3
                XMVectorMergeZW(n2, n3));   // z2 z3 w2 w3
            dp = XMVectorAdd(                                           // dp0 dp1 dp2 dp3
                XMVectorPermute<0, 1, 4, 5>(t0, t1),    // x0+z0 x1+z1 x2+z2 x3+z3
                XMVectorPermute<6, 7, 2, 3>(t1, t0));   // y0+w0 y1+w1 y2+w2 y3+w3

            bytemask = XMVector4GreaterByteMask(dp, g_XMZero);

            // Convert 4-bits mask to 4 bytes, and store results.
            *reinterpret_cast<uint32*>(lightFacings) = 0x01010101 &
                *reinterpret_cast<const uint32*>(&bytemask);
            lightFacings += 4;
        }

        // Dealing with remaining faces
        switch (numFaces)
        {
        case 3:
            n0 = __DX_LOAD_PS(&faceNormals[0].x);
            n1 = __DX_LOAD_PS(&faceNormals[1].x);
            n2 = __DX_LOAD_PS(&faceNormals[2].x);

            n0 = XMVectorMultiply(n0, lp);        // x0 y0 z0 w0
            n1 = XMVectorMultiply(n1, lp);        // x1 y1 z1 w1
            n2 = XMVectorMultiply(n2, lp);        // x2 y2 z2 w2

            t0 = XMVectorAdd(                                            // x0+z0 x1+z1 y0+w0 y1+w1
                XMVectorMergeXY(n0, n1),    // x0 x1 y0 y1
                XMVectorMergeZW(n0, n1));   // z0 z1 w0 w1
            t1 = XMVectorAdd(                                            // x2+z2 x2+z2 y2+w2 y2+w2
                XMVectorMergeXY(n2, n2),    // x2 x2 y2 y2
                XMVectorMergeZW(n2, n2));   // z2 z2 w2 w2
            dp = XMVectorAdd(                                            // dp0 dp1 dp2 dp2
                XMVectorPermute<0, 1, 4, 5>(t0, t1),    // x0+z0 x1+z1 x2+z2 x2+z2
                XMVectorPermute<6, 7, 2, 3>(t1, t0));   // y0+w0 y1+w1 y2+w2 y2+w2

            bytemask = XMVector4GreaterByteMask(dp, g_XMZero);

            lightFacings[0] = 0x01 & bytemask.c[0];
            lightFacings[1] = 0x01 & bytemask.c[1];
            lightFacings[2] = 0x01 & bytemask.c[2];
            break;

        case 2:
            n0 = __DX_LOAD_PS(&faceNormals[0].x);
            n1 = __DX_LOAD_PS(&faceNormals[1].x);

            n0 = XMVectorMultiply(n0, lp);        // x0 y0 z0 w0
            n1 = XMVectorMultiply(n1, lp);        // x1 y1 z1 w1

            t0 = XMVectorAdd(                                            // x0+z0 x1+z1 y0+w0 y1+w1
                XMVectorMergeXY(n0, n1),    // x0 x1 y0 y1
                XMVectorMergeZW(n0, n1));   // z0 z1 w0 w1
            dp = XMVectorAdd(                                            // dp0 dp1 dp0 dp1
                XMVectorSwizzle<0, 1, 0, 1>(t0),        // x0+z0 x1+z1 x0+z0 x1+z1
                XMVectorSwizzle<2, 3, 2, 3>(t0));   // y0+w0 y1+w1 y0+w0 y1+w1

            bytemask = XMVector4GreaterByteMask(dp, g_XMZero);

            lightFacings[0] = 0x01 & bytemask.c[0];
            lightFacings[1] = 0x01 & bytemask.c[1];
            break;

        case 1:
            n0 = __DX_LOAD_PS(&faceNormals[0].x);

            n0 = XMVectorMultiply(n0, lp);        // x0 y0 z0 w0

            t0 = XMVectorAdd(                                            // x0+z0 x0+z0 y0+w0 y0+w0
                XMVectorMergeXY(n0, n0),    // x0 x0 y0 y0
                XMVectorMergeZW(n0, n0));   // z0 z0 w0 w0
            dp = XMVectorAdd(                                            // dp0 dp0 dp0 dp0
                XMVectorSplatX(t0),      // x0+z0 x0+z0 x0+z0 x0+z0
                XMVectorSplatZ(t0));     // y0+w0 y0+w0 y0+w0 y0+w0

            bytemask = XMVector4GreaterByteMask(dp, g_XMZero);

            lightFacings[0] = 0x01 & bytemask.c[0];
            break;
        }
    }
    //---------------------------------------------------------------------
    // Template to extrude vertices for directional light.
    template <bool srcAligned, bool destAligned>
    struct ExtrudeVertices_DirectXMath_DirectionalLight
    {
        static void apply(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* pSrcPos,
            float* pDestPos,
            size_t numVertices)
        {
            typedef DirectXMathMemoryAccessor<srcAligned> SrcAccessor;
            typedef DirectXMathMemoryAccessor<destAligned> DestAccessor;

            // Directional light, extrusion is along light direction

            // Load light vector, unaligned
            XMVECTOR lp = XMLoadFloat4((XMFLOAT4*)(&lightPos.x));

            // Calculate extrusion direction, note that we use inverted direction here
            // for eliminate an extra negative instruction, we'll compensate for that
            // by use subtract instruction instead later.
            XMVECTOR dir = XMVectorMultiply(      // X Y Z -
                XMVector3NormalizeEst(lp),
                XMVectorReplicate(extrudeDist));
            
            // Prepare extrude direction for extruding 4 vertices parallelly
            XMVECTOR dir0 = XMVectorSwizzle<0, 1, 2, 0>(dir);   // X Y Z X
            XMVECTOR dir1 = XMVectorSwizzle<1, 2, 0, 1>(dir);   // Y Z X Y
            XMVECTOR dir2 = XMVectorSwizzle<2, 0, 1, 2>(dir);   // Z X Y Z

            XMVECTOR s0, s1, s2;
            XMVECTOR d0, d1, d2;

            size_t numIterations = numVertices / 4;
            numVertices &= 3;

            // Extruding 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                s0 = SrcAccessor::load(pSrcPos + 0);
                s1 = SrcAccessor::load(pSrcPos + 4);
                s2 = SrcAccessor::load(pSrcPos + 8);
                pSrcPos += 12;

                // The extrusion direction is inverted, use subtract instruction here
                d0 = XMVectorSubtract(s0, dir0);    // X0 Y0 Z0 X1
                d1 = XMVectorSubtract(s1, dir1);    // Y1 Z1 X2 Y2
                d2 = XMVectorSubtract(s2, dir2);    // Z2 X3 Y3 Z3

                DestAccessor::store(pDestPos + 0, d0);
                DestAccessor::store(pDestPos + 4, d1);
                DestAccessor::store(pDestPos + 8, d2);
                pDestPos += 12;
            }

            // Dealing with remaining vertices
            switch (numVertices)
            {
            case 3:
                // 9 floating-point values
                s0 = SrcAccessor::load(pSrcPos + 0);
                s1 = SrcAccessor::load(pSrcPos + 4);
                s2 = XMLoadFloat(pSrcPos + 8);

                // The extrusion direction is inverted, use subtract instruction here
                d0 = XMVectorSubtract(s0, dir0);    // X0 Y0 Z0 X1
                d1 = XMVectorSubtract(s1, dir1);    // Y1 Z1 X2 Y2
                d2 = XMVectorSubtract(s2, dir2);    // Z2 -- -- --

                DestAccessor::store(pDestPos + 0, d0);
                DestAccessor::store(pDestPos + 4, d1);
                XMStoreFloat(pDestPos + 8, d2);
                break;

            case 2:
                // 6 floating-point values
                s0 = SrcAccessor::load(pSrcPos + 0);
                s1 = XMLoadFloat2((XMFLOAT2*)(pSrcPos + 4));

                // The extrusion direction is inverted, use subtract instruction here
                d0 = XMVectorSubtract(s0, dir0);    // X0 Y0 Z0 X1
                d1 = XMVectorSubtract(s1, dir1);    // Y1 Z1 -- --

                DestAccessor::store(pDestPos + 0, d0);
                XMStoreFloat2((XMFLOAT2*)(pDestPos + 4), d1);
                break;

            case 1:
                // 3 floating-point values
                s0 = XMLoadFloat3((XMFLOAT3*)(pSrcPos + 0));

                // The extrusion direction is inverted, use subtract instruction here
                d0 = XMVectorSubtract(s0, dir0);    // X0 Y0 Z0 --

                XMStoreFloat3((XMFLOAT3*)(pDestPos + 0), d0);
                break;
            }
        }
    };
    //---------------------------------------------------------------------
    // Template to extrude vertices for point light.
    template <bool srcAligned, bool destAligned>
    struct ExtrudeVertices_DirectXMath_PointLight
    {
        static void apply(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* pSrcPos,
            float* pDestPos,
            size_t numVertices)
        {
            typedef DirectXMathMemoryAccessor<srcAligned> SrcAccessor;
            typedef DirectXMathMemoryAccessor<destAligned> DestAccessor;

            // Point light, will calculate extrusion direction for every vertex

            // Load light vector, unaligned
            XMVECTOR lp = XMLoadFloat4((XMFLOAT4*)(&lightPos.x));

            // Load extrude distance
            XMVECTOR extrudeDist4 = XMVectorReplicate(extrudeDist);

            size_t numIterations = numVertices / 4;
            numVertices &= 3;

            // Extruding 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Load source positions
                XMVECTOR s0 = SrcAccessor::load(pSrcPos + 0);     // x0 y0 z0 x1
                XMVECTOR s1 = SrcAccessor::load(pSrcPos + 4);     // y1 z1 x2 y2
                XMVECTOR s2 = SrcAccessor::load(pSrcPos + 8);     // z2 x3 y3 z3
                pSrcPos += 12;

                // Arrange to 3x4 component-major for batches calculate
                __DX_TRANSPOSE4x3_PS(s0, s1, s2);

                // Calculate unnormalised extrusion direction
                XMVECTOR dx = XMVectorSubtract(s0, XMVectorSplatX(lp));     // X0 X1 X2 X3
                XMVECTOR dy = XMVectorSubtract(s1, XMVectorSplatY(lp));     // Y0 Y1 Y2 Y3
                XMVECTOR dz = XMVectorSubtract(s2, XMVectorSplatZ(lp));     // Z0 Z1 Z2 Z3

                // Normalise extrusion direction and multiply by extrude distance
                XMVECTOR tmp = __DX_DOT3x3_PS(dx, dy, dz, dx, dy, dz);
                tmp = XMVectorMultiply(XMVectorReciprocalSqrtEst(tmp), extrudeDist4);
                dx = XMVectorMultiply(dx, tmp);
                dy = XMVectorMultiply(dy, tmp);
                dz = XMVectorMultiply(dz, tmp);

                // Calculate extruded positions
                XMVECTOR d0 = XMVectorAdd(dx, s0);
                XMVECTOR d1 = XMVectorAdd(dy, s1);
                XMVECTOR d2 = XMVectorAdd(dz, s2);

                // Arrange back to 4x3 continuous format for store results
                __DX_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store extruded positions
                DestAccessor::store(pDestPos + 0, d0);
                DestAccessor::store(pDestPos + 4, d1);
                DestAccessor::store(pDestPos + 8, d2);
                pDestPos += 12;
            }

            // Dealing with remaining vertices
            for (size_t j = 0; j  < numVertices; ++j)
            {
                // Load source position
                XMVECTOR src = XMLoadFloat3((XMFLOAT3*)(pSrcPos + 0));  // x y z 0
                pSrcPos += 3;

                // Calculate unnormalised extrusion direction
                XMVECTOR dir = XMVectorSubtract(src, lp); // X Y Z 0

                // Normalise extrusion direction and multiply by extrude distance
                dir = XMVectorMultiply(
                    XMVector3NormalizeEst(dir),
                    extrudeDist4);

                // Calculate extruded position
                XMVECTOR dst = XMVectorAdd(dir, src);

                // Store extruded position
                XMStoreFloat3((XMFLOAT3*)(pDestPos + 0), dst);
                pDestPos += 3;
            }
        }
    };
    //---------------------------------------------------------------------
    void OptimisedUtilDirectXMath::extrudeVertices(
        const Vector4& lightPos,
        Real extrudeDist,
        const float* pSrcPos,
        float* pDestPos,
        size_t numVertices)
    {
        // Note: Since pDestPos is following tail of pSrcPos, we can't assume
        // it's aligned to SIMD alignment properly, so must check for it here.
        //
        // TODO: Add extra vertex to the vertex buffer for make sure pDestPos
        // aligned same as pSrcPos.
        //

        // We are use DirectXMath reciprocal square root directly while calculating
        // extrusion direction, since precision loss not that important here.
        //
        if (lightPos.w == 0.0f)
        {
            if (_isAlignedForDirectXMath(pSrcPos))
            {
                if (_isAlignedForDirectXMath(pDestPos))
                    ExtrudeVertices_DirectXMath_DirectionalLight<true, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_DirectXMath_DirectionalLight<true, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
            else
            {
                if (_isAlignedForDirectXMath(pDestPos))
                    ExtrudeVertices_DirectXMath_DirectionalLight<false, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_DirectXMath_DirectionalLight<false, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
        }
        else
        {
            assert(lightPos.w == 1.0f);

            if (_isAlignedForDirectXMath(pSrcPos))
            {
                if (_isAlignedForDirectXMath(pDestPos))
                    ExtrudeVertices_DirectXMath_PointLight<true, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_DirectXMath_PointLight<true, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
            else
            {
                if (_isAlignedForDirectXMath(pDestPos))
                    ExtrudeVertices_DirectXMath_PointLight<false, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_DirectXMath_PointLight<false, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    extern OptimisedUtil* _getOptimisedUtilDirectXMath(void)
    {
        static OptimisedUtilDirectXMath msOptimisedUtilDirectXMath;
        return &msOptimisedUtilDirectXMath;
    }

}

#endif // __OGRE_HAVE_DIRECTXMATH

