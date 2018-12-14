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
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTerrain.h"
#include "OgreVertexIndexData.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreCamera.h"
#include "OgreStreamSerialiser.h"
#include "OgreSceneNode.h"
#include "OgreLodStrategy.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif

namespace Ogre
{
    unsigned short TerrainQuadTreeNode::POSITION_BUFFER = 0;
    unsigned short TerrainQuadTreeNode::DELTA_BUFFER = 1;

    //---------------------------------------------------------------------
    TerrainQuadTreeNode::TerrainQuadTreeNode(Terrain* terrain, 
        TerrainQuadTreeNode* parent, uint16 xoff, uint16 yoff, uint16 size, 
        uint16 lod, uint16 depth, uint16 quadrant)
        : mTerrain(terrain)
        , mParent(parent)
        , mOffsetX(xoff)
        , mOffsetY(yoff)
        , mBoundaryX(xoff + size)
        , mBoundaryY(yoff + size)
        , mSize(size)
        , mBaseLod(lod)
        , mDepth(depth)
        , mQuadrant(quadrant)
        , mBoundingRadius(0)
        , mCurrentLod(-1)
        , mMaterialLodIndex(0)
        , mLodTransition(0)
        , mChildWithMaxHeightDelta(0)
        , mSelfOrChildRendered(false)
        , mNodeWithVertexData(0)
        , mVertexDataRecord(0)
        , mMovable(0)
        , mLocalNode(0)
        , mRend(0)
    {
        if (terrain->getMaxBatchSize() < size)
        {
            uint16 childSize = (uint16)(((size - 1) * 0.5f) + 1);
            uint16 childOff = childSize - 1;
            uint16 childLod = lod - 1; // LOD levels decrease down the tree (higher detail)
            uint16 childDepth = depth + 1;
            // create children
            mChildren[0] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff, childSize, childLod, childDepth, 0);
            mChildren[1] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff, childSize, childLod, childDepth, 1);
            mChildren[2] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff, yoff + childOff, childSize, childLod, childDepth, 2);
            mChildren[3] = OGRE_NEW TerrainQuadTreeNode(terrain, this, xoff + childOff, yoff + childOff, childSize, childLod, childDepth, 3);

            LodLevel* ll = OGRE_NEW LodLevel();
            // non-leaf nodes always render with minBatchSize vertices
            ll->batchSize = terrain->getMinBatchSize();
            ll->maxHeightDelta = 0;
            ll->calcMaxHeightDelta = 0;
            mLodLevels.push_back(ll);

        }
        else
        {
            // No children
            memset(mChildren, 0, sizeof(TerrainQuadTreeNode*) * 4);

            // this is a leaf node and may have internal LODs of its own
            uint16 ownLod = terrain->getNumLodLevelsPerLeaf();
            assert (lod == (ownLod - 1) && "The lod passed in should reflect the number of "
                "lods in a leaf");
            // leaf nodes always have a base LOD of 0, because they're always handling
            // the highest level of detail
            mBaseLod = 0;
            // leaf nodes render from max batch size to min batch size
            uint16 sz = terrain->getMaxBatchSize();

            while (ownLod--)
            {
                LodLevel* ll = OGRE_NEW LodLevel();
                ll->batchSize = sz;
                ll->maxHeightDelta = 0;
                ll->calcMaxHeightDelta = 0;
                mLodLevels.push_back(ll);
                if (ownLod)
                    sz = (uint16)(((sz - 1) * 0.5) + 1);
            }
            
            assert(sz == terrain->getMinBatchSize());

        }
        
        // Local centre calculation
        // Because of pow2 + 1 there is always a middle point
        uint16 midoffset = (size - 1) / 2;
        uint16 midpointx = mOffsetX + midoffset;
        uint16 midpointy = mOffsetY + midoffset;
        // derive the local centre, but give it a height of 0
        // TODO - what if we actually centred this at the terrain height at this point?
        // would this be better?
        mTerrain->getPoint(midpointx, midpointy, 0, &mLocalCentre);

        mMovable = OGRE_NEW Movable(this);
        mRend = OGRE_NEW Rend(this);

    
    }
    //---------------------------------------------------------------------
    TerrainQuadTreeNode::~TerrainQuadTreeNode()
    {
        if (mMovable->isAttached())
            mLocalNode->detachObject(mMovable);
        OGRE_DELETE mMovable;
        mMovable = 0;
        OGRE_DELETE mRend;
        mRend = 0;

        if (mLocalNode)
        {
            mTerrain->_getRootSceneNode()->removeAndDestroyChild(mLocalNode);
            mLocalNode = 0;
        }

        for (int i = 0; i < 4; ++i)
            OGRE_DELETE mChildren[i];

        destroyCpuVertexData();
        destroyGpuVertexData();
        destroyGpuIndexData();

        for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
            OGRE_DELETE *i;

        OGRE_DELETE mVertexDataRecord;
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::isLeaf() const
    {
        return mChildren[0] == 0;
    }
    //---------------------------------------------------------------------
    TerrainQuadTreeNode* TerrainQuadTreeNode::getChild(unsigned short child) const
    {
        if (isLeaf() || child >= 4)
            return 0;

        return mChildren[child];
    }
    //---------------------------------------------------------------------
    TerrainQuadTreeNode* TerrainQuadTreeNode::getParent() const
    {
        return mParent;
    }
    //---------------------------------------------------------------------
    Terrain* TerrainQuadTreeNode::getTerrain() const
    {
        return mTerrain;
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::prepare()
    {
        if (!isLeaf())
        {
            for (int i = 0; i < 4; ++i)
                mChildren[i]->prepare();
        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::prepare(StreamSerialiser& stream)
    {
        // load LOD data we need
        for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
        {
            LodLevel* ll = *i;
            // only read 'calc' and then copy to final (separation is only for
            // real-time calculation
            // Basically this is what finaliseHeightDeltas does in calc path
            stream.read(&ll->calcMaxHeightDelta);
            ll->maxHeightDelta = ll->calcMaxHeightDelta;
            ll->lastCFactor = 0;
        }

        if (!isLeaf())
        {
            for (int i = 0; i < 4; ++i)
                mChildren[i]->prepare(stream);
        }

        // If this is the root, do the post delta calc to finish
        if (!mParent)
        {
            Rect rect;
            rect.top = mOffsetY; rect.bottom = mBoundaryY;
            rect.left = mOffsetX; rect.right = mBoundaryX;
            postDeltaCalculation(rect);
        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::save(StreamSerialiser& stream)
    {
        // save LOD data we need
        for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
        {
            LodLevel* ll = *i;
            stream.write(&ll->maxHeightDelta);
        }

        if (!isLeaf())
        {
            for (int i = 0; i < 4; ++i)
                mChildren[i]->save(stream);
        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::load()
    {
        loadSelf();

        if (!isLeaf())
            for (int i = 0; i < 4; ++i)
                mChildren[i]->load();
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::load(uint16 treeDepthStart, uint16 treeDepthEnd)
    {
        if (mDepth >= treeDepthEnd)
            return ;

        if (mDepth >= treeDepthStart && mDepth < treeDepthEnd && mNodeWithVertexData)
            loadSelf();

        if (!isLeaf())
            for (int i = 0; i < 4; ++i)
                mChildren[i]->load(treeDepthStart, treeDepthEnd);
    }
    void TerrainQuadTreeNode::loadSelf()
    {
        createGpuVertexData();
        createGpuIndexData();
        if (!mLocalNode)
            mLocalNode = mTerrain->_getRootSceneNode()->createChildSceneNode(mLocalCentre);

        if (!mMovable->isAttached())
            mLocalNode->attachObject(mMovable);
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::unload()
    {
        if (!isLeaf())
            for (int i = 0; i < 4; ++i)
                mChildren[i]->unload();

        destroyGpuVertexData();

        if (mMovable->isAttached())
            mLocalNode->detachObject(mMovable);
    }

    void TerrainQuadTreeNode::unload(uint16 treeDepthStart, uint16 treeDepthEnd)
    {
        if (mDepth >= treeDepthEnd)
            return ;

        if (!isLeaf())
            for (int i = 0; i < 4; ++i)
                mChildren[i]->unload(treeDepthStart, treeDepthEnd);

        if (mDepth >= treeDepthStart && mDepth < treeDepthEnd)
        {
            destroyGpuVertexData();
            if (mMovable->isAttached())
                mLocalNode->detachObject(mMovable);

        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::unprepare()
    {
        if (!isLeaf())
            for (int i = 0; i < 4; ++i)
                mChildren[i]->unprepare();

        destroyCpuVertexData();
    }
    //---------------------------------------------------------------------
    uint16 TerrainQuadTreeNode::getLodCount() const
    {
        return static_cast<uint16>(mLodLevels.size());
    }
    //---------------------------------------------------------------------
    const TerrainQuadTreeNode::LodLevel* TerrainQuadTreeNode::getLodLevel(uint16 lod)
    {
        assert(lod < mLodLevels.size());

        return mLodLevels[lod];
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::preDeltaCalculation(const Rect& rect)
    {
        if (rect.left <= mBoundaryX || rect.right > mOffsetX
            || rect.top <= mBoundaryY || rect.bottom > mOffsetY)
        {
            // relevant to this node (overlaps)

            // if the rect covers the whole node, reset the max height
            // this means that if you recalculate the deltas progressively, end up keeping
            // a max height that's no longer the case (ie more conservative lod), 
            // but that's the price for not recaculating the whole node. If a 
            // complete recalculation is required, just dirty the entire node. (or terrain)

            // Note we use the 'calc' field here to avoid interfering with any
            // ongoing LOD calculations (this can be in the background)

            if (rect.left <= mOffsetX && rect.right > mBoundaryX 
                && rect.top <= mOffsetY && rect.bottom > mBoundaryY)
            {
                for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
                    (*i)->calcMaxHeightDelta = 0.0;
            }

            // pass on to children
            if (!isLeaf())
            {
                for (int i = 0; i < 4; ++i)
                    mChildren[i]->preDeltaCalculation(rect);

            }
        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::notifyDelta(uint16 x, uint16 y, uint16 lod, Real delta)
    {
        if (x >= mOffsetX && x < mBoundaryX 
            && y >= mOffsetY && y < mBoundaryY)
        {
            // within our bounds, check it's our LOD level
            if (lod >= mBaseLod && lod < mBaseLod + mLodLevels.size())
            {
                // Apply the delta to all LODs equal or lower detail to lod
                LodLevelList::iterator i = mLodLevels.begin();
                std::advance(i, lod - mBaseLod);
                (*i)->calcMaxHeightDelta = std::max((*i)->calcMaxHeightDelta, delta);
            }

            // pass on to children
            if (!isLeaf())
            {
                for (int i = 0; i < 4; ++i)
                {
                    mChildren[i]->notifyDelta(x, y, lod, delta);
                                }
            }

        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::postDeltaCalculation(const Rect& rect)
    {
        if (rect.left <= mBoundaryX || rect.right > mOffsetX
            || rect.top <= mBoundaryY || rect.bottom > mOffsetY)
        {
            // relevant to this node (overlaps)

            // each non-leaf node should know which of its children transitions
            // to the lower LOD level last, because this is the one which controls
            // when the parent takes over
            if (!isLeaf())
            {
                Real maxChildDelta = -1;
                TerrainQuadTreeNode* childWithMaxHeightDelta = 0;
                for (int i = 0; i < 4; ++i)
                {
                    TerrainQuadTreeNode* child = mChildren[i];

                    child->postDeltaCalculation(rect);

                    Real childDelta = child->getLodLevel(child->getLodCount()-1)->calcMaxHeightDelta;
                    if (childDelta > maxChildDelta)
                    {
                        childWithMaxHeightDelta = child;
                        maxChildDelta = childDelta;
                    }

                }

                // make sure that our highest delta value is greater than all children's
                // otherwise we could have some crossover problems
                // for a non-leaf, there is only one LOD level
                mLodLevels[0]->calcMaxHeightDelta = std::max(mLodLevels[0]->calcMaxHeightDelta, maxChildDelta * (Real)1.05);
                mChildWithMaxHeightDelta = childWithMaxHeightDelta;

            }
            else
            {
                // make sure own LOD levels delta values ascend
                for (size_t i = 0; i < mLodLevels.size() - 1; ++i)
                {
                    // the next LOD after this one should have a higher delta
                    // otherwise it won't come into affect further back like it should!
                    mLodLevels[i+1]->calcMaxHeightDelta = 
                        std::max(mLodLevels[i+1]->calcMaxHeightDelta, 
                            mLodLevels[i]->calcMaxHeightDelta * (Real)1.05);
                }

            }
        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::finaliseDeltaValues(const Rect& rect)
    {
        if (rect.left <= mBoundaryX || rect.right > mOffsetX
            || rect.top <= mBoundaryY || rect.bottom > mOffsetY)
        {
            // relevant to this node (overlaps)

            // Children
            if (!isLeaf())
            {
                for (int i = 0; i < 4; ++i)
                {
                    TerrainQuadTreeNode* child = mChildren[i];
                    child->finaliseDeltaValues(rect);
                }

            }

            // Self
            for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i)
            {
                // copy from 'calc' area to runtime value
                (*i)->maxHeightDelta = (*i)->calcMaxHeightDelta;
                // also trash stored cfactor
                (*i)->lastCFactor = 0;
            }

        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::assignVertexData(uint16 treeDepthStart, 
        uint16 treeDepthEnd, uint16 resolution, uint sz)
    {
        assert(treeDepthStart >= mDepth && "Should not be calling this");

        if (mDepth == treeDepthStart)
        {
            // we own this vertex data
            mNodeWithVertexData = this;
            if (!mVertexDataRecord)
                mVertexDataRecord = OGRE_NEW VertexDataRecord(resolution, sz, treeDepthEnd - treeDepthStart);

            createCpuVertexData();

            // pass on to children
            if (!isLeaf() && treeDepthEnd > (mDepth + 1)) // treeDepthEnd is exclusive, and this is children
            {
                for (int i = 0; i < 4; ++i)
                    mChildren[i]->useAncestorVertexData(this, treeDepthEnd, resolution);

            }
        }
        else
        {
            assert(!isLeaf() && "No more levels below this!");

            for (int i = 0; i < 4; ++i)
                mChildren[i]->assignVertexData(treeDepthStart, treeDepthEnd, resolution, sz);
            
        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::useAncestorVertexData(TerrainQuadTreeNode* owner, uint16 treeDepthEnd, uint16 resolution)
    {
        mNodeWithVertexData = owner; 
        mVertexDataRecord = 0;

        if (!isLeaf() && treeDepthEnd > (mDepth + 1)) // treeDepthEnd is exclusive, and this is children
        {
            for (int i = 0; i < 4; ++i)
                mChildren[i]->useAncestorVertexData(owner, treeDepthEnd, resolution);

        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::updateVertexData(bool positions, bool deltas, 
        const Rect& rect, bool cpuData)
    {
        if (rect.left <= mBoundaryX || rect.right > mOffsetX
            || rect.top <= mBoundaryY || rect.bottom > mOffsetY)
        {
            // Do we have vertex data?
            if (mVertexDataRecord)
            {
                // Trim to our bounds
                Rect updateRect(mOffsetX, mOffsetY, mBoundaryX, mBoundaryY);
                updateRect.left = std::max(updateRect.left, rect.left);
                updateRect.right = std::min(updateRect.right, rect.right);
                updateRect.top = std::max(updateRect.top, rect.top);
                updateRect.bottom = std::min(updateRect.bottom, rect.bottom);

                // update the GPU buffer directly
                // TODO: do we have no use for CPU vertex data after initial load?
                // if so, destroy it to free RAM, this should be fast enough to 
                // to direct
                HardwareVertexBufferSharedPtr posbuf, deltabuf;
                VertexData* targetVertexData = mVertexDataRecord->cpuVertexData;
                if(!cpuData)
                {
                    if(mVertexDataRecord->gpuVertexData == NULL) 
                        createGpuVertexData();
                    assert(mVertexDataRecord->gpuVertexData);
                    targetVertexData = mVertexDataRecord->gpuVertexData;
                }

                if (positions) 
                    posbuf = targetVertexData->vertexBufferBinding->getBuffer(POSITION_BUFFER);
                if (deltas)
                    deltabuf = targetVertexData->vertexBufferBinding->getBuffer(DELTA_BUFFER);
                updateVertexBuffer(posbuf, deltabuf, updateRect);
            }

            // pass on to children
            if (!isLeaf())
            {
                for (int i = 0; i < 4; ++i)
                {
                    mChildren[i]->updateVertexData(positions, deltas, rect, cpuData);

                    // merge bounds from children
                    AxisAlignedBox childBox = mChildren[i]->getAABB();
                    // this box is relative to child centre
                    Vector3 boxoffset = mChildren[i]->getLocalCentre() - getLocalCentre();
                    childBox.setMinimum(childBox.getMinimum() + boxoffset);
                    childBox.setMaximum(childBox.getMaximum() + boxoffset);
                    mAABB.merge(childBox);
                }

            }
            // Make sure node knows to update
            if (mMovable && mMovable->isAttached())
                mMovable->getParentSceneNode()->needUpdate();



        }

    }
    //---------------------------------------------------------------------
    const TerrainQuadTreeNode::VertexDataRecord* TerrainQuadTreeNode::getVertexDataRecord() const
    {
        return mNodeWithVertexData ? mNodeWithVertexData->mVertexDataRecord : 0;
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::createCpuVertexData()
    {
        if (mVertexDataRecord)
        {
            destroyCpuVertexData();

            // create vertex structure, not using GPU for now (these are CPU structures)
            VertexDeclaration* dcl = OGRE_NEW VertexDeclaration();
            VertexBufferBinding* bufbind = OGRE_NEW VertexBufferBinding();

            mVertexDataRecord->cpuVertexData = OGRE_NEW VertexData(dcl, bufbind);

            // Vertex declaration
            size_t offset = 0;
            
            if (mTerrain->_getUseVertexCompression())
            {
                // 16 bytes per vertex, shaders only
                // POSITION (encoded x/y as indexes from base)
                // short2(x, y)
                offset += dcl->addElement(POSITION_BUFFER, offset, VET_SHORT2, VES_POSITION).getSize();
                // UV0 - height
                // float(height)
                dcl->addElement(POSITION_BUFFER, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 0).getSize();
                // UV1 delta information
                // float2(delta, deltaLODthreshold)
                dcl->addElement(DELTA_BUFFER, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1).getSize();
            }
            else 
            {
                // 28 bytes per vertex, compatibility
                // POSITION 
                // float3(x, y, z)
                offset += dcl->addElement(POSITION_BUFFER, offset, VET_FLOAT3, VES_POSITION).getSize();
                // UV0
                // float2(u, v)
                dcl->addElement(POSITION_BUFFER, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0).getSize();
                // UV1 delta information
                // float2(delta, deltaLODthreshold)
                dcl->addElement(DELTA_BUFFER, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1).getSize();
            }

            // Calculate number of vertices
            // Base geometry size * size
            size_t baseNumVerts = (size_t)Math::Sqr(mVertexDataRecord->size);
            size_t numVerts = baseNumVerts;
            // Now add space for skirts
            // Skirts will be rendered as copies of the edge vertices translated downwards
            // Some people use one big fan with only 3 vertices at the bottom, 
            // but this requires creating them much bigger that necessary, meaning
            // more unnecessary overdraw, so we'll use more vertices 
            // You need 2^levels + 1 rows of full resolution (max 129) vertex copies, plus
            // the same number of columns. There are common vertices at intersections
            uint16 levels = mVertexDataRecord->treeLevels;
            mVertexDataRecord->numSkirtRowsCols = (uint16)(Math::Pow(2, levels) + 1);
            mVertexDataRecord->skirtRowColSkip = (mVertexDataRecord->size - 1) / (mVertexDataRecord->numSkirtRowsCols - 1);
            numVerts += mVertexDataRecord->size * mVertexDataRecord->numSkirtRowsCols;
            numVerts += mVertexDataRecord->size * mVertexDataRecord->numSkirtRowsCols;
            // manually create CPU-side buffer
            HardwareVertexBufferSharedPtr posbuf(
                OGRE_NEW DefaultHardwareVertexBuffer(dcl->getVertexSize(POSITION_BUFFER), numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY));
            HardwareVertexBufferSharedPtr deltabuf(
                OGRE_NEW DefaultHardwareVertexBuffer(dcl->getVertexSize(DELTA_BUFFER), numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY));

            mVertexDataRecord->cpuVertexData->vertexStart = 0;
            mVertexDataRecord->cpuVertexData->vertexCount = numVerts;
            
            Rect updateRect(mOffsetX, mOffsetY, mBoundaryX, mBoundaryY);
            updateVertexBuffer(posbuf, deltabuf, updateRect);
            bufbind->setBinding(POSITION_BUFFER, posbuf);
            bufbind->setBinding(DELTA_BUFFER, deltabuf);
            mVertexDataRecord->gpuVertexDataDirty = true;
        }
    }
    //----------------------------------------------------------------------
    void TerrainQuadTreeNode::updateVertexBuffer(HardwareVertexBufferSharedPtr& posbuf, 
        HardwareVertexBufferSharedPtr& deltabuf, const Rect& rect)
    {
        assert (rect.left >= mOffsetX && rect.right <= mBoundaryX && 
            rect.top >= mOffsetY && rect.bottom <= mBoundaryY);

        // potentially reset our bounds depending on coverage of the update
        resetBounds(rect);

        // Main data
        uint16 inc = (mTerrain->getSize()-1) / (mVertexDataRecord->resolution-1);
        long destOffsetX = rect.left <= mOffsetX ? 0 : (rect.left - mOffsetX) / inc;
        long destOffsetY = rect.top <= mOffsetY ? 0 : (rect.top - mOffsetY) / inc;
        // Fill the buffers
        
        HardwareBuffer::LockOptions lockMode;
        if (destOffsetX || destOffsetY || rect.width() < mSize
            || rect.height() < mSize)
        {
            lockMode = HardwareBuffer::HBL_NORMAL;
        }
        else
        {
            lockMode = HardwareBuffer::HBL_DISCARD;
        }

        Real uvScale = 1.0f / (mTerrain->getSize() - 1);
        const float* pBaseHeight = mTerrain->getHeightData(rect.left, rect.top);
        const float* pBaseDelta = mTerrain->getDeltaData(rect.left, rect.top);
        uint16 rowskip = mTerrain->getSize() * inc;
        uint16 destPosRowSkip = 0, destDeltaRowSkip = 0;
        unsigned char* pRootPosBuf = 0;
        unsigned char* pRootDeltaBuf = 0;
        unsigned char* pRowPosBuf = 0;
        unsigned char* pRowDeltaBuf = 0;

        if (posbuf)
        {
            destPosRowSkip = mVertexDataRecord->size * uint16(posbuf->getVertexSize());
            pRootPosBuf = static_cast<unsigned char*>(posbuf->lock(lockMode));
            pRowPosBuf = pRootPosBuf;
            // skip dest buffer in by left/top
            pRowPosBuf += destOffsetY * destPosRowSkip + destOffsetX * posbuf->getVertexSize();
        }
        if (deltabuf)
        {
            destDeltaRowSkip = mVertexDataRecord->size * uint16(deltabuf->getVertexSize());
            pRootDeltaBuf = static_cast<unsigned char*>(deltabuf->lock(lockMode));
            pRowDeltaBuf = pRootDeltaBuf;
            // skip dest buffer in by left/top
            pRowDeltaBuf += destOffsetY * destDeltaRowSkip + destOffsetX * deltabuf->getVertexSize();
        }
        Vector3 pos;
        
        bool vcompress = mTerrain->_getUseVertexCompression();
        
        for (uint16 y = rect.top; y < rect.bottom; y += inc)
        {
            const float* pHeight = pBaseHeight;
            const float* pDelta = pBaseDelta;
            float* pPosBuf = static_cast<float*>(static_cast<void*>(pRowPosBuf));
            float* pDeltaBuf = static_cast<float*>(static_cast<void*>(pRowDeltaBuf));
            for (uint16 x = rect.left; x < rect.right; x += inc)
            {
                if (pPosBuf)
                {
                    mTerrain->getPoint(x, y, *pHeight, &pos);

                    // Update bounds *before* making relative
                    mergeIntoBounds(x, y, pos);
                    // relative to local centre
                    pos -= mLocalCentre;
            
                    writePosVertex(vcompress, x, y, *pHeight, pos, uvScale, &pPosBuf);
                    pHeight += inc;
                    

                }

                if (pDeltaBuf)
                {
                    // delta, and delta LOD threshold
                    // we want delta to apply to LODs no higher than this value
                    // at runtime this will be combined with a per-renderable parameter
                    // to ensure we only apply morph to the correct LOD
                    writeDeltaVertex(vcompress, x, y, *pDelta, 
                        (float)mTerrain->getLODLevelWhenVertexEliminated(x, y) - 1.0f, 
                        &pDeltaBuf);
                    pDelta += inc;

                }

                
            }
            pBaseHeight += rowskip;
            pBaseDelta += rowskip;
            if (pRowPosBuf)
                pRowPosBuf += destPosRowSkip;
            if (pRowDeltaBuf)
                pRowDeltaBuf += destDeltaRowSkip;

        }

        // Skirts now
        // skirt spacing based on top-level resolution (* inc to cope with resolution which is not the max)
        uint16 skirtSpacing = mVertexDataRecord->skirtRowColSkip * inc;
        Vector3 skirtOffset;
        mTerrain->getVector(0, 0, -mTerrain->getSkirtSize(), &skirtOffset);

        // skirt rows
        // clamp rows to skirt spacing (round up)
        long skirtStartX = rect.left;
        long skirtStartY = rect.top;
        // for rows, clamp Y to skirt frequency, X to inc (LOD resolution vs top)
        if (skirtStartY % skirtSpacing)
            skirtStartY += skirtSpacing - (skirtStartY % skirtSpacing);
        if (skirtStartX % inc)
            skirtStartX += inc - (skirtStartX % inc);
        skirtStartY = std::max(skirtStartY, (long)mOffsetY);
        pBaseHeight = mTerrain->getHeightData(skirtStartX, skirtStartY);
        if (posbuf)
        {
            // position dest buffer just after the main vertex data
            pRowPosBuf = pRootPosBuf + posbuf->getVertexSize() 
                * mVertexDataRecord->size * mVertexDataRecord->size;
            // move it onwards to skip the skirts we don't need to update
            pRowPosBuf += destPosRowSkip * ((skirtStartY - mOffsetY) / skirtSpacing);
            pRowPosBuf += posbuf->getVertexSize() * (skirtStartX - mOffsetX) / inc;
        }
        if (deltabuf)
        {
            // position dest buffer just after the main vertex data
            pRowDeltaBuf = pRootDeltaBuf + deltabuf->getVertexSize() 
                * mVertexDataRecord->size * mVertexDataRecord->size;
            // move it onwards to skip the skirts we don't need to update
            pRowDeltaBuf += destDeltaRowSkip * (skirtStartY - mOffsetY) / skirtSpacing;
            pRowDeltaBuf += deltabuf->getVertexSize() * (skirtStartX - mOffsetX) / inc;
        }
        for (uint16 y = skirtStartY; y < rect.bottom; y += skirtSpacing)
        {
            const float* pHeight = pBaseHeight;
            float* pPosBuf = static_cast<float*>(static_cast<void*>(pRowPosBuf));
            float* pDeltaBuf = static_cast<float*>(static_cast<void*>(pRowDeltaBuf));
            for (uint16 x = skirtStartX; x < rect.right; x += inc)
            {
                if (pPosBuf)
                {
                    mTerrain->getPoint(x, y, *pHeight, &pos);
                    // relative to local centre
                    pos -= mLocalCentre;
                    pos += skirtOffset;
                    writePosVertex(vcompress, x, y, *pHeight - mTerrain->getSkirtSize(), pos, uvScale, &pPosBuf);

                    pHeight += inc;

                    

                }

                if (pDeltaBuf)
                {
                    // delta (none)
                    // delta threshold (irrelevant)
                    writeDeltaVertex(vcompress, x, y, 0, 99, &pDeltaBuf);
                }
            }
            pBaseHeight += mTerrain->getSize() * skirtSpacing;
            if (pRowPosBuf)
                pRowPosBuf += destPosRowSkip;
            if (pRowDeltaBuf)
                pRowDeltaBuf += destDeltaRowSkip;
        }
        // skirt cols
        // clamp cols to skirt spacing (round up)
        skirtStartX = rect.left;
        if (skirtStartX % skirtSpacing)
            skirtStartX += skirtSpacing - (skirtStartX % skirtSpacing);
        // clamp Y to inc (LOD resolution vs top)
        skirtStartY = rect.top;
        if (skirtStartY % inc)
            skirtStartY += inc - (skirtStartY % inc);
        skirtStartX = std::max(skirtStartX, (long)mOffsetX);
        if (posbuf)
        {
            // position dest buffer just after the main vertex data and skirt rows
            pRowPosBuf = pRootPosBuf + posbuf->getVertexSize() 
                * mVertexDataRecord->size * mVertexDataRecord->size;
            // skip the row skirts
            pRowPosBuf += mVertexDataRecord->numSkirtRowsCols * mVertexDataRecord->size * posbuf->getVertexSize();
            // move it onwards to skip the skirts we don't need to update
            pRowPosBuf += destPosRowSkip * (skirtStartX - mOffsetX) / skirtSpacing;
            pRowPosBuf += posbuf->getVertexSize() * (skirtStartY - mOffsetY) / inc;
        }
        if (deltabuf)
        {
            // Delta dest buffer just after the main vertex data and skirt rows
            pRowDeltaBuf = pRootDeltaBuf + deltabuf->getVertexSize() 
                * mVertexDataRecord->size * mVertexDataRecord->size;
            // skip the row skirts
            pRowDeltaBuf += mVertexDataRecord->numSkirtRowsCols * mVertexDataRecord->size * deltabuf->getVertexSize();
            // move it onwards to skip the skirts we don't need to update
            pRowDeltaBuf += destDeltaRowSkip * (skirtStartX - mOffsetX) / skirtSpacing;
            pRowDeltaBuf += deltabuf->getVertexSize() * (skirtStartY - mOffsetY) / inc;
        }
        
        for (uint16 x = skirtStartX; x < rect.right; x += skirtSpacing)
        {
            float* pPosBuf = static_cast<float*>(static_cast<void*>(pRowPosBuf));
            float* pDeltaBuf = static_cast<float*>(static_cast<void*>(pRowDeltaBuf));
            for (uint16 y = skirtStartY; y < rect.bottom; y += inc)
            {
                if (pPosBuf)
                {
                    float height = mTerrain->getHeightAtPoint(x, y);
                    mTerrain->getPoint(x, y, height, &pos);
                    // relative to local centre
                    pos -= mLocalCentre;
                    pos += skirtOffset;
                    
                    writePosVertex(vcompress, x, y, height - mTerrain->getSkirtSize(), pos, uvScale, &pPosBuf);

                }
                if (pDeltaBuf)
                {
                    // delta (none)
                    // delta threshold (irrelevant)
                    writeDeltaVertex(vcompress, x, y, 0, 99, &pDeltaBuf);
                }
            }
            if (pRowPosBuf)
                pRowPosBuf += destPosRowSkip;
            if (pRowDeltaBuf)
                pRowDeltaBuf += destDeltaRowSkip;
        }

        if (posbuf)
            posbuf->unlock();
        if (deltabuf)
            deltabuf->unlock();
        
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::writePosVertex(bool compress, uint16 x, uint16 y, float height, 
        const Vector3& pos, float uvScale, float** ppPos)
    {
        float* pPosBuf = *ppPos;
        
        if (compress)
        {
            short* pPosShort = static_cast<short*>(static_cast<void*>(pPosBuf));
            *pPosShort++ = (short)x;
            *pPosShort++ = (short)y;
            pPosBuf = static_cast<float*>(static_cast<void*>(pPosShort));

            *pPosBuf++ = height;
        }
        else 
        {
            *pPosBuf++ = pos.x;
            *pPosBuf++ = pos.y;
            *pPosBuf++ = pos.z;
            
            // UVs - base UVs vary from 0 to 1, all other values
            // will be derived using scalings
            *pPosBuf++ = x * uvScale;
            *pPosBuf++ = 1.0f - (y * uvScale);
        }
        
        *ppPos = pPosBuf;
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::writeDeltaVertex(bool compress, uint16 x, uint16 y, 
        float delta, float deltaThresh, float** ppDelta)
    {
        *(*ppDelta)++ = delta;
        *(*ppDelta)++ = deltaThresh;
    }
    //---------------------------------------------------------------------
    uint16 TerrainQuadTreeNode::calcSkirtVertexIndex(uint16 mainIndex, bool isCol)
    {
        const VertexDataRecord* vdr = getVertexDataRecord();
        // row / col in main vertex resolution
        uint16 row = mainIndex / vdr->size;
        uint16 col = mainIndex % vdr->size;
        
        // skrits are after main vertices, so skip them
        uint16 base = vdr->size * vdr->size;

        // The layout in vertex data is:
        // 1. row skirts
        //    numSkirtRowsCols rows of resolution vertices each
        // 2. column skirts
        //    numSkirtRowsCols cols of resolution vertices each

        // No offsets used here, this is an index into the current vertex data, 
        // which is already relative
        if (isCol)
        {
            uint16 skirtNum = col / vdr->skirtRowColSkip;
            uint16 colbase = vdr->numSkirtRowsCols * vdr->size;
            return base + colbase + vdr->size * skirtNum + row;
        }
        else
        {
            uint16 skirtNum = row / vdr->skirtRowColSkip;
            return base + vdr->size * skirtNum + col;
        }
        
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::destroyCpuVertexData()
    {
        if (mVertexDataRecord && mVertexDataRecord->cpuVertexData)
        {
            // avoid copy empty buffer
            mVertexDataRecord->gpuVertexDataDirty = false;
            // delete the bindings and declaration manually since not from a buf mgr
            OGRE_DELETE mVertexDataRecord->cpuVertexData->vertexDeclaration;
            mVertexDataRecord->cpuVertexData->vertexDeclaration = 0;

            OGRE_DELETE mVertexDataRecord->cpuVertexData->vertexBufferBinding;
            mVertexDataRecord->cpuVertexData->vertexBufferBinding = 0;

            OGRE_DELETE mVertexDataRecord->cpuVertexData;
            mVertexDataRecord->cpuVertexData = 0;
        }


    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::populateIndexData(uint16 batchSize, IndexData* destData)
    {
        const VertexDataRecord* vdr = getVertexDataRecord();

        // Ratio of the main terrain resolution in relation to this vertex data resolution
        uint16 resolutionRatio = (mTerrain->getSize() - 1) / (vdr->resolution - 1);
        // At what frequency do we sample the vertex data we're using?
        // mSize is the coverage in terms of the original terrain data (not split to fit in 16-bit)
        uint16 vertexIncrement = (mSize-1) / (batchSize-1);
        // however, the vertex data we're referencing may not be at the full resolution anyway
        vertexIncrement /= resolutionRatio;
        uint16 vdatasizeOffsetX = (mOffsetX - mNodeWithVertexData->mOffsetX) / resolutionRatio;
        uint16 vdatasizeOffsetY = (mOffsetY - mNodeWithVertexData->mOffsetY) / resolutionRatio;

        destData->indexBuffer = mTerrain->getGpuBufferAllocator()->getSharedIndexBuffer(batchSize, vdr->size, 
            vertexIncrement, vdatasizeOffsetX, vdatasizeOffsetY, 
            vdr->numSkirtRowsCols, vdr->skirtRowColSkip);
        destData->indexStart = 0;
        destData->indexCount = destData->indexBuffer->getNumIndexes();

        // shared index buffer is pre-populated     
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::createGpuVertexData()
    {
        // TODO - mutex cpu data
        if (mVertexDataRecord && mVertexDataRecord->cpuVertexData && !mVertexDataRecord->gpuVertexData)
        {
            // copy data from CPU to GPU, but re-use vertex buffers (so don't use regular clone)
            mVertexDataRecord->gpuVertexData = OGRE_NEW VertexData();
            VertexData* srcData = mVertexDataRecord->cpuVertexData;
            VertexData* destData = mVertexDataRecord->gpuVertexData;

            // copy vertex buffers
            // get new buffers
            HardwareVertexBufferSharedPtr destPosBuf, destDeltaBuf;
            mTerrain->getGpuBufferAllocator()->allocateVertexBuffers(mTerrain, srcData->vertexCount, 
                destPosBuf, destDeltaBuf);
                
            // copy data
            destPosBuf->copyData(*srcData->vertexBufferBinding->getBuffer(POSITION_BUFFER));
            destDeltaBuf->copyData(*srcData->vertexBufferBinding->getBuffer(DELTA_BUFFER));

            // set bindings
            destData->vertexBufferBinding->setBinding(POSITION_BUFFER, destPosBuf);
            destData->vertexBufferBinding->setBinding(DELTA_BUFFER, destDeltaBuf);

            // Basic vertex info
            destData->vertexStart = srcData->vertexStart;
            destData->vertexCount = srcData->vertexCount;
            // Copy elements
            const VertexDeclaration::VertexElementList elems = 
                srcData->vertexDeclaration->getElements();
            VertexDeclaration::VertexElementList::const_iterator ei, eiend;
            eiend = elems.end();
            for (ei = elems.begin(); ei != eiend; ++ei)
            {
                destData->vertexDeclaration->addElement(
                    ei->getSource(),
                    ei->getOffset(),
                    ei->getType(),
                    ei->getSemantic(),
                    ei->getIndex() );
            }

            // We don't need the CPU copy anymore
            destroyCpuVertexData();
        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::updateGpuVertexData()
    {
        if (mVertexDataRecord && mVertexDataRecord->gpuVertexDataDirty)
        {
            mVertexDataRecord->gpuVertexData->vertexBufferBinding->getBuffer(POSITION_BUFFER)->
                copyData(*mVertexDataRecord->cpuVertexData->vertexBufferBinding->getBuffer(POSITION_BUFFER).get());
            mVertexDataRecord->gpuVertexData->vertexBufferBinding->getBuffer(DELTA_BUFFER)->
                copyData(*mVertexDataRecord->cpuVertexData->vertexBufferBinding->getBuffer(DELTA_BUFFER).get());
            mVertexDataRecord->gpuVertexDataDirty = false;
        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::destroyGpuVertexData()
    {
        if (mVertexDataRecord && mVertexDataRecord->gpuVertexData)
        {
            // Before we delete, free up the vertex buffers for someone else
            mTerrain->getGpuBufferAllocator()->freeVertexBuffers(
                mVertexDataRecord->gpuVertexData->vertexBufferBinding->getBuffer(POSITION_BUFFER), 
                mVertexDataRecord->gpuVertexData->vertexBufferBinding->getBuffer(DELTA_BUFFER));
            OGRE_DELETE mVertexDataRecord->gpuVertexData;
            mVertexDataRecord->gpuVertexData = 0;
        }


    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::createGpuIndexData()
    {
        for (size_t lod = 0; lod < mLodLevels.size(); ++lod)
        {
            LodLevel* ll = mLodLevels[lod];

            if (!ll->gpuIndexData)
            {
                // clone, using default buffer manager ie hardware
                ll->gpuIndexData = OGRE_NEW IndexData();
                populateIndexData(ll->batchSize, ll->gpuIndexData);
            }

        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::destroyGpuIndexData()
    {
        for (size_t lod = 0; lod < mLodLevels.size(); ++lod)
        {
            LodLevel* ll = mLodLevels[lod];

            OGRE_DELETE ll->gpuIndexData;
            ll->gpuIndexData = 0;
        }

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::mergeIntoBounds(long x, long y, const Vector3& pos)
    {
        if (pointIntersectsNode(x, y))
        {
            Vector3 localPos = pos - mLocalCentre;
            mAABB.merge(localPos);
            mBoundingRadius = std::max(mBoundingRadius, localPos.length());
            
            if (!isLeaf())
            {
                for (int i = 0; i < 4; ++i)
                    mChildren[i]->mergeIntoBounds(x, y, pos);
            }
        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::resetBounds(const Rect& rect)
    {
        if (rectContainsNode(rect))
        {
            mAABB.setNull();
            mBoundingRadius = 0;
            
            if (!isLeaf())
            {
                for (int i = 0; i < 4; ++i)
                    mChildren[i]->resetBounds(rect);
            }

            
        }
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::rectContainsNode(const Rect& rect)
    {
        return (rect.left <= mOffsetX && rect.right > mBoundaryX &&
            rect.top <= mOffsetY && rect.bottom > mBoundaryY);
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::rectIntersectsNode(const Rect& rect)
    {
        return (rect.right >= mOffsetX && rect.left <= mBoundaryX &&
                rect.bottom >= mOffsetY && rect.top <= mBoundaryY);
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::pointIntersectsNode(long x, long y)
    {
        return x >= mOffsetX && x < mBoundaryX && 
            y >= mOffsetY && y < mBoundaryY;
    }
    //---------------------------------------------------------------------
    const AxisAlignedBox& TerrainQuadTreeNode::getAABB() const
    {
        return mAABB;
    }
    //---------------------------------------------------------------------
    Real TerrainQuadTreeNode::getBoundingRadius() const
    {
        return mBoundingRadius;
    }
    //---------------------------------------------------------------------
    Real TerrainQuadTreeNode::getMinHeight() const
    {
        switch (mTerrain->getAlignment())
        {
        case Terrain::ALIGN_X_Y:
        default:
            return mAABB.getMinimum().z;
        case Terrain::ALIGN_X_Z:
            return mAABB.getMinimum().y;
        case Terrain::ALIGN_Y_Z:
            return mAABB.getMinimum().x;
        };
    }
    //---------------------------------------------------------------------
    Real TerrainQuadTreeNode::getMaxHeight() const
    {
        switch (mTerrain->getAlignment())
        {
        case Terrain::ALIGN_X_Y:
        default:
            return mAABB.getMaximum().z;
        case Terrain::ALIGN_X_Z:
            return mAABB.getMaximum().y;
        case Terrain::ALIGN_Y_Z:
            return mAABB.getMaximum().x;
        };

    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::calculateCurrentLod(const Camera* cam, Real cFactor)
    {
        mSelfOrChildRendered = false;

        // early-out
        if (!cam->isVisible(mMovable->getWorldBoundingBox(true)))
        {
            mCurrentLod = -1;
            return mSelfOrChildRendered;
        }

        // Check children first
        int childRenderedCount = 0;
        if (!isLeaf())
        {
            for (int i = 0; i < 4; ++i)
            {
                if (mChildren[i]->calculateCurrentLod(cam, cFactor))
                    ++childRenderedCount;
            }

        }

        // this node not loaded yet so skip
        if (!mMovable->isAttached())
        {
            mCurrentLod = -1;
            return mSelfOrChildRendered;
        }

        if (childRenderedCount == 0)
        {

            // no children were within their LOD ranges, so we should consider our own
            Vector3 localPos = cam->getDerivedPosition() - mLocalCentre - mTerrain->getPosition();
            Real dist;
            if (TerrainGlobalOptions::getSingleton().getUseRayBoxDistanceCalculation())
            {
                // Get distance to this terrain node (to closest point of the box)
                // head towards centre of the box (note, box may not cover mLocalCentre because of height)
                Vector3 dir(mAABB.getCenter() - localPos);
                dir.normalise();
                Ray ray(localPos, dir);
                std::pair<bool, Real> intersectRes = Math::intersects(ray, mAABB);

                // ray will always intersect, we just want the distance
                dist = intersectRes.second;
            }
            else
            {
                // distance to tile centre
                dist = localPos.length();
                // deduct half the radius of the box, assume that on average the 
                // worst case is best approximated by this
                dist -= (mBoundingRadius * 0.5f);
            }

            // Do material LOD
            MaterialPtr material = getMaterial();
            const LodStrategy *materialStrategy = material->getLodStrategy();
            Real lodValue = materialStrategy->getValue(mMovable, cam);
            // Get the index at this biased depth
            mMaterialLodIndex = material->getLodIndex(lodValue);


            // For each LOD, the distance at which the LOD will transition *downwards*
            // is given by 
            // distTransition = maxDelta * cFactor;
            uint lodLvl = 0;
            mCurrentLod = -1;
            for (LodLevelList::iterator i = mLodLevels.begin(); i != mLodLevels.end(); ++i, ++lodLvl)
            {
                // If we have no parent, and this is the lowest LOD, we always render
                // this is the 'last resort' so to speak, we always enoucnter this last
                if (lodLvl+1 == mLodLevels.size() && !mParent)
                {
                    mCurrentLod = lodLvl;
                    mSelfOrChildRendered = true;
                    mLodTransition = 0;
                }
                else
                {
                    // check the distance
                    LodLevel* ll = *i;

                    // Calculate or reuse transition distance
                    Real distTransition;
                    if (Math::RealEqual(cFactor, ll->lastCFactor))
                        distTransition = ll->lastTransitionDist;
                    else
                    {
                        distTransition = ll->maxHeightDelta * cFactor;
                        ll->lastCFactor = cFactor;
                        ll->lastTransitionDist = distTransition;
                    }

                    if (dist < distTransition)
                    {
                        // we're within range of this LOD
                        mCurrentLod = lodLvl;
                        mSelfOrChildRendered = true;

                        if (mTerrain->_getMorphRequired())
                        {
                            // calculate the transition percentage
                            // we need a percentage of the total distance for just this LOD, 
                            // which means taking off the distance for the next higher LOD
                            // which is either the previous entry in the LOD list, 
                            // or the largest of any children. In both cases these will
                            // have been calculated before this point, since we process
                            // children first. Distances at lower LODs are guaranteed
                            // to be larger than those at higher LODs

                            Real distTotal = distTransition;
                            if (isLeaf())
                            {
                                // Any higher LODs?
                                if (i != mLodLevels.begin())
                                {
                                    LodLevelList::iterator prev = i - 1;
                                    distTotal -= (*prev)->lastTransitionDist;
                                }
                            }
                            else
                            {
                                // Take the distance of the lowest LOD of child
                                const LodLevel* childLod = mChildWithMaxHeightDelta->getLodLevel(
                                    mChildWithMaxHeightDelta->getLodCount()-1);
                                distTotal -= childLod->lastTransitionDist;
                            }
                            // fade from 0 to 1 in the last 25% of the distance
                            Real distMorphRegion = distTotal * 0.25f;
                            Real distRemain = distTransition - dist;

                            mLodTransition = 1.0f - (distRemain / distMorphRegion);
                            mLodTransition = std::min(1.0f, mLodTransition);
                            mLodTransition = std::max(0.0f, mLodTransition);

                            // Pass both the transition % and target LOD (GLOBAL current + 1)
                            // this selectively applies the morph just to the
                            // vertices which would drop out at this LOD, even 
                            // while using the single shared vertex data
                            mRend->setCustomParameter(Terrain::LOD_MORPH_CUSTOM_PARAM, 
                                Vector4(mLodTransition, mCurrentLod + mBaseLod + 1, 0, 0));

                        }
                        // since LODs are ordered from highest to lowest detail, 
                        // we can stop looking now
                        break;
                    }

                }
            }

        }
        else 
        {
            // we should not render ourself
            mCurrentLod = -1;
            mSelfOrChildRendered = true; 
            if (childRenderedCount < 4)
            {
                // only *some* children decided to render on their own, but either 
                // none or all need to render, so set the others manually to their lowest
                for (int i = 0; i < 4; ++i)
                {
                    TerrainQuadTreeNode* child = mChildren[i];
                    if (!child->isSelfOrChildRenderedAtCurrentLod())
                    {
                        child->setCurrentLod(child->getLodCount()-1);
                        child->setLodTransition(1.0);
                    }
                }
            } // (childRenderedCount < 4)

        } // (childRenderedCount == 0)


        return mSelfOrChildRendered;

    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::setCurrentLod(int lod)
    {
         mCurrentLod = lod;
         mRend->setCustomParameter(Terrain::LOD_MORPH_CUSTOM_PARAM, 
             Vector4(mLodTransition, mCurrentLod + mBaseLod + 1, 0, 0));
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::setLodTransition(float t)
    {
        mLodTransition = t;                         
        mRend->setCustomParameter(Terrain::LOD_MORPH_CUSTOM_PARAM, 
            Vector4(mLodTransition, mCurrentLod + mBaseLod + 1, 0, 0));
    }
    //---------------------------------------------------------------------
    Renderable *TerrainQuadTreeNode::_getRenderable()
    {
        return mRend;
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::isRenderedAtCurrentLod() const
    {
        return mCurrentLod != -1;
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::isSelfOrChildRenderedAtCurrentLod() const
    {
        return mSelfOrChildRendered;
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::updateRenderQueue(RenderQueue* queue)
    {
        if (isRenderedAtCurrentLod())
        {
            queue->addRenderable(mRend, mTerrain->getRenderQueueGroup());           
        }
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::visitRenderables(Renderable::Visitor* visitor,  bool debugRenderables)
    {
        visitor->visit(mRend, 0, false);
    }
    //---------------------------------------------------------------------
    const MaterialPtr& TerrainQuadTreeNode::getMaterial(void) const
    {
        return mTerrain->getMaterial();
    }
    //---------------------------------------------------------------------
    Technique* TerrainQuadTreeNode::getTechnique(void) const
    { 
        return getMaterial()->getBestTechnique(mMaterialLodIndex, mRend); 
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::getRenderOperation(RenderOperation& op)
    {
        mNodeWithVertexData->updateGpuVertexData();

        op.indexData = mLodLevels[mCurrentLod]->gpuIndexData;
        op.operationType = RenderOperation::OT_TRIANGLE_STRIP;
        op.useIndexes = true;
        op.vertexData = getVertexDataRecord()->gpuVertexData;
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::getWorldTransforms(Matrix4* xform) const
    {
        if (mTerrain->_getUseVertexCompression())
        {
            // vertex data is generated in terrain space
            *xform = Matrix4::IDENTITY;
            xform->setTrans(mTerrain->getPosition());
            
        }
        else 
        {
            // the vertex data is relative to the node that owns the vertex data
            *xform = mNodeWithVertexData->mMovable->_getParentNodeFullTransform();          
        }

    }
    //---------------------------------------------------------------------
    Real TerrainQuadTreeNode::getSquaredViewDepth(const Camera* cam) const
    {
        return mMovable->getParentSceneNode()->getSquaredViewDepth(cam);
    }
    //---------------------------------------------------------------------
    const LightList& TerrainQuadTreeNode::getLights(void) const
    {
        return mMovable->queryLights();
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::getCastsShadows(void) const
    {
        return TerrainGlobalOptions::getSingleton().getCastsDynamicShadows();
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    TerrainQuadTreeNode::Movable::Movable(TerrainQuadTreeNode* parent)
        : mParent(parent)
    {
    }
    //---------------------------------------------------------------------
    TerrainQuadTreeNode::Movable::~Movable()
    {

    }
    //---------------------------------------------------------------------
    const String& TerrainQuadTreeNode::Movable::getMovableType(void) const
    {
        static String stype("OgreTerrainNodeMovable");

        return stype;

    }
    //---------------------------------------------------------------------
    const AxisAlignedBox& TerrainQuadTreeNode::Movable::getBoundingBox(void) const
    {
        return mParent->getAABB();
    }
    //---------------------------------------------------------------------
    Real TerrainQuadTreeNode::Movable::getBoundingRadius(void) const
    {
        return mParent->getBoundingRadius();
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::Movable::isVisible(void) const
    {
        if (mParent->getCurrentLod() == -1)
            return false;
        else
            return MovableObject::isVisible();
    }
    //---------------------------------------------------------------------
    uint32 TerrainQuadTreeNode::Movable::getVisibilityFlags(void) const
    {
        // Combine own vis (in case anyone sets this) and terrain overall
        return mVisibilityFlags & mParent->getTerrain()->getVisibilityFlags();
    }
    //---------------------------------------------------------------------
    uint32 TerrainQuadTreeNode::Movable::getQueryFlags(void) const
    {
        // Combine own vis (in case anyone sets this) and terrain overall
        return mQueryFlags & mParent->getTerrain()->getQueryFlags();
    }
    //------------------------------------------------------------------------
    void TerrainQuadTreeNode::Movable::_updateRenderQueue(RenderQueue* queue)
    {
        mParent->updateRenderQueue(queue);      
    }
    //------------------------------------------------------------------------
    void TerrainQuadTreeNode::Movable::visitRenderables(Renderable::Visitor* visitor,  bool debugRenderables)
    {
        mParent->visitRenderables(visitor, debugRenderables);   
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::Movable::getCastShadows(void) const
    {
        return mParent->getCastsShadows();
    }
    //------------------------------------------------------------------------
    //---------------------------------------------------------------------
    TerrainQuadTreeNode::Rend::Rend(TerrainQuadTreeNode* parent)
        :mParent(parent)
    {
    }
    //---------------------------------------------------------------------
    TerrainQuadTreeNode::Rend::~Rend()
    {
    }
    //---------------------------------------------------------------------
    const MaterialPtr& TerrainQuadTreeNode::Rend::getMaterial(void) const
    {
        return mParent->getMaterial();
    }
    //---------------------------------------------------------------------
    Technique* TerrainQuadTreeNode::Rend::getTechnique() const
    {
        return mParent->getTechnique();
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::Rend::getRenderOperation(RenderOperation& op)
    {
        mParent->getRenderOperation(op);
    }
    //---------------------------------------------------------------------
    void TerrainQuadTreeNode::Rend::getWorldTransforms(Matrix4* xform) const
    {
        mParent->getWorldTransforms(xform);
    }
    //---------------------------------------------------------------------
    Real TerrainQuadTreeNode::Rend::getSquaredViewDepth(const Camera* cam) const
    {
        return mParent->getSquaredViewDepth(cam);
    }
    //---------------------------------------------------------------------
    const LightList& TerrainQuadTreeNode::Rend::getLights(void) const
    {
        return mParent->getLights();
    }
    //---------------------------------------------------------------------
    bool TerrainQuadTreeNode::Rend::getCastsShadows(void) const
    {
        return mParent->getCastsShadows();
    }
    //---------------------------------------------------------------------


    



}

