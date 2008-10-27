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
OgrePCZoneFactory.cpp  -  PCZone Factory & Factory Manager

-----------------------------------------------------------------------------
begin                : Mon Apr 16 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#include "OgrePCZoneFactory.h"
#include "OgrePCZSceneManager.h"
#include "OgreDefaultZone.h"
#include "OgreLogManager.h"

namespace Ogre 
{
	//-------------------------------------------------------------------------
	// PCZoneFactory functions
	PCZoneFactory::PCZoneFactory(const String & typeName) : mFactoryTypeName(typeName)
	{
	}
	PCZoneFactory::~PCZoneFactory()
	{
	}
	//-------------------------------------------------------------------------
	// DefaultZoneFactory functions
	//String defaultString = String("ZoneType_Default"); 
	DefaultZoneFactory::DefaultZoneFactory() : PCZoneFactory("ZoneType_Default")
	{
	}
	DefaultZoneFactory::~DefaultZoneFactory()
	{
	}
	bool DefaultZoneFactory::supportsPCZoneType(const String& zoneType)
	{
		if (mFactoryTypeName == zoneType)
		{
			return true;
		}
		return false;
	}
	PCZone* DefaultZoneFactory::createPCZone(PCZSceneManager * pczsm, const String& zoneName)
	{
		return new DefaultZone(pczsm, zoneName);
	}
	//-------------------------------------------------------------------------
	// PCZoneFactoryManager functions
    template<> PCZoneFactoryManager* Singleton<PCZoneFactoryManager>::ms_Singleton = 0;
    PCZoneFactoryManager* PCZoneFactoryManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    PCZoneFactoryManager& PCZoneFactoryManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	PCZoneFactoryManager::PCZoneFactoryManager()
	{
        registerPCZoneFactory(&mDefaultFactory);
	}
	PCZoneFactoryManager::~PCZoneFactoryManager()
	{
	}

	void PCZoneFactoryManager::registerPCZoneFactory(PCZoneFactory* factory)
	{
		String name = factory->getFactoryTypeName();
        mPCZoneFactories[name] = factory;
        LogManager::getSingleton().logMessage("PCZone Factory Type '" + name + "' registered");
	}
	void PCZoneFactoryManager::unregisterPCZoneFactory(PCZoneFactory* factory)
	{
		if (factory)
		{
			//find and remove factory from mPCZoneFactories
			// Note that this does not free the factory from memory, just removes from the factory manager
			String name = factory->getFactoryTypeName();
			PCZoneFactoryMap::iterator zi = mPCZoneFactories.find(name);
			if (zi != mPCZoneFactories.end())
			{
				mPCZoneFactories.erase( mPCZoneFactories.find( name ) );
				LogManager::getSingleton().logMessage("PCZone Factory Type '" + name + "' unregistered");
			}
		}
	}
	PCZone* PCZoneFactoryManager::createPCZone(PCZSceneManager * pczsm,
											   const String& zoneType, 
											   const String& zoneName)
	{
		//find a factory that supports this zone type and then call createPCZone() on it
		PCZone * inst = 0;
		for(PCZoneFactoryMap::iterator i = mPCZoneFactories.begin(); i != mPCZoneFactories.end(); ++i)
		{
			if (i->second->supportsPCZoneType(zoneType))
			{
				// use this factory
				inst = i->second->createPCZone(pczsm, zoneName);
			}
		}
		if (!inst)
		{
			// Error!
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"No factory found for zone of type '" + zoneType + "'",
				"PCZoneFactoryManager::createPCZone");
		}
		return inst;
	}
	//-----------------------------------------------------------------------
	PCZoneFactoryManager::PCZoneFactoryIterator 
	PCZoneFactoryManager::getPCZoneFactoryIterator(void)
	{
		return PCZoneFactoryIterator(mPCZoneFactories.begin(), mPCZoneFactories.end());
	}
}

