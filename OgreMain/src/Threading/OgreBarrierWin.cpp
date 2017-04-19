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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#ifndef __MINGW32__
	#include <intrin.h>
#else
    // MinGW needs some extra headers and define MemoryBarrier manually
	#include <x86intrin.h>
    #include <winbase.h>
    #include <windef.h>

    #define MemoryBarrier __sync_synchronize
#endif

namespace Ogre
{
    Barrier::Barrier( size_t threadCount ) : mNumThreads( threadCount ), mIndex( 0 ), mLockCount( 0 )
    {
        for( size_t i=0; i<2; ++i )
            mSemaphores[i] = CreateSemaphore( NULL, 0, mNumThreads, NULL );
    }
    //-----------------------------------------------------------------------------------
    Barrier::~Barrier()
    {
        for( size_t i=0; i<2; ++i )
            CloseHandle( mSemaphores[i] );
    }
    //-----------------------------------------------------------------------------------
    void Barrier::sync(void)
    {
        //We need to be absolutely certain we read mIndex before incrementing mLockCount
        volatile size_t idx = mIndex;
        MemoryBarrier();

        #ifndef __MINGW32__
            LONG oldLockCount = _InterlockedExchangeAdd( &mLockCount, 1 );
        #else
            LONG oldLockCount = InterlockedExchangeAdd( &mLockCount, 1 );
        #endif
        if( oldLockCount != mNumThreads - 1 )
        {
            WaitForSingleObject( mSemaphores[idx], INFINITE );
        }
        else
        {
            //Swap the index to use the other semaphore. Otherwise a thread that runs too fast
            //gets to the next sync point and enters the semaphore, causing threads from this
            //one to get stuck in the current sync point (and ultimately, deadlock).
            mIndex      = !idx;
            mLockCount  = 0;
            if( mNumThreads > 1 )
                ReleaseSemaphore( mSemaphores[idx], mNumThreads - 1, NULL );
        }
    }
}