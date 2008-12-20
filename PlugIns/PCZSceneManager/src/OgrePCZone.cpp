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
PCZone.cpp  -  description
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com
-----------------------------------------------------------------------------
*/

#include "OgrePCZone.h"
#include "OgreSceneNode.h"
#include "OgrePortal.h"
#include "OgrePCZSceneNode.h"
#include "OgrePCZSceneManager.h"

namespace Ogre
{

    PCZone::PCZone( PCZSceneManager * creator, const String& name ) 
    {
        mLastVisibleFrame = 0;
		mLastVisibleFromCamera = 0;
		mName = name;
		mZoneTypeName = "ZoneType_Undefined";
		mEnclosureNode = 0;
		mPCZSM = creator;
		mHasSky = false;
	}

    PCZone::~PCZone()
    {
		// clear list of nodes contained within the zone
		_clearNodeLists(HOME_NODE_LIST|VISITOR_NODE_LIST);
		// clear portal list (actual deletion of portals takes place in the PCZSM)
		mPortals.clear();
    }

	/** Remove all nodes from the node reference list and clear it
	*/
	void PCZone::_clearNodeLists(short type)
	{
		if (type & HOME_NODE_LIST)
		{
			mHomeNodeList.clear();
		}
		if (type & VISITOR_NODE_LIST)
		{
			mVisitorNodeList.clear();
		}
	}

	Portal * PCZone::findMatchingPortal(Portal * portal)
	{
		// look through all the portals in zone2 for a match
		Portal* portal2;
		PortalList::iterator pi2, piend2;
		piend2 = mPortals.end();
		for (pi2 = mPortals.begin(); pi2 != piend2; pi2++)
		{
			portal2 = *pi2;
			//portal2->updateDerivedValues();
			if (portal2->getTargetZone() == 0 &&
				portal2->isAntiPortal() == false &&
				portal2->closeTo(portal) &&
				portal2->getDerivedDirection().dotProduct(portal->getDerivedDirection()) < -0.9)
			{
				// found a match!
				return portal2;
			}
		}
		// no match
		return 0;
	}

	/* create node specific zone data if necessary
	*/
	void PCZone::createNodeZoneData(PCZSceneNode *)
	{
	}

	/* get the aabb of the zone - default implementation
	   uses the enclosure node, but there are other perhaps
	   better ways
	*/
	void PCZone::getAABB(AxisAlignedBox & aabb)
	{
		// if there is no node, just return a null box
		if (mEnclosureNode == 0)
		{
			aabb.setNull();
		}
		else
		{
			aabb = mEnclosureNode->_getWorldAABB();
			// since this is the "local" AABB, subtract out any translations
			aabb.setMinimum(aabb.getMinimum() - mEnclosureNode->_getDerivedPosition());
			aabb.setMaximum(aabb.getMaximum() - mEnclosureNode->_getDerivedPosition());
		}
		return;
	}

	/***********************************************************************\
	ZoneData - Zone-specific Data structure for Scene Nodes
	/***********************************************************************/

	ZoneData::ZoneData(PCZSceneNode * node, PCZone * zone)
	{
		mAssociatedZone = zone;
		mAssociatedNode = node;
	}

	ZoneData::~ZoneData()
	{
	}

	void ZoneData::update(void)
	{
	}
}
