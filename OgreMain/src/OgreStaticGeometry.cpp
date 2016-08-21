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
#include "OgreLodStrategyManager.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {
namespace v1 {
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
        mRenderQueueID(0),
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
            // Calculate the region centre
            Vector3 centre = getRegionCentre(x, y, z);
            ret = OGRE_NEW Region( Id::generateNewId<MovableObject>(),
                                   &mOwner->_getEntityMemoryManager( SCENE_STATIC ),
                                   this, mOwner, index, centre );
            mOwner->injectMovableObject(ret);
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
        if (msh->hasManualLodLevel())
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
            q->materialName = se->getMaterial()->getName();
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
        ushort numLods = sm->parent->hasManualLodLevel() ? 1 :
            sm->parent->getNumLodLevels();
        lodList->resize(numLods);
        for (ushort lod = 0; lod < numLods; ++lod)
        {
            SubMeshLodGeometryLink& geomLink = (*lodList)[lod];
            IndexData *lodIndexData;
            if (lod == 0)
            {
                lodIndexData = sm->indexData[VpNormal];
            }
            else
            {
                lodIndexData = sm->mLodFaceList[VpNormal][lod - 1];
            }
            // Can use the original mesh geometry?
            if (sm->useSharedVertices)
            {
                if (sm->parent->getNumSubMeshes() == 1)
                {
                    // Ok, this is actually our own anyway
                    geomLink.vertexData = sm->parent->sharedVertexData[VpNormal];
                    geomLink.indexData = lodIndexData;
                }
                else
                {
                    // We have to split it
                    splitGeometry(sm->parent->sharedVertexData[VpNormal],
                        lodIndexData, &geomLink);
                }
            }
            else
            {
                if (lod == 0)
                {
                    // Ok, we can use the existing geometry; should be in full
                    // use by just this SubMesh
                    geomLink.vertexData = sm->vertexData[VpNormal];
                    geomLink.indexData = sm->indexData[VpNormal];
                }
                else
                {
                    // We have to split it
                    splitGeometry(sm->vertexData[VpNormal],
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
        IndexRemap indexRemap;
        if (use32bitIndexes)
        {
            uint32 *p32 = static_cast<uint32*>(id->indexBuffer->lock(
                id->indexStart * id->indexBuffer->getIndexSize(), 
                id->indexCount * id->indexBuffer->getIndexSize(), 
                HardwareBuffer::HBL_READ_ONLY));
            buildIndexRemap(p32, id->indexCount, indexRemap);
            id->indexBuffer->unlock();
        }
        else
        {
            uint16 *p16 = static_cast<uint16*>(id->indexBuffer->lock(
                id->indexStart * id->indexBuffer->getIndexSize(), 
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
                id->indexStart * id->indexBuffer->getIndexSize(), 
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
                id->indexStart * id->indexBuffer->getIndexSize(), 
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
        SceneNode::ConstNodeVecIterator nodei = node->getChildIterator();

        while (nodei.hasMoreElements())
        {
            const SceneNode* subNode = static_cast<const SceneNode*>(nodei.getNext());
            // Add this subnode and its children...
            addSceneNode( subNode );
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

        // Now tell each region to build itself
        for (RegionMap::iterator ri = mRegionMap.begin();
            ri != mRegionMap.end(); ++ri)
        {
            ri->second->build( mVisible );
            
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
    //--------------------------------------------------------------------------
    StaticGeometry::RegionIterator StaticGeometry::getRegionIterator(void)
    {
        return RegionIterator(mRegionMap.begin(), mRegionMap.end());
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    StaticGeometry::Region::Region( IdType id, ObjectMemoryManager *objectMemoryManager,
                                    StaticGeometry* parent, SceneManager* mgr, uint32 regionID,
                                    const Vector3& centre ) :
        MovableObject( id, objectMemoryManager, 0, 1 ),
        mParent(parent), mSceneMgr(mgr), mNode(0),
        mRegionID(regionID), mCentre(centre)
    {

        mObjectData.mQueryFlags[mObjectData.mIndex] = SceneManager::QUERY_STATICGEOMETRY_DEFAULT_MASK;

        mObjectData.mLocalAabb->setFromAabb( Aabb( Vector3::ZERO,
                                                   Vector3( -std::numeric_limits<Real>::max(),
                                                            -std::numeric_limits<Real>::max(),
                                                            -std::numeric_limits<Real>::max() ) ),
                                             mObjectData.mIndex );
        mLodMesh = &mLodValues;
    }
    //--------------------------------------------------------------------------
    StaticGeometry::Region::~Region()
    {
        if (mNode)
        {
            mNode->getParentSceneNode()->removeAndDestroyChild(mNode);
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
    void StaticGeometry::Region::assign(QueuedSubMesh* qmesh)
    {
        mQueuedSubMeshes.push_back(qmesh);

        // Set/check LOD strategy
        const LodStrategy *lodStrategy = LodStrategyManager::getSingleton().getDefaultStrategy();
        // First LOD mandatory, and always from base LOD value
        mLodValues.push_back(lodStrategy->getBaseValue());

        // update LOD values
        ushort lodLevels = qmesh->submesh->parent->getNumLodLevels();
        assert(qmesh->geometryLodList->size() == lodLevels);

        while(mLodValues.size() < lodLevels)
        {
            mLodValues.push_back(0.0f);
        }
        // Make sure LOD levels are max of all at the requested level
        for (ushort lod = 1; lod < lodLevels; ++lod)
        {
            const MeshLodUsage& meshLod = qmesh->submesh->parent->getLodLevel(lod);
            mLodValues[lod] = Ogre::max(mLodValues[lod], meshLod.value);
        }

        // update bounds
        // Transform world bounds relative to our centre
        Aabb localAabb = Aabb::newFromExtents( qmesh->worldBounds.getMinimum() - mCentre,
                                               qmesh->worldBounds.getMaximum() - mCentre );
        Aabb regionAabb = mObjectData.mLocalAabb->getAsAabb( mObjectData.mIndex );
        regionAabb.merge( localAabb );
        mObjectData.mLocalAabb->setFromAabb( regionAabb, mObjectData.mIndex );
        mObjectData.mLocalRadius[mObjectData.mIndex] = regionAabb.getRadius();
    }
    //--------------------------------------------------------------------------
    void StaticGeometry::Region::build( bool parentVisible )
    {
        // Create a node
        mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_STATIC, mCentre);
        mNode->attachObject(this);
        this->setVisible( parentVisible );
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
    const String& StaticGeometry::Region::getMovableType(void) const
    {
        static String sType = "StaticGeometry";
        return sType;
    }
    //--------------------------------------------------------------------------
    /*void StaticGeometry::Region::_notifyCurrentCamera(Camera* cam)
    {
        const LodStrategy *lodStrategy = LodStrategyManager::getSingleton().getDefaultStrategy();

        // Sanity check
        assert(!mLodValues.empty());

        // Calculate LOD value
        Real lodValue = lodStrategy->getValue(this, cam);

        // Store LOD value for this strategy
        mLodValue = lodValue;
    }*/
    //--------------------------------------------------------------------------
    void StaticGeometry::Region::_updateRenderQueue(RenderQueue* queue, Camera *camera, const Camera *lodCamera)
    {
        // Cache squared view depth for use by GeometryBucket
        mCamera = lodCamera;
        mSquaredViewDepth = mParentNode->getSquaredViewDepth( lodCamera );
        mLodBucketList[mCurrentMeshLod]->addRenderables(queue, mRenderQueueID, 0);
    }
    //--------------------------------------------------------------------------
    StaticGeometry::Region::LODIterator
    StaticGeometry::Region::getLODIterator(void)
    {
        return LODIterator(mLodBucketList.begin(), mLodBucketList.end());
    }
    //--------------------------------------------------------------------------
    EdgeData* StaticGeometry::Region::getEdgeList(void)
    {
        return mLodBucketList[mCurrentMeshLod]->getEdgeList();
    }
    //--------------------------------------------------------------------------
    bool StaticGeometry::Region::hasEdgeList(void)
    {
        return getEdgeList() != 0;
    }
    //--------------------------------------------------------------------------
    void StaticGeometry::Region::dump(std::ofstream& of) const
    {
        Aabb localAabb = mObjectData.mLocalAabb->getAsAabb( mObjectData.mIndex );
        AxisAlignedBox localBox( localAabb.getMinimum(), localAabb.getMaximum() );

        of << "Region " << mRegionID << std::endl;
        of << "--------------------------" << std::endl;
        of << "Centre: " << mCentre << std::endl;
        of << "Local AABB: " << localBox << std::endl;
        of << "Bounding radius: " << mObjectData.mLocalRadius[mObjectData.mIndex] << std::endl;
        of << "Number of LODs: " << mLodBucketList.size() << std::endl;

        for (LODBucketList::const_iterator i = mLodBucketList.begin();
            i != mLodBucketList.end(); ++i)
        {
            (*i)->dump(of);
        }
        of << "--------------------------" << std::endl;
    }
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
    void StaticGeometry::LODBucket::build()
    {

        EdgeListBuilder eb;

        // Just pass this on to child buckets
        for (MaterialBucketMap::iterator i = mMaterialBucketMap.begin();
            i != mMaterialBucketMap.end(); ++i)
        {
            MaterialBucket* mat = i->second;

            mat->build();
        }
    }
    //--------------------------------------------------------------------------
    void StaticGeometry::LODBucket::addRenderables(RenderQueue* queue,
        uint8 group, const FastArray<unsigned char> &currentMatLod )
    {
        // Just pass this on to child buckets
        MaterialBucketMap::iterator i, iend;
        iend =  mMaterialBucketMap.end();
        FastArray<unsigned char>::const_iterator itMatLod = currentMatLod.begin();
        for (i = mMaterialBucketMap.begin(); i != iend; ++i)
        {
            assert( itMatLod < currentMatLod.end() );
            i->second->addRenderables(queue, group, *itMatLod++);
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
        of << "LOD Value: " << mLodValue << std::endl;
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
    void StaticGeometry::MaterialBucket::build()
    {
        mTechnique = 0;
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
            (*i)->build();
        }
    }
    //--------------------------------------------------------------------------
    void StaticGeometry::MaterialBucket::addRenderables(RenderQueue* queue, uint8 group, size_t materialLod)
    {
        // Get region
        Region *region = mParent->getParent();

        // Determine the current material technique
        mTechnique = mMaterial->getBestTechnique( 0 );
        GeometryBucketList::iterator i, iend;
        iend =  mGeometryBucketList.end();
        //TODO: RENDER QUEUE
        //TODO: mCurrentMeshLod
        /*for (i = mGeometryBucketList.begin(); i != iend; ++i)
        {
            queue->addRenderable(*i, group);
        }*/

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
		StringStream str;

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
    void StaticGeometry::GeometryBucket::getRenderOperation(RenderOperation& op, bool casterPass)
    {
        op.indexData = mIndexData;
        op.operationType = OT_TRIANGLE_LIST;
#if OGRE_DEBUG_MODE
        op.srcRenderable = this;
#endif
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
    void StaticGeometry::GeometryBucket::build()
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

        vector<uchar*>::type destBufferLocks;
        vector<VertexDeclaration::VertexElementList>::type bufferElements;
        for (b = 0; b < binds->getBufferCount(); ++b)
        {
            size_t vertexCount = mVertexData->vertexCount;
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
                        srcIdxData->indexStart * srcIdxData->indexBuffer->getIndexSize(), 
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
                    srcIdxData->indexStart * srcIdxData->indexBuffer->getIndexSize(), 
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
}

