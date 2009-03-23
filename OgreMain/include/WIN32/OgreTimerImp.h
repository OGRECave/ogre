/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __Win32Timer_H__
#define __Win32Timer_H__

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // required to stop windows.h messing up std::min
#include "windows.h"

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

		DWORD mTimerMask;
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
