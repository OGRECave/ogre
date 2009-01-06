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
/***************************************************************************
octree.h  -  description
-------------------
begin                : Mon Sep 30 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team
***************************************************************************/

#ifndef OCTREE_H
#define OCTREE_H

#include <OgreAxisAlignedBox.h>
#include <OgreWireBoundingBox.h>

#include <list>

namespace Ogre
{

class OctreeNode;

typedef list< OctreeNode * >::type NodeList;


/** Octree datastructure for managing scene nodes.
@remarks
This is a loose octree implementation, meaning that each
octant child of the octree actually overlaps it's siblings by a factor
of .5.  This guarantees that any thing that is half the size of the parent will
fit completely into a child, with no splitting necessary.
*/

class Octree : public NodeAlloc
{
public:
    Octree( Octree * p );
    ~Octree();

    /** Adds an Octree scene node to this octree level.
    @remarks
    This is called by the OctreeSceneManager after
    it has determined the correct Octree to insert the node into.
    */
    void _addNode( OctreeNode * );

    /** Removes an Octree scene node to this octree level.
    */
    void _removeNode( OctreeNode * );

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


    /** Public list of SceneNodes attached to this particular octree
    */
    NodeList mNodes;

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


