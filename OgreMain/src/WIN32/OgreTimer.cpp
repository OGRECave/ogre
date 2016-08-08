/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#include "OgreTimer.h"
#include "OgreBitwise.h"

using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
bool Timer::sUseQPCAffinityWorkaround = Timer::isQPCAffinityWorkaroundRequired();
bool Timer::isQPCAffinityWorkaroundRequired()
{
    // Workaround is needed for Windows XP and Windows 2000 and only if CPU has no invariant TSC or there are many CPUs
    // All OSes since Vista fallbacks to reliable timestamp sources in QPC implementation
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
    SYSTEM_INFO sysInfo;
    return (DWORD)(LOBYTE(LOWORD(GetVersion()))) < 6 // version.major < Vista
        && ((GetSystemInfo(&sysInfo), sysInfo.dwNumberOfProcessors > 1)
        || !PlatformInformation::hasCpuFeature(PlatformInformation::CPU_FEATURE_INVARIANT_TSC));
}
#endif
//-------------------------------------------------------------------------
Timer::Timer()
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    : mTimerMask(0)
#endif
{
    reset();
}
//-------------------------------------------------------------------------
Timer::~Timer()
{
}

//-------------------------------------------------------------------------
bool Timer::setOption( const String & key, const void * val )
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    if (sUseQPCAffinityWorkaround && key == "QueryAffinityMask" )
    {
        // Telling timer what core to use for a timer read
        DWORD newTimerMask = * static_cast < const DWORD * > ( val );

        // Get the current process core mask
        DWORD_PTR procMask;
        DWORD_PTR sysMask;
        GetProcessAffinityMask(GetCurrentProcess(), &procMask, &sysMask);

        // If new mask is 0, then set to default behavior, otherwise check
        // to make sure new timer core mask overlaps with process core mask
        // and that new timer core mask is a power of 2 (i.e. a single core)
        if( ( newTimerMask == 0 ) ||
            ( ( ( newTimerMask & procMask ) != 0 ) && Bitwise::isPO2( newTimerMask ) ) )
        {
            mTimerMask = newTimerMask;
            return true;
        }
    }
#endif

    return false;
}

//-------------------------------------------------------------------------
void Timer::reset()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    // Get the current process core mask
    HANDLE thread;
    DWORD_PTR oldMask;
    if (sUseQPCAffinityWorkaround)
    {
        DWORD_PTR procMask;
        DWORD_PTR sysMask;
        GetProcessAffinityMask(GetCurrentProcess(), &procMask, &sysMask);

        // If procMask is 0, consider there is only one core available
        // (using 0 as procMask will cause an infinite loop below)
        if (procMask == 0)
            procMask = 1;

        // Find the lowest core that this process uses
        if (mTimerMask == 0)
        {
            mTimerMask = 1;
            while ((mTimerMask & procMask) == 0)
            {
                mTimerMask <<= 1;
            }
        }

        thread = GetCurrentThread();

        // Set affinity to the first core


        oldMask = SetThreadAffinityMask(thread, mTimerMask);
    }
#endif

    // Get the constant frequency
    QueryPerformanceFrequency(&mFrequency);

    // Query the timer
    QueryPerformanceCounter(&mStartTime);
    mStartTick = GetTickCount();

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    // Reset affinity
    if (sUseQPCAffinityWorkaround)
        SetThreadAffinityMask(thread, oldMask);
#endif

    mLastTime = 0;
    mZeroClock = clock();
}

//-------------------------------------------------------------------------
unsigned long Timer::getMilliseconds()
{
    LARGE_INTEGER curTime;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    HANDLE thread;
    DWORD_PTR oldMask;
    if (sUseQPCAffinityWorkaround)
    {
        thread = GetCurrentThread();
        // Set affinity to the first core
        oldMask = SetThreadAffinityMask(thread, mTimerMask);
    }
#endif

    // Query the timer
    QueryPerformanceCounter(&curTime);

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    // Reset affinity
    if (sUseQPCAffinityWorkaround)
        SetThreadAffinityMask(thread, oldMask);
#endif

    LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;

    // scale by 1000 for milliseconds
    unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);

    // detect and compensate for performance counter leaps
    // (surprisingly common, see Microsoft KB: Q274323)
    unsigned long check = GetTickCount() - mStartTick;
    signed long msecOff = (signed long)(newTicks - check);
    if (msecOff < -100 || msecOff > 100)
    {
        // We must keep the timer running forward :)
        LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
        mStartTime.QuadPart += adjust;
        newTime -= adjust;

        // Re-calculate milliseconds
        newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
    }

    // Record last time for adjust
    mLastTime = newTime;

    return newTicks;
}

//-------------------------------------------------------------------------
unsigned long Timer::getMicroseconds()
{
    LARGE_INTEGER curTime;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    HANDLE thread;
    DWORD_PTR oldMask;
    if (sUseQPCAffinityWorkaround)
    {
        thread = GetCurrentThread();
        // Set affinity to the first core

        oldMask = SetThreadAffinityMask(thread, mTimerMask);
    }
#endif

    // Query the timer
    QueryPerformanceCounter(&curTime);

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && _WIN32_WINNT < _WIN32_WINNT_VISTA
    // Reset affinity
    if (sUseQPCAffinityWorkaround)
        SetThreadAffinityMask(thread, oldMask);
#endif

    LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;

    // get milliseconds to check against GetTickCount
    unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
    
    // detect and compensate for performance counter leaps
    // (surprisingly common, see Microsoft KB: Q274323)
    unsigned long check = GetTickCount() - mStartTick;
    signed long msecOff = (signed long)(newTicks - check);
    if (msecOff < -100 || msecOff > 100)
    {
        // We must keep the timer running forward :)
        LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
        mStartTime.QuadPart += adjust;
        newTime -= adjust;
    }

    // Record last time for adjust
    mLastTime = newTime;

    // scale by 1000000 for microseconds
    unsigned long newMicro = (unsigned long) (1000000 * newTime / mFrequency.QuadPart);

    return newMicro;
}

//-------------------------------------------------------------------------
unsigned long Timer::getMillisecondsCPU()
{
    clock_t newClock = clock();
    return (unsigned long)( (float)( newClock - mZeroClock ) / ( (float)CLOCKS_PER_SEC / 1000.0 ) ) ;
}

//-------------------------------------------------------------------------
unsigned long Timer::getMicrosecondsCPU()
{
    clock_t newClock = clock();
    return (unsigned long)( (float)( newClock - mZeroClock ) / ( (float)CLOCKS_PER_SEC / 1000000.0 ) ) ;
}
