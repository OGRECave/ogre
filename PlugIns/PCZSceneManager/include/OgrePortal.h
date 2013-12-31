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
Portal.h  -  Portals are special constructs which which are used to connect 
			 two Zones in a PCZScene.  Portals are defined by 4 coplanr 
             corners and a direction.  Portals are contained within Zones and 
             are essentially "one way" connectors.  Objects and entities can
			 use them to travel to other Zones, but to return, there must be
			 a corresponding Portal which connects back to the original zone
			 from the new zone.

-----------------------------------------------------------------------------
begin                : Thu Feb 22 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 : Apr 5, 2007
-----------------------------------------------------------------------------
*/

#ifndef PORTAL_H
#define PORTAL_H

#include "OgrePortalBase.h"

namespace Ogre
{
	/** Portal datastructure for connecting zones. */
	class _OgrePCZPluginExport Portal : public PortalBase
	{
	public:
		Portal(const String &name, const PORTAL_TYPE type = PORTAL_TYPE_QUAD);
		virtual ~Portal();

		void setTargetZone(PCZone* zone);
		/** Set the target portal pointer */
		void setTargetPortal(Portal* portal);

		/** Get the Zone the Portal connects to */
		PCZone* getTargetZone() {return mTargetZone;}
		/** Get the connected portal (if any) */
		Portal* getTargetPortal() {return mTargetPortal;}

		/** @copydoc MovableObject::getMovableType. */
		const String& getMovableType() const;

	protected:
		///connected Zone
		PCZone* mTargetZone;
		/** Matching Portal in the target zone (usually in same world space 
			as this portal, but pointing the opposite direction)
		*/
		Portal* mTargetPortal;
	};

	/** Factory object for creating Portal instances */
	class _OgrePCZPluginExport PortalFactory : public PortalBaseFactory
	{
	protected:
		MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);
	public:
		PortalFactory() {}
		~PortalFactory() {}

		static String FACTORY_TYPE_NAME;
		static unsigned long FACTORY_TYPE_FLAG;

		const String& getType() const
		{ return FACTORY_TYPE_NAME; }

		void destroyInstance(MovableObject* obj);

		/** Return true here as we want to get a unique type flag. */
		bool requestTypeFlags() const
		{ return true; }

	};

}

#endif



