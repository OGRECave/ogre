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
#ifndef __OgreDefaultWorkQueueStandard_H__
#define __OgreDefaultWorkQueueStandard_H__

#include "../OgreWorkQueue.h"

namespace Ogre
{
	/** Implementation of a general purpose request / response style background work queue.
	@remarks
		This default implementation of a work queue starts a thread pool and 
		provides queues to process requests. 
	*/
	class _OgreExport DefaultWorkQueue : public DefaultWorkQueueBase
	{
	public:

		DefaultWorkQueue(const String& name = StringUtil::BLANK);
		virtual ~DefaultWorkQueue(); 

		/// Main function for each thread spawned.
		virtual void _threadMain();

		/// @copydoc WorkQueue::shutdown
		virtual void shutdown();

		/// @copydoc WorkQueue::startup
		virtual void startup(bool forceRestart = true);

	protected:
		/** To be called by a separate thread; will return immediately if there
			are items in the queue, or suspend the thread until new items are added
			otherwise.
		*/
		virtual void waitForNextRequest();

		/// Notify that a thread has registered itself with the render system
		virtual void notifyThreadRegistered();

		virtual void notifyWorkers();

		size_t mNumThreadsRegisteredWithRS;
		/// Init notification mutex (must lock before waiting on initCondition)
		OGRE_MUTEX(mInitMutex)
		/// Synchroniser token to wait / notify on thread init 
		OGRE_THREAD_SYNCHRONISER(mInitSync)

		OGRE_THREAD_SYNCHRONISER(mRequestCondition)
#if OGRE_THREAD_SUPPORT
		typedef vector<OGRE_THREAD_TYPE*>::type WorkerThreadList;
		WorkerThreadList mWorkers;
#endif

	};

}

#endif
