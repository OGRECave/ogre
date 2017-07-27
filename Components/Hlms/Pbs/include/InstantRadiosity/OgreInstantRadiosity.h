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
#include "OgreRay.h"
#include "OgreRawPtr.h"
#include "OgreVector2.h"
#include "Math/Array/OgreArrayRay.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class RandomNumberGenerator;
    class IrradianceVolume;

    class _OgreHlmsPbsExport InstantRadiosity
    {
        struct MeshData
        {
            float * RESTRICT_ALIAS vertexData;
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

            float* getUvStart( uint8_t uvSet ) const;
        };

        struct MaterialData
        {
            Vector3     diffuse;
            bool        needsUv;
            //TODO: missing translate & scale
            Image const *image[5]; //1 for diffuse, 4 for the detail maps.
            uint8       uvSet[5];
        };

        struct RayHit
        {
            Real    distance;
            Real    accumDistance;
            Ray ray;
            //Vector3 pointOnTri;
            MaterialData material;
            Vector3 triVerts[3];
            Vector3 triNormal;
            Vector2 triUVs[5][3]; //Up to 5 UVs (one per material image)
        };
        struct Vpl
        {
            Light *light;
            Vector3 diffuse;
            Vector3 position;
            Vector3 normal;
            Vector3 dirDiffuse[6]; /// Directional diffuse
            Real    numMergedVpls;
        };

        struct SparseCluster
        {
            int32   blockHash[3];
            Vector3 diffuse;
            Vector3 direction;
            Vector3 dirDiffuse[6];

            SparseCluster();
            SparseCluster( int32 blockX, int32 blockY, int32 blockZ,
                           const Vector3 &_diffuse, const Vector3 &dir,
                           const Vector3 _dirDiffuse[6] );
            SparseCluster( int32 _blockHash[3] );

            bool operator () ( const SparseCluster &_l, int32 _r[3] ) const;
            bool operator () ( int32 _l[3], const SparseCluster &_r ) const;
            bool operator () ( const SparseCluster &_l, const SparseCluster &_r ) const;
        };

        typedef vector<RayHit>::type RayHitVec;
        typedef vector<Vpl>::type VplVec;
        typedef set<SparseCluster, SparseCluster>::type SparseClusterSet;

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
        /// In range [0; inf). Controls how many bounces we'll generate.
        /// Increases the total number of rays (i.e. more than mNumRays).
        size_t          mNumRayBounces;
        /// In range (0; 1]; how many rays that fired in the previous bounce should survive
        /// for a next round of bounces.
        Real            mSurvivingRayFraction;
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
        uint32          mNumSpreadIterations;
        Real            mSpreadThreshold;

        /// Areas of Interest are defined by both AABB and distance (can be 0). AoIs serve two purposes:
        ///     1. Define where to shoot the rays (i.e. windows, holes, etc)
        ///     2. Limit the scope of the rays for performance rasons.
        ///
        /// We will shoot rays towards the AABB from the largest distance between the AABB and the sphere
        /// of radius sphereRadius and center at aabb.mCenter. Only the hits that lay inside
        /// the AABB or the sphere will be considered.
        ///
        /// Example 1:
        /// Let's suppose for performance reasons you want to restrict AoI to four buildings in a city,
        /// In this case you set aabb to encompass all 4 buildings, and most likely sphereRadius to 0.
        ///
        /// Example 2:
        /// You have a big building, but only want to shoot rays against the windows to avoid generating
        /// many unnecesary VPLs that hit the outer walls of the building.
        /// In this case you generate multiple AABBs each with the shape of each window, and set radius
        /// to a large value that will create A SPHERE with sphereRadius and center at aabb.mCenter.
        /// This distance will ensure the rays are hit from a place far enough to hit other walls and
        /// buidings occluding this object (i.e. the sphere should encompass the entire room or the
        /// entire house)
        struct AreaOfInterest
        {
            Aabb    aabb;
            Real    sphereRadius;
            AreaOfInterest( const Aabb &_aabb, Real _sphereRadius ) :
                aabb( _aabb ), sphereRadius( _sphereRadius ) {}
        };

        /// Areas of interest. Only used for directional lights. Normally you don't want to
        /// use Instant Radiosity for empty landscapes because a regular environment map and simple
        /// math can take care of that. You want to focus on a particular building, or
        /// in different cities; but not everything.
        /// If left unfilled, the system will auto-calculate one (not recommended).
        /// See AreaOfInterest
        typedef vector<AreaOfInterest>::type AreaOfInterestVec;
        AreaOfInterestVec   mAoI;

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

        /// When true, mVplIntensityRangeMultiplier will be used and each VPL will have
        /// a dynamic max range (can't exceed mVplMaxRange though), based on its
        /// intensity (smaller VPLs = shorter ranges, powerful VPLs = larger ranges)
        bool            mVplUseIntensityForMaxRange;
        double          mVplIntensityRangeMultiplier;

        uint32          mMipmapBias;
    private:
        size_t          mTotalNumRays; /// Includes bounces. Autogenerated.
        VplVec          mVpls;
        RayHitVec       mRayHits;
        RawSimdUniquePtr<ArrayRay, MEMCATEGORY_GENERAL> mArrayRays;

        FastArray<size_t> mTmpRaysThatHitObject[ARRAY_PACKED_REALS];
        SparseClusterSet  mTmpSparseClusters[3];

        typedef map<VertexArrayObject*, MeshData>::type MeshDataMapV2;
        typedef map<v1::RenderOperation, MeshData, OrderRenderOperation>::type MeshDataMapV1;
        MeshDataMapV2   mMeshDataMapV2;
        MeshDataMapV1   mMeshDataMapV1;

        typedef map<Texture*, Image>::type ImageMap;
        ImageMap        mImageMap;

        vector<Item*>::type mDebugMarkers;
        bool                mEnableDebugMarkers;

        bool                mUseTextures;

        bool                mUseIrradianceVolume;

        /**
        @param lightPos
        @param lightRot
        @param lightType
        @param angle
        @param lightColour
        @param lightRange
        @param attenConst
        @param attenLinear
        @param attenQuad
        @param areaOfInterest
            Only used for directional light types. See mAoI
        */
        void processLight( Vector3 lightPos, const Quaternion &lightRot, uint8 lightType,
                           Radian angle, Vector3 lightColour, Real lightRange,
                           Real attenConst, Real attenLinear, Real attenQuad,
                           const AreaOfInterest &areaOfInterest );

        /// Generates the ray bounces based on mRayHits[raySrcStart] through
        /// mRayHits[raySrcStart+raySrcCount-1]; generating up to 'raysToGenerate' rays
        /// Returns the number of actually generated rays (which is <= raysToGenerate)
        /// The generated rays are stored between mRayHits[raySrcStart+raySrcCount] &
        /// mRayHits[raySrcStart+raySrcCount+returnValue]
        size_t generateRayBounces( size_t raySrcStart, size_t raySrcCount,
                                   size_t raysToGenerate, RandomNumberGenerator &rng);

        const MeshData* downloadVao( VertexArrayObject *vao );
        const MeshData* downloadRenderOp( const v1::RenderOperation &renderOp );
        const Image& downloadTexture( const TexturePtr &texture );

        void testLightVsAllObjects( uint8 lightType, Real lightRange,
                                    ObjectData objData, size_t numNodes,
                                    const AreaOfInterest &areaOfInterest,
                                    size_t rayStart, size_t numRays );
        void raycastLightRayVsMesh( Real lightRange, const MeshData meshData,
                                    Matrix4 worldMatrix, const MaterialData &material,
                                    const FastArray<size_t> &raysThatHitObj );

        Vpl convertToVpl( Vector3 lightColour, Vector3 pointOnTri, const RayHit &hit );
        /// Generates the VPLs from a particular lights, and clusters them.
        void generateAndClusterVpls( Vector3 lightColour, Real attenConst,
                                     Real attenLinear, Real attenQuad );
        void spreadSparseClusters( const SparseClusterSet &grid0, SparseClusterSet &inOutGrid1 );
        void createVplsFromSpreadClusters( const SparseClusterSet &spreadCluster );
        /// Clusters the VPL from all lights (these VPLs may have been clustered with other
        /// VPLs from the same light, now we need to do this again with lights from different
        /// clusters)
        void clusterAllVpls(void);
        void autogenerateAreaOfInterest(void);

        /// lightDir is normalized
        static void mergeDirectionalDiffuse( const Vector3 &diffuse, const Vector3 &lightDir,
                                             Vector3 *inOutDirDiffuse );

        void createDebugMarkers(void);
        void destroyDebugMarkers(void);

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

        void setEnableDebugMarkers( bool bEnable );
        bool getEnableDebugMarkers(void) const      { return mEnableDebugMarkers; }

        /** Whether to evaluate diffuse & detail map textures.
            Disabling textures can speed up build() time and significantly reduce
            memory consumption, at the expense of more innacurate results (only
            material diffuse colour will be considered)
        @remarks
            Changing this setting implies calling freeMemory!
        @param bUseTextures
            Whether to enable or disable using diffuse textures (and detail maps).
        */
        void setUseTextures( bool bUseTextures );
        bool getUseTextures(void) const             { return mUseTextures; }

        /** Whether to use Irradiance Volume instead of VPLs.
        @param bUseIrradianceVolume
            Whether to use Irradiance Volume.
        */
        void setUseIrradianceVolume(bool bUseIrradianceVolume);
        bool getUseIrradianceVolume(void) const             { return mUseIrradianceVolume; }

        /** Outputs suggested parameters for a volumetric texture that will encompass all
            VPLs. They are suggestions, you don't have to follow them.
        @param inCellSize
            The size of the voxel size. Doesn't have to match mCellSize. The suggested
            output parameters will be based on this input parameter.
        @param outVolumeOrigin
            Where the volume should start. This value will be in the same unit of measure
            you are working with (your Items/Entities, mCellSize).
            If your Items and mCellSize are in centimeters, this value will be in cm.
            If you've been working in meters, it will be in meters.
            This value will be quantized to increments of mCellSize, and that's the only
            requirement (we'll quantize it for you if you change it later).
        @param outLightMaxPower
            The maximum light power of the brightest VPL. Useful to maximize the quality
            of the 10-bits we use for the 3D texture.
        @param outNumBlocksX
            The suggested with for the volume texture. Volume's width in units will be:
                outVolumeOrigin.x + mCellSize * outNumBlocksX;
        @param outNumBlocksY
            The suggested with for the volume texture times 6. Volume's height in units will be:
                outVolumeOrigin.y + mCellSize * outNumBlocksY;
        @param outNumBlocksZ
            The suggested depth for the volume texture times. Volume's depth in units will be:
                outVolumeOrigin.z + mCellSize * outNumBlocksZ;
        */
        void suggestIrradianceVolumeParameters( const Vector3 &inCellSize,
                                                Vector3 &outVolumeOrigin,
                                                Real &outLightMaxPower,
                                                uint32 &outNumBlocksX,
                                                uint32 &outNumBlocksY,
                                                uint32 &outNumBlocksZ );

        /**
        @param volume
        @param cellSize
        @param volumeOrigin
        @param lightMaxPower
        @param fadeAttenuationOverDistance
            Whether to fade the attenuation with distance (not physically based).
            See ForwardPlusBase::setFadeAttenuationRange
        */
        void fillIrradianceVolume( IrradianceVolume *volume,
                                   Vector3 cellSize, Vector3 volumeOrigin, Real lightMaxPower,
                                   bool fadeAttenuationOverDistance );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
