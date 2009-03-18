/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
PortalBase.h  -  PortalBase is the base class for Portal and AntiPortal.

*/

#ifndef ANTIPORTAL_H
#define ANTIPORTAL_H

#include "OgrePortalBase.h"

namespace Ogre
{
	/** AntiPortal datastructure for occlusion culling. */
	class _OgrePCZPluginExport AntiPortal : public PortalBase
	{
	public:
		AntiPortal(const String &name, const PORTAL_TYPE type = PORTAL_TYPE_QUAD);
		virtual ~AntiPortal();

		/** @copydoc MovableObject::getMovableType. */
		const String& getMovableType() const;

	};

	/** Factory object for creating AntiPortal instances */
	class _OgrePCZPluginExport AntiPortalFactory : public PortalBaseFactory
	{
	protected:
		MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params);
	public:
		AntiPortalFactory() {}
		~AntiPortalFactory() {}

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
