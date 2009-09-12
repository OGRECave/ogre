/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESSupport.h"

#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre {
    void GLESSupport::setConfigOption(const String &name, const String &value)
    {
        ConfigOptionMap::iterator it = mOptions.find(name);

        if (it == mOptions.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Option named " + name +  " does not exist.",
                        "GLESSupport::setConfigOption");
        }
        else
        {
            it->second.currentValue = value;
        }
    }

    ConfigOptionMap& GLESSupport::getConfigOptions(void)
    {
        return mOptions;
    }

    void GLESSupport::initialiseExtensions(void)
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
        std::stringstream ext;
        String str;

        const GLubyte* pcExt = glGetString(GL_EXTENSIONS);
        LogManager::getSingleton().logMessage("GL_EXTENSIONS = " + String((const char*)pcExt));

        assert(pcExt && "Problems getting GL extension string using glGetString");

        ext << pcExt;

        while (ext >> str)
        {
            printf("EXT: %s\n", str.c_str());
            extensionList.insert(str);
        }
        
        // Get function pointers on non-iPhone platforms
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
        glIsRenderbufferOES = (PFNGLISRENDERBUFFEROES)getProcAddress("glIsRenderbufferOES");
        glBindRenderbufferOES = (PFNGLBINDRENDERBUFFEROES)getProcAddress("glBindRenderbufferOES");
        glDeleteRenderbuffersOES = (PFNGLDELETERENDERBUFFERSOES)getProcAddress("glDeleteRenderbuffersOES");
        glGenRenderbuffersOES = (PFNGLGENRENDERBUFFERSOES)getProcAddress("glGenRenderbuffersOES");
        glRenderbufferStorageOES = (PFNGLRENDERBUFFERSTORAGEOES)getProcAddress("glRenderbufferStorageOES");
        glGetRenderbufferParameterivOES = (PFNGLGETRENDERBUFFERPARAMETERIVOES)getProcAddress("glGetRenderbufferParameterivOES");
        glIsFramebufferOES = (PFNGLISFRAMEBUFFEROES)getProcAddress("glIsFramebufferOES");
        glBindFramebufferOES = (PFNGLBINDFRAMEBUFFEROES)getProcAddress("glBindFramebufferOES");
        glDeleteFramebuffersOES = (PFNGLDELETEFRAMEBUFFERSOES)getProcAddress("glDeleteFramebuffersOES");
        glGenFramebuffersOES = (PFNGLGENFRAMEBUFFERSOES)getProcAddress("glGenFramebuffersOES");
        glCheckFramebufferStatusOES = (PFNGLCHECKFRAMEBUFFERSTATUSOES)getProcAddress("glCheckFramebufferStatusOES");
        glFramebufferRenderbufferOES = (PFNGLFRAMEBUFFERRENDERBUFFEROES)getProcAddress("glFramebufferRenderbufferOES");
        glFramebufferTexture2DOES = (PFNGLFRAMEBUFFERTEXTURE2DOES)getProcAddress("glFramebufferTexture2DOES");
        glGetFramebufferAttachmentParameterivOES = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES)getProcAddress("glGetFramebufferAttachmentParameterivOES");
        glGenerateMipmapOES = (PFNGLGENERATEMIPMAPOES)getProcAddress("glGenerateMipmapOES");
#endif
    }

    bool GLESSupport::checkExtension(const String& ext) const
    {
        if(extensionList.find(ext) == extensionList.end())
            return false;

        return true;
    }
}
