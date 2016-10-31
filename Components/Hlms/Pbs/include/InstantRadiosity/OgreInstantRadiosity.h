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
            Vector3 diffuse;
            Real radius;
            Vector3 position;
            Vector3 normal;
        };

        typedef vector<RayHit>::type RayHitVec;

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

        size_t          mNumRays;
        Real            mCellSize;
    private:
        RayHitVec       mRayHits;

        typedef map<VertexArrayObject*, MeshData>::type MeshDataMapV2;
        typedef map<v1::RenderOperation, MeshData, OrderRenderOperation>::type MeshDataMapV1;
        MeshDataMapV2   mMeshDataMapV2;
        MeshDataMapV1   mMeshDataMapV1;

        void processLight( Vector3 lightPos, const Vector3 &lightDir,
                           Radian angle, Vector3 lightColour );

        const MeshData* downloadVao( VertexArrayObject *vao );
        const MeshData* downloadRenderOp( const v1::RenderOperation &renderOp );

        void processLight( const Vector3 &lightPos, ObjectData objData, size_t numNodes );
        void processLight( const Vector3 &lightPos, const MeshData meshData,
                           Matrix4 worldMatrix, Vector3 materialDiffuse );

        Vpl convertToVpl( Vector3 lightColour, Vector3 pointOnTri, const RayHit &hit );
        void clusterLights( Vector3 lightPos, Vector3 lightColour );

    public:
        InstantRadiosity( SceneManager *sceneManager, HlmsManager *hlmsManager );
        ~InstantRadiosity();

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
