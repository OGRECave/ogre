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

#ifndef DEFAULTZONE_H
#define DEFAULTZONE_H

#include "OgrePCZone.h"

namespace Ogre
{
    class PCZFrustum;
    struct VisibleObjectsBoundsInfo;

    class _OgrePCZPluginExport DefaultZone : public PCZone
    {
    public:
        DefaultZone( PCZSceneManager *, const String& );
        ~DefaultZone();

        /** Set the enclosure node for this Zone
        */
        void setEnclosureNode(PCZSceneNode *) override;

        /** Adds an SceneNode to this Zone.

        The PCZSceneManager calls this function to add a node
        to the zone.  
        */
        void _addNode( PCZSceneNode * ) override;

        /** Removes all references to a SceneNode from this Zone.
        */
        void removeNode( PCZSceneNode * ) override;

        /** Indicates whether or not this zone requires zone-specific data for 
         *  each scene node
         */
        bool requiresZoneSpecificNodeData(void) override;

        /** (recursive) check the given node against all portals in the zone
        */
        void _checkNodeAgainstPortals(PCZSceneNode *, Portal * ) override;

        /** (recursive) check the given light against all portals in the zone
        */
        void _checkLightAgainstPortals(PCZLight *, 
                                       unsigned long, 
                                       PCZFrustum *,
                                       Portal *) override;

        /* Update the zone data for each portal 
        */
        void updatePortalsZoneData(void) override;

        /** Mark nodes dirty base on moving portals. */
        void dirtyNodeByMovingPortals(void) override;

        /* Update a node's home zone */
        PCZone * updateNodeHomeZone(PCZSceneNode * pczsn, bool allowBackTouces) override;

        /** Find and add visible objects to the render queue.

        Starts with objects in the zone and proceeds through visible portals   
        This is a recursive call (the main call should be to _findVisibleObjects)
        */
        void findVisibleNodes(PCZCamera *, 
                              NodeList & visibleNodeList,
                              RenderQueue * queue,
                              VisibleObjectsBoundsInfo* visibleBounds, 
                              bool onlyShadowCasters,
                              bool displayNodes,
                              bool showBoundingBoxes) override;

        /* Functions for finding Nodes that intersect various shapes */
        void _findNodes( const AxisAlignedBox &t, 
                         PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
                         bool includeVisitors,
                         bool recurseThruPortals,
                         PCZSceneNode *exclude) override;
        void _findNodes( const Sphere &t, 
                         PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
                         bool includeVisitors,
                         bool recurseThruPortals,
                         PCZSceneNode *exclude ) override;
        void _findNodes( const PlaneBoundedVolume &t, 
                         PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
                         bool includeVisitors,
                         bool recurseThruPortals,
                         PCZSceneNode *exclude ) override;
        void _findNodes( const Ray &t, 
                         PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
                         bool includeVisitors,
                         bool recurseThruPortals,
                         PCZSceneNode *exclude ) override;

        /** Sets the options for the Zone */
        bool setOption( const String &, const void * ) override;

        /** called when the scene manager creates a camera because
            some zone managers (like TerrainZone) need the camera info.
        */
        void notifyCameraCreated( Camera* c ) override;
        /* called by PCZSM during setWorldGeometryRenderQueue() */
        void notifyWorldGeometryRenderQueue(uint8 qid) override;
        /* Called when a _renderScene is called in the SceneManager */
        void notifyBeginRenderScene(void) override;
        /* called by PCZSM during setZoneGeometry() */
        void setZoneGeometry(const String &filename, PCZSceneNode * parentNode) override;

    protected:

    };

}

#endif



