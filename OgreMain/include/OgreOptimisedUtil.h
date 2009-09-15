/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __OptimisedUtil_H__
#define __OptimisedUtil_H__

#include "OgrePrerequisites.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Utility class for provides optimised functions.
    @note
        This class are supposed used by internal engine only.
    */
    class _OgreExport OptimisedUtil
    {
    private:
        /// Privated copy constructor, to prevent misuse
        OptimisedUtil(const OptimisedUtil& rhs); /* do nothing, should not use */
        /// Privated operator=, to prevent misuse
        OptimisedUtil& operator=(const OptimisedUtil& rhs); /* do not use */

    protected:
        /// Store a pointer to the implementation
        static OptimisedUtil* msImplementation;

        /// Detect best implementation based on run-time environment
        static OptimisedUtil* _detectImplementation(void);

    public:
        // Default constructor
        OptimisedUtil(void) {}
        // Destructor
        virtual ~OptimisedUtil() {}

        /** Gets the implementation of this class.
        @note
            Don't cache the pointer returned by this function, it'll change due
            run-time environment detection to pick up the best implementation.
        */
        static OptimisedUtil* getImplementation(void) { return msImplementation; }

        /** Performs software vertex skinning.
        @param srcPosPtr Pointer to source position buffer.
        @param destPosPtr Pointer to destination position buffer.
        @param srcNormPtr Pointer to source normal buffer, if NULL,
            means blend position only.
        @param destNormPtr Pointer to destination normal buffer, it's
            ignored if srcNormPtr is NULL.
        @param blendWeightPtr Pointer to blend weight buffer.
        @param blendIndexPtr Pointer to blend index buffer.
        @param blendMatrices An array of pointer of blend matrix, the matrix
            must be aligned to SIMD alignment, but not necessary for the array
            itself.
        @param srcPosStride The stride of source position in bytes.
        @param destPosStride The stride of destination position in bytes.
        @param srcNormStride The stride of source normal in bytes,
            it's ignored if srcNormPtr is NULL.
        @param destNormStride The stride of destination normal in bytes,
            it's ignored if srcNormPtr is NULL.
        @param blendWeightStride The stride of blend weight buffer in bytes.
        @param blendIndexStride The stride of blend index buffer in bytes.
        @param numWeightsPerVertex Number of blend weights per-vertex, as well
            as for blend indices.
        @param numVertices Number of vertices to blend.
        */
        virtual void softwareVertexSkinning(
            const float *srcPosPtr, float *destPosPtr,
            const float *srcNormPtr, float *destNormPtr,
            const float *blendWeightPtr, const unsigned char* blendIndexPtr,
            const Matrix4* const* blendMatrices,
            size_t srcPosStride, size_t destPosStride,
            size_t srcNormStride, size_t destNormStride,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numVertices) = 0;

        /** Performs a software vertex morph, of the kind used for
            morph animation although it can be used for other purposes. 
        @remarks
            This function will linearly interpolate positions between two
            source buffers, into a third buffer.
        @param t Parametric distance between the start and end positions
        @param srcPos1 Pointer to buffer for the start positions
        @param srcPos2 Pointer to buffer for the end positions
        @param dstPos Pointer to buffer for the destination positions
        @param numVertices Number of vertices to morph, which agree with
            the number in start, end and destination buffer. Bear in mind
            three floating-point values per vertex
        */
        virtual void softwareVertexMorph(
            Real t,
            const float *srcPos1, const float *srcPos2,
            float *dstPos,
            size_t numVertices) = 0;

        /** Concatenate an affine matrix to an array of affine matrices.
        @note
            An affine matrix is a 4x4 matrix with row 3 equal to (0, 0, 0, 1),
            e.g. no projective coefficients.
        @param baseMatrix The matrix used as first operand.
        @param srcMatrices An array of matrix used as second operand.
        @param dstMatrices An array of matrix to store matrix concatenate results.
        @param numMatrices Number of matrices in the array.
        */
        virtual void concatenateAffineMatrices(
            const Matrix4& baseMatrix,
            const Matrix4* srcMatrices,
            Matrix4* dstMatrices,
            size_t numMatrices) = 0;

        /** Calculate the face normals for the triangles based on position
            information.
        @param positions Pointer to position information, which packed in
            (x, y, z) format, indexing by vertex index in the triangle. No
            alignment requests.
        @param triangles The triangles need to calculate face normal, the vertex
            positions is indexed by vertex index to position information.
        @param faceNormals The array of Vector4 used to store triangles face normal,
            Must be aligned to SIMD alignment.
        @param numTriangles Number of triangles to calculate face normal.
        */
        virtual void calculateFaceNormals(
            const float *positions,
            const EdgeData::Triangle *triangles,
            Vector4 *faceNormals,
            size_t numTriangles) = 0;

        /** Calculate the light facing state of the triangle's face normals
        @remarks
            This is normally the first stage of calculating a silhouette, i.e.
            establishing which tris are facing the light and which are facing
            away.
        @param lightPos 4D position of the light in object space, note that
            for directional lights (which have no position), the w component
            is 0 and the x/y/z position are the direction.
        @param faceNormals An array of face normals for the triangles, the face
            normal are unit vector orthogonal to the triangles, plus distance
            from origin. This array must be aligned to SIMD alignment.
        @param lightFacings An array of flags for store light facing state
            results, the result flag is true if corresponding face normal facing
            the light, false otherwise. This array no alignment requires.
        @param numFaces Number of face normals to calculate.
        */
        virtual void calculateLightFacing(
            const Vector4& lightPos,
            const Vector4* faceNormals,
            char* lightFacings,
            size_t numFaces) = 0;

        /** Extruding vertices by a fixed distance based on light position.
        @param lightPos 4D light position, when w=0.0f this represents a
            directional light, otherwise, w must be equal to 1.0f, which
            represents a point light.
        @param extrudeDist The distance to extrude.
        @param srcPositions Pointer to source vertex's position buffer, which
            the position is a 3D vector packed in xyz format. No SIMD alignment
            requirement but loss performance for unaligned data.
        @param destPositions Pointer to destination vertex's position buffer,
            which the position is a 3D vector packed in xyz format. No SIMD
            alignment requirement but loss performance for unaligned data.
        @param numVertices Number of vertices need to extruding, which agree
            with source and destination buffers.
        */
        virtual void extrudeVertices(
            const Vector4& lightPos,
            Real extrudeDist,
            const float* srcPositions,
            float* destPositions,
            size_t numVertices) = 0;
    };

    /** Returns raw offseted of the given pointer.
    @note
        The offset are in bytes, no matter what type of the pointer.
    */
    template <class T>
    static FORCEINLINE T* rawOffsetPointer(T* ptr, ptrdiff_t offset)
    {
        return (T*)((char*)(ptr) + offset);
    }

    /** Advance the pointer with raw offset.
    @note
        The offset are in bytes, no matter what type of the pointer.
    */
    template <class T>
    static FORCEINLINE void advanceRawPointer(T*& ptr, ptrdiff_t offset)
    {
        ptr = rawOffsetPointer(ptr, offset);
    }
	/** @} */
	/** @} */

}

#endif  // __OptimisedUtil_H__
