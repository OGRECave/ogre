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
#include "OgreSubEntity.h"

#include "OgreEntity.h"
#include "OgreSkeletonInstance.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh.h"
#include "OgreTagPoint.h"
#include "OgreLogManager.h"
#include "OgreMesh.h"
#include "OgreException.h"
#include "OgreCamera.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubEntity::SubEntity (Entity* parent, SubMesh* subMeshBasis)
        : Renderable(), mParentEntity(parent), //mMaterialName("BaseWhite"),
		mSubMesh(subMeshBasis), mCachedCamera(0)
    {
        //mMaterialPtr = MaterialManager::getSingleton().getByName(mMaterialName, subMeshBasis->parent->getGroup());
        mMaterialLodIndex = 0;
        mVisible = true;
        mRenderQueueID = 0;
        mRenderQueueIDSet = false;
        mRenderQueuePrioritySet = false;
        mSkelAnimVertexData = 0;
        mVertexAnimationAppliedThisFrame = false;
		mSoftwareVertexAnimVertexData = 0;
		mHardwareVertexAnimVertexData = 0;
		mHardwarePoseCount = 0;
		mIndexStart = 0;
        mIndexEnd = 0;
    }
    //-----------------------------------------------------------------------
    SubEntity::~SubEntity()
    {
        if (mSkelAnimVertexData)
            OGRE_DELETE mSkelAnimVertexData;
		if (mHardwareVertexAnimVertexData)
			OGRE_DELETE mHardwareVertexAnimVertexData;
		if (mSoftwareVertexAnimVertexData)
			OGRE_DELETE mSoftwareVertexAnimVertexData;
    }
    //-----------------------------------------------------------------------
    SubMesh* SubEntity::getSubMesh(void)
    {
        return mSubMesh;
    }
    //-----------------------------------------------------------------------
    const String& SubEntity::getMaterialName(void) const
    {
		return !mMaterialPtr.isNull() ? mMaterialPtr->getName() : StringUtil::BLANK;
        //return mMaterialName;
    }
    //-----------------------------------------------------------------------
    void SubEntity::setMaterialName( const String& name, const String& groupName /* = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME */)
    {


		MaterialPtr material = MaterialManager::getSingleton().getByName(name, groupName);

		if( material.isNull() )
		{
			LogManager::getSingleton().logMessage("Can't assign material " + name +
				" to SubEntity of " + mParentEntity->getName() + " because this "
				"Material does not exist. Have you forgotten to define it in a "
				".material script?", LML_CRITICAL);

			material = MaterialManager::getSingleton().getByName("BaseWhite");

			if (material.isNull())
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
					"to SubEntity of " + mParentEntity->getName() + ". Did "
					"you forget to call MaterialManager::initialise()?",
					"SubEntity::setMaterialName");
			}
		}

        setMaterial( material );
    }

	void SubEntity::setMaterial( const MaterialPtr& material )
	{
		mMaterialPtr = material;
		
        if (mMaterialPtr.isNull())
        {
			LogManager::getSingleton().logMessage("Can't assign material "  
                " to SubEntity of " + mParentEntity->getName() + " because this "
                "Material does not exist. Have you forgotten to define it in a "
                ".material script?", LML_CRITICAL);
			
			mMaterialPtr = MaterialManager::getSingleton().getByName("BaseWhite");
			
            if (mMaterialPtr.isNull())
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
                    "to SubEntity of " + mParentEntity->getName() + ". Did "
                    "you forget to call MaterialManager::initialise()?",
                    "SubEntity::setMaterial");
            }
        }
		
        // Ensure new material loaded (will not load again if already loaded)
        mMaterialPtr->load();

        // tell parent to reconsider material vertex processing options
        mParentEntity->reevaluateVertexProcessing();
	}

    //-----------------------------------------------------------------------
    const MaterialPtr& SubEntity::getMaterial(void) const
    {
        return mMaterialPtr;
    }
    //-----------------------------------------------------------------------
    Technique* SubEntity::getTechnique(void) const
    {
        return mMaterialPtr->getBestTechnique(mMaterialLodIndex, this);
    }
    //-----------------------------------------------------------------------
    void SubEntity::getRenderOperation(RenderOperation& op)
    {
		// Use LOD
        mSubMesh->_getRenderOperation(op, mParentEntity->mMeshLodIndex);
		// Deal with any vertex data overrides
		op.vertexData = getVertexDataForBinding();

		// If we use custom index position the client is responsible to set meaningful values 
		if(mIndexStart != mIndexEnd)
		{
			op.indexData->indexStart = mIndexStart;
			op.indexData->indexCount = mIndexEnd;
		}
    }
	//-----------------------------------------------------------------------
    void SubEntity::setIndexDataStartIndex(size_t start_index)
    {
		if(start_index < mSubMesh->indexData->indexCount)
	        mIndexStart = start_index;
    }
    //-----------------------------------------------------------------------
    size_t SubEntity::getIndexDataStartIndex() const
    {
        return mIndexStart;
    }
    //-----------------------------------------------------------------------
    void SubEntity::setIndexDataEndIndex(size_t end_index)
    {
		if(end_index > 0 && end_index <= mSubMesh->indexData->indexCount)
	        mIndexEnd = end_index;
    }
    //-----------------------------------------------------------------------
    size_t SubEntity::getIndexDataEndIndex() const
    {
        return mIndexEnd;
    }
    //-----------------------------------------------------------------------
	void SubEntity::resetIndexDataStartEndIndex()
	{
		mIndexStart = 0;
		mIndexEnd = 0;
	}
	//-----------------------------------------------------------------------
	VertexData* SubEntity::getVertexDataForBinding(void)
	{
		if (mSubMesh->useSharedVertices)
		{
			return mParentEntity->getVertexDataForBinding();
		}
		else
		{
			Entity::VertexDataBindChoice c = 
				mParentEntity->chooseVertexDataForBinding(
					mSubMesh->getVertexAnimationType() != VAT_NONE);
			switch(c)
			{
			case Entity::BIND_ORIGINAL:
				return mSubMesh->vertexData;
			case Entity::BIND_HARDWARE_MORPH:
				return mHardwareVertexAnimVertexData;
			case Entity::BIND_SOFTWARE_MORPH:
				return mSoftwareVertexAnimVertexData;
			case Entity::BIND_SOFTWARE_SKELETAL:
				return mSkelAnimVertexData;
			};
			// keep compiler happy
			return mSubMesh->vertexData;

		}
	}
    //-----------------------------------------------------------------------
    void SubEntity::getWorldTransforms(Matrix4* xform) const
    {
        if (!mParentEntity->mNumBoneMatrices ||
            !mParentEntity->isHardwareAnimationEnabled())
        {
            // No skeletal animation, or software skinning
            *xform = mParentEntity->_getParentNodeFullTransform();
        }
        else
        {
            // Hardware skinning, pass all actually used matrices
            const Mesh::IndexMap& indexMap = mSubMesh->useSharedVertices ?
                mSubMesh->parent->sharedBlendIndexToBoneIndexMap : mSubMesh->blendIndexToBoneIndexMap;
            assert(indexMap.size() <= mParentEntity->mNumBoneMatrices);

            if (mParentEntity->_isSkeletonAnimated())
            {
                // Bones, use cached matrices built when Entity::_updateRenderQueue was called
                assert(mParentEntity->mBoneWorldMatrices);

                Mesh::IndexMap::const_iterator it, itend;
                itend = indexMap.end();
                for (it = indexMap.begin(); it != itend; ++it, ++xform)
                {
                    *xform = mParentEntity->mBoneWorldMatrices[*it];
                }
            }
            else
            {
                // All animations disabled, use parent entity world transform only
                std::fill_n(xform, indexMap.size(), mParentEntity->_getParentNodeFullTransform());
            }
        }
    }
    //-----------------------------------------------------------------------
    unsigned short SubEntity::getNumWorldTransforms(void) const
    {
        if (!mParentEntity->mNumBoneMatrices ||
            !mParentEntity->isHardwareAnimationEnabled())
        {
            // No skeletal animation, or software skinning
            return 1;
        }
        else
        {
            // Hardware skinning, pass all actually used matrices
            const Mesh::IndexMap& indexMap = mSubMesh->useSharedVertices ?
                mSubMesh->parent->sharedBlendIndexToBoneIndexMap : mSubMesh->blendIndexToBoneIndexMap;
            assert(indexMap.size() <= mParentEntity->mNumBoneMatrices);

            return static_cast<unsigned short>(indexMap.size());
        }
    }
    //-----------------------------------------------------------------------
    Real SubEntity::getSquaredViewDepth(const Camera* cam) const
    {
        // First of all, check the cached value
		// NB this is manually invalidated by parent each _notifyCurrentCamera call
		// Done this here rather than there since we only need this for transparent objects
        if (mCachedCamera == cam)
            return mCachedCameraDist;

        Node* n = mParentEntity->getParentNode();
        assert(n);
        Real dist;
        if (!mSubMesh->extremityPoints.empty())
        {
            const Vector3 &cp = cam->getDerivedPosition();
            const Matrix4 &l2w = mParentEntity->_getParentNodeFullTransform();
			dist = std::numeric_limits<Real>::infinity();
            for (vector<Vector3>::type::const_iterator i = mSubMesh->extremityPoints.begin();
                 i != mSubMesh->extremityPoints.end (); ++i)
            {
                Vector3 v = l2w * (*i);
                Real d = (v - cp).squaredLength();
                
				dist = std::min(d, dist);
            }
        }
        else
            dist = n->getSquaredViewDepth(cam);

        mCachedCameraDist = dist;
        mCachedCamera = cam;

        return dist;
    }
    //-----------------------------------------------------------------------
    const LightList& SubEntity::getLights(void) const
    {
        return mParentEntity->queryLights();
    }
    //-----------------------------------------------------------------------
    void SubEntity::setVisible(bool visible)
    {
        mVisible = visible;
    }
    //-----------------------------------------------------------------------
    bool SubEntity::isVisible(void) const
    {
        return mVisible;

    }
    //-----------------------------------------------------------------------
    void SubEntity::prepareTempBlendBuffers(void)
    {
		if (mSubMesh->useSharedVertices)
			return;

        if (mSkelAnimVertexData) 
        {
            OGRE_DELETE mSkelAnimVertexData;
            mSkelAnimVertexData = 0;
        }
		if (mSoftwareVertexAnimVertexData) 
		{
			OGRE_DELETE mSoftwareVertexAnimVertexData;
			mSoftwareVertexAnimVertexData = 0;
		}
		if (mHardwareVertexAnimVertexData) 
		{
			OGRE_DELETE mHardwareVertexAnimVertexData;
			mHardwareVertexAnimVertexData = 0;
		}

		if (!mSubMesh->useSharedVertices)
		{
			if (mSubMesh->getVertexAnimationType() != VAT_NONE)
			{
				// Create temporary vertex blend info
				// Prepare temp vertex data if needed
				// Clone without copying data, don't remove any blending info
				// (since if we skeletally animate too, we need it)
				mSoftwareVertexAnimVertexData = mSubMesh->vertexData->clone(false);
				mParentEntity->extractTempBufferInfo(mSoftwareVertexAnimVertexData, &mTempVertexAnimInfo);

				// Also clone for hardware usage, don't remove blend info since we'll
				// need it if we also hardware skeletally animate
				mHardwareVertexAnimVertexData = mSubMesh->vertexData->clone(false);
			}

			if (mParentEntity->hasSkeleton())
			{
				// Create temporary vertex blend info
				// Prepare temp vertex data if needed
				// Clone without copying data, remove blending info
				// (since blend is performed in software)
				mSkelAnimVertexData = 
					mParentEntity->cloneVertexDataRemoveBlendInfo(mSubMesh->vertexData);
				mParentEntity->extractTempBufferInfo(mSkelAnimVertexData, &mTempSkelAnimInfo);

			}
		}
    }
    //-----------------------------------------------------------------------
    bool SubEntity::getCastsShadows(void) const
    {
        return mParentEntity->getCastShadows();
    }
	//-----------------------------------------------------------------------
	VertexData* SubEntity::_getSkelAnimVertexData(void) 
	{
		assert (mSkelAnimVertexData && "Not software skinned or has no dedicated geometry!");
		return mSkelAnimVertexData;
	}
	//-----------------------------------------------------------------------
	VertexData* SubEntity::_getSoftwareVertexAnimVertexData(void)
	{
		assert (mSoftwareVertexAnimVertexData && "Not vertex animated or has no dedicated geometry!");
		return mSoftwareVertexAnimVertexData;
	}
	//-----------------------------------------------------------------------
	VertexData* SubEntity::_getHardwareVertexAnimVertexData(void)
	{
		assert (mHardwareVertexAnimVertexData && "Not vertex animated or has no dedicated geometry!");
		return mHardwareVertexAnimVertexData;
	}
	//-----------------------------------------------------------------------
	TempBlendedBufferInfo* SubEntity::_getSkelAnimTempBufferInfo(void) 
	{
		return &mTempSkelAnimInfo;
	}
	//-----------------------------------------------------------------------
	TempBlendedBufferInfo* SubEntity::_getVertexAnimTempBufferInfo(void) 
	{
		return &mTempVertexAnimInfo;
	}
	//-----------------------------------------------------------------------
	void SubEntity::_updateCustomGpuParameter(
		const GpuProgramParameters::AutoConstantEntry& constantEntry,
		GpuProgramParameters* params) const
	{
		if (constantEntry.paramType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
		{
			// Set up to 4 values, or up to limit of hardware animation entries
			// Pack into 4-element constants offset based on constant data index
			// If there are more than 4 entries, this will be called more than once
			Vector4 val(0.0f,0.0f,0.0f,0.0f);
			const VertexData* vd = mHardwareVertexAnimVertexData ? mHardwareVertexAnimVertexData : mParentEntity->mHardwareVertexAnimVertexData;
			
			size_t animIndex = constantEntry.data * 4;
			for (size_t i = 0; i < 4 && 
				animIndex < vd->hwAnimationDataList.size();
				++i, ++animIndex)
			{
				val[i] = 
					vd->hwAnimationDataList[animIndex].parametric;
			}
			// set the parametric morph value
			params->_writeRawConstant(constantEntry.physicalIndex, val);
		}
		else
		{
			// default
			return Renderable::_updateCustomGpuParameter(constantEntry, params);
		}
	}
	//-----------------------------------------------------------------------------
	void SubEntity::_markBuffersUnusedForAnimation(void)
	{
		mVertexAnimationAppliedThisFrame = false;
	}
	//-----------------------------------------------------------------------------
	void SubEntity::_markBuffersUsedForAnimation(void)
	{
		mVertexAnimationAppliedThisFrame = true;
	}
	//-----------------------------------------------------------------------------
	void SubEntity::_restoreBuffersForUnusedAnimation(bool hardwareAnimation)
	{
		// Rebind original positions if:
		//  We didn't apply any animation and 
		//    We're morph animated (hardware binds keyframe, software is missing)
		//    or we're pose animated and software (hardware is fine, still bound)
		if (mSubMesh->getVertexAnimationType() != VAT_NONE && 
			!mSubMesh->useSharedVertices && 
			!mVertexAnimationAppliedThisFrame &&
			(!hardwareAnimation || mSubMesh->getVertexAnimationType() == VAT_MORPH))
		{
			// Note, VES_POSITION is specified here but if normals are included in animation
			// then these will be re-bound too (buffers must be shared)
			const VertexElement* srcPosElem = 
				mSubMesh->vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
			HardwareVertexBufferSharedPtr srcBuf = 
				mSubMesh->vertexData->vertexBufferBinding->getBuffer(
				srcPosElem->getSource());

			// Bind to software
			const VertexElement* destPosElem = 
				mSoftwareVertexAnimVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
			mSoftwareVertexAnimVertexData->vertexBufferBinding->setBinding(
				destPosElem->getSource(), srcBuf);
			
		}

		// rebind any missing hardware pose buffers
		// Caused by not having any animations enabled, or keyframes which reference
		// no poses
		if (!mSubMesh->useSharedVertices && hardwareAnimation 
			&& mSubMesh->getVertexAnimationType() == VAT_POSE)
		{
			mParentEntity->bindMissingHardwarePoseBuffers(
				mSubMesh->vertexData, mHardwareVertexAnimVertexData);
		}

	}

    void SubEntity::setRenderQueueGroup(uint8 queueID)
    {
        mRenderQueueIDSet = true;
        mRenderQueueID = queueID;
    }

    void SubEntity::setRenderQueueGroupAndPriority(uint8 queueID, ushort priority)
    {
        setRenderQueueGroup(queueID);
        mRenderQueuePrioritySet = true;
        mRenderQueuePriority = priority;
    }

    uint8 SubEntity::getRenderQueueGroup(void) const
    {
        return mRenderQueueID;
    }

    ushort SubEntity::getRenderQueuePriority(void) const
    {
        return mRenderQueuePriority;
    }

    bool SubEntity::isRenderQueueGroupSet(void) const
    {
        return mRenderQueueIDSet;
    }

    bool SubEntity::isRenderQueuePrioritySet(void) const
    {
        return mRenderQueuePrioritySet;
    }

}
