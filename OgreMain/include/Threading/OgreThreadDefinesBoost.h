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
#ifndef __OgreThreadDefinesBoost_H__
#define __OgreThreadDefinesBoost_H__

#define OGRE_AUTO_MUTEX mutable boost::recursive_mutex OGRE_AUTO_MUTEX_NAME;
#define OGRE_LOCK_AUTO_MUTEX boost::recursive_mutex::scoped_lock ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME);
#define OGRE_MUTEX(name) mutable boost::recursive_mutex name;
#define OGRE_STATIC_MUTEX(name) static boost::recursive_mutex name;
#define OGRE_STATIC_MUTEX_INSTANCE(name) boost::recursive_mutex name;
#define OGRE_LOCK_MUTEX(name) boost::recursive_mutex::scoped_lock ogrenameLock(name);
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) boost::recursive_mutex::scoped_lock lockName(mutexName);
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable boost::recursive_mutex *OGRE_AUTO_MUTEX_NAME;
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); boost::recursive_mutex::scoped_lock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME);
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new boost::recursive_mutex();
#define OGRE_DELETE_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME;
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from;
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0;
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
#define OGRE_THREAD_SYNCHRONISER(sync) boost::condition sync;
#define OGRE_THREAD_WAIT(sync, mutex, lock) sync.wait(lock);
#define OGRE_THREAD_NOTIFY_ONE(sync) sync.notify_one(); 
#define OGRE_THREAD_NOTIFY_ALL(sync) sync.notify_all(); 
// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable boost::shared_mutex name
#define OGRE_LOCK_RW_MUTEX_READ(name) boost::shared_lock<boost::shared_mutex> ogrenameLock(name)
#define OGRE_LOCK_RW_MUTEX_WRITE(name) boost::unique_lock<boost::shared_mutex> ogrenameLock(name)
// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) boost::thread_specific_ptr<T> var
#define OGRE_THREAD_POINTER_INIT(var) var(&deletePtr)
#define OGRE_THREAD_POINTER_VAR(T, var) boost::thread_specific_ptr<T> var (&deletePtr<T>)
#define OGRE_THREAD_POINTER_SET(var, expr) var.reset(expr)
#define OGRE_THREAD_POINTER_GET(var) var.get()
#define OGRE_THREAD_POINTER_DELETE(var) var.reset(0)
// Thread objects and related functions
#define OGRE_THREAD_TYPE boost::thread
#define OGRE_THREAD_CREATE(name, worker) boost::thread* name = OGRE_NEW_T(boost::thread, MEMCATEGORY_GENERAL)(worker);
#define OGRE_THREAD_DESTROY(name) OGRE_DELETE_T(name, thread, MEMCATEGORY_GENERAL)
#define OGRE_THREAD_HARDWARE_CONCURRENCY boost::thread::hardware_concurrency()
#define OGRE_THREAD_CURRENT_ID boost::this_thread::get_id()
#define OGRE_THREAD_WORKER_INHERIT
// Utility
#define OGRE_THREAD_SLEEP(ms) boost::this_thread::sleep(boost::posix_time::millisec(ms));

#endif
