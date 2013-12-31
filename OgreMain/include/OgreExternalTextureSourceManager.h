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
#include "OgreHeaderPrefix.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Materials
	*  @{
	*/
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
		ExternalTextureSource* getCurrentPlugIn( void ) const { return mCurrExternalTextureSource; }
	
		/** Calls the destroy method of all registered plugins... 
		Only the owner plugin should perform the destroy action. */
		void destroyAdvancedTexture( const String& sTextureName,
			const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		/** Returns the plugin which registered itself with a specific name 
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
		/// The current texture controller selected
		ExternalTextureSource* mCurrExternalTextureSource;
		
        // Collection of loaded texture System PlugIns, keyed by registered type
        typedef map< String, ExternalTextureSource*>::type TextureSystemList;
        TextureSystemList mTextureSystems;
    };
	/** @} */
	/** @} */
} 

#include "OgreHeaderSuffix.h"

#endif
