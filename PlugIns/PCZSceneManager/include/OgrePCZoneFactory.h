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
