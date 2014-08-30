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

#if __OGRE_HAVE_SSE

#include "OgreMatrix4.h"

// Should keep this includes at latest to avoid potential "xmmintrin.h" included by
// other header file on some platform for some reason.
#include "OgreSIMDHelper.h"

// I'd like to merge this file with OgreOptimisedUtil.cpp, but it's
// impossible when compile with gcc, due SSE instructions can only
// enable/disable at file level.

//-------------------------------------------------------------------------
//
// The routines implemented in this file are performance oriented,
// which means saving every penny as possible. This requirement might
// break some C++/STL-rules.
//
//
// Some rules I'd like to respects:
//
// 1. Had better use unpacklo/hi, movelh/hl instead of shuffle because
//    it can saving one byte of binary code :)
// 2. Use add/sub instead of mul.
// 3. Eliminate prolog code of function call.
//
// The last, anything recommended by Intel Optimization Reference Manual.
//
//-------------------------------------------------------------------------

// Use unrolled SSE version when vertices exceeds this limit
#define OGRE_SSE_SKINNING_UNROLL_VERTICES  16

namespace Ogre {

//-------------------------------------------------------------------------
// Local classes
//-------------------------------------------------------------------------

    /** SSE implementation of OptimisedUtil.
    @note
        Don't use this class directly, use OptimisedUtil instead.
    */
    class _OgrePrivate OptimisedUtilSSE : public OptimisedUtil
    {
    protected:
        /// Do we prefer to use a general SSE version for position/normal shared buffers?
        bool mPreferGeneralVersionForSharedBuffers;

    public:
        /// Constructor
        OptimisedUtilSSE(void);

        /// @copydoc OptimisedUtil::softwareVertexSkinning
        virtual void _OGRE_SIMD_ALIGN_ATTRIBUTE softwareVertexSkinning(
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
        virtual void _OGRE_SIMD_ALIGN_ATTRIBUTE softwareVertexMorph(
            Real t,
            const float *srcPos1, const float *srcPos2,
            float *dstPos,
            size_t pos1VSize, size_t pos2VSize, size_t dstVSize, 
            size_t numVertices,
            bool morphNormals);

        /// @copydoc OptimisedUtil::concatenateAffineMatrices
        virtual void _OGRE_SIMD_ALIGN_ATTRIBUTE concatenateAffineMatrices(
            const Matrix4& baseMatrix,
            const Matrix4* srcMatrices,
            Matrix4* dstMatrices,
            size_t numMatrices);

        /// @copydoc OptimisedUtil::calculateFaceNormals
        virtual void _OGRE_SIMD_ALIGN_ATTRIBUTE calculateFaceNormals(
            const float *positions,
            const v1::EdgeData::Triangle *triangles,
            Vector4 *faceNormals,
            size_t numTriangles);

        /// @copydoc OptimisedUtil::calculateLightFacing
        virtual void _OGRE_SIMD_ALIGN_ATTRIBUTE calculateLightFacing(
            const Vector4& lightPos,
            const Vector4* faceNormals,
            char* lightFacings,
            size_t numFaces);

        /// @copydoc OptimisedUtil::extrudeVertices
        virtual void _OGRE_SIMD_ALIGN_ATTRIBUTE extrudeVertices(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* srcPositions,
            float* destPositions,
            size_t numVertices);
    };

#if defined(__OGRE_SIMD_ALIGN_STACK)
    /** Stack-align implementation of OptimisedUtil.
    @remarks
        User code compiled by icc and gcc might not align stack
        properly, we need ensure stack align to a 16-bytes boundary
        when execute SSE function.
    @par
        We implemeted as align stack following a virtual function call,
        then should guarantee call instruction are used instead of inline
        underlying function body here (which might causing problem).
    @note
        Don't use this class directly, use OptimisedUtil instead.
    */
    class _OgrePrivate OptimisedUtilWithStackAlign : public OptimisedUtil
    {
    protected:
        /// The actual implementation
        OptimisedUtil* mImpl;

    public:
        /// Constructor
        OptimisedUtilWithStackAlign(OptimisedUtil* impl)
            : mImpl(impl)
        {
        }

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
            size_t numVertices)
        {
            __OGRE_SIMD_ALIGN_STACK();

            mImpl->softwareVertexSkinning(
                srcPosPtr, destPosPtr,
                srcNormPtr, destNormPtr,
                blendWeightPtr, blendIndexPtr,
                blendMatrices,
                srcPosStride, destPosStride,
                srcNormStride, destNormStride,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numVertices);
        }

        /// @copydoc OptimisedUtil::softwareVertexMorph
        virtual void softwareVertexMorph(
            Real t,
            const float *srcPos1, const float *srcPos2,
            float *dstPos,
            size_t pos1VSize, size_t pos2VSize, size_t dstVSize, 
            size_t numVertices,
            bool morphNormals)
        {
            __OGRE_SIMD_ALIGN_STACK();

            mImpl->softwareVertexMorph(
                t,
                srcPos1, srcPos2,
                dstPos,
                pos1VSize, pos2VSize, dstVSize, 
                numVertices,
                morphNormals);
        }

        /// @copydoc OptimisedUtil::concatenateAffineMatrices
        virtual void concatenateAffineMatrices(
            const Matrix4& baseMatrix,
            const Matrix4* srcMatrices,
            Matrix4* dstMatrices,
            size_t numMatrices)
        {
            __OGRE_SIMD_ALIGN_STACK();

            mImpl->concatenateAffineMatrices(
                baseMatrix,
                srcMatrices,
                dstMatrices,
                numMatrices);
        }

        /// @copydoc OptimisedUtil::calculateFaceNormals
        virtual void calculateFaceNormals(
            const float *positions,
            const EdgeData::Triangle *triangles,
            Vector4 *faceNormals,
            size_t numTriangles)
        {
            __OGRE_SIMD_ALIGN_STACK();

            mImpl->calculateFaceNormals(
                positions,
                triangles,
                faceNormals,
                numTriangles);
        }

        /// @copydoc OptimisedUtil::calculateLightFacing
        virtual void calculateLightFacing(
            const Vector4& lightPos,
            const Vector4* faceNormals,
            char* lightFacings,
            size_t numFaces)
        {
            __OGRE_SIMD_ALIGN_STACK();

            mImpl->calculateLightFacing(
                lightPos,
                faceNormals,
                lightFacings,
                numFaces);
        }

        /// @copydoc OptimisedUtil::extrudeVertices
        virtual void extrudeVertices(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* srcPositions,
            float* destPositions,
            size_t numVertices)
        {
            __OGRE_SIMD_ALIGN_STACK();

            mImpl->extrudeVertices(
                lightPos,
                extrudeDist,
                srcPositions,
                destPositions,
                numVertices);
        }
    };
#endif  // !defined(__OGRE_SIMD_ALIGN_STACK)

//---------------------------------------------------------------------
// Some useful macro for collapse matrices.
//---------------------------------------------------------------------

#define __LOAD_MATRIX(row0, row1, row2, pMatrix)                        \
    {                                                                   \
        row0 = __MM_LOAD_PS((*pMatrix)[0]);                             \
        row1 = __MM_LOAD_PS((*pMatrix)[1]);                             \
        row2 = __MM_LOAD_PS((*pMatrix)[2]);                             \
    }

#define __LERP_MATRIX(row0, row1, row2, weight, pMatrix)                \
    {                                                                   \
        row0 = __MM_LERP_PS(weight, row0, __MM_LOAD_PS((*pMatrix)[0])); \
        row1 = __MM_LERP_PS(weight, row1, __MM_LOAD_PS((*pMatrix)[1])); \
        row2 = __MM_LERP_PS(weight, row2, __MM_LOAD_PS((*pMatrix)[2])); \
    }

#define __LOAD_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix)       \
    {                                                                   \
        row0 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix)[0]), weight);         \
        row1 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix)[1]), weight);         \
        row2 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix)[2]), weight);         \
    }

#define __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix)      \
    {                                                                   \
        row0 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix)[0]), weight, row0); \
        row1 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix)[1]), weight, row1); \
        row2 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix)[2]), weight, row2); \
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
        weight = _mm_load_ps1(pWeights + 1);                                    \
        pMatrix0 = ppMatrices[pIndices[0]];                                     \
        __LOAD_MATRIX(row0, row1, row2, pMatrix0);                              \
        pMatrix1 = ppMatrices[pIndices[1]];                                     \
        __LERP_MATRIX(row0, row1, row2, weight, pMatrix1);                      \
    }

/** Collapse three-weighted matrix.
*/
#define __COLLAPSE_MATRIX_W3(row0, row1, row2, ppMatrices, pIndices, pWeights)  \
    {                                                                           \
        weight = _mm_load_ps1(pWeights + 0);                                    \
        pMatrix0 = ppMatrices[pIndices[0]];                                     \
        __LOAD_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix0);             \
        weight = _mm_load_ps1(pWeights + 1);                                    \
        pMatrix1 = ppMatrices[pIndices[1]];                                     \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix1);            \
        weight = _mm_load_ps1(pWeights + 2);                                    \
        pMatrix2 = ppMatrices[pIndices[2]];                                     \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix2);            \
    }

/** Collapse four-weighted matrix.
*/
#define __COLLAPSE_MATRIX_W4(row0, row1, row2, ppMatrices, pIndices, pWeights)  \
    {                                                                           \
        /* Load four blend weights at one time, they will be shuffled later */  \
        weights = _mm_loadu_ps(pWeights);                                       \
                                                                                \
        pMatrix0 = ppMatrices[pIndices[0]];                                     \
        weight = __MM_SELECT(weights, 0);                                       \
        __LOAD_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix0);             \
        pMatrix1 = ppMatrices[pIndices[1]];                                     \
        weight = __MM_SELECT(weights, 1);                                       \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix1);            \
        pMatrix2 = ppMatrices[pIndices[2]];                                     \
        weight = __MM_SELECT(weights, 2);                                       \
        __ACCUM_WEIGHTED_MATRIX(row0, row1, row2, weight, pMatrix2);            \
        pMatrix3 = ppMatrices[pIndices[3]];                                     \
        weight = __MM_SELECT(weights, 3);                                       \
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
        __m128 weight, weights;                                                 \
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
        __m128 weight, weights;                                                 \
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
    // General SSE version skinning positions, and optional skinning normals.
    static void softwareVertexSkinning_SSE_General(
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
            __m128 m00, m01, m02;
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

            // Rearrange to column-major matrix with rows shuffled order to: Z 0 X Y
            __m128 m03 = _mm_setzero_ps();
            __MM_TRANSPOSE4x4_PS(m02, m03, m00, m01);

            //------------------------------------------------------------------
            // Transform position
            //------------------------------------------------------------------

            __m128 s0, s1, s2;

            // Load source position
            s0 = _mm_load_ps1(pSrcPos + 0);
            s1 = _mm_load_ps1(pSrcPos + 1);
            s2 = _mm_load_ps1(pSrcPos + 2);

            // Transform by collapsed matrix
            __m128 accumPos = __MM_DOT4x3_PS(m02, m03, m00, m01, s0, s1, s2);   // z 0 x y

            // Store blended position, no aligned requirement
            _mm_storeh_pi((__m64*)pDestPos, accumPos);
            _mm_store_ss(pDestPos+2, accumPos);

            // Advance source and target position pointers
            advanceRawPointer(pSrcPos, srcPosStride);
            advanceRawPointer(pDestPos, destPosStride);

            //------------------------------------------------------------------
            // Optional blend normal
            //------------------------------------------------------------------

            if (pSrcNorm)
            {
                // Load source normal
                s0 = _mm_load_ps1(pSrcNorm + 0);
                s1 = _mm_load_ps1(pSrcNorm + 1);
                s2 = _mm_load_ps1(pSrcNorm + 2);

                // Transform by collapsed matrix
                __m128 accumNorm = __MM_DOT3x3_PS(m02, m03, m00, s0, s1, s2);   // z 0 x y

                // Normalise normal
                __m128 tmp = _mm_mul_ps(accumNorm, accumNorm);                  // z^2 0 x^2 y^2
                tmp = __MM_ACCUM3_PS(tmp,
                        _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,3,1,2)),         // x^2 0 y^2 z^2
                        _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,0,1,3)));        // y^2 0 z^2 x^2
                // Note: zero divided here, but neglectable
                tmp = __MM_RSQRT_PS(tmp);
                accumNorm = _mm_mul_ps(accumNorm, tmp);

                // Store blended normal, no aligned requirement
                _mm_storeh_pi((__m64*)pDestNorm, accumNorm);
                _mm_store_ss(pDestNorm+2, accumNorm);

                // Advance source and target normal pointers
                advanceRawPointer(pSrcNorm, srcNormStride);
                advanceRawPointer(pDestNorm, destNormStride);
            }
        }
    }
    //---------------------------------------------------------------------
    // Special SSE version skinning shared buffers of position and normal,
    // and the buffer are packed.
    template <bool srcAligned, bool destAligned>
    struct SoftwareVertexSkinning_SSE_PosNorm_Shared_Packed
    {
        static void apply(
            const float* pSrc, float* pDest,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef SSEMemoryAccessor<srcAligned> SrcAccessor;
            typedef SSEMemoryAccessor<destAligned> DestAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                __m128 m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
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

                __m128 s0, s1, s2, s3, s4, s5, d0, d1, d2, d3, d4, d5;
                __m128 t0, t1, t2, t3, t4, t5;

                // Load source position/normals
                s0 = SrcAccessor::load(pSrc + 0);                       // px0 py0 pz0 nx0
                s1 = SrcAccessor::load(pSrc + 4);                       // ny0 nz0 px1 py1
                s2 = SrcAccessor::load(pSrc + 8);                       // pz1 nx1 ny1 nz1
                s3 = SrcAccessor::load(pSrc + 12);                      // px2 py2 pz2 nx2
                s4 = SrcAccessor::load(pSrc + 16);                      // ny2 nz2 px3 py3
                s5 = SrcAccessor::load(pSrc + 20);                      // pz3 nx3 ny3 nz3

                // Rearrange to component-major for batches calculate.

                t0 = _mm_unpacklo_ps(s0, s3);                           // px0 px2 py0 py2
                t1 = _mm_unpackhi_ps(s0, s3);                           // pz0 pz2 nx0 nx2
                t2 = _mm_unpacklo_ps(s1, s4);                           // ny0 ny2 nz0 nz2
                t3 = _mm_unpackhi_ps(s1, s4);                           // px1 px3 py1 py3
                t4 = _mm_unpacklo_ps(s2, s5);                           // pz1 pz3 nx1 nx3
                t5 = _mm_unpackhi_ps(s2, s5);                           // ny1 ny3 nz1 nz3

                s0 = _mm_unpacklo_ps(t0, t3);                           // px0 px1 px2 px3
                s1 = _mm_unpackhi_ps(t0, t3);                           // py0 py1 py2 py3
                s2 = _mm_unpacklo_ps(t1, t4);                           // pz0 pz1 pz2 pz3
                s3 = _mm_unpackhi_ps(t1, t4);                           // nx0 nx1 nx2 nx3
                s4 = _mm_unpacklo_ps(t2, t5);                           // ny0 ny1 ny2 ny3
                s5 = _mm_unpackhi_ps(t2, t5);                           // nz0 nz1 nz2 nz3

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __MM_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __MM_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // PX0 PX1 PX2 PX3
                d3 = __MM_DOT3x3_PS(m00, m10, m20, s3, s4, s5);         // NX0 NX1 NX2 NX3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __MM_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __MM_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // PY0 PY1 PY2 PY3
                d4 = __MM_DOT3x3_PS(m01, m11, m21, s3, s4, s5);         // NY0 NY1 NY2 NY3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __MM_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __MM_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // PZ0 PZ1 PZ2 PZ3
                d5 = __MM_DOT3x3_PS(m02, m12, m22, s3, s4, s5);         // NZ0 NZ1 NZ2 NZ3

                // Normalise normals
                __m128 tmp = __MM_DOT3x3_PS(d3, d4, d5, d3, d4, d5);
                tmp = __MM_RSQRT_PS(tmp);
                d3 = _mm_mul_ps(d3, tmp);
                d4 = _mm_mul_ps(d4, tmp);
                d5 = _mm_mul_ps(d5, tmp);

                // Arrange back to continuous format for store results

                t0 = _mm_unpacklo_ps(d0, d1);                           // PX0 PY0 PX1 PY1
                t1 = _mm_unpackhi_ps(d0, d1);                           // PX2 PY2 PX3 PY3
                t2 = _mm_unpacklo_ps(d2, d3);                           // PZ0 NX0 PZ1 NX1
                t3 = _mm_unpackhi_ps(d2, d3);                           // PZ2 NX2 PZ3 NX3
                t4 = _mm_unpacklo_ps(d4, d5);                           // NY0 NZ0 NY1 NZ1
                t5 = _mm_unpackhi_ps(d4, d5);                           // NY2 NZ2 NY3 NZ3

                d0 = _mm_movelh_ps(t0, t2);                             // PX0 PY0 PZ0 NX0
                d1 = _mm_shuffle_ps(t4, t0, _MM_SHUFFLE(3,2,1,0));      // NY0 NZ0 PX1 PY1
                d2 = _mm_movehl_ps(t4, t2);                             // PZ1 NX1 NY1 NZ1
                d3 = _mm_movelh_ps(t1, t3);                             // PX2 PY2 PZ2 NX2
                d4 = _mm_shuffle_ps(t5, t1, _MM_SHUFFLE(3,2,1,0));      // NY2 NZ2 PX3 PY3
                d5 = _mm_movehl_ps(t5, t3);                             // PZ3 NX3 NY3 NZ3

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
    static FORCEINLINE void softwareVertexSkinning_SSE_PosNorm_Shared_Packed(
            const float* pSrcPos, float* pDestPos,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
    {
        // pSrcPos might can't align to 16 bytes because 8 bytes alignment shift per-vertex

        // Instantiating two version only, since other alignment combinations are not that important.
        if (_isAlignedForSSE(pSrcPos) && _isAlignedForSSE(pDestPos))
        {
            SoftwareVertexSkinning_SSE_PosNorm_Shared_Packed<true, true>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_SSE_PosNorm_Shared_Packed<false, false>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    // Special SSE version skinning separated buffers of position and normal,
    // both of position and normal buffer are packed.
    template <bool srcPosAligned, bool destPosAligned, bool srcNormAligned, bool destNormAligned>
    struct SoftwareVertexSkinning_SSE_PosNorm_Separated_Packed
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
            typedef SSEMemoryAccessor<srcPosAligned> SrcPosAccessor;
            typedef SSEMemoryAccessor<destPosAligned> DestPosAccessor;
            typedef SSEMemoryAccessor<srcNormAligned> SrcNormAccessor;
            typedef SSEMemoryAccessor<destNormAligned> DestNormAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                __m128 m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
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

                __m128 s0, s1, s2, d0, d1, d2;

                // Load source positions
                s0 = SrcPosAccessor::load(pSrcPos + 0);                 // x0 y0 z0 x1
                s1 = SrcPosAccessor::load(pSrcPos + 4);                 // y1 z1 x2 y2
                s2 = SrcPosAccessor::load(pSrcPos + 8);                 // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __MM_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __MM_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // X0 X1 X2 X3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __MM_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __MM_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // Y0 Y1 Y2 Y3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __MM_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __MM_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // Z0 Z1 Z2 Z3

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

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
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed and shuffled matrices
                d0 = __MM_DOT3x3_PS(m00, m10, m20, s0, s1, s2);         // X0 X1 X2 X3
                d1 = __MM_DOT3x3_PS(m01, m11, m21, s0, s1, s2);         // Y0 Y1 Y2 Y3
                d2 = __MM_DOT3x3_PS(m02, m12, m22, s0, s1, s2);         // Z0 Z1 Z2 Z3

                // Normalise normals
                __m128 tmp = __MM_DOT3x3_PS(d0, d1, d2, d0, d1, d2);
                tmp = __MM_RSQRT_PS(tmp);
                d0 = _mm_mul_ps(d0, tmp);
                d1 = _mm_mul_ps(d1, tmp);
                d2 = _mm_mul_ps(d2, tmp);

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

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
    static FORCEINLINE void softwareVertexSkinning_SSE_PosNorm_Separated_Packed(
        const float* pSrcPos, float* pDestPos,
        const float* pSrcNorm, float* pDestNorm,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numIterations)
    {
        assert(_isAlignedForSSE(pSrcPos));

        // Instantiating two version only, since other alignment combination not that important.
        if (_isAlignedForSSE(pSrcNorm) && _isAlignedForSSE(pDestPos) && _isAlignedForSSE(pDestNorm))
        {
            SoftwareVertexSkinning_SSE_PosNorm_Separated_Packed<true, true, true, true>::apply(
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
            SoftwareVertexSkinning_SSE_PosNorm_Separated_Packed<true, false, false, false>::apply(
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
    // Special SSE version skinning position only, the position buffer are
    // packed.
    template <bool srcPosAligned, bool destPosAligned>
    struct SoftwareVertexSkinning_SSE_PosOnly_Packed
    {
        static void apply(
            const float* pSrcPos, float* pDestPos,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef SSEMemoryAccessor<srcPosAligned> SrcPosAccessor;
            typedef SSEMemoryAccessor<destPosAligned> DestPosAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                __m128 m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
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

                __m128 s0, s1, s2, d0, d1, d2;

                // Load source positions
                s0 = SrcPosAccessor::load(pSrcPos + 0);                 // x0 y0 z0 x1
                s1 = SrcPosAccessor::load(pSrcPos + 4);                 // y1 z1 x2 y2
                s2 = SrcPosAccessor::load(pSrcPos + 8);                 // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __MM_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __MM_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // X0 X1 X2 X3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __MM_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __MM_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // Y0 Y1 Y2 Y3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __MM_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __MM_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // Z0 Z1 Z2 Z3

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

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
    static FORCEINLINE void softwareVertexSkinning_SSE_PosOnly_Packed(
        const float* pSrcPos, float* pDestPos,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numIterations)
    {
        assert(_isAlignedForSSE(pSrcPos));

        // Instantiating two version only, since other alignment combination not that important.
        if (_isAlignedForSSE(pDestPos))
        {
            SoftwareVertexSkinning_SSE_PosOnly_Packed<true, true>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_SSE_PosOnly_Packed<true, false>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    OptimisedUtilSSE::OptimisedUtilSSE(void)
        : mPreferGeneralVersionForSharedBuffers(false)
    {
        // For AMD Athlon XP (but not that for Althon 64), it's prefer to never use
        // unrolled version for shared buffers at all, I guess because that version
        // run out of usable CPU registers, or L1/L2 cache related problem, causing
        // slight performance loss than general version.
        //

        if (PlatformInformation::getCpuIdentifier().find("AuthenticAMD") != String::npos)
        {
            // How can I check it's an Athlon XP but not Althon 64?
            // Ok, just test whether supports SSE2/SSE3 or not, if not,
            // assume general version faster than unrolled version :)
            //
            if (!(PlatformInformation::getCpuFeatures() &
                (PlatformInformation::CPU_FEATURE_SSE2 | PlatformInformation::CPU_FEATURE_SSE3)))
            {
                mPreferGeneralVersionForSharedBuffers = true;
            }
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilSSE::softwareVertexSkinning(
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
        __OGRE_CHECK_STACK_ALIGNED_FOR_SSE();

        // All position/normal pointers should be perfect aligned, but still check here
        // for avoid hardware buffer which allocated by potential buggy driver doesn't
        // support alignment properly.
        // Because we are used meta-function technique here, the code is easy to maintenance
        // and still provides all possible alignment combination.
        //

        // Use unrolled routines only if there a lot of vertices
        if (numVertices > OGRE_SSE_SKINNING_UNROLL_VERTICES)
        {
            if (pSrcNorm)
            {
                // Blend position and normal

                if (!mPreferGeneralVersionForSharedBuffers &&
                    srcPosStride == sizeof(float) * (3 + 3) && destPosStride == sizeof(float) * (3 + 3) &&
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
                        softwareVertexSkinning_SSE_General(
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
                    softwareVertexSkinning_SSE_PosNorm_Shared_Packed(
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
                        softwareVertexSkinning_SSE_General(
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
                    softwareVertexSkinning_SSE_PosNorm_Separated_Packed(
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
                        softwareVertexSkinning_SSE_General(
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
                    softwareVertexSkinning_SSE_PosOnly_Packed(
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
            softwareVertexSkinning_SSE_General(
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
    void OptimisedUtilSSE::softwareVertexMorph(
        Real t,
        const float *pSrc1, const float *pSrc2,
        float *pDst,
        size_t pos1VSize, size_t pos2VSize, size_t dstVSize, 
        size_t numVertices,
        bool morphNormals)
    {   
        __OGRE_CHECK_STACK_ALIGNED_FOR_SSE();

        __m128 src01, src02, src11, src12, src21, src22;
        __m128 dst0, dst1, dst2;

        __m128 t4 = _mm_load_ps1(&t);


        // If we're morphing normals, we have twice the number of floats to process
        // Positions are interleaved with normals, so we'll have to separately
        // normalise just the normals later; we'll just lerp in the first pass
        // We can't normalise as we go because normals & positions are only 3 floats
        // each so are not aligned for SSE, we'd mix the data up
        size_t normalsMultiplier = morphNormals ? 2 : 1;
        size_t numIterations = (numVertices*normalsMultiplier) / 4;
        size_t numVerticesRemainder = (numVertices*normalsMultiplier) & 3;
        
        // Save for later
        float *pStartDst = pDst;
                        
        // Never use meta-function technique to accessing memory because looks like
        // VC7.1 generate a bit inefficient binary code when put following code into
        // inline function.

        if (_isAlignedForSSE(pSrc1) && _isAlignedForSSE(pSrc2) && _isAlignedForSSE(pDst))
        {
            // All data aligned

            // Morph 4 vertices per-iteration. Special designed for use all
            // available CPU registers as possible (7 registers used here),
            // and avoid temporary values allocated in stack for suppress
            // extra memory access.
            for (size_t i = 0; i < numIterations; ++i)
            {
                // 12 floating-point values
                src01 = __MM_LOAD_PS(pSrc1 + 0);
                src02 = __MM_LOAD_PS(pSrc2 + 0);
                src11 = __MM_LOAD_PS(pSrc1 + 4);
                src12 = __MM_LOAD_PS(pSrc2 + 4);
                src21 = __MM_LOAD_PS(pSrc1 + 8);
                src22 = __MM_LOAD_PS(pSrc2 + 8);
                pSrc1 += 12; pSrc2 += 12;

                dst0 = __MM_LERP_PS(t4, src01, src02);
                dst1 = __MM_LERP_PS(t4, src11, src12);
                dst2 = __MM_LERP_PS(t4, src21, src22);

                __MM_STORE_PS(pDst + 0, dst0);
                __MM_STORE_PS(pDst + 4, dst1);
                __MM_STORE_PS(pDst + 8, dst2);
                pDst += 12;
            }

            // Morph remaining vertices
            switch (numVerticesRemainder)
            {
            case 3:
                // 9 floating-point values
                src01 = __MM_LOAD_PS(pSrc1 + 0);
                src02 = __MM_LOAD_PS(pSrc2 + 0);
                src11 = __MM_LOAD_PS(pSrc1 + 4);
                src12 = __MM_LOAD_PS(pSrc2 + 4);
                src21 = _mm_load_ss(pSrc1 + 8);
                src22 = _mm_load_ss(pSrc2 + 8);

                dst0 = __MM_LERP_PS(t4, src01, src02);
                dst1 = __MM_LERP_PS(t4, src11, src12);
                dst2 = __MM_LERP_SS(t4, src21, src22);

                __MM_STORE_PS(pDst + 0, dst0);
                __MM_STORE_PS(pDst + 4, dst1);
                _mm_store_ss(pDst + 8, dst2);
                break;

            case 2:
                // 6 floating-point values
                src01 = __MM_LOAD_PS(pSrc1 + 0);
                src02 = __MM_LOAD_PS(pSrc2 + 0);
                src11 = _mm_loadl_pi(t4, (const __m64*)(pSrc1 + 4));  // t4 is meaningless here
                src12 = _mm_loadl_pi(t4, (const __m64*)(pSrc2 + 4));  // t4 is meaningless here

                dst0 = __MM_LERP_PS(t4, src01, src02);
                dst1 = __MM_LERP_PS(t4, src11, src12);

                __MM_STORE_PS(pDst + 0, dst0);
                _mm_storel_pi((__m64*)(pDst + 4), dst1);
                break;

            case 1:
                // 3 floating-point values
                src01 = _mm_load_ss(pSrc1 + 2);
                src02 = _mm_load_ss(pSrc2 + 2);
                src01 = _mm_loadh_pi(src01, (const __m64*)(pSrc1 + 0));
                src02 = _mm_loadh_pi(src02, (const __m64*)(pSrc2 + 0));

                dst0 = __MM_LERP_PS(t4, src01, src02);

                _mm_storeh_pi((__m64*)(pDst + 0), dst0);
                _mm_store_ss(pDst + 2, dst0);
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
                src01 = _mm_loadu_ps(pSrc1 + 0);
                src02 = _mm_loadu_ps(pSrc2 + 0);
                src11 = _mm_loadu_ps(pSrc1 + 4);
                src12 = _mm_loadu_ps(pSrc2 + 4);
                src21 = _mm_loadu_ps(pSrc1 + 8);
                src22 = _mm_loadu_ps(pSrc2 + 8);
                pSrc1 += 12; pSrc2 += 12;

                dst0 = __MM_LERP_PS(t4, src01, src02);
                dst1 = __MM_LERP_PS(t4, src11, src12);
                dst2 = __MM_LERP_PS(t4, src21, src22);

                _mm_storeu_ps(pDst + 0, dst0);
                _mm_storeu_ps(pDst + 4, dst1);
                _mm_storeu_ps(pDst + 8, dst2);
                pDst += 12;
                
            }

            // Morph remaining vertices
            switch (numVerticesRemainder)
            {
            case 3:
                // 9 floating-point values
                src01 = _mm_loadu_ps(pSrc1 + 0);
                src02 = _mm_loadu_ps(pSrc2 + 0);
                src11 = _mm_loadu_ps(pSrc1 + 4);
                src12 = _mm_loadu_ps(pSrc2 + 4);
                src21 = _mm_load_ss(pSrc1 + 8);
                src22 = _mm_load_ss(pSrc2 + 8);

                dst0 = __MM_LERP_PS(t4, src01, src02);
                dst1 = __MM_LERP_PS(t4, src11, src12);
                dst2 = __MM_LERP_SS(t4, src21, src22);

                _mm_storeu_ps(pDst + 0, dst0);
                _mm_storeu_ps(pDst + 4, dst1);
                _mm_store_ss(pDst + 8, dst2);
                break;

            case 2:
                // 6 floating-point values
                src01 = _mm_loadu_ps(pSrc1 + 0);
                src02 = _mm_loadu_ps(pSrc2 + 0);
                src11 = _mm_loadl_pi(t4, (const __m64*)(pSrc1 + 4));  // t4 is meaningless here
                src12 = _mm_loadl_pi(t4, (const __m64*)(pSrc2 + 4));  // t4 is meaningless here

                dst0 = __MM_LERP_PS(t4, src01, src02);
                dst1 = __MM_LERP_PS(t4, src11, src12);

                _mm_storeu_ps(pDst + 0, dst0);
                _mm_storel_pi((__m64*)(pDst + 4), dst1);
                break;

            case 1:
                // 3 floating-point values
                src01 = _mm_load_ss(pSrc1 + 2);
                src02 = _mm_load_ss(pSrc2 + 2);
                src01 = _mm_loadh_pi(src01, (const __m64*)(pSrc1 + 0));
                src02 = _mm_loadh_pi(src02, (const __m64*)(pSrc2 + 0));

                dst0 = __MM_LERP_PS(t4, src01, src02);

                _mm_storeh_pi((__m64*)(pDst + 0), dst0);
                _mm_store_ss(pDst + 2, dst0);
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
            
            // We'll do one normal each iteration, but still use SSE
            for (size_t n = 0; n < numVertices; ++n)
            {
                // normalise function
                __m128 norm;
                
                // load 3 floating-point normal values
                // This loads into [0] and clears the rest
                norm = _mm_load_ss(pNorm + 2);
                // This loads into [2,3]. [1] is unused
                norm = _mm_loadh_pi(norm, (__m64*)(pNorm + 0));
                
                // Fill a 4-vec with vector length
                // square
                __m128 tmp = _mm_mul_ps(norm, norm);
                // Add - for this we want this effect:
                // orig   3 | 2 | 1 | 0
                // add1   0 | 0 | 0 | 2
                // add2   2 | 3 | 0 | 3
                // This way elements 0, 2 and 3 have the sum of all entries (except 1 which is unused)
                
                tmp = _mm_add_ps(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,0,0,2)));
                // Add final combination & sqrt 
                // bottom 3 elements of l will have length, we don't care about 4
                tmp = _mm_add_ps(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,3,0,3)));
                // Then divide to normalise
                norm = _mm_div_ps(norm, _mm_sqrt_ps(tmp));
                
                // Store back in the same place
                _mm_storeh_pi((__m64*)(pNorm + 0), norm);
                _mm_store_ss(pNorm + 2, norm);
                
                // Skip to next vertex (3x normal components, 3x position components)
                pNorm += 6;

                
            }
            

        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilSSE::concatenateAffineMatrices(
        const Matrix4& baseMatrix,
        const Matrix4* pSrcMat,
        Matrix4* pDstMat,
        size_t numMatrices)
    {
        __OGRE_CHECK_STACK_ALIGNED_FOR_SSE();

        assert(_isAlignedForSSE(pSrcMat));
        assert(_isAlignedForSSE(pDstMat));

        // Load base matrix, unaligned
        __m128 m0 = _mm_loadu_ps(baseMatrix[0]);
        __m128 m1 = _mm_loadu_ps(baseMatrix[1]);
        __m128 m2 = _mm_loadu_ps(baseMatrix[2]);
        __m128 m3 = _mm_loadu_ps(baseMatrix[3]);        // m3 should be equal to (0, 0, 0, 1)

        for (size_t i = 0; i < numMatrices; ++i)
        {
            // Load source matrix, aligned
            __m128 s0 = __MM_LOAD_PS((*pSrcMat)[0]);
            __m128 s1 = __MM_LOAD_PS((*pSrcMat)[1]);
            __m128 s2 = __MM_LOAD_PS((*pSrcMat)[2]);

            ++pSrcMat;

            __m128 t0, t1, t2, t3;

            // Concatenate matrix, and store results

            // Row 0
            t0 = _mm_mul_ps(__MM_SELECT(m0, 0), s0);
            t1 = _mm_mul_ps(__MM_SELECT(m0, 1), s1);
            t2 = _mm_mul_ps(__MM_SELECT(m0, 2), s2);
            t3 = _mm_mul_ps(m0, m3);    // Compiler should optimise this out of the loop
            __MM_STORE_PS((*pDstMat)[0], __MM_ACCUM4_PS(t0,t1,t2,t3));

            // Row 1
            t0 = _mm_mul_ps(__MM_SELECT(m1, 0), s0);
            t1 = _mm_mul_ps(__MM_SELECT(m1, 1), s1);
            t2 = _mm_mul_ps(__MM_SELECT(m1, 2), s2);
            t3 = _mm_mul_ps(m1, m3);    // Compiler should optimise this out of the loop
            __MM_STORE_PS((*pDstMat)[1], __MM_ACCUM4_PS(t0,t1,t2,t3));

            // Row 2
            t0 = _mm_mul_ps(__MM_SELECT(m2, 0), s0);
            t1 = _mm_mul_ps(__MM_SELECT(m2, 1), s1);
            t2 = _mm_mul_ps(__MM_SELECT(m2, 2), s2);
            t3 = _mm_mul_ps(m2, m3);    // Compiler should optimise this out of the loop
            __MM_STORE_PS((*pDstMat)[2], __MM_ACCUM4_PS(t0,t1,t2,t3));

            // Row 3
            __MM_STORE_PS((*pDstMat)[3], m3);

            ++pDstMat;
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilSSE::calculateFaceNormals(
        const float *positions,
        const v1::EdgeData::Triangle *triangles,
        Vector4 *faceNormals,
        size_t numTriangles)
    {
        __OGRE_CHECK_STACK_ALIGNED_FOR_SSE();

        assert(_isAlignedForSSE(faceNormals));

// Load Vector3 as: (x, 0, y, z)
#define __LOAD_VECTOR3(p)   _mm_loadh_pi(_mm_load_ss(p), (const __m64*)((p)+1))

        // Mask used to changes sign of single precision floating point values.
        OGRE_SIMD_ALIGNED_DECL(static const uint32, msSignMask[4]) =
        {
            0x80000000, 0x80000000, 0x80000000, 0x80000000,
        };

        size_t numIterations = numTriangles / 4;
        numTriangles &= 3;

        // Four triangles per-iteration
        for (size_t i = 0; i < numIterations; ++i)
        {

// Load four Vector3 as: (x0, x1, x2, x3), (y0, y1, y2, y3), (z0, z1, z2, z3)
#define __LOAD_FOUR_VECTOR3(x, y, z, p0, p1, p2, p3)                    \
            {                                                           \
                __m128 v0 = __LOAD_VECTOR3(p0);     /* x0 -- y0 z0 */   \
                __m128 v1 = __LOAD_VECTOR3(p1);     /* x1 -- y1 z1 */   \
                __m128 v2 = __LOAD_VECTOR3(p2);     /* x2 -- y2 z2 */   \
                __m128 v3 = __LOAD_VECTOR3(p3);     /* x3 -- y3 z3 */   \
                __m128 t0, t1;                                          \
                                                                        \
                t0 = _mm_unpacklo_ps(v0, v2);       /* x0 x2 -- -- */   \
                t1 = _mm_unpacklo_ps(v1, v3);       /* x1 x3 -- -- */   \
                x  = _mm_unpacklo_ps(t0, t1);       /* x0 x1 x2 x3 */   \
                                                                        \
                t0 = _mm_unpackhi_ps(v0, v2);       /* y0 y2 z0 z2 */   \
                t1 = _mm_unpackhi_ps(v1, v3);       /* y1 y3 z1 z3 */   \
                y  = _mm_unpacklo_ps(t0, t1);       /* y0 y1 y2 y3 */   \
                z  = _mm_unpackhi_ps(t0, t1);       /* z0 z1 z2 z3 */   \
            }

            __m128 x0, x1, x2, y0, y1, y2, z0, z1, z2;

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
            __m128 ax = _mm_sub_ps(x1, x0);
            __m128 ay = _mm_sub_ps(y1, y0);
            __m128 az = _mm_sub_ps(z1, z0);

            // b = v2 - v0
            __m128 bx = _mm_sub_ps(x2, x0);
            __m128 by = _mm_sub_ps(y2, y0);
            __m128 bz = _mm_sub_ps(z2, z0);

            // n = a cross b
            __m128 nx = _mm_sub_ps(_mm_mul_ps(ay, bz), _mm_mul_ps(az, by));
            __m128 ny = _mm_sub_ps(_mm_mul_ps(az, bx), _mm_mul_ps(ax, bz));
            __m128 nz = _mm_sub_ps(_mm_mul_ps(ax, by), _mm_mul_ps(ay, bx));

            // w = - (n dot v0)
            __m128 nw = _mm_xor_ps(
                __MM_DOT3x3_PS(nx, ny, nz, x0, y0, z0),
                *(const __m128 *)&msSignMask);

            // Arrange to per-triangle face normal major format
            __MM_TRANSPOSE4x4_PS(nx, ny, nz, nw);

            // Store results
            __MM_STORE_PS(&faceNormals[0].x, nx);
            __MM_STORE_PS(&faceNormals[1].x, ny);
            __MM_STORE_PS(&faceNormals[2].x, nz);
            __MM_STORE_PS(&faceNormals[3].x, nw);
            faceNormals += 4;

#undef __LOAD_FOUR_VECTOR3
        }

        // Dealing with remaining triangles
        for (size_t j = 0; j < numTriangles; ++j)
        {
            // Load vertices of the triangle
            __m128 v0 = __LOAD_VECTOR3(positions + triangles->vertIndex[0] * 3);
            __m128 v1 = __LOAD_VECTOR3(positions + triangles->vertIndex[1] * 3);
            __m128 v2 = __LOAD_VECTOR3(positions + triangles->vertIndex[2] * 3);
            ++triangles;

            // Calculate face normal

            __m128 t0, t1;

            __m128 a = _mm_sub_ps(v1, v0);                      // ax 0 ay az
            __m128 b = _mm_sub_ps(v2, v0);                      // bx 0 by bz
            t0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,0,1,3));    // az 0 ax ay
            t1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(2,0,1,3));    // bz 0 bx by
            t0 = _mm_mul_ps(t0, b);                             // az*bx 0 ax*by ay*bz
            t1 = _mm_mul_ps(t1, a);                             // ax*bz 0 ay*bx az*by

            __m128 n = _mm_sub_ps(t0, t1);                      // ny 0  nz nx

            __m128 d = _mm_mul_ps(                              // dy 0  dz dx
                _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0,3,1,2)), n);

            n = _mm_sub_ps(_mm_sub_ps(_mm_sub_ps(               // nx ny nz -(dx+dy+dz)
                _mm_shuffle_ps(n, n, _MM_SHUFFLE(1,2,0,3)),     // nx ny nz 0
                _mm_shuffle_ps(d, d, _MM_SHUFFLE(3,1,1,1))),    // 0  0  0  dx
                _mm_shuffle_ps(d, d, _MM_SHUFFLE(0,1,1,1))),    // 0  0  0  dy
                _mm_shuffle_ps(d, d, _MM_SHUFFLE(2,1,1,1)));    // 0  0  0  dz

            // Store result
            __MM_STORE_PS(&faceNormals->x, n);
            ++faceNormals;
        }

#undef __LOAD_VECTOR3
    }
    //---------------------------------------------------------------------
    void OptimisedUtilSSE::calculateLightFacing(
        const Vector4& lightPos,
        const Vector4* faceNormals,
        char* lightFacings,
        size_t numFaces)
    {
        __OGRE_CHECK_STACK_ALIGNED_FOR_SSE();

        assert(_isAlignedForSSE(faceNormals));

        // Map to convert 4-bits mask to 4 byte values
        static const char msMaskMapping[16][4] =
        {
            {0, 0, 0, 0},   {1, 0, 0, 0},   {0, 1, 0, 0},   {1, 1, 0, 0},
            {0, 0, 1, 0},   {1, 0, 1, 0},   {0, 1, 1, 0},   {1, 1, 1, 0},
            {0, 0, 0, 1},   {1, 0, 0, 1},   {0, 1, 0, 1},   {1, 1, 0, 1},
            {0, 0, 1, 1},   {1, 0, 1, 1},   {0, 1, 1, 1},   {1, 1, 1, 1},
        };

        __m128 n0, n1, n2, n3;
        __m128 t0, t1;
        __m128 dp;
        int bitmask;

        // Load light vector, unaligned
        __m128 lp = _mm_loadu_ps(&lightPos.x);

        // Perload zero to register for compare dot product values
        __m128 zero = _mm_setzero_ps();

        size_t numIterations = numFaces / 4;
        numFaces &= 3;

        // Four faces per-iteration
        for (size_t i = 0; i < numIterations; ++i)
        {
            // Load face normals, aligned
            n0 = __MM_LOAD_PS(&faceNormals[0].x);
            n1 = __MM_LOAD_PS(&faceNormals[1].x);
            n2 = __MM_LOAD_PS(&faceNormals[2].x);
            n3 = __MM_LOAD_PS(&faceNormals[3].x);
            faceNormals += 4;

            // Multiply by light vector
            n0 = _mm_mul_ps(n0, lp);        // x0 y0 z0 w0
            n1 = _mm_mul_ps(n1, lp);        // x1 y1 z1 w1
            n2 = _mm_mul_ps(n2, lp);        // x2 y2 z2 w2
            n3 = _mm_mul_ps(n3, lp);        // x3 y3 z3 w3

            // Horizontal add four vector values.
            t0 = _mm_add_ps(                                            // x0+z0 x1+z1 y0+w0 y1+w1
                _mm_unpacklo_ps(n0, n1),    // x0 x1 y0 y1
                _mm_unpackhi_ps(n0, n1));   // z0 z1 w0 w1
            t1 = _mm_add_ps(                                            // x2+z2 x3+z3 y2+w2 y3+w3
                _mm_unpacklo_ps(n2, n3),    // x2 x3 y2 y3
                _mm_unpackhi_ps(n2, n3));   // z2 z3 w2 w3
            dp = _mm_add_ps(                                            // dp0 dp1 dp2 dp3
                _mm_movelh_ps(t0, t1),      // x0+z0 x1+z1 x2+z2 x3+z3
                _mm_movehl_ps(t1, t0));     // y0+w0 y1+w1 y2+w2 y3+w3

            // Compare greater than zero and setup 4-bits mask. Use '_mm_cmpnle_ps'
            // instead of '_mm_cmpgt_ps' here because we want keep 'zero' untouch,
            // i.e. it's 2nd operand of the assembly instruction. And in fact
            // '_mm_cmpgt_ps' was implemented as 'CMPLTPS' with operands swapped
            // in VC7.1.
            bitmask = _mm_movemask_ps(_mm_cmpnle_ps(dp, zero));

            // Convert 4-bits mask to 4 bytes, and store results.
            /*
            *reinterpret_cast<uint32*>(lightFacings) =
                *reinterpret_cast<const uint32*>(msMaskMapping[bitmask]);
                */
            memcpy(lightFacings, msMaskMapping[bitmask], sizeof(uint32));
            
            
            lightFacings += 4;
        }

        // Dealing with remaining faces
        switch (numFaces)
        {
        case 3:
            n0 = __MM_LOAD_PS(&faceNormals[0].x);
            n1 = __MM_LOAD_PS(&faceNormals[1].x);
            n2 = __MM_LOAD_PS(&faceNormals[2].x);

            n0 = _mm_mul_ps(n0, lp);        // x0 y0 z0 w0
            n1 = _mm_mul_ps(n1, lp);        // x1 y1 z1 w1
            n2 = _mm_mul_ps(n2, lp);        // x2 y2 z2 w2

            t0 = _mm_add_ps(                                            // x0+z0 x1+z1 y0+w0 y1+w1
                _mm_unpacklo_ps(n0, n1),    // x0 x1 y0 y1
                _mm_unpackhi_ps(n0, n1));   // z0 z1 w0 w1
            t1 = _mm_add_ps(                                            // x2+z2 x2+z2 y2+w2 y2+w2
                _mm_unpacklo_ps(n2, n2),    // x2 x2 y2 y2
                _mm_unpackhi_ps(n2, n2));   // z2 z2 w2 w2
            dp = _mm_add_ps(                                            // dp0 dp1 dp2 dp2
                _mm_movelh_ps(t0, t1),      // x0+z0 x1+z1 x2+z2 x2+z2
                _mm_movehl_ps(t1, t0));     // y0+w0 y1+w1 y2+w2 y2+w2

            bitmask = _mm_movemask_ps(_mm_cmpnle_ps(dp, zero));

            lightFacings[0] = msMaskMapping[bitmask][0];
            lightFacings[1] = msMaskMapping[bitmask][1];
            lightFacings[2] = msMaskMapping[bitmask][2];
            break;

        case 2:
            n0 = __MM_LOAD_PS(&faceNormals[0].x);
            n1 = __MM_LOAD_PS(&faceNormals[1].x);

            n0 = _mm_mul_ps(n0, lp);        // x0 y0 z0 w0
            n1 = _mm_mul_ps(n1, lp);        // x1 y1 z1 w1

            t0 = _mm_add_ps(                                            // x0+z0 x1+z1 y0+w0 y1+w1
                _mm_unpacklo_ps(n0, n1),    // x0 x1 y0 y1
                _mm_unpackhi_ps(n0, n1));   // z0 z1 w0 w1
            dp = _mm_add_ps(                                            // dp0 dp1 dp0 dp1
                _mm_movelh_ps(t0, t0),      // x0+z0 x1+z1 x0+z0 x1+z1
                _mm_movehl_ps(t0, t0));     // y0+w0 y1+w1 y0+w0 y1+w1

            bitmask = _mm_movemask_ps(_mm_cmpnle_ps(dp, zero));

            lightFacings[0] = msMaskMapping[bitmask][0];
            lightFacings[1] = msMaskMapping[bitmask][1];
            break;

        case 1:
            n0 = __MM_LOAD_PS(&faceNormals[0].x);

            n0 = _mm_mul_ps(n0, lp);        // x0 y0 z0 w0

            t0 = _mm_add_ps(                                            // x0+z0 x0+z0 y0+w0 y0+w0
                _mm_unpacklo_ps(n0, n0),    // x0 x0 y0 y0
                _mm_unpackhi_ps(n0, n0));   // z0 z0 w0 w0
            dp = _mm_add_ps(                                            // dp0 dp0 dp0 dp0
                _mm_movelh_ps(t0, t0),      // x0+z0 x0+z0 x0+z0 x0+z0
                _mm_movehl_ps(t0, t0));     // y0+w0 y0+w0 y0+w0 y0+w0

            bitmask = _mm_movemask_ps(_mm_cmpnle_ps(dp, zero));

            lightFacings[0] = msMaskMapping[bitmask][0];
            break;
        }
    }
    //---------------------------------------------------------------------
    // Template to extrude vertices for directional light.
    template <bool srcAligned, bool destAligned>
    struct ExtrudeVertices_SSE_DirectionalLight
    {
        static void apply(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* pSrcPos,
            float* pDestPos,
            size_t numVertices)
        {
            typedef SSEMemoryAccessor<srcAligned> SrcAccessor;
            typedef SSEMemoryAccessor<destAligned> DestAccessor;

            // Directional light, extrusion is along light direction

            // Load light vector, unaligned
            __m128 lp = _mm_loadu_ps(&lightPos.x);

            // Calculate extrusion direction, note that we use inverted direction here
            // for eliminate an extra negative instruction, we'll compensate for that
            // by use subtract instruction instead later.
            __m128 tmp = _mm_mul_ps(lp, lp);
            tmp = _mm_add_ss(_mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, 1)), _mm_movehl_ps(tmp, tmp));
            // Looks like VC7.1 generate a bit inefficient code for 'rsqrtss', so use 'rsqrtps' instead
            tmp = _mm_mul_ss(_mm_rsqrt_ps(tmp), _mm_load_ss(&extrudeDist));
            __m128 dir = _mm_mul_ps(lp, __MM_SELECT(tmp, 0));               // X Y Z -

            // Prepare extrude direction for extruding 4 vertices parallelly
            __m128 dir0 = _mm_shuffle_ps(dir, dir, _MM_SHUFFLE(0,2,1,0));   // X Y Z X
            __m128 dir1 = _mm_shuffle_ps(dir, dir, _MM_SHUFFLE(1,0,2,1));   // Y Z X Y
            __m128 dir2 = _mm_shuffle_ps(dir, dir, _MM_SHUFFLE(2,1,0,2));   // Z X Y Z

            __m128 s0, s1, s2;
            __m128 d0, d1, d2;

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
                d0 = _mm_sub_ps(s0, dir0);                      // X0 Y0 Z0 X1
                d1 = _mm_sub_ps(s1, dir1);                      // Y1 Z1 X2 Y2
                d2 = _mm_sub_ps(s2, dir2);                      // Z2 X3 Y3 Z3

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
                s2 = _mm_load_ss(pSrcPos + 8);

                // The extrusion direction is inverted, use subtract instruction here
                d0 = _mm_sub_ps(s0, dir0);                      // X0 Y0 Z0 X1
                d1 = _mm_sub_ps(s1, dir1);                      // Y1 Z1 X2 Y2
                d2 = _mm_sub_ss(s2, dir2);                      // Z2 -- -- --

                DestAccessor::store(pDestPos + 0, d0);
                DestAccessor::store(pDestPos + 4, d1);
                _mm_store_ss(pDestPos + 8, d2);
                break;

            case 2:
                // 6 floating-point values
                s0 = SrcAccessor::load(pSrcPos + 0);
                s1 = _mm_loadl_pi(dir1, (const __m64*)(pSrcPos + 4)); // dir1 is meaningless here

                // The extrusion direction is inverted, use subtract instruction here
                d0 = _mm_sub_ps(s0, dir0);                      // X0 Y0 Z0 X1
                d1 = _mm_sub_ps(s1, dir1);                      // Y1 Z1 -- --

                DestAccessor::store(pDestPos + 0, d0);
                _mm_storel_pi((__m64*)(pDestPos + 4), d1);
                break;

            case 1:
                // 3 floating-point values
                s0 = _mm_loadl_pi(dir0, (const __m64*)(pSrcPos + 0)); // dir0 is meaningless here
                s1 = _mm_load_ss(pSrcPos + 2);

                // The extrusion direction is inverted, use subtract instruction here
                d0 = _mm_sub_ps(s0, dir0);                      // X0 Y0 -- --
                d1 = _mm_sub_ss(s1, dir2);                      // Z0 -- -- --

                _mm_storel_pi((__m64*)(pDestPos + 0), d0);
                _mm_store_ss(pDestPos + 2, d1);
                break;
            }
        }
    };
    //---------------------------------------------------------------------
    // Template to extrude vertices for point light.
    template <bool srcAligned, bool destAligned>
    struct ExtrudeVertices_SSE_PointLight
    {
        static void apply(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* pSrcPos,
            float* pDestPos,
            size_t numVertices)
        {
            typedef SSEMemoryAccessor<srcAligned> SrcAccessor;
            typedef SSEMemoryAccessor<destAligned> DestAccessor;

            // Point light, will calculate extrusion direction for every vertex

            // Load light vector, unaligned
            __m128 lp = _mm_loadu_ps(&lightPos.x);

            // Load extrude distance
            __m128 extrudeDist4 = _mm_load_ps1(&extrudeDist);

            size_t numIterations = numVertices / 4;
            numVertices &= 3;

            // Extruding 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Load source positions
                __m128 s0 = SrcAccessor::load(pSrcPos + 0);     // x0 y0 z0 x1
                __m128 s1 = SrcAccessor::load(pSrcPos + 4);     // y1 z1 x2 y2
                __m128 s2 = SrcAccessor::load(pSrcPos + 8);     // z2 x3 y3 z3
                pSrcPos += 12;

                // Arrange to 3x4 component-major for batches calculate
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Calculate unnormalised extrusion direction
                __m128 dx = _mm_sub_ps(s0, __MM_SELECT(lp, 0)); // X0 X1 X2 X3
                __m128 dy = _mm_sub_ps(s1, __MM_SELECT(lp, 1)); // Y0 Y1 Y2 Y3
                __m128 dz = _mm_sub_ps(s2, __MM_SELECT(lp, 2)); // Z0 Z1 Z2 Z3

                // Normalise extrusion direction and multiply by extrude distance
                __m128 tmp = __MM_DOT3x3_PS(dx, dy, dz, dx, dy, dz);
                tmp = _mm_mul_ps(_mm_rsqrt_ps(tmp), extrudeDist4);
                dx = _mm_mul_ps(dx, tmp);
                dy = _mm_mul_ps(dy, tmp);
                dz = _mm_mul_ps(dz, tmp);

                // Calculate extruded positions
                __m128 d0 = _mm_add_ps(dx, s0);
                __m128 d1 = _mm_add_ps(dy, s1);
                __m128 d2 = _mm_add_ps(dz, s2);

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

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
                __m128 src = _mm_loadh_pi(_mm_load_ss(pSrcPos + 0), (const __m64*)(pSrcPos + 1)); // x 0 y z
                pSrcPos += 3;

                // Calculate unnormalised extrusion direction
                __m128 dir = _mm_sub_ps(src, _mm_shuffle_ps(lp, lp, _MM_SHUFFLE(2,1,3,0))); // X 1 Y Z

                // Normalise extrusion direction and multiply by extrude distance
                __m128 tmp = _mm_mul_ps(dir, dir);
                tmp = _mm_add_ss(_mm_add_ss(tmp, _mm_movehl_ps(tmp, tmp)), _mm_shuffle_ps(tmp, tmp, 3));
                // Looks like VC7.1 generate a bit inefficient code for 'rsqrtss', so use 'rsqrtps' instead
                tmp = _mm_mul_ss(_mm_rsqrt_ps(tmp), extrudeDist4);
                dir = _mm_mul_ps(dir, __MM_SELECT(tmp, 0));

                // Calculate extruded position
                __m128 dst = _mm_add_ps(dir, src);

                // Store extruded position
                _mm_store_ss(pDestPos + 0, dst);
                _mm_storeh_pi((__m64*)(pDestPos + 1), dst);
                pDestPos += 3;
            }
        }
    };
    //---------------------------------------------------------------------
    void OptimisedUtilSSE::extrudeVertices(
        const Vector4& lightPos,
        Real extrudeDist,
        const float* pSrcPos,
        float* pDestPos,
        size_t numVertices)
    {
        __OGRE_CHECK_STACK_ALIGNED_FOR_SSE();

        // Note: Since pDestPos is following tail of pSrcPos, we can't assume
        // it's aligned to SIMD alignment properly, so must check for it here.
        //
        // TODO: Add extra vertex to the vertex buffer for make sure pDestPos
        // aligned same as pSrcPos.
        //

        // We are use SSE reciprocal square root directly while calculating
        // extrusion direction, since precision loss not that important here.
        //
        if (lightPos.w == 0.0f)
        {
            if (_isAlignedForSSE(pSrcPos))
            {
                if (_isAlignedForSSE(pDestPos))
                    ExtrudeVertices_SSE_DirectionalLight<true, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_SSE_DirectionalLight<true, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
            else
            {
                if (_isAlignedForSSE(pDestPos))
                    ExtrudeVertices_SSE_DirectionalLight<false, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_SSE_DirectionalLight<false, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
        }
        else
        {
            assert(lightPos.w == 1.0f);

            if (_isAlignedForSSE(pSrcPos))
            {
                if (_isAlignedForSSE(pDestPos))
                    ExtrudeVertices_SSE_PointLight<true, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_SSE_PointLight<true, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
            else
            {
                if (_isAlignedForSSE(pDestPos))
                    ExtrudeVertices_SSE_PointLight<false, true>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
                else
                    ExtrudeVertices_SSE_PointLight<false, false>::apply(
                        lightPos, extrudeDist, pSrcPos, pDestPos, numVertices);
            }
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    extern OptimisedUtil* _getOptimisedUtilSSE(void)
    {
        static OptimisedUtilSSE msOptimisedUtilSSE;
#if defined(__OGRE_SIMD_ALIGN_STACK)
        static OptimisedUtilWithStackAlign msOptimisedUtilWithStackAlign(&msOptimisedUtilSSE);
        return &msOptimisedUtilWithStackAlign;
#else
        return &msOptimisedUtilSSE;
#endif
    }

}

#endif // __OGRE_HAVE_SSE
