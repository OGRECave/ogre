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
#ifndef __ResourceBackgroundQueue_H__
#define __ResourceBackgroundQueue_H__


#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreSingleton.h"
#include "OgreResource.h"
#include "OgreWorkQueue.h"
#include "OgreHeaderPrefix.h"

#include <future>

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** Performs resource operations on the shared background work queue.

        All operations are queued via Root::getWorkQueue in order
        to share the thread pool amongst all background tasks. You should therefore
        refer to @ref background-resource for configuring the threads and for the
        lifetime and exception rules of tasks; this class merely provides an
        interface that is specific to resource handling.

        Every call returns a `std::future<void>` that becomes ready when the
        background portion of the operation has finished. This may be before the
        corresponding Resource::Listener callback has run, because those callbacks
        are dispatched back to the main thread. Use the future to wait for the data
        to become available, and a listener to observe the completed resource.

        Which resource methods can run on a worker thread depends on
        `OGRE_THREAD_SUPPORT`. With the default configuration (3), workers cannot
        access the RenderSystem: prepare() runs Resource::prepare in the background,
        while load() prepares in the background and performs the actual load on the
        main thread. With `OGRE_THREAD_SUPPORT == 1`, resources are fully thread-safe
        and load() runs entirely in the background.

        @see @ref background-resource
    */
    class _OgreExport ResourceBackgroundQueue : public Singleton<ResourceBackgroundQueue>
    {
    public:
        ResourceBackgroundQueue();
        virtual ~ResourceBackgroundQueue();

        /** Initialise a resource group in the background.
        @see ResourceGroupManager::initialiseResourceGroup
        @param name The name of the resource group to initialise
        */
        std::future<void> initialiseResourceGroup(const String& name);

        /** Initialise all resource groups which are yet to be initialised in 
            the background.
        @see ResourceGroupManager::intialiseResourceGroup
        */
        std::future<void> initialiseAllResourceGroups();
        /** Prepares a resource group in the background.
        @see ResourceGroupManager::prepareResourceGroup
        @param name The name of the resource group to prepare
        */
        std::future<void> prepareResourceGroup(const String& name);

        /** Loads a resource group in the background.
        @see ResourceGroupManager::loadResourceGroup
        @param name The name of the resource group to load
        */
        std::future<void> loadResourceGroup(const String& name);

        /** Unload a single resource in the background. 
        @see ResourceManager::unload
        */
        std::future<void> unload(const ResourcePtr& res);

        /** Unloads a resource group in the background.
        @see ResourceGroupManager::unloadResourceGroup
        @param name The name of the resource group to load
        */
        std::future<void> unloadResourceGroup(const String& name);

        /** Prepare a single resource in the background. 
        @see ResourceManager::prepare
        */
        std::future<void> prepare(const ResourcePtr& res);

        /** Load a single resource in the background. 
        @see ResourceManager::load
        */
        std::future<void> load(const ResourcePtr& res);

        /// @copydoc Singleton::getSingleton()
        static ResourceBackgroundQueue& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static ResourceBackgroundQueue* getSingletonPtr(void);

    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

