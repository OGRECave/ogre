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
#ifndef _ArchiveManager_H__
#define _ArchiveManager_H__

#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreIteratorWrapper.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** This class manages the available ArchiveFactory plugins. 
    */
    class _OgreExport ArchiveManager : public Singleton<ArchiveManager>, public ArchiveAlloc
    {
    private:
        typedef std::map<String, ArchiveFactory*> ArchiveFactoryMap;
        /// Factories available to create archives, indexed by archive type (String identifier e.g. 'Zip')
        ArchiveFactoryMap mArchFactories;
        /// Currently loaded archives
        typedef std::map<String, Archive*> ArchiveMap;
        ArchiveMap mArchives;

    public:
        /** Default constructor - should never get called by a client app.
        */
        ArchiveManager();
        /** Default destructor.
        */
        virtual ~ArchiveManager();

        /** Opens an archive for file reading.

                The archives are created using class factories within
                extension libraries.
            @param filename
                The filename that will be opened
            @param archiveType
                The type of archive that this is. For example: "Zip".
            @param readOnly
                Whether the Archive is read only
            @return
                If the function succeeds, a valid pointer to an Archive
                object is returned.
            @par
                If the function fails, an exception is thrown.
        */
        Archive* load( const String& filename, const String& archiveType, bool readOnly);

        /** Unloads an archive.

            You must ensure that this archive is not being used before removing it.
        */
        void unload(Archive* arch);
        /** Unloads an archive by name.

            You must ensure that this archive is not being used before removing it.
        */
        void unload(const String& filename);
        typedef MapIterator<ArchiveMap> ArchiveMapIterator;
        /** Get an iterator over the Archives in this Manager. */
        ArchiveMapIterator getArchiveIterator(void);

        /** Adds a new ArchiveFactory to the list of available factories.

                Plugin developers who add new archive codecs need to call
                this after defining their ArchiveFactory subclass and
                Archive subclasses for their archive type.
        */
        void addArchiveFactory(ArchiveFactory* factory);
        /// @copydoc Singleton::getSingleton()
        static ArchiveManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static ArchiveManager* getSingletonPtr(void);
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
