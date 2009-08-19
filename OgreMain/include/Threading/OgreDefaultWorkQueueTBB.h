/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
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

