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
OgrePCZoneFactory.h  -  PCZone Factory & Factory Manager

-----------------------------------------------------------------------------
begin                : Mon Apr 16 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PCZONE_FACTORY_H
#define PCZONE_FACTORY_H

#include <OgreSingleton.h>
#include "OgrePCZone.h"

namespace Ogre
{
	class PCZSceneManager;

	/// Factory for PCZones
	class _OgrePCZPluginExport PCZoneFactory : public SceneCtlAllocatedObject
    {
	public:
		PCZoneFactory(const String & typeName);
		virtual ~PCZoneFactory();
		virtual bool supportsPCZoneType(const String& zoneType) = 0;
		virtual PCZone* createPCZone(PCZSceneManager * pczsm, const String& zoneName) = 0;
		const String& getFactoryTypeName() const { return mFactoryTypeName; }
		/// Factory type name
		String mFactoryTypeName;
    };

	// Factory for default zone
	class _OgrePCZPluginExport DefaultZoneFactory : public PCZoneFactory
	{
	public:
		DefaultZoneFactory();
		virtual ~DefaultZoneFactory();
		bool supportsPCZoneType(const String& zoneType);
		PCZone* createPCZone(PCZSceneManager * pczsm, const String& zoneName);
	};

	// PCZoneFactory manager class
	class _OgrePCZPluginExport PCZoneFactoryManager : public Singleton<PCZoneFactoryManager>, public SceneCtlAllocatedObject
	{
	public:
		PCZoneFactoryManager();	
		~PCZoneFactoryManager();
		void registerPCZoneFactory(PCZoneFactory* factory);
		void unregisterPCZoneFactory(PCZoneFactory* factory);
		PCZone* createPCZone(PCZSceneManager * pczsm,
							 const String& zoneType, 
							 const String& zoneName);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static PCZoneFactoryManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static PCZoneFactoryManager* getSingletonPtr(void);
		/* PCZoneFactory Iterator - for querying what types of PCZone
		factories are available */
		typedef map<String, PCZoneFactory*>::type PCZoneFactoryMap;
		typedef MapIterator<PCZoneFactoryMap> PCZoneFactoryIterator;
		/** Return an iterator over the PCZone factories currently registered */
		PCZoneFactoryIterator getPCZoneFactoryIterator(void);

	protected:
		PCZoneFactoryMap mPCZoneFactories;
		DefaultZoneFactory mDefaultFactory;
	};
}

#endif
