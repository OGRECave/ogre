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
#ifndef _BspNode_H__
#define _BspNode_H__

#include "OgreBspPrerequisites.h"
#include "OgrePlane.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSceneQuery.h"

namespace Ogre {

    /** Encapsulates a node in a BSP tree.
        A BSP tree represents space partitioned by planes . The space which is
        partitioned is either the world (in the case of the root node) or the space derived
        from their parent node. Each node can have elements which are in front or behind it, which are
        it's children and these elements can either be further subdivided by planes,
        or they can be undivided spaces or 'leaf nodes' - these are the nodes which actually contain
        objects and world geometry.The leaves of the tree are the stopping point of any tree walking algorithm,
        both for rendering and collision detection etc.</p>
        Ogre chooses not to represent splitting nodes and leaves as separate structures, but to merge the two for simplicity
        of the walking algorithm. If a node is a leaf, the isLeaf() method returns true and both getFront() and
        getBack() return null pointers. If the node is a partitioning plane isLeaf() returns false and getFront()
        and getBack() will return the corresponding BspNode objects.
    */
	class BspNode : public NodeAlloc
    {
        friend class BspLevel;

    public:
        /** Constructor, only to be used by BspLevel. */
        BspNode(BspLevel* owner, bool isLeaf);

        BspNode();
        ~BspNode();

        /** Returns true if this node is a leaf (i.e. contains geometry) or false if it is a splitting plane.
            A BspNode can either be a splitting plane (the typical representation of a BSP node) or an undivided
            region contining geometry (a leaf node). Ogre represents both using the same class for simplicity
            of tree walking. However it is important that you use this method to determine which type you are dealing
            with, since certain methods are only supported with one of the subtypes. Details are given in the individual methods.
            Note that I could have represented splitting / leaf nodes as a class hierarchy but the
            virtual methods / run-time type identification would have a performance hit, and it would not make the
            code much (any?) simpler anyway. I think this is a fair trade-off in this case.
        */
        bool isLeaf(void) const;

        /** Returns a pointer to a BspNode containing the subspace on the positive side of the splitting plane.
            This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
            method on a leaf node will throw an exception.
        */
        BspNode* getFront(void) const;

        /** Returns a pointer to a BspNode containing the subspace on the negative side of the splitting plane.
            This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
            method on a leaf node will throw an exception.
        */
        BspNode* getBack(void) const;

        /** Determines which side of the splitting plane a worldspace point is.
            This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
            method on a leaf node will throw an exception.
        */
        Plane::Side getSide (const Vector3& point) const;

        /** Gets the next node down in the tree, with the intention of
            locating the leaf containing the given point.
            This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
            method on a leaf node will throw an exception.
        */
        BspNode* getNextNode(const Vector3& point) const;


        /** Returns details of the plane which is used to subdivide the space of his node's children.
            This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
            method on a leaf node will throw an exception.
        */
        const Plane& getSplitPlane(void) const;

        /** Returns the axis-aligned box which contains this node if it is a leaf.
            This method should only be called on a leaf node. It returns a box which can be used in calls like
            Camera::isVisible to determine if the leaf node is visible in the view.
        */
        const AxisAlignedBox& getBoundingBox(void) const;

        /** Returns the number of faces contained in this leaf node.
            Should only be called on a leaf node.
        */
        int getNumFaceGroups(void) const;
        /** Returns the index to the face group index list for this leaf node.
            The contents of this buffer is a list of indexes which point to the
            actual face groups held in a central buffer in the BspLevel class (in
            actual fact for efficency the indexes themselves are also held in a single
            buffer in BspLevel too). The reason for this indirection is that the buffer
            of indexes to face groups is organised in chunks relative to nodes, whilst the
            main buffer of face groups may not be.
            Should only be called on a leaf node.
        */
        int getFaceGroupStart(void) const;

        /** Determines if the passed in node (must also be a leaf) is visible from this leaf.
            Must only be called on a leaf node, and the parameter must also be a leaf node. If
            this method returns true, then the leaf passed in is visible from this leaf.
            Note that internally this uses the Potentially Visible Set (PVS) which is precalculated
            and stored with the BSP level.
        */
        bool isLeafVisible(const BspNode* leaf) const;

        friend std::ostream& operator<< (std::ostream& o, BspNode& n);

        /// Internal method for telling the node that a movable intersects it
        void _addMovable(const MovableObject* mov);

        /// Internal method for telling the node that a movable no longer intersects it
        void _removeMovable(const MovableObject* mov);

        /// Gets the signed distance to the dividing plane
        Real getDistance(const Vector3& pos) const;

        typedef std::set<const MovableObject*> IntersectingObjectSet;

        struct Brush
        {
            std::list<Plane> planes;
            SceneQuery::WorldFragment fragment; // For query reporting
        };
        typedef std::vector<Brush*> NodeBrushList; // Main brush memory held on level

        /** Get the list of solid Brushes for this node.
        @remarks Only applicable for leaf nodes. 
        */
        const NodeBrushList& getSolidBrushes(void) const;
    protected:
        BspLevel* mOwner; // Back-reference to containing level
        bool mIsLeaf;

        // Node-only members
        /** The plane which splits space in a non-leaf node.
            Note that nodes do not allocate the memory for other nodes - for simplicity and bulk-allocation
            of memory the BspLevel is responsible for assigning enough memory for all nodes in one go.
        */
        Plane mSplitPlane;
        /** Pointer to the node in front of this non-leaf node. */
        BspNode* mFront;
        /** Pointer to the node behind this non-leaf node. */
        BspNode* mBack;

        // Leaf-only members
        /** The cluster number of this leaf.
            Leaf nodes are assigned to 'clusters' of nodes, which are used to group nodes together for
            visibility testing. There is a lookup table which is used to determine if one cluster of leaves
            is visible from another cluster. Whilst it would be possible to expand all this out so that
            each node had a list of pointers to other visible nodes, this would be very expensive in terms
            of storage (using the cluster method there is a table which is 1-bit squared per cluster, rounded
            up to the nearest byte obviously, which uses far less space than 4-bytes per linked node per source
            node). Of course the limitation here is that you have to each leaf in turn to determine if it is visible
            rather than just following a list, but since this is only done once per frame this is not such a big
            overhead.
        */
        int mVisCluster;

        /** The axis-aligned box which bounds node if it is a leaf. */
        AxisAlignedBox mBounds;
        /** Number of face groups in this node if it is a leaf. */
        int mNumFaceGroups;
        /** Index to the part of the main leaf facegroup index buffer(held in BspLevel) for this leaf.
            This leaf uses mNumFaceGroups from this pointer onwards. From here you use the index
            in this buffer to look up the actual face.
            Note that again for simplicity and bulk memory allocation the face
            group list itself is allocated by the BspLevel for all nodes, and each leaf node is given a section of it to
            work on. This saves lots of small memory allocations / deallocations which limits memory fragmentation.
        */
        int mFaceGroupStart;

        IntersectingObjectSet mMovables;

        NodeBrushList mSolidBrushes;
    public:
        const IntersectingObjectSet& getObjects(void) const { return mMovables; }


    };


}

#endif
