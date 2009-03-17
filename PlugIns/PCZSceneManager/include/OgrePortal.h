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



