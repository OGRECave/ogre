/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
