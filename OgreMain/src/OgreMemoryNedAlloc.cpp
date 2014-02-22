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
#include "OgrePrerequisites.h"
#include "OgreMemoryNedAlloc.h"
#include "OgrePlatformInformation.h"
#include "OgreMemoryTracker.h"

#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_NED

// include ned implementation
// don't abort() on asserts, behave as normal assert()
#define ABORT_ON_ASSERT_FAILURE 0
#include <nedmalloc.c>

namespace Ogre
{

    //---------------------------------------------------------------------
    void* NedAllocImpl::allocBytes(size_t count, 
        const char* file, int line, const char* func)
    {
        void* ptr = nedalloc::nedmalloc(count);
    #if OGRE_MEMORY_TRACKER
        // this alloc policy doesn't do pools (yet, ned can do it)
        MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
    #endif
        return ptr;
    }
    //---------------------------------------------------------------------
    void NedAllocImpl::deallocBytes(void* ptr)
    {
        // deal with null
        if (!ptr)
            return;
#if OGRE_MEMORY_TRACKER
        MemoryTracker::get()._recordDealloc(ptr);
#endif
        nedalloc::nedfree(ptr);
    }
    //---------------------------------------------------------------------
    void* NedAllocImpl::allocBytesAligned(size_t align, size_t count, 
        const char* file, int line, const char* func)
    {
        // default to platform SIMD alignment if none specified
        void* ptr =  align ? nedalloc::nedmemalign(align, count)
            : nedalloc::nedmemalign(OGRE_SIMD_ALIGNMENT, count);
#if OGRE_MEMORY_TRACKER
        // this alloc policy doesn't do pools (yet, ned can do it)
        MemoryTracker::get()._recordAlloc(ptr, count, 0, file, line, func);
#endif
        return ptr;
    }
    //---------------------------------------------------------------------
    void NedAllocImpl::deallocBytesAligned(size_t align, void* ptr)
    {
        // deal with null
        if (!ptr)
            return;
#if OGRE_MEMORY_TRACKER
        // this alloc policy doesn't do pools (yet, ned can do it)
        MemoryTracker::get()._recordDealloc(ptr);
#endif
        nedalloc::nedfree(ptr);
    }


}


#endif

