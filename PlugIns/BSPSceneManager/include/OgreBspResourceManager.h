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
#ifndef _BspLevelManager_H__
#define _BspLevelManager_H__

#include "OgreBspPrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** Manages the locating and loading of BSP-based indoor levels.
    Like other ResourceManager specialisations it manages the location and loading
    of a specific type of resource, in this case files containing Binary
    Space Partition (BSP) based level files e.g. Quake3 levels.</p>
    However, note that unlike other ResourceManager implementations,
    only 1 BspLevel resource is allowed to be loaded at one time. Loading
    another automatically unloads the currently loaded level if any.
    */
    class BspResourceManager : public ResourceManager, public Singleton<BspResourceManager>
    {
    public:
        BspResourceManager();
        ~BspResourceManager();

        /** Loads a BSP-based level from the named file.
            Currently only supports loading of Quake3 .bsp files.
        */
        ResourcePtr load(const String& name, 
            const String& group, bool isManual = false, 
            ManualResourceLoader* loader = 0, const NameValuePairList* loadParams = 0);

        /** Loads a BSP-based level from a stream.
            Currently only supports loading of Quake3 .bsp files.
        */
        ResourcePtr load(DataStreamPtr& stream, const String& group);

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
        static BspResourceManager& getSingleton(void);
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
        static BspResourceManager* getSingletonPtr(void);


    protected:
        /** @copydoc ResourceManager::createImpl. */
        Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader, 
            const NameValuePairList* createParams);

        // Singleton managed by this class
        Quake3ShaderManager *mShaderMgr;

    };

}

#endif
