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
#ifndef __OgreThreadDefinesBoost_H__
#define __OgreThreadDefinesBoost_H__

#define OGRE_TOKEN_PASTE(x, y) x ## y
#define OGRE_TOKEN_PASTE_EXTRA(x, y) OGRE_TOKEN_PASTE(x, y)

#if BOOST_THREAD_VERSION < 4
#define OGRE_LOCK_AUTO_MUTEX boost::recursive_mutex::scoped_lock ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME)
#define OGRE_LOCK_MUTEX(name) boost::recursive_mutex::scoped_lock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) boost::recursive_mutex::scoped_lock lockName(mutexName)
#define OGRE_THREAD_SYNCHRONISER(sync) boost::condition sync
#define OGRE_THREAD_SLEEP(ms) boost::this_thread::sleep(boost::posix_time::millisec(ms))
#else
#define OGRE_LOCK_AUTO_MUTEX boost::unique_lock<boost::recursive_mutex> ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME)
#define OGRE_LOCK_MUTEX(name) boost::unique_lock<boost::recursive_mutex> OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) boost::unique_lock<boost::recursive_mutex> lockName(mutexName)
#define OGRE_THREAD_SYNCHRONISER(sync) boost::condition_variable_any sync
#define OGRE_THREAD_SLEEP(ms) boost::this_thread::sleep_for(boost::chrono::milliseconds(ms))
#endif

#define OGRE_AUTO_MUTEX mutable boost::recursive_mutex OGRE_AUTO_MUTEX_NAME
#define OGRE_MUTEX(name) mutable boost::recursive_mutex name
#define OGRE_STATIC_MUTEX(name) static boost::recursive_mutex name
#define OGRE_STATIC_MUTEX_INSTANCE(name) boost::recursive_mutex name
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable boost::recursive_mutex *OGRE_AUTO_MUTEX_NAME
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); boost::recursive_mutex::scoped_lock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME)
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new boost::recursive_mutex()
#define OGRE_DELETE_AUTO_SHARED_MUTEX do { assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME; } while (0)
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
#define OGRE_THREAD_WAIT(sync, mutex, lock) sync.wait(lock)
#define OGRE_THREAD_NOTIFY_ONE(sync) sync.notify_one()
#define OGRE_THREAD_NOTIFY_ALL(sync) sync.notify_all()
// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable boost::shared_mutex name
#define OGRE_LOCK_RW_MUTEX_READ(name) boost::shared_lock<boost::shared_mutex> OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_RW_MUTEX_WRITE(name) boost::unique_lock<boost::shared_mutex> OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) boost::thread_specific_ptr<T> var
#define OGRE_THREAD_POINTER_INIT(var) var(&deletePtr)
#define OGRE_THREAD_POINTER_VAR(T, var) boost::thread_specific_ptr<T> var (&deletePtr<T>)
#define OGRE_THREAD_POINTER_SET(var, expr) var.reset(expr)
#define OGRE_THREAD_POINTER_GET(var) var.get()
#define OGRE_THREAD_POINTER_DELETE(var) var.reset(0)
// Thread objects and related functions
#define OGRE_THREAD_TYPE boost::thread
#define OGRE_THREAD_CREATE(name, worker) boost::thread* name = OGRE_NEW_T(boost::thread, MEMCATEGORY_GENERAL)(worker)
#define OGRE_THREAD_DESTROY(name) OGRE_DELETE_T(name, thread, MEMCATEGORY_GENERAL)
#define OGRE_THREAD_HARDWARE_CONCURRENCY boost::thread::hardware_concurrency()
#define OGRE_THREAD_CURRENT_ID boost::this_thread::get_id()
#define OGRE_THREAD_WORKER_INHERIT
// Utility
#define OGRE_THREAD_ID_TYPE boost::thread::id
#define OGRE_THREAD_YIELD boost::this_thread::yield()

#endif
