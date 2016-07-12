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

#ifndef __Barrier_H__
#define __Barrier_H__

#include "OgrePlatform.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    //No need to include the heavy windows.h header for something like this!
    typedef long LONG;
    typedef void* HANDLE;
#else
    #include <pthread.h>
    #if defined(ANDROID) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    typedef struct
    {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        int count;
        int tripCount;
    } pthread_barrier_t;
    #endif
#endif

namespace Ogre
{
    /** A barrier is a synchronization mechanism where multiple threads wait until all
        of them have reached the barrier sync point before continuing.
        A fixed number of threads must be provided on initialization.
    @remarks
        On Windows, Synchronization Barriers weren't introduced until Windows 8 (!?!?!? No comments...)
        Therefore, we emulate them using two Semaphores and (for performance reasons)
     @author
        Matias N. Goldberg
    */
    class _OgreExport Barrier
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        size_t                  mNumThreads;
        size_t                  mIndex;
        OGRE_ALIGNED_DECL( volatile LONG,   mLockCount,     4 );
        HANDLE                  mSemaphores[2];
#else
        pthread_barrier_t       mBarrier;
#endif

    public:
        Barrier( size_t threadCount );
        ~Barrier();

        /** When calling this function, it will block until all N threads reach this point; where
            N is the thread count passed to the Barrier's constructor.
        */
        void sync(void);
    };
}

#endif
