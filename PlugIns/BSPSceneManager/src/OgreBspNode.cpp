/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreBspNode.h"
#include "OgreBspLevel.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    BspNode::BspNode(BspLevel* owner, bool isLeaf)
    {
        mOwner = owner;
        mIsLeaf = isLeaf;

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
