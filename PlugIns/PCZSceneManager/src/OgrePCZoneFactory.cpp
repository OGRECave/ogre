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
		return OGRE_NEW DefaultZone(pczsm, zoneName);
	}
	//-------------------------------------------------------------------------
	// PCZoneFactoryManager functions
    template<> PCZoneFactoryManager* Singleton<PCZoneFactoryManager>::msSingleton = 0;
    PCZoneFactoryManager* PCZoneFactoryManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    PCZoneFactoryManager& PCZoneFactoryManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
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

