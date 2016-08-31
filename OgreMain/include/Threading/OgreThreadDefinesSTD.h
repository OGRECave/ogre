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
#ifndef __OgreThreadDefinesSTD_H__
#define __OgreThreadDefinesSTD_H__

namespace Ogre
{
    template< typename T > class ThreadLocalPtr
    {
    private:

        ThreadLocalPtr(const ThreadLocalPtr&) = delete;
        ThreadLocalPtr& operator = (const ThreadLocalPtr&) = delete;

        std::vector< std::unique_ptr<T> >& _getVect() const
        {
            thread_local std::vector< std::unique_ptr<T> > locals;
            return locals;
        }

        std::unique_ptr<T>& _get() const
        {
            return *std::next(std::begin(_getVect()), static_cast<int>(m_LocalID));
        }
    public:
        ThreadLocalPtr() : m_LocalID(m_VarCounter++) {}

        inline T* release()
        {
            _get().reset();
        }

        inline void reset(T* a = 0)
        {
            auto& vect = _getVect();
            if (vect.size() <= m_LocalID)
                vect.resize(static_cast<int>(m_LocalID) + 1);
            _get().reset(a);
        }

        inline T* get() const
        {
            return _get().get();
        }

        inline T* operator->() const
        {
            return _get().get();
        }

        inline T& operator*() const
        {
            return *_get();
        }

        static thread_local std::int64_t m_VarCounter;
        const std::int64_t m_LocalID;
    };

    template< typename T >
    thread_local std::int64_t ThreadLocalPtr<T>::m_VarCounter = 0;
}

#define OGRE_TOKEN_PASTE(x, y) x ## y
#define OGRE_TOKEN_PASTE_EXTRA(x, y) OGRE_TOKEN_PASTE(x, y)

#define OGRE_LOCK_AUTO_MUTEX std::unique_lock<std::recursive_mutex> ogreAutoMutexLock(OGRE_AUTO_MUTEX_NAME)
#define OGRE_LOCK_MUTEX(name) std::unique_lock<std::recursive_mutex> OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) std::unique_lock<std::recursive_mutex> lockName(mutexName)
#define OGRE_THREAD_SYNCHRONISER(sync) std::condition_variable_any sync
#define OGRE_THREAD_SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))

#define OGRE_AUTO_MUTEX mutable std::recursive_mutex OGRE_AUTO_MUTEX_NAME
#define OGRE_MUTEX(name) mutable std::recursive_mutex name
#define OGRE_STATIC_MUTEX(name) static std::recursive_mutex name
#define OGRE_STATIC_MUTEX_INSTANCE(name) std::recursive_mutex name
// like OGRE_AUTO_MUTEX but mutex held by pointer
#define OGRE_AUTO_SHARED_MUTEX mutable std::recursive_mutex *OGRE_AUTO_MUTEX_NAME
#define OGRE_LOCK_AUTO_SHARED_MUTEX assert(OGRE_AUTO_MUTEX_NAME); std::recursive_mutex::scoped_lock ogreAutoMutexLock(*OGRE_AUTO_MUTEX_NAME)
#define OGRE_NEW_AUTO_SHARED_MUTEX assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = new std::recursive_mutex()
#define OGRE_DELETE_AUTO_SHARED_MUTEX do { assert(OGRE_AUTO_MUTEX_NAME); delete OGRE_AUTO_MUTEX_NAME; } while (0)
#define OGRE_COPY_AUTO_SHARED_MUTEX(from) assert(!OGRE_AUTO_MUTEX_NAME); OGRE_AUTO_MUTEX_NAME = from
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL OGRE_AUTO_MUTEX_NAME = 0
#define OGRE_MUTEX_CONDITIONAL(mutex) if (mutex)
#define OGRE_THREAD_WAIT(sync, mutex, lock) sync.wait(lock)
#define OGRE_THREAD_NOTIFY_ONE(sync) sync.notify_one()
#define OGRE_THREAD_NOTIFY_ALL(sync) sync.notify_all()

// Read-write mutex
#define OGRE_RW_MUTEX(name) mutable std::recursive_mutex name
#define OGRE_LOCK_RW_MUTEX_READ(name) std::unique_lock<std::recursive_mutex> OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)
#define OGRE_LOCK_RW_MUTEX_WRITE(name) std::unique_lock<std::recursive_mutex> OGRE_TOKEN_PASTE_EXTRA(ogrenameLock, __LINE__) (name)

// Thread-local pointer
#define OGRE_THREAD_POINTER(T, var) Ogre::ThreadLocalPtr<T> var
#define OGRE_THREAD_POINTER_INIT(var) var()
#define OGRE_THREAD_POINTER_VAR(T, var) Ogre::ThreadLocalPtr<T> var ()
#define OGRE_THREAD_POINTER_SET(var, expr) var.reset(expr)
#define OGRE_THREAD_POINTER_GET(var) var.get()
#define OGRE_THREAD_POINTER_DELETE(var) var.reset(0)

// Thread objects and related functions
#define OGRE_THREAD_TYPE std::thread
#define OGRE_THREAD_CREATE(name, worker) std::thread* name = OGRE_NEW_T(std::thread, MEMCATEGORY_GENERAL)(worker)
#define OGRE_THREAD_DESTROY(name) OGRE_DELETE_T(name, thread, MEMCATEGORY_GENERAL)
#define OGRE_THREAD_HARDWARE_CONCURRENCY std::thread::hardware_concurrency()
#define OGRE_THREAD_CURRENT_ID std::this_thread::get_id()
#define OGRE_THREAD_WORKER_INHERIT

// Utility
#define OGRE_THREAD_ID_TYPE std::thread::id
#define OGRE_THREAD_YIELD std::this_thread::yield()

#endif
