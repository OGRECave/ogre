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
OctreeZone.h  -  Portal Connected Zone (OctreeZone) header file.

OctreeZones are a type of PCZone.  Octree Zones partition their space into
Octants.  For details on Zones in general, see PCZone.h/cpp.
-----------------------------------------------------------------------------
begin                : Mon Apr 16 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    :
-----------------------------------------------------------------------------
*/

#ifndef OCTREEZONE_H
#define OCTREEZONE_H

#include "OgreOctreeZonePrerequisites.h"
#include "OgrePCZone.h"
#include "OgrePCZoneFactory.h"

namespace Ogre
{
    /** Octree version of PCZone 
    */
    class Octree;
    class OctreeZoneData;

    class _OgreOctreeZonePluginExport OctreeZone : public PCZone
    {
    public:
        OctreeZone( PCZSceneManager *, const String& );
        virtual ~OctreeZone();

        /** Set the enclosure node for this OctreeZone
        */
        virtual void setEnclosureNode(PCZSceneNode *);

        /** Adds an SceneNode to this OctreeZone.
        @remarks
        The PCZSceneManager calls this function to add a node
        to the zone.  
        */
        virtual void _addNode( PCZSceneNode * );

        /** Removes all references to a SceneNode from this Zone.
        */
        virtual void removeNode( PCZSceneNode * );

        /** Remove all nodes from the node reference list and clear it
        */
        virtual void _clearNodeLists(short nodeListTypes);

        /** Indicates whether or not this zone requires zone-specific data for 
         *  each scene node
         */
        virtual bool requiresZoneSpecificNodeData(void);

        /** Create zone specific data for a node
        */
        virtual void createNodeZoneData(PCZSceneNode *);

        /** (recursive) Check the given node against all portals in the zone
        */
        virtual void _checkNodeAgainstPortals(PCZSceneNode *, Portal * );

        /** (recursive) Check the given light against all portals in the zone
        */
        virtual void _checkLightAgainstPortals(PCZLight *, 
                                               unsigned long, 
                                               PCZFrustum *,
                                               Portal*);

        /** Update the zone data for each portal
        */
        void updatePortalsZoneData(void);

        /** Mark nodes dirty base on moving portals. */
        void dirtyNodeByMovingPortals(void);

        /** Update a node's home zone */
        virtual PCZone * updateNodeHomeZone(PCZSceneNode * pczsn, bool allowBackTouces);

        /** Find and add visible objects to the render queue.
        @remarks
        Starts with objects in the zone and proceeds through visible portals   
        This is a recursive call (the main call should be to _findVisibleObjects)
        */
        virtual void findVisibleNodes(PCZCamera *, 
                                      NodeList & visibleNodeList,
                                      RenderQueue * queue,
                                      VisibleObjectsBoundsInfo* visibleBounds, 
                                      bool onlyShadowCasters,
                                      bool displayNodes,
                                      bool showBoundingBoxes);

        /** Functions for finding Nodes that intersect various shapes */
        virtual void _findNodes(const AxisAlignedBox &t, 
                                PCZSceneNodeList &list,
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude);
        virtual void _findNodes(const Sphere &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude );
        virtual void _findNodes(const PlaneBoundedVolume &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude );
        virtual void _findNodes(const Ray &t, 
                                PCZSceneNodeList &list, 
                                PortalList &visitedPortals,
                                bool includeVisitors,
                                bool recurseThruPortals,
                                PCZSceneNode *exclude );

        /** Sets the given option for the Zone
         @remarks
            Options are:
            "Size", AxisAlignedBox *;
            "Depth", int *;
            "ShowOctree", bool *;
        */
        virtual bool setOption( const String &, const void * );

        /** Called when the scene manager creates a camera because
            some zone managers (like TerrainZone) need the camera info.
        */
        virtual void notifyCameraCreated( Camera* c );

        /** Called by PCZSM during setWorldGeometryRenderQueue() */
        virtual void notifyWorldGeometryRenderQueue(uint8 qid);

        /** Called when a _renderScene is called in the SceneManager */
        virtual void notifyBeginRenderScene(void);

        /** Called by PCZSM during setZoneGeometry() */
        virtual void setZoneGeometry(const String &filename, PCZSceneNode * parentNode);

        /** Get the world coordinate aabb of the zone */
        virtual void getAABB(AxisAlignedBox &);

        /// Init function carried over from OctreeSceneManager
        void init(AxisAlignedBox &box, int depth);
        /** Resizes the octree to the given size */
        void resize( const AxisAlignedBox &box );
        /** Checks the given OctreeNode, and determines if it needs to be moved
        * to a different octant.
        */
        void updateNodeOctant( OctreeZoneData * zoneData );
        /** Removes the node from the octree it is in */
        void removeNodeFromOctree( PCZSceneNode * );
        /** Adds the Octree Node, starting at the given octree, and recursing at max to the specified depth.
        */
        void addNodeToOctree( PCZSceneNode *, Octree *octree, int depth = 0 );


    protected:
        /** Walks through the octree, adding any visible objects to the render queue.
        @remarks
        If any octant in the octree if completely within the view frustum,
        all subchildren are automatically added with no visibility tests.
        */
        void walkOctree( PCZCamera *, 
                         NodeList &,
                         RenderQueue *, 
                         Octree *, 
                         VisibleObjectsBoundsInfo* visibleBounds, 
                         bool foundvisible, 
                         bool onlyShadowCasters,
                         bool displayNodes,
                         bool showBoundingBoxes);

    protected:
        /// The root octree
        Octree *mOctree;
        /// Max depth for the tree
        int mMaxDepth;
        /// Size of the octree
        AxisAlignedBox mBox;
    };

    class _OgreOctreeZonePluginExport OctreeZoneData : public ZoneData
    {
    public:
        /** Standard Constructor */
        OctreeZoneData(PCZSceneNode *, PCZone * );
        /** Standard destructor */
        ~OctreeZoneData();
        /** Update data */
        void update(void);

        /** Returns the Octree in which this OctreeNode resides
        */
        Octree * getOctant()
        {
            return mOctant;
        };
        /** Sets the Octree in which this OctreeNode resides
        */
        void setOctant( Octree *o )
        {
            mOctant = o;
        };
        bool _isIn( AxisAlignedBox &box );

    public:
        /// Octree this node is attached to.
        Octree *        mOctant;
        /// Octree-specific world bounding box (only includes attached objects, not children)
        AxisAlignedBox  mOctreeWorldAABB;
    };

    /// Factory for OctreeZone
    class OctreeZoneFactory : public PCZoneFactory
    {
    public:
        OctreeZoneFactory();
        virtual ~OctreeZoneFactory();

        bool supportsPCZoneType(const String& zoneType);
        PCZone* createPCZone(PCZSceneManager * pczsm, const String& zoneName);
    };

}

#endif



