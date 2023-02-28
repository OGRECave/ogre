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

#include "OgreSkeletonManager.h"
#include "OgreEdgeListBuilder.h"
#include "OgreAnimation.h"
#include "OgreAnimationState.h"
#include "OgreAnimationTrack.h"
#include "OgreOptimisedUtil.h"
#include "OgreTangentSpaceCalc.h"
#include "OgreLodStrategyManager.h"
#include "OgrePixelCountLodStrategy.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    Mesh::Mesh(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader),
        mBoundRadius(0.0f),
        mBoneBoundingRadius(0.0f),
        mBoneAssignmentsOutOfDate(false),
        mLodStrategy(LodStrategyManager::getSingleton().getDefaultStrategy()),
        mHasManualLodLevel(false),
        mNumLods(1),
        mBufferManager(0),
        mVertexBufferUsage(HardwareBuffer::HBU_STATIC_WRITE_ONLY),
        mIndexBufferUsage(HardwareBuffer::HBU_STATIC_WRITE_ONLY),
        mVertexBufferShadowBuffer(false),
        mIndexBufferShadowBuffer(false),
        mPreparedForShadowVolumes(false),
        mEdgeListsBuilt(false),
        mAutoBuildEdgeLists(false), // will be set to true by serializers of 1.20 and below
        mSharedVertexDataAnimationType(VAT_NONE),
        mSharedVertexDataAnimationIncludesNormals(false),
        mAnimationTypesDirty(true),
        mPosesIncludeNormals(false),
        sharedVertexData(0)
    {
        // Init first (manual) lod
        MeshLodUsage lod;
        lod.userValue = 0; // User value not used for base LOD level
        lod.value = getLodStrategy()->getBaseValue();
        lod.edgeData = NULL;
        lod.manualMesh.reset();
        mMeshLodUsageList.push_back(lod);
    }
    //-----------------------------------------------------------------------
    Mesh::~Mesh()
    {
        if (!HardwareBufferManager::getSingletonPtr()) // LogManager might be also gone already
        {
            printf("ERROR: '%s' is being destroyed after HardwareBufferManager. This is a bug in user code.\n", mName.c_str());
            OgreAssert(false,  "Mesh destroyed after HardwareBufferManager"); // assert in debug mode
            return; // try not to crash
        }
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    //-----------------------------------------------------------------------
    HardwareBufferManagerBase* Mesh::getHardwareBufferManager()
    {
        return mBufferManager ? mBufferManager : HardwareBufferManager::getSingletonPtr();
    }
    //-----------------------------------------------------------------------
    SubMesh* Mesh::createSubMesh()
    {
        SubMesh* sub = OGRE_NEW SubMesh();
        sub->parent = this;

        mSubMeshList.push_back(sub);

        if (isLoaded())
            _dirtyState();

        return sub;
    }
    //-----------------------------------------------------------------------
    SubMesh* Mesh::createSubMesh(const String& name)
    {
        SubMesh *sub = createSubMesh();
        nameSubMesh(name, (ushort)mSubMeshList.size()-1);
        return sub ;
    }
    //-----------------------------------------------------------------------
    void Mesh::destroySubMesh(unsigned short index)
    {
        OgreAssert(index < mSubMeshList.size(), "");
        SubMeshList::iterator i = mSubMeshList.begin();
        std::advance(i, index);
        OGRE_DELETE *i;
        mSubMeshList.erase(i);
        
        // Fix up any name/index entries
        for(SubMeshNameMap::iterator ni = mSubMeshNameMap.begin(); ni != mSubMeshNameMap.end();)
        {
            if (ni->second == index)
            {
                SubMeshNameMap::iterator eraseIt = ni++;
                mSubMeshNameMap.erase(eraseIt);
            }
            else
            {
                // reduce indexes following
                if (ni->second > index)
                    ni->second = ni->second - 1;

                ++ni;
            }
        }

        // fix edge list data by simply recreating all edge lists
        if( mEdgeListsBuilt)
        {
            this->freeEdgeList();
            this->buildEdgeList();
        }

        if (isLoaded())
            _dirtyState();
        
    }
    //-----------------------------------------------------------------------
    void Mesh::destroySubMesh(const String& name)
    {
        unsigned short index = _getSubMeshIndex(name);
        destroySubMesh(index);
    }
    //---------------------------------------------------------------------
    void Mesh::nameSubMesh(const String& name, ushort index)
    {
        mSubMeshNameMap[name] = index ;
    }

    //---------------------------------------------------------------------
    void Mesh::unnameSubMesh(const String& name)
    {
        SubMeshNameMap::iterator i = mSubMeshNameMap.find(name);
        if (i != mSubMeshNameMap.end())
            mSubMeshNameMap.erase(i);
    }
    //-----------------------------------------------------------------------
    SubMesh* Mesh::getSubMesh(const String& name) const
    {
        ushort index = _getSubMeshIndex(name);
        return getSubMesh(index);
    }
    //-----------------------------------------------------------------------
    void Mesh::postLoadImpl(void)
    {
        // Prepare for shadow volumes?
        if (MeshManager::getSingleton().getPrepareAllMeshesForShadowVolumes())
        {
            if (mEdgeListsBuilt || mAutoBuildEdgeLists)
            {
                prepareForShadowVolume();
            }

            if (!mEdgeListsBuilt && mAutoBuildEdgeLists)
            {
                buildEdgeList();
            }
        }
#if !OGRE_NO_MESHLOD
        // The loading process accesses LOD usages directly, so
        // transformation of user values must occur after loading is complete.

        // Transform user LOD values (starting at index 1, no need to transform base value)
        for (MeshLodUsageList::iterator i = mMeshLodUsageList.begin() + 1; i != mMeshLodUsageList.end(); ++i)
            i->value = mLodStrategy->transformUserValue(i->userValue);
        // Rewrite first value
        mMeshLodUsageList[0].value = mLodStrategy->getBaseValue();
#endif
    }
    //-----------------------------------------------------------------------
    void Mesh::prepareImpl()
    {
        // Load from specified 'name'
        if (getCreator()->getVerbose())
            LogManager::getSingleton().logMessage("Mesh: Loading "+mName+".");

        mFreshFromDisk =
            ResourceGroupManager::getSingleton().openResource(
                mName, mGroup, this);
 
        // fully prebuffer into host RAM
        mFreshFromDisk = DataStreamPtr(OGRE_NEW MemoryDataStream(mName,mFreshFromDisk));
    }
    //-----------------------------------------------------------------------
    void Mesh::unprepareImpl()
    {
        mFreshFromDisk.reset();
    }
    void Mesh::loadImpl()
    {
        // If the only copy is local on the stack, it will be cleaned
        // up reliably in case of exceptions, etc
        DataStreamPtr data(mFreshFromDisk);
        mFreshFromDisk.reset();

        if (!data) {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Data doesn't appear to have been prepared in " + mName,
                        "Mesh::loadImpl()");
        }

        String baseName, strExt;
        StringUtil::splitBaseFilename(mName, baseName, strExt);
        auto codec = Codec::getCodec(strExt);
        if (!codec)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "No codec found to load " + mName);

        codec->decode(data, this);
    }

    //-----------------------------------------------------------------------
    void Mesh::unloadImpl()
    {
        // Teardown submeshes
        for (auto & i : mSubMeshList)
        {
            OGRE_DELETE i;
        }
        if (sharedVertexData)
        {
            OGRE_DELETE sharedVertexData;
            sharedVertexData = NULL;
        }
        // Clear SubMesh lists
        mSubMeshList.clear();
        mSubMeshNameMap.clear();

        freeEdgeList();
#if !OGRE_NO_MESHLOD
        // Removes all LOD data
        removeLodLevels();
#endif
        mPreparedForShadowVolumes = false;

        // remove all poses & animations
        removeAllAnimations();
        removeAllPoses();

        // Clear bone assignments
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = false;

        // Removes reference to skeleton
        mSkeleton.reset();
    }
    //-----------------------------------------------------------------------
    void Mesh::reload(LoadingFlags flags)
    {
        bool wasPreparedForShadowVolumes = mPreparedForShadowVolumes;
        bool wasEdgeListsBuilt = mEdgeListsBuilt;
        bool wasAutoBuildEdgeLists = mAutoBuildEdgeLists;

        Resource::reload(flags);

        if(flags & LF_PRESERVE_STATE)
        {
            if(wasPreparedForShadowVolumes)
                prepareForShadowVolume();
            if(wasEdgeListsBuilt)
                buildEdgeList();
            setAutoBuildEdgeLists(wasAutoBuildEdgeLists);
        }
    }
    //-----------------------------------------------------------------------
    MeshPtr Mesh::clone(const String& newName, const String& newGroup)
    {
        // This is a bit like a copy constructor, but with the additional aspect of registering the clone with
        //  the MeshManager

        // New Mesh is assumed to be manually defined rather than loaded since you're cloning it for a reason
        String theGroup = newGroup.empty() ? this->getGroup() : newGroup;
        MeshPtr newMesh = MeshManager::getSingleton().createManual(newName, theGroup);

        if(!newMesh) // interception by collision handler
            return newMesh;

        newMesh->mBufferManager = mBufferManager;
        newMesh->mVertexBufferUsage = mVertexBufferUsage;
        newMesh->mIndexBufferUsage = mIndexBufferUsage;
        newMesh->mVertexBufferShadowBuffer = mVertexBufferShadowBuffer;
        newMesh->mIndexBufferShadowBuffer = mIndexBufferShadowBuffer;

        // Copy submeshes first
        for (auto *subi : mSubMeshList)
        {
            subi->clone("", newMesh.get());
        }

        // Copy shared geometry and index map, if any
        if (sharedVertexData)
        {
            newMesh->sharedVertexData = sharedVertexData->clone(true, mBufferManager);
            newMesh->sharedBlendIndexToBoneIndexMap = sharedBlendIndexToBoneIndexMap;
        }

        // Copy submesh names
        newMesh->mSubMeshNameMap = mSubMeshNameMap ;
        // Copy any bone assignments
        newMesh->mBoneAssignments = mBoneAssignments;
        newMesh->mBoneAssignmentsOutOfDate = mBoneAssignmentsOutOfDate;
        // Copy bounds
        newMesh->mAABB = mAABB;
        newMesh->mBoundRadius = mBoundRadius;
        newMesh->mBoneBoundingRadius = mBoneBoundingRadius;
        newMesh->mAutoBuildEdgeLists = mAutoBuildEdgeLists;
        newMesh->mEdgeListsBuilt = mEdgeListsBuilt;

#if !OGRE_NO_MESHLOD
        newMesh->mHasManualLodLevel = mHasManualLodLevel;
        newMesh->mLodStrategy = mLodStrategy;
        newMesh->mNumLods = mNumLods;
        newMesh->mMeshLodUsageList = mMeshLodUsageList;
#endif
        // Unreference edge lists, otherwise we'll delete the same lot twice, build on demand
        MeshLodUsageList::iterator lodOldi;
        lodOldi = mMeshLodUsageList.begin();
        for (auto& lodi : newMesh->mMeshLodUsageList) {
            MeshLodUsage& lod = *lodOldi;
            lodi.manualName = lod.manualName;
            lodi.userValue = lod.userValue;
            lodi.value = lod.value;
            if (lod.edgeData) {
                lodi.edgeData = lod.edgeData->clone();
            }
            ++lodOldi;
        }

        newMesh->mSkeleton = mSkeleton;

        // Keep prepared shadow volume info (buffers may already be prepared)
        newMesh->mPreparedForShadowVolumes = mPreparedForShadowVolumes;

        newMesh->mEdgeListsBuilt = mEdgeListsBuilt;
        
        // Clone vertex animation
        for (auto & i : mAnimationsList)
        {
            Animation *newAnim = i.second->clone(i.second->getName());
            newMesh->mAnimationsList[i.second->getName()] = newAnim;
        }
        // Clone pose list
        for (auto & i : mPoseList)
        {
            Pose* newPose = i->clone();
            newMesh->mPoseList.push_back(newPose);
        }
        newMesh->mSharedVertexDataAnimationType = mSharedVertexDataAnimationType;
        newMesh->mAnimationTypesDirty = true;

        newMesh->load();
        newMesh->touch();

        return newMesh;
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& Mesh::getBounds(void) const
    {
        return mAABB;
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBounds(const AxisAlignedBox& bounds, bool pad)
    {
        mAABB = bounds;
        mBoundRadius = Math::boundingRadiusFromAABB(mAABB);

        if( mAABB.isFinite() )
        {
            Vector3 max = mAABB.getMaximum();
            Vector3 min = mAABB.getMinimum();

            if (pad)
            {
                // Pad out the AABB a little, helps with most bounds tests
                Vector3 scaler = (max - min) * MeshManager::getSingleton().getBoundsPaddingFactor();
                mAABB.setExtents(min  - scaler, max + scaler);
                // Pad out the sphere a little too
                mBoundRadius = mBoundRadius + (mBoundRadius * MeshManager::getSingleton().getBoundsPaddingFactor());
            }
        }
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBoundingSphereRadius(Real radius)
    {
        mBoundRadius = radius;
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBoneBoundingRadius(Real radius)
    {
        mBoneBoundingRadius = radius;
    }
    //-----------------------------------------------------------------------
    void Mesh::_updateBoundsFromVertexBuffers(bool pad)
    {
        bool extendOnly = false; // First time we need full AABB of the given submesh, but on the second call just extend that one.
        if (sharedVertexData){
            _calcBoundsFromVertexBuffer(sharedVertexData, mAABB, mBoundRadius, extendOnly);
            extendOnly = true;
        }
        for (auto & i : mSubMeshList){
            if (i->vertexData){
                _calcBoundsFromVertexBuffer(i->vertexData, mAABB, mBoundRadius, extendOnly);
                extendOnly = true;
            }
        }
        if (pad)
        {
            Vector3 max = mAABB.getMaximum();
            Vector3 min = mAABB.getMinimum();
            // Pad out the AABB a little, helps with most bounds tests
            Vector3 scaler = (max - min) * MeshManager::getSingleton().getBoundsPaddingFactor();
            mAABB.setExtents(min - scaler, max + scaler);
            // Pad out the sphere a little too
            mBoundRadius = mBoundRadius + (mBoundRadius * MeshManager::getSingleton().getBoundsPaddingFactor());
        }
    }
    void Mesh::_calcBoundsFromVertexBuffer(VertexData* vertexData, AxisAlignedBox& outAABB, Real& outRadius, bool extendOnly /*= false*/)
    {
        if (vertexData->vertexCount == 0) {
            if (!extendOnly) {
                outAABB = AxisAlignedBox(Vector3::ZERO, Vector3::ZERO);
                outRadius = 0;
            }
            return;
        }
        const VertexElement* elemPos = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(elemPos->getSource());
        HardwareBufferLockGuard vertexLock(vbuf, HardwareBuffer::HBL_READ_ONLY);
        unsigned char* vertex = static_cast<unsigned char*>(vertexLock.pData);

        if (!extendOnly){
            // init values
            outRadius = 0;
            float* pFloat;
            elemPos->baseVertexPointerToElement(vertex, &pFloat);
            Vector3 basePos(pFloat[0], pFloat[1], pFloat[2]);
            outAABB.setExtents(basePos, basePos);
        }
        size_t vSize = vbuf->getVertexSize();
        unsigned char* vEnd = vertex + vertexData->vertexCount * vSize;
        Real radiusSqr = outRadius * outRadius;
        // Loop through all vertices.
        for (; vertex < vEnd; vertex += vSize) {
            float* pFloat;
            elemPos->baseVertexPointerToElement(vertex, &pFloat);
            Vector3 pos(pFloat[0], pFloat[1], pFloat[2]);
            outAABB.getMinimum().makeFloor(pos);
            outAABB.getMaximum().makeCeil(pos);
            radiusSqr = std::max<Real>(radiusSqr, pos.squaredLength());
        }
        outRadius = std::sqrt(radiusSqr);
    }
    //-----------------------------------------------------------------------
    void Mesh::setSkeletonName(const String& skelName)
    {
        if (skelName != getSkeletonName())
        {
            if (skelName.empty())
            {
                // No skeleton
                mSkeleton.reset();
            }
            else
            {
                // Load skeleton
                try {
                    mSkeleton = static_pointer_cast<Skeleton>(SkeletonManager::getSingleton().load(skelName, mGroup));
                }
                catch (...)
                {
                    mSkeleton.reset();
                    // Log this error
                    String msg = "Unable to load skeleton '";
                    msg += skelName + "' for Mesh '" + mName + "'. This Mesh will not be animated.";
                    LogManager::getSingleton().logError(msg);

                }


            }
            if (isLoaded())
                _dirtyState();
        }
    }
    //-----------------------------------------------------------------------
    void Mesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        mBoneAssignments.emplace(vertBoneAssign.vertexIndex, vertBoneAssign);
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void Mesh::clearBoneAssignments(void)
    {
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void Mesh::_initAnimationState(AnimationStateSet* animSet)
    {
        // Animation states for skeletal animation
        if (mSkeleton)
        {
            // Delegate to Skeleton
            mSkeleton->_initAnimationState(animSet);

            // Take the opportunity to update the compiled bone assignments
            _updateCompiledBoneAssignments();

        }

        // Animation states for vertex animation
        for (auto & i : mAnimationsList)
        {
            // Only create a new animation state if it doesn't exist
            // We can have the same named animation in both skeletal and vertex
            // with a shared animation state affecting both, for combined effects
            // The animations should be the same length if this feature is used!
            if (!animSet->hasAnimationState(i.second->getName()))
            {
                animSet->createAnimationState(i.second->getName(), 0.0,
                    i.second->getLength());
            }

        }

    }
    //---------------------------------------------------------------------
    void Mesh::_refreshAnimationState(AnimationStateSet* animSet)
    {
        if (mSkeleton)
        {
            mSkeleton->_refreshAnimationState(animSet);
        }

        // Merge in any new vertex animations
        for (auto& i : mAnimationsList)
        {
            // Create animation at time index 0, default params mean this has weight 1 and is disabled
            auto anim = i.second;
            const String& animName = anim->getName();
            if (!animSet->hasAnimationState(animName))
            {
                animSet->createAnimationState(animName, 0.0, anim->getLength());
            }
            else
            {
                // Update length incase changed
                AnimationState* animState = animSet->getAnimationState(animName);
                animState->setLength(anim->getLength());
                animState->setTimePosition(std::min(anim->getLength(), animState->getTimePosition()));
            }
        }
    }
    //-----------------------------------------------------------------------
    void Mesh::_updateCompiledBoneAssignments(void)
    {
        if (mBoneAssignmentsOutOfDate)
            _compileBoneAssignments();

        for (auto *m : mSubMeshList)
        {
            if (m->mBoneAssignmentsOutOfDate)
            {
                m->_compileBoneAssignments();
            }
        }
    }
    //-----------------------------------------------------------------------
    typedef std::multimap<Real, Mesh::VertexBoneAssignmentList::iterator> WeightIteratorMap;
    unsigned short Mesh::_rationaliseBoneAssignments(size_t vertexCount, Mesh::VertexBoneAssignmentList& assignments)
    {
        // Iterate through, finding the largest # bones per vertex
        unsigned short maxBones = 0;
        bool existsNonSkinnedVertices = false;
        VertexBoneAssignmentList::iterator i;

        for (size_t v = 0; v < vertexCount; ++v)
        {
            // Get number of entries for this vertex
            short currBones = static_cast<unsigned short>(assignments.count(v));
            if (currBones <= 0)
                existsNonSkinnedVertices = true;

            // Deal with max bones update
            // (note this will record maxBones even if they exceed limit)
            if (maxBones < currBones)
                maxBones = currBones;
            // does the number of bone assignments exceed limit?
            if (currBones > OGRE_MAX_BLEND_WEIGHTS)
            {
                // To many bone assignments on this vertex
                // Find the start & end (end is in iterator terms ie exclusive)
                std::pair<VertexBoneAssignmentList::iterator, VertexBoneAssignmentList::iterator> range;
                // map to sort by weight
                WeightIteratorMap weightToAssignmentMap;
                range = assignments.equal_range(v);
                // Add all the assignments to map
                for (i = range.first; i != range.second; ++i)
                {
                    // insert value weight->iterator
                    weightToAssignmentMap.emplace(i->second.weight, i);
                }
                // Reverse iterate over weight map, remove lowest n
                unsigned short numToRemove = currBones - OGRE_MAX_BLEND_WEIGHTS;
                WeightIteratorMap::iterator remIt = weightToAssignmentMap.begin();

                while (numToRemove--)
                {
                    // Erase this one
                    assignments.erase(remIt->second);
                    ++remIt;
                }
            } // if (currBones > OGRE_MAX_BLEND_WEIGHTS)

            // Make sure the weights are normalised
            // Do this irrespective of whether we had to remove assignments or not
            //   since it gives us a guarantee that weights are normalised
            //  We assume this, so it's a good idea since some modellers may not
            std::pair<VertexBoneAssignmentList::iterator, VertexBoneAssignmentList::iterator> normalise_range = assignments.equal_range(v);
            Real totalWeight = 0;
            // Find total first
            for (i = normalise_range.first; i != normalise_range.second; ++i)
            {
                totalWeight += i->second.weight;
            }
            // Now normalise if total weight is outside tolerance
            if (!Math::RealEqual(totalWeight, 1.0f))
            {
                for (i = normalise_range.first; i != normalise_range.second; ++i)
                {
                    i->second.weight = i->second.weight / totalWeight;
                }
            }

        }

        if (maxBones > OGRE_MAX_BLEND_WEIGHTS)
        {
            // Warn that we've reduced bone assignments
            LogManager::getSingleton().logWarning("the mesh '" + mName + "' "
                "includes vertices with more than " +
                StringConverter::toString(OGRE_MAX_BLEND_WEIGHTS) + " bone assignments. "
                "The lowest weighted assignments beyond this limit have been removed, so "
                "your animation may look slightly different. To eliminate this, reduce "
                "the number of bone assignments per vertex on your mesh to " +
                StringConverter::toString(OGRE_MAX_BLEND_WEIGHTS) + ".");
            // we've adjusted them down to the max
            maxBones = OGRE_MAX_BLEND_WEIGHTS;

        }

        if (existsNonSkinnedVertices)
        {
            // Warn that we've non-skinned vertices
            LogManager::getSingleton().logWarning("the mesh '" + mName + "' "
                "includes vertices without bone assignments. Those vertices will "
                "transform to wrong position when skeletal animation enabled. "
                "To eliminate this, assign at least one bone assignment per vertex "
                "on your mesh.");
        }

        return maxBones;
    }
    //-----------------------------------------------------------------------
    void  Mesh::_compileBoneAssignments(void)
    {
        if (sharedVertexData)
        {
            unsigned short maxBones = _rationaliseBoneAssignments(sharedVertexData->vertexCount, mBoneAssignments);

            if (maxBones != 0)
            {
                compileBoneAssignments(mBoneAssignments, maxBones, 
                    sharedBlendIndexToBoneIndexMap, sharedVertexData);
            }
        }
        mBoneAssignmentsOutOfDate = false;
    }
    //---------------------------------------------------------------------
    void Mesh::buildIndexMap(const VertexBoneAssignmentList& boneAssignments,
        IndexMap& boneIndexToBlendIndexMap, IndexMap& blendIndexToBoneIndexMap)
    {
        if (boneAssignments.empty())
        {
            // Just in case
            boneIndexToBlendIndexMap.clear();
            blendIndexToBoneIndexMap.clear();
            return;
        }

        typedef std::set<unsigned short> BoneIndexSet;
        BoneIndexSet usedBoneIndices;

        // Collect actually used bones
        for (auto& itVBA : boneAssignments)
        {
            usedBoneIndices.insert(itVBA.second.boneIndex);
        }

        // Allocate space for index map
        blendIndexToBoneIndexMap.resize(usedBoneIndices.size());
        boneIndexToBlendIndexMap.resize(*usedBoneIndices.rbegin() + 1);

        // Make index map between bone index and blend index
        unsigned short blendIndex = 0;
        for (auto& itBoneIndex : usedBoneIndices)
        {
            boneIndexToBlendIndexMap[itBoneIndex] = blendIndex;
            blendIndexToBoneIndexMap[blendIndex] = itBoneIndex;
            ++blendIndex;
        }
    }
    //---------------------------------------------------------------------
    void Mesh::compileBoneAssignments(
        const VertexBoneAssignmentList& boneAssignments,
        unsigned short numBlendWeightsPerVertex,
        IndexMap& blendIndexToBoneIndexMap,
        VertexData* targetVertexData)
    {
        // Create or reuse blend weight / indexes buffer
        // Indices are always a UBYTE4 no matter how many weights per vertex
        VertexDeclaration* decl = targetVertexData->vertexDeclaration;
        VertexBufferBinding* bind = targetVertexData->vertexBufferBinding;
        unsigned short bindIndex;

        // Build the index map brute-force. It's possible to store the index map
        // in .mesh, but maybe trivial.
        IndexMap boneIndexToBlendIndexMap;
        buildIndexMap(boneAssignments, boneIndexToBlendIndexMap, blendIndexToBoneIndexMap);

        const VertexElement* testElem =
            decl->findElementBySemantic(VES_BLEND_INDICES);
        if (testElem)
        {
            // Already have a buffer, unset it & delete elements
            bindIndex = testElem->getSource();
            // unset will cause deletion of buffer
            bind->unsetBinding(bindIndex);
            decl->removeElement(VES_BLEND_INDICES);
            decl->removeElement(VES_BLEND_WEIGHTS);
        }
        else
        {
            // Get new binding
            bindIndex = bind->getNextIndex();
        }
        // type of Weights is settable on the MeshManager.
        VertexElementType weightsBaseType = MeshManager::getSingleton().getBlendWeightsBaseElementType();
        VertexElementType weightsVertexElemType = VertexElement::multiplyTypeCount( weightsBaseType, numBlendWeightsPerVertex );
        HardwareVertexBufferSharedPtr vbuf = getHardwareBufferManager()->createVertexBuffer(
            sizeof( unsigned char ) * 4 + VertexElement::getTypeSize( weightsVertexElemType ),
                targetVertexData->vertexCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY,
                true // use shadow buffer
                );
        // bind new buffer
        bind->setBinding(bindIndex, vbuf);
        const VertexElement *pIdxElem, *pWeightElem;

        // add new vertex elements
        // Note, insert directly after all elements using the same source as
        // position to abide by pre-Dx9 format restrictions
        const VertexElement* firstElem = decl->getElement(0);
        if(firstElem->getSemantic() == VES_POSITION)
        {
            unsigned short insertPoint = 1;
            while (insertPoint < decl->getElementCount() &&
                decl->getElement(insertPoint)->getSource() == firstElem->getSource())
            {
                ++insertPoint;
            }
            const VertexElement& idxElem =
                decl->insertElement(insertPoint, bindIndex, 0, VET_UBYTE4, VES_BLEND_INDICES);
            const VertexElement& wtElem =
                decl->insertElement(insertPoint+1, bindIndex, sizeof(unsigned char)*4, weightsVertexElemType, VES_BLEND_WEIGHTS);
            pIdxElem = &idxElem;
            pWeightElem = &wtElem;
        }
        else
        {
            // Position is not the first semantic, therefore this declaration is
            // not pre-Dx9 compatible anyway, so just tack it on the end
            const VertexElement& idxElem =
                decl->addElement(bindIndex, 0, VET_UBYTE4, VES_BLEND_INDICES);
            const VertexElement& wtElem =
                decl->addElement(bindIndex, sizeof(unsigned char)*4, weightsVertexElemType, VES_BLEND_WEIGHTS );
            pIdxElem = &idxElem;
            pWeightElem = &wtElem;
        }

        unsigned int maxIntWt = 0;
        // keeping a switch out of the loop
        switch ( weightsBaseType )
        {
            default:
            	OgreAssert(false, "Invalid BlendWeightsBaseElementType");
            	break;
            case VET_FLOAT1:
                break;
            case VET_UBYTE4_NORM:
                maxIntWt = 0xff;
                break;
            case VET_USHORT2_NORM:
                maxIntWt = 0xffff;
                break;
            case VET_SHORT2_NORM:
                maxIntWt = 0x7fff;
                break;
        }
        // Assign data
        size_t v;
        VertexBoneAssignmentList::const_iterator i, iend;
        i = boneAssignments.begin();
        iend = boneAssignments.end();
        HardwareBufferLockGuard vertexLock(vbuf, HardwareBuffer::HBL_DISCARD);
        unsigned char *pBase = static_cast<unsigned char*>(vertexLock.pData);
        // Iterate by vertex
        for (v = 0; v < targetVertexData->vertexCount; ++v)
        {
            // collect the indices/weights in these arrays
            unsigned char indices[ 4 ] = { 0, 0, 0, 0 };
            float weights[ 4 ] = { 1.0f, 0.0f, 0.0f, 0.0f };
            for (unsigned short bone = 0; bone < numBlendWeightsPerVertex; ++bone)
            {
                // Do we still have data for this vertex?
                if (i != iend && i->second.vertexIndex == v)
                {
                    // If so, grab weight and index
                    weights[ bone ] = i->second.weight;
                    indices[ bone ] = static_cast<unsigned char>( boneIndexToBlendIndexMap[ i->second.boneIndex ] );
                    ++i;
                }
            }
            // if weights are integers,
            if ( weightsBaseType != VET_FLOAT1 )
            {
                // pack the float weights into shorts/bytes
                unsigned int intWeights[ 4 ];
                unsigned int sum = 0;
                const unsigned int wtScale = maxIntWt;  // this value corresponds to a weight of 1.0
                for ( int ii = 0; ii < 4; ++ii )
                {
                    unsigned int bw = static_cast<unsigned int>( weights[ ii ] * wtScale );
                    intWeights[ ii ] = bw;
                    sum += bw;
                }
                // if the sum doesn't add up due to roundoff error, we need to adjust the intWeights so that the sum is wtScale
                if ( sum != maxIntWt )
                {
                    // find the largest weight (it isn't necessarily the first one...)
                    int iMaxWeight = 0;
                    unsigned int maxWeight = 0;
                    for ( int ii = 0; ii < 4; ++ii )
                    {
                        unsigned int bw = intWeights[ ii ];
                        if ( bw > maxWeight )
                        {
                            iMaxWeight = ii;
                            maxWeight = bw;
                        }
                    }
                    // Adjust the largest weight to make sure the sum is correct.
                    // The idea is that changing the largest weight will have the smallest effect
                    // on the ratio of weights.  This works best when there is one dominant weight,
                    // and worst when 2 or more weights are similar in magnitude.
                    // A better method could be used to reduce the quantization error, but this is
                    // being done at run-time so it needs to be quick.
                    intWeights[ iMaxWeight ] += maxIntWt - sum;
                }

                // now write the weights
                if ( weightsBaseType == VET_UBYTE4_NORM )
                {
                    // write out the weights as bytes
                    unsigned char* pWeight;
                    pWeightElem->baseVertexPointerToElement( pBase, &pWeight );
                    // NOTE: always writes out 4 regardless of numBlendWeightsPerVertex
                    for (unsigned int intWeight : intWeights)
                    {
                        *pWeight++ = static_cast<unsigned char>( intWeight );
                    }
                }
                else
                {
                    // write out the weights as shorts
                    unsigned short* pWeight;
                    pWeightElem->baseVertexPointerToElement( pBase, &pWeight );
                    for ( int ii = 0; ii < numBlendWeightsPerVertex; ++ii )
                    {
                        *pWeight++ = static_cast<unsigned short>( intWeights[ ii ] );
                    }
                }
            }
            else
            {
                // write out the weights as floats
                float* pWeight;
                pWeightElem->baseVertexPointerToElement( pBase, &pWeight );
                for ( int ii = 0; ii < numBlendWeightsPerVertex; ++ii )
                {
                    *pWeight++ = weights[ ii ];
                }
            }
            unsigned char* pIndex;
            pIdxElem->baseVertexPointerToElement( pBase, &pIndex );
            for (unsigned char indice : indices)
            {
                *pIndex++ = indice;
            }
            pBase += vbuf->getVertexSize();
        }
    }
    //---------------------------------------------------------------------
    static Real distLineSegToPoint( const Vector3& line0, const Vector3& line1, const Vector3& pt )
    {
        Vector3 v01 = line1 - line0;
        Real tt = v01.dotProduct( pt - line0 ) / std::max( v01.dotProduct(v01), std::numeric_limits<Real>::epsilon() );
        tt = Math::Clamp( tt, Real(0.0f), Real(1.0f) );
        Vector3 onLine = line0 + tt * v01;
        return pt.distance( onLine );
    }
    //---------------------------------------------------------------------
    static Real _computeBoneBoundingRadiusHelper( VertexData* vertexData,
        const Mesh::VertexBoneAssignmentList& boneAssignments,
        const std::vector<Vector3>& bonePositions,
        const std::vector< std::vector<ushort> >& boneChildren
        )
    {
        std::vector<Vector3> vertexPositions;
        {
            // extract vertex positions
            const VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
            HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
            // if usage is write only,
            if ( !vbuf->hasShadowBuffer() && (vbuf->getUsage() & HBU_DETAIL_WRITE_ONLY) )
            {
                // can't do it
                return Real(0.0f);
            }
            vertexPositions.resize( vertexData->vertexCount );
            HardwareBufferLockGuard vertexLock(vbuf, HardwareBuffer::HBL_READ_ONLY);
            unsigned char* vertex = static_cast<unsigned char*>(vertexLock.pData);
            float* pFloat;

            for(size_t i = 0; i < vertexData->vertexCount; ++i)
            {
                posElem->baseVertexPointerToElement(vertex, &pFloat);
                vertexPositions[ i ] = Vector3( pFloat[0], pFloat[1], pFloat[2] );
                vertex += vbuf->getVertexSize();
            }
        }
        Real maxRadius = Real(0);
        Real minWeight = Real(0.01);
        // for each vertex-bone assignment,
        for (const auto & boneAssignment : boneAssignments)
        {
            // if weight is close to zero, ignore
            if (boneAssignment.second.weight > minWeight)
            {
                // if we have a bounding box around all bone origins, we consider how far outside this box the
                // current vertex could ever get (assuming it is only attached to the given bone, and the bones all have unity scale)
                size_t iBone = boneAssignment.second.boneIndex;
                const Vector3& v = vertexPositions[ boneAssignment.second.vertexIndex ];
                Vector3 diff = v - bonePositions[ iBone ];
                Real dist = diff.length();  // max distance of vertex v outside of bounding box
                // if this bone has children, we can reduce the dist under the assumption that the children may rotate wrt their parent, but don't translate
                for (size_t iChild = 0; iChild < boneChildren[iBone].size(); ++iChild)
                {
                    // given this assumption, we know that the bounding box will enclose both the bone origin as well as the origin of the child bone,
                    // and therefore everything on a line segment between the bone origin and the child bone will be inside the bounding box as well
                    size_t iChildBone = boneChildren[ iBone ][ iChild ];
                    // compute distance from vertex to line segment between bones
                    Real distChild = distLineSegToPoint( bonePositions[ iBone ], bonePositions[ iChildBone ], v );
                    dist = std::min( dist, distChild );
                }
                // scale the distance by the weight, this prevents the radius from being over-inflated because of a vertex that is lightly influenced by a faraway bone
                dist *= boneAssignment.second.weight;
                maxRadius = std::max( maxRadius, dist );
            }
        }
        return maxRadius;
    }
    //---------------------------------------------------------------------
    void Mesh::_computeBoneBoundingRadius()
    {
        if (mBoneBoundingRadius == Real(0) && mSkeleton)
        {
            Real radius = Real(0);
            std::vector<Vector3> bonePositions;
            std::vector< std::vector<ushort> > boneChildren;  // for each bone, a list of children
            {
                // extract binding pose bone positions, and also indices for child bones
                uint16 numBones = mSkeleton->getNumBones();
                mSkeleton->setBindingPose();
                mSkeleton->_updateTransforms();
                bonePositions.resize( numBones );
                boneChildren.resize( numBones );
                // for each bone,
                for (uint16 iBone = 0; iBone < numBones; ++iBone)
                {
                    Bone* bone = mSkeleton->getBone( iBone );
                    bonePositions[ iBone ] = bone->_getDerivedPosition();
                    boneChildren[ iBone ].reserve( bone->numChildren() );
                    for (uint16 iChild = 0; iChild < bone->numChildren(); ++iChild)
                    {
                        Bone* child = static_cast<Bone*>( bone->getChild( iChild ) );
                        boneChildren[ iBone ].push_back( child->getHandle() );
                    }
                }
            }
            if (sharedVertexData)
            {
                // check shared vertices
                radius = _computeBoneBoundingRadiusHelper(sharedVertexData, mBoneAssignments, bonePositions, boneChildren);
            }

            // check submesh vertices
            for(auto *submesh : mSubMeshList)
            {
                if (!submesh->useSharedVertices && submesh->vertexData)
                {
                    Real r = _computeBoneBoundingRadiusHelper(submesh->vertexData, submesh->mBoneAssignments, bonePositions, boneChildren);
                    radius = std::max( radius, r );
                }
            }
            if (radius > Real(0))
            {
                mBoneBoundingRadius = radius;
            }
            else
            {
                // fallback if we failed to find the vertices
                mBoneBoundingRadius = mBoundRadius;
            }
        }
    }
    //---------------------------------------------------------------------
    void Mesh::_notifySkeleton(const SkeletonPtr& pSkel)
    {
        mSkeleton = pSkel;
    }
    //---------------------------------------------------------------------
    Mesh::BoneAssignmentIterator Mesh::getBoneAssignmentIterator(void)
    {
        return BoneAssignmentIterator(mBoneAssignments.begin(),
            mBoneAssignments.end());
    }
    //---------------------------------------------------------------------
    const String& Mesh::getSkeletonName(void) const
    {
        return mSkeleton ? mSkeleton->getName() : BLANKSTRING;
    }
    //---------------------------------------------------------------------
    const MeshLodUsage& Mesh::getLodLevel(ushort index) const
    {
#if !OGRE_NO_MESHLOD
        index = std::min(index, (ushort)(mMeshLodUsageList.size() - 1));
        if (this->_isManualLodLevel(index) && index > 0 && !mMeshLodUsageList[index].manualMesh)
        {
            // Load the mesh now
            try {
                mMeshLodUsageList[index].manualMesh =
                    MeshManager::getSingleton().load(
                        mMeshLodUsageList[index].manualName,
                        getGroup());
                // get the edge data, if required
                if (!mMeshLodUsageList[index].edgeData)
                {
                    mMeshLodUsageList[index].edgeData =
                        mMeshLodUsageList[index].manualMesh->getEdgeList(0);
                }
            }
            catch (Exception& )
            {
                LogManager::getSingleton().stream()
                    << "Error while loading manual LOD level "
                    << mMeshLodUsageList[index].manualName
                    << " - this LOD level will not be rendered. You can "
                    << "ignore this error in offline mesh tools.";
            }

        }
        return mMeshLodUsageList[index];
#else
        return mMeshLodUsageList[0];
#endif
    }
    //---------------------------------------------------------------------
    ushort Mesh::getLodIndex(Real value) const
    {
#if !OGRE_NO_MESHLOD
        // Get index from strategy
        return mLodStrategy->getIndex(value, mMeshLodUsageList);
#else
        return 0;
#endif
    }
    //---------------------------------------------------------------------
#if !OGRE_NO_MESHLOD
    void Mesh::updateManualLodLevel(ushort index, const String& meshName)
    {

        // Basic prerequisites
        assert(index != 0 && "Can't modify first LOD level (full detail)");
        assert(index < mMeshLodUsageList.size() && "Idndex out of bounds");
        // get lod
        MeshLodUsage* lod = &(mMeshLodUsageList[index]);

        lod->manualName = meshName;
        lod->manualMesh.reset();
        OGRE_DELETE lod->edgeData;
        lod->edgeData = 0;
    }
    //---------------------------------------------------------------------
    void Mesh::_setLodInfo(unsigned short numLevels)
    {
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

        // Basic prerequisites
        assert(numLevels > 0 && "Must be at least one level (full detail level must exist)");

        mNumLods = numLevels;
        mMeshLodUsageList.resize(numLevels);
        // Resize submesh face data lists too
        for (auto & i : mSubMeshList)
        {
            i->mLodFaceList.resize(numLevels - 1);
        }
    }
    //---------------------------------------------------------------------
    void Mesh::_setLodUsage(unsigned short level, const MeshLodUsage& usage)
    {
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

        // Basic prerequisites
        assert(level != 0 && "Can't modify first LOD level (full detail)");
        assert(level < mMeshLodUsageList.size() && "Index out of bounds");

        mMeshLodUsageList[level] = usage;

        if(!mMeshLodUsageList[level].manualName.empty()){
            mHasManualLodLevel = true;
        }
    }
    //---------------------------------------------------------------------
    void Mesh::_setSubMeshLodFaceList(unsigned short subIdx, unsigned short level,
        IndexData* facedata)
    {
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

        // Basic prerequisites
        assert(mMeshLodUsageList[level].manualName.empty() && "Not using generated LODs!");
        assert(subIdx < mSubMeshList.size() && "Index out of bounds");
        assert(level != 0 && "Can't modify first LOD level (full detail)");
        assert(level-1 < (unsigned short)mSubMeshList[subIdx]->mLodFaceList.size() && "Index out of bounds");

        SubMesh* sm = mSubMeshList[subIdx];
        sm->mLodFaceList[level - 1] = facedata;
    }
#endif
    //---------------------------------------------------------------------
    bool Mesh::_isManualLodLevel( unsigned short level ) const
    {
#if !OGRE_NO_MESHLOD
        return !mMeshLodUsageList[level].manualName.empty();
#else
        return false;
#endif
    }
    //---------------------------------------------------------------------
    ushort Mesh::_getSubMeshIndex(const String& name) const
    {
        SubMeshNameMap::const_iterator i = mSubMeshNameMap.find(name) ;
        if (i == mSubMeshNameMap.end())
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No SubMesh named " + name + " found.",
                "Mesh::_getSubMeshIndex");

        return i->second;
    }
    //--------------------------------------------------------------------
    void Mesh::removeLodLevels(void)
    {
#if !OGRE_NO_MESHLOD
        // Remove data from SubMeshes
        for (auto *isub : mSubMeshList)
        {
            isub->removeLodLevels();
        }

        bool edgeListWasBuilt = isEdgeListBuilt();
        freeEdgeList();

        // Reinitialise
        mNumLods = 1;
        mMeshLodUsageList.resize(1);
        mMeshLodUsageList[0].edgeData = NULL;

        if(edgeListWasBuilt)
            buildEdgeList();
#endif
    }

    //---------------------------------------------------------------------
    Real Mesh::getBoundingSphereRadius(void) const
    {
        return mBoundRadius;
    }
    //---------------------------------------------------------------------
    Real Mesh::getBoneBoundingRadius(void) const
    {
        return mBoneBoundingRadius;
    }
    //---------------------------------------------------------------------
    void Mesh::setVertexBufferPolicy(HardwareBuffer::Usage vbUsage, bool shadowBuffer)
    {
        mVertexBufferUsage = vbUsage;
        mVertexBufferShadowBuffer = shadowBuffer;
    }
    //---------------------------------------------------------------------
    void Mesh::setIndexBufferPolicy(HardwareBuffer::Usage vbUsage, bool shadowBuffer)
    {
        mIndexBufferUsage = vbUsage;
        mIndexBufferShadowBuffer = shadowBuffer;
    }
    //---------------------------------------------------------------------
    void Mesh::mergeAdjacentTexcoords( unsigned short finalTexCoordSet,
                                        unsigned short texCoordSetToDestroy )
    {
        if( sharedVertexData )
            mergeAdjacentTexcoords( finalTexCoordSet, texCoordSetToDestroy, sharedVertexData );

        for (auto *s : mSubMeshList)
        {
            if(!s->useSharedVertices)
                mergeAdjacentTexcoords (finalTexCoordSet, texCoordSetToDestroy, s->vertexData );
        }
    }
    //---------------------------------------------------------------------
    void Mesh::mergeAdjacentTexcoords( unsigned short finalTexCoordSet,
                                        unsigned short texCoordSetToDestroy,
                                        VertexData *vertexData )
    {
        VertexDeclaration *vDecl    = vertexData->vertexDeclaration;

        const VertexElement *uv0 = vDecl->findElementBySemantic( VES_TEXTURE_COORDINATES,
                                                                    finalTexCoordSet );
        const VertexElement *uv1 = vDecl->findElementBySemantic( VES_TEXTURE_COORDINATES,
                                                                    texCoordSetToDestroy );

        if( uv0 && uv1 )
        {
            //Check that both base types are compatible (mix floats w/ shorts) and there's enough space
            VertexElementType baseType0 = VertexElement::getBaseType( uv0->getType() );
            VertexElementType baseType1 = VertexElement::getBaseType( uv1->getType() );

            unsigned short totalTypeCount = VertexElement::getTypeCount( uv0->getType() ) +
                                            VertexElement::getTypeCount( uv1->getType() );
            if( baseType0 == baseType1 && totalTypeCount <= 4 )
            {
                const VertexDeclaration::VertexElementList &veList = vDecl->getElements();
                VertexDeclaration::VertexElementList::const_iterator uv0Itor = std::find( veList.begin(),
                                                                                    veList.end(), *uv0 );
                unsigned short elem_idx     = std::distance( veList.begin(), uv0Itor );
                VertexElementType newType   = VertexElement::multiplyTypeCount( baseType0,
                                                                                totalTypeCount );

                if( ( uv0->getOffset() + uv0->getSize() == uv1->getOffset() ||
                      uv1->getOffset() + uv1->getSize() == uv0->getOffset() ) &&
                    uv0->getSource() == uv1->getSource() )
                {
                    //Special case where they adjacent, just change the declaration & we're done.
                    size_t newOffset        = std::min( uv0->getOffset(), uv1->getOffset() );
                    unsigned short newIdx   = std::min( uv0->getIndex(), uv1->getIndex() );

                    vDecl->modifyElement( elem_idx, uv0->getSource(), newOffset, newType,
                                            VES_TEXTURE_COORDINATES, newIdx );
                    vDecl->removeElement( VES_TEXTURE_COORDINATES, texCoordSetToDestroy );
                    uv1 = 0;
                }

                vDecl->closeGapsInSource();
            }
        }
    }
    //---------------------------------------------------------------------
    void Mesh::organiseTangentsBuffer(VertexData *vertexData,
        VertexElementSemantic targetSemantic, unsigned short index, 
        unsigned short sourceTexCoordSet)
    {
        VertexDeclaration *vDecl = vertexData->vertexDeclaration ;
        VertexBufferBinding *vBind = vertexData->vertexBufferBinding ;

        const VertexElement *tangentsElem = vDecl->findElementBySemantic(targetSemantic, index);
        bool needsToBeCreated = false;

        if (!tangentsElem)
        { // no tex coords with index 1
                needsToBeCreated = true ;
        }
        else if (tangentsElem->getType() != VET_FLOAT3)
        {
            //  buffer exists, but not 3D
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Target semantic set already exists but is not 3D, therefore "
                "cannot contain tangents. Pick an alternative destination semantic. ",
                "Mesh::organiseTangentsBuffer");
        }

        HardwareVertexBufferSharedPtr newBuffer;
        if (needsToBeCreated)
        {
            // To be most efficient with our vertex streams,
            // tack the new tangents onto the same buffer as the
            // source texture coord set
            const VertexElement* prevTexCoordElem =
                vertexData->vertexDeclaration->findElementBySemantic(
                    VES_TEXTURE_COORDINATES, sourceTexCoordSet);
            if (!prevTexCoordElem)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                    "Cannot locate the first texture coordinate element to "
                    "which to append the new tangents.", 
                    "Mesh::orgagniseTangentsBuffer");
            }
            // Find the buffer associated with  this element
            HardwareVertexBufferSharedPtr origBuffer =
                vertexData->vertexBufferBinding->getBuffer(
                    prevTexCoordElem->getSource());
            // Now create a new buffer, which includes the previous contents
            // plus extra space for the 3D coords
            newBuffer = getHardwareBufferManager()->createVertexBuffer(
                origBuffer->getVertexSize() + 3*sizeof(float),
                vertexData->vertexCount,
                origBuffer->getUsage(),
                origBuffer->hasShadowBuffer() );
            // Add the new element
            vDecl->addElement(
                prevTexCoordElem->getSource(),
                origBuffer->getVertexSize(),
                VET_FLOAT3,
                targetSemantic,
                index);
            // Now copy the original data across
            HardwareBufferLockGuard srcLock(origBuffer, HardwareBuffer::HBL_READ_ONLY);
            HardwareBufferLockGuard dstLock(newBuffer, HardwareBuffer::HBL_DISCARD);
            unsigned char* pSrc = static_cast<unsigned char*>(srcLock.pData);
            unsigned char* pDest = static_cast<unsigned char*>(dstLock.pData);
            size_t vertSize = origBuffer->getVertexSize();
            for (size_t v = 0; v < vertexData->vertexCount; ++v)
            {
                // Copy original vertex data
                memcpy(pDest, pSrc, vertSize);
                pSrc += vertSize;
                pDest += vertSize;
                // Set the new part to 0 since we'll accumulate in this
                memset(pDest, 0, sizeof(float)*3);
                pDest += sizeof(float)*3;
            }

            // Rebind the new buffer
            vBind->setBinding(prevTexCoordElem->getSource(), newBuffer);
        }
    }
    //---------------------------------------------------------------------
    void Mesh::buildTangentVectors(unsigned short sourceTexCoordSet, bool splitMirrored, bool splitRotated,
                                   bool storeParityInW)
    {

        TangentSpaceCalc tangentsCalc;
        tangentsCalc.setSplitMirrored(splitMirrored);
        tangentsCalc.setSplitRotated(splitRotated);
        tangentsCalc.setStoreParityInW(storeParityInW);

        // shared geometry first
        if (sharedVertexData)
        {
            tangentsCalc.setVertexData(sharedVertexData);
            bool found = false;
            for (auto sm : mSubMeshList)
            {
                if (sm->useSharedVertices)
                {
                    tangentsCalc.addIndexData(sm->indexData);
                    found = true;
                }
            }
            if (found)
            {
                TangentSpaceCalc::Result res = tangentsCalc.build(sourceTexCoordSet);

                // If any vertex splitting happened, we have to give them bone assignments
                if (mSkeleton)
                {
                    for (auto & remap : res.indexesRemapped)
                    {
                        // Copy all bone assignments from the split vertex
                        VertexBoneAssignmentList::iterator vbstart = mBoneAssignments.lower_bound(remap.splitVertex.first);
                        VertexBoneAssignmentList::iterator vbend = mBoneAssignments.upper_bound(remap.splitVertex.first);
                        for (VertexBoneAssignmentList::iterator vba = vbstart; vba != vbend; ++vba)
                        {
                            VertexBoneAssignment newAsgn = vba->second;
                            newAsgn.vertexIndex = static_cast<unsigned int>(remap.splitVertex.second);
                            // multimap insert doesn't invalidate iterators
                            addBoneAssignment(newAsgn);
                        }
                        
                    }
                }

                // Update poses (some vertices might have been duplicated)
                // we will just check which vertices have been split and copy
                // the offset for the original vertex to the corresponding new vertex
                for (auto *current_pose : mPoseList)
                {
                    const Pose::VertexOffsetMap& offset_map = current_pose->getVertexOffsets();
                    for(auto& split : res.vertexSplits)
                    {
                        Pose::VertexOffsetMap::const_iterator found_offset = offset_map.find(split.first);

                        // copy the offset
                        if( found_offset != offset_map.end() )
                        {
                            current_pose->addVertex (split.second, found_offset->second );
                        }
                    }
                }
            }
        }

        // Dedicated geometry
        for (auto sm : mSubMeshList)
        {
            if (!sm->useSharedVertices)
            {
                tangentsCalc.clear();
                tangentsCalc.setVertexData(sm->vertexData);
                tangentsCalc.addIndexData(sm->indexData, sm->operationType);
                TangentSpaceCalc::Result res = tangentsCalc.build(sourceTexCoordSet);

                // If any vertex splitting happened, we have to give them bone assignments
                if (mSkeleton)
                {
                    for (auto & remap : res.indexesRemapped)
                    {
                        // Copy all bone assignments from the split vertex
                        VertexBoneAssignmentList::const_iterator vbstart = 
                            sm->getBoneAssignments().lower_bound(remap.splitVertex.first);
                        VertexBoneAssignmentList::const_iterator vbend = 
                            sm->getBoneAssignments().upper_bound(remap.splitVertex.first);
                        for (VertexBoneAssignmentList::const_iterator vba = vbstart; vba != vbend; ++vba)
                        {
                            VertexBoneAssignment newAsgn = vba->second;
                            newAsgn.vertexIndex = static_cast<unsigned int>(remap.splitVertex.second);
                            // multimap insert doesn't invalidate iterators
                            sm->addBoneAssignment(newAsgn);
                        }

                    }

                }
            }
        }

    }
    //---------------------------------------------------------------------
    bool Mesh::suggestTangentVectorBuildParams(unsigned short& outSourceCoordSet)
    {
        // Go through all the vertex data and locate source and dest (must agree)
        bool sharedGeometryDone = false;
        bool foundExisting = false;
        bool firstOne = true;
        for (auto *sm : mSubMeshList)
        {
            VertexData* vertexData;
            if (sm->useSharedVertices)
            {
                if (sharedGeometryDone)
                    continue;
                vertexData = sharedVertexData;
                sharedGeometryDone = true;
            }
            else
            {
                vertexData = sm->vertexData;
            }

            const VertexElement *sourceElem = 0;
            unsigned short targetIndex = 0;
            for (targetIndex = 0; targetIndex < OGRE_MAX_TEXTURE_COORD_SETS; ++targetIndex)
            {
                auto testElem =
                    vertexData->vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, targetIndex);
                if (!testElem)
                    break; // finish if we've run out, t will be the target

                // We're still looking for the source texture coords
                if (testElem->getType() == VET_FLOAT2)
                {
                    // Ok, we found it
                    sourceElem = testElem;
                    break;
                }
            }

            // After iterating, we should have a source and a possible destination (t)
            if (!sourceElem)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                            "Cannot locate an appropriate 2D texture coordinate set for "
                            "all the vertex data in this mesh to create tangents from. ");
            }

            // Look for existing semantic
            foundExisting = vertexData->vertexDeclaration->findElementBySemantic(VES_TANGENT);

            // Check that we agree with previous decisions, if this is not the
            // first one, and if we're not just using the existing one
            if (!firstOne && !foundExisting)
            {
                if (sourceElem->getIndex() != outSourceCoordSet)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                                "Multiple sets of vertex data in this mesh disagree on "
                                "the appropriate index to use for the texture coordinates. "
                                "This ambiguity must be rectified before tangents can be generated.");
                }
            }

            // Otherwise, save this result
            outSourceCoordSet = sourceElem->getIndex();

            firstOne = false;

       }

        return foundExisting;

    }
    //---------------------------------------------------------------------
    void Mesh::buildEdgeList(void)
    {
        if (mEdgeListsBuilt)
            return;
#if !OGRE_NO_MESHLOD
        // Loop over LODs
        for (unsigned short lodIndex = 0; lodIndex < (unsigned short)mMeshLodUsageList.size(); ++lodIndex)
        {
            // use getLodLevel to enforce loading of manual mesh lods
            const MeshLodUsage& usage = getLodLevel(lodIndex);

            if (!usage.manualName.empty() && lodIndex != 0)
            {
                // Delegate edge building to manual mesh
                // It should have already built it's own edge list while loading
                if (usage.manualMesh)
                {
                    usage.edgeData = usage.manualMesh->getEdgeList(0);
                }
            }
            else
            {
                // Build
                EdgeListBuilder eb;
                size_t vertexSetCount = 0;
                bool atLeastOneIndexSet = false;

                if (sharedVertexData)
                {
                    eb.addVertexData(sharedVertexData);
                    vertexSetCount++;
                }

                // Prepare the builder using the submesh information
                for (auto *s : mSubMeshList)
                {
                    if (s->operationType != RenderOperation::OT_TRIANGLE_FAN && 
                        s->operationType != RenderOperation::OT_TRIANGLE_LIST && 
                        s->operationType != RenderOperation::OT_TRIANGLE_STRIP)
                    {
                        continue;
                    }
                    if (s->useSharedVertices)
                    {
                        // Use shared vertex data, index as set 0
                        if (lodIndex == 0)
                        {
                            eb.addIndexData(s->indexData, 0, s->operationType);
                        }
                        else
                        {
                            eb.addIndexData(s->mLodFaceList[lodIndex-1], 0,
                                s->operationType);
                        }
                    }
                    else if(s->isBuildEdgesEnabled())
                    {
                        // own vertex data, add it and reference it directly
                        eb.addVertexData(s->vertexData);
                        if (lodIndex == 0)
                        {
                            // Base index data
                            eb.addIndexData(s->indexData, vertexSetCount++,
                                s->operationType);
                        }
                        else
                        {
                            // LOD index data
                            eb.addIndexData(s->mLodFaceList[lodIndex-1],
                                vertexSetCount++, s->operationType);
                        }

                    }
                    atLeastOneIndexSet = true;
                }

                if (atLeastOneIndexSet)
                {
                    usage.edgeData = eb.build();

                #if OGRE_DEBUG_MODE
                    // Override default log
                    Log* log = LogManager::getSingleton().createLog(
                        mName + "_lod" + StringConverter::toString(lodIndex) +
                        "_prepshadow.log", false, false);
                    usage.edgeData->log(log);
                    // clean up log & close file handle
                    LogManager::getSingleton().destroyLog(log);
                #endif
                }
                else
                {
                    // create empty edge data
                    usage.edgeData = OGRE_NEW EdgeData();
                }
            }
        }
#else
        // Build
        EdgeListBuilder eb;
        size_t vertexSetCount = 0;
        if (sharedVertexData)
        {
            eb.addVertexData(sharedVertexData);
            vertexSetCount++;
        }

        // Prepare the builder using the submesh information
        for (auto *s : mSubMeshList)
        {
            if (s->operationType != RenderOperation::OT_TRIANGLE_FAN && 
                s->operationType != RenderOperation::OT_TRIANGLE_LIST && 
                s->operationType != RenderOperation::OT_TRIANGLE_STRIP)
            {
                continue;
            }
            if (s->useSharedVertices)
            {
                eb.addIndexData(s->indexData, 0, s->operationType);
            }
            else if(s->isBuildEdgesEnabled())
            {
                // own vertex data, add it and reference it directly
                eb.addVertexData(s->vertexData);
                // Base index data
                eb.addIndexData(s->indexData, vertexSetCount++,
                    s->operationType);
            }
        }

        mMeshLodUsageList[0].edgeData = eb.build();

#if OGREUG_MODE
        // Override default log
        Log* log = LogManager::getSingleton().createLog(
            mName + "_lod0"+
            "_prepshadow.log", false, false);
        mMeshLodUsageList[0].edgeData->log(log);
        // clean up log & close file handle
        LogManager::getSingleton().destroyLog(log);
#endif
#endif
        mEdgeListsBuilt = true;
    }
    //---------------------------------------------------------------------
    void Mesh::freeEdgeList(void)
    {
        if (!mEdgeListsBuilt)
            return;
#if !OGRE_NO_MESHLOD
        // Loop over LODs
        unsigned short index = 0;
        for (auto& usage : mMeshLodUsageList)
        {
            if (usage.manualName.empty() || index == 0)
            {
                // Only delete if we own this data
                // Manual LODs > 0 own their own
                OGRE_DELETE usage.edgeData;
            }
            usage.edgeData = NULL;
            ++index;
        }
#else
        OGRE_DELETE mMeshLodUsageList[0].edgeData;
        mMeshLodUsageList[0].edgeData = NULL;
#endif
        mEdgeListsBuilt = false;
    }
    //---------------------------------------------------------------------
    void Mesh::prepareForShadowVolume(void)
    {
        if (mPreparedForShadowVolumes)
            return;

        if (sharedVertexData)
        {
            sharedVertexData->prepareForShadowVolume();
        }
        for (auto *s : mSubMeshList)
        {
            if (!s->useSharedVertices && 
                (s->operationType == RenderOperation::OT_TRIANGLE_FAN || 
                s->operationType == RenderOperation::OT_TRIANGLE_LIST ||
                s->operationType == RenderOperation::OT_TRIANGLE_STRIP))
            {
                s->vertexData->prepareForShadowVolume();
            }
        }
        mPreparedForShadowVolumes = true;
    }
    //---------------------------------------------------------------------
    EdgeData* Mesh::getEdgeList(unsigned short lodIndex)
    {
        // Build edge list on demand
        if (!mEdgeListsBuilt && mAutoBuildEdgeLists)
        {
            buildEdgeList();
        }
#if !OGRE_NO_MESHLOD
        return getLodLevel(lodIndex).edgeData;
#else
        assert(lodIndex == 0);
        return mMeshLodUsageList[0].edgeData;
#endif
    }
    //---------------------------------------------------------------------
    const EdgeData* Mesh::getEdgeList(unsigned short lodIndex) const
    {
#if !OGRE_NO_MESHLOD
        return getLodLevel(lodIndex).edgeData;
#else
        assert(lodIndex == 0);
        return mMeshLodUsageList[0].edgeData;
#endif
    }
    //---------------------------------------------------------------------
    void Mesh::prepareMatricesForVertexBlend(const Affine3** blendMatrices,
        const Affine3* boneMatrices, const IndexMap& indexMap)
    {
        assert(indexMap.size() <= OGRE_MAX_NUM_BONES);
        for (auto& i : indexMap)
        {
            *blendMatrices++ = boneMatrices + i;
        }
    }
    //---------------------------------------------------------------------
    void Mesh::softwareVertexBlend(const VertexData* sourceVertexData,
        const VertexData* targetVertexData,
        const Affine3* const* blendMatrices, size_t numMatrices,
        bool blendNormals)
    {
        float *pSrcPos = 0;
        float *pSrcNorm = 0;
        float *pDestPos = 0;
        float *pDestNorm = 0;
        float *pBlendWeight = 0;
        unsigned char* pBlendIdx = 0;
        size_t srcNormStride = 0;
        size_t destNormStride = 0;

        // Get elements for source
        auto srcElemPos = sourceVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        auto srcElemNorm = sourceVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        auto srcElemBlendIndices = sourceVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        auto srcElemBlendWeights = sourceVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
        OgreAssert(srcElemPos && srcElemBlendIndices && srcElemBlendWeights,
                   "You must supply at least positions, blend indices and blend weights");
        // Get elements for target
        auto destElemPos = targetVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        auto destElemNorm = targetVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);

        // Get buffers for source
        HardwareVertexBufferSharedPtr srcPosBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemPos->getSource());
        HardwareVertexBufferSharedPtr srcIdxBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemBlendIndices->getSource());
        HardwareVertexBufferSharedPtr srcWeightBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemBlendWeights->getSource());
        HardwareVertexBufferSharedPtr srcNormBuf;

        // Get buffers for target
        HardwareVertexBufferSharedPtr destPosBuf = targetVertexData->vertexBufferBinding->getBuffer(destElemPos->getSource());

        // Lock source buffers for reading
        HardwareBufferLockGuard srcPosLock(srcPosBuf, HardwareBuffer::HBL_READ_ONLY);
        srcElemPos->baseVertexPointerToElement(srcPosLock.pData, &pSrcPos);

        // Do we have normals and want to blend them?
        bool includeNormals = blendNormals && srcElemNorm && destElemNorm;
        HardwareVertexBufferSharedPtr destNormBuf;
        HardwareBufferLockGuard srcNormLock;
        if (includeNormals)
        {
            // Get buffers for source
            srcNormBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemNorm->getSource());
            srcNormStride = srcNormBuf->getVertexSize();
            // Get buffers for target
            destNormBuf = targetVertexData->vertexBufferBinding->getBuffer(destElemNorm->getSource());
            destNormStride = destNormBuf->getVertexSize();

            if (srcNormBuf != srcPosBuf)
            {
                // Different buffer
                srcNormLock.lock(srcNormBuf, HardwareBuffer::HBL_READ_ONLY);
            }
            srcElemNorm->baseVertexPointerToElement(srcNormLock.pData ? srcNormLock.pData : srcPosLock.pData, &pSrcNorm);
        }

        // Indices must be 4 bytes
        assert(srcElemBlendIndices->getType() == VET_UBYTE4 && "Blend indices must be VET_UBYTE4");
        HardwareBufferLockGuard srcIdxLock(srcIdxBuf, HardwareBuffer::HBL_READ_ONLY);
        srcElemBlendIndices->baseVertexPointerToElement(srcIdxLock.pData, &pBlendIdx);
        HardwareBufferLockGuard srcWeightLock;
        if (srcWeightBuf != srcIdxBuf)
        {
            // Lock buffer
            srcWeightLock.lock(srcWeightBuf, HardwareBuffer::HBL_READ_ONLY);
        }
        srcElemBlendWeights->baseVertexPointerToElement(srcWeightLock.pData ? srcWeightLock.pData : srcIdxLock.pData, &pBlendWeight);
        unsigned short numWeightsPerVertex = VertexElement::getTypeCount(srcElemBlendWeights->getType());

        // Lock destination buffers for writing
        HardwareBufferLockGuard destPosLock(destPosBuf,
            (destNormBuf != destPosBuf && destPosBuf->getVertexSize() == destElemPos->getSize()) ||
            (destNormBuf == destPosBuf && destPosBuf->getVertexSize() == destElemPos->getSize() + destElemNorm->getSize()) ?
            HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        destElemPos->baseVertexPointerToElement(destPosLock.pData, &pDestPos);
        HardwareBufferLockGuard destNormLock;
        if (includeNormals)
        {
            if (destNormBuf != destPosBuf)
            {
                destNormLock.lock(destNormBuf, destNormBuf->getVertexSize() == destElemNorm->getSize()
                                                   ? HardwareBuffer::HBL_DISCARD
                                                   : HardwareBuffer::HBL_NORMAL);
            }
            destElemNorm->baseVertexPointerToElement(destNormLock.pData ? destNormLock.pData : destPosLock.pData, &pDestNorm);
        }

        auto srcPosStride = srcPosBuf->getVertexSize();
        auto destPosStride = destPosBuf->getVertexSize();
        auto blendIdxStride = srcIdxBuf->getVertexSize();
        auto blendWeightStride = srcWeightBuf->getVertexSize();

        OptimisedUtil::getImplementation()->softwareVertexSkinning(
            pSrcPos, pDestPos,
            pSrcNorm, pDestNorm,
            pBlendWeight, pBlendIdx,
            blendMatrices,
            srcPosStride, destPosStride,
            srcNormStride, destNormStride,
            blendWeightStride, blendIdxStride,
            numWeightsPerVertex,
            targetVertexData->vertexCount);
    }
    //---------------------------------------------------------------------
    void Mesh::softwareVertexMorph(Real t,
        const HardwareVertexBufferSharedPtr& b1,
        const HardwareVertexBufferSharedPtr& b2,
        VertexData* targetVertexData)
    {
        HardwareBufferLockGuard b1Lock(b1, HardwareBuffer::HBL_READ_ONLY);
        float* pb1 = static_cast<float*>(b1Lock.pData);
        HardwareBufferLockGuard b2Lock;
        float* pb2;
        if (b1.get() != b2.get())
        {
            b2Lock.lock(b2, HardwareBuffer::HBL_READ_ONLY);
            pb2 = static_cast<float*>(b2Lock.pData);
        }
        else
        {
            // Same buffer - track with only one entry or time index exactly matching
            // one keyframe
            // For simplicity of main code, interpolate still but with same val
            pb2 = pb1;
        }

        const VertexElement* posElem =
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        assert(posElem);
        const VertexElement* normElem =
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        
        bool morphNormals = false;
        if (normElem && normElem->getSource() == posElem->getSource() &&
            b1->getVertexSize() == 24 && b2->getVertexSize() == 24)
            morphNormals = true;
        
        HardwareVertexBufferSharedPtr destBuf =
            targetVertexData->vertexBufferBinding->getBuffer(
                posElem->getSource());
        assert((posElem->getSize() == destBuf->getVertexSize()
                || (morphNormals && posElem->getSize() + normElem->getSize() == destBuf->getVertexSize())) &&
            "Positions (or positions & normals) must be in a buffer on their own for morphing");
        HardwareBufferLockGuard destLock(destBuf, HardwareBuffer::HBL_DISCARD);
        float* pdst = static_cast<float*>(destLock.pData);

        OptimisedUtil::getImplementation()->softwareVertexMorph(
            t, pb1, pb2, pdst,
            b1->getVertexSize(), b2->getVertexSize(), destBuf->getVertexSize(),
            targetVertexData->vertexCount,
            morphNormals);
    }
    //---------------------------------------------------------------------
    void Mesh::softwareVertexPoseBlend(Real weight,
        const std::map<size_t, Vector3>& vertexOffsetMap,
        const std::map<size_t, Vector3>& normalsMap,
        VertexData* targetVertexData)
    {
        // Do nothing if no weight
        if (weight == 0.0f)
            return;

        const VertexElement* posElem =
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* normElem =
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        assert(posElem);
        // Support normals if they're in the same buffer as positions and pose includes them
        bool normals = normElem && !normalsMap.empty() && posElem->getSource() == normElem->getSource();
        HardwareVertexBufferSharedPtr destBuf =
            targetVertexData->vertexBufferBinding->getBuffer(
            posElem->getSource());

        size_t elemsPerVertex = destBuf->getVertexSize()/sizeof(float);

        // Have to lock in normal mode since this is incremental
        HardwareBufferLockGuard destLock(destBuf, HardwareBuffer::HBL_NORMAL);
        float* pBase = static_cast<float*>(destLock.pData);
                
        // Iterate over affected vertices
        for (const auto & i : vertexOffsetMap)
        {
            // Adjust pointer
            float *pdst = pBase + i.first*elemsPerVertex;

            *pdst = *pdst + (i.second.x * weight);
            ++pdst;
            *pdst = *pdst + (i.second.y * weight);
            ++pdst;
            *pdst = *pdst + (i.second.z * weight);
            ++pdst;
            
        }
        
        if (normals)
        {
            float* pNormBase;
            normElem->baseVertexPointerToElement((void*)pBase, &pNormBase);
            for (const auto & i : normalsMap)
            {
                // Adjust pointer
                float *pdst = pNormBase + i.first*elemsPerVertex;

                *pdst = *pdst + (i.second.x * weight);
                ++pdst;
                *pdst = *pdst + (i.second.y * weight);
                ++pdst;
                *pdst = *pdst + (i.second.z * weight);
                ++pdst;             
                
            }
        }
    }
    //---------------------------------------------------------------------
    size_t Mesh::calculateSize(void) const
    {
        // calculate GPU size
        size_t ret = 0;
        unsigned short i;
        // Shared vertices
        if (sharedVertexData)
        {
            for (i = 0;
                i < sharedVertexData->vertexBufferBinding->getBufferCount();
                ++i)
            {
                ret += sharedVertexData->vertexBufferBinding
                    ->getBuffer(i)->getSizeInBytes();
            }
        }

        for (auto *s : mSubMeshList)
        {
            // Dedicated vertices
            if (!s->useSharedVertices)
            {
                for (i = 0;
                    i < s->vertexData->vertexBufferBinding->getBufferCount();
                    ++i)
                {
                    ret += s->vertexData->vertexBufferBinding
                        ->getBuffer(i)->getSizeInBytes();
                }
            }
            if (s->indexData->indexBuffer)
            {
                // Index data
                ret += s->indexData->indexBuffer->getSizeInBytes();
            }

        }
        return ret;
    }
    //-----------------------------------------------------------------------------
    bool Mesh::hasVertexAnimation(void) const
    {
        return !mAnimationsList.empty();
    }
    //---------------------------------------------------------------------
    VertexAnimationType Mesh::getSharedVertexDataAnimationType(void) const
    {
        if (mAnimationTypesDirty)
        {
            _determineAnimationTypes();
        }

        return mSharedVertexDataAnimationType;
    }
    //---------------------------------------------------------------------
    void Mesh::_determineAnimationTypes(void) const
    {
        // Don't check flag here; since detail checks on track changes are not
        // done, allow caller to force if they need to

        // Initialise all types to nothing
        mSharedVertexDataAnimationType = VAT_NONE;
        mSharedVertexDataAnimationIncludesNormals = false;
        for (auto i : mSubMeshList)
        {
            i->mVertexAnimationType = VAT_NONE;
            i->mVertexAnimationIncludesNormals = false;
        }
        
        mPosesIncludeNormals = false;
        for (PoseList::const_iterator i = mPoseList.begin(); i != mPoseList.end(); ++i)
        {
            if (i == mPoseList.begin())
                mPosesIncludeNormals = (*i)->getIncludesNormals();
            else if (mPosesIncludeNormals != (*i)->getIncludesNormals())
                // only support normals if consistently included
                mPosesIncludeNormals = mPosesIncludeNormals && (*i)->getIncludesNormals();
        }

        // Scan all animations and determine the type of animation tracks
        // relating to each vertex data
        for(const auto& ai : mAnimationsList)
        {
            for (const auto& vit : ai.second->_getVertexTrackList())
            {
                VertexAnimationTrack* track = vit.second;
                ushort handle = vit.first;
                if (handle == 0)
                {
                    // shared data
                    if (mSharedVertexDataAnimationType != VAT_NONE &&
                        mSharedVertexDataAnimationType != track->getAnimationType())
                    {
                        // Mixing of morph and pose animation on same data is not allowed
                        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Animation tracks for shared vertex data on mesh "
                            + mName + " try to mix vertex animation types, which is "
                            "not allowed.",
                            "Mesh::_determineAnimationTypes");
                    }
                    mSharedVertexDataAnimationType = track->getAnimationType();
                    if (track->getAnimationType() == VAT_MORPH)
                        mSharedVertexDataAnimationIncludesNormals = track->getVertexAnimationIncludesNormals();
                    else 
                        mSharedVertexDataAnimationIncludesNormals = mPosesIncludeNormals;

                }
                else
                {
                    // submesh index (-1)
                    SubMesh* sm = getSubMesh(handle-1);
                    if (sm->mVertexAnimationType != VAT_NONE &&
                        sm->mVertexAnimationType != track->getAnimationType())
                    {
                        // Mixing of morph and pose animation on same data is not allowed
                        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Animation tracks for dedicated vertex data "
                            + StringConverter::toString(handle-1) + " on mesh "
                            + mName + " try to mix vertex animation types, which is "
                            "not allowed.",
                            "Mesh::_determineAnimationTypes");
                    }
                    sm->mVertexAnimationType = track->getAnimationType();
                    if (track->getAnimationType() == VAT_MORPH)
                        sm->mVertexAnimationIncludesNormals = track->getVertexAnimationIncludesNormals();
                    else 
                        sm->mVertexAnimationIncludesNormals = mPosesIncludeNormals;

                }
            }
        }

        mAnimationTypesDirty = false;
    }
    //---------------------------------------------------------------------
    Animation* Mesh::createAnimation(const String& name, Real length)
    {
        // Check name not used
        if (mAnimationsList.find(name) != mAnimationsList.end())
        {
            OGRE_EXCEPT(
                Exception::ERR_DUPLICATE_ITEM,
                "An animation with the name " + name + " already exists",
                "Mesh::createAnimation");
        }

        Animation* ret = OGRE_NEW Animation(name, length);
        ret->_notifyContainer(this);

        // Add to list
        mAnimationsList[name] = ret;

        // Mark animation types dirty
        mAnimationTypesDirty = true;

        return ret;

    }
    //---------------------------------------------------------------------
    Animation* Mesh::getAnimation(const String& name) const
    {
        Animation* ret = _getAnimationImpl(name);
        if (!ret)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                "No animation entry found named " + name,
                "Mesh::getAnimation");
        }

        return ret;
    }
    //---------------------------------------------------------------------
    Animation* Mesh::getAnimation(unsigned short index) const
    {
        // If you hit this assert, then the index is out of bounds.
        assert( index < mAnimationsList.size() );

        AnimationList::const_iterator i = mAnimationsList.begin();

        std::advance(i, index);

        return i->second;

    }
    //---------------------------------------------------------------------
    unsigned short Mesh::getNumAnimations(void) const
    {
        return static_cast<unsigned short>(mAnimationsList.size());
    }
    //---------------------------------------------------------------------
    bool Mesh::hasAnimation(const String& name) const
    {
        return _getAnimationImpl(name) != 0;
    }
    //---------------------------------------------------------------------
    Animation* Mesh::_getAnimationImpl(const String& name) const
    {
        Animation* ret = 0;
        AnimationList::const_iterator i = mAnimationsList.find(name);

        if (i != mAnimationsList.end())
        {
            ret = i->second;
        }

        return ret;

    }
    //---------------------------------------------------------------------
    void Mesh::removeAnimation(const String& name)
    {
        AnimationList::iterator i = mAnimationsList.find(name);

        if (i == mAnimationsList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + name,
                "Mesh::getAnimation");
        }

        OGRE_DELETE i->second;

        mAnimationsList.erase(i);

        mAnimationTypesDirty = true;
    }
    //---------------------------------------------------------------------
    void Mesh::removeAllAnimations(void)
    {
        for (auto& a : mAnimationsList)
        {
            OGRE_DELETE a.second;
        }
        mAnimationsList.clear();
        mAnimationTypesDirty = true;
    }
    //---------------------------------------------------------------------
    VertexData* Mesh::getVertexDataByTrackHandle(unsigned short handle)
    {
        if (handle == 0)
        {
            return sharedVertexData;
        }
        else
        {
            return getSubMesh(handle-1)->vertexData;
        }
    }
    //---------------------------------------------------------------------
    Pose* Mesh::createPose(ushort target, const String& name)
    {
        Pose* retPose = OGRE_NEW Pose(target, name);
        mPoseList.push_back(retPose);
        return retPose;
    }
    //---------------------------------------------------------------------
    Pose* Mesh::getPose(const String& name) const
    {
        for (auto i : mPoseList)
        {
            if (i->getName() == name)
                return i;
        }
        StringStream str;
        str << "No pose called " << name << " found in Mesh " << mName;
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            str.str(),
            "Mesh::getPose");

    }
    //---------------------------------------------------------------------
    void Mesh::removePose(ushort index)
    {
        OgreAssert(index < mPoseList.size(), "");
        PoseList::iterator i = mPoseList.begin();
        std::advance(i, index);
        OGRE_DELETE *i;
        mPoseList.erase(i);

    }
    //---------------------------------------------------------------------
    void Mesh::removePose(const String& name)
    {
        for (PoseList::iterator i = mPoseList.begin(); i != mPoseList.end(); ++i)
        {
            if ((*i)->getName() == name)
            {
                OGRE_DELETE *i;
                mPoseList.erase(i);
                return;
            }
        }
        StringStream str;
        str << "No pose called " << name << " found in Mesh " << mName;
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
            str.str(),
            "Mesh::removePose");
    }
    //---------------------------------------------------------------------
    void Mesh::removeAllPoses(void)
    {
        for (auto & i : mPoseList)
        {
            OGRE_DELETE i;
        }
        mPoseList.clear();
    }
    //---------------------------------------------------------------------
    Mesh::PoseIterator Mesh::getPoseIterator(void)
    {
        return PoseIterator(mPoseList.begin(), mPoseList.end());
    }
    //---------------------------------------------------------------------
    Mesh::ConstPoseIterator Mesh::getPoseIterator(void) const
    {
        return ConstPoseIterator(mPoseList.begin(), mPoseList.end());
    }
    //-----------------------------------------------------------------------------
    const PoseList& Mesh::getPoseList(void) const
    {
        return mPoseList;
    }
    //---------------------------------------------------------------------
    const LodStrategy *Mesh::getLodStrategy() const
    {
        return mLodStrategy;
    }
#if !OGRE_NO_MESHLOD
    //---------------------------------------------------------------------
    void Mesh::setLodStrategy(LodStrategy *lodStrategy)
    {
        mLodStrategy = lodStrategy;

        assert(mMeshLodUsageList.size());
        
        // Re-transform user LOD values (starting at index 1, no need to transform base value)
        for (MeshLodUsageList::iterator i = mMeshLodUsageList.begin()+1; i != mMeshLodUsageList.end(); ++i)
            i->value = mLodStrategy->transformUserValue(i->userValue);

        // Rewrite first value
        mMeshLodUsageList[0].value = mLodStrategy->getBaseValue();
    }
#endif

    void Mesh::_convertVertexElement(VertexElementSemantic semantic, VertexElementType dstType)
    {
        if (sharedVertexData)
            sharedVertexData->convertVertexElement(semantic, dstType);

        for (auto s : getSubMeshes())
            if (s->vertexData)
                s->vertexData->convertVertexElement(semantic, dstType);
    }
}

