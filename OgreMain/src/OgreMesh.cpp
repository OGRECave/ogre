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
#include "OgreStableHeaders.h"
#include "OgreMesh.h"

#include "OgreSubMesh.h"
#include "OgreLogManager.h"
#include "OgreMeshSerializer.h"
#include "OgreSkeletonManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreMeshManager.h"
#include "OgreEdgeListBuilder.h"
#include "OgreAnimation.h"
#include "OgreAnimationState.h"
#include "OgreAnimationTrack.h"
#include "OgreOptimisedUtil.h"
#include "OgreTangentSpaceCalc.h"
#include "OgreLodStrategyManager.h"


namespace Ogre {
    //-----------------------------------------------------------------------
    MeshPtr::MeshPtr(const ResourcePtr& r) : SharedPtr<Mesh>()
    {
		// lock & copy other mutex pointer
        OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
        {
		    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<Mesh*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }
    }
    //-----------------------------------------------------------------------
    MeshPtr& MeshPtr::operator=(const ResourcePtr& r)
    {
        if (pRep == static_cast<Mesh*>(r.getPointer()))
            return *this;
        release();
		// lock & copy other mutex pointer
        OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
        {
		    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = static_cast<Mesh*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }
		else
		{
			// RHS must be a null pointer
			assert(r.isNull() && "RHS must be null if it has no mutex!");
			setNull();
		}
        return *this;
    }
    //-----------------------------------------------------------------------
    void MeshPtr::destroy(void)
    {
        // We're only overriding so that we can destroy after full definition of Mesh
        SharedPtr<Mesh>::destroy();
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    Mesh::Mesh(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader),
        mBoundRadius(0.0f),
        mBoneAssignmentsOutOfDate(false),
        mIsLodManual(false),
        mNumLods(1),
        mVertexBufferUsage(HardwareBuffer::HBU_STATIC_WRITE_ONLY),
        mIndexBufferUsage(HardwareBuffer::HBU_STATIC_WRITE_ONLY),
        mVertexBufferShadowBuffer(true),
        mIndexBufferShadowBuffer(true),
        mPreparedForShadowVolumes(false),
        mEdgeListsBuilt(false),
        mAutoBuildEdgeLists(true), // will be set to false by serializers of 1.30 and above
		mSharedVertexDataAnimationType(VAT_NONE),
		mAnimationTypesDirty(true),
		sharedVertexData(0)
    {

        // Initialise to default strategy
        mLodStrategy = LodStrategyManager::getSingleton().getDefaultStrategy();

		// Init first (manual) lod
		MeshLodUsage lod;
        lod.userValue = std::numeric_limits<Real>::quiet_NaN(); // User value not used for base lod level
		lod.value = mLodStrategy->getBaseValue();
        lod.edgeData = NULL;
        lod.manualMesh.setNull();
		mMeshLodUsageList.push_back(lod);

    }
    //-----------------------------------------------------------------------
    Mesh::~Mesh()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
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
        if (index >= mSubMeshList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Index out of bounds.",
						"Mesh::removeSubMesh");
        }
		SubMeshList::iterator i = mSubMeshList.begin();
		std::advance(i, index);
		mSubMeshList.erase(i);
		
		if (isLoaded())
			_dirtyState();
		
	}
    //-----------------------------------------------------------------------
    void Mesh::destroySubMesh(const String& name)
	{
		unsigned short index = _getSubMeshIndex(name);
		destroySubMesh(index);
	}
	//-----------------------------------------------------------------------
    unsigned short Mesh::getNumSubMeshes() const
    {
        return static_cast< unsigned short >( mSubMeshList.size() );
    }

    //---------------------------------------------------------------------
	void Mesh::nameSubMesh(const String& name, ushort index)
	{
		mSubMeshNameMap[name] = index ;
	}

    //-----------------------------------------------------------------------
    SubMesh* Mesh::getSubMesh(const String& name) const
	{
		ushort index = _getSubMeshIndex(name);
		return getSubMesh(index);
	}
    //-----------------------------------------------------------------------
    SubMesh* Mesh::getSubMesh(unsigned short index) const
    {
        if (index >= mSubMeshList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds.",
                "Mesh::getSubMesh");
        }

        return mSubMeshList[index];
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

        // The loading process accesses lod usages directly, so
        // transformation of user values must occur after loading is complete.

        // Transform user lod values (starting at index 1, no need to transform base value)
		for (MeshLodUsageList::iterator i = mMeshLodUsageList.begin(); i != mMeshLodUsageList.end(); ++i)
            i->value = mLodStrategy->transformUserValue(i->userValue);
	}
	//-----------------------------------------------------------------------
    void Mesh::prepareImpl()
    {
        // Load from specified 'name'
        if (getCreator()->getVerbose())
            LogManager::getSingleton().logMessage("Mesh: Loading "+mName+".");

        mFreshFromDisk =
            ResourceGroupManager::getSingleton().openResource(
				mName, mGroup, true, this);
 
        // fully prebuffer into host RAM
        mFreshFromDisk = DataStreamPtr(OGRE_NEW MemoryDataStream(mName,mFreshFromDisk));
    }
    //-----------------------------------------------------------------------
    void Mesh::unprepareImpl()
    {
        mFreshFromDisk.setNull();
    }
    void Mesh::loadImpl()
    {
        MeshSerializer serializer;
        serializer.setListener(MeshManager::getSingleton().getListener());

        // If the only copy is local on the stack, it will be cleaned
        // up reliably in case of exceptions, etc
        DataStreamPtr data(mFreshFromDisk);
        mFreshFromDisk.setNull();

        if (data.isNull()) {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Data doesn't appear to have been prepared in " + mName,
                        "Mesh::loadImpl()");
        }

		serializer.importMesh(data, this);

        /* check all submeshes to see if their materials should be
           updated.  If the submesh has texture aliases that match those
           found in the current material then a new material is created using
           the textures from the submesh.
        */
        updateMaterialForAllSubMeshes();
    }

    //-----------------------------------------------------------------------
    void Mesh::unloadImpl()
    {
        // Teardown submeshes
        for (SubMeshList::iterator i = mSubMeshList.begin();
            i != mSubMeshList.end(); ++i)
        {
            OGRE_DELETE *i;
        }
        if (sharedVertexData)
        {
            OGRE_DELETE sharedVertexData;
            sharedVertexData = NULL;
        }
		// Clear SubMesh lists
		mSubMeshList.clear();
		mSubMeshNameMap.clear();
        // Removes all LOD data
        removeLodLevels();
        mPreparedForShadowVolumes = false;

		// remove all poses & animations
		removeAllAnimations();
		removeAllPoses();

        // Clear bone assignments
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = false;

        // Removes reference to skeleton
        setSkeletonName(StringUtil::BLANK);
    }

    //-----------------------------------------------------------------------
    MeshPtr Mesh::clone(const String& newName, const String& newGroup)
    {
        // This is a bit like a copy constructor, but with the additional aspect of registering the clone with
        //  the MeshManager

        // New Mesh is assumed to be manually defined rather than loaded since you're cloning it for a reason
        String theGroup;
        if (newGroup == StringUtil::BLANK)
        {
            theGroup = this->getGroup();
        }
        else
        {
            theGroup = newGroup;
        }
        MeshPtr newMesh = MeshManager::getSingleton().createManual(newName, theGroup);

        // Copy submeshes first
        vector<SubMesh*>::type::iterator subi;
        SubMesh* newSub;
        for (subi = mSubMeshList.begin(); subi != mSubMeshList.end(); ++subi)
        {
            newSub = newMesh->createSubMesh();
            newSub->mMaterialName = (*subi)->mMaterialName;
            newSub->mMatInitialised = (*subi)->mMatInitialised;
            newSub->operationType = (*subi)->operationType;
            newSub->useSharedVertices = (*subi)->useSharedVertices;
            newSub->extremityPoints = (*subi)->extremityPoints;

            if (!(*subi)->useSharedVertices)
            {
                // Copy unique vertex data
				newSub->vertexData = (*subi)->vertexData->clone();
                // Copy unique index map
                newSub->blendIndexToBoneIndexMap = (*subi)->blendIndexToBoneIndexMap;
            }

            // Copy index data
            OGRE_DELETE newSub->indexData;
			newSub->indexData = (*subi)->indexData->clone();
            // Copy any bone assignments
            newSub->mBoneAssignments = (*subi)->mBoneAssignments;
            newSub->mBoneAssignmentsOutOfDate = (*subi)->mBoneAssignmentsOutOfDate;
            // Copy texture aliases
            newSub->mTextureAliases = (*subi)->mTextureAliases;

            // Copy lod face lists
            newSub->mLodFaceList.reserve((*subi)->mLodFaceList.size());
            ProgressiveMesh::LODFaceList::const_iterator facei;
            for (facei = (*subi)->mLodFaceList.begin(); facei != (*subi)->mLodFaceList.end(); ++facei) {
                IndexData* newIndexData = (*facei)->clone();
                newSub->mLodFaceList.push_back(newIndexData);
            }
        }

        // Copy shared geometry and index map, if any
        if (sharedVertexData)
        {
            newMesh->sharedVertexData = sharedVertexData->clone();
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

        newMesh->mLodStrategy = mLodStrategy;
		newMesh->mIsLodManual = mIsLodManual;
		newMesh->mNumLods = mNumLods;
		newMesh->mMeshLodUsageList = mMeshLodUsageList;
        // Unreference edge lists, otherwise we'll delete the same lot twice, build on demand
        MeshLodUsageList::iterator lodi;
        for (lodi = newMesh->mMeshLodUsageList.begin(); lodi != newMesh->mMeshLodUsageList.end(); ++lodi) {
            MeshLodUsage& lod = *lodi;
            lod.edgeData = NULL;
            // TODO: Copy manual lod meshes
        }

		newMesh->mVertexBufferUsage = mVertexBufferUsage;
		newMesh->mIndexBufferUsage = mIndexBufferUsage;
		newMesh->mVertexBufferShadowBuffer = mVertexBufferShadowBuffer;
		newMesh->mIndexBufferShadowBuffer = mIndexBufferShadowBuffer;

        newMesh->mSkeletonName = mSkeletonName;
        newMesh->mSkeleton = mSkeleton;

		// Keep prepared shadow volume info (buffers may already be prepared)
		newMesh->mPreparedForShadowVolumes = mPreparedForShadowVolumes;

		// mEdgeListsBuilt and edgeData of mMeshLodUsageList
		// will up to date on demand. Not copied since internal references, and mesh
		// data may be altered
		
		// Clone vertex animation
		for (AnimationList::iterator i = mAnimationsList.begin();
			i != mAnimationsList.end(); ++i)
		{
			Animation *newAnim = i->second->clone(i->second->getName());
			newMesh->mAnimationsList[i->second->getName()] = newAnim;
		}
		// Clone pose list
		for (PoseList::iterator i = mPoseList.begin(); i != mPoseList.end(); ++i)
		{
			Pose* newPose = (*i)->clone();
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
        Vector3 max = mAABB.getMaximum();
        Vector3 min = mAABB.getMinimum();
		mBoundRadius = Math::boundingRadiusFromAABB(mAABB);

        if (pad)
        {
            // Pad out the AABB a little, helps with most bounds tests
            Vector3 scaler = (max - min) * MeshManager::getSingleton().getBoundsPaddingFactor();
            mAABB.setExtents(min  - scaler, max + scaler);
            // Pad out the sphere a little too
            mBoundRadius = mBoundRadius + (mBoundRadius * MeshManager::getSingleton().getBoundsPaddingFactor());
        }
        else
        {
            mAABB.setExtents(min, max);
            mBoundRadius = mBoundRadius;
        }

    }
    //-----------------------------------------------------------------------
    void Mesh::_setBoundingSphereRadius(Real radius)
    {
        mBoundRadius = radius;
    }
    //-----------------------------------------------------------------------
    void Mesh::setSkeletonName(const String& skelName)
    {
		if (skelName != mSkeletonName)
		{
			mSkeletonName = skelName;

			if (skelName.empty())
			{
				// No skeleton
				mSkeleton.setNull();
			}
			else
			{
				// Load skeleton
				try {
					mSkeleton = SkeletonManager::getSingleton().load(skelName, mGroup);
				}
				catch (...)
				{
					mSkeleton.setNull();
					// Log this error
					String msg = "Unable to load skeleton ";
					msg += skelName + " for Mesh " + mName
						+ ". This Mesh will not be animated. "
						+ "You can ignore this message if you are using an offline tool.";
					LogManager::getSingleton().logMessage(msg);

				}


			}
			if (isLoaded())
				_dirtyState();
		}
    }
    //-----------------------------------------------------------------------
    bool Mesh::hasSkeleton(void) const
    {
        return !(mSkeletonName.empty());
    }
    //-----------------------------------------------------------------------
    const SkeletonPtr& Mesh::getSkeleton(void) const
    {
        return mSkeleton;
    }
    //-----------------------------------------------------------------------
    void Mesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        mBoneAssignments.insert(
            VertexBoneAssignmentList::value_type(vertBoneAssign.vertexIndex, vertBoneAssign));
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
		if (hasSkeleton())
		{
			// Delegate to Skeleton
			assert(!mSkeleton.isNull() && "Skeleton not present");
			mSkeleton->_initAnimationState(animSet);

			// Take the opportunity to update the compiled bone assignments
            _updateCompiledBoneAssignments();
		}

		// Animation states for vertex animation
		for (AnimationList::iterator i = mAnimationsList.begin();
			i != mAnimationsList.end(); ++i)
		{
			// Only create a new animation state if it doesn't exist
			// We can have the same named animation in both skeletal and vertex
			// with a shared animation state affecting both, for combined effects
			// The animations should be the same length if this feature is used!
			if (!animSet->hasAnimationState(i->second->getName()))
			{
				animSet->createAnimationState(i->second->getName(), 0.0,
					i->second->getLength());
			}

		}

    }
	//---------------------------------------------------------------------
	void Mesh::_refreshAnimationState(AnimationStateSet* animSet)
	{
		if (hasSkeleton())
		{
			mSkeleton->_refreshAnimationState(animSet);
		}

		// Merge in any new vertex animations
		AnimationList::iterator i;
		for (i = mAnimationsList.begin(); i != mAnimationsList.end(); ++i)
		{
			Animation* anim = i->second;
			// Create animation at time index 0, default params mean this has weight 1 and is disabled
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

        SubMeshList::iterator i;
        for (i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
        {
            if ((*i)->mBoneAssignmentsOutOfDate)
            {
                (*i)->_compileBoneAssignments();
            }
        }
    }
    //-----------------------------------------------------------------------
    typedef multimap<Real, Mesh::VertexBoneAssignmentList::iterator>::type WeightIteratorMap;
    unsigned short Mesh::_rationaliseBoneAssignments(size_t vertexCount, Mesh::VertexBoneAssignmentList& assignments)
    {
        // Iterate through, finding the largest # bones per vertex
        unsigned short maxBones = 0;
		bool existsNonSkinnedVertices = false;
        VertexBoneAssignmentList::iterator i;

        for (size_t v = 0; v < vertexCount; ++v)
        {
            // Get number of entries for this vertex
            unsigned short currBones = static_cast<unsigned short>(assignments.count(v));
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
                    weightToAssignmentMap.insert(
                        WeightIteratorMap::value_type(i->second.weight, i));
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
            LogManager::getSingleton().logMessage("WARNING: the mesh '" + mName + "' "
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
            LogManager::getSingleton().logMessage("WARNING: the mesh '" + mName + "' "
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
        unsigned short maxBones =
            _rationaliseBoneAssignments(sharedVertexData->vertexCount, mBoneAssignments);

        if (maxBones != 0)
        {
            compileBoneAssignments(mBoneAssignments, maxBones, 
                sharedBlendIndexToBoneIndexMap, sharedVertexData);
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

        typedef set<unsigned short>::type BoneIndexSet;
        BoneIndexSet usedBoneIndices;

        // Collect actually used bones
        VertexBoneAssignmentList::const_iterator itVBA, itendVBA;
        itendVBA = boneAssignments.end();
        for (itVBA = boneAssignments.begin(); itVBA != itendVBA; ++itVBA)
        {
            usedBoneIndices.insert(itVBA->second.boneIndex);
        }

        // Allocate space for index map
        blendIndexToBoneIndexMap.resize(usedBoneIndices.size());
        boneIndexToBlendIndexMap.resize(*usedBoneIndices.rbegin() + 1);

        // Make index map between bone index and blend index
        BoneIndexSet::const_iterator itBoneIndex, itendBoneIndex;
        unsigned short blendIndex = 0;
        itendBoneIndex = usedBoneIndices.end();
        for (itBoneIndex = usedBoneIndices.begin(); itBoneIndex != itendBoneIndex; ++itBoneIndex, ++blendIndex)
        {
            boneIndexToBlendIndexMap[*itBoneIndex] = blendIndex;
            blendIndexToBoneIndexMap[blendIndex] = *itBoneIndex;
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
        // Weights are more specific though since they are Reals
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

        HardwareVertexBufferSharedPtr vbuf =
            HardwareBufferManager::getSingleton().createVertexBuffer(
                sizeof(unsigned char)*4 + sizeof(float)*numBlendWeightsPerVertex,
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
                decl->insertElement(insertPoint+1, bindIndex, sizeof(unsigned char)*4,
                VertexElement::multiplyTypeCount(VET_FLOAT1, numBlendWeightsPerVertex),
                VES_BLEND_WEIGHTS);
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
                decl->addElement(bindIndex, sizeof(unsigned char)*4,
                VertexElement::multiplyTypeCount(VET_FLOAT1, numBlendWeightsPerVertex),
                VES_BLEND_WEIGHTS);
            pIdxElem = &idxElem;
            pWeightElem = &wtElem;
        }

        // Assign data
        size_t v;
        VertexBoneAssignmentList::const_iterator i, iend;
        i = boneAssignments.begin();
		iend = boneAssignments.end();
        unsigned char *pBase = static_cast<unsigned char*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        // Iterate by vertex
        float *pWeight;
        unsigned char *pIndex;
        for (v = 0; v < targetVertexData->vertexCount; ++v)
        {
            /// Convert to specific pointers
            pWeightElem->baseVertexPointerToElement(pBase, &pWeight);
            pIdxElem->baseVertexPointerToElement(pBase, &pIndex);
            for (unsigned short bone = 0; bone < numBlendWeightsPerVertex; ++bone)
            {
                // Do we still have data for this vertex?
                if (i != iend && i->second.vertexIndex == v)
                {
                    // If so, write weight
                    *pWeight++ = i->second.weight;
                    *pIndex++ = boneIndexToBlendIndexMap[i->second.boneIndex];
                    ++i;
                }
                else
                {
                    // Ran out of assignments for this vertex, use weight 0 to indicate empty
                    *pWeight++ = 0.0f;
                    *pIndex++ = 0;
                }
            }
            pBase += vbuf->getVertexSize();
        }

        vbuf->unlock();

    }
    //---------------------------------------------------------------------
    void Mesh::_notifySkeleton(SkeletonPtr& pSkel)
    {
        mSkeleton = pSkel;
        mSkeletonName = pSkel->getName();
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
        return mSkeletonName;
    }
    //---------------------------------------------------------------------
    void Mesh::generateLodLevels(const LodValueList& lodValues,
        ProgressiveMesh::VertexReductionQuota reductionMethod, Real reductionValue)
    {
#if OGRE_DEBUG_MODE
        mLodStrategy->assertSorted(lodValues);
#endif

        removeLodLevels();

		LogManager::getSingleton().stream()
			<< "Generating " << lodValues.size()
			<< " lower LODs for mesh " << mName;

        SubMeshList::iterator isub, isubend;
        isubend = mSubMeshList.end();
        for (isub = mSubMeshList.begin(); isub != isubend; ++isub)
        {
            // check if triangles are present
            if ((*isub)->indexData->indexCount > 0)
            {
                // Set up data for reduction
                VertexData* pVertexData = (*isub)->useSharedVertices ? sharedVertexData : (*isub)->vertexData;

                ProgressiveMesh pm(pVertexData, (*isub)->indexData);
                pm.build(
                static_cast<ushort>(lodValues.size()),
                    &((*isub)->mLodFaceList),
                    reductionMethod, reductionValue);

            }
            else
            {
                // create empty index data for each lod
                for (size_t i = 0; i < lodValues.size(); ++i)
                {
                    (*isub)->mLodFaceList.push_back(OGRE_NEW IndexData);
                }
            }
        }

        // Iterate over the lods and record usage
        LodValueList::const_iterator ivalue, ivalueend;
        ivalueend = lodValues.end();
        mMeshLodUsageList.resize(lodValues.size() + 1);
        MeshLodUsageList::iterator ilod = mMeshLodUsageList.begin();
        for (ivalue = lodValues.begin(); ivalue != ivalueend; ++ivalue)
        {
            // Record usage
            MeshLodUsage& lod = *++ilod;
            lod.userValue = (*ivalue);
            lod.value = mLodStrategy->transformUserValue(lod.userValue);
            lod.edgeData = 0;
            lod.manualMesh.setNull();
        }
        mNumLods = static_cast<ushort>(lodValues.size() + 1);
    }
    //---------------------------------------------------------------------
    ushort Mesh::getNumLodLevels(void) const
    {
        return mNumLods;
    }
    //---------------------------------------------------------------------
    const MeshLodUsage& Mesh::getLodLevel(ushort index) const
    {
        assert(index < mMeshLodUsageList.size());
        if (mIsLodManual && index > 0 && mMeshLodUsageList[index].manualMesh.isNull())
        {
            // Load the mesh now
			try {
				mMeshLodUsageList[index].manualMesh =
					MeshManager::getSingleton().load(
						mMeshLodUsageList[index].manualName,
						mGroup);
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
    }
    //---------------------------------------------------------------------
	void Mesh::createManualLodLevel(Real lodValue, const String& meshName)
	{

		// Basic prerequisites
        assert((mIsLodManual || mNumLods == 1) && "Generated LODs already in use!");

		mIsLodManual = true;
		MeshLodUsage lod;
		lod.userValue = lodValue;
        lod.value = mLodStrategy->transformUserValue(lod.userValue);
		lod.manualName = meshName;
		lod.manualMesh.setNull();
        lod.edgeData = 0;
		mMeshLodUsageList.push_back(lod);
		++mNumLods;

        mLodStrategy->sort(mMeshLodUsageList);
	}
    //---------------------------------------------------------------------
	void Mesh::updateManualLodLevel(ushort index, const String& meshName)
	{

		// Basic prerequisites
		assert(mIsLodManual && "Not using manual LODs!");
		assert(index != 0 && "Can't modify first lod level (full detail)");
		assert(index < mMeshLodUsageList.size() && "Index out of bounds");
		// get lod
		MeshLodUsage* lod = &(mMeshLodUsageList[index]);

		lod->manualName = meshName;
		lod->manualMesh.setNull();
        if (lod->edgeData) OGRE_DELETE lod->edgeData;
        lod->edgeData = 0;
	}
    //---------------------------------------------------------------------
	ushort Mesh::getLodIndex(Real value) const
	{
        // Get index from strategy
        return mLodStrategy->getIndex(value, mMeshLodUsageList);
	}
    //---------------------------------------------------------------------
	void Mesh::_setLodInfo(unsigned short numLevels, bool isManual)
	{
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

		// Basic prerequisites
        assert(numLevels > 0 && "Must be at least one level (full detail level must exist)");

		mNumLods = numLevels;
		mMeshLodUsageList.resize(numLevels);
		// Resize submesh face data lists too
		for (SubMeshList::iterator i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
		{
			(*i)->mLodFaceList.resize(numLevels - 1);
		}
		mIsLodManual = isManual;
	}
    //---------------------------------------------------------------------
	void Mesh::_setLodUsage(unsigned short level, MeshLodUsage& usage)
	{
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

		// Basic prerequisites
		assert(level != 0 && "Can't modify first lod level (full detail)");
		assert(level < mMeshLodUsageList.size() && "Index out of bounds");

		mMeshLodUsageList[level] = usage;
	}
    //---------------------------------------------------------------------
	void Mesh::_setSubMeshLodFaceList(unsigned short subIdx, unsigned short level,
		IndexData* facedata)
	{
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

		// Basic prerequisites
		assert(!mIsLodManual && "Not using generated LODs!");
        assert(subIdx <= mSubMeshList.size() && "Index out of bounds");
		assert(level != 0 && "Can't modify first lod level (full detail)");
		assert(level <= mSubMeshList[subIdx]->mLodFaceList.size() && "Index out of bounds");

		SubMesh* sm = mSubMeshList[subIdx];
		sm->mLodFaceList[level - 1] = facedata;

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
    //---------------------------------------------------------------------
    void Mesh::removeLodLevels(void)
    {
        if (!mIsLodManual)
        {
            // Remove data from SubMeshes
            SubMeshList::iterator isub, isubend;
            isubend = mSubMeshList.end();
            for (isub = mSubMeshList.begin(); isub != isubend; ++isub)
            {
                (*isub)->removeLodLevels();
            }
        }

        freeEdgeList();
        mMeshLodUsageList.clear();

        // Reinitialise
        mNumLods = 1;
		// Init first (manual) lod
		MeshLodUsage lod;
        lod.userValue = std::numeric_limits<Real>::quiet_NaN();
		lod.value = mLodStrategy->getBaseValue();
        lod.edgeData = 0;
        lod.manualMesh.setNull();
		mMeshLodUsageList.push_back(lod);
		mIsLodManual = false;


    }
    //---------------------------------------------------------------------
    Real Mesh::getBoundingSphereRadius(void) const
    {
        return mBoundRadius;
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
		    newBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
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
            unsigned char* pSrc = static_cast<unsigned char*>(
                origBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
            unsigned char* pDest = static_cast<unsigned char*>(
                newBuffer->lock(HardwareBuffer::HBL_DISCARD));
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
            origBuffer->unlock();
            newBuffer->unlock();

            // Rebind the new buffer
            vBind->setBinding(prevTexCoordElem->getSource(), newBuffer);
	    }
    }
    //---------------------------------------------------------------------
    void Mesh::buildTangentVectors(VertexElementSemantic targetSemantic, 
		unsigned short sourceTexCoordSet, unsigned short index, 
		bool splitMirrored, bool splitRotated, bool storeParityInW)
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
			for (SubMeshList::iterator i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
			{
				SubMesh* sm = *i;
				if (sm->useSharedVertices)
				{
					tangentsCalc.addIndexData(sm->indexData);
					found = true;
				}
			}
			if (found)
			{
				TangentSpaceCalc::Result res = 
					tangentsCalc.build(targetSemantic, sourceTexCoordSet, index);

				// If any vertex splitting happened, we have to give them bone assignments
				if (getSkeletonName() != StringUtil::BLANK)
				{
					for (TangentSpaceCalc::IndexRemapList::iterator r = res.indexesRemapped.begin(); 
						r != res.indexesRemapped.end(); ++r)
					{
						TangentSpaceCalc::IndexRemap& remap = *r;
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
			}
		}

		// Dedicated geometry
		for (SubMeshList::iterator i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
		{
			SubMesh* sm = *i;
			if (!sm->useSharedVertices)
			{
				tangentsCalc.clear();
				tangentsCalc.setVertexData(sm->vertexData);
				tangentsCalc.addIndexData(sm->indexData);
				TangentSpaceCalc::Result res = 
					tangentsCalc.build(targetSemantic, sourceTexCoordSet, index);

				// If any vertex splitting happened, we have to give them bone assignments
				if (getSkeletonName() != StringUtil::BLANK)
				{
					for (TangentSpaceCalc::IndexRemapList::iterator r = res.indexesRemapped.begin(); 
						r != res.indexesRemapped.end(); ++r)
					{
						TangentSpaceCalc::IndexRemap& remap = *r;
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
    bool Mesh::suggestTangentVectorBuildParams(VertexElementSemantic targetSemantic,
		unsigned short& outSourceCoordSet, unsigned short& outIndex)
    {
        // Go through all the vertex data and locate source and dest (must agree)
        bool sharedGeometryDone = false;
        bool foundExisting = false;
		VertexElementSemantic foundSemantic = VES_TEXTURE_COORDINATES;
        bool firstOne = true;
        SubMeshList::iterator i, iend;
        iend = mSubMeshList.end();
        for (i = mSubMeshList.begin(); i != iend; ++i)
        {
            SubMesh* sm = *i;
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
                const VertexElement* testElem =
                    vertexData->vertexDeclaration->findElementBySemantic(
                        VES_TEXTURE_COORDINATES, targetIndex);
                if (!testElem)
                    break; // finish if we've run out, t will be the target

                if (!sourceElem)
                {
                    // We're still looking for the source texture coords
                    if (testElem->getType() == VET_FLOAT2)
                    {
                        // Ok, we found it
                        sourceElem = testElem;
                    }
                }
                
				if(!foundExisting && targetSemantic == VES_TEXTURE_COORDINATES)
                {
                    // We're looking for the destination
                    // Check to see if we've found a possible
                    if (testElem->getType() == VET_FLOAT3)
                    {
                        // This is a 3D set, might be tangents
                        foundExisting = true;
						foundSemantic = VES_TEXTURE_COORDINATES;
                    }

                }

            }

			if (!foundExisting && targetSemantic != VES_TEXTURE_COORDINATES)
			{
				targetIndex = 0;
				// Look for existing semantic
				const VertexElement* testElem =
					vertexData->vertexDeclaration->findElementBySemantic(
					targetSemantic, targetIndex);
				if (testElem)
				{
					foundExisting = true;
					foundSemantic = targetSemantic;
				}

			}

            // After iterating, we should have a source and a possible destination (t)
            if (!sourceElem)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                    "Cannot locate an appropriate 2D texture coordinate set for "
                    "all the vertex data in this mesh to create tangents from. ",
                    "Mesh::suggestTangentVectorBuildParams");
            }
            // Check that we agree with previous decisions, if this is not the
            // first one, and if we're not just using the existing one
            if (!firstOne && !foundExisting)
            {
                if (sourceElem->getIndex() != outSourceCoordSet)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Multiple sets of vertex data in this mesh disagree on "
                        "the appropriate index to use for the source texture coordinates. "
                        "This ambiguity must be rectified before tangents can be generated.",
                        "Mesh::suggestTangentVectorBuildParams");
                }
                if (targetIndex != outIndex)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Multiple sets of vertex data in this mesh disagree on "
                        "the appropriate index to use for the target texture coordinates. "
                        "This ambiguity must be rectified before tangents can be generated.",
                        "Mesh::suggestTangentVectorBuildParams");
                }
            }

            // Otherwise, save this result
            outSourceCoordSet = sourceElem->getIndex();
            outIndex = targetIndex;

            firstOne = false;

       }

        return foundExisting;

    }
    //---------------------------------------------------------------------
    void Mesh::buildEdgeList(void)
    {
        if (mEdgeListsBuilt)
            return;

        // Loop over LODs
        for (unsigned int lodIndex = 0; lodIndex < mMeshLodUsageList.size(); ++lodIndex)
        {
            // use getLodLevel to enforce loading of manual mesh lods
            MeshLodUsage& usage = const_cast<MeshLodUsage&>(getLodLevel(lodIndex));

			bool atLeastOneIndexSet = false;

            if (mIsLodManual && lodIndex != 0)
            {
                // Delegate edge building to manual mesh
                // It should have already built it's own edge list while loading
				if (!usage.manualMesh.isNull())
				{
					usage.edgeData = usage.manualMesh->getEdgeList(0);
				}
            }
            else
            {
                // Build
                EdgeListBuilder eb;
                size_t vertexSetCount = 0;

                if (sharedVertexData)
                {
                    eb.addVertexData(sharedVertexData);
                    vertexSetCount++;
                }

                // Prepare the builder using the submesh information
                SubMeshList::iterator i, iend;
                iend = mSubMeshList.end();
                for (i = mSubMeshList.begin(); i != iend; ++i)
                {
                    SubMesh* s = *i;
					if (s->operationType != RenderOperation::OT_TRIANGLE_FAN && 
						s->operationType != RenderOperation::OT_TRIANGLE_LIST && 
						s->operationType != RenderOperation::OT_TRIANGLE_STRIP)
					{
                        // create empty edge data and skip
						usage.edgeData = OGRE_NEW EdgeData();
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

            }
        }
        mEdgeListsBuilt = true;
    }
    //---------------------------------------------------------------------
    void Mesh::freeEdgeList(void)
    {
        if (!mEdgeListsBuilt)
            return;

        // Loop over LODs
        MeshLodUsageList::iterator i, iend;
        iend = mMeshLodUsageList.end();
        unsigned short index = 0;
        for (i = mMeshLodUsageList.begin(); i != iend; ++i, ++index)
        {
            MeshLodUsage& usage = *i;

            if (!mIsLodManual || index == 0)
            {
                // Only delete if we own this data
                // Manual LODs > 0 own their own
                OGRE_DELETE usage.edgeData;
            }
            usage.edgeData = NULL;
        }

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
        SubMeshList::iterator i, iend;
        iend = mSubMeshList.end();
        for (i = mSubMeshList.begin(); i != iend; ++i)
        {
            SubMesh* s = *i;
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
    EdgeData* Mesh::getEdgeList(unsigned int lodIndex)
    {
        // Build edge list on demand
        if (!mEdgeListsBuilt && mAutoBuildEdgeLists)
        {
            buildEdgeList();
        }

        return getLodLevel(lodIndex).edgeData;
    }
    //---------------------------------------------------------------------
    const EdgeData* Mesh::getEdgeList(unsigned int lodIndex) const
    {
        return getLodLevel(lodIndex).edgeData;
    }
    //---------------------------------------------------------------------
    void Mesh::prepareMatricesForVertexBlend(const Matrix4** blendMatrices,
        const Matrix4* boneMatrices, const IndexMap& indexMap)
    {
        assert(indexMap.size() <= 256);
        IndexMap::const_iterator it, itend;
        itend = indexMap.end();
        for (it = indexMap.begin(); it != itend; ++it)
        {
            *blendMatrices++ = boneMatrices + *it;
        }
    }
    //---------------------------------------------------------------------
    void Mesh::softwareVertexBlend(const VertexData* sourceVertexData,
        const VertexData* targetVertexData,
        const Matrix4* const* blendMatrices, size_t numMatrices,
        bool blendNormals)
    {
        float *pSrcPos = 0;
        float *pSrcNorm = 0;
        float *pDestPos = 0;
        float *pDestNorm = 0;
        float *pBlendWeight = 0;
        unsigned char* pBlendIdx = 0;
        size_t srcPosStride = 0;
        size_t srcNormStride = 0;
        size_t destPosStride = 0;
        size_t destNormStride = 0;
        size_t blendWeightStride = 0;
        size_t blendIdxStride = 0;


        // Get elements for source
        const VertexElement* srcElemPos =
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* srcElemNorm =
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* srcElemBlendIndices =
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        const VertexElement* srcElemBlendWeights =
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
        assert (srcElemPos && srcElemBlendIndices && srcElemBlendWeights &&
            "You must supply at least positions, blend indices and blend weights");
        // Get elements for target
        const VertexElement* destElemPos =
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* destElemNorm =
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);

        // Do we have normals and want to blend them?
        bool includeNormals = blendNormals && (srcElemNorm != NULL) && (destElemNorm != NULL);


        // Get buffers for source
        HardwareVertexBufferSharedPtr srcPosBuf, srcNormBuf, srcIdxBuf, srcWeightBuf;
        srcPosBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemPos->getSource());
        srcPosStride = srcPosBuf->getVertexSize();
        srcIdxBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemBlendIndices->getSource());
        blendIdxStride = srcIdxBuf->getVertexSize();
        srcWeightBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemBlendWeights->getSource());
        blendWeightStride = srcWeightBuf->getVertexSize();
        if (includeNormals)
        {
            srcNormBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemNorm->getSource());
            srcNormStride = srcNormBuf->getVertexSize();
        }
        // Get buffers for target
        HardwareVertexBufferSharedPtr destPosBuf, destNormBuf;
        destPosBuf = targetVertexData->vertexBufferBinding->getBuffer(destElemPos->getSource());
        destPosStride = destPosBuf->getVertexSize();
        if (includeNormals)
        {
            destNormBuf = targetVertexData->vertexBufferBinding->getBuffer(destElemNorm->getSource());
            destNormStride = destNormBuf->getVertexSize();
        }

        void* pBuffer;

        // Lock source buffers for reading
        pBuffer = srcPosBuf->lock(HardwareBuffer::HBL_READ_ONLY);
        srcElemPos->baseVertexPointerToElement(pBuffer, &pSrcPos);
        if (includeNormals)
        {
            if (srcNormBuf != srcPosBuf)
            {
                // Different buffer
                pBuffer = srcNormBuf->lock(HardwareBuffer::HBL_READ_ONLY);
            }
            srcElemNorm->baseVertexPointerToElement(pBuffer, &pSrcNorm);
        }

        // Indices must be 4 bytes
        assert(srcElemBlendIndices->getType() == VET_UBYTE4 &&
               "Blend indices must be VET_UBYTE4");
        pBuffer = srcIdxBuf->lock(HardwareBuffer::HBL_READ_ONLY);
        srcElemBlendIndices->baseVertexPointerToElement(pBuffer, &pBlendIdx);
        if (srcWeightBuf != srcIdxBuf)
        {
            // Lock buffer
            pBuffer = srcWeightBuf->lock(HardwareBuffer::HBL_READ_ONLY);
        }
        srcElemBlendWeights->baseVertexPointerToElement(pBuffer, &pBlendWeight);
        unsigned short numWeightsPerVertex =
            VertexElement::getTypeCount(srcElemBlendWeights->getType());


        // Lock destination buffers for writing
        pBuffer = destPosBuf->lock(
            (destNormBuf != destPosBuf && destPosBuf->getVertexSize() == destElemPos->getSize()) ||
            (destNormBuf == destPosBuf && destPosBuf->getVertexSize() == destElemPos->getSize() + destElemNorm->getSize()) ?
            HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        destElemPos->baseVertexPointerToElement(pBuffer, &pDestPos);
        if (includeNormals)
        {
            if (destNormBuf != destPosBuf)
            {
                pBuffer = destNormBuf->lock(
                    destNormBuf->getVertexSize() == destElemNorm->getSize() ?
                    HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
            }
            destElemNorm->baseVertexPointerToElement(pBuffer, &pDestNorm);
        }

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

        // Unlock source buffers
        srcPosBuf->unlock();
        srcIdxBuf->unlock();
        if (srcWeightBuf != srcIdxBuf)
        {
            srcWeightBuf->unlock();
        }
        if (includeNormals && srcNormBuf != srcPosBuf)
        {
            srcNormBuf->unlock();
        }
        // Unlock destination buffers
        destPosBuf->unlock();
        if (includeNormals && destNormBuf != destPosBuf)
        {
            destNormBuf->unlock();
        }

    }
	//---------------------------------------------------------------------
	void Mesh::softwareVertexMorph(Real t,
		const HardwareVertexBufferSharedPtr& b1,
		const HardwareVertexBufferSharedPtr& b2,
		VertexData* targetVertexData)
	{
		float* pb1 = static_cast<float*>(b1->lock(HardwareBuffer::HBL_READ_ONLY));
		float* pb2;
		if (b1.get() != b2.get())
		{
			pb2 = static_cast<float*>(b2->lock(HardwareBuffer::HBL_READ_ONLY));
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
		HardwareVertexBufferSharedPtr destBuf =
			targetVertexData->vertexBufferBinding->getBuffer(
				posElem->getSource());
		assert(posElem->getSize() == destBuf->getVertexSize() &&
			"Positions must be in a buffer on their own for morphing");
		float* pdst = static_cast<float*>(
			destBuf->lock(HardwareBuffer::HBL_DISCARD));

        OptimisedUtil::getImplementation()->softwareVertexMorph(
            t, pb1, pb2, pdst,
            targetVertexData->vertexCount);

		destBuf->unlock();
		b1->unlock();
		if (b1.get() != b2.get())
			b2->unlock();
	}
	//---------------------------------------------------------------------
	void Mesh::softwareVertexPoseBlend(Real weight,
		const map<size_t, Vector3>::type& vertexOffsetMap,
		VertexData* targetVertexData)
	{
		// Do nothing if no weight
		if (weight == 0.0f)
			return;

		const VertexElement* posElem =
			targetVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
		assert(posElem);
		HardwareVertexBufferSharedPtr destBuf =
			targetVertexData->vertexBufferBinding->getBuffer(
			posElem->getSource());
		assert(posElem->getSize() == destBuf->getVertexSize() &&
			"Positions must be in a buffer on their own for pose blending");

		// Have to lock in normal mode since this is incremental
		float* pBase = static_cast<float*>(
			destBuf->lock(HardwareBuffer::HBL_NORMAL));

		// Iterate over affected vertices
		for (map<size_t, Vector3>::type::const_iterator i = vertexOffsetMap.begin();
			i != vertexOffsetMap.end(); ++i)
		{
			// Adjust pointer
			float *pdst = pBase + i->first*3;

			*pdst = *pdst + (i->second.x * weight);
			++pdst;
			*pdst = *pdst + (i->second.y * weight);
			++pdst;
			*pdst = *pdst + (i->second.z * weight);
			++pdst;

		}

		destBuf->unlock();
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

		SubMeshList::const_iterator si;
		for (si = mSubMeshList.begin(); si != mSubMeshList.end(); ++si)
		{
			// Dedicated vertices
			if (!(*si)->useSharedVertices)
			{
				for (i = 0;
					i < (*si)->vertexData->vertexBufferBinding->getBufferCount();
					++i)
				{
					ret += (*si)->vertexData->vertexBufferBinding
						->getBuffer(i)->getSizeInBytes();
				}
			}
			if (!(*si)->indexData->indexBuffer.isNull())
			{
				// Index data
				ret += (*si)->indexData->indexBuffer->getSizeInBytes();
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
		for (SubMeshList::const_iterator i = mSubMeshList.begin();
			i != mSubMeshList.end(); ++i)
		{
			(*i)->mVertexAnimationType = VAT_NONE;
		}

		// Scan all animations and determine the type of animation tracks
		// relating to each vertex data
		for(AnimationList::const_iterator ai = mAnimationsList.begin();
			ai != mAnimationsList.end(); ++ai)
		{
			Animation* anim = ai->second;
			Animation::VertexTrackIterator vit = anim->getVertexTrackIterator();
			while (vit.hasMoreElements())
			{
				VertexAnimationTrack* track = vit.getNext();
				ushort handle = track->getHandle();
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
	bool Mesh::hasAnimation(const String& name)
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
		AnimationList::iterator i = mAnimationsList.begin();
		for (; i != mAnimationsList.end(); ++i)
		{
			OGRE_DELETE i->second;
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
	Pose* Mesh::getPose(ushort index)
	{
		if (index >= getPoseCount())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Index out of bounds",
				"Mesh::getPose");
		}

		return mPoseList[index];

	}
	//---------------------------------------------------------------------
	Pose* Mesh::getPose(const String& name)
	{
		for (PoseList::iterator i = mPoseList.begin(); i != mPoseList.end(); ++i)
		{
			if ((*i)->getName() == name)
				return *i;
		}
		StringUtil::StrStreamType str;
		str << "No pose called " << name << " found in Mesh " << mName;
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			str.str(),
			"Mesh::getPose");

	}
	//---------------------------------------------------------------------
	void Mesh::removePose(ushort index)
	{
		if (index >= getPoseCount())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Index out of bounds",
				"Mesh::removePose");
		}
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
		StringUtil::StrStreamType str;
		str << "No pose called " << name << " found in Mesh " << mName;
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			str.str(),
			"Mesh::removePose");
	}
	//---------------------------------------------------------------------
	void Mesh::removeAllPoses(void)
	{
		for (PoseList::iterator i = mPoseList.begin(); i != mPoseList.end(); ++i)
		{
			OGRE_DELETE *i;
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
	void Mesh::updateMaterialForAllSubMeshes(void)
	{
        // iterate through each sub mesh and request the submesh to update its material
        vector<SubMesh*>::type::iterator subi;
        for (subi = mSubMeshList.begin(); subi != mSubMeshList.end(); ++subi)
        {
            (*subi)->updateMaterialUsingTextureAliases();
        }

    }
	//---------------------------------------------------------------------
    const LodStrategy *Mesh::getLodStrategy() const
    {
        return mLodStrategy;
    }
    //---------------------------------------------------------------------
    void Mesh::setLodStrategy(LodStrategy *lodStrategy)
    {
        mLodStrategy = lodStrategy;

        assert(mMeshLodUsageList.size());
        mMeshLodUsageList[0].value = mLodStrategy->getBaseValue();

        // Re-transform user lod values (starting at index 1, no need to transform base value)
		for (MeshLodUsageList::iterator i = mMeshLodUsageList.begin(); i != mMeshLodUsageList.end(); ++i)
            i->value = mLodStrategy->transformUserValue(i->userValue);

    }
    //---------------------------------------------------------------------

}

