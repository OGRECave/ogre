/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
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
#ifndef __OgreDefaultWorkQueueTBB_H__
#define __OgreDefaultWorkQueueTBB_H__

#include "../OgreWorkQueue.h"
#include <set>

namespace Ogre
{
	/** Implementation of a general purpose request / response style background work queue.
	@remarks
		This implementation utilises tbb's task system for the WorkQueue implementation.
	*/
	class _OgreExport DefaultWorkQueue : public DefaultWorkQueueBase
	{
	public:
		DefaultWorkQueue(const String& name = StringUtil::BLANK);
		virtual ~DefaultWorkQueue();
		/** Process the next request on the queue. 
		@remarks
			This method is public, but only intended for advanced users to call. 
			The only reason you would call this, is if you were using your 
			own thread to drive the worker processing. The thread calling this
			method will be the thread used to call the RequestHandler.
		*/

		/// Main function for each thread spawned.
		virtual void _threadMain();

		/// @copydoc WorkQueue::shutdown
		virtual void shutdown();

		/// @copydoc WorkQueue::startup
		virtual void startup(bool forceRestart = true);

		/// Register the current thread with the rendersystem
		void _registerThreadWithRenderSystem();

	protected:
		virtual void notifyWorkers();

	private:
		tbb::task_scheduler_init mTaskScheduler;
		tbb::task_group mTaskGroup;
		/// Synchronise registering threads with the RenderSystem
		OGRE_MUTEX(mRegisterRSMutex);
		std::set<tbb::tbb_thread::id> mRegisteredThreads;
	};


}

#endif

