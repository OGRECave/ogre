//
//  TimeEvents.cpp
//  OGRE
//
//  Created by Chilly Willy on 11/29/25.
//

#include "TimeEvents.h"

#include <algorithm>


void TimeEventDispatcher::addEventList(const TimeEventList * list)
{
    if (std::find(mEventLists.begin(), mEventLists.end(), list) == mEventLists.end())
    {
        mEventLists.push_back(list);
    }
}

void TimeEventDispatcher::removeEventList(const TimeEventList * list)
{
    auto i = std::find(mEventLists.begin(), mEventLists.end(), list);
    if (i != mEventLists.end())
    {
        mEventLists.erase(i);
    }
}

void TimeEventDispatcher::addListener(TimeEventListener * listener)
{
    if (std::find(mListeners.begin(), mListeners.end(), listener) == mListeners.end())
    {
        mListeners.push_back(listener);
    }
}

void TimeEventDispatcher::removeListener(TimeEventListener * listener)
{
    auto i = std::find(mListeners.begin(), mListeners.end(), listener);
    if (i != mListeners.end())
    {
        mListeners.erase(i);
    }
}

/*
 lastTime, thisTime, n=loops

 lastTime is the thisTime from the last frame and thisTime will be the lastTime in the next frame.
 Events at those times should only be dispatched once so we should include one and exclude the other.

 If we include thisTime and exclude lastTime, then we need to dispatch events before we start playing,
 eg when we call setTimePosition(), and we would need to somehow know whether it should be dispatched
 as Forward or Backward.

 If we include lastTime and exclude thisTime, then we don't need to do anything before playing but
 we need to include the length/0 when we loop or finish, which requires special handling anyway.

 Looping Forward:
                        next lastTime
    thisTime < length   thisTime                lastTime <= t < thisTime
    thisTime = length   0                       lastTime <= t <= length
    thisTime > length   thisTime - n * length   lastTime <= t <= length        ...        0 <= t < thisTime

    ...     0 <= t <= length

 Looping Backward:
                        next lastTime
    thisTime > 0        thisTime                lastTime >= t > thisTime
    thisTime = 0        0 !!!                   lastTime >= t >= 0
    thisTime < 0        thisTime + n * length   lastTime >= t >= 0             ...        length >= t > thisTime

    ...     length >= t >= 0

 Not-Looping Forward:
                        next lastTime
    thisTime < length   thisTime                lastTime <= t < thisTime
    thisTime = length   length                  lastTime <= t <= length
    thisTime > length   length                  lastTime <= t <= length


 Not-Looping Backward:
                        next lastTime
    thisTime > 0        thisTime                lastTime >= t > thisTime
    thisTime = 0        0                       lastTime >= t >= 0
    thisTime < 0        0                       lastTime >= t >= 0

 */

void TimeEventDispatcher::dispatch(float lastTime, float thisTime, int loops, float length)
{
    if ((loops > 0) || (thisTime > lastTime))
    {
        while (loops--)
        {
            dispatchForwardInclusive(lastTime, length);
            lastTime = 0.0f;
        }
        if (thisTime >= length)
        {
            dispatchForwardInclusive(lastTime, length);
        }
        else
        {
            dispatchForwardExclusive(lastTime, thisTime);
        }
    }
    else if ((loops < 0) || (thisTime < lastTime))
    {
        if (lastTime == 0.0f)
        {
            /*  length mod length = 0 mod length

                Either we've already been looping backward and the last frame just happened
                to stop on 0, in which case we already triggered its events, or we are just
                starting to to loop backward, in which case we should not trigger events at
                0 until we do a full backward run through the animation. If we haven't been
                looping then we would not get in here because thisTime == lastTime == 0.0f.
                For the same reason we also know loops < 0.
             */
            lastTime = length;
            ++loops;
        }
        while (loops++)
        {
            dispatchBackwardInclusive(lastTime, 0.0f);
            lastTime = length;
        }
        if (thisTime <= 0.0f)
        {
            dispatchBackwardInclusive(lastTime, 0.0f);
        }
        else
        {
            dispatchBackwardExclusive(lastTime, thisTime);
        }
    }
    else
    {
        // Nothing doing
    }
}

void TimeEventDispatcher::dispatchForwardInclusive(float lastTime, float thisTime)
{
    for (const TimeEventList * events : mEventLists)
    {
        for (auto ie = events->begin(); ie != events->end(); ++ie)
        {
            if (ie->first >= lastTime && ie->first <= thisTime)
            {
                dispatchEvent(ie->second, TED_FORWARD);
            }
        }
    }
}

void TimeEventDispatcher::dispatchForwardExclusive(float lastTime, float thisTime)
{
    for (const TimeEventList * events : mEventLists)
    {
        for (auto ie = events->begin(); ie != events->end(); ++ie)
        {
            if (ie->first >= lastTime && ie->first < thisTime)
            {
                dispatchEvent(ie->second, TED_FORWARD);
            }
        }
    }
}

void TimeEventDispatcher::dispatchBackwardInclusive(float lastTime, float thisTime)
{
    for (const TimeEventList * events : mEventLists)
    {
        for (auto ie = events->rbegin(); ie != events->rend(); ++ie)
        {
            if (ie->first <= lastTime && ie->first >= thisTime)
            {
                dispatchEvent(ie->second, TED_BACKWARD);
            }
        }
    }
}

void TimeEventDispatcher::dispatchBackwardExclusive(float lastTime, float thisTime)
{
    for (const TimeEventList * events : mEventLists)
    {
        for (auto ie = events->rbegin(); ie != events->rend(); ++ie)
        {
            if (ie->first <= lastTime && ie->first > thisTime)
            {
                dispatchEvent(ie->second, TED_BACKWARD);
            }
        }
    }
}

void TimeEventDispatcher::dispatchEvent(const std::string & name, TimeEventDirection direction)
{
    for (TimeEventListener * listener : mListeners)
    {
        listener->eventOccurred(name, direction);
    }
}
