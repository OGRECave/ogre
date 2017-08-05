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
#ifndef __OgreThreadDefines_H__
#define __OgreThreadDefines_H__

#define OGRE_AUTO_MUTEX_NAME mutex
#if OGRE_THREAD_PROVIDER == 0
    #include "OgreThreadDefinesNone.h"
#elif OGRE_THREAD_PROVIDER == 1
    #include "OgreThreadDefinesBoost.h"
#elif OGRE_THREAD_PROVIDER == 2
    #include "OgreThreadDefinesPoco.h"
#elif OGRE_THREAD_PROVIDER == 3
    #include "OgreThreadDefinesTBB.h"
#elif OGRE_THREAD_PROVIDER == 4
   #include "OgreThreadDefinesSTD.h"
#endif

#if OGRE_THREAD_SUPPORT == 3
    #include "OgreThreadDefinesNone.h"

    // all empty definitions
    #define OGRE_MUTEX(name)

    #define OGRE_LOCK_MUTEX(name)
    #define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName)

    #define OGRE_RW_MUTEX(name)
    #define OGRE_LOCK_RW_MUTEX_READ(name)
    #define OGRE_LOCK_RW_MUTEX_WRITE(name)
    #define OGRE_THREAD_SYNCHRONISER(sync)
#else
    // alias to WQ names
    #define OGRE_MUTEX(name) OGRE_WQ_MUTEX(name)

    #define OGRE_LOCK_MUTEX(name) OGRE_WQ_LOCK_MUTEX(name)
    #define OGRE_LOCK_MUTEX_NAMED(mutexName, lockName) OGRE_WQ_LOCK_MUTEX_NAMED(mutexName, lockName)

    #define OGRE_RW_MUTEX(name) OGRE_WQ_RW_MUTEX(name)
    #define OGRE_LOCK_RW_MUTEX_READ(name) OGRE_WQ_LOCK_RW_MUTEX_READ(name)
    #define OGRE_LOCK_RW_MUTEX_WRITE(name) OGRE_WQ_LOCK_RW_MUTEX_WRITE(name)
    #define OGRE_THREAD_SYNCHRONISER(sync) OGRE_WQ_THREAD_SYNCHRONISER(sync)
#endif

#endif

