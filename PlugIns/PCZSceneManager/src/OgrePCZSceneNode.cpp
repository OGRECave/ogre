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
PCZSceneNode.cpp  -  PCZ-specific version of SceneNode

The PCZSceneNode is an extension used to store zone information and provide
additional functionality for a given Ogre::SceneNode.  A PCZSceneNode contains a 
pointer to the home zone for the node and a list of all zones being visited by
the node.  The PCZSceneManager contains a STD::MAP of PCZSceneNodes which are
keyed by the name of each node.  This allows quick lookup of
a given scenenode's PCZSceneNode by the scene manager.

-----------------------------------------------------------------------------
begin                : Sat Mar 24 2007
author               : Eric Cha
email                : ericc@xenopi.com
TODO				 : None known
-----------------------------------------------------------------------------
*/

#include <OgreRoot.h>
#include "OgrePCZSceneNode.h"
#include "OgreSceneNode.h"
#include "OgrePCZSceneManager.h"
#include "OgrePortal.h"
#include "OgrePCZone.h"
#include "OgrePCZCamera.h"

namespace Ogre
{
	PCZSceneNode::PCZSceneNode( SceneManager* creator ) : SceneNode( creator )
	{
		mHomeZone = 0;
		mAnchored = false;
		mAllowedToVisit = true;
		mLastVisibleFrame = 0;
		mLastVisibleFromCamera = 0;
		mEnabled = true;
	}
	PCZSceneNode::PCZSceneNode( SceneManager* creator, const String& name ) : SceneNode( creator, name )
	{
		mHomeZone = 0;
		mAnchored = false;
		mAllowedToVisit = true;
		mLastVisibleFrame = 0;
		mLastVisibleFromCamera = 0;
		mEnabled = true;
	}
	PCZSceneNode::~PCZSceneNode()
	{
		// clear visiting zones list
		mVisitingZones.clear();

		// delete zone data
		ZoneData* zoneData;
		ZoneDataMap::iterator it = mZoneData.begin();

		while ( it != mZoneData.end() )
		{
			zoneData = it->second;
			OGRE_DELETE zoneData;
			++it;
		}
		mZoneData.clear();
	}
	void PCZSceneNode::_update(bool updateChildren, bool parentHasChanged)
	{
		SceneNode::_update(updateChildren, parentHasChanged);

		mPrevPosition = mNewPosition;
		mNewPosition = _getDerivedPosition();   // do this way since _update is called through SceneManager::_updateSceneGraph which comes before PCZSceneManager::_updatePCZSceneNodes
	}
    //-----------------------------------------------------------------------
	SceneNode* PCZSceneNode::createChildSceneNode(const Vector3& translate, 
        const Quaternion& rotate)
	{
		PCZSceneNode * childSceneNode = (PCZSceneNode*)(this->createChild(translate, rotate));
		if (mAnchored)
		{
			childSceneNode->anchorToHomeZone(mHomeZone);
			mHomeZone->_addNode(childSceneNode);
		}
		return static_cast<SceneNode*>(childSceneNode);
	}
    //-----------------------------------------------------------------------
    SceneNode* PCZSceneNode::createChildSceneNode(const String& name, const Vector3& translate, 
		const Quaternion& rotate)
	{
		PCZSceneNode * childSceneNode = (PCZSceneNode*)(this->createChild(name, translate, rotate));
		if (mAnchored)
		{
			childSceneNode->anchorToHomeZone(mHomeZone);
			mHomeZone->_addNode(childSceneNode);
		}
		return static_cast<SceneNode*>(childSceneNode);
	}

	PCZone* PCZSceneNode::getHomeZone(void)
	{
		return mHomeZone;
	}
	void PCZSceneNode::setHomeZone(PCZone * zone)
	{
		// if the new home zone is different than the current, remove 
		// the node from the current home zone's list of home nodes first
		if (zone != mHomeZone && mHomeZone)
		{
			mHomeZone->removeNode(this);
		}
		mHomeZone = zone;
	}
	void PCZSceneNode::anchorToHomeZone(PCZone * zone)
	{
		mHomeZone = zone;
		mAnchored = true;
	}
	void PCZSceneNode::addZoneToVisitingZonesMap(PCZone * zone)
	{
		mVisitingZones[zone->getName()] = zone;
	}
	void PCZSceneNode::clearVisitingZonesMap(void)
	{
		mVisitingZones.clear();
	}
	/* The following function does the following:
	 * 1) Remove references to the node from zones the node is visiting
	 * 2) Clear the node's list of zones it is visiting
	 */
	void PCZSceneNode::clearNodeFromVisitedZones( void )
	{
		if (mVisitingZones.size() > 0)
		{
			// first go through the list of zones this node is visiting 
			// and remove references to this node
			PCZone* zone;
			ZoneMap::iterator it = mVisitingZones.begin();

			while ( it != mVisitingZones.end() )
			{
				zone = it->second;
				zone->removeNode(this);
				++it;
			}

			// second, clear the visiting zones list
			mVisitingZones.clear();

		}
	}

	/* Remove all references that the node has to the given zone
	*/
	void PCZSceneNode::removeReferencesToZone(PCZone * zone)
	{
		if (mHomeZone == zone)
		{
			mHomeZone = 0;
		}
		// search the map of visiting zones and remove
		ZoneMap::iterator i;
		i = mVisitingZones.find(zone->getName());
		if (i != mVisitingZones.end())
		{
			mVisitingZones.erase(i);
		}
	}
	
	/* returns true if zone is in the node's visiting zones map
	   false otherwise.
	*/
	bool PCZSceneNode::isVisitingZone(PCZone * zone)
	{
		ZoneMap::iterator i;
		i = mVisitingZones.find(zone->getName());
		if (i != mVisitingZones.end())
		{
			return true;
		}
		return false;
	}

    /** Adds the attached objects of this PCZSceneNode into the queue. */
    void PCZSceneNode::_addToRenderQueue( Camera* cam, 
                                          RenderQueue *queue, 
                                          bool onlyShadowCasters, 
                                          VisibleObjectsBoundsInfo* visibleBounds )
    {
		ObjectMap::iterator mit = mObjectsByName.begin();

		while ( mit != mObjectsByName.end() )
		{
			MovableObject * mo = mit->second;

			mo->_notifyCurrentCamera(cam);
			if ( mo->isVisible() &&
				(!onlyShadowCasters || mo->getCastShadows()))
			{
				mo -> _updateRenderQueue( queue );

			    if (visibleBounds)
			    {
				    visibleBounds->merge(mo->getWorldBoundingBox(true), 
					                     mo->getWorldBoundingSphere(true), 
                                         cam);
			    }
            }
			++mit;
        }
    }

	/** Save the node's current position as the previous position
	*/
	void PCZSceneNode::savePrevPosition(void)
	{
		mPrevPosition = _getDerivedPosition();
	}

	void PCZSceneNode::setZoneData(PCZone * zone, ZoneData * zoneData)
	{

		// first make sure that the data doesn't already exist
		if (mZoneData.find(zone->getName()) != mZoneData.end())
		{
			OGRE_EXCEPT(
				Exception::ERR_DUPLICATE_ITEM,
				"A ZoneData associated with zone " + zone->getName() + " already exists",
				"PCZSceneNode::setZoneData" );
		}
		mZoneData[zone->getName()] = zoneData;
	}

	// get zone data for this node for given zone
	// NOTE: This routine assumes that the zone data is present!
	ZoneData* PCZSceneNode::getZoneData(PCZone * zone)
	{
		return mZoneData[zone->getName()];
	}

	// update zone-specific data for any zone that the node is touching
	void PCZSceneNode::updateZoneData(void)
	{
		ZoneData* zoneData;
		PCZone * zone;
		// make sure home zone data is updated
		zone = mHomeZone;
		if (zone->requiresZoneSpecificNodeData())
		{
			zoneData = getZoneData(zone);
			zoneData->update();
		}
		// update zone data for any zones visited
		ZoneMap::iterator it = mVisitingZones.begin();
		while ( it != mVisitingZones.end() )
		{
			zone = it->second;
			if (zone->requiresZoneSpecificNodeData())
			{
				zoneData = getZoneData(zone);
				zoneData->update();
			}
			++it;
		}
	}
}