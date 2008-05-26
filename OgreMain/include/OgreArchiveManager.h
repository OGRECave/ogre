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
#ifndef _ArchiveManager_H__
#define _ArchiveManager_H__

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** This class manages the available ArchiveFactory plugins. 
    */
    class _OgreExport ArchiveManager : public Singleton<ArchiveManager>, public ArchiveAlloc
    {
    protected:
        typedef std::map<String, ArchiveFactory*> ArchiveFactoryMap;
        /// Factories available to create archives, indexed by archive type (String identifier e.g. 'Zip')
        ArchiveFactoryMap mArchFactories;
        /// Currently loaded archives
        typedef std::map<String, Archive*> ArchiveMap;
        ArchiveMap mArchives;

    public:
        /** Default constructor - should never get called by a client app.
        */
        ArchiveManager() {}
        /** Default destructor.
        */
        virtual ~ArchiveManager();

        /** Opens an archive for file reading.
            @remarks
                The archives are created using class factories within
                extension libraries.
            @param filename
                The filename that will be opened
            @param refLibrary
                The library that contains the data-handling code
            @returns
                If the function succeeds, a valid pointer to an Archive
                object is returned.
            @par
                If the function fails, an exception is thrown.
        */
        Archive* load( const String& filename, const String& archiveType);

		/** Unloads an archive.
		@remarks
			You must ensure that this archive is not being used before removing it.
		*/
		void unload(Archive* arch);
		/** Unloads an archive by name.
		@remarks
			You must ensure that this archive is not being used before removing it.
		*/
		void unload(const String& filename);
		typedef MapIterator<ArchiveMap> ArchiveMapIterator;
		/** Get an iterator over the Archives in this Manager. */
		ArchiveMapIterator getArchiveIterator(void);

        /** Adds a new ArchiveFactory to the list of available factories.
            @remarks
                Plugin developers who add new archive codecs need to call
                this after defining their ArchiveFactory subclass and
                Archive subclasses for their archive type.
        */
        void addArchiveFactory(ArchiveFactory* factory);
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
        static ArchiveManager& getSingleton(void);
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
        static ArchiveManager* getSingletonPtr(void);
    };

}

#endif
