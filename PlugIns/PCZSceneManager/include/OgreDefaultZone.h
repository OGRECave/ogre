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
DefaultZone.h  -  Default implementation of PCZone header file.

Default Implementation of PCZone
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef DEFAULTZONE_H
#define DEFAULTZONE_H

#include "OgrePCZone.h"

namespace Ogre
{
	class _OgrePCZPluginExport DefaultZone : public PCZone
    {
    public:
        DefaultZone( PCZSceneManager *, const String& );
        ~DefaultZone();

		/** Set the enclosure node for this Zone
		*/
		void setEnclosureNode(PCZSceneNode *);

        /** Adds an SceneNode to this Zone.
        @remarks
        The PCZSceneManager calls this function to add a node
        to the zone.  
        */
        void _addNode( PCZSceneNode * );

        /** Removes all references to a SceneNode from this Zone.
        */
        void removeNode( PCZSceneNode * );

		/** Indicates whether or not this zone requires zone-specific data for 
		 *  each scene node
		 */
		bool requiresZoneSpecificNodeData(void);

		/* Add a portal to the zone
		*/
		void _addPortal( Portal * );

		/* Remove a portal from the zone
		*/
		void _removePortal( Portal * );

		/** (recursive) check the given node against all portals in the zone
		*/
		void _checkNodeAgainstPortals(PCZSceneNode *, Portal * );

        /** (recursive) check the given light against all portals in the zone
        */
        void _checkLightAgainstPortals(PCZLight *, 
                                       long, 
                                       PCZFrustum *,
                                       Portal *);

		/* Update the zone data for each portal 
		*/
		void updatePortalsZoneData(void);

		/** Mark nodes dirty base on moving portals. */
		void dirtyNodeByMovingPortals(void);

		/* Update a node's home zone */
		PCZone * updateNodeHomeZone(PCZSceneNode * pczsn, bool allowBackTouces);

        /** Find and add visible objects to the render queue.
        @remarks
        Starts with objects in the zone and proceeds through visible portals   
        This is a recursive call (the main call should be to _findVisibleObjects)
        */
        void findVisibleNodes(PCZCamera *, 
							  NodeList & visibleNodeList,
							  RenderQueue * queue,
							  VisibleObjectsBoundsInfo* visibleBounds, 
							  bool onlyShadowCasters,
							  bool displayNodes,
							  bool showBoundingBoxes);

		/* Functions for finding Nodes that intersect various shapes */
		void _findNodes( const AxisAlignedBox &t, 
						 PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
						 bool includeVisitors,
						 bool recurseThruPortals,
						 PCZSceneNode *exclude);
	    void _findNodes( const Sphere &t, 
						 PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
						 bool includeVisitors,
						 bool recurseThruPortals,
						 PCZSceneNode *exclude );
	    void _findNodes( const PlaneBoundedVolume &t, 
						 PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
						 bool includeVisitors,
						 bool recurseThruPortals,
						 PCZSceneNode *exclude );
	    void _findNodes( const Ray &t, 
						 PCZSceneNodeList &list, 
                         PortalList &visitedPortals,
						 bool includeVisitors,
						 bool recurseThruPortals,
						 PCZSceneNode *exclude );

		/** Sets the options for the Zone */
		bool setOption( const String &, const void * );

		/** called when the scene manager creates a camera because
		    some zone managers (like TerrainZone) need the camera info.
		*/
		void notifyCameraCreated( Camera* c );
		/* called by PCZSM during setWorldGeometryRenderQueue() */
		virtual void notifyWorldGeometryRenderQueue(uint8 qid);
		/* Called when a _renderScene is called in the SceneManager */
		virtual void notifyBeginRenderScene(void);
		/* called by PCZSM during setZoneGeometry() */
		virtual void setZoneGeometry(const String &filename, PCZSceneNode * parentNode);

	protected:

    };

}

#endif



