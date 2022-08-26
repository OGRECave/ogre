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
PCZLight.cpp  -  description
-----------------------------------------------------------------------------
begin                : Wed May 23 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    :
-----------------------------------------------------------------------------
*/

#include "OgrePCZLight.h"
#include "OgrePCZone.h" // need for testing affected zone 
#include "OgrePCZSceneNode.h"
#include "OgrePCZFrustum.h"

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
        for (auto & iter : affectedZonesList)
        { 
            if(iter->getPortalsUpdated()) return true;   // return immediately to prevent further iterating
        }

        return false;   // light hasn't moved, and no zones have updated portals. no light update.
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

} // Namespace
