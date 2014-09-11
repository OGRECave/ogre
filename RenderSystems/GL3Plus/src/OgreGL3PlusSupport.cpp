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
        // Set version string
        const GLubyte* pcVer = glGetString(GL_VERSION);

        assert(pcVer && "Problems getting GL version string using glGetString");

        String tmpStr = (const char*)pcVer;
        LogManager::getSingleton().logMessage("GL_VERSION = " + tmpStr);
        mVersion = tmpStr.substr(0, tmpStr.find(" "));

        // Get vendor
        const GLubyte* pcVendor = glGetString(GL_VENDOR);
        tmpStr = (const char*)pcVendor;
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

    bool GL3PlusSupport::checkMinGLVersion(const String& v) const
    {
        unsigned int first, second, third;
        unsigned int cardFirst, cardSecond, cardThird;
        if(v == mVersion)
            return true;

        String::size_type pos = v.find(".");
        if(pos == String::npos)
            return false;

        String::size_type pos1 = v.rfind(".");
        if(pos1 == String::npos)
            return false;

        first = ::atoi(v.substr(0, pos).c_str());
        second = ::atoi(v.substr(pos + 1, pos1 - (pos + 1)).c_str());
        third = ::atoi(v.substr(pos1 + 1, v.length()).c_str());

        pos = mVersion.find(".");
        if(pos == String::npos)
            return false;

        pos1 = mVersion.rfind(".");
        if(pos1 == String::npos)
            return false;

        cardFirst  = ::atoi(mVersion.substr(0, pos).c_str());
        cardSecond = ::atoi(mVersion.substr(pos + 1, pos1 - (pos + 1)).c_str());
        cardThird  = ::atoi(mVersion.substr(pos1 + 1, mVersion.length()).c_str());

        if(first <= cardFirst && second <= cardSecond && third <= cardThird)
          return true;

        return false;
    }

    bool GL3PlusSupport::checkExtension(const String& ext) const
    {
        if(extensionList.find(ext) == extensionList.end())
            return false;

        return true;
    }

}
