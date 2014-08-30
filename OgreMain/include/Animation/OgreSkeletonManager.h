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
#ifndef _OgreSkeletonManager_H_
#define _OgreSkeletonManager_H_

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */
    /** Handles the management of skeleton resources.
        @remarks
            This class deals with the runtime management of
            skeleton data; like other resource managers it handles
            the creation of resources (in this case skeleton data).
    */
    class _OgreExport SkeletonManager : public Singleton<SkeletonManager>, public ResourceAlloc
    {
        typedef map<IdString, SkeletonDefPtr>::type SkeletonDefMap;
        SkeletonDefMap mSkeletonDefs;

    public:
        /// Constructor
        SkeletonManager();
        ~SkeletonManager();

        /** Creates a skeletondef based on an existing one from the legacy skeleton system.
            If a skeleton def with the same name already exists, returns that one instead.
        */
        SkeletonDefPtr getSkeletonDef( v1::Skeleton *oldSkeletonBase );

        /// Create a new skeleton or retrieves an existing one. Will throw if can't find the skeleton.
        SkeletonDefPtr getSkeletonDef( const String &name,
                        const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /** Adds an external pointer for us to track. Throws
            if a skeleton with the same name already exists
        */
        void add( SkeletonDefPtr skeletonDef );

        /** Will remove the SkeletonDef from our lists, but the memory pointer may not actually be
            deleted, which will happen when all references to the shared object are destroyed.
        */
        void remove( const IdString &name );

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
        static SkeletonManager& getSingleton(void);
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
        static SkeletonManager* getSingletonPtr(void);
    };

    /** @} */
    /** @} */

}


#endif
