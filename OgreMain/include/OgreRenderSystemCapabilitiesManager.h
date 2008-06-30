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
#ifndef __RENDERSYSTEMCAPABILITIESMANAGER_H__
#define __RENDERSYSTEMCAPABILITIESMANAGER_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreStringVector.h"

#include "OgreRenderSystemCapabilities.h"



namespace Ogre {


    /** Class for managing RenderSystemCapabilities database for Ogre.
        @remarks This class behaves similarly to other ResourceManager, although .rendercaps are not resources.
						It contains and abstract a .rendercaps Serializer
    */
    class _OgreExport RenderSystemCapabilitiesManager :  public Singleton<RenderSystemCapabilitiesManager>, public RenderSysAlloc
    {

    public:

        /** Default constructor.
        */
        RenderSystemCapabilitiesManager();

        /** Default destructor.
        */
        virtual ~RenderSystemCapabilitiesManager();


        /** @see ScriptLoader::parseScript
        */
        void parseCapabilitiesFromArchive(const String& filename, const String& archiveType, bool recursive = true);
		
		/** Returns a capability loaded with RenderSystemCapabilitiesManager::parseCapabilitiesFromArchive method
		* @return NULL if the name is invalid, a parsed RenderSystemCapabilities otherwise.
		*/
        RenderSystemCapabilities* loadParsedCapabilities(const String& name);

        /** Method used by RenderSystemCapabilitiesSerializer::parseScript */
        void _addRenderSystemCapabilities(const String& name, RenderSystemCapabilities* caps);

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
        static RenderSystemCapabilitiesManager& getSingleton(void);
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
        static RenderSystemCapabilitiesManager* getSingletonPtr(void);

    protected:

        RenderSystemCapabilitiesSerializer* mSerializer;

        typedef std::map<String, RenderSystemCapabilities*> CapabilitiesMap;
        CapabilitiesMap mCapabilitiesMap;

        const String mScriptPattern;

    };

}

#endif
