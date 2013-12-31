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
#ifndef __Win32Timer_H__
#define __Win32Timer_H__

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#endif
#include <windows.h>

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

	class _OgreExport Timer : public TimerAlloc
    {
    private:
		clock_t mZeroClock;

        DWORD mStartTick;
		LONGLONG mLastTime;
        LARGE_INTEGER mStartTime;
        LARGE_INTEGER mFrequency;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		DWORD_PTR mTimerMask;
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		DWORD GetTickCount() { return (DWORD)GetTickCount64(); }
#endif

    public:
		/** Timer constructor.  MUST be called on same thread that calls getMilliseconds() */
		Timer();
		~Timer();

		/** Method for setting a specific option of the Timer. These options are usually
            specific for a certain implementation of the Timer class, and may (and probably
            will) not exist across different implementations.  reset() must be called after
			all setOption() calls.
			@par
			Current options supported are:
			<ul><li>"QueryAffinityMask" (DWORD): Set the thread affinity mask to be used
			to check the timer. If 'reset' has been called already this mask should
			overlap with the process mask that was in force at that point, and should
			be a power of two (a single core).</li></ul>
            @param
                strKey The name of the option to set
            @param
                pValue A pointer to the value - the size should be calculated by the timer
                based on the key
            @return
                On success, true is returned.
            @par
                On failure, false is returned.
        */
        bool setOption( const String& strKey, const void* pValue );

		/** Resets timer */
		void reset();

		/** Returns milliseconds since initialisation or last reset */
		unsigned long getMilliseconds();

		/** Returns microseconds since initialisation or last reset */
		unsigned long getMicroseconds();

		/** Returns milliseconds since initialisation or last reset, only CPU time measured */
		unsigned long getMillisecondsCPU();

		/** Returns microseconds since initialisation or last reset, only CPU time measured */
		unsigned long getMicrosecondsCPU();
    };
	/** @} */
	/** @} */
}
#endif
