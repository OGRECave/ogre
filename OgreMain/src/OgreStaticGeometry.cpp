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
#include "OgreStaticGeometry.h"
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

namespace Ogre {

	#define REGION_RANGE 1024
	#define REGION_HALF_RANGE 512
	#define REGION_MAX_INDEX 511
	#define REGION_MIN_INDEX -512

	//--------------------------------------------------------------------------
	StaticGeometry::StaticGeometry(SceneManager* owner, const String& name):
		mOwner(owner),
		mName(name),
		mBuilt(false),
		mUpperDistance(0.0f),
		mSquaredUpperDistance(0.0f),
		mCastShadows(false),
		mRegionDimensions(Vector3(1000,1000,1000)),
		mHalfRegionDimensions(Vector3(500,500,500)),
		mOrigin(Vector3(0,0,0)),
		mVisible(true),
        mRenderQueueID(RENDER_QUEUE_MAIN),
        mRenderQueueIDSet(false),
		mVisibilityFlags(Ogre::MovableObject::getDefaultVisibilityFlags())
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::~StaticGeometry()
	{
		reset();
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(const AxisAlignedBox& bounds,
		bool autoCreate)
	{
		if (bounds.isNull())
			return 0;

		// Get the region which has the largest overlapping volume
		const Vector3 min = bounds.getMinimum();
		const Vector3 max = bounds.getMaximum();

		// Get the min and max region indexes
		ushort minx, miny, minz;
		ushort maxx, maxy, maxz;
		getRegionIndexes(min, minx, miny, minz);
		getRegionIndexes(max, maxx, maxy, maxz);
		Real maxVolume = 0.0f;
		ushort finalx = 0, finaly = 0, finalz = 0;
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

		return getRegion(finalx, finaly, finalz, autoCreate);

	}
	//--------------------------------------------------------------------------
	Real StaticGeometry::getVolumeIntersection(const AxisAlignedBox& box,
		ushort x, ushort y, ushort z)
	{
		// Get bounds of indexed region
		AxisAlignedBox regionBounds = getRegionBounds(x, y, z);
		AxisAlignedBox intersectBox = regionBounds.intersection(box);
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
	AxisAlignedBox StaticGeometry::getRegionBounds(ushort x, ushort y, ushort z)
	{
		Vector3 min(
			((Real)x - REGION_HALF_RANGE) * mRegionDimensions.x + mOrigin.x,
			((Real)y - REGION_HALF_RANGE) * mRegionDimensions.y + mOrigin.y,
			((Real)z - REGION_HALF_RANGE) * mRegionDimensions.z + mOrigin.z
			);
		Vector3 max = min + mRegionDimensions;
		return AxisAlignedBox(min, max);
	}
	//--------------------------------------------------------------------------
 	Vector3 StaticGeometry::getRegionCentre(ushort x, ushort y, ushort z)
	{
		return Vector3(
			((Real)x - REGION_HALF_RANGE) * mRegionDimensions.x + mOrigin.x
				+ mHalfRegionDimensions.x,
			((Real)y - REGION_HALF_RANGE) * mRegionDimensions.y + mOrigin.y
				+ mHalfRegionDimensions.y,
			((Real)z - REGION_HALF_RANGE) * mRegionDimensions.z + mOrigin.z
				+ mHalfRegionDimensions.z
			);
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(
			ushort x, ushort y, ushort z, bool autoCreate)
	{
		uint32 index = packIndex(x, y, z);
		Region* ret = getRegion(index);
		if (!ret && autoCreate)
		{
			// Make a name
			StringUtil::StrStreamType str;
			str << mName << ":" << index;
			// Calculate the region centre
			Vector3 centre = getRegionCentre(x, y, z);
			ret = OGRE_NEW Region(this, str.str(), mOwner, index, centre);
			mOwner->injectMovableObject(ret);
			ret->setVisible(mVisible);
			ret->setCastShadows(mCastShadows);
			if (mRenderQueueIDSet)
			{
				ret->setRenderQueueGroup(mRenderQueueID);
			}
			mRegionMap[index] = ret;
		}
		return ret;
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(uint32 index)
	{
		RegionMap::iterator i = mRegionMap.find(index);
		if (i != mRegionMap.end())
		{
			return i->second;
		}
		else
		{
			return 0;
		}

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::getRegionIndexes(const Vector3& point,
		ushort& x, ushort& y, ushort& z)
	{
		// Scale the point into multiples of region and adjust for origin
		Vector3 scaledPoint = (point - mOrigin) / mRegionDimensions;

		// Round down to 'bottom left' point which represents the cell index
		int ix = Math::IFloor(scaledPoint.x);
		int iy = Math::IFloor(scaledPoint.y);
		int iz = Math::IFloor(scaledPoint.z);

		// Check bounds
		if (ix < REGION_MIN_INDEX || ix > REGION_MAX_INDEX
			|| iy < REGION_MIN_INDEX || iy > REGION_MAX_INDEX
			|| iz < REGION_MIN_INDEX || iz > REGION_MAX_INDEX)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Point out of bounds",
				"StaticGeometry::getRegionIndexes");
		}
		// Adjust for the fact that we use unsigned values for simplicity
		// (requires less faffing about for negatives give 10-bit packing
		x = static_cast<ushort>(ix + REGION_HALF_RANGE);
		y = static_cast<ushort>(iy + REGION_HALF_RANGE);
		z = static_cast<ushort>(iz + REGION_HALF_RANGE);


	}
	//--------------------------------------------------------------------------
	uint32 StaticGeometry::packIndex(ushort x, ushort y, ushort z)
	{
		return x + (y << 10) + (z << 20);
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(const Vector3& point,
		bool autoCreate)
	{
		ushort x, y, z;
		getRegionIndexes(point, x, y, z);
		return getRegion(x, y, z, autoCreate);
	}
	//--------------------------------------------------------------------------
	AxisAlignedBox StaticGeometry::calculateBounds(VertexData* vertexData,
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
	void StaticGeometry::addEntity(Entity* ent, const Vector3& position,
		const Quaternion& orientation, const Vector3& scale)
	{
		const MeshPtr& msh = ent->getMesh();
		// Validate
		if (msh->isLodManual())
		{
			LogManager::getSingleton().logMessage(
				"WARNING (StaticGeometry): Manual LOD is not supported. "
				"Using only highest LOD level for mesh " + msh->getName());
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
			// Determine the bounds based on the highest LOD
			q->worldBounds = calculateBounds(
				(*q->geometryLodList)[0].vertexData,
					position, orientation, scale);

			mQueuedSubMeshes.push_back(q);
		}
	}
	//--------------------------------------------------------------------------
	StaticGeometry::SubMeshLodGeometryLinkList*
	StaticGeometry::determineGeometry(SubMesh* sm)
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
	void StaticGeometry::splitGeometry(VertexData* vd, IndexData* id,
			StaticGeometry::SubMeshLodGeometryLink* targetGeomLink)
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
				id->indexStart, 
				id->indexCount * id->indexBuffer->getIndexSize(), 
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
				id->indexStart, 
				id->indexCount * id->indexBuffer->getIndexSize(), 
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
	void StaticGeometry::addSceneNode(const SceneNode* node)
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
			const SceneNode* node = static_cast<const SceneNode*>(nodei.getNext());
			// Add this subnode and its children...
			addSceneNode( node );
		}
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::build(void)
	{
		// Make sure there's nothing from previous builds
		destroy();

		// Firstly allocate meshes to regions
		for (QueuedSubMeshList::iterator qi = mQueuedSubMeshes.begin();
			qi != mQueuedSubMeshes.end(); ++qi)
		{
			QueuedSubMesh* qsm = *qi;
			Region* region = getRegion(qsm->worldBounds, true);
			region->assign(qsm);
		}
		bool stencilShadows = false;
		if (mCastShadows && mOwner->isShadowTechniqueStencilBased())
		{
			stencilShadows = true;
		}

		// Now tell each region to build itself
		for (RegionMap::iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->build(stencilShadows);
			
			// Set the visibility flags on these regions
			ri->second->setVisibilityFlags(mVisibilityFlags);
		}

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::destroy(void)
	{
		// delete the regions
		for (RegionMap::iterator i = mRegionMap.begin();
			i != mRegionMap.end(); ++i)
		{
			mOwner->extractMovableObject(i->second);
			OGRE_DELETE i->second;
		}
		mRegionMap.clear();
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::reset(void)
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
	void StaticGeometry::setVisible(bool visible)
	{
		mVisible = visible;
		// tell any existing regions
		for (RegionMap::iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->setVisible(visible);
		}
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::setCastShadows(bool castShadows)
	{
		mCastShadows = castShadows;
		// tell any existing regions
		for (RegionMap::iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->setCastShadows(castShadows);
		}

	}
	//--------------------------------------------------------------------------
    void StaticGeometry::setRenderQueueGroup(uint8 queueID)
	{
		assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
		mRenderQueueIDSet = true;
		mRenderQueueID = queueID;
		// tell any existing regions
		for (RegionMap::iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->setRenderQueueGroup(queueID);
		}
	}
	//--------------------------------------------------------------------------
	uint8 StaticGeometry::getRenderQueueGroup(void) const
	{
		return mRenderQueueID;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::setVisibilityFlags(uint32 flags)
	{
		mVisibilityFlags = flags;
		for (RegionMap::const_iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->setVisibilityFlags(flags);
		}
	}
	//--------------------------------------------------------------------------
	uint32 StaticGeometry::getVisibilityFlags() const
	{
		if(mRegionMap.empty())
			return MovableObject::getDefaultVisibilityFlags();

		RegionMap::const_iterator ri = mRegionMap.begin();
		return ri->second->getVisibilityFlags();
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::dump(const String& filename) const
	{
		std::ofstream of(filename.c_str());
		of << "Static Geometry Report for " << mName << std::endl;
		of << "-------------------------------------------------" << std::endl;
		of << "Number of queued submeshes: " << mQueuedSubMeshes.size() << std::endl;
		of << "Number of regions: " << mRegionMap.size() << std::endl;
		of << "Region dimensions: " << mRegionDimensions << std::endl;
		of << "Origin: " << mOrigin << std::endl;
		of << "Max distance: " << mUpperDistance << std::endl;
		of << "Casts shadows?: " << mCastShadows << std::endl;
		of << std::endl;
		for (RegionMap::const_iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->dump(of);
		}
		of << "-------------------------------------------------" << std::endl;
	}
	//---------------------------------------------------------------------
	void StaticGeometry::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		for (RegionMap::const_iterator ri = mRegionMap.begin();
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->visitRenderables(visitor, debugRenderables);
		}

	}
	//--------------------------------------------------------------------------
	StaticGeometry::RegionIterator StaticGeometry::getRegionIterator(void)
	{
		return RegionIterator(mRegionMap.begin(), mRegionMap.end());
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::Region::Region(StaticGeometry* parent, const String& name,
		SceneManager* mgr, uint32 regionID, const Vector3& centre)
		: MovableObject(name), mParent(parent), mSceneMgr(mgr), mNode(0),
		mRegionID(regionID), mCentre(centre), mBoundingRadius(0.0f),
		mCurrentLod(0), mLodStrategy(0)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region::~Region()
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

		// no need to delete queued meshes, these are managed in StaticGeometry

	}
	//--------------------------------------------------------------------------
	uint32 StaticGeometry::Region::getTypeFlags(void) const
	{
		return SceneManager::STATICGEOMETRY_TYPE_MASK;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::assign(QueuedSubMesh* qmesh)
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
                    "StaticGeometry::Region::assign");
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
			qmesh->worldBounds.getMinimum() - mCentre,
			qmesh->worldBounds.getMaximum() - mCentre);
		mAABB.merge(localBounds);
		mBoundingRadius = Math::boundingRadiusFromAABB(mAABB);

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::build(bool stencilShadows)
	{
		// Create a node
		mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mName,
			mCentre);
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
			lodBucket->build(stencilShadows);
		}



					}
	//--------------------------------------------------------------------------
	const String& StaticGeometry::Region::getMovableType(void) const
	{
		static String sType = "StaticGeometry";
		return sType;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::_notifyCurrentCamera(Camera* cam)
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
	const AxisAlignedBox& StaticGeometry::Region::getBoundingBox(void) const
	{
		return mAABB;
	}
	//--------------------------------------------------------------------------
	Real StaticGeometry::Region::getBoundingRadius(void) const
	{
		return mBoundingRadius;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::_updateRenderQueue(RenderQueue* queue)
	{
		mLodBucketList[mCurrentLod]->addRenderables(queue, mRenderQueueID,
			mLodValue);
	}
	//---------------------------------------------------------------------
	void StaticGeometry::Region::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		for (LODBucketList::iterator i = mLodBucketList.begin(); i != mLodBucketList.end(); ++i)
		{
			(*i)->visitRenderables(visitor, debugRenderables);
		}

	}
	//--------------------------------------------------------------------------
	bool StaticGeometry::Region::isVisible(void) const
	{
		if(!mVisible || mBeyondFarDistance)
			return false;

		SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
        if (sm && !(mVisibilityFlags & sm->_getCombinedVisibilityMask()))
            return false;

        return true;
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region::LODIterator
	StaticGeometry::Region::getLODIterator(void)
	{
		return LODIterator(mLodBucketList.begin(), mLodBucketList.end());
	}
	//---------------------------------------------------------------------
	ShadowCaster::ShadowRenderableListIterator
	StaticGeometry::Region::getShadowVolumeRenderableIterator(
		ShadowTechnique shadowTechnique, const Light* light,
		HardwareIndexBufferSharedPtr* indexBuffer,
		bool extrude, Real extrusionDistance, unsigned long flags)
	{
		// Calculate the object space light details
		Vector4 lightPos = light->getAs4DVector();
		Matrix4 world2Obj = mParentNode->_getFullTransform().inverseAffine();
		lightPos = world2Obj.transformAffine(lightPos);

		// per-LOD shadow lists & edge data
		mLodBucketList[mCurrentLod]->updateShadowRenderables(
			shadowTechnique, lightPos, indexBuffer, extrude, extrusionDistance, flags);
		
		EdgeData* edgeList = mLodBucketList[mCurrentLod]->getEdgeList();
		ShadowRenderableList& shadowRendList = mLodBucketList[mCurrentLod]->getShadowRenderableList();

		// Calc triangle light facing
		updateEdgeListLightFacing(edgeList, lightPos);

		// Generate indexes and update renderables
		generateShadowVolume(edgeList, *indexBuffer, light,
			shadowRendList, flags);


		return ShadowCaster::ShadowRenderableListIterator(shadowRendList.begin(), shadowRendList.end());

	}
	//--------------------------------------------------------------------------
	EdgeData* StaticGeometry::Region::getEdgeList(void)
	{
		return mLodBucketList[mCurrentLod]->getEdgeList();
	}
	//--------------------------------------------------------------------------
	bool StaticGeometry::Region::hasEdgeList(void)
	{
		return getEdgeList() != 0;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::dump(std::ofstream& of) const
	{
		of << "Region " << mRegionID << std::endl;
		of << "--------------------------" << std::endl;
		of << "Centre: " << mCentre << std::endl;
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
	StaticGeometry::LODBucket::LODShadowRenderable::LODShadowRenderable(
		LODBucket* parent, HardwareIndexBufferSharedPtr* indexBuffer,
		const VertexData* vertexData, bool createSeparateLightCap,
		bool isLightCap)
		: mParent(parent)
	{
		// Initialise render op
		mRenderOp.indexData = OGRE_NEW IndexData();
		mRenderOp.indexData->indexBuffer = *indexBuffer;
		mRenderOp.indexData->indexStart = 0;
		// index start and count are sorted out later

		// Create vertex data which just references position component (and 2 component)
		mRenderOp.vertexData = OGRE_NEW VertexData();
		// Map in position data
		mRenderOp.vertexData->vertexDeclaration->addElement(0,0,VET_FLOAT3, VES_POSITION);
		ushort origPosBind =
			vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION)->getSource();
		mPositionBuffer = vertexData->vertexBufferBinding->getBuffer(origPosBind);
		mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mPositionBuffer);
		// Map in w-coord buffer (if present)
		if(!vertexData->hardwareShadowVolWBuffer.isNull())
		{
			mRenderOp.vertexData->vertexDeclaration->addElement(1,0,VET_FLOAT1, VES_TEXTURE_COORDINATES, 0);
			mWBuffer = vertexData->hardwareShadowVolWBuffer;
			mRenderOp.vertexData->vertexBufferBinding->setBinding(1, mWBuffer);
		}
		// Use same vertex start as input
		mRenderOp.vertexData->vertexStart = vertexData->vertexStart;

		if (isLightCap)
		{
			// Use original vertex count, no extrusion
			mRenderOp.vertexData->vertexCount = vertexData->vertexCount;
		}
		else
		{
			// Vertex count must take into account the doubling of the buffer,
			// because second half of the buffer is the extruded copy
			mRenderOp.vertexData->vertexCount =
				vertexData->vertexCount * 2;
			if (createSeparateLightCap)
			{
				// Create child light cap
				mLightCap = OGRE_NEW LODShadowRenderable(parent,
					indexBuffer, vertexData, false, true);
			}
		}
	}
	//--------------------------------------------------------------------------
	StaticGeometry::LODBucket::LODShadowRenderable::~LODShadowRenderable()
	{
		OGRE_DELETE mRenderOp.indexData;
		OGRE_DELETE mRenderOp.vertexData;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::LODBucket::LODShadowRenderable::getWorldTransforms(
		Matrix4* xform) const
	{
		// pretransformed
		*xform = mParent->getParent()->_getParentNodeFullTransform();
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::LODBucket::LODBucket(Region* parent, unsigned short lod,
		Real lodValue)
		: mParent(parent), mLod(lod), mLodValue(lodValue), mEdgeList(0)
		, mVertexProgramInUse(false)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::LODBucket::~LODBucket()
	{
		OGRE_DELETE mEdgeList;
		for (ShadowCaster::ShadowRenderableList::iterator s = mShadowRenderables.begin();
			s != mShadowRenderables.end(); ++s)
		{
			OGRE_DELETE *s;
		}
		mShadowRenderables.clear();
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

		// no need to delete queued meshes, these are managed in StaticGeometry
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::LODBucket::assign(QueuedSubMesh* qmesh, ushort atLod)
	{
		QueuedGeometry* q = OGRE_NEW QueuedGeometry();
		mQueuedGeometryList.push_back(q);
		q->position = qmesh->position;
		q->orientation = qmesh->orientation;
		q->scale = qmesh->scale;
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
	void StaticGeometry::LODBucket::build(bool stencilShadows)
	{

		EdgeListBuilder eb;
		size_t vertexSet = 0;

		// Just pass this on to child buckets
		for (MaterialBucketMap::iterator i = mMaterialBucketMap.begin();
			i != mMaterialBucketMap.end(); ++i)
		{
			MaterialBucket* mat = i->second;

			mat->build(stencilShadows);

			if (stencilShadows)
			{
				MaterialBucket::GeometryIterator geomIt =
					mat->getGeometryIterator();
				// Check if we have vertex programs here
				Technique* t = mat->getMaterial()->getBestTechnique();
				if (t)
				{
					Pass* p = t->getPass(0);
					if (p)
					{
						if (p->hasVertexProgram())
						{
							mVertexProgramInUse = true;
		}
	}
				}

				while (geomIt.hasMoreElements())
				{
					GeometryBucket* geom = geomIt.getNext();

					// Check we're dealing with 16-bit indexes here
					// Since stencil shadows can only deal with 16-bit
					// More than that and stencil is probably too CPU-heavy
					// in any case
					assert(geom->getIndexData()->indexBuffer->getType()
						== HardwareIndexBuffer::IT_16BIT &&
						"Only 16-bit indexes allowed when using stencil shadows");
					eb.addVertexData(geom->getVertexData());
					eb.addIndexData(geom->getIndexData(), vertexSet++);
				}

			}
		}

		if (stencilShadows)
		{
			mEdgeList = eb.build();
		}
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::LODBucket::addRenderables(RenderQueue* queue,
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
	//--------------------------------------------------------------------------
	StaticGeometry::LODBucket::MaterialIterator
	StaticGeometry::LODBucket::getMaterialIterator(void)
	{
		return MaterialIterator(
			mMaterialBucketMap.begin(), mMaterialBucketMap.end());
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::LODBucket::dump(std::ofstream& of) const
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
	//---------------------------------------------------------------------
	void StaticGeometry::LODBucket::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		for (MaterialBucketMap::const_iterator i = mMaterialBucketMap.begin();
			i != mMaterialBucketMap.end(); ++i)
		{
			i->second->visitRenderables(visitor, debugRenderables);
		}

	}
	//---------------------------------------------------------------------
	void StaticGeometry::LODBucket::updateShadowRenderables(
		ShadowTechnique shadowTechnique, const Vector4& lightPos, 
		HardwareIndexBufferSharedPtr* indexBuffer, bool extrude, 
		Real extrusionDistance, unsigned long flags /* = 0  */)
	{
		assert(indexBuffer && "Only external index buffers are supported right now");
		assert((*indexBuffer)->getType() == HardwareIndexBuffer::IT_16BIT &&
			"Only 16-bit indexes supported for now");

		// We need to search the edge list for silhouette edges
		if (!mEdgeList)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"You enabled stencil shadows after the buid process!",
				"StaticGeometry::LODBucket::getShadowVolumeRenderableIterator");
		}

		// Init shadow renderable list if required
		bool init = mShadowRenderables.empty();

		EdgeData::EdgeGroupList::iterator egi;
		ShadowCaster::ShadowRenderableList::iterator si, siend;
		LODShadowRenderable* esr = 0;
		if (init)
			mShadowRenderables.resize(mEdgeList->edgeGroups.size());

		//bool updatedSharedGeomNormals = false;
		siend = mShadowRenderables.end();
		egi = mEdgeList->edgeGroups.begin();
		for (si = mShadowRenderables.begin(); si != siend; ++si, ++egi)
		{
			if (init)
			{
				// Create a new renderable, create a separate light cap if
				// we're using a vertex program (either for this model, or
				// for extruding the shadow volume) since otherwise we can
				// get depth-fighting on the light cap

				*si = OGRE_NEW LODShadowRenderable(this, indexBuffer,
					egi->vertexData, mVertexProgramInUse || !extrude);
			}
			// Get shadow renderable
			esr = static_cast<LODShadowRenderable*>(*si);
			HardwareVertexBufferSharedPtr esrPositionBuffer = esr->getPositionBuffer();
			// Extrude vertices in software if required
			if (extrude)
			{
				mParent->extrudeVertices(esrPositionBuffer,
					egi->vertexData->vertexCount,
					lightPos, extrusionDistance);

			}

		}

	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::MaterialBucket::MaterialBucket(LODBucket* parent,
		const String& materialName)
		: mParent(parent)
		, mMaterialName(materialName)
		, mTechnique(0)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::MaterialBucket::~MaterialBucket()
	{
		// delete
		for (GeometryBucketList::iterator i = mGeometryBucketList.begin();
			i != mGeometryBucketList.end(); ++i)
		{
			OGRE_DELETE *i;
		}
		mGeometryBucketList.clear();

		// no need to delete queued meshes, these are managed in StaticGeometry
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::assign(QueuedGeometry* qgeom)
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
					"StaticGeometry::MaterialBucket::assign");
			}
		}
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::build(bool stencilShadows)
	{
		mMaterial = MaterialManager::getSingleton().getByName(mMaterialName);
		if (mMaterial.isNull())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
				"Material '" + mMaterialName + "' not found.",
				"StaticGeometry::MaterialBucket::build");
		}
		mMaterial->load();
		// tell the geometry buckets to build
		for (GeometryBucketList::iterator i = mGeometryBucketList.begin();
			i != mGeometryBucketList.end(); ++i)
		{
			(*i)->build(stencilShadows);
		}
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::addRenderables(RenderQueue* queue,
		uint8 group, Real lodValue)
	{
        // Get region
        Region *region = mParent->getParent();

        // Get material lod strategy
        const LodStrategy *materialLodStrategy = mMaterial->getLodStrategy();

        // If material strategy doesn't match, recompute lod value with correct strategy
        if (materialLodStrategy != region->mLodStrategy)
            lodValue = materialLodStrategy->getValue(region, region->mCamera);

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
	//--------------------------------------------------------------------------
	String StaticGeometry::MaterialBucket::getGeometryFormatString(
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
	StaticGeometry::MaterialBucket::GeometryIterator
	StaticGeometry::MaterialBucket::getGeometryIterator(void)
	{
		return GeometryIterator(
			mGeometryBucketList.begin(), mGeometryBucketList.end());
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::dump(std::ofstream& of) const
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
	//---------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		for (GeometryBucketList::const_iterator i = mGeometryBucketList.begin();
			i != mGeometryBucketList.end(); ++i)
		{
			visitor->visit(*i, mParent->getLod(), false);
		}

	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::GeometryBucket::GeometryBucket(MaterialBucket* parent,
		const String& formatString, const VertexData* vData,
		const IndexData* iData)
		: Renderable(), mParent(parent), mFormatString(formatString)
	{
		// Clone the structure from the example
		mVertexData = vData->clone(false);
		mIndexData = iData->clone(false);
		mVertexData->vertexCount = 0;
		mVertexData->vertexStart = 0;
		mIndexData->indexCount = 0;
		mIndexData->indexStart = 0;
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

		// Check to see if we have blend indices / blend weights
		// remove them if so, they can try to blend non-existent bones!
		const VertexElement* blendIndices =
			mVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
		const VertexElement* blendWeights =
			mVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
		if (blendIndices && blendWeights)
		{
			assert(blendIndices->getSource() == blendWeights->getSource()
				&& "Blend indices and weights should be in the same buffer");
			// Get the source
			ushort source = blendIndices->getSource();
			assert(blendIndices->getSize() + blendWeights->getSize() ==
				mVertexData->vertexBufferBinding->getBuffer(source)->getVertexSize()
				&& "Blend indices and blend buffers should have buffer to themselves!");
			// Unset the buffer
			mVertexData->vertexBufferBinding->unsetBinding(source);
			// Remove the elements
			mVertexData->vertexDeclaration->removeElement(VES_BLEND_INDICES);
			mVertexData->vertexDeclaration->removeElement(VES_BLEND_WEIGHTS);
            // Close gaps in bindings for effective and safely
            mVertexData->closeGapsInBindings();
		}


	}
	//--------------------------------------------------------------------------
	StaticGeometry::GeometryBucket::~GeometryBucket()
	{
		OGRE_DELETE mVertexData;
		OGRE_DELETE mIndexData;
	}
	//--------------------------------------------------------------------------
	const MaterialPtr& StaticGeometry::GeometryBucket::getMaterial(void) const
	{
		return mParent->getMaterial();
	}
	//--------------------------------------------------------------------------
	Technique* StaticGeometry::GeometryBucket::getTechnique(void) const
	{
		return mParent->getCurrentTechnique();
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::getRenderOperation(RenderOperation& op)
	{
		op.indexData = mIndexData;
		op.operationType = RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = mVertexData;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::getWorldTransforms(Matrix4* xform) const
	{
		// Should be the identity transform, but lets allow transformation of the
		// nodes the regions are attached to for kicks
		*xform = mParent->getParent()->getParent()->_getParentNodeFullTransform();
	}
	//--------------------------------------------------------------------------
	Real StaticGeometry::GeometryBucket::getSquaredViewDepth(const Camera* cam) const
	{
        const Region *region = mParent->getParent()->getParent();
        if (cam == region->mCamera)
            return region->mSquaredViewDepth;
        else
            return region->getParentNode()->getSquaredViewDepth(cam->getLodCamera());
	}
	//--------------------------------------------------------------------------
	const LightList& StaticGeometry::GeometryBucket::getLights(void) const
	{
		return mParent->getParent()->getParent()->queryLights();
	}
	//--------------------------------------------------------------------------
	bool StaticGeometry::GeometryBucket::getCastsShadows(void) const
	{
		return mParent->getParent()->getParent()->getCastShadows();
	}
	//--------------------------------------------------------------------------
	bool StaticGeometry::GeometryBucket::assign(QueuedGeometry* qgeom)
	{
		// Do we have enough space?
		// -2 first to avoid overflow (-1 to adjust count to index, -1 to ensure
		// no overflow at 32 bits and use >= instead of >)
		if ((mVertexData->vertexCount - 2 + qgeom->geometry->vertexData->vertexCount)
			>= mMaxVertexIndex)
		{
			return false;
		}

		mQueuedGeometry.push_back(qgeom);
		mVertexData->vertexCount += qgeom->geometry->vertexData->vertexCount;
		mIndexData->indexCount += qgeom->geometry->indexData->indexCount;

		return true;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::build(bool stencilShadows)
	{
		// Ok, here's where we transfer the vertices and indexes to the shared
		// buffers
		// Shortcuts
		VertexDeclaration* dcl = mVertexData->vertexDeclaration;
		VertexBufferBinding* binds = mVertexData->vertexBufferBinding;

		// create index buffer, and lock
		mIndexData->indexBuffer = HardwareBufferManager::getSingleton()
			.createIndexBuffer(mIndexType, mIndexData->indexCount,
				HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		uint32* p32Dest = 0;
		uint16* p16Dest = 0;
		if (mIndexType == HardwareIndexBuffer::IT_32BIT)
		{
			p32Dest = static_cast<uint32*>(
				mIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		}
		else
		{
			p16Dest = static_cast<uint16*>(
				mIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
		}
		// create all vertex buffers, and lock
		ushort b;
		ushort posBufferIdx = dcl->findElementBySemantic(VES_POSITION)->getSource();

		vector<uchar*>::type destBufferLocks;
		vector<VertexDeclaration::VertexElementList>::type bufferElements;
		for (b = 0; b < binds->getBufferCount(); ++b)
		{
			size_t vertexCount = mVertexData->vertexCount;
			// Need to double the vertex count for the position buffer
			// if we're doing stencil shadows
			if (stencilShadows && b == posBufferIdx)
			{
				vertexCount = vertexCount * 2;
				assert(vertexCount <= mMaxVertexIndex &&
					"Index range exceeded when using stencil shadows, consider "
					"reducing your region size or reducing poly count");
			}
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
		}


		// Iterate over the geometry items
		size_t indexOffset = 0;
		QueuedGeometryList::iterator gi, giend;
		giend = mQueuedGeometry.end();
		Vector3 regionCentre = mParent->getParent()->getParent()->getCentre();
		for (gi = mQueuedGeometry.begin(); gi != giend; ++gi)
		{
			QueuedGeometry* geom = *gi;
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
					// Iterate over vertex elements
					VertexDeclaration::VertexElementList& elems =
						bufferElements[b];
					VertexDeclaration::VertexElementList::iterator ei;
					for (ei = elems.begin(); ei != elems.end(); ++ei)
					{
						VertexElement& elem = *ei;
						elem.baseVertexPointerToElement(pSrcBase, &pSrcReal);
						elem.baseVertexPointerToElement(pDstBase, &pDstReal);
						switch (elem.getSemantic())
						{
						case VES_POSITION:
							tmp.x = *pSrcReal++;
							tmp.y = *pSrcReal++;
							tmp.z = *pSrcReal++;
							// transform
							tmp = (geom->orientation * (tmp * geom->scale)) +
								geom->position;
							// Adjust for region centre
							tmp -= regionCentre;
							*pDstReal++ = tmp.x;
							*pDstReal++ = tmp.y;
							*pDstReal++ = tmp.z;
							break;
						case VES_NORMAL:
						case VES_TANGENT:
						case VES_BINORMAL:
							tmp.x = *pSrcReal++;
							tmp.y = *pSrcReal++;
							tmp.z = *pSrcReal++;
							// scale (invert)
							tmp = tmp / geom->scale;
							tmp.normalise();
							// rotation
							tmp = geom->orientation * tmp;
							*pDstReal++ = tmp.x;
							*pDstReal++ = tmp.y;
							*pDstReal++ = tmp.z;
							// copy parity for tangent.
							if (elem.getType() == Ogre::VET_FLOAT4)
								*pDstReal = *pSrcReal;
							break;
						default:
							// just raw copy
							memcpy(pDstReal, pSrcReal,
									VertexElement::getTypeSize(elem.getType()));
							break;
						};

					}

					// Increment both pointers
					pDstBase += bufInc;
					pSrcBase += bufInc;

				}

				// Update pointer
				destBufferLocks[b] = pDstBase;
				srcBuf->unlock();
			}

			indexOffset += geom->geometry->vertexData->vertexCount;
		}

		// Unlock everything
		mIndexData->indexBuffer->unlock();
		for (b = 0; b < binds->getBufferCount(); ++b)
		{
			binds->getBuffer(b)->unlock();
		}

		// If we're dealing with stencil shadows, copy the position data from
		// the early half of the buffer to the latter part
		if (stencilShadows)
		{
			HardwareVertexBufferSharedPtr buf = binds->getBuffer(posBufferIdx);
			void* pSrc = buf->lock(HardwareBuffer::HBL_NORMAL);
			// Point dest at second half (remember vertexcount is original count)
			void* pDest = static_cast<uchar*>(pSrc) +
				buf->getVertexSize() * mVertexData->vertexCount;
			memcpy(pDest, pSrc, buf->getVertexSize() * mVertexData->vertexCount);
			buf->unlock();

			// Also set up hardware W buffer if appropriate
			RenderSystem* rend = Root::getSingleton().getRenderSystem();
			if (rend && rend->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
			{
				buf = HardwareBufferManager::getSingleton().createVertexBuffer(
					sizeof(float), mVertexData->vertexCount * 2,
					HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
				// Fill the first half with 1.0, second half with 0.0
				float *pW = static_cast<float*>(
					buf->lock(HardwareBuffer::HBL_DISCARD));
				size_t v;
				for (v = 0; v < mVertexData->vertexCount; ++v)
				{
					*pW++ = 1.0f;
				}
				for (v = 0; v < mVertexData->vertexCount; ++v)
				{
					*pW++ = 0.0f;
				}
				buf->unlock();
				mVertexData->hardwareShadowVolWBuffer = buf;
			}
		}

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::dump(std::ofstream& of) const
	{
		of << "Geometry Bucket" << std::endl;
		of << "---------------" << std::endl;
		of << "Format string: " << mFormatString << std::endl;
		of << "Geometry items: " << mQueuedGeometry.size() << std::endl;
		of << "Vertex count: " << mVertexData->vertexCount << std::endl;
		of << "Index count: " << mIndexData->indexCount << std::endl;
		of << "---------------" << std::endl;

	}
	//--------------------------------------------------------------------------

}

