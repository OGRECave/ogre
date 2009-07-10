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
PCZLight.h  -  description
-----------------------------------------------------------------------------
begin                : Wed May 23 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PCZLIGHT_H
#define PCZLIGHT_H

#include <OgreLight.h>
#include "OgrePCZPrerequisites.h"
#include "OgrePCZone.h"

namespace Ogre
{

    typedef list<PCZone*>::type ZoneList;

    /** Specialized version of Ogre::Light which caches which zones the light affects
    @remarks
    */

    class _OgrePCZPluginExport PCZLight : public Light
    {
    public:
        /** Default constructor (for Python mainly).
        */
        PCZLight();

        /** Normal constructor. Should not be called directly, but rather the SceneManager::createLight method should be used.
        */
        PCZLight(const String& name);

        /** Standard destructor.
        */
        ~PCZLight();

        /** Overridden from MovableObject */
        const String& getMovableType(void) const;

        /** Clear the affectedZonesList 
        */
        void clearAffectedZones(void);

        /** Manually add a zone to the zones affected list
        */
        void addZoneToAffectedZonesList(PCZone * zone);

        /** check if a zone is in the list of zones affected by the light 
        */
        bool affectsZone(PCZone * zone);

        /** returns flag indicating if the light affects a zone which is visible
        *   in the current frame
        */
        bool affectsVisibleZone(void) {return mAffectsVisibleZone;}

		/** Marks a light as affecting a visible zone */
		void setAffectsVisibleZone(bool affects) { mAffectsVisibleZone = affects; }

        /** Update the list of zones the light affects 
        */
        void updateZones(PCZone * defaultZone, unsigned long frameCount);

		void removeZoneFromAffectedZonesList(PCZone * zone); // manually remove a zone from the affected list

		// MovableObject notified when SceneNode changes
		virtual void _notifyMoved(void);   

		// clear update flag
		void clearNeedsUpdate(void)   { mNeedsUpdate = false; } 

		// get status of need for update. this checks all affected zones
		bool getNeedsUpdate(void);   

    protected:
        /** flag indicating if any of the zones in the affectedZonesList is 
        *   visible in the current frame
        */
        bool mAffectsVisibleZone;

        /** List of PCZones which are affected by the light
        */
        ZoneList affectedZonesList;

		// flag recording if light has moved, therefore affected list needs updating 
		bool mNeedsUpdate;   
	};

	/** Factory object for creating PCZLight instances */
	class _OgrePCZPluginExport PCZLightFactory : public MovableObjectFactory
	{
	protected:
		MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
	public:
		PCZLightFactory() {}
		~PCZLightFactory() {}

		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
		void destroyInstance( MovableObject* obj);  

	};


} // Namespace
#endif