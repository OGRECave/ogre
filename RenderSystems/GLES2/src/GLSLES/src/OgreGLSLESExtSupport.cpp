/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreGLSLESExtSupport.h"
#include "OgreLogManager.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------
	String logObjectInfo(const String& msg, const GLuint obj)
	{
		String logMessage = msg;

		if (obj > 0)
		{
			GLint infologLength = 0;

            if(glIsShader(obj))
            {
                glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
                GL_CHECK_ERROR
            }
            else if(glIsProgram(obj))
            {
                glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
                GL_CHECK_ERROR
            }

			if (infologLength > 0)
			{
				GLint charsWritten  = 0;

				//GLchar * infoLog = OGRE_NEW GLchar[infologLength];
				char * infoLog = new char [infologLength];

                if(glIsShader(obj))
                {
                    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
                    GL_CHECK_ERROR
                }
                else if(glIsProgram(obj))
                {
                    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
                    GL_CHECK_ERROR
                }
				logMessage += "\n" + String(infoLog);

				OGRE_DELETE [] infoLog;
			}
		}
        LogManager::getSingleton().logMessage(logMessage);

		return logMessage;
	}


} // namespace Ogre
