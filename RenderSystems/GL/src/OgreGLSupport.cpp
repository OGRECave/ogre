
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


#include "OgreGLSupport.h"
#include "OgreGLTexture.h"
#include "OgreLogManager.h"

namespace Ogre {

	void GLSupport::setConfigOption(const String &name, const String &value)
	{
		ConfigOptionMap::iterator it = mOptions.find(name);

        if (it != mOptions.end())
            it->second.currentValue = value;
	}

	ConfigOptionMap& GLSupport::getConfigOptions(void)
	{
		return mOptions;
	}

    void GLSupport::initialiseExtensions(void)
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

        const GLubyte* pcExt = glGetString(GL_EXTENSIONS);
        LogManager::getSingleton().logMessage("GL_EXTENSIONS = " + String((const char*)pcExt));

        assert(pcExt && "Problems getting GL extension string using glGetString");

        ext << pcExt;

        while(ext >> str)
        {
            extensionList.insert(str);
        }
    }

    bool GLSupport::checkMinGLVersion(const String& v) const
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

    bool GLSupport::checkExtension(const String& ext) const
    {
        if(extensionList.find(ext) == extensionList.end())
            return false; 
        
        return true;
    }
    
    bool GLSupport::supportsPBuffers()
    {
        return (GLEW_ARB_pixel_buffer_object || GLEW_EXT_pixel_buffer_object) != GL_FALSE;
    }

    GLPBuffer* GLSupport::createPBuffer(PixelComponentType format, size_t width, size_t height)
    {
        return 0;
    }

}
