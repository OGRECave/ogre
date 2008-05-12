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

/***************************************************************************
OgreExternalTextureSourceManager.cpp  -  
	Implementation of the manager class

-------------------
date                 : Jan 1 2004
email                : pjcast@yahoo.com
***************************************************************************/

#include "OgreStableHeaders.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLogManager.h"


namespace Ogre 
{
	//****************************************************************************************
    template<> ExternalTextureSourceManager* Singleton<ExternalTextureSourceManager>::ms_Singleton = 0;
    ExternalTextureSourceManager* ExternalTextureSourceManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ExternalTextureSourceManager& ExternalTextureSourceManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	//****************************************************************************************

	//****************************************************************************************
	ExternalTextureSourceManager::ExternalTextureSourceManager()
	{
		mpCurrExternalTextureSource = 0;
	}

	//****************************************************************************************
	ExternalTextureSourceManager::~ExternalTextureSourceManager()
	{
		mTextureSystems.clear();
	}

	//****************************************************************************************
	
	void ExternalTextureSourceManager::setCurrentPlugIn( const String& sTexturePlugInType )
	{
		TextureSystemList::iterator i;
			
		for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
		{
			if( i->first == sTexturePlugInType )
			{
				mpCurrExternalTextureSource = i->second;
				mpCurrExternalTextureSource->initialise();	//Now call overridden Init function
				return;
			}
		}
		mpCurrExternalTextureSource = 0;
		LogManager::getSingleton().logMessage( "ExternalTextureSourceManager::SetCurrentPlugIn(ENUM) failed setting texture plugin ");
	}

	//****************************************************************************************
	void ExternalTextureSourceManager::destroyAdvancedTexture( const String& sTextureName,
		const String& groupName )
	{
		TextureSystemList::iterator i;
		for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
		{
			//Broadcast to every registered System... Only the true one will destroy texture
			i->second->destroyAdvancedTexture( sTextureName, groupName );
		}
	}

	//****************************************************************************************
	void ExternalTextureSourceManager::setExternalTextureSource( const String& sTexturePlugInType, ExternalTextureSource* pTextureSystem )
	{
		LogManager::getSingleton().logMessage( "Registering Texture Controller: Type = "
						+ sTexturePlugInType + " Name = " + pTextureSystem->getPlugInStringName());

		TextureSystemList::iterator i;
			
		for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
		{
			if( i->first == sTexturePlugInType )
			{
				LogManager::getSingleton().logMessage( "Shutting Down Texture Controller: " 
						+ i->second->getPlugInStringName() 
						+ " To be replaced by: "
						+ pTextureSystem->getPlugInStringName());

				i->second->shutDown();				//Only one plugIn of Sent Type can be registered at a time
													//so shut down old plugin before starting new plugin
				i->second = pTextureSystem;
				// **Moved this line b/c Rendersystem needs to be selected before things
				// such as framelistners can be added
				// pTextureSystem->Initialise();
				return;
			}
		}
		mTextureSystems[sTexturePlugInType] = pTextureSystem;	//If we got here then add it to map
	}

	//****************************************************************************************
	ExternalTextureSource* ExternalTextureSourceManager::getExternalTextureSource( const String& sTexturePlugInType )
	{
		TextureSystemList::iterator i;
		for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
		{
			if( i->first == sTexturePlugInType )
				return i->second;
		}
		return 0;
	}

	//****************************************************************************************

}  //End Ogre Namespace

