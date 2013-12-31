/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreVolumeOctreeNode.h"

#include "OgreVolumeMeshBuilder.h"


namespace Ogre {
namespace Volume {
    
    const Real OctreeNode::NEAR_FACTOR = (Real)2.0;
    const size_t OctreeNode::OCTREE_CHILDREN_COUNT = 8;
    uint32 OctreeNode::mGridPositionCount = 0;
    size_t OctreeNode::mNodeI = 0;
    
    //-----------------------------------------------------------------------

    void OctreeNode::buildOctreeGridLines(ManualObject *manual) const
    {
        if (!mChildren)
        {
            Vector3 xWidth(mTo.x - mFrom.x, (Real)0.0, (Real)0.0);
            Vector3 yWidth((Real)0.0, mTo.y - mFrom.y, (Real)0.0);
            Vector3 zWidth((Real)0.0, (Real)0.0, mTo.z - mFrom.z);
            MeshBuilder::addCubeToManualObject(
                manual,
                mFrom,
                mFrom + xWidth,
                mFrom + xWidth + zWidth,
                mFrom + zWidth,
                mFrom + yWidth,
                mFrom + yWidth + xWidth,
                mFrom + yWidth + xWidth + zWidth,
                mFrom + yWidth + zWidth,
                mGridPositionCount);
        }
        else
        {
            mChildren[0]->buildOctreeGridLines(manual);
            mChildren[1]->buildOctreeGridLines(manual);
            mChildren[2]->buildOctreeGridLines(manual);
            mChildren[3]->buildOctreeGridLines(manual);
            mChildren[4]->buildOctreeGridLines(manual);
            mChildren[5]->buildOctreeGridLines(manual);
            mChildren[6]->buildOctreeGridLines(manual);
            mChildren[7]->buildOctreeGridLines(manual);
        }
    }
    
    //-----------------------------------------------------------------------

    OctreeNode::OctreeNode(const Vector3 &from, const Vector3 &to) : mFrom(from), mTo(to),
        mChildren(0), mOctreeGrid(0), mCenterValue(0.0, 0.0, 0.0, 0.0)
    {
    }
    
    //-----------------------------------------------------------------------

    OctreeNode::~OctreeNode(void)
    {
        if (mChildren)
        {
            for (size_t i = 0; i < OCTREE_CHILDREN_COUNT; ++i)
            {
                OGRE_DELETE mChildren[i];
            }
            delete[] mChildren;
        }
    }
    
    //-----------------------------------------------------------------------

    OctreeNode* OctreeNode::createInstance(const Vector3& from, const Vector3& to)
    {
        return OGRE_NEW OctreeNode(from, to);
    }
    
    //-----------------------------------------------------------------------

    void OctreeNode::split(const OctreeNodeSplitPolicy *splitPolicy, const Source *src, const Real geometricError)
    {
        if (splitPolicy->doSplit(this, geometricError))
        {
            Vector3 newCenter, xWidth, yWidth, zWidth;
            OctreeNode::getChildrenDimensions(mFrom, mTo, newCenter, xWidth, yWidth, zWidth);
            /*
               4 5
              7 6
               0 1
              3 2
              0 == from
              6 == to
            */
            mChildren = new OctreeNode*[OCTREE_CHILDREN_COUNT];
            mChildren[0] = createInstance(mFrom, newCenter);
            mChildren[0]->split(splitPolicy, src, geometricError);
            mChildren[1] = createInstance(mFrom + xWidth, newCenter + xWidth);
            mChildren[1]->split(splitPolicy, src, geometricError);
            mChildren[2] = createInstance(mFrom + xWidth + zWidth, newCenter + xWidth + zWidth);
            mChildren[2]->split(splitPolicy, src, geometricError);
            mChildren[3] = createInstance(mFrom + zWidth, newCenter + zWidth);
            mChildren[3]->split(splitPolicy, src, geometricError);
            mChildren[4] = createInstance(mFrom + yWidth, newCenter + yWidth);
            mChildren[4]->split(splitPolicy, src, geometricError);
            mChildren[5] = createInstance(mFrom + yWidth + xWidth, newCenter + yWidth + xWidth);
            mChildren[5]->split(splitPolicy, src, geometricError);
            mChildren[6] = createInstance(mFrom + yWidth + xWidth + zWidth, newCenter + yWidth + xWidth + zWidth);
            mChildren[6]->split(splitPolicy, src, geometricError);
            mChildren[7] = createInstance(mFrom + yWidth + zWidth, newCenter + yWidth + zWidth);
            mChildren[7]->split(splitPolicy, src, geometricError);
        }
        else
        {
            if (mCenterValue.x == (Real)0.0 && mCenterValue.y == (Real)0.0 && mCenterValue.z == (Real)0.0 && mCenterValue.w == (Real)0.0)
            {
                setCenterValue(src->getValueAndGradient(getCenter()));
            }
        }
    }
    
    //-----------------------------------------------------------------------

    Entity* OctreeNode::getOctreeGrid(SceneManager *sceneManager)
    {
        if (!mOctreeGrid)
        {
            mGridPositionCount = 0;
            mNodeI++;
            ManualObject* manual = sceneManager->createManualObject();
            manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
            manual->colour((Real)1.0, (Real)0.0, (Real)0.0);
            buildOctreeGridLines(manual); 
            manual->end();
        
            StringUtil::StrStreamType meshName;
            meshName << "VolumeOctreeGridMesh" << mNodeI;
            manual->convertToMesh(meshName.str());
            StringUtil::StrStreamType entityName;
            entityName << "VolumeOctreeGrid" << mNodeI;
            mOctreeGrid = sceneManager->createEntity(entityName.str(), meshName.str());
        }
        return mOctreeGrid;
    }

}
}