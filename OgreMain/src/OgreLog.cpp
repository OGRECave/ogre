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

#include "OgreLog.h"
#include "OgreLogManager.h"
#include "OgreString.h"

namespace Ogre
{

    //-----------------------------------------------------------------------
    Log::Log( const String& name, bool debuggerOuput, bool suppressFile ) : 
        mLogLevel(LL_NORMAL), mDebugOut(debuggerOuput),
        mSuppressFile(suppressFile), mLogName(name), mTimeStamp(true)
    {
		if (!mSuppressFile)
		{
			mfpLog.open(name.c_str());
		}
    }
    //-----------------------------------------------------------------------
    Log::~Log()
    {
		OGRE_LOCK_AUTO_MUTEX
		if (!mSuppressFile)
		{
	        mfpLog.close();
		}
    }
    //-----------------------------------------------------------------------
    void Log::logMessage( const String& message, LogMessageLevel lml, bool maskDebug )
    {
		OGRE_LOCK_AUTO_MUTEX
        if ((mLogLevel + lml) >= OGRE_LOG_THRESHOLD)
        {
            for( mtLogListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
                (*i)->messageLogged( message, lml, maskDebug, mLogName );

			if (mDebugOut && !maskDebug)
                std::cerr << message << std::endl;

            // Write time into log
			if (!mSuppressFile)
			{
				if (mTimeStamp)
			    {
                    struct tm *pTime;
                    time_t ctTime; time(&ctTime);
                    pTime = localtime( &ctTime );
                    mfpLog << std::setw(2) << std::setfill('0') << pTime->tm_hour
                        << ":" << std::setw(2) << std::setfill('0') << pTime->tm_min
                        << ":" << std::setw(2) << std::setfill('0') << pTime->tm_sec
                        << ": ";
                }
                mfpLog << message << std::endl;

				// Flush stcmdream to ensure it is written (incase of a crash, we need log to be up to date)
				mfpLog.flush();
			}
        }
    }
    
    //-----------------------------------------------------------------------
    void Log::setTimeStampEnabled(bool timeStamp)
    {
		OGRE_LOCK_AUTO_MUTEX
        mTimeStamp = timeStamp;
    }

    //-----------------------------------------------------------------------
    void Log::setDebugOutputEnabled(bool debugOutput)
    {
		OGRE_LOCK_AUTO_MUTEX
        mDebugOut = debugOutput;
    }

	//-----------------------------------------------------------------------
    void Log::setLogDetail(LoggingLevel ll)
    {
		OGRE_LOCK_AUTO_MUTEX
        mLogLevel = ll;
    }

    //-----------------------------------------------------------------------
    void Log::addListener(LogListener* listener)
    {
		OGRE_LOCK_AUTO_MUTEX
        mListeners.push_back(listener);
    }

    //-----------------------------------------------------------------------
    void Log::removeListener(LogListener* listener)
    {
		OGRE_LOCK_AUTO_MUTEX
        mListeners.erase(std::find(mListeners.begin(), mListeners.end(), listener));
    }
	//---------------------------------------------------------------------
	Log::Stream Log::stream(LogMessageLevel lml, bool maskDebug) 
	{
		return Stream(this, lml, maskDebug);

	}
}
