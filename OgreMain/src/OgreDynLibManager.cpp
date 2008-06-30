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

#include "OgreDynLibManager.h"

#include "OgreDynLib.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    template<> DynLibManager* Singleton<DynLibManager>::ms_Singleton = 0;
    DynLibManager* DynLibManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
	//-----------------------------------------------------------------------
    DynLibManager& DynLibManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
	DynLibManager::DynLibManager()
	{
	}
	//-----------------------------------------------------------------------
    DynLib* DynLibManager::load( const String& filename)
    {
		DynLibList::iterator i = mLibList.find(filename);
		if (i != mLibList.end())
		{
			return i->second;
		}
		else
		{
	        DynLib* pLib = OGRE_NEW DynLib(filename);
			pLib->load();        
        	mLibList[filename] = pLib;
	        return pLib;
		}
    }
	//-----------------------------------------------------------------------
	void DynLibManager::unload(DynLib* lib)
	{
		DynLibList::iterator i = mLibList.find(lib->getName());
		if (i != mLibList.end())
		{
			mLibList.erase(i);
		}
		lib->unload();
		OGRE_DELETE lib;
	}
	//-----------------------------------------------------------------------
    DynLibManager::~DynLibManager()
    {
        // Unload & delete resources in turn
        for( DynLibList::iterator it = mLibList.begin(); it != mLibList.end(); ++it )
        {
            it->second->unload();
            OGRE_DELETE it->second;
        }

        // Empty the list
        mLibList.clear();
    }
}
