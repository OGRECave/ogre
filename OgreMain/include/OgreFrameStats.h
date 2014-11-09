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
#ifndef __FrameStats_H__
#define __FrameStats_H__

#include "OgreMemoryAllocatorConfig.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

#define OGRE_FRAME_STATS_SAMPLES 60
    /** All return values are either in milliseconds or frames per second;
        but they're internally stored in microseconds
    */
    class _OgreExport FrameStats : public ProfilerAlloc
    {
        int mNextFrame;
        unsigned long mBestFrameTime;
        unsigned long mWorstFrameTime;
        unsigned long mLastTime;
        unsigned long mFrameTimes[OGRE_FRAME_STATS_SAMPLES];
        size_t mFramesSampled;

    public:
        FrameStats(void)
        {
            reset( 0 );
        }

        float getFps(void) const                { return 1000.0f / getLastTime(); }
        float getAvgFps(void) const             { return 1000.0f / getAvgTime(); }

        float getBestTime(void) const   { return mBestFrameTime * 0.001f; }
        float getWorstTime(void) const  { return mWorstFrameTime * 0.001f; }
        float getLastTime(void) const
        {
            return mFrameTimes[(mNextFrame + OGRE_FRAME_STATS_SAMPLES - 1) %
                                OGRE_FRAME_STATS_SAMPLES] * 0.001f;
        }

        float getAvgTime(void) const
        {
            if( !mFramesSampled )
                return 0.0f;

            unsigned long avg = 0;

            for( size_t i=0; i<mFramesSampled; ++i )
            {
                //idx is first in the range [(mNextFrame - mFramesSampled); mNextFrame),
                //but then warped around mNextFrame
                size_t idx = (i + mNextFrame + (mFramesSampled * 2 - 1)) % mFramesSampled;
                avg += mFrameTimes[idx];
            }

            return avg / (float)mFramesSampled * 0.001f;
        }

        /// Adds a new measured time, in *microseconds*
        void addSample( unsigned long timeMs )
        {
            unsigned long frameTimeMs = timeMs - mLastTime;
            mFrameTimes[mNextFrame]  = frameTimeMs;
            mBestFrameTime              = std::min( frameTimeMs, mBestFrameTime );
            mWorstFrameTime             = std::max( frameTimeMs, mWorstFrameTime );

            mFramesSampled = std::min<size_t>( (mFramesSampled + 1), OGRE_FRAME_STATS_SAMPLES );
            mNextFrame = (mNextFrame + 1) % OGRE_FRAME_STATS_SAMPLES;

            mLastTime = timeMs;
        }

        void reset( unsigned long timeMs )
        {
            mNextFrame   = 0;
            mBestFrameTime  = std::numeric_limits<unsigned long>::max();
            mWorstFrameTime = 0;
            mLastTime       = timeMs;
            memset( mFrameTimes, 0, sizeof( unsigned long ) * OGRE_FRAME_STATS_SAMPLES );
            mFramesSampled  = 0;
        }
    };

} // Namespace Ogre
#endif
