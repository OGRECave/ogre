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
OgreOctree.h  -  description
-----------------------------------------------------------------------------
begin                : Mon Sep 30 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Modified slightly for use with PCZSceneManager Octree Zones by Eric Cha

-----------------------------------------------------------------------------
*/

#ifndef OCTREE_H
#define OCTREE_H

#include <OgreAxisAlignedBox.h>
#include <OgreWireBoundingBox.h>

#include <list>

namespace Ogre
{

class PCZSceneNode;
class PCZone;

typedef set< PCZSceneNode * >::type PCZSceneNodeList;


/** Octree datastructure for managing scene nodes.
@remarks
This is a loose octree implementation, meaning that each
octant child of the octree actually overlaps it's siblings by a factor
of .5.  This guarantees that any thing that is half the size of the parent will
fit completely into a child, with no splitting necessary.
*/

class Octree : public SceneCtlAllocatedObject
{
public:
    Octree( PCZone * zone, Octree * p );
    ~Octree();

    /** Adds an PCZscene node to this octree level.
    @remarks
    This is called by the OctreeZone after
    it has determined the correct Octree to insert the node into.
    */
    void _addNode( PCZSceneNode * );

    /** Removes an PCZscene node to this octree level.
    */
    void _removeNode( PCZSceneNode * );

    /** Returns the number of scene nodes attached to this octree
    */
    int numNodes()
    {
        return mNumNodes;
    };

    /** The bounding box of the octree
    @remarks
    This is used for octant index determination and rendering, but not culling
    */
    AxisAlignedBox mBox;
    WireBoundingBox* mWireBoundingBox;
    
    /** Creates the wire frame bounding box for this octant
    */
    WireBoundingBox* getWireBoundingBox();

    /** Vector containing the dimensions of this octree / 2
    */
    Vector3 mHalfSize;

    /** 3D array of children of this octree.
    @remarks
    Children are dynamically created as needed when nodes are inserted in the Octree.
    If, later, all the nodes are removed from the child, it is still kept around.
    */
    Octree * mChildren[ 2 ][ 2 ][ 2 ];

    /** Determines if this octree is twice as big as the given box.
    @remarks
    This method is used by the OctreeSceneManager to determine if the given
    box will fit into a child of this octree.
    */
    bool _isTwiceSize( const AxisAlignedBox &box ) const;

    /**  Returns the appropriate indexes for the child of this octree into which the box will fit.
    @remarks
    This is used by the OctreeSceneManager to determine which child to traverse next when
    finding the appropriate octree to insert the box.  Since it is a loose octree, only the
    center of the box is checked to determine the octant.
    */
    void _getChildIndexes( const AxisAlignedBox &, int *x, int *y, int *z ) const;

    /** Creates the AxisAlignedBox used for culling this octree.
    @remarks
    Since it's a loose octree, the culling bounds can be different than the actual bounds of the octree.
    */
    void _getCullBounds( AxisAlignedBox * ) const;

    /* Recurse through the Octree to find the scene nodes which intersect an aab
    */
    void _findNodes(const AxisAlignedBox &t, 
                    PCZSceneNodeList &list, 
                    PCZSceneNode *exclude, 
					bool includeVisitors,
                    bool full );

    /* Recurse through the Octree to find the scene nodes which intersect a ray
    */
    void _findNodes(const Ray &t, 
                    PCZSceneNodeList &list, 
                    PCZSceneNode *exclude, 
					bool includeVisitors,
                    bool full );

    /* Recurse through the Octree to find the scene nodes which intersect a sphere
    */
    void _findNodes(const Sphere &t, 
                    PCZSceneNodeList &list, 
                    PCZSceneNode *exclude, 
					bool includeVisitors,
                    bool full );

    /* Recurse through the Octree to find the scene nodes which intersect a PBV
    */
    void _findNodes(const PlaneBoundedVolume &t, 
                    PCZSceneNodeList &list, 
                    PCZSceneNode *exclude, 
					bool includeVisitors,
                    bool full );

    /** Public list of SceneNodes attached to this particular octree
    */
    PCZSceneNodeList mNodes;

	/* Zone that this octree is in */
	PCZone * mZone;

protected:

    /** Increments the overall node count of this octree and all its parents
    */
    inline void _ref()
    {
        mNumNodes++;

        if ( mParent != 0 ) mParent -> _ref();
    };

    /** Decrements the overall node count of this octree and all its parents
    */
    inline void _unref()
    {
        mNumNodes--;

        if ( mParent != 0 ) mParent -> _unref();
    };

    ///number of SceneNodes in this octree and all its children.
    int mNumNodes;

    ///parent octree
    Octree * mParent;

};

}

#endif


