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
#include "OgreVolumeOctreeNodeSplitPolicy.h"


namespace Ogre {
namespace Volume {
    
    OctreeNodeSplitPolicy::OctreeNodeSplitPolicy(const Source *src, const Real maxCellSize) :
        mSrc(src), mMaxCellSize(maxCellSize)
    {
    }

    //-----------------------------------------------------------------------

    bool OctreeNodeSplitPolicy::doSplit(OctreeNode *node, const Real geometricError) const
    {

        // We have a highest resolution.
        Vector3 from = node->getFrom();
        Vector3 to = node->getTo();
        if (to.x - from.x <= mMaxCellSize)
        {
            return false;
        }

        // Don't split if nothing is inside.
        Vector4 centerValue = mSrc->getValueAndGradient(node->getCenter());
        if (Math::Abs(centerValue.w) > (to - from).length() * mSrc->getVolumeSpaceToWorldSpaceFactor())
        {
            node->setCenterValue(centerValue);
            return false;
        }

        // Error metric of http://www.andrew.cmu.edu/user/jessicaz/publication/meshing/
        Real f000 = mSrc->getValue(from);
        Real f001 = mSrc->getValue(node->getCorner3());
        Real f010 = mSrc->getValue(node->getCorner4());
        Real f011 = mSrc->getValue(node->getCorner7());
        Real f100 = mSrc->getValue(node->getCorner1());
        Real f101 = mSrc->getValue(node->getCorner2());
        Real f110 = mSrc->getValue(node->getCorner5());
        Real f111 = mSrc->getValue(to);
    
        Vector3 gradients[19];
        gradients[9] = Vector3(centerValue.x, centerValue.y, centerValue.z);

        Vector3 positions[19][2] = {
            {node->getCenterBackBottom(), Vector3((Real)0.5, (Real)0.0, (Real)0.0)},
            {node->getCenterLeftBottom(), Vector3((Real)0.0, (Real)0.0, (Real)0.5)},
            {node->getCenterBottom(), Vector3((Real)0.5, (Real)0.0, (Real)0.5)},
            {node->getCenterRightBottom(), Vector3((Real)1.0, (Real)0.0, (Real)0.5)},
            {node->getCenterFrontBottom(), Vector3((Real)0.5, (Real)0.0, (Real)1.0)},

            {node->getCenterBackLeft(), Vector3((Real)0.0, (Real)0.5, (Real)0.0)},
            {node->getCenterBack(), Vector3((Real)0.5, (Real)0.5, (Real)0.0)},
            {node->getCenterBackRight(), Vector3((Real)1.0, (Real)0.5, (Real)0.0)},
            {node->getCenterLeft(), Vector3((Real)0.0, (Real)0.5, (Real)0.5)},
            {node->getCenter(), Vector3((Real)0.5, (Real)0.5, (Real)0.5)},
            {node->getCenterRight(), Vector3((Real)1.0, (Real)0.5, (Real)0.5)},
            {node->getCenterFrontLeft(), Vector3((Real)0.0, (Real)0.5, (Real)1.0)},
            {node->getCenterFront(), Vector3((Real)0.5, (Real)0.5, (Real)1.0)},
            {node->getCenterFrontRight(), Vector3((Real)1.0, (Real)0.5, (Real)1.0)},
        
            {node->getCenterBackTop(), Vector3((Real)0.5, (Real)1.0, (Real)0.0)},
            {node->getCenterLeftTop(), Vector3((Real)0.0, (Real)1.0, (Real)0.5)},
            {node->getCenterTop(), Vector3((Real)0.5, (Real)1.0, (Real)0.5)},
            {node->getCenterRightTop(), Vector3((Real)1.0, (Real)1.0, (Real)0.5)},
            {node->getCenterFrontTop(), Vector3((Real)0.5, (Real)1.0, (Real)1.0)}
        };

    
        Real error = (Real)0.0;
        Real interpolated, gradientMagnitude;
        Vector4 value;
        Vector3 gradient;
        for (size_t i = 0; i < 19; ++i)
        {
            value = mSrc->getValueAndGradient(positions[i][0]);
            gradient.x = value.x;
            gradient.y = value.y;
            gradient.z = value.z;
            interpolated = interpolate(f000, f001, f010, f011, f100, f101, f110, f111, positions[i][1]);
            gradientMagnitude = gradient.length();
            if (gradientMagnitude < FLT_EPSILON)
            {
                gradientMagnitude = (Real)1.0;
            }
            error += Math::Abs(value.w - interpolated) / gradientMagnitude;
            if (error >= geometricError)
            {
                return true;
            }
        }
        node->setCenterValue(centerValue);
        return false;
    }

}
}