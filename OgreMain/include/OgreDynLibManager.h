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
#ifndef __DynLibManager_H__
#define __DynLibManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** Manager for Dynamic-loading Libraries.

        This manager keeps a track of all the open dynamic-loading
        libraries, opens them and returns references to already-open
        libraries.
    */
    class _OgreExport DynLibManager: public Singleton<DynLibManager>, public DynLibAlloc
    {
    private:
        typedef std::map<String, DynLib*> DynLibList;
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
        ~DynLibManager();

        /** Loads the passed library.
        @param filename
            The name of the library. The extension can be omitted.
        */
        DynLib* load(const String& filename);

        /** Unloads the passed library.
        @param lib
            The library.
        */
        void unload(DynLib* lib);

        /// @copydoc Singleton::getSingleton()
        static DynLibManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static DynLibManager* getSingletonPtr(void);
    };
    /** @} */
    /** @} */
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __DynLibManager_H__
