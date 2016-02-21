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
#ifndef __OgreThreadDefinesPoco_H__
#define __OgreThreadDefinesPoco_H__

#define OGRE_TOKEN_PASTE(x, y) x ## y
#define OGRE_TOKEN_PASTE_EXTRA(x, y) OGRE_TOKEN_PASTE(x, y)
#define OGRE_AUTO_MUTEX mutable Poco::Mutex OGRE_AUTO_MUTEX_NAME
#define OGRE_LOCK_AUTO_MUTEX Poco::Mutex::ScopedLock ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME)
#define OGRE_MUTEX(name) mutable Poco::Mutex name
#define OGRE_STATIC_MUTEX(name) static Poco::Mutex name
#define OGRE_STATIC_MUTEX_INSTANCE(name) Poco::Mutex name
#define OGRE_LOCK_MUTEX(name) Poco::Mutex::ScopedLock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) Poco::Mutex::ScopedLock lockName(mutexName)
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable Poco::Mutex *OGRE_AUTO_MUTEX_NAME
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); Poco::Mutex::ScopedLock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME)
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new Poco::Mutex()
#define OGRE_DELETE_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
#define OGRE_THREAD_SYNCHRONISER(sync) Poco::Condition sync
#define OGRE_THREAD_WAIT(sync, mutex, lock) sync.wait(mutex)
#define OGRE_THREAD_NOTIFY_ONE(sync) sync.signal()
#define OGRE_THREAD_NOTIFY_ALL(sync) sync.broadcast()
// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable Poco::RWLock name
#define OGRE_LOCK_RW_MUTEX_READ(name) Poco::RWLock::ScopedLock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name, false)
#define OGRE_LOCK_RW_MUTEX_WRITE(name) Poco::RWLock::ScopedLock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name, true)
// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) Poco::ThreadLocal<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_INIT(var) var()
#define OGRE_THREAD_POINTER_VAR(T, var) Poco::ThreadLocal<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_GET(var) var.get().get()
#define OGRE_THREAD_POINTER_SET(var, expr) do { var.get().setNull(); var.get().bind(expr); } while (0)
#define OGRE_THREAD_POINTER_DELETE(var) var.get().setNull()
// Thread objects and related functions
#define OGRE_THREAD_TYPE Poco::Thread
#define OGRE_THREAD_CREATE(name, worker) Poco::Thread* name = OGRE_NEW_T(Poco::Thread, MEMCATEGORY_GENERAL)(); name->start(worker)
#define OGRE_THREAD_DESTROY(name) OGRE_DELETE_T(name, Thread, MEMCATEGORY_GENERAL)
#define OGRE_THREAD_HARDWARE_CONCURRENCY Poco::Environment::processorCount()
#define OGRE_THREAD_CURRENT_ID (size_t)Poco::Thread::current()
#define OGRE_THREAD_WORKER_INHERIT : public Poco::Runnable
// (hardware concurrency is not accessible via POCO atm)
// Utility
#define OGRE_THREAD_SLEEP(ms) Poco::Thread::sleep(ms)
#define OGRE_THREAD_ID_TYPE size_t
#define OGRE_THREAD_YIELD Poco::Thread::yield()

#endif
