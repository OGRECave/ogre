/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

#include "OgreGLES2Support.h"

#include "OgreLogManager.h"

namespace Ogre {
    void GLES2Support::setConfigOption(const String &name, const String &value)
    {
        ConfigOptionMap::iterator it = mOptions.find(name);

        if (it == mOptions.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Option named " + name +  " does not exist.",
                        "GLES2Support::setConfigOption");
        }
        else
        {
            it->second.currentValue = value;
        }
    }

    ConfigOptionMap& GLES2Support::getConfigOptions(void)
    {
        return mOptions;
    }

    void GLES2Support::initialiseExtensions(void)
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
            LogManager::getSingleton().logMessage("EXT:" + str);
            extensionList.insert(str);
        }

    		// Get function pointers on platforms that don't have prototypes
#ifndef GL_GLEXT_PROTOTYPES

		// define the GL types if they are not defined
#	ifndef PFNGLMAPBUFFEROES
        typedef void* (GL_APIENTRY *PFNGLMAPBUFFEROES)(GLenum target, GLenum access);
        typedef GLboolean (GL_APIENTRY *PFNGLUNMAPBUFFEROES)(GLenum target);
#	endif

#   ifndef PFNGLDRAWBUFFERSARB
        typedef void (GL_APIENTRY *PFNGLDRAWBUFFERSARB) (GLsizei n, const GLenum *bufs);
#   endif
        
#   ifndef PFNGLREADBUFFERNV
        typedef void (GL_APIENTRY *PFNGLREADBUFFERNV) (GLenum mode);
#   endif
        
#   ifndef PFNGLGETTEXIMAGENV
        typedef void (GL_APIENTRY *PFNGLGETTEXIMAGENV) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid* img);
        typedef void (GL_APIENTRY *PFNGLGETCOMPRESSEDTEXIMAGENV) (GLenum target, GLint level, GLvoid* img);
        typedef void (GL_APIENTRY *PFNGLGETTEXLEVELPARAMETERFVNV) (GLenum target, GLint level, GLenum pname, GLfloat* params);
        typedef void (GL_APIENTRY *PFNGLGETTEXLEVELPARAMETERiVNV) (GLenum target, GLint level, GLenum pname, GLint* params);
#   endif
        glMapBufferOES = (PFNGLMAPBUFFEROES)getProcAddress("glMapBufferOES");
        glUnmapBufferOES = (PFNGLUNMAPBUFFEROES)getProcAddress("glUnmapBufferOES");
        glDrawBuffersARB = (PFNGLDRAWBUFFERSARB)getProcAddress("glDrawBuffersARB");
        glReadBufferNV = (PFNGLREADBUFFERNV)getProcAddress("glReadBufferNV");
        glGetTexImageNV = (PFNGLGETTEXIMAGENV)getProcAddress("glGetTexImageNV");
        glGetCompressedTexImageNV = (PFNGLGETCOMPRESSEDTEXIMAGENV)getProcAddress("glGetCompressedTexImageNV");
        glGetTexLevelParameterfvNV = (PFNGLGETTEXLEVELPARAMETERFVNV)getProcAddress("glGetTexLevelParameterfvNV");
        glGetTexLevelParameterivNV = (PFNGLGETTEXLEVELPARAMETERiVNV)getProcAddress("glGetTexLevelParameterivNV");
#endif
}

    bool GLES2Support::checkExtension(const String& ext) const
    {
        if(extensionList.find(ext) == extensionList.end())
            return false;

        return true;
    }
}
