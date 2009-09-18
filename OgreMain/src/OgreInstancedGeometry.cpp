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
#include "OgreInstancedGeometry.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreSceneNode.h"
#include "OgreException.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreLogManager.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreMaterialManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreEdgeListBuilder.h"
#include "OgreStringConverter.h"

namespace Ogre {

	#define BatchInstance_RANGE 1024
	#define BatchInstance_HALF_RANGE 512
	#define BatchInstance_MAX_INDEX 511
	#define BatchInstance_MIN_INDEX -512

	//--------------------------------------------------------------------------
	InstancedGeometry::InstancedGeometry(SceneManager* owner, const String& name):
		mOwner(owner),
		mName(name),
		mBuilt(false),
		mUpperDistance(0.0f),
		mSquaredUpperDistance(0.0f),
		mCastShadows(false),
		mBatchInstanceDimensions(Vector3(1000,1000,1000)),
		mHalfBatchInstanceDimensions(Vector3(500,500,500)),
		mOrigin(Vector3(0,0,0)),
		mVisible(true),
        mRenderQueueID(RENDER_QUEUE_MAIN),
        mRenderQueueIDSet(false),
		mObjectCount(0),
		mInstancedGeometryInstance(0),
		mSkeletonInstance(0)
	{
		mBaseSkeleton.setNull();
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::~InstancedGeometry()
	{
		reset();
		if(mSkeletonInstance)
			OGRE_DELETE mSkeletonInstance;

			
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance*InstancedGeometry::getInstancedGeometryInstance(void)
	{
		if (!mInstancedGeometryInstance)
		{
			uint32 index = 0;
			// Make a name
			StringUtil::StrStreamType str;
			str << mName << ":" << index;

			mInstancedGeometryInstance = OGRE_NEW BatchInstance(this, str.str(), mOwner, index);
			mOwner->injectMovableObject(mInstancedGeometryInstance);
			mInstancedGeometryInstance->setVisible(mVisible);
			mInstancedGeometryInstance->setCastShadows(mCastShadows);
			if (mRenderQueueIDSet)
			{
				mInstancedGeometryInstance->setRenderQueueGroup(mRenderQueueID);
			}
			mBatchInstanceMap[index] = mInstancedGeometryInstance;
		
	
		}
		return mInstancedGeometryInstance;
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance* InstancedGeometry::getBatchInstance(const AxisAlignedBox& bounds,
		bool autoCreate)
	{
		if (bounds.isNull())
			return 0;

		// Get the BatchInstance which has the largest overlapping volume
		const Vector3 min = bounds.getMinimum();
		const Vector3 max = bounds.getMaximum();

		// Get the min and max BatchInstance indexes
		ushort minx, miny, minz;
		ushort maxx, maxy, maxz;
		getBatchInstanceIndexes(min, minx, miny, minz);
		getBatchInstanceIndexes(max, maxx, maxy, maxz);
		Real maxVolume = 0.0f;
		ushort finalx =0 , finaly = 0, finalz = 0;
		for (ushort x = minx; x <= maxx; ++x)
		{
			for (ushort y = miny; y <= maxy; ++y)
			{
				for (ushort z = minz; z <= maxz; ++z)
				{
					Real vol = getVolumeIntersection(bounds, x, y, z);
					if (vol > maxVolume)
					{
						maxVolume = vol;
						finalx = x;
						finaly = y;
						finalz = z;
					}

				}
			}
		}

		assert(maxVolume > 0.0f &&
			"Static geometry: Problem determining closest volume match!");

		return getBatchInstance(finalx, finaly, finalz, autoCreate);

	}
	//--------------------------------------------------------------------------
	Real InstancedGeometry::getVolumeIntersection(const AxisAlignedBox& box,
		ushort x, ushort y, ushort z)
	{
		// Get bounds of indexed BatchInstance
		AxisAlignedBox BatchInstanceBounds = getBatchInstanceBounds(x, y, z);
		AxisAlignedBox intersectBox = BatchInstanceBounds.intersection(box);
		// return a 'volume' which ignores zero dimensions
		// since we only use this for relative comparisons of the same bounds
		// this will still be internally consistent
		Vector3 boxdiff = box.getMaximum() - box.getMinimum();
		Vector3 intersectDiff = intersectBox.getMaximum() - intersectBox.getMinimum();

		return (boxdiff.x == 0 ? 1 : intersectDiff.x) *
			(boxdiff.y == 0 ? 1 : intersectDiff.y) *
			(boxdiff.z == 0 ? 1 : intersectDiff.z);

	}
	//--------------------------------------------------------------------------
	AxisAlignedBox InstancedGeometry::getBatchInstanceBounds(ushort x, ushort y, ushort z)
	{
		Vector3 min(
			((Real)x - BatchInstance_HALF_RANGE) * mBatchInstanceDimensions.x + mOrigin.x,
			((Real)y - BatchInstance_HALF_RANGE) * mBatchInstanceDimensions.y + mOrigin.y,
			((Real)z - BatchInstance_HALF_RANGE) * mBatchInstanceDimensions.z + mOrigin.z
			);
		Vector3 max = min + mBatchInstanceDimensions;
		return AxisAlignedBox(min, max);
	}
	//--------------------------------------------------------------------------
 	Vector3 InstancedGeometry::getBatchInstanceCentre(ushort x, ushort y, ushort z)
	{
		return Vector3(
			((Real)x - BatchInstance_HALF_RANGE) * mBatchInstanceDimensions.x + mOrigin.x
				+ mHalfBatchInstanceDimensions.x,
			((Real)y - BatchInstance_HALF_RANGE) * mBatchInstanceDimensions.y + mOrigin.y
				+ mHalfBatchInstanceDimensions.y,
			((Real)z - BatchInstance_HALF_RANGE) * mBatchInstanceDimensions.z + mOrigin.z
				+ mHalfBatchInstanceDimensions.z
			);
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance* InstancedGeometry::getBatchInstance(
			ushort x, ushort y, ushort z, bool autoCreate)
	{
		uint32 index = packIndex(x, y, z);
		BatchInstance* ret = getBatchInstance(index);
		if (!ret && autoCreate)
		{
			// Make a name
			StringUtil::StrStreamType str;
			str << mName << ":" << index;
			// Calculate the BatchInstance centre
			Vector3 centre(0,0,0);// = getBatchInstanceCentre(x, y, z);
			ret = OGRE_NEW BatchInstance(this, str.str(), mOwner, index/*, centre*/);
			mOwner->injectMovableObject(ret);
			ret->setVisible(mVisible);
			ret->setCastShadows(mCastShadows);
			if (mRenderQueueIDSet)
			{
				ret->setRenderQueueGroup(mRenderQueueID);
			}
			mBatchInstanceMap[index] = ret;
		}
		return ret;
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance* InstancedGeometry::getBatchInstance(uint32 index)
	{
		BatchInstanceMap::iterator i = mBatchInstanceMap.find(index);
		if (i != mBatchInstanceMap.end())
		{
			return i->second;
		}
		else
		{
			return 0;
		}

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::getBatchInstanceIndexes(const Vector3& point,
		ushort& x, ushort& y, ushort& z)
	{
		// Scale the point into multiples of BatchInstance and adjust for origin
		Vector3 scaledPoint = (point - mOrigin) / mBatchInstanceDimensions;

		// Round down to 'bottom left' point which represents the cell index
		int ix = Math::IFloor(scaledPoint.x);
		int iy = Math::IFloor(scaledPoint.y);
		int iz = Math::IFloor(scaledPoint.z);

		// Check bounds
		if (ix < BatchInstance_MIN_INDEX || ix > BatchInstance_MAX_INDEX
			|| iy < BatchInstance_MIN_INDEX || iy > BatchInstance_MAX_INDEX
			|| iz < BatchInstance_MIN_INDEX || iz > BatchInstance_MAX_INDEX)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Point out of bounds",
				"InstancedGeometry::getBatchInstanceIndexes");
		}
		// Adjust for the fact that we use unsigned values for simplicity
		// (requires less faffing about for negatives give 10-bit packing
		x = static_cast<ushort>(ix + BatchInstance_HALF_RANGE);
		y = static_cast<ushort>(iy + BatchInstance_HALF_RANGE);
		z = static_cast<ushort>(iz + BatchInstance_HALF_RANGE);


	}
	//--------------------------------------------------------------------------
	uint32 InstancedGeometry::packIndex(ushort x, ushort y, ushort z)
	{
		return x + (y << 10) + (z << 20);
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance* InstancedGeometry::getBatchInstance(const Vector3& point,
		bool autoCreate)
	{
		ushort x, y, z;
		getBatchInstanceIndexes(point, x, y, z);
		return getBatchInstance(x, y, z, autoCreate);
	}
	//--------------------------------------------------------------------------
	AxisAlignedBox InstancedGeometry::calculateBounds(VertexData* vertexData,
		const Vector3& position, const Quaternion& orientation,
		const Vector3& scale)
	{
		const VertexElement* posElem =
			vertexData->vertexDeclaration->findElementBySemantic(
				VES_POSITION);
		HardwareVertexBufferSharedPtr vbuf =
			vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* vertex =
			static_cast<unsigned char*>(
				vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
		float* pFloat;

		Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE;
		bool first = true;

		for(size_t j = 0; j < vertexData->vertexCount; ++j, vertex += vbuf->getVertexSize())
		{
			posElem->baseVertexPointerToElement(vertex, &pFloat);

			Vector3 pt;

			pt.x = (*pFloat++);
			pt.y = (*pFloat++);
			pt.z = (*pFloat++);
			// Transform to world (scale, rotate, translate)
			pt = (orientation * (pt * scale)) + position;
			if (first)
			{
				min = max = pt;
				first = false;
			}
			else
			{
				min.makeFloor(pt);
				max.makeCeil(pt);
			}

		}
		vbuf->unlock();
		return AxisAlignedBox(min, max);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::addEntity(Entity* ent, const Vector3& position,
		const Quaternion& orientation, const Vector3& scale)
	{

		const MeshPtr& msh = ent->getMesh();
		// Validate
		if (msh->isLodManual())
		{
			LogManager::getSingleton().logMessage(
				"WARNING (InstancedGeometry): Manual LOD is not supported. "
				"Using only highest LOD level for mesh " + msh->getName());
		}

		//get the skeleton of the entity, if that's not already done
		if(!ent->getMesh()->getSkeleton().isNull()&&mBaseSkeleton.isNull())
		{
			mBaseSkeleton=ent->getMesh()->getSkeleton();
			mSkeletonInstance= OGRE_NEW SkeletonInstance(mBaseSkeleton);
			mSkeletonInstance->load();
			mAnimationState=ent->getAllAnimationStates();
		}
		AxisAlignedBox sharedWorldBounds;
		// queue this entities submeshes and choice of material
		// also build the lists of geometry to be used for the source of lods


		for (uint i = 0; i < ent->getNumSubEntities(); ++i)
		{
			SubEntity* se = ent->getSubEntity(i);
			QueuedSubMesh* q = OGRE_NEW QueuedSubMesh();

			// Get the geometry for this SubMesh
			q->submesh = se->getSubMesh();
			q->geometryLodList = determineGeometry(q->submesh);
			q->materialName = se->getMaterialName();
			q->orientation = orientation;
			q->position = position;
			q->scale = scale;
			q->ID = mObjectCount;
			// Determine the bounds based on the highest LOD
			q->worldBounds = calculateBounds(
				(*q->geometryLodList)[0].vertexData,
					position, orientation, scale);

			mQueuedSubMeshes.push_back(q);
		}
		mObjectCount++;

	}
	//--------------------------------------------------------------------------
	InstancedGeometry::SubMeshLodGeometryLinkList*
	InstancedGeometry::determineGeometry(SubMesh* sm)
	{
		// First, determine if we've already seen this submesh before
		SubMeshGeometryLookup::iterator i =
			mSubMeshGeometryLookup.find(sm);
		if (i != mSubMeshGeometryLookup.end())
		{
			return i->second;
		}
		// Otherwise, we have to create a new one
		SubMeshLodGeometryLinkList* lodList = OGRE_NEW_T(SubMeshLodGeometryLinkList, MEMCATEGORY_GEOMETRY)();
		mSubMeshGeometryLookup[sm] = lodList;
		ushort numLods = sm->parent->isLodManual() ? 1 :
			sm->parent->getNumLodLevels();
		lodList->resize(numLods);
		for (ushort lod = 0; lod < numLods; ++lod)
		{
			SubMeshLodGeometryLink& geomLink = (*lodList)[lod];
			IndexData *lodIndexData;
			if (lod == 0)
			{
				lodIndexData = sm->indexData;
			}
			else
			{
				lodIndexData = sm->mLodFaceList[lod - 1];
			}
			// Can use the original mesh geometry?
			if (sm->useSharedVertices)
			{
				if (sm->parent->getNumSubMeshes() == 1)
				{
					// Ok, this is actually our own anyway
					geomLink.vertexData = sm->parent->sharedVertexData;
					geomLink.indexData = lodIndexData;
				}
				else
				{
					// We have to split it
					splitGeometry(sm->parent->sharedVertexData,
						lodIndexData, &geomLink);
				}
			}
			else
			{
				if (lod == 0)
				{
					// Ok, we can use the existing geometry; should be in full
					// use by just this SubMesh
					geomLink.vertexData = sm->vertexData;
					geomLink.indexData = sm->indexData;
				}
				else
				{
					// We have to split it
					splitGeometry(sm->vertexData,
						lodIndexData, &geomLink);
				}
			}
			assert (geomLink.vertexData->vertexStart == 0 &&
				"Cannot use vertexStart > 0 on indexed geometry due to "
				"rendersystem incompatibilities - see the docs!");
		}


		return lodList;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::splitGeometry(VertexData* vd, IndexData* id,
			InstancedGeometry::SubMeshLodGeometryLink* targetGeomLink)
	{
		// Firstly we need to scan to see how many vertices are being used
		// and while we're at it, build the remap we can use later
		bool use32bitIndexes =
			id->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT;
		uint16 *p16;
		uint32 *p32;
		IndexRemap indexRemap;
		if (use32bitIndexes)
		{
			p32 = static_cast<uint32*>(id->indexBuffer->lock(
				id->indexStart, 
				id->indexCount * id->indexBuffer->getIndexSize(), 
				HardwareBuffer::HBL_READ_ONLY));
			buildIndexRemap(p32, id->indexCount, indexRemap);
			id->indexBuffer->unlock();
		}
		else
		{
			p16 = static_cast<uint16*>(id->indexBuffer->lock(
				id->indexStart, 
				id->indexCount * id->indexBuffer->getIndexSize(), 
				HardwareBuffer::HBL_READ_ONLY));
			buildIndexRemap(p16, id->indexCount, indexRemap);
			id->indexBuffer->unlock();
		}
		if (indexRemap.size() == vd->vertexCount)
		{
			// ha, complete usage after all
			targetGeomLink->vertexData = vd;
			targetGeomLink->indexData = id;
			return;
		}


		// Create the new vertex data records
		targetGeomLink->vertexData = vd->clone(false);
		// Convenience
		VertexData* newvd = targetGeomLink->vertexData;
		//IndexData* newid = targetGeomLink->indexData;
		// Update the vertex count
		newvd->vertexCount = indexRemap.size();

		size_t numvbufs = vd->vertexBufferBinding->getBufferCount();
		// Copy buffers from old to new
		for (unsigned short b = 0; b < numvbufs; ++b)
		{
			// Lock old buffer
			HardwareVertexBufferSharedPtr oldBuf =
				vd->vertexBufferBinding->getBuffer(b);
			// Create new buffer
			HardwareVertexBufferSharedPtr newBuf =
				HardwareBufferManager::getSingleton().createVertexBuffer(
					oldBuf->getVertexSize(),
					indexRemap.size(),
					HardwareBuffer::HBU_STATIC);
			// rebind
			newvd->vertexBufferBinding->setBinding(b, newBuf);

			// Copy all the elements of the buffer across, by iterating over
			// the IndexRemap which describes how to move the old vertices
			// to the new ones. By nature of the map the remap is in order of
			// indexes in the old buffer, but note that we're not guaranteed to
			// address every vertex (which is kinda why we're here)
			uchar* pSrcBase = static_cast<uchar*>(
				oldBuf->lock(HardwareBuffer::HBL_READ_ONLY));
			uchar* pDstBase = static_cast<uchar*>(
				newBuf->lock(HardwareBuffer::HBL_DISCARD));
			size_t vertexSize = oldBuf->getVertexSize();
			// Buffers should be the same size
			assert (vertexSize == newBuf->getVertexSize());

			for (IndexRemap::iterator r = indexRemap.begin();
				r != indexRemap.end(); ++r)
			{
				assert (r->first < oldBuf->getNumVertices());
				assert (r->second < newBuf->getNumVertices());

				uchar* pSrc = pSrcBase + r->first * vertexSize;
				uchar* pDst = pDstBase + r->second * vertexSize;
				memcpy(pDst, pSrc, vertexSize);
			}
			// unlock
			oldBuf->unlock();
			newBuf->unlock();

		}

		// Now create a new index buffer
		HardwareIndexBufferSharedPtr ibuf =
			HardwareBufferManager::getSingleton().createIndexBuffer(
				id->indexBuffer->getType(), id->indexCount,
				HardwareBuffer::HBU_STATIC);

		if (use32bitIndexes)
		{
			uint32 *pSrc32, *pDst32;
			pSrc32 = static_cast<uint32*>(id->indexBuffer->lock(
				id->indexStart, id->indexCount * id->indexBuffer->getIndexSize(), 
				HardwareBuffer::HBL_READ_ONLY));
			pDst32 = static_cast<uint32*>(ibuf->lock(
				HardwareBuffer::HBL_DISCARD));
			remapIndexes(pSrc32, pDst32, indexRemap, id->indexCount);
			id->indexBuffer->unlock();
			ibuf->unlock();
		}
		else
		{
			uint16 *pSrc16, *pDst16;
			pSrc16 = static_cast<uint16*>(id->indexBuffer->lock(
				id->indexStart, id->indexCount * id->indexBuffer->getIndexSize(), 
				HardwareBuffer::HBL_READ_ONLY));
			pDst16 = static_cast<uint16*>(ibuf->lock(
				HardwareBuffer::HBL_DISCARD));
			remapIndexes(pSrc16, pDst16, indexRemap, id->indexCount);
			id->indexBuffer->unlock();
			ibuf->unlock();
		}

		targetGeomLink->indexData = OGRE_NEW IndexData();
		targetGeomLink->indexData->indexStart = 0;
		targetGeomLink->indexData->indexCount = id->indexCount;
		targetGeomLink->indexData->indexBuffer = ibuf;

		// Store optimised geometry for deallocation later
		OptimisedSubMeshGeometry *optGeom = OGRE_NEW OptimisedSubMeshGeometry();
		optGeom->indexData = targetGeomLink->indexData;
		optGeom->vertexData = targetGeomLink->vertexData;
		mOptimisedSubMeshGeometryList.push_back(optGeom);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::addSceneNode(const SceneNode* node)
	{
		SceneNode::ConstObjectIterator obji = node->getAttachedObjectIterator();
		while (obji.hasMoreElements())
		{
			MovableObject* mobj = obji.getNext();
			if (mobj->getMovableType() == "Entity")
			{
				addEntity(static_cast<Entity*>(mobj),
					node->_getDerivedPosition(),
					node->_getDerivedOrientation(),
					node->_getDerivedScale());
			}
		}
		// Iterate through all the child-nodes
		SceneNode::ConstChildNodeIterator nodei = node->getChildIterator();

		while (nodei.hasMoreElements())
		{
			const SceneNode* newNode = static_cast<const SceneNode*>(nodei.getNext());
			// Add this subnode and its children...
			addSceneNode( newNode );
		}
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::build(void)
	{
		// Make sure there's nothing from previous builds
		destroy();

		// Firstly allocate meshes to BatchInstances
		for (QueuedSubMeshList::iterator qi = mQueuedSubMeshes.begin();
			qi != mQueuedSubMeshes.end(); ++qi)
		{
			QueuedSubMesh* qsm = *qi;
			//BatchInstance* BatchInstance = getBatchInstance(qsm->worldBounds, true);
			BatchInstance* batchInstance = getInstancedGeometryInstance();
			batchInstance->assign(qsm);
		}

		// Now tell each BatchInstance to build itself
		for (BatchInstanceMap::iterator ri = mBatchInstanceMap.begin();
			ri != mBatchInstanceMap.end(); ++ri)
		{
			ri->second->build();
		}


	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::addBatchInstance(void)
	{
		

		BatchInstanceIterator regIt = getBatchInstanceIterator();
		BatchInstance*lastBatchInstance=0 ;
		while(regIt.hasMoreElements())
		{
			lastBatchInstance= regIt.getNext();
		}
		uint32 index=(lastBatchInstance)?lastBatchInstance->getID()+1:0;
		//create a new BatchInstance

		BatchInstance*ret = OGRE_NEW BatchInstance(this, mName+":"+StringConverter::toString(index),
			mOwner, index);

		ret->attachToScene();

		mOwner->injectMovableObject(ret);
		ret->setVisible(mVisible);
		ret->setCastShadows(mCastShadows);
		mBatchInstanceMap[index] = ret;

		if (mRenderQueueIDSet)
		{
				ret->setRenderQueueGroup(mRenderQueueID);
		}
	
		const size_t numLod = lastBatchInstance->mLodValues.size();
		ret->mLodValues.resize(numLod);
		for (ushort lod = 0; lod < numLod; lod++)
		{
			ret->mLodValues[lod] =
			lastBatchInstance->mLodValues[lod];
		}


	
		// update bounds
		AxisAlignedBox box(lastBatchInstance->mAABB.getMinimum(),lastBatchInstance->mAABB.getMaximum());
		ret->mAABB.merge(box);

		ret->mBoundingRadius = lastBatchInstance->mBoundingRadius ;
		//now create  news instanced objects
		BatchInstance::ObjectsMap::iterator objIt;
		for(objIt=lastBatchInstance->getInstancesMap().begin();objIt!=lastBatchInstance->getInstancesMap().end();objIt++)
		{
			InstancedObject* instancedObject = ret->isInstancedObjectPresent(objIt->first);
			if(instancedObject == NULL)
			{
				if(mBaseSkeleton.isNull())
				{
					instancedObject= OGRE_NEW InstancedObject(objIt->first);
				}
				else
				{
					instancedObject= OGRE_NEW InstancedObject(objIt->first,mSkeletonInstance,mAnimationState);
				}
				ret->addInstancedObject(objIt->first,instancedObject);
			}

		}



		BatchInstance::LODIterator lodIterator = lastBatchInstance->getLODIterator();
		//parse all the lod buckets of the BatchInstance
		while (lodIterator.hasMoreElements())
		{
		
			LODBucket* lod = lodIterator.getNext();
			//create a new lod bucket for the new BatchInstance
			LODBucket* lodBucket= OGRE_NEW LODBucket(ret, lod->getLod(), lod->getLodValue());

			//add the lod bucket to the BatchInstance list
			ret->updateContainers(lodBucket);
			
			LODBucket::MaterialIterator matIt = lod->getMaterialIterator();
			//parse all the material buckets of the lod bucket
			while (matIt.hasMoreElements())
			{
				
				MaterialBucket*mat = matIt.getNext();
				//create a new material bucket
				String materialName=mat->getMaterialName();
				MaterialBucket* matBucket = OGRE_NEW MaterialBucket(lodBucket,materialName);

				//add the material bucket to the lod buckets list and map
				lodBucket->updateContainers(matBucket, materialName);

				MaterialBucket::GeometryIterator geomIt = mat->getGeometryIterator();
				//parse all the geometry buckets of the material bucket
				while(geomIt.hasMoreElements())
				{
					//get the source geometry bucket
					GeometryBucket *geom = geomIt.getNext();
					//create a new geometry bucket 
					GeometryBucket *geomBucket = OGRE_NEW GeometryBucket(matBucket,geom->getFormatString(),geom);
			
					//update the material bucket map of the material bucket
					matBucket->updateContainers(geomBucket, geomBucket->getFormatString() );

					//copy bounding informations
					geomBucket->getAABB()=geom->getAABB();
					geomBucket->setBoundingBox(	geom->getBoundingBox());
					//now setups the news InstancedObjects.
					for(objIt=ret->getInstancesMap().begin();objIt!=ret->getInstancesMap().end();objIt++)
					{
						//get the destination IntanciedObject
						InstancedObject*obj=objIt->second;
						InstancedObject::GeometryBucketList::iterator findIt;
						//check if the bucket is not already in the list
						findIt=std::find(obj->getGeometryBucketList().begin(),obj->getGeometryBucketList().end(),geomBucket);
						if(findIt==obj->getGeometryBucketList().end())
								obj->addBucketToList(geomBucket);
					}
			

				}	
			}
		}
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::updateContainers(LODBucket* bucket )
	{
		mLodBucketList.push_back(bucket);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::LODBucket::updateContainers(MaterialBucket* bucket, String& name )
	{
		mMaterialBucketMap[name] = bucket;
	}

	//--------------------------------------------------------------------------
	String InstancedGeometry::GeometryBucket::getFormatString(void) const
	{
		return mFormatString;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::attachToScene()
	{

			mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mName/*,mCentre*/);
			mNode->attachObject(this);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::MaterialBucket::updateContainers(InstancedGeometry::GeometryBucket* bucket, const String & format)
	{
		mCurrentGeometryMap[format]=bucket;
		mGeometryBucketList.push_back(bucket);
	}
	void InstancedGeometry::MaterialBucket:: setMaterial(const String & name)
	{
		mMaterial=MaterialManager::getSingleton().getByName(name);
	};
	//--------------------------------------------------------------------------
	void InstancedGeometry::destroy(void)
	{
		RenderOperationVector::iterator it;
		for(it=mRenderOps.begin();it!=mRenderOps.end();++it)
		{
			OGRE_DELETE (*it)->vertexData;
			OGRE_DELETE (*it)->indexData;
	
		}
		mRenderOps.clear();

		// delete the BatchInstances
		for (BatchInstanceMap::iterator i = mBatchInstanceMap.begin();
			i != mBatchInstanceMap.end(); ++i)
		{
			mOwner->extractMovableObject(i->second);
			OGRE_DELETE i->second;

		}
		mBatchInstanceMap.clear();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::reset(void)
	{
		destroy();

		for (QueuedSubMeshList::iterator i = mQueuedSubMeshes.begin();
			i != mQueuedSubMeshes.end(); ++i)
		{
			OGRE_DELETE *i;
			
		}
		mQueuedSubMeshes.clear();
		// Delete precached geoemtry lists
		for (SubMeshGeometryLookup::iterator l = mSubMeshGeometryLookup.begin();
			l != mSubMeshGeometryLookup.end(); ++l)
		{
			OGRE_DELETE_T(l->second, SubMeshLodGeometryLinkList, MEMCATEGORY_GEOMETRY);
		
		}
		mSubMeshGeometryLookup.clear();
		// Delete optimised geometry
		for (OptimisedSubMeshGeometryList::iterator o = mOptimisedSubMeshGeometryList.begin();
			o != mOptimisedSubMeshGeometryList.end(); ++o)
		{
			OGRE_DELETE *o;
			
		}
		mOptimisedSubMeshGeometryList.clear();

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::setVisible(bool visible)
	{
			
		mVisible = visible;
		// tell any existing BatchInstances
		for (BatchInstanceMap::iterator ri = mBatchInstanceMap.begin();
			ri != mBatchInstanceMap.end(); ++ri)
		{

			
			ri->second->setVisible(visible);
		}
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::setCastShadows(bool castShadows)
	{
		mCastShadows = castShadows;
		// tell any existing BatchInstances
		for (BatchInstanceMap::iterator ri = mBatchInstanceMap.begin();
			ri != mBatchInstanceMap.end(); ++ri)
		{
			ri->second->setCastShadows(castShadows);
		}

	}
	//--------------------------------------------------------------------------
    void InstancedGeometry::setRenderQueueGroup(uint8 queueID)
	{
		assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
		mRenderQueueIDSet = true;
		mRenderQueueID = queueID;
		// tell any existing BatchInstances
		for (BatchInstanceMap::iterator ri = mBatchInstanceMap.begin();
			ri != mBatchInstanceMap.end(); ++ri)
		{
			ri->second->setRenderQueueGroup(queueID);
		}
	}
	//--------------------------------------------------------------------------
	uint8 InstancedGeometry::getRenderQueueGroup(void) const
	{
		return mRenderQueueID;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::dump(const String& filename) const
	{
		std::ofstream of(filename.c_str());
		of << "Static Geometry Report for " << mName << std::endl;
		of << "-------------------------------------------------" << std::endl;
		of << "Number of queued submeshes: " << mQueuedSubMeshes.size() << std::endl;
		of << "Number of BatchInstances: " << mBatchInstanceMap.size() << std::endl;
		of << "BatchInstance dimensions: " << mBatchInstanceDimensions << std::endl;
		of << "Origin: " << mOrigin << std::endl;
		of << "Max distance: " << mUpperDistance << std::endl;
		of << "Casts shadows?: " << mCastShadows << std::endl;
		of << std::endl;
		for (BatchInstanceMap::const_iterator ri = mBatchInstanceMap.begin();
			ri != mBatchInstanceMap.end(); ++ri)
		{
			ri->second->dump(of);
		}
		of << "-------------------------------------------------" << std::endl;
	}
	//---------------------------------------------------------------------
	void InstancedGeometry::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		for (BatchInstanceMap::const_iterator ri = mBatchInstanceMap.begin();
			ri != mBatchInstanceMap.end(); ++ri)
		{
			ri->second->visitRenderables(visitor, debugRenderables);
		}

	}
	//--------------------------------------------------------------------------
	InstancedGeometry::InstancedObject::InstancedObject(int index,SkeletonInstance *skeleton, AnimationStateSet*animations):mIndex(index),
		mTransformation(Matrix4::ZERO),
		mOrientation(Quaternion::IDENTITY),
		mScale(Vector3::UNIT_SCALE),
		mPosition(Vector3::ZERO),
		mSkeletonInstance(skeleton),
		mBoneWorldMatrices(NULL),
        mBoneMatrices(NULL),
        mNumBoneMatrices(0),
		mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max())

	{
			
			mSkeletonInstance->load();
		
			mAnimationState = OGRE_NEW AnimationStateSet();
			mNumBoneMatrices = mSkeletonInstance->getNumBones();
			mBoneMatrices = OGRE_ALLOC_T(Matrix4, mNumBoneMatrices, MEMCATEGORY_ANIMATION);
			AnimationStateIterator it=animations->getAnimationStateIterator();
			while (it.hasMoreElements())
			{
				AnimationState*anim= it.getNext();
				mAnimationState->createAnimationState(anim->getAnimationName(),anim->getTimePosition(),anim->getLength(),
				anim->getWeight());

			}
			
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::InstancedObject::InstancedObject(int index):mIndex(index),
		mTransformation(Matrix4::ZERO),
		mOrientation(Quaternion::IDENTITY),
		mScale(Vector3::UNIT_SCALE),
		mPosition(Vector3::ZERO),
		mSkeletonInstance(0),
		mBoneWorldMatrices(0),
        mBoneMatrices(0),
		mAnimationState(0),
        mNumBoneMatrices(0),
		mFrameAnimationLastUpdated(std::numeric_limits<unsigned long>::max())
	{
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::InstancedObject::~InstancedObject()
	{
		mGeometryBucketList.clear();
		OGRE_DELETE mAnimationState;
		OGRE_FREE(mBoneMatrices, MEMCATEGORY_ANIMATION);
		OGRE_FREE(mBoneWorldMatrices, MEMCATEGORY_ANIMATION);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::addBucketToList(GeometryBucket*bucket)
	{
		mGeometryBucketList.push_back(bucket);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::setPosition( Vector3  position)
	{

		mPosition=position;
		needUpdate();
		BatchInstance*parentBatchInstance=(*(mGeometryBucketList.begin()))->getParent()->getParent()->getParent();
		parentBatchInstance->updateBoundingBox();

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::translate(const Vector3 &d)
	{
		mPosition += d;
		needUpdate();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::translate(const Matrix3 & axes,const Vector3 &move)
	{
		Vector3 derived = axes * move;
        translate(derived);
	}
	//--------------------------------------------------------------------------
	Matrix3 InstancedGeometry::InstancedObject::getLocalAxes() const
	{
		Vector3 axisX = Vector3::UNIT_X;
        Vector3 axisY = Vector3::UNIT_Y;
        Vector3 axisZ = Vector3::UNIT_Z;

        axisX = mOrientation * axisX;
        axisY = mOrientation * axisY;
        axisZ = mOrientation * axisZ;

        return Matrix3(axisX.x, axisY.x, axisZ.x,
                       axisX.y, axisY.y, axisZ.y,
                       axisX.z, axisY.z, axisZ.z);
	}
	//--------------------------------------------------------------------------
	Vector3 & InstancedGeometry::InstancedObject::getPosition(void)
	{
		return mPosition;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::yaw(const Radian&angle)
	{
		Quaternion q;
        q.FromAngleAxis(angle,Vector3::UNIT_Y);
        rotate(q);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::pitch(const Radian&angle)
	{
		Quaternion q;
        q.FromAngleAxis(angle,Vector3::UNIT_X);
        rotate(q);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::roll(const Radian&angle)
	{
		Quaternion q;
        q.FromAngleAxis(angle,Vector3::UNIT_Z);
        rotate(q);

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::setScale(const Vector3&scale)
	{
		mScale=scale;
		needUpdate();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::rotate(const Quaternion& q)
	{
	
            mOrientation = mOrientation * q;
			 needUpdate();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::setOrientation(const Quaternion& q)
	{	
        mOrientation = q;
		needUpdate();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::setPositionAndOrientation(Vector3 p, const Quaternion& q)
	{	
		mPosition = p;
        mOrientation = q;
        needUpdate();
		BatchInstance* parentBatchInstance=(*(mGeometryBucketList.begin()))->getParent()->getParent()->getParent();
		parentBatchInstance->updateBoundingBox();
	}

    //--------------------------------------------------------------------------
    Quaternion &InstancedGeometry::InstancedObject::getOrientation(void)
    {
        return mOrientation;
    }

	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::needUpdate()
	{
		 mTransformation.makeTransform(
                mPosition,
                mScale,
                mOrientation);
		 	
			
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::InstancedObject::updateAnimation(void)
	{

		if(mSkeletonInstance)
		{
			GeometryBucketList::iterator it;
		    mSkeletonInstance->setAnimationState(*mAnimationState);
            mSkeletonInstance->_getBoneMatrices(mBoneMatrices);

			 // Allocate bone world matrices on demand, for better memory footprint
            // when using software animation.
            if (!mBoneWorldMatrices)
            {
                mBoneWorldMatrices = OGRE_ALLOC_T(Matrix4, mNumBoneMatrices, MEMCATEGORY_ANIMATION);
            }

            for (unsigned short i = 0; i < mNumBoneMatrices; ++i)
            {
                mBoneWorldMatrices[i] =  mTransformation * mBoneMatrices[i];
			
            }
			


		}

	}
	//--------------------------------------------------------------------------
	AnimationState* InstancedGeometry::InstancedObject::getAnimationState(const String& name) const
    {
        if (!mAnimationState)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Object is not animated",
                "InstancedGeometry::InstancedObject::getAnimationState");
        }
//		AnimationStateIterator it=mAnimationState->getAnimationStateIterator();
//		while (it.hasMoreElements())
//		{
//			AnimationState*anim= it.getNext();
//
//			
//		}
		return mAnimationState->getAnimationState(name);
    }
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstanceIterator InstancedGeometry::getBatchInstanceIterator(void)
	{
		return BatchInstanceIterator(mBatchInstanceMap.begin(), mBatchInstanceMap.end());
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance::BatchInstance(InstancedGeometry* parent, const String& name,
		SceneManager* mgr, uint32 BatchInstanceID)
		: MovableObject(name), mParent(parent), mSceneMgr(mgr), mNode(0),
		mBatchInstanceID(BatchInstanceID), mBoundingRadius(0.0f),
		mCurrentLod(0),
        mLodStrategy(0)
	{
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance::~BatchInstance()
	{
		if (mNode)
		{
			mNode->getParentSceneNode()->removeChild(mNode);
			mSceneMgr->destroySceneNode(mNode->getName());
			mNode = 0;
		}
		// delete
		for (LODBucketList::iterator i = mLodBucketList.begin();
			i != mLodBucketList.end(); ++i)
		{
			OGRE_DELETE *i;
		}
		mLodBucketList.clear();
		ObjectsMap::iterator o;

		for(o=mInstancesMap.begin();o!=mInstancesMap.end();o++)
		{
			OGRE_DELETE o->second;
		}
		mInstancesMap.clear();
		// no need to delete queued meshes, these are managed in InstancedGeometry
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::assign(QueuedSubMesh* qmesh)
	{
		mQueuedSubMeshes.push_back(qmesh);

        // Set/check lod strategy
        const LodStrategy *lodStrategy = qmesh->submesh->parent->getLodStrategy();
        if (mLodStrategy == 0)
        {
            mLodStrategy = lodStrategy;

            // First LOD mandatory, and always from base lod value
            mLodValues.push_back(mLodStrategy->getBaseValue());
        }
        else
        {
            if (mLodStrategy != lodStrategy)
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Lod strategies do not match",
                    "InstancedGeometry::InstancedObject::assign");
        }

		// update lod values
		ushort lodLevels = qmesh->submesh->parent->getNumLodLevels();
		assert(qmesh->geometryLodList->size() == lodLevels);

		while(mLodValues.size() < lodLevels)
		{
			mLodValues.push_back(0.0f);
		}
		// Make sure LOD levels are max of all at the requested level
		for (ushort lod = 1; lod < lodLevels; ++lod)
		{
			const MeshLodUsage& meshLod =
				qmesh->submesh->parent->getLodLevel(lod);
			mLodValues[lod] = std::max(mLodValues[lod],
				meshLod.value);
		}

		// update bounds
		// Transform world bounds relative to our centre
		AxisAlignedBox localBounds(
			qmesh->worldBounds.getMinimum() ,
			qmesh->worldBounds.getMaximum());
		mAABB.merge(localBounds);
		mBoundingRadius = Math::boundingRadiusFromAABB(mAABB);

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::build()
	{
		// Create a node
		mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mName);
		mNode->attachObject(this);
		// We need to create enough LOD buckets to deal with the highest LOD
		// we encountered in all the meshes queued
		for (ushort lod = 0; lod < mLodValues.size(); ++lod)
		{
			LODBucket* lodBucket =
				OGRE_NEW LODBucket(this, lod, mLodValues[lod]);
			mLodBucketList.push_back(lodBucket);
			// Now iterate over the meshes and assign to LODs
			// LOD bucket will pick the right LOD to use
			QueuedSubMeshList::iterator qi, qiend;
			qiend = mQueuedSubMeshes.end();
			for (qi = mQueuedSubMeshes.begin(); qi != qiend; ++qi)
			{
				lodBucket->assign(*qi, lod);
			}
			// now build
			lodBucket->build();
		}


	}
	
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::updateBoundingBox()
	{

			Vector3 *Positions = OGRE_ALLOC_T(Vector3, mInstancesMap.size(), MEMCATEGORY_GEOMETRY);

			ObjectsMap::iterator objIt;
			size_t k = 0;
			for(objIt=mInstancesMap.begin();objIt!=mInstancesMap.end();objIt++)
			{
				Positions[k++] = objIt->second->getPosition();
			}

			LODIterator lodIterator = getLODIterator();
			while (lodIterator.hasMoreElements())
			{			
				LODBucket* lod = lodIterator.getNext();
				LODBucket::MaterialIterator matIt = lod->getMaterialIterator();
				while (matIt.hasMoreElements())
				{					
					MaterialBucket*mat = matIt.getNext();
					MaterialBucket::GeometryIterator geomIt = mat->getGeometryIterator();
					while(geomIt.hasMoreElements())
					{
						// to generate the boundingBox
						Real Xmin= Positions[0].x;
						Real Ymin= Positions[0].y;
						Real Zmin= Positions[0].z;

						Real Xmax= Positions[0].x;
						Real Ymax= Positions[0].y;
						Real Zmax= Positions[0].z;

						GeometryBucket*geom=geomIt.getNext();
						for (size_t i = 0; i < mInstancesMap.size (); i++)
						{
							if(Positions[i].x<Xmin)
								Xmin = Positions[i].x;
							if(Positions[i].y<Ymin)
								Ymin = Positions[i].y;
							if(Positions[i].z<Zmin)
								Zmin = Positions[i].z;
							if(Positions[i].x>Xmax)
								Xmax = Positions[i].x;
							if(Positions[i].y>Ymax)
								Ymax = Positions[i].y;
							if(Positions[i].z>Zmax)
								Zmax = Positions[i].z;
						}
						geom->setBoundingBox(AxisAlignedBox(Xmin,Ymin,Zmin,Xmax,Ymax,Zmax));						
                        this->mNode->_updateBounds();
						mAABB=AxisAlignedBox(
							Vector3(Xmin,Ymin,Zmin) + geom->getAABB().getMinimum(),
							Vector3(Xmax,Ymax,Zmax) + geom->getAABB().getMaximum());
					
					}
				}
			}

			OGRE_FREE(Positions, MEMCATEGORY_GEOMETRY);		
	}
	
	
	
	
	
	
	
	
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::addInstancedObject(int index,InstancedObject* object)
	{
		mInstancesMap[index]=object;
	}
	//--------------------------------------------------------------------------
	 InstancedGeometry::InstancedObject* InstancedGeometry::BatchInstance::isInstancedObjectPresent(int index)
	{
		if (mInstancesMap.find(index)!=mInstancesMap.end())
			return mInstancesMap[index];
		else return NULL;
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance::InstancedObjectIterator 
	InstancedGeometry::BatchInstance::getObjectIterator()
	{
		return InstancedObjectIterator(mInstancesMap.begin(), mInstancesMap.end());
	} 
	//--------------------------------------------------------------------------
	const String& InstancedGeometry::BatchInstance::getMovableType(void) const
	{
		static String sType = "InstancedGeometry";
		return sType;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::_notifyCurrentCamera(Camera* cam)
	{
        // Set camera
        mCamera = cam;


        // Cache squared view depth for use by GeometryBucket
        mSquaredViewDepth = mParentNode->getSquaredViewDepth(cam->getLodCamera());

        // No lod strategy set yet, skip (this indicates that there are no submeshes)
        if (mLodStrategy == 0)
            return;

        // Sanity check
        assert(!mLodValues.empty());

        // Calculate lod value
        Real lodValue = mLodStrategy->getValue(this, cam);

        // Store lod value for this strategy
        mLodValue = lodValue;

        // Get lod index
        mCurrentLod = mLodStrategy->getIndex(lodValue, mLodValues);
	}
	//--------------------------------------------------------------------------
	const AxisAlignedBox& InstancedGeometry::BatchInstance::getBoundingBox(void) const
	{
		return mAABB;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::setBoundingBox(AxisAlignedBox & box)
	{
		mAABB=box;
	}
	//--------------------------------------------------------------------------
	Real InstancedGeometry::BatchInstance::getBoundingRadius(void) const
	{
		return mBoundingRadius;
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::_updateRenderQueue(RenderQueue* queue)
	{
		ObjectsMap::iterator it;
		//we parse the Instanced Object map to update the animations.

		for (it=mInstancesMap.begin();it!=mInstancesMap.end();++it)
		{
			it->second->updateAnimation();
			
		}
	
		mLodBucketList[mCurrentLod]->addRenderables(queue, mRenderQueueID,
			mLodValue);
	}
	//---------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::visitRenderables(
		Renderable::Visitor* visitor, bool debugRenderables)
	{
		for (LODBucketList::iterator i = mLodBucketList.begin(); i != mLodBucketList.end(); ++i)
		{
			(*i)->visitRenderables(visitor, debugRenderables);
		}

	}
	//--------------------------------------------------------------------------
	bool InstancedGeometry::BatchInstance::isVisible(void) const
	{
		return mVisible && !mBeyondFarDistance;
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::BatchInstance::LODIterator
	InstancedGeometry::BatchInstance::getLODIterator(void)
	{
		return LODIterator(mLodBucketList.begin(), mLodBucketList.end());
	}
	//--------------------------------------------------------------------------
	const LightList& InstancedGeometry::BatchInstance::getLights(void) const
	{
		return queryLights();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::BatchInstance::dump(std::ofstream& of) const
	{
		of << "BatchInstance " << mBatchInstanceID << std::endl;
		of << "--------------------------" << std::endl;
		of << "Local AABB: " << mAABB << std::endl;
		of << "Bounding radius: " << mBoundingRadius << std::endl;
		of << "Number of LODs: " << mLodBucketList.size() << std::endl;

		for (LODBucketList::const_iterator i = mLodBucketList.begin();
			i != mLodBucketList.end(); ++i)
		{
			(*i)->dump(of);
		}
		of << "--------------------------" << std::endl;
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	InstancedGeometry::LODBucket::LODBucket(BatchInstance* parent, unsigned short lod,
		Real lodValue)
		: mParent(parent), mLod(lod), mLodValue(lodValue)
	{
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::LODBucket::~LODBucket()
	{
		// delete
		for (MaterialBucketMap::iterator i = mMaterialBucketMap.begin();
			i != mMaterialBucketMap.end(); ++i)
		{
			OGRE_DELETE i->second;
		}
		mMaterialBucketMap.clear();
		for(QueuedGeometryList::iterator qi = mQueuedGeometryList.begin();
			qi != mQueuedGeometryList.end(); ++qi)
		{
			OGRE_DELETE *qi;
		}
		mQueuedGeometryList.clear();
		// no need to delete queued meshes, these are managed in InstancedGeometry
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::LODBucket::assign(QueuedSubMesh* qmesh, ushort atLod)
	{
		QueuedGeometry* q = OGRE_NEW QueuedGeometry();
		mQueuedGeometryList.push_back(q);
		q->position = qmesh->position;
		q->orientation = qmesh->orientation;
		q->scale = qmesh->scale;
		q->ID = qmesh->ID;
		if (qmesh->geometryLodList->size() > atLod)
		{
			// This submesh has enough lods, use the right one
			q->geometry = &(*qmesh->geometryLodList)[atLod];
		}
		else
		{
			// Not enough lods, use the lowest one we have
			q->geometry =
				&(*qmesh->geometryLodList)[qmesh->geometryLodList->size() - 1];
		}
		// Locate a material bucket
		MaterialBucket* mbucket = 0;
		MaterialBucketMap::iterator m =
			mMaterialBucketMap.find(qmesh->materialName);
		if (m != mMaterialBucketMap.end())
		{
			mbucket = m->second;
		}
		else
		{
			mbucket = OGRE_NEW MaterialBucket(this, qmesh->materialName);
			mMaterialBucketMap[qmesh->materialName] = mbucket;
		}
		mbucket->assign(q);
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::LODBucket::build()
	{
		// Just pass this on to child buckets
		
		for (MaterialBucketMap::iterator i = mMaterialBucketMap.begin();
			i != mMaterialBucketMap.end(); ++i)
		{
			i->second->build();
		}
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::LODBucket::addRenderables(RenderQueue* queue,
		uint8 group, Real lodValue)
	{
		// Just pass this on to child buckets
		MaterialBucketMap::iterator i, iend;
		iend =  mMaterialBucketMap.end();
		for (i = mMaterialBucketMap.begin(); i != iend; ++i)
		{
			i->second->addRenderables(queue, group, lodValue);
		}
	}
	//---------------------------------------------------------------------
	void InstancedGeometry::LODBucket::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		MaterialBucketMap::iterator i, iend;
		iend =  mMaterialBucketMap.end();
		for (i = mMaterialBucketMap.begin(); i != iend; ++i)
		{
			i->second->visitRenderables(visitor, debugRenderables);
		}

	}
	//--------------------------------------------------------------------------
	InstancedGeometry::LODBucket::MaterialIterator
	InstancedGeometry::LODBucket::getMaterialIterator(void)
	{
		return MaterialIterator(
			mMaterialBucketMap.begin(), mMaterialBucketMap.end());
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::LODBucket::dump(std::ofstream& of) const
	{
		of << "LOD Bucket " << mLod << std::endl;
		of << "------------------" << std::endl;
		of << "Lod Value: " << mLodValue << std::endl;
		of << "Number of Materials: " << mMaterialBucketMap.size() << std::endl;
		for (MaterialBucketMap::const_iterator i = mMaterialBucketMap.begin();
			i != mMaterialBucketMap.end(); ++i)
		{
			i->second->dump(of);
		}
		of << "------------------" << std::endl;

	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	InstancedGeometry::MaterialBucket::MaterialBucket(LODBucket* parent,
		const String& materialName)
		: mParent(parent)
		, mMaterialName(materialName)
		, mTechnique(0)
		, mLastIndex(0)
	{
						mMaterial = MaterialManager::getSingleton().getByName(mMaterialName);
	}
	//--------------------------------------------------------------------------
	InstancedGeometry::MaterialBucket::~MaterialBucket()
	{
		// delete
		for (GeometryBucketList::iterator i = mGeometryBucketList.begin();
			i != mGeometryBucketList.end(); ++i)
		{
			OGRE_DELETE *i;
		}
		mGeometryBucketList.clear();
		// no need to delete queued meshes, these are managed in InstancedGeometry

	}
	//--------------------------------------------------------------------------

	void InstancedGeometry::MaterialBucket::assign(QueuedGeometry* qgeom)
	{
		// Look up any current geometry
		String formatString = getGeometryFormatString(qgeom->geometry);
		CurrentGeometryMap::iterator gi = mCurrentGeometryMap.find(formatString);
		bool newBucket = true;
		if (gi != mCurrentGeometryMap.end())
		{
			// Found existing geometry, try to assign
			newBucket = !gi->second->assign(qgeom);
			// Note that this bucket will be replaced as the 'current'
			// for this format string below since it's out of space
		}
		// Do we need to create a new one?
		if (newBucket)
		{
			GeometryBucket* gbucket = OGRE_NEW GeometryBucket(this, formatString,
				qgeom->geometry->vertexData, qgeom->geometry->indexData);
			// Add to main list
			mGeometryBucketList.push_back(gbucket);
			// Also index in 'current' list
			mCurrentGeometryMap[formatString] = gbucket;
			if (!gbucket->assign(qgeom))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					"Somehow we couldn't fit the requested geometry even in a "
					"brand new GeometryBucket!! Must be a bug, please report.",
					"InstancedGeometry::MaterialBucket::assign");
			}
		}
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::MaterialBucket::build()
	{
		mMaterial = MaterialManager::getSingleton().getByName(mMaterialName);
		if (mMaterial.isNull())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
				"Material '" + mMaterialName + "' not found.",
				"InstancedGeometry::MaterialBucket::build");
		}
		mMaterial->load();
		// tell the geometry buckets to build
		
		for (GeometryBucketList::iterator i = mGeometryBucketList.begin();
			i != mGeometryBucketList.end(); ++i)
		{
			(*i)->build();
		}
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::MaterialBucket::addRenderables(RenderQueue* queue,
		uint8 group, Real lodValue)
	{
        // Get batch instance
        BatchInstance *batchInstance = mParent->getParent();

        // Get material lod strategy
        const LodStrategy *materialLodStrategy = mMaterial->getLodStrategy();

        // If material strategy doesn't match, recompute lod value with correct strategy
        if (materialLodStrategy != batchInstance->mLodStrategy)
            lodValue = materialLodStrategy->getValue(batchInstance, batchInstance->mCamera);

		// Determine the current material technique
		mTechnique = mMaterial->getBestTechnique(
			mMaterial->getLodIndex(lodValue));	
		GeometryBucketList::iterator i, iend;
		iend =  mGeometryBucketList.end();
			
		for (i = mGeometryBucketList.begin(); i != iend; ++i)
		{
			queue->addRenderable(*i, group);
		}

	}
	//---------------------------------------------------------------------
	void InstancedGeometry::MaterialBucket::visitRenderables(
		Renderable::Visitor* visitor, bool debugRenderables)
	{
		GeometryBucketList::iterator i, iend;
		iend =  mGeometryBucketList.end();
		for (i = mGeometryBucketList.begin(); i != iend; ++i)
		{
			(*i)->visitRenderables(visitor, debugRenderables);
		}

	}
	//--------------------------------------------------------------------------
	String InstancedGeometry::MaterialBucket::getGeometryFormatString(
		SubMeshLodGeometryLink* geom)
	{
		// Formulate an identifying string for the geometry format
		// Must take into account the vertex declaration and the index type
		// Format is (all lines separated by '|'):
		// Index type
		// Vertex element (repeating)
		//   source
		//   semantic
		//   type
		StringUtil::StrStreamType str;

		str << geom->indexData->indexBuffer->getType() << "|";
		const VertexDeclaration::VertexElementList& elemList =
			geom->vertexData->vertexDeclaration->getElements();
		VertexDeclaration::VertexElementList::const_iterator ei, eiend;
		eiend = elemList.end();
		for (ei = elemList.begin(); ei != eiend; ++ei)
		{
			const VertexElement& elem = *ei;
			str << elem.getSource() << "|";
			str << elem.getSource() << "|";
			str << elem.getSemantic() << "|";
			str << elem.getType() << "|";
		}

		return str.str();

	}
	//--------------------------------------------------------------------------
	InstancedGeometry::MaterialBucket::GeometryIterator
	InstancedGeometry::MaterialBucket::getGeometryIterator(void)
	{
		return GeometryIterator(
			mGeometryBucketList.begin(), mGeometryBucketList.end());
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::MaterialBucket::dump(std::ofstream& of) const
	{
		of << "Material Bucket " << mMaterialName << std::endl;
		of << "--------------------------------------------------" << std::endl;
		of << "Geometry buckets: " << mGeometryBucketList.size() << std::endl;
		for (GeometryBucketList::const_iterator i = mGeometryBucketList.begin();
			i != mGeometryBucketList.end(); ++i)
		{
			(*i)->dump(of);
		}
		of << "--------------------------------------------------" << std::endl;

	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	InstancedGeometry::GeometryBucket::GeometryBucket(MaterialBucket* parent,
		const String& formatString, const VertexData* vData,
		const IndexData* iData)
		: SimpleRenderable(),
		 mParent(parent), 
		 mFormatString(formatString),
		 mVertexData(0),
		 mIndexData(0)
	{
	   	mBatch=mParent->getParent()->getParent()->getParent();
		if(!mBatch->getBaseSkeleton().isNull())
			setCustomParameter(0,Vector4(mBatch->getBaseSkeleton()->getNumBones(),0,0,0));
		//mRenderOperation=OGRE_NEW RenderOperation();
		// Clone the structure from the example
		mVertexData = vData->clone(false);

		mRenderOp.useIndexes = true;
		mRenderOp.indexData = OGRE_NEW IndexData();

		mRenderOp.indexData->indexCount = 0;
		mRenderOp.indexData->indexStart = 0;
		mRenderOp.vertexData = OGRE_NEW VertexData();
		mRenderOp.vertexData->vertexCount = 0;

		mRenderOp.vertexData->vertexDeclaration = vData->vertexDeclaration->clone();
		mIndexType = iData->indexBuffer->getType();
		// Derive the max vertices
		if (mIndexType == HardwareIndexBuffer::IT_32BIT)
		{
			mMaxVertexIndex = 0xFFFFFFFF;
		}
		else
		{
			mMaxVertexIndex = 0xFFFF;
		}


		size_t offset=0;	
		unsigned short texCoordOffset=0;
		unsigned short texCoordSource=0;

		const Ogre::VertexElement*elem=mRenderOp.vertexData->vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES);
	
		texCoordSource=elem->getSource();
		for(ushort i=0;i<mRenderOp.vertexData->vertexDeclaration->getElementCount();i++)
		{
			if(mRenderOp.vertexData->vertexDeclaration->getElement(i)->getSemantic() == VES_TEXTURE_COORDINATES)
			{
				texCoordOffset++;
			}
			if(texCoordSource==mRenderOp.vertexData->vertexDeclaration->getElement(i)->getSource())
			{
				offset+= VertexElement::getTypeSize(
				mRenderOp.vertexData->vertexDeclaration->getElement(i)->getType());
			}		
		}

		mRenderOp.vertexData->vertexDeclaration->addElement(texCoordSource, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, texCoordOffset);
 		mTexCoordIndex = texCoordOffset;

	}
	//--------------------------------------------------------------------------
	InstancedGeometry::GeometryBucket::GeometryBucket(MaterialBucket* parent,
		const String& formatString,GeometryBucket* bucket)
		: SimpleRenderable(),
		 mParent(parent),
		  mFormatString(formatString),
		  mVertexData(0),
		  mIndexData(0)
	{

	   	mBatch=mParent->getParent()->getParent()->getParent();
		if(!mBatch->getBaseSkeleton().isNull())
			setCustomParameter(0,Vector4(mBatch->getBaseSkeleton()->getNumBones(),0,0,0));
		bucket->getRenderOperation(mRenderOp);
		mVertexData=mRenderOp.vertexData;
		mIndexData=mRenderOp.indexData;
		setBoundingBox(AxisAlignedBox(-10000,-10000,-10000,
		10000,10000,10000));

	}
	//--------------------------------------------------------------------------
	InstancedGeometry::GeometryBucket::~GeometryBucket()
	{	
	}

	//--------------------------------------------------------------------------
	Real InstancedGeometry::GeometryBucket::getBoundingRadius(void) const
	{
		return 1;
	}
	//--------------------------------------------------------------------------
	const MaterialPtr& InstancedGeometry::GeometryBucket::getMaterial(void) const
	{
		return mParent->getMaterial();
	}
	//--------------------------------------------------------------------------
	Technique* InstancedGeometry::GeometryBucket::getTechnique(void) const
	{
		return mParent->getCurrentTechnique();
	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::GeometryBucket::getWorldTransforms(Matrix4* xform) const
	{
			// Should be the identity transform, but lets allow transformation of the
		// nodes the BatchInstances are attached to for kicks
		if(mBatch->getBaseSkeleton().isNull())
		{
			BatchInstance::ObjectsMap::iterator it,itbegin,itend,newit;
			itbegin=mParent->getParent()->getParent()->getInstancesMap().begin();
			itend=mParent->getParent()->getParent()->getInstancesMap().end();

			for (it=itbegin;
				it!=itend;
				++it,++xform)
			{
				
					*xform = it->second->mTransformation;
			}
		}
		else
		{
			BatchInstance::ObjectsMap::iterator it,itbegin,itend,newit;
			itbegin=mParent->getParent()->getParent()->getInstancesMap().begin();
			itend=mParent->getParent()->getParent()->getInstancesMap().end();

			for (it=itbegin;
				it!=itend;
				++it)
			{
				
				for(int i=0;i<it->second->mNumBoneMatrices;++i,++xform)
				{
					*xform = it->second->mBoneWorldMatrices[i];
				}
			}
				
		}

	}
	//--------------------------------------------------------------------------
	unsigned short InstancedGeometry::GeometryBucket::getNumWorldTransforms(void) const 
	{
		if(mBatch->getBaseSkeleton().isNull())
		{
			BatchInstance* batch=mParent->getParent()->getParent();
			return static_cast<ushort>(batch->getInstancesMap().size());
		}
		else
		{
			BatchInstance* batch=mParent->getParent()->getParent();
			return static_cast<ushort>(
				mBatch->getBaseSkeleton()->getNumBones()*batch->getInstancesMap().size());
		}
	}
	//--------------------------------------------------------------------------
	Real InstancedGeometry::GeometryBucket::getSquaredViewDepth(const Camera* cam) const
	{
        const BatchInstance *batchInstance = mParent->getParent()->getParent();
        if (cam == batchInstance->mCamera)
            return batchInstance->mSquaredViewDepth;
        else
            return batchInstance->getParentNode()->getSquaredViewDepth(cam->getLodCamera());
	}
	//--------------------------------------------------------------------------
	const LightList& InstancedGeometry::GeometryBucket::getLights(void) const
	{
		return mParent->getParent()->getParent()->getLights();
	}
	//--------------------------------------------------------------------------
	bool InstancedGeometry::GeometryBucket::getCastsShadows(void) const
	{
		return mParent->getParent()->getParent()->getCastShadows();
	}
	//--------------------------------------------------------------------------
	bool InstancedGeometry::GeometryBucket::assign(QueuedGeometry* qgeom)
	{
		// Do we have enough space?
		if (mRenderOp.vertexData->vertexCount + qgeom->geometry->vertexData->vertexCount
			> mMaxVertexIndex)
		{
			return false;
		}

		mQueuedGeometry.push_back(qgeom);
		mRenderOp.vertexData->vertexCount += qgeom->geometry->vertexData->vertexCount;
		mRenderOp.indexData->indexCount += qgeom->geometry->indexData->indexCount;

		return true;
	}

	//--------------------------------------------------------------------------
	void InstancedGeometry::GeometryBucket::build()
	{


	
		// Ok, here's where we transfer the vertices and indexes to the shared
		// buffers
		// Shortcuts
		VertexDeclaration* dcl = mRenderOp.vertexData->vertexDeclaration;
		VertexBufferBinding* binds =mVertexData->vertexBufferBinding;

		// create index buffer, and lock
		mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton()
			.createIndexBuffer(mIndexType, mRenderOp.indexData->indexCount,
				HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		uint32* p32Dest = 0;
		uint16* p16Dest = 0;

		if (mIndexType == HardwareIndexBuffer::IT_32BIT)
		{
			p32Dest = static_cast<uint32*>(
				mRenderOp.indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		}
		else
		{
			p16Dest = static_cast<uint16*>(
				mRenderOp.indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		}

		// create all vertex buffers, and lock
		ushort b;
		//ushort posBufferIdx = dcl->findElementBySemantic(VES_POSITION)->getSource();
	
		vector<uchar*>::type destBufferLocks;
		vector<VertexDeclaration::VertexElementList>::type bufferElements;

		for (b = 0; b < binds->getBufferCount(); ++b)
		{

			size_t vertexCount = mRenderOp.vertexData->vertexCount;

			HardwareVertexBufferSharedPtr vbuf =
				HardwareBufferManager::getSingleton().createVertexBuffer(
					dcl->getVertexSize(b),
					vertexCount,
					HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			binds->setBinding(b, vbuf);
			uchar* pLock = static_cast<uchar*>(
				vbuf->lock(HardwareBuffer::HBL_DISCARD));
			destBufferLocks.push_back(pLock);
			// Pre-cache vertex elements per buffer
			bufferElements.push_back(dcl->findElementsBySource(b));
			mRenderOp.vertexData->vertexBufferBinding->setBinding(b,vbuf);
		}


		// Iterate over the geometry items
		size_t indexOffset = 0;

		QueuedGeometryList::iterator gi, giend;
		giend = mQueuedGeometry.end();

		// to generate the boundingBox
		Real Xmin,Ymin,Zmin,Xmax,Ymax,Zmax; 
		Xmin=0;
		Ymin=0;
		Zmin=0;
		Xmax=0;
		Ymax=0;
		Zmax=0;
		QueuedGeometry* precGeom = *(mQueuedGeometry.begin());
		int index=0;
		if( mParent->getLastIndex()!=0)
			index=mParent->getLastIndex()+1;

		for (gi = mQueuedGeometry.begin(); gi != giend; ++gi)
		{

			QueuedGeometry* geom = *gi;
			if(precGeom->ID!=geom->ID)
					index++;

			//create  a new instanced object
			InstancedObject* instancedObject = mParent->getParent()->getParent()->isInstancedObjectPresent(index);
			if(instancedObject == NULL)
			{
				if(mBatch->getBaseSkeleton().isNull())
				{
					instancedObject= OGRE_NEW InstancedObject(index);
				}
				else
				{
					instancedObject= OGRE_NEW InstancedObject(index,mBatch->getBaseSkeletonInstance(),
						mBatch->getBaseAnimationState());
				}
				mParent->getParent()->getParent()->addInstancedObject(index,instancedObject);

			}
			instancedObject->addBucketToList(this);


			
			// Copy indexes across with offset
			IndexData* srcIdxData = geom->geometry->indexData;
			if (mIndexType == HardwareIndexBuffer::IT_32BIT)
			{
				// Lock source indexes
				uint32* pSrc = static_cast<uint32*>(
					srcIdxData->indexBuffer->lock(
						srcIdxData->indexStart, 
						srcIdxData->indexCount * srcIdxData->indexBuffer->getIndexSize(),
						HardwareBuffer::HBL_READ_ONLY));

				copyIndexes(pSrc, p32Dest, srcIdxData->indexCount, indexOffset);
				p32Dest += srcIdxData->indexCount;
				srcIdxData->indexBuffer->unlock();
			}
			else
			{
				
				// Lock source indexes
				uint16* pSrc = static_cast<uint16*>(
					srcIdxData->indexBuffer->lock(
					srcIdxData->indexStart, 
					srcIdxData->indexCount * srcIdxData->indexBuffer->getIndexSize(),
					HardwareBuffer::HBL_READ_ONLY));

				copyIndexes(pSrc, p16Dest, srcIdxData->indexCount, indexOffset);
				p16Dest += srcIdxData->indexCount;
				srcIdxData->indexBuffer->unlock();
			}

			// Now deal with vertex buffers
			// we can rely on buffer counts / formats being the same
			VertexData* srcVData = geom->geometry->vertexData;
			VertexBufferBinding* srcBinds = srcVData->vertexBufferBinding;
		
			for (b = 0; b < binds->getBufferCount(); ++b)
			{

				// lock source
				HardwareVertexBufferSharedPtr srcBuf =
					srcBinds->getBuffer(b);
				uchar* pSrcBase = static_cast<uchar*>(
					srcBuf->lock(HardwareBuffer::HBL_READ_ONLY));
				// Get buffer lock pointer, we'll update this later
				uchar* pDstBase = destBufferLocks[b];
				size_t bufInc = srcBuf->getVertexSize();
			
				// Iterate over vertices
				float *pSrcReal, *pDstReal;
				Vector3 tmp;

			
			
				for (size_t v = 0; v < srcVData->vertexCount; ++v)
				{
					//to know if the current buffer is the one with the buffer or not
					bool isTheBufferWithIndex=false;
					// Iterate over vertex elements
					VertexDeclaration::VertexElementList& elems =
						bufferElements[b];
					VertexDeclaration::VertexElementList::iterator ei;
				
					for (ei = elems.begin(); ei != elems.end(); ++ei)
					{
						VertexElement& elem = *ei;
						elem.baseVertexPointerToElement(pSrcBase, &pSrcReal);
						elem.baseVertexPointerToElement(pDstBase, &pDstReal);
						if(elem.getSemantic()==VES_TEXTURE_COORDINATES && elem.getIndex()==mTexCoordIndex)
						{
							isTheBufferWithIndex=true;
							*pDstReal++=index;
						}
						else
						{
							switch (elem.getSemantic())
							{
								case VES_POSITION:
									tmp.x = *pSrcReal++;
									tmp.y = *pSrcReal++;
									tmp.z = *pSrcReal++;
									*pDstReal++ = tmp.x;
									*pDstReal++ = tmp.y;
									*pDstReal++ = tmp.z;
									if(tmp.x<Xmin)
										Xmin = tmp.x;
									if(tmp.y<Ymin)
										Ymin = tmp.y;
									if(tmp.z<Zmin)
										Zmin = tmp.z;
									if(tmp.x>Xmax)
										Xmax = tmp.x;
									if(tmp.y>Ymax)
										Ymax = tmp.y;
									if(tmp.z>Zmax)
										Zmax = tmp.z;
								default:
								// just raw copy
								memcpy(pDstReal, pSrcReal,
									VertexElement::getTypeSize(elem.getType()));
								break;
							};

						}
					

					}
					if (isTheBufferWithIndex)
					pDstBase += bufInc+4;
					else
					pDstBase += bufInc;
					pSrcBase += bufInc;

				}
	
				// Update pointer
				destBufferLocks[b] = pDstBase;
				srcBuf->unlock();

			
			}
			indexOffset += geom->geometry->vertexData->vertexCount;


		precGeom=geom;

		}
		mParent->setLastIndex(index);
		// Unlock everything
		mRenderOp.indexData->indexBuffer->unlock();
		for (b = 0; b < binds->getBufferCount(); ++b)
		{
			binds->getBuffer(b)->unlock();
		}
	
	OGRE_DELETE mVertexData;
	OGRE_DELETE mIndexData;
	
	mVertexData=mRenderOp.vertexData;
	mIndexData=mRenderOp.indexData;
	mBatch->getRenderOperationVector().push_back(&mRenderOp);
	setBoundingBox(AxisAlignedBox(Xmin,Ymin,Zmin,Xmax,Ymax,Zmax));
	mAABB=AxisAlignedBox(Xmin,Ymin,Zmin,Xmax,Ymax,Zmax);

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::GeometryBucket::dump(std::ofstream& of) const
	{
		of << "Geometry Bucket" << std::endl;
		of << "---------------" << std::endl;
		of << "Format string: " << mFormatString << std::endl;
		of << "Geometry items: " << mQueuedGeometry.size() << std::endl;
		of << "---------------" << std::endl;

	}
	//--------------------------------------------------------------------------
	void InstancedGeometry::GeometryBucket::visitRenderables(
		Renderable::Visitor* visitor, bool debugRenderables)
	{
		visitor->visit(this, mParent->getParent()->getLod(), false);
	}
	//---------------------------------------------------------------------

}

