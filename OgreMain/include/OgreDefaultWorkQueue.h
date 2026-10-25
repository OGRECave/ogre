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
    /** Threaded implementation of a general purpose task-based background work queue.

        This implementation starts a pool of worker threads on @ref startup and
        provides the FIFO queues for background and main-thread tasks. Worker threads
        are named `[Ogre] Worker` where the platform supports it.

        @see @ref background-resource
    */
    class _OgreExport DefaultWorkQueue : public DefaultWorkQueueBase
    {
    public:

        DefaultWorkQueue(const String& name = BLANKSTRING);
        virtual ~DefaultWorkQueue(); 

        /** Worker thread loop.

            Waits for tasks and executes them until @ref shutdown is requested.
            With workers allowed to access the render system, the thread registers
            itself before entering the loop.
        */
        void _threadMain() override;

        /** Stop and join all worker threads.

            Tasks that are already running are allowed to finish. Tasks still waiting
            in the queues are not executed and are left in the queues, so they may run
            if the queue is started again before it is destroyed. The destructor calls
            this method.
        */
        void shutdown() override;

        /** Start the worker threads.

            @param forceRestart If the queue is already running, shut it down first.
                Otherwise this is a no-op while running.
        */
        void startup(bool forceRestart = true) override;

    protected:
        /** To be called by a worker thread; returns immediately if there
            are tasks in the queue, otherwise suspends the thread until new tasks
            are added or a shutdown is requested.
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
