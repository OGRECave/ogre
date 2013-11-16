/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreLogManager.h"
#include "OgreException.h"
#include <algorithm>
namespace Ogre {

    //-----------------------------------------------------------------------
    template<> LogManager* Singleton<LogManager>::msSingleton = 0;
    LogManager* LogManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    LogManager& LogManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    LogManager::LogManager()
    {
        mDefaultLog = NULL;
    }
    //-----------------------------------------------------------------------
    LogManager::~LogManager()
    {
        OGRE_LOCK_AUTO_MUTEX;
        // Destroy all logs
        LogList::iterator i;
        for (i = mLogs.begin(); i != mLogs.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
    }
    //-----------------------------------------------------------------------
    Log* LogManager::createLog( const String& name, bool defaultLog, bool debuggerOutput, 
		bool suppressFileOutput)
    {
        OGRE_LOCK_AUTO_MUTEX;

        Log* newLog = OGRE_NEW Log(name, debuggerOutput, suppressFileOutput);

        if( !mDefaultLog || defaultLog )
        {
            mDefaultLog = newLog;
        }

        mLogs.insert( LogList::value_type( name, newLog ) );

        return newLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::getDefaultLog()
    {
        OGRE_LOCK_AUTO_MUTEX;
        return mDefaultLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::setDefaultLog(Log* newLog)
    {
        OGRE_LOCK_AUTO_MUTEX;
        Log* oldLog = mDefaultLog;
        mDefaultLog = newLog;
        return oldLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::getLog( const String& name)
    {
        OGRE_LOCK_AUTO_MUTEX;
        LogList::iterator i = mLogs.find(name);
        if (i != mLogs.end())
            return i->second;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Log not found. ", "LogManager::getLog");


    }
	//-----------------------------------------------------------------------
	void LogManager::destroyLog(const String& name)
	{
		LogList::iterator i = mLogs.find(name);
		if (i != mLogs.end())
		{
			if (mDefaultLog == i->second)
			{
				mDefaultLog = 0;
			}
			OGRE_DELETE i->second;
			mLogs.erase(i);
		}

		// Set another default log if this one removed
		if (!mDefaultLog && !mLogs.empty())
		{
			mDefaultLog = mLogs.begin()->second;
		}
	}
	//-----------------------------------------------------------------------
	void LogManager::destroyLog(Log* log)
	{
        if(!log)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot destroy a null log.", "LogManager::destroyLog");

        destroyLog(log->getName());
	}
    //-----------------------------------------------------------------------
    void LogManager::logMessage( const String& message, LogMessageLevel lml, bool maskDebug)
    {
        OGRE_LOCK_AUTO_MUTEX;
		if (mDefaultLog)
		{
			mDefaultLog->logMessage(message, lml, maskDebug);
		}
    }
    //-----------------------------------------------------------------------
    void LogManager::setLogDetail(LoggingLevel ll)
    {
        OGRE_LOCK_AUTO_MUTEX;
		if (mDefaultLog)
		{
	        mDefaultLog->setLogDetail(ll);
		}
    }
	//---------------------------------------------------------------------
	Log::Stream LogManager::stream(LogMessageLevel lml, bool maskDebug)
	{
            OGRE_LOCK_AUTO_MUTEX;
		if (mDefaultLog)
			return mDefaultLog->stream(lml, maskDebug);
		else
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Default log not found. ", "LogManager::stream");

	}
}
