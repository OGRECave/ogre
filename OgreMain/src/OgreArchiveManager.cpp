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
*/
#include "OgreStableHeaders.h"

#include "OgreArchiveManager.h"

#include "OgreArchiveFactory.h"
#include "OgreArchive.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {
    typedef void (*createFunc)( Archive**, const String& );

    //-----------------------------------------------------------------------
    template<> ArchiveManager* Singleton<ArchiveManager>::ms_Singleton = 0;
    ArchiveManager* ArchiveManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ArchiveManager& ArchiveManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    Archive* ArchiveManager::load( const String& filename, const String& archiveType)
    {
        ArchiveMap::iterator i = mArchives.find(filename);
        Archive* pArch = 0;

        if (i == mArchives.end())
        {
            // Search factories
            ArchiveFactoryMap::iterator it = mArchFactories.find(archiveType);
            if (it == mArchFactories.end())
                // Factory not found
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot find an archive factory "
                    "to deal with archive of type " + archiveType, "ArchiveManager::load");

            pArch = it->second->createInstance(filename);
            pArch->load();
            mArchives[filename] = pArch;

        }
        else
        {
            pArch = i->second;
        }
        return pArch;
    }
	//-----------------------------------------------------------------------
	void ArchiveManager::unload(Archive* arch)
	{
		unload(arch->getName());
	}
	//-----------------------------------------------------------------------
	void ArchiveManager::unload(const String& filename)
	{
		ArchiveMap::iterator i = mArchives.find(filename);

		if (i != mArchives.end())
		{
			i->second->unload();
			// Find factory to destroy
			ArchiveFactoryMap::iterator fit = mArchFactories.find(i->second->getType());
			if (fit == mArchFactories.end())
			{
				// Factory not found
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot find an archive factory "
					"to deal with archive of type " + i->second->getType(), "ArchiveManager::~ArchiveManager");
			}
			fit->second->destroyInstance(i->second);
			mArchives.erase(i);
		}
	}
	//-----------------------------------------------------------------------
	ArchiveManager::ArchiveMapIterator ArchiveManager::getArchiveIterator(void)
	{
		return ArchiveMapIterator(mArchives.begin(), mArchives.end());
	}
    //-----------------------------------------------------------------------
    ArchiveManager::~ArchiveManager()
    {
        // Unload & delete resources in turn
        for( ArchiveMap::iterator it = mArchives.begin(); it != mArchives.end(); ++it )
        {
            Archive* arch = it->second;
            // Unload
            arch->unload();
            // Find factory to destroy
            ArchiveFactoryMap::iterator fit = mArchFactories.find(arch->getType());
            if (fit == mArchFactories.end())
            {
                // Factory not found
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot find an archive factory "
                "to deal with archive of type " + arch->getType(), "ArchiveManager::~ArchiveManager");
            }
            fit->second->destroyInstance(arch);
            
        }
        // Empty the list
        mArchives.clear();
    }
    //-----------------------------------------------------------------------
    void ArchiveManager::addArchiveFactory(ArchiveFactory* factory)
    {        
        mArchFactories.insert( ArchiveFactoryMap::value_type( factory->getType(), factory ) );
        LogManager::getSingleton().logMessage("ArchiveFactory for archive type " +     factory->getType() + " registered.");
    }

}

