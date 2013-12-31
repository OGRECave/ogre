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
#ifndef __Ogre_Volume_OctreeNode_H__
#define __Ogre_Volume_OctreeNode_H__

#include "OgreMatrix3.h"
#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreManualObject.h"
#include "OgreVolumeOctreeNodeSplitPolicy.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    class OctreeNodeSplitPolicy;

    /** A node in the volume octree.
    */
    class _OgreVolumeExport OctreeNode : public UtilityAlloc
    {
    protected:
        
        /// Factor to the diagonal of the cell to decide whether this cell is near the isosurface or not.
        static const Real NEAR_FACTOR;

        /// To count some indices while creating the debug view and recursing through the instances.
        static uint32 mGridPositionCount;

        /// To give the debug manual object an unique name.
        static size_t mNodeI;

        /// The back lower left corner of the cell.
        Vector3 mFrom;
        
        /// The front upper right corner of the cell.
        Vector3 mTo;

        /// The children of this node.
        OctreeNode **mChildren;

        /// Holds the debug visualization of the octree. Just set in the root.
        Entity* mOctreeGrid;

        /// Density and gradient of the center.
        Vector4 mCenterValue;

        /** Method to actually add the lines of the octree cells to
            the debug visualization.
        @param manual
            The manual object to add the lines to if this is a leaf in the octree.
        */
        void buildOctreeGridLines(ManualObject *manual) const;
    public:

        /// Even in an OCtree, the amount of children should not be hardcoded.
        static const size_t OCTREE_CHILDREN_COUNT;
        
        /** Gets the center and width / height / depth vector of the children of a node.
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @param center
            Out parameter of the calculated center.
        @param width
            Out parameter of the width vector (width in x, rest zero).
        @param height
            Out parameter of the height vector (height in y, rest zero).
        @param depth
            Out parameter of the depth vector (depth in z, rest zero).
        */
        static inline void getChildrenDimensions(const Vector3 &from, const Vector3 &to, Vector3 &center, Vector3 &width, Vector3 &height, Vector3 &depth)
        {
            center = (to - from) / (Real)2.0;
            width.x = center.x;
            width.y = (Real)0.0;
            width.z = (Real)0.0;
            height.x = (Real)0.0;
            height.y = center.y;
            height.z = (Real)0.0;
            depth.x = (Real)0.0;
            depth.y = (Real)0.0;
            depth.z = center.z;
            center += from;
        }

        /** Constructor.
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        */
        OctreeNode(const Vector3 &from = Vector3::ZERO, const Vector3 &to = Vector3::ZERO);

        /** Destructor.
        */
        virtual ~OctreeNode(void);

        /** Factory method to create octree nodes.
        @param from
            The back lower left corner of the cell.
        @param to
            The front upper right corner of the cell.
        @return
            The created entity. Make sure to destroy it when you don't need it anymore.
        */
        virtual OctreeNode* createInstance(const Vector3& from, const Vector3& to);

        /** Splits this cell if the split policy says so.
        @param splitPolicy
            Defines the policy deciding whether to split this node or not.
        @param src
            The volume source.
        @param geometricError
            The accepted geometric error.
        */
        void split(const OctreeNodeSplitPolicy *splitPolicy, const Source *src, const Real geometricError);

        /** Getter for the octree debug visualization of the octree starting with
            this node.
        @param sceneManager
            The scenemanager creating the actual entity.
        @return
            The lazily created debug visualization.
        */
        Entity* getOctreeGrid(SceneManager *sceneManager);

        /** Setter for the from-part of this cell.
        @param from
            The back lower left corner of the cell.
        */
        inline void setFrom(Vector3 from)
        {
            mFrom = from;
        }
        
        /** Setter for the to-part of this cell.
        @param to
            The front upper right corner of the cell.
        */
        inline void setTo(Vector3 to)
        {
            mTo = to;
        }

        /** Gets whether this cell has any children.
        @return
            True if so.
        */
        inline bool isSubdivided(void) const 
        {
            return mChildren != 0;
        }

        /** Gets an octree child. Enumeration:
              4 5
             7 6
              0 1
             3 2
         @param i
            The child index.
         @return
            The child.
         */
        inline const OctreeNode* getChild(const size_t i) const
        {
            return mChildren[i];
        }
        
        /** Gets the center of this cell.
        @return
            The center.
        */
        inline const Vector3 getCenter(void) const
        {
            return (mFrom + mTo) / (Real)2.0;
        }
        
        /** Gets the back lower left corner of the cell.
        @return
            The back lower left corner of the cell.
        */
        inline const Vector3& getFrom(void) const
        {
            return mFrom;
        }
        
        /** Gets the front upper right corner of the cell.
        @return
            The front upper right corner of the cell.
        */
        inline const Vector3& getTo(void) const
        {
            return mTo;
        }

        /** Gets whether this cell is at the left of the given root cell.
        @param root
            The octree root node to test against.
        @return
            true if so.
        */
        inline bool isBorderLeft(const OctreeNode &root) const
        {
            return mFrom.x == root.mFrom.x;
        }
        
        /** Gets whether this cell is at the right of the given root cell.
        @param root
            The octree root node to test against.
        @return
            true if so.
        */
        inline bool isBorderRight(const OctreeNode &root) const
        {
            return mTo.x == root.mTo.x;
        }
        
        /** Gets whether this cell is at the bottom of the given root cell.
        @param root
            The octree root node to test against.
        @return
            true if so.
        */
        inline bool isBorderBottom(const OctreeNode &root) const
        {
            return mFrom.y == root.mFrom.y;
        }
        
        /** Gets whether this cell is at the top of the given root cell.
        @param root
            The octree root node to test against.
        @return
            true if so.
        */
        inline bool isBorderTop(const OctreeNode &root) const
        {
            return mTo.y == root.mTo.y;
        }
        
        /** Gets whether this cell is at the back of the given root cell.
        @param root
            The octree root node to test against.
        @return
            true if so.
        */
        inline bool isBorderBack(const OctreeNode &root) const
        {
            return mFrom.z == root.mFrom.z;
        }
        
        /** Gets whether this cell is at the front of the given root cell.
        @param root
            The octree root node to test against.
        @return
            true if so.
        */
        inline bool isBorderFront(const OctreeNode &root) const
        {
            return mTo.z == root.mTo.z;
        }

        /** Gets the center of the corners 0, 1, 4, 5.
        @return
            The center.
        */
        inline const Vector3 getCenterBack(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mFrom.z);
        }
        
        /** Gets the center of the corners 2, 3, 6, 7.
        @return
            The center.
        */
        inline const Vector3 getCenterFront(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mTo.z);
        }
        
        /** Gets the center of the corners 0, 3, 4, 6.
        @return
            The center.
        */
        inline const Vector3 getCenterLeft(void) const
        {
            return Vector3(mFrom.x, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 1, 2, 5, 6.
        @return
            The center.
        */
        inline const Vector3 getCenterRight(void) const
        {
            return Vector3(mTo.x, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 4, 5, 6, 7.
        @return
            The center.
        */
        inline const Vector3 getCenterTop(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mTo.y, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 0, 1, 2, 3.
        @return
            The center.
        */
        inline const Vector3 getCenterBottom(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mFrom.y, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 4, 5.
        @return
            The center.
        */
        inline const Vector3 getCenterBackTop(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mTo.y, mFrom.z);
        }
        
        /** Gets the center of the corners 0, 1.
        @return
            The center.
        */
        inline const Vector3 getCenterBackBottom(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mFrom.y, mFrom.z);
        }
        
        /** Gets the center of the corners 6, 7.
        @return
            The center.
        */
        inline const Vector3 getCenterFrontTop(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mTo.y, mTo.z);
        }
        
        /** Gets the center of the corners 2, 3.
        @return
            The center.
        */
        inline const Vector3 getCenterFrontBottom(void) const
        {
            return Vector3(mFrom.x + (mTo.x - mFrom.x) / (Real)2.0, mFrom.y, mTo.z);
        }
        
        /** Gets the center of the corners 4, 7.
        @return
            The center.
        */
        inline const Vector3 getCenterLeftTop(void) const
        {
            return Vector3(mFrom.x, mTo.y, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 0, 3.
        @return
            The center.
        */
        inline const Vector3 getCenterLeftBottom(void) const
        {
            return Vector3(mFrom.x, mFrom.y, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 5, 6.
        @return
            The center.
        */
        inline const Vector3 getCenterRightTop(void) const
        {
            return Vector3(mTo.x, mTo.y, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 1, 2.
        @return
            The center.
        */
        inline const Vector3 getCenterRightBottom(void) const
        {
            return Vector3(mTo.x, mFrom.y, mFrom.z + (mTo.z - mFrom.z) / (Real)2.0);
        }
        
        /** Gets the center of the corners 0, 4.
        @return
            The center.
        */
        inline const Vector3 getCenterBackLeft(void) const
        {
            return Vector3(mFrom.x, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mFrom.z);
        }

        /** Gets the center of the corners 3, 7.
        @return
            The center.
        */
        inline const Vector3 getCenterFrontLeft(void) const
        {
            return Vector3(mFrom.x, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mTo.z);
        }
        
        /** Gets the center of the corners 1, 5.
        @return
            The center.
        */
        inline const Vector3 getCenterBackRight(void) const
        {
            return Vector3(mTo.x, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mFrom.z);
        }

        /** Gets the center of the corners 2, 6.
        @return
            The center.
        */
        inline const Vector3 getCenterFrontRight(void) const
        {
            return Vector3(mTo.x, mFrom.y + (mTo.y - mFrom.y) / (Real)2.0, mTo.z);
        }

        /** Gets the coordinate of corner 1.
        @return
            The corner.
        */
        inline const Vector3 getCorner1(void) const
        {
            return Vector3(mTo.x, mFrom.y, mFrom.z);
        }
        
        /** Gets the coordinate of corner 2.
        @return
            The corner.
        */
        inline const Vector3 getCorner2(void) const
        {
            return Vector3(mTo.x, mFrom.y, mTo.z);
        }
        
        /** Gets the coordinate of corner 3.
        @return
            The corner.
        */
        inline const Vector3 getCorner3(void) const
        {
            return Vector3(mFrom.x, mFrom.y, mTo.z);
        }
        
        /** Gets the coordinate of corner 4.
        @return
            The corner.
        */
        inline const Vector3 getCorner4(void) const
        {
            return Vector3(mFrom.x, mTo.y, mFrom.z);
        }
        
        /** Gets the coordinate of corner 5.
        @return
            The corner.
        */
        inline const Vector3 getCorner5(void) const
        {
            return Vector3(mTo.x, mTo.y, mFrom.z);
        }
        
        /** Gets the coordinate of corner 7.
        @return
            The corner.
        */
        inline const Vector3 getCorner7(void) const
        {
            return Vector3(mFrom.x, mTo.y, mTo.z);
        }

        /** Raw setter for the center value.
        @param value
            The density value.
        */
        inline void setCenterValue(Vector4 value)
        {
            mCenterValue = value;
        }

        /** Gets the center value.
        @return
            The center value, one Vector4 consisting of gradient (x, y, z) and density (w).
        */
        inline const Vector4 getCenterValue(void) const
        {
            return mCenterValue;
        }

        /** Gets whether the isosurface is somewhat near to this node.
        @return
            true if somewhat near.
        */
        inline bool isIsoSurfaceNear(void) const
        {
            if (mCenterValue.w == (Real)0.0)
            {
                return true;
            }
            return Math::Abs(mCenterValue.w) < (mFrom - mTo).length() * NEAR_FACTOR;
        }
    };
}
}

#endif