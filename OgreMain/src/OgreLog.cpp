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
#include "OgreStableHeaders.h"

#include <iostream>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   include <windows.h>
#   if _WIN32_WINNT >= _WIN32_WINNT_VISTA
#       include <werapi.h>
#   endif
#endif

// LogMessageLevel + LoggingLevel > OGRE_LOG_THRESHOLD = message logged
#define OGRE_LOG_THRESHOLD 4

namespace {
    const char* RED = "\x1b[31;1m";
    const char* YELLOW = "\x1b[33;1m";
    const char* RESET = "\x1b[0m";
}

namespace Ogre
{
    //-----------------------------------------------------------------------
    Log::Log( const String& name, bool debuggerOutput, bool suppressFile ) : 
        mLogLevel(LML_NORMAL), mDebugOut(debuggerOutput),
        mSuppressFile(suppressFile), mTimeStamp(true), mLogName(name), mTermHasColours(false)
    {
        if (!mSuppressFile)
        {
            mLog.open(name.c_str());

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && _WIN32_WINNT >= _WIN32_WINNT_VISTA
            // Register log file to be collected by Windows Error Reporting
            const int utf16Length = ::MultiByteToWideChar(CP_ACP, 0, name.c_str(), (int)name.size(), NULL, 0);
            if(utf16Length > 0)
            {
                std::wstring wname;
                wname.resize(utf16Length);
                if (0 != ::MultiByteToWideChar(CP_ACP, 0, name.c_str(), (int)name.size(), &wname[0], (int)wname.size()))
                    WerRegisterFile(wname.c_str(), WerRegFileTypeOther, WER_FILE_ANONYMOUS_DATA);
            }
#endif
        }

#if OGRE_PLATFORM != OGRE_PLATFORM_WINRT
        char* val = getenv("OGRE_MIN_LOGLEVEL");
        int min_lml;
        if(val && StringConverter::parse(val, min_lml))
            setMinLogLevel(LogMessageLevel(min_lml));

        if(mDebugOut)
        {
            val = getenv("TERM");
            mTermHasColours = val && String(val).find("xterm") != String::npos;
        }
#endif
    }
    //-----------------------------------------------------------------------
    Log::~Log()
    {
        OGRE_LOCK_AUTO_MUTEX;
        if (!mSuppressFile)
        {
            mLog.close();
        }
    }

    //-----------------------------------------------------------------------
    void Log::logMessage( const String& message, LogMessageLevel lml, bool maskDebug )
    {
        OGRE_LOCK_AUTO_MUTEX;
        if (lml >= mLogLevel)
        {
            bool skipThisMessage = false;
            for( mtLogListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
                (*i)->messageLogged( message, lml, maskDebug, mLogName, skipThisMessage);
            
            if (!skipThisMessage)
            {
                if (mDebugOut && !maskDebug)
                {
#    if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && OGRE_DEBUG_MODE
                    OutputDebugStringA("Ogre: ");
                    OutputDebugStringA(message.c_str());
                    OutputDebugStringA("\n");
#    endif

                    std::ostream& os = int(lml) >= int(LML_WARNING) ? std::cerr : std::cout;

                    if(mTermHasColours) {
                        if(lml == LML_WARNING)
                            os << YELLOW;
                        if(lml == LML_CRITICAL)
                            os << RED;
                    }

                    os << message;

                    if(mTermHasColours) {
                        os << RESET;
                    }

                    os << std::endl;
                }

                // Write time into log
                if (!mSuppressFile)
                {
                    if (mTimeStamp)
                    {
                        struct tm *pTime;
                        time_t ctTime; time(&ctTime);
                        pTime = localtime( &ctTime );
                        mLog << std::setw(2) << std::setfill('0') << pTime->tm_hour
                            << ":" << std::setw(2) << std::setfill('0') << pTime->tm_min
                            << ":" << std::setw(2) << std::setfill('0') << pTime->tm_sec
                            << ": ";
                    }
                    mLog << message << std::endl;

                    // Flush stcmdream to ensure it is written (incase of a crash, we need log to be up to date)
                    mLog.flush();
                }
            }
        }
    }
    
    //-----------------------------------------------------------------------
    void Log::setTimeStampEnabled(bool timeStamp)
    {
        OGRE_LOCK_AUTO_MUTEX;
        mTimeStamp = timeStamp;
    }

    //-----------------------------------------------------------------------
    void Log::setDebugOutputEnabled(bool debugOutput)
    {
        OGRE_LOCK_AUTO_MUTEX;
        mDebugOut = debugOutput;
    }

    //-----------------------------------------------------------------------
    void Log::setLogDetail(LoggingLevel ll)
    {
        OGRE_LOCK_AUTO_MUTEX;
        mLogLevel = LogMessageLevel(OGRE_LOG_THRESHOLD - ll);
    }

    void Log::setMinLogLevel(LogMessageLevel lml)
    {
        OGRE_LOCK_AUTO_MUTEX;
        mLogLevel = lml;
    }

    //-----------------------------------------------------------------------
    void Log::addListener(LogListener* listener)
    {
        OGRE_LOCK_AUTO_MUTEX;
        if (std::find(mListeners.begin(), mListeners.end(), listener) == mListeners.end())
            mListeners.push_back(listener);
    }

    //-----------------------------------------------------------------------
    void Log::removeListener(LogListener* listener)
    {
        OGRE_LOCK_AUTO_MUTEX;
        mtLogListener::iterator i = std::find(mListeners.begin(), mListeners.end(), listener);
        if (i != mListeners.end())
            mListeners.erase(i);
    }
    //---------------------------------------------------------------------
    Log::Stream Log::stream(LogMessageLevel lml, bool maskDebug) 
    {
        return Stream(this, lml, maskDebug);

    }
}
