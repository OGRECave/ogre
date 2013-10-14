/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreSDLWindow.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#   include <GL/gl.h>
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#   include <GL/glu.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#   include <GL/gl.h>
#   include <GL/glu.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <OpenGL/gl.h>
#   define GL_EXT_texture_env_combine 1
#   include <OpenGL/glext.h>
#   include <OpenGL/glu.h>
#endif

namespace Ogre {

    SDLWindow::SDLWindow() :
        mScreen(NULL), mActive(false), mClosed(false)
    {
    }

    SDLWindow::~SDLWindow()
    {
        // according to http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fSetVideoMode
        // never free the surface returned from SDL_SetVideoMode
        /*if (mScreen != NULL)
            SDL_FreeSurface(mScreen);*/

    }

	void SDLWindow::create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams)
    {
		int colourDepth = 32;
		String title = name;
		if(miscParams)
		{
			// Parse miscellenous parameters
			NameValuePairList::const_iterator opt;
			// Bit depth
			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end()) //check for FSAA parameter, if not ignore it...
				colourDepth = StringConverter::parseUnsignedInt(opt->second);
			// Full screen antialiasing
			opt = miscParams->find("FSAA");
			if(opt != miscParams->end()) //check for FSAA parameter, if not ignore it...
			{
				size_t fsaa_x_samples = StringConverter::parseUnsignedInt(opt->second);
				if(fsaa_x_samples>1) {
					// If FSAA is enabled in the parameters, enable the MULTISAMPLEBUFFERS
					// and set the number of samples before the render window is created.
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,1);
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,fsaa_x_samples);
				}
			}
			// Window title
			opt = miscParams->find("title");
			if(opt != miscParams->end()) //check for FSAA parameter, if not ignore it...
				title = opt->second;
		}   
	
        LogManager::getSingleton().logMessage("SDLWindow::create", LML_TRIVIAL);
        SDL_Surface* screen;
        int flags = SDL_OPENGL | SDL_HWPALETTE | SDL_RESIZABLE;
		
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        // request good stencil size if 32-bit colour
        if (colourDepth == 32)
        {
            SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);
        }
		
        if (fullScreen)
            flags |= SDL_FULLSCREEN;

        LogManager::getSingleton().logMessage("Create window", LML_TRIVIAL);
        screen = SDL_SetVideoMode(width, height, colourDepth, flags);
        if (!screen)
        {
            LogManager::getSingleton().logMessage(LML_CRITICAL, 
                String("Could not make screen: ") + SDL_GetError());
            exit(1);
        }
        LogManager::getSingleton().logMessage("screen is valid", LML_TRIVIAL);
        mScreen = screen;

        mName = name;

        mWidth = width;
        mHeight = height;

        mActive = true;

        if (!fullScreen)
            SDL_WM_SetCaption(title.c_str(), 0);

        glXGetVideoSyncSGI = (int (*)(unsigned int *))SDL_GL_GetProcAddress("glXGetVideoSyncSGI");
        glXWaitVideoSyncSGI = (int (*)(int, int, unsigned int *))SDL_GL_GetProcAddress("glXWaitVideoSyncSGI");
    }

    void SDLWindow::destroy(void)
    {
        // according to http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fSetVideoMode
        // never free the surface returned from SDL_SetVideoMode
        //SDL_FreeSurface(mScreen);
        mScreen = NULL;
        mActive = false;

        Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
    }

    bool SDLWindow::isActive() const
    {
        return mActive;
    }

    bool SDLWindow::isClosed() const
    {
        return mClosed;
    }

    void SDLWindow::reposition(int left, int top)
    {
        // XXX FIXME
    }

    void SDLWindow::resize(unsigned int width, unsigned int height)
    {
        SDL_Surface* screen;
        int flags = SDL_OPENGL | SDL_HWPALETTE | SDL_RESIZABLE;
		
        LogManager::getSingleton().logMessage("Updating window", LML_TRIVIAL);
        screen = SDL_SetVideoMode(width, height, mScreen->format->BitsPerPixel, flags);
        if (!screen)
        {
            LogManager::getSingleton().logMessage(LML_CRITICAL, 
                String("Could not make screen: ") + SDL_GetError());
            exit(1);
        }
        LogManager::getSingleton().logMessage("screen is valid", LML_TRIVIAL);
        mScreen = screen;


        mWidth = width;
        mHeight = height;

        for (ViewportList::iterator it = mViewportList.begin();
             it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
    }

    void SDLWindow::setVSyncEnabled(bool vsync)
	{
        mVSync = vsync;
	}

	bool SDLWindow::isVSyncEnabled() const
	{
        return mVSync;
	}

    void SDLWindow::swapBuffers()
    {
        if ( mVSync && glXGetVideoSyncSGI && glXWaitVideoSyncSGI )
        {
            unsigned int retraceCount;
            glXGetVideoSyncSGI( &retraceCount );
            glXWaitVideoSyncSGI( 2, ( retraceCount + 1 ) & 1, &retraceCount);
        }

        SDL_GL_SwapBuffers();
        // XXX More?
    }

	void SDLWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		if ((dst.left < 0) || (dst.right > mWidth) ||
			(dst.top < 0) || (dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Invalid box.",
						"SDLWindow::copyContentsToMemory" );
		}
	
		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen? FB_FRONT : FB_BACK;
		}
	
		GLenum format = Ogre::GL3PlusPixelUtil::getGLOriginFormat(dst.format);
		GLenum type = Ogre::GL3PlusPixelUtil::getGLOriginDataType(dst.format);
	
		if ((format == GL_NONE) || (type == 0))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Unsupported format.",
						"SDLWindow::copyContentsToMemory" );
		}
	
        if(dst.getWidth() != dst.rowPitch)
        {
            glPixelStorei(GL_PACK_ROW_LENGTH, dst.rowPitch);
        }
        if((dst.getWidth()*Ogre::PixelUtil::getNumElemBytes(dst.format)) & 3)
        {
            // Standard alignment of 4 is not right
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
        }

		glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
        glReadPixels((GLint)0, (GLint)(mHeight - dst.getHeight()),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());
	
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        
        PixelUtil::bulkPixelVerticalFlip(dst);
	}
}
