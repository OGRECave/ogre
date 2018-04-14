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
#ifndef __OgreThreadDefinesNone_H__
#define __OgreThreadDefinesNone_H__

#if OGRE_THREAD_SUPPORT != 3
#define OGRE_THREAD_HARDWARE_CONCURRENCY 1
#define OGRE_THREAD_CURRENT_ID "main"
#define OGRE_THREAD_WORKER_INHERIT

// will be defined by the respective thread provider
#define OGRE_WQ_MUTEX(name)
#define OGRE_WQ_LOCK_MUTEX(name)
#define OGRE_WQ_LOCK_MUTEX_NAMED(mutexName, lockName)

#define OGRE_WQ_RW_MUTEX(name)
#define OGRE_WQ_LOCK_RW_MUTEX_READ(name)
#define OGRE_WQ_LOCK_RW_MUTEX_WRITE(name)

#define OGRE_WQ_THREAD_SYNCHRONISER(sync)
#define OGRE_THREAD_NOTIFY_ONE(sync)
#define OGRE_THREAD_NOTIFY_ALL(sync)
#endif

#define OGRE_AUTO_MUTEX
#define OGRE_LOCK_AUTO_MUTEX
#define OGRE_STATIC_MUTEX(name)
#define OGRE_STATIC_MUTEX_INSTANCE(name)
#define OGRE_AUTO_SHARED_MUTEX
#define OGRE_LOCK_AUTO_SHARED_MUTEX
#define OGRE_NEW_AUTO_SHARED_MUTEX
#define OGRE_DELETE_AUTO_SHARED_MUTEX
#define OGRE_COPY_AUTO_SHARED_MUTEX(from)
#define OGRE_SET_AUTO_SHARED_MUTEX_NULL
#define OGRE_MUTEX_CONDITIONAL(name) if(true)

#define OGRE_THREAD_SLEEP(ms)
#define OGRE_THREAD_YIELD

#endif
