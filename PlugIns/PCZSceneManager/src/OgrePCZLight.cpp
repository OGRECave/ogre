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
PCZLight.cpp  -  description
-----------------------------------------------------------------------------
begin                : Wed May 23 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"
#include "OgrePCZLight.h"
#include "OgrePCZone.h" // need for testing affected zone 
#include "OgreException.h"
#include "OgrePCZSceneNode.h"
#include "OgrePCZCamera.h"
#include "OgrePCZFrustum.h"
#include "OgrePCZSceneManager.h"

namespace Ogre 
{
    //-----------------------------------------------------------------------
    PCZLight::PCZLight() : Light()
    {
		mNeedsUpdate = true;   // need to update the first time, regardless of attachment or movement 
    }
    //-----------------------------------------------------------------------
	PCZLight::PCZLight(const String& name) : Light(name)
    {
		mNeedsUpdate = true;   // need to update the first time, regardless of attachment or movement 
    }
    //-----------------------------------------------------------------------
    PCZLight::~PCZLight()
    {
        affectedZonesList.clear();
    }
    //-----------------------------------------------------------------------
    const String& PCZLight::getMovableType(void) const
    {
		return PCZLightFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    /** Clear the affectedZonesList 
    */
    void PCZLight::clearAffectedZones(void)
    {
        affectedZonesList.clear();
    }
    //-----------------------------------------------------------------------
    /** Add a zone to the zones affected list
    */
    void PCZLight::addZoneToAffectedZonesList(PCZone * zone)
    {
        affectedZonesList.push_back(zone);
    }
    //-----------------------------------------------------------------------
    /** check if a zone is in the list of zones affected by the light 
    */
    bool PCZLight::affectsZone(PCZone * zone)
    {
        ZoneList::iterator it = std::find(affectedZonesList.begin(), affectedZonesList.end(), zone);
        if (it == affectedZonesList.end())
        {
            // not in the affectedZonesList
            return false;
        }
        return true;
    }
    //-----------------------------------------------------------------------
    void PCZLight::updateZones(PCZone * defaultZone, unsigned long frameCount)
    {
        //update the zones this light affects
        PCZone * homeZone;
        affectedZonesList.clear();
        mAffectsVisibleZone = false;
        PCZSceneNode * sn = (PCZSceneNode*)(this->getParentSceneNode());
        if (sn)
        {
            // start with the zone the light is in
            homeZone = sn->getHomeZone();
            if (homeZone)
            {
                affectedZonesList.push_back(homeZone);
                if (homeZone->getLastVisibleFrame() == frameCount)
                {
                    mAffectsVisibleZone = true;
                }
            }
            else
            {
                // error - scene node has no homezone!
                // just say it affects the default zone and leave it at that.
                affectedZonesList.push_back(defaultZone);
                if (defaultZone->getLastVisibleFrame() == frameCount)
                {
                    mAffectsVisibleZone = true;
                }
                return;
            }
        }
        else
        {
            // ERROR! not connected to a scene node,                 
            // just say it affects the default zone and leave it at that.
            affectedZonesList.push_back(defaultZone);
            if (defaultZone->getLastVisibleFrame() == frameCount)
            {
                mAffectsVisibleZone = true;
            }
            return;
        }

        // now check visibility of each portal in the home zone.  If visible to
        // the light then add the target zone of the portal to the list of
        // affected zones and recurse into the target zone
        static PCZFrustum portalFrustum;
        Vector3 v = getDerivedPosition();
        portalFrustum.setOrigin(v);
        homeZone->_checkLightAgainstPortals(this, frameCount, &portalFrustum, 0);
    }
	//-----------------------------------------------------------------------
	void PCZLight::removeZoneFromAffectedZonesList(PCZone * zone)
	{
		ZoneList::iterator it = std::find(affectedZonesList.begin(), affectedZonesList.end(), zone);

		if (it != affectedZonesList.end())
		{
			affectedZonesList.erase( it );   // zone is in list, erase it.
		}
	}
	//-----------------------------------------------------------------------
	void PCZLight::_notifyMoved(void)
	{
		Light::_notifyMoved();   // inform ogre Light of movement

		mNeedsUpdate = true;   // set need update flag
	}
	//-----------------------------------------------------------------------
	bool PCZLight::getNeedsUpdate(void)
	{
		if(mNeedsUpdate)   // if this light has moved, return true immediately
			return true;

		// if any zones affected by this light have updated portals, then this light needs updating too
		for (ZoneList::iterator iter = affectedZonesList.begin() ; iter != affectedZonesList.end(); iter++)
		{ 
			if((*iter)->getPortalsUpdated()) return true;   // return immediately to prevent further iterating
		}

		return false;   // light hasnt moved, and no zones have updated portals. no light update.
	}


	//-----------------------------------------------------------------------
	String PCZLightFactory::FACTORY_TYPE_NAME = "PCZLight";
	//-----------------------------------------------------------------------
	const String& PCZLightFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	MovableObject* PCZLightFactory::createInstanceImpl( const String& name, 
		const NameValuePairList* params)
	{

		return OGRE_NEW PCZLight(name);

	}
	//-----------------------------------------------------------------------
	void PCZLightFactory::destroyInstance( MovableObject* obj)
	{
		OGRE_DELETE obj;
	}

} // Namespace
