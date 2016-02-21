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
#ifndef __Ogre_Volume_OctreeNodeSplitPolicy_H__
#define __Ogre_Volume_OctreeNodeSplitPolicy_H__

#include "OgreVector3.h"

#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    class OctreeNode;
    class Source;

    /** The class deciding on whether to split an octree node or not
        when building the octree.
        Splitting like Zhang in http://www.andrew.cmu.edu/user/jessicaz/publication/meshing/.
    */
    class _OgreVolumeExport OctreeNodeSplitPolicy
    {
    protected:

        /// Holds the volume source to decide something.
        const Source *mSrc;
        
        /// The maximum cell size where the policy stops the splitting.
        Real mMaxCellSize;
        
        /** Trilinear interpolation of a relative point.
        @param f000
            Value of the lower back left corner.
        @param f001
            Value of the lower front right corner.
        @param f010
            Value of the upper back left corner.
        @param f011
            Value of the upper front left corner.
        @param f100
            Value of the lower back right corner.
        @param f101
            Value of the lower back right corner.
        @param f110
            Value of the upper front right corner.
        @param f111
            Value of the upper front right corner.
        @param position
            The relative (0-1) position to interpolate.
        @return
            The interpolated value.
        */
        inline Real interpolate(const Real f000, const Real f001, const Real f010, const Real f011,
            const Real f100, const Real f101, const Real f110, const Real f111, const Vector3 &position) const
        {
            Real oneMinX = (Real)1.0 - position.x;
            Real oneMinY = (Real)1.0 - position.y;
            Real oneMinZ = (Real)1.0 - position.z;
            Real oneMinXoneMinY = oneMinX * oneMinY;
            Real xOneMinY = position.x * oneMinY;
            return oneMinZ * (f000 * oneMinXoneMinY
                + f100 * xOneMinY
                + f010 * oneMinX * position.y)
                + position.z * (f001 * oneMinXoneMinY
                + f101 * xOneMinY
                + f011 * oneMinX * position.y)
                + position.x * position.y * (f110 * oneMinZ
                + f111 * position.z);
        }

    public:
        
        /** Constructur. Protected to have the initialization.
        @param src
            The volume source to decide something.
        @param maxCellSize
            The maximum size when the splitting will stop anyway.
        */
        OctreeNodeSplitPolicy(const Source *src, const Real maxCellSize);
                
        /** Decider for the splitting.
        @param node
            The split candidate.
        @param geometricError
            The accepted geometric error.
        @return
            true if the node should be split.
        */
        bool doSplit(OctreeNode *node, const Real geometricError) const;
    };

}
}

#endif