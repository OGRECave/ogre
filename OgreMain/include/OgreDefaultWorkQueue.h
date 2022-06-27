/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------*/
#ifndef __OgreDefaultWorkQueue_H__
#define __OgreDefaultWorkQueue_H__

#include "OgreWorkQueue.h"

namespace Ogre
{
    /** Implementation of a general purpose request / response style background work queue.

        This default implementation of a work queue starts a thread pool and 
        provides queues to process requests. 
    */
    class _OgreExport DefaultWorkQueue : public DefaultWorkQueueBase
    {
    public:

        DefaultWorkQueue(const String& name = BLANKSTRING);
        virtual ~DefaultWorkQueue(); 

        /// Main function for each thread spawned.
        void _threadMain() override;

        /// @copydoc WorkQueue::shutdown
        void shutdown() override;

        /// @copydoc WorkQueue::startup
        void startup(bool forceRestart = true) override;

    protected:
        /** To be called by a separate thread; will return immediately if there
            are items in the queue, or suspend the thread until new items are added
            otherwise.
        */
        virtual void waitForNextRequest();

        /// Notify that a thread has registered itself with the render system
        virtual void notifyThreadRegistered();

        void notifyWorkers() override;

        size_t mNumThreadsRegisteredWithRS;
        /// Init notification mutex (must lock before waiting on initCondition)
        OGRE_WQ_MUTEX(mInitMutex);
        /// Synchroniser token to wait / notify on thread init 
        OGRE_WQ_THREAD_SYNCHRONISER(mInitSync);

        OGRE_WQ_THREAD_SYNCHRONISER(mRequestCondition);
#if OGRE_THREAD_SUPPORT
        typedef std::vector<OGRE_THREAD_TYPE*> WorkerThreadList;
        WorkerThreadList mWorkers;
#endif

    };

}

#endif
