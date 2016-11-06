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
#ifndef _OgreInstantRadiosity_H_
#define _OgreInstantRadiosity_H_

#include "OgreHlmsPbsPrerequisites.h"
#include "OgreHlmsBufferManager.h"
#include "OgreConstBufferPool.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class _OgreHlmsPbsExport InstantRadiosity
    {
        struct MeshData
        {
            float * RESTRICT_ALIAS vertexPos;
            //float * RESTRICT_ALIAS vertexUv[8];
            /// Index data may be directly pointing to IndexBufferPacked's shadow copy.
            /// Don't free the memory in that case!
            union
            {
                uint8 * RESTRICT_ALIAS indexData;
                uint8 const * RESTRICT_ALIAS indexDataConst;
            };
            size_t  numVertices;
            size_t  numIndices;
            bool    useIndices16bit;
        };

        struct RayHit
        {
            Real    distance;
            //Vector3 pointOnTri;
            Vector3 materialDiffuse;
            Vector3 triVerts[3];
            Vector3 triNormal;
            Vector3 rayDir;
        };
        struct Vpl
        {
            Light *light;
            Vector3 diffuse;
            Vector3 position;
            Vector3 normal;
            Real    numMergedVpls;
        };

        typedef vector<RayHit>::type RayHitVec;
        typedef vector<Vpl>::type VplVec;

        struct OrderRenderOperation
        {
            bool operator () ( const v1::RenderOperation &_l, const v1::RenderOperation &_r ) const;
        };

        SceneManager    *mSceneManager;
        HlmsManager     *mHlmsManager;
    public:
        uint8           mFirstRq;
        uint8           mLastRq;
        uint32          mVisibilityMask;
        uint32          mLightMask;

        /// Number of rays to trace. More usually results in more accuracy. Sometimes really
        /// low values (e.g. 32 rays) may achieve convincing results with high performance, while
        /// high large values (e.g. 10000) achieve more accurate results.
        size_t          mNumRays;
        /// Controls how we cluster multiple VPLs into one averaged VPL. Smaller values generate
        /// more VPLs (reducing performance but improving quality). Bigger values result in less
        /// VPLs (higher performance, less quality)
        Real            mCellSize;
        /// Value ideally in range (0; 1]
        /// When 1, the VPL is placed at exactly the location where the light ray hits the triangle.
        /// At 0.99 it will be placed at 99% the distance from light to the location (i.e. moves away
        /// from the triangle). Using Bias can help with light bleeding, and also allows reducing
        /// mVplMaxRange (thus increasing performance) at the cost of lower accuracy but still
        /// "looking good".
        Real            mBias;

        /// ANY CHANGE TO A mVpl* variable will take effect after calling updateExistingVpls
        /// (or calling build)
        /// How big each VPL should be. Larger ranges leak light more but also are more accurate
        /// in the sections they lit correctly, but they are also get more expensive.
        Real            mVplMaxRange;
        Real            mVplConstAtten;
        Real            mVplLinearAtten;
        Real            mVplQuadAtten;
        /// If all three components of the diffuse colour of a VPL light is below this threshold,
        /// the VPL is removed (useful for improving performance for VPLs that barely contribute
        /// to the scene).
        Real            mVplThreshold;
        /// Tweaks how strong VPL lights should be.
        /// In range (0; inf)
        Real            mVplPowerBoost;
    private:
        VplVec          mVpls;
        RayHitVec       mRayHits;

        typedef map<VertexArrayObject*, MeshData>::type MeshDataMapV2;
        typedef map<v1::RenderOperation, MeshData, OrderRenderOperation>::type MeshDataMapV1;
        MeshDataMapV2   mMeshDataMapV2;
        MeshDataMapV1   mMeshDataMapV1;

        void processLight( Vector3 lightPos, const Quaternion &lightRot, uint8 lightType,
                           Radian angle, Vector3 lightColour,
                           Real attenConst, Real attenLinear, Real attenQuad );

        const MeshData* downloadVao( VertexArrayObject *vao );
        const MeshData* downloadRenderOp( const v1::RenderOperation &renderOp );

        void testLightVsAllObjects( const Vector3 &lightPos, ObjectData objData, size_t numNodes );
        void raycastLightRayVsMesh( const Vector3 &lightPos, const MeshData meshData,
                                    Matrix4 worldMatrix, Vector3 materialDiffuse );

        Vpl convertToVpl( Vector3 lightColour, Vector3 pointOnTri, const RayHit &hit );
        /// Generates the VPLs from a particular lights, and clusters them.
        void generateAndClusterVpls( Vector3 lightPos, Vector3 lightColour,
                                     Real attenConst, Real attenLinear, Real attenQuad );
        /// Clusters the VPL from all lights (these VPLs may have been clustered with other
        /// VPLs from the same light, now we need to do this again with lights from different
        /// clusters)
        void clusterAllVpls(void);

    public:
        InstantRadiosity( SceneManager *sceneManager, HlmsManager *hlmsManager );
        ~InstantRadiosity();

        /// Does nothing if build hasn't been called yet.
        /// Updates VPLs with the latest changes made to all mVpl* variables.
        /// May create/remove VPL lights because of mVplThreshold
        void updateExistingVpls(void);

        /// Clears everything, removing our VPLs. Does not freeMemory.
        /// You will have to call build again to get VPLs again.
        void clear(void);

        void build(void);

        /// "build" will download meshes for raycasting. We will not free
        /// them after build (in case you want to build again).
        /// If you wish to free that memory, call this function.
        void freeMemory(void);
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
