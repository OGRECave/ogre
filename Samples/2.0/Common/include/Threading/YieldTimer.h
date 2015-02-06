
#ifndef _Demo_YieldTimer_H_
#define _Demo_YieldTimer_H_

#include "OgreTimer.h"

namespace Demo
{
    class YieldTimer
    {
        Ogre::Timer *mExternalTimer;

    public:
        YieldTimer( Ogre::Timer *externalTimer ) :
            mExternalTimer( externalTimer )
        {
        }

        unsigned long yield( double frameTime, unsigned long startTime )
        {
            unsigned long endTime = mExternalTimer->getMicroseconds();

            while( frameTime * 1000000.0 > (endTime - startTime) )
            {
                endTime = mExternalTimer->getMicroseconds();

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
                SwitchToThread();
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
                sched_yield();
#endif
            }

            return endTime;
        }
    };
}

#endif
