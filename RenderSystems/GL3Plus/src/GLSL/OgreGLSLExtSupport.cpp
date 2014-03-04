/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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

#include "OgreGLSLExtSupport.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {

    String logObjectInfo(const String& msg, const GLuint obj)
    {
        String logMessage = msg;

        // Invalid object.
        if (obj <= 0)
        {
            return logMessage;
        }

        GLint infologLength = 0;

        GLboolean isShader = glIsShader(obj);
        GLboolean isProgramPipeline = glIsProgramPipeline(obj);
        GLboolean isProgram = glIsProgram(obj);

        if (isShader)
        {
            OGRE_CHECK_GL_ERROR(glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength));
        }
        else if (isProgramPipeline &&
                 Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            //FIXME Crashes on NVIDIA? See GL3+ GSoC forum
            // posts around 2013-11-25.
            OGRE_CHECK_GL_ERROR(glGetProgramPipelineiv(obj, GL_INFO_LOG_LENGTH, &infologLength));
        }
        else if (isProgram)
        {
            OGRE_CHECK_GL_ERROR(glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength));
        }

        // No info log available.
        // if (infologLength <= 1)
        if (infologLength < 1)
        {
            return logMessage;
        }

        GLint charsWritten  = 0;

        char * infoLog = new char [infologLength];
        infoLog[0] = 0;

        if (isShader)
        {
            OGRE_CHECK_GL_ERROR(glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog));
        }
        else if (isProgramPipeline &&
                 Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            OGRE_CHECK_GL_ERROR(glGetProgramPipelineInfoLog(obj, infologLength, &charsWritten, infoLog));
        }
        else if (isProgram)
        {
            OGRE_CHECK_GL_ERROR(glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog));
        }

        if (strlen(infoLog) > 0)
        {
            logMessage += "\n" + String(infoLog);
        }

        delete [] infoLog;

        if (logMessage.size() > 0)
        {
            // Remove empty lines from the end of the log.
            while (logMessage[logMessage.size() - 1] == '\n')
            {
                logMessage.erase(logMessage.size() - 1, 1);
            }
            LogManager::getSingleton().logMessage(logMessage);
        }

        return logMessage;
    }


}
