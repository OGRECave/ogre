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
#if OGRE_MEMORY_ALLOCATOR == OGRE_MEMORY_ALLOCATOR_TRACK

#include "OgreMemoryTrackAlloc.h"
#include "Threading/OgreLightweightMutex.h"

namespace Ogre
{
    //1GB pool
    const size_t OGRE_TRACK_POOL_SIZE = 1024u * 1024u * 1024u;

    /*char* cleanMemory( size_t bytes )
    {
        char *retVal = (char*)malloc( bytes );
        memset( retVal, 0, bytes );

        return retVal;
    }*/

    LightweightMutex Mutex;
    char*   TrackAllocPolicy::MemoryPool    = 0;
    size_t  TrackAllocPolicy::CurrentOffset = 0;
    char    TrackAllocPolicy::Magic[4]      = { 0x4F, 0x47, 0x52, 0x45 };
    size_t  TrackAllocPolicy::RandomOffset  = 987026;

    DECL_MALLOC void* TrackAllocPolicy::allocateBytes(size_t count, 
#if OGRE_MEMORY_TRACKER
            const char* file, int line, const char* func
#else
            const char*, int, const char*
#endif
            )
    {
        if( !count )
            return 0;

        if( !MemoryPool )
        {
            //Initialize the pool
            MemoryPool= (char*)malloc( OGRE_TRACK_POOL_SIZE );
            CurrentOffset   = 0;
            RandomOffset    = 987026;
            Magic[0] = 0x4F;
            Magic[1] = 0x47;
            Magic[2] = 0x52;
            Magic[3] = 0x45;

            size_t j=0;
            for( int i=0; i<OGRE_TRACK_POOL_SIZE; ++i )
            {
                MemoryPool[i] = Magic[j];
                j = (j + 1) % 4;
            }

            //This isn't thread safe, but oh well, it's never meant to be
            //used in shipment, and it gets called at initialization
            Mutex = LightweightMutex();
        }

        Mutex.lock();

        size_t oldOffset = CurrentOffset;

        //Count is first 8 bytes more, then rounded up to multiple of size_t
        count += 8;
        size_t internalCount = ((count + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
        CurrentOffset += internalCount;

        assert( CurrentOffset < OGRE_TRACK_POOL_SIZE && "Pool's memory limit reached!!!" );

        size_t j = oldOffset % 4;
        for( size_t i=oldOffset; i<CurrentOffset; ++i )
        {
            assert( MemoryPool[i] == Magic[j] && "Memory corruption detected!!!" );
            j = (j + 1) % 4;
        }

        *(uint32*)(MemoryPool+oldOffset)    = count;
        *(uint32*)(MemoryPool+oldOffset+4)  = count + RandomOffset;

        Mutex.unlock();

        return MemoryPool + oldOffset + 8;
    }

    void TrackAllocPolicy::deallocateBytes(void* _ptr)
    {
        //Nothing goes back to the pool, but write the magic words again
        if( !_ptr )
            return;

        Mutex.lock();
        char *ptr = (char*)_ptr - 8;

        if( ptr < MemoryPool || ptr >= MemoryPool + OGRE_TRACK_POOL_SIZE )
            assert( false && "We didn't create this pointer!!!" );

        uint32 count0 = *(uint32*)(ptr);
        uint32 count1 = *(uint32*)(ptr+4);

        assert( count0 + RandomOffset == count1 && "Memory corruption detected!!!" );
        assert( count0 < OGRE_TRACK_POOL_SIZE && "Memory corruption detected!!!" );

        //Set the magic word again.
        size_t start = (ptr - MemoryPool);
        size_t j = start % 4;
        for( size_t i=start; i<start + count0; ++i )
        {
            MemoryPool[i] = Magic[j];
            j = (j + 1) % 4;
        }

        size_t internalCount = ((count0 + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
        for( size_t i=start + count0; i<start + internalCount; ++i )
        {
            assert( MemoryPool[i] == Magic[j] && "Memory corruption detected!!!" );
            j = (j + 1) % 4;
        }

        /*bool foreignMagicFailed = false;
        for( size_t i=internalCount; i<internalCount+8; ++i )
        {
            foreignMagicFailed |= MemoryPool[i] != Magic[j];
            j = (j + 1) % 4;
        }

        uint32 foreignCount0 = *(uint32*)(MemoryPool+internalCount);
        uint32 foreignCount1 = *(uint32*)(MemoryPool+internalCount+4);

        if( foreignCount0 + RandomOffset != foreignCount1 &&
            foreignMagicFailed )
        {
            assert( false && "Memory corruption detected!!!" );
        }*/
        Mutex.unlock();
    }
}


#endif

