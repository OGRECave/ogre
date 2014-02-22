#include "LogToTextRedirector.h"

#include <wx/textctrl.h>
#include "OgreLogManager.h"

LogToTextRedirector::LogToTextRedirector(wxTextCtrl* textCtrl, const String& logName)
: mTextControl(textCtrl)
{
    mLogName = logName;
    Log* log = LogManager::getSingletonPtr()->getLog(mLogName);
    if(log) log->addListener(this);
}

LogToTextRedirector::~LogToTextRedirector()
{
    if(LogManager::getSingletonPtr())
    {
        Log* log = LogManager::getSingletonPtr()->getLog(mLogName);
        if(log) log->removeListener(this);
    }
}

void LogToTextRedirector::messageLogged(const String& message, LogMessageLevel level, bool maskDebug, const String &logName)
{
    if(mTextControl && !mTextControl->IsBeingDeleted())
        *mTextControl << "[" << logName << "] " << message << "\n";
}