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
#ifndef __PatchSurface_H__
#define __PatchSurface_H__

#include "OgrePrerequisites.h"

#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreAxisAlignedBox.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup LOD
    *  @{
    */
    /** A surface which is defined by curves of some kind to form a patch, e.g. a Bezier patch.

        This object will take a list of control points with various assorted data, and will
        subdivide it into a patch mesh. Currently only Bezier curves are supported for defining
        the surface, but other techniques such as NURBS would follow the same basic approach.
    */
    class _OgreExport PatchSurface : public PatchAlloc
    {
    public:
        PatchSurface();
        ~PatchSurface();

        enum PatchSurfaceType
        {
            /// A patch defined by a set of bezier curves
            PST_BEZIER
        };

        /// Constant for indicating automatic determination of subdivision level for patches
        enum
        {
            AUTO_LEVEL = -1
        };

        enum VisibleSide {
            /// The side from which u goes right and v goes up (as in texture coords)
            VS_FRONT,
            /// The side from which u goes right and v goes down (reverse of texture coords)
            VS_BACK,
            /// Both sides are visible - warning this creates 2x the number of triangles and adds extra overhead for calculating normals
            VS_BOTH
        };
        /** Sets up the surface by defining it's control points, type and initial subdivision level.

            This method initialises the surface by passing it a set of control points. The type of curves to be used
            are also defined here, although the only supported option currently is a bezier patch. You can also
            specify a global subdivision level here if you like, although it is recommended that the parameter
            is left as AUTO_LEVEL, which means the system decides how much subdivision is required (based on the
            curvature of the surface)
            @param
                controlPointBuffer A pointer to a buffer containing the vertex data which defines control points 
                of the curves rather than actual vertices. Note that you are expected to provide not
                just position information, but potentially normals and texture coordinates too. The
                format of the buffer is defined in the VertexDeclaration parameter
            @param
                declaration VertexDeclaration describing the contents of the buffer. 
                Note this declaration must _only_ draw on buffer source 0!
            @param
                width Specifies the width of the patch in control points.
            @param
                height Specifies the height of the patch in control points. 
            @param
                pType The type of surface - currently only PST_BEZIER is supported
            @param
                uMaxSubdivisionLevel,vMaxSubdivisionLevel If you want to manually set the top level of subdivision, 
                do it here, otherwise let the system decide.
            @param
                visibleSide Determines which side of the patch (or both) triangles are generated for.
        */
        void defineSurface(void* controlPointBuffer, 
            VertexDeclaration *declaration, size_t width, size_t height,
            PatchSurfaceType pType = PST_BEZIER, 
            size_t uMaxSubdivisionLevel = AUTO_LEVEL, size_t vMaxSubdivisionLevel = AUTO_LEVEL,
            VisibleSide visibleSide = VS_FRONT);

        /** Based on a previous call to defineSurface, establishes the number of vertices required
            to hold this patch at the maximum detail level. 
            @remarks This is useful when you wish to build the patch into external vertex / index buffers.

        */
        size_t getRequiredVertexCount(void) const;
        /** Based on a previous call to defineSurface, establishes the number of indexes required
            to hold this patch at the maximum detail level. 
            @remarks This is useful when you wish to build the patch into external vertex / index buffers.

        */
        size_t getRequiredIndexCount(void) const;

        /** Gets the current index count based on the current subdivision level. */
        size_t getCurrentIndexCount(void) const;
        /// Returns the index offset used by this buffer to write data into the buffer
        size_t getIndexOffset(void) const { return mIndexOffset; }
        /// Returns the vertex offset used by this buffer to write data into the buffer
        size_t getVertexOffset(void) const { return mVertexOffset; }


        /** Gets the bounds of this patch, only valid after calling defineSurface. */
        const AxisAlignedBox& getBounds(void) const;
        /** Gets the radius of the bounding sphere for this patch, only valid after defineSurface 
        has been called. */
        Real getBoundingSphereRadius(void) const;
        /** Tells the system to build the mesh relating to the surface into externally created
            buffers.

            The VertexDeclaration of the vertex buffer must be identical to the one passed into
            defineSurface.  In addition, there must be enough space in the buffer to
            accommodate the patch at full detail level; you should call getRequiredVertexCount
            and getRequiredIndexCount to determine this. This method does not create an internal
            mesh for this patch and so getMesh will return null if you call it after building the
            patch this way.
            @param destVertexBuffer The destination vertex buffer in which to build the patch.
            @param vertexStart The offset at which to start writing vertices for this patch
            @param destIndexBuffer The destination index buffer in which to build the patch.
            @param indexStart The offset at which to start writing indexes for this patch

        */
        void build(HardwareVertexBufferSharedPtr destVertexBuffer, size_t vertexStart,
            HardwareIndexBufferSharedPtr destIndexBuffer, size_t indexStart);

        /** Alters the level of subdivision for this surface.

            This method changes the proportionate detail level of the patch; since
            the U and V directions can have different subdivision levels, this method
            takes a single Real value where 0 is the minimum detail (the control points)
            and 1 is the maximum detail level as supplied to the original call to
            defineSurface.
        */
        void setSubdivisionFactor(Real factor);

        /** Gets the current level of subdivision. */
        Real getSubdivisionFactor(void) const;

        void* getControlPointBuffer(void) const
        {
            return mControlPointBuffer;
        }
        /** Convenience method for telling the patch that the control points have been 
            deleted, since once the patch has been built they are not required. */
        void notifyControlPointBufferDeallocated(void) { 
            mControlPointBuffer = 0;
        }
    private:
        /// Vertex declaration describing the control point buffer
        VertexDeclaration* mDeclaration;
        /// Buffer containing the system-memory control points
        void* mControlPointBuffer;
        /// Type of surface
        PatchSurfaceType mType;
        /// Width in control points
        size_t mCtlWidth;
        /// Height in control points
        size_t mCtlHeight;
        /// TotalNumber of control points
        size_t mCtlCount;
        /// U-direction subdivision level
        size_t mULevel;
        /// V-direction subdivision level
        size_t mVLevel;
        /// Max subdivision level
        size_t mMaxULevel;
        size_t mMaxVLevel;
        /// Width of the subdivided mesh (big enough for max level)
        size_t mMeshWidth;
        /// Height of the subdivided mesh (big enough for max level)
        size_t mMeshHeight;
        /// Which side is visible
        VisibleSide mVSide;

        Real mSubdivisionFactor;

        std::vector<Vector3> mVecCtlPoints;

        /** Internal method for finding the subdivision level given 3 control points.
        */
        size_t findLevel( Vector3& a, Vector3& b, Vector3& c);

        void distributeControlPoints(void* lockedBuffer);
        void subdivideCurve(void* lockedBuffer, size_t startIdx, size_t stepSize, size_t numSteps, size_t iterations);
        void interpolateVertexData(void* lockedBuffer, size_t leftIndex, size_t rightIndex, size_t destIndex);
        void makeTriangles(void);

        size_t getAutoULevel(bool forMax = false);
        size_t getAutoVLevel(bool forMax = false);

        HardwareVertexBufferSharedPtr mVertexBuffer;
        HardwareIndexBufferSharedPtr mIndexBuffer;
        size_t mVertexOffset;
        size_t mIndexOffset;
        size_t mRequiredVertexCount;
        size_t mRequiredIndexCount;
        size_t mCurrIndexCount;

        AxisAlignedBox mAABB;
        Real mBoundingSphere;



    };

    /** @} */
    /** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif
