/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreLogManager.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLSupport.h"

namespace Ogre
{

    //-----------------------------------------------------------------------------
    void checkForGLSLError(const String& ogreMethod, const String& errorTextPrefix, const GLhandleARB obj, const bool forceInfoLog, const bool forceException)
    {
		GLenum glErr;
		bool errorsFound = false;
		String msg = errorTextPrefix;

		// get all the GL errors
		glErr = glGetError();
		while (glErr != GL_NO_ERROR)
        {
			const char* glerrStr = (const char*)gluErrorString(glErr);
			if (glerrStr)
			{
				msg += String(glerrStr);
			}
			glErr = glGetError();
			errorsFound = true;
        }


		// if errors were found then put them in the Log and raise and exception
		if (errorsFound || forceInfoLog)
		{
			// if shader or program object then get the log message and send to the log manager
			msg += logObjectInfo( msg, obj );

            if (forceException) 
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, msg, ogreMethod);
			}
		}
    }

    //-----------------------------------------------------------------------------
	String logObjectInfo(const String& msg, const GLhandleARB obj)
	{
		String logMessage = msg;

		if (obj > 0)
		{
			GLint infologLength = 0;

			glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);

			if (infologLength > 0)
			{
				GLint charsWritten  = 0;

				GLcharARB * infoLog = new GLcharARB[infologLength];

				glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
				logMessage += String(infoLog);
				LogManager::getSingleton().logMessage(logMessage);

				delete [] infoLog;
			}
		}

		return logMessage;

	}


} // namespace Ogre
