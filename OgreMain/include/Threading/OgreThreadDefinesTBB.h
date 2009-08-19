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
#ifndef __OgreThreadDefinesTBB_H__
#define __OgreThreadDefinesTBB_H__

#define OGRE_AUTO_MUTEX mutable tbb::recursive_mutex OGRE_AUTO_MUTEX_NAME;
#define OGRE_LOCK_AUTO_MUTEX tbb::recursive_mutex::scoped_lock ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME);
#define OGRE_MUTEX(name) mutable tbb::recursive_mutex name;
#define OGRE_STATIC_MUTEX(name) static tbb::recursive_mutex name;
#define OGRE_STATIC_MUTEX_INSTANCE(name) tbb::recursive_mutex name;
#define OGRE_LOCK_MUTEX(name) tbb::recursive_mutex::scoped_lock ogrenameLock(name);
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) tbb::recursive_mutex::scoped_lock lockName(mutexName);
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable tbb::recursive_mutex *OGRE_AUTO_MUTEX_NAME;
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); tbb::recursive_mutex::scoped_lock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME);
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new tbb::recursive_mutex();
#define OGRE_DELETE_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME;
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from;
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0;
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable tbb::queuing_rw_mutex name
#define OGRE_LOCK_RW_MUTEX_READ(name) tbb::queuing_rw_mutex::scoped_lock ogrenameLock(name, false)
#define OGRE_LOCK_RW_MUTEX_WRITE(name) tbb::queuing_rw_mutex::scoped_lock ogrenameLock(name, true)
// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) tbb::enumerable_thread_specific<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_INIT(var) var()
#define OGRE_THREAD_POINTER_VAR(T, var) tbb::enumerable_thread_specific<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_GET(var) var.local().get()
#define OGRE_THREAD_POINTER_SET(var, expr) { var.local().setNull(); var.local().bind(expr); }
#define OGRE_THREAD_POINTER_DELETE(var) var.local().setNull()
// Utility
#define OGRE_THREAD_HARDWARE_CONCURRENCY tbb::task_scheduler_init::default_num_threads()
#define OGRE_THREAD_CURRENT_ID tbb::this_tbb_thread::get_id() 
#define OGRE_THREAD_SLEEP(ms) tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(double(ms)/1000))
#define OGRE_THREAD_WORKER_INHERIT

#endif


