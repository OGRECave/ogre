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
#ifndef _OgreExternalTextureSourceManager_H
#define _OgreExternalTextureSourceManager_H

/***************************************************************************
OgreExternalTextureSourceManager.h  -  
	Handles the registering / unregistering of texture modifier plugins

-------------------
date                 : Jan 1 2004
email                : pjcast@yahoo.com
***************************************************************************/
#include "OgreSingleton.h"
#include "OgreString.h"
#include "OgreResourceGroupManager.h"
#include "OgreExternalTextureSource.h"

namespace Ogre
{
	/** 
	Singleton Class which handles the registering and control of texture plugins. The plugins
	will be mostly controlled via a string interface. */
	class _OgreExport ExternalTextureSourceManager : public Singleton<ExternalTextureSourceManager>, public ResourceAlloc
	{
	public:
		/** Constructor */
		ExternalTextureSourceManager();
		/** Destructor */
		~ExternalTextureSourceManager();

		/** Sets active plugin (ie. "video", "effect", "generic", etc..) */
		void setCurrentPlugIn( const String& sTexturePlugInType );

		/** Returns currently selected plugin, may be null if none selected */
		ExternalTextureSource* getCurrentPlugIn( void ) const { return mpCurrExternalTextureSource; }
	
		/** Calls the destroy method of all registered plugins... 
		Only the owner plugin should perform the destroy action. */
		void destroyAdvancedTexture( const String& sTextureName,
			const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		/** Returns the plugin which reistered itself with a specific name 
		(eg. "video"), or null if specified plugin not found */
		ExternalTextureSource* getExternalTextureSource( const String& sTexturePlugInType );

		/** Called from plugin to register itself */
		void setExternalTextureSource( const String& sTexturePlugInType, ExternalTextureSource* pTextureSystem );

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
        static ExternalTextureSourceManager& getSingleton(void);
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
        static ExternalTextureSourceManager* getSingletonPtr(void);
	protected:
		//The current texture controller selected
		ExternalTextureSource* mpCurrExternalTextureSource;
		
        // Collection of loaded texture System PlugIns, keyed by registered type
        typedef map< String, ExternalTextureSource*>::type TextureSystemList;
        TextureSystemList mTextureSystems;
    };
} 
#endif
