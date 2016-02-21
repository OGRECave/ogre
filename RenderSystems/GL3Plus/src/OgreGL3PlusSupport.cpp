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

#include "OgreGL3PlusSupport.h"
#include "OgreLogManager.h"

namespace Ogre {

    void GL3PlusSupport::setConfigOption(const String &name, const String &value)
    {
        ConfigOptionMap::iterator it = mOptions.find(name);

        if (it == mOptions.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Option named " + name +  " does not exist.",
                        "GL3PlusSupport::setConfigOption");
        }
        else
        {
            it->second.currentValue = value;
        }
    }

    ConfigOptionMap& GL3PlusSupport::getConfigOptions(void)
    {
        return mOptions;
    }

    void GL3PlusSupport::initialiseExtensions(void)
    {
        // get driver version.
        // this is the recommended way for GL3 see: https://www.opengl.org/wiki/Get_Context_Info
        glGetIntegerv(GL_MAJOR_VERSION, &mVersion.major);
        glGetIntegerv(GL_MINOR_VERSION, &mVersion.minor);

        LogManager::getSingleton().logMessage("GL_VERSION = " + mVersion.toString());

        // Get vendor
        const GLubyte* pcVendor = glGetString(GL_VENDOR);
        String tmpStr = (const char*)pcVendor;
        LogManager::getSingleton().logMessage("GL_VENDOR = " + tmpStr);
        mVendor = tmpStr.substr(0, tmpStr.find(" "));

        // Get renderer
        const GLubyte* pcRenderer = glGetString(GL_RENDERER);
        tmpStr = (const char*)pcRenderer;
        LogManager::getSingleton().logMessage("GL_RENDERER = " + tmpStr);

        // Set extension list
        StringStream ext;
        String str;

        GLint numExt;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

        LogManager::getSingleton().logMessage("GL_EXTENSIONS = ");
        for(int i = 0; i < numExt; i++)
        {
            const GLubyte* pcExt = glGetStringi(GL_EXTENSIONS, i);
            assert(pcExt && "Problems getting GL extension string using glGetString");
            str = String((const char*)pcExt);
            LogManager::getSingleton().logMessage(str);
            extensionList.insert(str);
        }
    }

    bool GL3PlusSupport::hasMinGLVersion(int major, int minor) const
    {
        if (mVersion.major == major) {
            return mVersion.minor >= minor;
        }
        return mVersion.major >= major;
    }

    bool GL3PlusSupport::checkExtension(const String& ext) const
    {
        return extensionList.find(ext) != extensionList.end();
    }


}
