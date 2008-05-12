#ifndef _LOGTOTEXTREDIRECTOR_H_
#define _LOGTOTEXTREDIRECTOR_H_

#include "OgreLog.h"
#include "OgreString.h"

class wxTextCtrl;

using Ogre::Log;
using Ogre::LogListener;
using Ogre::LogManager;
using Ogre::LogMessageLevel;
using Ogre::String;

class LogToTextRedirector : public LogListener
{
public:
	LogToTextRedirector(wxTextCtrl* textCtrl, const String& logName);
	~LogToTextRedirector();

	void messageLogged(const String& message, LogMessageLevel level, bool maskDebug, const String &logName);

protected:
	wxTextCtrl* mTextControl;
	String mLogName;
};

#endif // _LOGTOTEXTREDIRECTOR_H_