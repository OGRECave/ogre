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
#ifndef __BaseInstanceBatchVTF_H__
#define __BaseInstanceBatchVTF_H__

#include "OgreInstanceBatch.h"
#include "OgreTexture.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** Instancing implementation using vertex texture through Vertex Texture Fetch (VTF)
        This implementation has the following advantages:
          - Supports huge amount of instances per batch
          - Supports skinning even with huge ammounts of instances per batch
          - Doesn't need shader constants registers.
          - Best suited for skinned entities

        But beware the disadvantages:
          - VTF is only fast on modern GPUs (ATI Radeon HD 2000+, GeForce 8+ series onwards)
          - On GeForce 6/7 series VTF is too slow
          - VTF isn't (controversely) supported on old ATI X1800 hardware
          - Only one bone weight per vertex is supported
          - GPUs with low memory bandwidth (i.e. laptops and integrated GPUs)
          may perform even worse than no instancing

        Whether this performs great or bad depends on the hardware. It improved up to 4x performance on
        a Intel Core 2 Quad Core X9650 GeForce 8600 GTS, and in an Intel Core 2 Duo P7350 ATI
        Mobility Radeon HD 4650, but went 0.75x slower on an AthlonX2 5000+ integrated nForce 6150 SE
        Each BaseInstanceBatchVTF has it's own texture, which occupies memory in VRAM.
        Approx VRAM usage can be computed by doing 12 bytes * 3 * numInstances * numBones
        Use flag IM_VTFBESTFIT to avoid wasting VRAM (but may reduce amount of instances per batch).

        The material requires at least a texture unit stage named @c InstancingVTF

     */
    class _OgreExport BaseInstanceBatchVTF : public InstanceBatch
    {
    protected:
        typedef std::vector<uint8> HWBoneIdxVec;
        typedef std::vector<float> HWBoneWgtVec;
        typedef std::vector<Matrix4> Matrix4Vec;

        size_t                  mMatricesPerInstance; //number of bone matrices per instance
        size_t                  mNumWorldMatrices;  //Num bones * num instances
        TexturePtr              mMatrixTexture; //The VTF

        //Used when all matrices from each instance must be in the same row (i.e. HW Instancing).
        //A few pixels are wasted, but resizing the texture puts the danger of not sampling the
        //right pixel... (in theory it should work, but in practice doesn't)
        size_t                  mWidthFloatsPadding;
        size_t                  mMaxFloatsPerLine;

        size_t                  mRowLength;
        size_t                  mWeightCount;
        //Temporary array used to store 3x4 matrices before they are converted to dual quaternions
        Matrix3x4f*             mTempTransformsArray3x4;

        // The state of the usage of bone matrix lookup
        bool mUseBoneMatrixLookup;
        size_t mMaxLookupTableInstances;

        bool mUseBoneDualQuaternions;
        bool mForceOneWeight;
        bool mUseOneWeight;

        /** Clones the base material so it can have it's own vertex texture, and also
            clones it's shadow caster materials, if it has any
        */
        void cloneMaterial( const MaterialPtr &material );

        /** Retrieves bone data from the original sub mesh and puts it into an appropriate buffer,
            later to be read when creating the vertex semantics.
            Assumes outBoneIdx has enough space (base submesh vertex count)
        */
        void retrieveBoneIdx( VertexData *baseVertexData, HWBoneIdxVec &outBoneIdx );

        /** @see retrieveBoneIdx()
            Assumes outBoneIdx has enough space (twice the base submesh vertex count, one for each weight)
            Assumes outBoneWgt has enough space (twice the base submesh vertex count, one for each weight)
        */
        void retrieveBoneIdxWithWeights(VertexData *baseVertexData, HWBoneIdxVec &outBoneIdx, HWBoneWgtVec &outBoneWgt);

        /** Setups the material to use a vertex texture */
        void setupMaterialToUseVTF( TextureType textureType, MaterialPtr &material ) const;

        /** Creates the vertex texture */
        void createVertexTexture( const SubMesh* baseSubMesh );

        /** Creates 2 TEXCOORD semantics that will be used to sample the vertex texture */
        virtual void createVertexSemantics( VertexData *thisVertexData, VertexData *baseVertexData,
                                    const HWBoneIdxVec &hwBoneIdx, const HWBoneWgtVec &hwBoneWgt) = 0;

        size_t convert3x4MatricesToDualQuaternions(Matrix3x4f* matrices, size_t numOfMatrices, float* outDualQuaternions);
                                    
        /** Keeps filling the VTF with world matrix data */
        void updateVertexTexture(void);

        /** Affects VTF texture's width dimension */
        virtual bool matricesTogetherPerRow() const = 0;

        /** update the lookup numbers for entities with shared transforms */
        virtual void updateSharedLookupIndexes();

        /** @see InstanceBatch::generateInstancedEntity() */
        InstancedEntity* generateInstancedEntity(size_t num) override;

    public:
        BaseInstanceBatchVTF( InstanceManager *creator, MeshPtr &meshReference, const MaterialPtr &material,
                            size_t instancesPerBatch, const Mesh::IndexMap *indexToBoneMap,
                            const String &batchName);
        virtual ~BaseInstanceBatchVTF();

        /** @see InstanceBatch::buildFrom */
        void buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation ) override;

        //Renderable overloads
        void getWorldTransforms( Matrix4* xform ) const override;

        /** Overloaded to be able to updated the vertex texture */
        void _updateRenderQueue(RenderQueue* queue) override;

        /** Sets the state of the usage of bone matrix lookup
        
        Under default condition each instance entity is assigned a specific area in the vertex 
        texture for bone matrix data. When turned on the amount of area in the vertex texture 
        assigned for bone matrix data will be relative to the amount of unique animation states.
        Instanced entities sharing the same animation state will share the same area in the matrix.
        The specific position of each entity is placed in the vertex data and added in a second phase
        in the shader.

        Note this feature only works in VTF_HW for now.
        This value needs to be set before adding any instanced entities
        */
        void setBoneMatrixLookup(bool enable, size_t maxLookupTableInstances) { assert(mInstancedEntities.empty()); 
            mUseBoneMatrixLookup = enable; mMaxLookupTableInstances = maxLookupTableInstances; }

        /** Tells whether to use bone matrix lookup
        @see setBoneMatrixLookup()
        */
        bool useBoneMatrixLookup() const { return mUseBoneMatrixLookup; }

        void setBoneDualQuaternions(bool enable) { assert(mInstancedEntities.empty());
            mUseBoneDualQuaternions = enable; mRowLength = (mUseBoneDualQuaternions ? 2 : 3); }

        bool useBoneDualQuaternions() const { return mUseBoneDualQuaternions; }

        void setForceOneWeight(bool enable) {  assert(mInstancedEntities.empty());
            mForceOneWeight = enable; }

        bool forceOneWeight() const { return mForceOneWeight; }

        void setUseOneWeight(bool enable) {  assert(mInstancedEntities.empty());
            mUseOneWeight = enable; }

        bool useOneWeight() const { return mUseOneWeight; }

        /** @see InstanceBatch::useBoneWorldMatrices()  */
        bool useBoneWorldMatrices() const override { return !mUseBoneMatrixLookup; }

        /** @return the maximum amount of shared transform entities when using lookup table*/
        virtual size_t getMaxLookupTableInstances() const { return mMaxLookupTableInstances; }
        
    };

    class _OgreExport InstanceBatchVTF : public BaseInstanceBatchVTF
    {
        
        void setupVertices( const SubMesh* baseSubMesh ) override;
        void setupIndices( const SubMesh* baseSubMesh ) override;

        /** Creates 2 TEXCOORD semantics that will be used to sample the vertex texture */
        void createVertexSemantics( VertexData *thisVertexData, VertexData *baseVertexData,
            const HWBoneIdxVec &hwBoneIdx, const HWBoneWgtVec &hwBoneWgt ) override;

        bool matricesTogetherPerRow() const override { return false; }
    public:
        InstanceBatchVTF( InstanceManager *creator, MeshPtr &meshReference, const MaterialPtr &material,
                            size_t instancesPerBatch, const Mesh::IndexMap *indexToBoneMap,
                            const String &batchName);
        virtual ~InstanceBatchVTF();

        /** @see InstanceBatch::calculateMaxNumInstances */
        size_t calculateMaxNumInstances( const SubMesh *baseSubMesh, uint16 flags ) const override;
    };
}

#include "OgreHeaderSuffix.h"

#endif
