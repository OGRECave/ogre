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

    /** This class is used to perform Resource operations in a
        background thread. 

        All these requests are now queued via Root::getWorkQueue in order
        to share the thread pool amongst all background tasks. You should therefore
        refer to that class for configuring the behaviour of the threads
        themselves, this class merely provides an interface that is specific
        to resource loading around this common functionality.
    @par
        The general approach here is that on requesting a background resource
        process, your request is placed on a queue ready for the background
        thread to be picked up, and you will get a 'ticket' back, identifying
        the request. Your call will then return and your thread can
        proceed, knowing that at some point in the background the operation will 
        be performed. In it's own thread, the resource operation will be 
        performed, and once finished the ticket will be marked as complete.
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

