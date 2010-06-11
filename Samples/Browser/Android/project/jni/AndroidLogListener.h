#ifndef __AndroidLogListener_H__
#define __AndroidLogListener_H__

#include "OgreLog.h"

class AndroidLogListener : public Ogre::LogListener
{
public:
	AndroidLogListener();
	
	virtual void messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName);
};

#endif