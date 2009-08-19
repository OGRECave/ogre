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
#ifndef __OgreThreadDefinesPoco_H__
#define __OgreThreadDefinesPoco_H__

#define OGRE_AUTO_MUTEX mutable Poco::Mutex OGRE_AUTO_MUTEX_NAME;
#define OGRE_LOCK_AUTO_MUTEX Poco::Mutex::ScopedLock ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME);
#define OGRE_MUTEX(name) mutable Poco::Mutex name;
#define OGRE_STATIC_MUTEX(name) static Poco::Mutex name;
#define OGRE_STATIC_MUTEX_INSTANCE(name) Poco::Mutex name;
#define OGRE_LOCK_MUTEX(name) Poco::Mutex::ScopedLock ogrenameLock(name);
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) Poco::Mutex::ScopedLock lockName(mutexName);
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable Poco::Mutex *OGRE_AUTO_MUTEX_NAME;
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); Poco::Mutex::ScopedLock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME);
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new Poco::Mutex();
#define OGRE_DELETE_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME;
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from;
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0;
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
#define OGRE_THREAD_SYNCHRONISER(sync) Poco::Condition sync;
#define OGRE_THREAD_WAIT(sync, mutex, lock) sync.wait(mutex);
#define OGRE_THREAD_NOTIFY_ONE(sync) sync.signal(); 
#define OGRE_THREAD_NOTIFY_ALL(sync) sync.broadcast(); 
// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable Poco::RWLock name
#define OGRE_LOCK_RW_MUTEX_READ(name) Poco::RWLock::ScopedLock ogrenameLock(name, false);
#define OGRE_LOCK_RW_MUTEX_WRITE(name) Poco::RWLock::ScopedLock ogrenameLock(name, true);
// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) Poco::ThreadLocal<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_INIT(var) var()
#define OGRE_THREAD_POINTER_VAR(T, var) Poco::ThreadLocal<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_GET(var) var.get().get()
#define OGRE_THREAD_POINTER_SET(var, expr) { var.get().setNull(); var.get().bind(expr); }
#define OGRE_THREAD_POINTER_DELETE(var) var.get().setNull()
// Thread objects and related functions
#define OGRE_THREAD_TYPE Poco::Thread
#define OGRE_THREAD_CREATE(name, worker) Poco::Thread* name = OGRE_NEW_T(Poco::Thread, MEMCATEGORY_GENERAL)(); name->start(worker); 
#define OGRE_THREAD_DESTROY(name) OGRE_DELETE_T(name, Thread, MEMCATEGORY_GENERAL)
#define OGRE_THREAD_HARDWARE_CONCURRENCY 2
#define OGRE_THREAD_CURRENT_ID (size_t)Poco::Thread::current()
#define OGRE_THREAD_WORKER_INHERIT : public Poco::Runnable
// (hardware concurrency is not accessible via POCO atm)
// Utility
#define OGRE_THREAD_SLEEP(ms) Poco::Thread::sleep(ms);

#endif

