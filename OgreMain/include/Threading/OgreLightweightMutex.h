/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __LightweightMutex_H__
#define __LightweightMutex_H__

#include "OgrePlatform.h"
#include "OgrePlatformInformation.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    //No need to include the heavy windows.h header for something like this!
    typedef void* HANDLE;
#else
    #include <pthread.h>
#endif

namespace Ogre
{
    /** A lightweight mutex is a synchronization mechanism, very similar to a regular mutex.
        Regular mutexes are well known to be expensive because they need to enter & leave
        kernel mode.
    @par
        Lightweight mutexes usually make use of atomic instructions (in x86 they're referred to
        as instructions with the "lock" prefix) to avoid entering kernel mode when no other
        thread has aquired the lock.
    @par
        This comes with a couple caveats that we don't care for the applications we need,
        but is important to keep them in mind:
            1) LightweightMutexes can't be shared among processes
            2) Priority inversion is not a concern
            3) Recursive locks aren't supported (same thread calling lock() twice can deadlock)
        Note that some LightweightMutex implementations may offer this functionality, but we don't
        guarantee them in all platforms/architectures.
        It is possible to write a lightweight mutex that supports recursive locks, but that requires
        a call to GetCurrentThreadId (in Windows), which as much as saying just use a regular mutex.
    @par
        Windows users are familiar with the concept of LightweightMutexes because there is already
        an implementation provided: CRITICAL_SECTION. We go further by reinventing the wheel and
        writting it ourselves. If you ever wondered how a CRITICAL_SECTION works, now you know.
    @par
        Interesting reads:
            http://preshing.com/20111124/always-use-a-lightweight-mutex
            http://preshing.com/20120226/roll-your-own-lightweight-mutex
            http://preshing.com/20120305/implementing-a-recursive-mutex
    */
    class _OgreExport LightweightMutex
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        OGRE_ALIGNED_DECL( long, mCounter, 4 );
        HANDLE  mSemaphore;
#else
        //The joys of POSIX programming... PThread mutexes are already lightweight.
        //MS is lighyears behind UNIX regarding thread synchronization. (I'm sorry
        //if I don't like CRITICAL_SECTION; MS changed their behavior between XP &
        //Vista. We need consistent behavior)
        pthread_mutex_t mMutex;
#endif

    public:
        LightweightMutex();
        ~LightweightMutex();

        /** Acquires the exclusive lock. Waits if necessary until another thread releases the lock.
            Recursive locking is not guaranteed (do not call this function twice from the same thread)
        */
        void lock();

        /** Tries to aquire the lock and returns immediately.
            On failure returns false, true on success
        */
        bool tryLock();

        /// Releases the lock aquired through either @see lock or @see tryLock
        void unlock();
    };
}

#endif
