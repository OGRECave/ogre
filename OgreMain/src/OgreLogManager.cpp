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
#include "OgreStableHeaders.h"

#include "OgreLogManager.h"
#include "OgreException.h"
#include <algorithm>
namespace Ogre {

    //-----------------------------------------------------------------------
    template<> LogManager* Singleton<LogManager>::ms_Singleton = 0;
    LogManager* LogManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    LogManager& LogManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
    LogManager::LogManager()
    {
        mDefaultLog = NULL;
    }
    //-----------------------------------------------------------------------
    LogManager::~LogManager()
    {
		OGRE_LOCK_AUTO_MUTEX
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
		OGRE_LOCK_AUTO_MUTEX

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
		OGRE_LOCK_AUTO_MUTEX
        return mDefaultLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::setDefaultLog(Log* newLog)
    {
		OGRE_LOCK_AUTO_MUTEX
        Log* oldLog = mDefaultLog;
        mDefaultLog = newLog;
        return oldLog;
    }
    //-----------------------------------------------------------------------
    Log* LogManager::getLog( const String& name)
    {
		OGRE_LOCK_AUTO_MUTEX
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
		destroyLog(log->getName());
	}
    //-----------------------------------------------------------------------
    void LogManager::logMessage( const String& message, LogMessageLevel lml, bool maskDebug)
    {
		OGRE_LOCK_AUTO_MUTEX
		if (mDefaultLog)
		{
			mDefaultLog->logMessage(message, lml, maskDebug);
		}
    }
    //-----------------------------------------------------------------------
    void LogManager::setLogDetail(LoggingLevel ll)
    {
		OGRE_LOCK_AUTO_MUTEX
		if (mDefaultLog)
		{
	        mDefaultLog->setLogDetail(ll);
		}
    }
	//---------------------------------------------------------------------
	Log::Stream LogManager::stream(LogMessageLevel lml, bool maskDebug)
	{
		OGRE_LOCK_AUTO_MUTEX
		if (mDefaultLog)
			return mDefaultLog->stream(lml, maskDebug);
		else
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Default log not found. ", "LogManager::stream");

	}
}
