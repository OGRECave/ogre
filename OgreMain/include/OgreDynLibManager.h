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
#ifndef __DynLibManager_H__
#define __DynLibManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** Manager for Dynamic-loading Libraries.
        @remarks
            This manager keeps a track of all the open dynamic-loading
            libraries, opens them and returns references to already-open
            libraries.
    */
    class _OgreExport DynLibManager: public Singleton<DynLibManager>, public DynLibAlloc
    {
	protected:
		typedef map<String, DynLib*>::type DynLibList;
		DynLibList mLibList;
    public:
        /** Default constructor.
            @note
                <br>Should never be called as the singleton is automatically
                created during the creation of the Root object.
            @see
                Root::Root
        */
        DynLibManager();

        /** Default destructor.
            @see
                Root::~Root
        */
        virtual ~DynLibManager();

        /** Loads the passed library.
            @param
                filename The name of the library. The extension can be omitted
        */
        DynLib* load(const String& filename);

		/** Unloads the passed library.
		@param
		filename The name of the library. The extension can be omitted
		*/
		void unload(DynLib* lib);

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
        static DynLibManager& getSingleton(void);
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
        static DynLibManager* getSingletonPtr(void);
    };
	/** @} */
	/** @} */
}

#endif
