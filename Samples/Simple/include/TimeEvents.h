//
//  TimeEvents.h
//  OGRE
//
//  Created by Chilly Willy on 11/29/25.
//

#ifndef INCLUDE_OGRE_TIME_EVENTS_H
#define INCLUDE_OGRE_TIME_EVENTS_H


#include <map>
#include <vector>
#include <string>


typedef std::multimap<float, std::string> TimeEventList;


enum TimeEventDirection
{
	kTEDForward,
	kTEDBackward,
};


class TimeEventListener
{
public:
	virtual void eventOccurred(const std::string & name, TimeEventDirection direction) {}
};


class TimeEventDispatcher
{
public:
	void addEventList(const TimeEventList * list);
	void removeEventList(const TimeEventList * list);

	void addListener(TimeEventListener * listener);
	void removeListener(TimeEventListener * listener);

	void dispatch(float lastTime, float thisTime, int loops, float length);
	
private:
	void dispatchForwardInclusive(float lastTime, float thisTime);
	void dispatchForwardExclusive(float lastTime, float thisTime);
	void dispatchBackwardInclusive(float lastTime, float thisTime);
	void dispatchBackwardExclusive(float lastTime, float thisTime);

	void dispatchEvent(const std::string & name, TimeEventDirection direction);

	std::vector<const TimeEventList *> mEventLists;
	std::vector<TimeEventListener *> mListeners;
};


#endif /* INCLUDE_OGRE_TIME_EVENTS_H */
