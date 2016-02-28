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

#include "OgreBspNode.h"
#include "OgreBspLevel.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    BspNode::BspNode(BspLevel* owner, bool inIsLeaf)
    {
        mOwner = owner;
        mIsLeaf = inIsLeaf;
    }

    //-----------------------------------------------------------------------
    BspNode::BspNode()
    {
    }
    //-----------------------------------------------------------------------
    BspNode::~BspNode()
    {
    }

    //-----------------------------------------------------------------------
    bool BspNode::isLeaf(void) const
    {
        return mIsLeaf;
    }

    //-----------------------------------------------------------------------
    BspNode* BspNode::getFront(void) const
    {
        if (mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getFront");
        return mFront;
    }

    //-----------------------------------------------------------------------
    BspNode* BspNode::getBack(void) const
    {
        if (mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getBack");
        return mBack;
    }

    //-----------------------------------------------------------------------
    const Plane& BspNode::getSplitPlane(void) const
    {
        if (mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getSplitPlane");

        return mSplitPlane;

    }

    //-----------------------------------------------------------------------
    const AxisAlignedBox& BspNode::getBoundingBox(void) const
    {
        if (!mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on a leaf node.",
                "BspNode::getBoundingBox");
        return mBounds;

    }

    //-----------------------------------------------------------------------
    int BspNode::getNumFaceGroups(void) const
    {
        if (!mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on a leaf node.",
                "BspNode::getNumFaces");
        return mNumFaceGroups;
    }

    //-----------------------------------------------------------------------
    int BspNode::getFaceGroupStart(void) const
    {
        if (!mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on a leaf node.",
                "BspNode::getFaces");
        return mFaceGroupStart;
    }

    //-----------------------------------------------------------------------
    bool BspNode::isLeafVisible(const BspNode* leaf) const
    {
        return mOwner->isLeafVisible(this, leaf);
    }
    //-----------------------------------------------------------------------
    Plane::Side BspNode::getSide (const Vector3& point) const
    {
        if (mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getSide");

        return mSplitPlane.getSide(point);

    }
    //-----------------------------------------------------------------------
    BspNode* BspNode::getNextNode(const Vector3& point) const
    {

        if (mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getNextNode");

        Plane::Side sd = getSide(point);
        if (sd == Plane::NEGATIVE_SIDE)
        {
            //LogManager::getSingleton().logMessage("back");
            return getBack();
        }
        else
        {
            //LogManager::getSingleton().logMessage("front");
            return getFront();
        }



    }
    //-----------------------------------------------------------------------
    void BspNode::_addMovable(const MovableObject* mov)
    {
        mMovables.insert(mov);
    }
    //-----------------------------------------------------------------------
    void BspNode::_removeMovable(const MovableObject* mov)
    {
        mMovables.erase(mov);
    }
    //-----------------------------------------------------------------------
    Real BspNode::getDistance(const Vector3& pos) const
    {
        if (mIsLeaf)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getSide");

        return mSplitPlane.getDistance(pos);

    }
    //-----------------------------------------------------------------------
    const BspNode::NodeBrushList& BspNode::getSolidBrushes(void) const
    {
        return mSolidBrushes;
    }
    //-----------------------------------------------------------------------
    std::ostream& operator<< (std::ostream& o, BspNode& n)
    {
        o << "BspNode(";
        if (n.mIsLeaf)
        {
            o << "leaf, bbox=" << n.mBounds << ", cluster=" << n.mVisCluster;
            o << ", faceGrps=" << n.mNumFaceGroups << ", faceStart=" << n.mFaceGroupStart << ")";
        }
        else
        {
            o <<  "splitter, plane=" << n.mSplitPlane << ")";
        }
        return o;

    }

}
