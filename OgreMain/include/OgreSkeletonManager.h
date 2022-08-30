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
#ifndef __SkeletonManager_H__
#define __SkeletonManager_H__

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

            This class deals with the runtime management of
            skeleton data; like other resource managers it handles
            the creation of resources (in this case skeleton data),
            working within a fixed memory budget.
    */
    class _OgreExport SkeletonManager: public ResourceManager, public Singleton<SkeletonManager>
    {
    public:
        /// Constructor
        SkeletonManager();
        ~SkeletonManager();

        /// Create a new skeleton
        /// @see ResourceManager::createResource
        SkeletonPtr create (const String& name, const String& group,
                            bool isManual = false, ManualResourceLoader* loader = 0,
                            const NameValuePairList* createParams = 0);

        /// Get a resource by name
        /// @see ResourceManager::getResourceByName
        SkeletonPtr getByName(const String& name, const String& groupName OGRE_RESOURCE_GROUP_INIT) const;

        /// @copydoc Singleton::getSingleton()
        static SkeletonManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static SkeletonManager* getSingletonPtr(void);
    private:

        /// @copydoc ResourceManager::createImpl
        Resource* createImpl(const String& name, ResourceHandle handle, 
            const String& group, bool isManual, ManualResourceLoader* loader, 
            const NameValuePairList* createParams) override;

    };

    /** @} */
    /** @} */

}


#endif
