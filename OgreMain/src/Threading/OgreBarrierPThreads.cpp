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

#include "OgreStableHeaders.h"
#include "Threading/OgreBarrier.h"
#include <errno.h>

namespace Ogre
{

#if OGRE_CPU == OGRE_CPU_ARM
#define __dmb() asm volatile ( "dmb sy\n" ::: "cc" );
#endif

#if defined(ANDROID) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    typedef int pthread_barrierattr_t;
    //-----------------------------------------------------------------------------------
    int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, int count)
    {
        if(count == 0)
        {
            errno = EINVAL;
            return -1;
        }
        if(pthread_mutex_init(&barrier->mutex, 0) < 0)
        {
            return -1;
        }
        if(pthread_cond_init(&barrier->cond, 0) < 0)
        {
            pthread_mutex_destroy(&barrier->mutex);
            return -1;
        }
        barrier->tripCount = count;
        barrier->count = 0;
        
        return 0;
    }
    //-----------------------------------------------------------------------------------
    int pthread_barrier_destroy(pthread_barrier_t *barrier)
    {
        pthread_cond_destroy(&barrier->cond);
        pthread_mutex_destroy(&barrier->mutex);
        return 0;
    }
    //-----------------------------------------------------------------------------------
    int pthread_barrier_wait(pthread_barrier_t *barrier)
    {
        pthread_mutex_lock(&barrier->mutex);
        ++(barrier->count);
        if(barrier->count >= barrier->tripCount)
        {
            barrier->count = 0;
            pthread_cond_broadcast(&barrier->cond);
            pthread_mutex_unlock(&barrier->mutex);
            return 1;
        }
        else
        {
            pthread_cond_wait(&barrier->cond, &(barrier->mutex));
            pthread_mutex_unlock(&barrier->mutex);
            return 0;
        }
    }
    //-----------------------------------------------------------------------------------
#endif
    
    Barrier::Barrier( size_t threadCount )
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        pthread_barrier_init( &mBarrier, 0, static_cast<int>(threadCount) );
#endif
    }
    //-----------------------------------------------------------------------------------
    Barrier::~Barrier()
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        pthread_barrier_destroy( &mBarrier );
#endif
    }
    //-----------------------------------------------------------------------------------
    void Barrier::sync(void)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        pthread_barrier_wait( &mBarrier );
#endif
    }
}
