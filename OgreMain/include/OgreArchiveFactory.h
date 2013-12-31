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
#ifndef _ArchiveFactory_H__
#define _ArchiveFactory_H__

#include "OgrePrerequisites.h"
#include "OgreFactoryObj.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
    /** Abstract factory class, archive codec plugins can register concrete
        subclasses of this.
        @remarks
            All access to 'archives' (collections of files, compressed or
            just folders, maybe even remote) is managed via the abstract
            Archive class. Plugins are expected to provide the
            implementation for the actual codec itself, but because a
            subclass of Archive has to be created for every archive, a
            factory class is required to create the appropriate subclass.
        @par
            So archive plugins create a subclass of Archive AND a subclass
            of ArchiveFactory which creates instances of the Archive
            subclass. See the 'Zip' and 'FileSystem' plugins for examples.
            Each Archive and ArchiveFactory subclass pair deal with a
            single archive type (identified by a string).
    */
    class _OgreExport ArchiveFactory : public FactoryObj< Archive >, public ArchiveAlloc
    {
    public:
        virtual ~ArchiveFactory() {}
        /** Creates a new object.
        @param name Name of the object to create
        @return
            An object created by the factory. The type of the object depends on
            the factory.
        */
        virtual Archive* createInstance(const String& name, bool readOnly) = 0;

        virtual Archive* createInstance(const String& name) { return createInstance(name, true); }
    };
	/** @} */
	/** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif
