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
#ifndef __OgreThreadDefinesTBB_H__
#define __OgreThreadDefinesTBB_H__

#define OGRE_TOKEN_PASTE(x, y) x ## y
#define OGRE_TOKEN_PASTE_EXTRA(x, y) OGRE_TOKEN_PASTE(x, y)
#define OGRE_AUTO_MUTEX mutable tbb::recursive_mutex OGRE_AUTO_MUTEX_NAME
#define OGRE_LOCK_AUTO_MUTEX tbb::recursive_mutex::scoped_lock ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME)
#define OGRE_MUTEX(name) mutable tbb::recursive_mutex name
#define OGRE_STATIC_MUTEX(name) static tbb::recursive_mutex name
#define OGRE_STATIC_MUTEX_INSTANCE(name) tbb::recursive_mutex name
#define OGRE_LOCK_MUTEX(name) tbb::recursive_mutex::scoped_lock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) tbb::recursive_mutex::scoped_lock lockName(mutexName)
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable tbb::recursive_mutex *OGRE_AUTO_MUTEX_NAME
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); tbb::recursive_mutex::scoped_lock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME)
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new tbb::recursive_mutex()
#define OGRE_DELETE_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable tbb::queuing_rw_mutex name
#define OGRE_LOCK_RW_MUTEX_READ(name) tbb::queuing_rw_mutex::scoped_lock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name, false)
#define OGRE_LOCK_RW_MUTEX_WRITE(name) tbb::queuing_rw_mutex::scoped_lock OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name, true)
// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) tbb::enumerable_thread_specific<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_INIT(var) var()
#define OGRE_THREAD_POINTER_VAR(T, var) tbb::enumerable_thread_specific<SharedPtr<T> > var
#define OGRE_THREAD_POINTER_GET(var) var.local().get()
#define OGRE_THREAD_POINTER_SET(var, expr) do { var.local().setNull(); var.local().bind(expr); } while (0)
#define OGRE_THREAD_POINTER_DELETE(var) var.local().setNull()
// Utility
#define OGRE_THREAD_HARDWARE_CONCURRENCY tbb::task_scheduler_init::default_num_threads()
#define OGRE_THREAD_CURRENT_ID tbb::this_tbb_thread::get_id() 
#define OGRE_THREAD_SLEEP(ms) tbb::this_tbb_thread::sleep(tbb::tick_count::interval_t(double(ms)/1000))
#define OGRE_THREAD_WORKER_INHERIT
#define OGRE_THREAD_ID_TYPE tbb::tbb_thread::id
#define OGRE_THREAD_YIELD tbb::this_tbb_thread::yield()

#endif


