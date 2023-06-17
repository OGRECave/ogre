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

namespace Ogre {

//-------------------------------------------------------------------------
// Local classes
//-------------------------------------------------------------------------

    /** General implementation of OptimisedUtil.
    @note
        Don't use this class directly, use OptimisedUtil instead.
    */
    class _OgrePrivate OptimisedUtilGeneral : public OptimisedUtil
    {
    public:
        /// @copydoc OptimisedUtil::softwareVertexSkinning
        void softwareVertexSkinning(
            const float *srcPosPtr, float *destPosPtr,
            const float *srcNormPtr, float *destNormPtr,
            const float *blendWeightPtr, const unsigned char* blendIndexPtr,
            const Affine3* const* blendMatrices,
            size_t srcPosStride, size_t destPosStride,
            size_t srcNormStride, size_t destNormStride,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numVertices) override;

        /// @copydoc OptimisedUtil::softwareVertexMorph
        void softwareVertexMorph(
            float t,
            const float *srcPos1, const float *srcPos2,
            float *dstPos,
            size_t pos1VSize, size_t pos2VSize, size_t dstVSize, 
            size_t numVertices,
            bool morphNormals) override;

        /// @copydoc OptimisedUtil::concatenateAffineMatrices
        void concatenateAffineMatrices(
            const Affine3& baseMatrix,
            const Affine3* srcMatrices,
            Affine3* dstMatrices,
            size_t numMatrices) override;

        /// @copydoc OptimisedUtil::calculateFaceNormals
        void calculateFaceNormals(
            const float *positions,
            const EdgeData::Triangle *triangles,
            Vector4 *faceNormals,
            size_t numTriangles) override;

        /// @copydoc OptimisedUtil::calculateLightFacing
        void calculateLightFacing(
            const Vector4& lightPos,
            const Vector4* faceNormals,
            char* lightFacings,
            size_t numFaces) override;

        /// @copydoc OptimisedUtil::extrudeVertices
        void extrudeVertices(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* srcPositions,
            float* destPositions,
            size_t numVertices) override;
    };
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::softwareVertexSkinning(
        const float *pSrcPos, float *pDestPos,
        const float *pSrcNorm, float *pDestNorm,
        const float *pBlendWeight, const unsigned char* pBlendIndex,
        const Affine3* const* blendMatrices,
        size_t srcPosStride, size_t destPosStride,
        size_t srcNormStride, size_t destNormStride,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numVertices)
    {
        // Source vectors
        Vector3 sourceVec = Vector3::ZERO, sourceNorm = Vector3::ZERO;
        // Accumulation vectors
        Vector3 accumVecPos, accumVecNorm;

        // Loop per vertex
        for (size_t vertIdx = 0; vertIdx < numVertices; ++vertIdx)
        {
            // Load source vertex elements
            sourceVec.x = pSrcPos[0];
            sourceVec.y = pSrcPos[1];
            sourceVec.z = pSrcPos[2];

            if (pSrcNorm)
            {
                sourceNorm.x = pSrcNorm[0];
                sourceNorm.y = pSrcNorm[1];
                sourceNorm.z = pSrcNorm[2];
            }

            // Load accumulators
            accumVecPos = Vector3::ZERO;
            accumVecNorm = Vector3::ZERO;

            // Loop per blend weight
            //
            // Note: Don't change "unsigned short" here!!! If use "size_t" instead,
            // VC7.1 unroll this loop to four blend weights pre-iteration, and then
            // loss performance 10% in this function. Ok, this give a hint that we
            // should unroll this loop manually for better performance, will do that
            // later.
            //
            for (unsigned short blendIdx = 0; blendIdx < numWeightsPerVertex; ++blendIdx)
            {
                // Blend by multiplying source by blend matrix and scaling by weight
                // Add to accumulator
                // NB weights must be normalised!!
                Real weight = pBlendWeight[blendIdx];
                if (weight)
                {
                    // Blend position, use 3x4 matrix
                    const Affine3& mat = *blendMatrices[pBlendIndex[blendIdx]];
                    accumVecPos.x +=
                        (mat[0][0] * sourceVec.x +
                         mat[0][1] * sourceVec.y +
                         mat[0][2] * sourceVec.z +
                         mat[0][3])
                         * weight;
                    accumVecPos.y +=
                        (mat[1][0] * sourceVec.x +
                         mat[1][1] * sourceVec.y +
                         mat[1][2] * sourceVec.z +
                         mat[1][3])
                         * weight;
                    accumVecPos.z +=
                        (mat[2][0] * sourceVec.x +
                         mat[2][1] * sourceVec.y +
                         mat[2][2] * sourceVec.z +
                         mat[2][3])
                         * weight;
                    if (pSrcNorm)
                    {
                        // Blend normal
                        // We should blend by inverse transpose here, but because we're assuming the 3x3
                        // aspect of the matrix is orthogonal (no non-uniform scaling), the inverse transpose
                        // is equal to the main 3x3 matrix
                        // Note because it's a normal we just extract the rotational part, saves us renormalising here
                        accumVecNorm.x +=
                            (mat[0][0] * sourceNorm.x +
                             mat[0][1] * sourceNorm.y +
                             mat[0][2] * sourceNorm.z)
                             * weight;
                        accumVecNorm.y +=
                            (mat[1][0] * sourceNorm.x +
                             mat[1][1] * sourceNorm.y +
                             mat[1][2] * sourceNorm.z)
                            * weight;
                        accumVecNorm.z +=
                            (mat[2][0] * sourceNorm.x +
                             mat[2][1] * sourceNorm.y +
                             mat[2][2] * sourceNorm.z)
                            * weight;
                    }
                }
            }

            // Stored blended vertex in hardware buffer
            pDestPos[0] = accumVecPos.x;
            pDestPos[1] = accumVecPos.y;
            pDestPos[2] = accumVecPos.z;

            // Stored blended vertex in temp buffer
            if (pSrcNorm)
            {
                // Normalise
                accumVecNorm.normalise();
                pDestNorm[0] = accumVecNorm.x;
                pDestNorm[1] = accumVecNorm.y;
                pDestNorm[2] = accumVecNorm.z;
                // Advance pointers
                advanceRawPointer(pSrcNorm, srcNormStride);
                advanceRawPointer(pDestNorm, destNormStride);
            }

            // Advance pointers
            advanceRawPointer(pSrcPos, srcPosStride);
            advanceRawPointer(pDestPos, destPosStride);
            advanceRawPointer(pBlendWeight, blendWeightStride);
            advanceRawPointer(pBlendIndex, blendIndexStride);
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::concatenateAffineMatrices(
        const Affine3& baseMatrix,
        const Affine3* pSrcMat,
        Affine3* pDstMat,
        size_t numMatrices)
    {
        for (size_t i = 0; i < numMatrices; ++i)
        {
            *pDstMat = baseMatrix * *pSrcMat ;

            ++pSrcMat;
            ++pDstMat;
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::softwareVertexMorph(
        float t,
        const float *pSrc1, const float *pSrc2,
        float *pDst,
        size_t pos1VSize, size_t pos2VSize, size_t dstVSize,
        size_t numVertices,
        bool morphNormals)
    {
        size_t src1Skip = pos1VSize/sizeof(float) - 3 - (morphNormals ? 3 : 0);
        size_t src2Skip = pos2VSize/sizeof(float) - 3 - (morphNormals ? 3 : 0);
        size_t dstSkip = dstVSize/sizeof(float) - 3 - (morphNormals ? 3 : 0);
        
        Vector3f nlerpNormal;
        for (size_t i = 0; i < numVertices; ++i)
        {
            // x
            *pDst++ = *pSrc1 + t * (*pSrc2 - *pSrc1) ;
            ++pSrc1; ++pSrc2;
            // y
            *pDst++ = *pSrc1 + t * (*pSrc2 - *pSrc1) ;
            ++pSrc1; ++pSrc2;
            // z
            *pDst++ = *pSrc1 + t * (*pSrc2 - *pSrc1) ;
            ++pSrc1; ++pSrc2;
            
            if (morphNormals)
            {
                // normals must be in the same buffer as pos
                // perform an nlerp
                // we don't have enough information for a spherical interp
                nlerpNormal[0] = *pSrc1 + t * (*pSrc2 - *pSrc1);
                ++pSrc1; ++pSrc2;
                nlerpNormal[1] = *pSrc1 + t * (*pSrc2 - *pSrc1);
                ++pSrc1; ++pSrc2;
                nlerpNormal[2] = *pSrc1 + t * (*pSrc2 - *pSrc1);
                ++pSrc1; ++pSrc2;
                nlerpNormal.normalise();
                *pDst++ = nlerpNormal[0];
                *pDst++ = nlerpNormal[1];
                *pDst++ = nlerpNormal[2];
            }
            
            pSrc1 += src1Skip;
            pSrc2 += src2Skip;
            pDst += dstSkip;
            
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::calculateFaceNormals(
        const float *positions,
        const EdgeData::Triangle *triangles,
        Vector4 *faceNormals,
        size_t numTriangles)
    {
        for ( ; numTriangles; --numTriangles)
        {
            const EdgeData::Triangle& t = *triangles++;
            size_t offset;

            offset = t.vertIndex[0] * 3;
            Vector3 v1(positions[offset+0], positions[offset+1], positions[offset+2]);

            offset = t.vertIndex[1] * 3;
            Vector3 v2(positions[offset+0], positions[offset+1], positions[offset+2]);

            offset = t.vertIndex[2] * 3;
            Vector3 v3(positions[offset+0], positions[offset+1], positions[offset+2]);

            *faceNormals++ = Math::calculateFaceNormalWithoutNormalize(v1, v2, v3);
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::calculateLightFacing(
        const Vector4& lightPos,
        const Vector4* faceNormals,
        char* lightFacings,
        size_t numFaces)
    {
        for (size_t i = 0; i < numFaces; ++i)
        {
            *lightFacings++ = (lightPos.dotProduct(*faceNormals++) > 0);
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::extrudeVertices(
        const Vector4& lightPos,
        Real extrudeDist,
        const float* pSrcPos,
        float* pDestPos,
        size_t numVertices)
    {
        if (lightPos.w == 0.0f)
        {
            // Directional light, extrusion is along light direction

            Vector3 extrusionDir(
                -lightPos.x,
                -lightPos.y,
                -lightPos.z);
            extrusionDir.normalise();
            extrusionDir *= extrudeDist;

            for (size_t vert = 0; vert < numVertices; ++vert)
            {
                *pDestPos++ = *pSrcPos++ + extrusionDir.x;
                *pDestPos++ = *pSrcPos++ + extrusionDir.y;
                *pDestPos++ = *pSrcPos++ + extrusionDir.z;
            }
        }
        else
        {
            // Point light, calculate extrusionDir for every vertex
            assert(lightPos.w == 1.0f);

            for (size_t vert = 0; vert < numVertices; ++vert)
            {
                Vector3 extrusionDir(
                    pSrcPos[0] - lightPos.x,
                    pSrcPos[1] - lightPos.y,
                    pSrcPos[2] - lightPos.z);
                extrusionDir.normalise();
                extrusionDir *= extrudeDist;

                *pDestPos++ = *pSrcPos++ + extrusionDir.x;
                *pDestPos++ = *pSrcPos++ + extrusionDir.y;
                *pDestPos++ = *pSrcPos++ + extrusionDir.z;
            }
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    extern OptimisedUtil* _getOptimisedUtilGeneral(void);
    extern OptimisedUtil* _getOptimisedUtilGeneral(void)
    {
        static OptimisedUtilGeneral msOptimisedUtilGeneral;
        return &msOptimisedUtilGeneral;
    }

}
