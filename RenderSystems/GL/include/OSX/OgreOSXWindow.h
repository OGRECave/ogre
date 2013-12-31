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

#ifndef __OSXWindow_H__
#define __OSXWindow_H__

#include <Carbon/Carbon.h>
#include "OgreRenderWindow.h"
#include "OgreOSXContext.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>

namespace Ogre 
{
	class OSXWindow : public RenderWindow
	{
	public:
		OSXWindow();
		virtual ~OSXWindow();
		
		/** Overridden - see RenderWindow */
		void create( const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams ) = 0;
        /** Overridden - see RenderWindow */
        virtual void destroy( void ) = 0;
        /** Overridden - see RenderWindow */
        virtual bool isActive( void ) const = 0;
        /** Overridden - see RenderWindow */
        virtual bool isClosed( void ) const = 0;
        virtual bool isHidden() const = 0;
        virtual void setHidden(bool hidden) = 0;
        /** Overridden - see RenderWindow */
        virtual void reposition( int left, int top ) = 0;
        /** Overridden - see RenderWindow */
        virtual void resize( unsigned int width, unsigned int height ) = 0;
        /** Overridden - see RenderWindow */
        virtual void swapBuffers( ) = 0;
        /** Overridden - see RenderTarget */
        virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		/** Overridden - see RenderTarget */
		virtual void windowMovedOrResized() {};

	protected:
		OSXContext* mContext;
		CGLContextObj mCGLContextObj;
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        CGDisplayModeRef mOriginalDisplayMode;
#else
        CFDictionaryRef mOriginalDisplayMode;
#endif

		/** Switch to full screen mode using CGL */
		void createCGLFullscreen(unsigned int width, unsigned int height, unsigned int depth, unsigned int fsaa, CGLContextObj sharedContext);
		/** Kill full screen mode, and return to default windowed mode */
		void destroyCGLFullscreen(void);
		/** Update the full screen context */
		void swapCGLBuffers(void);
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        uint32 bitDepthFromDisplayMode(CGDisplayModeRef mode);
#endif
	};
    
    #define ENABLE_CG_CHECK 0
    #if ENABLE_CG_CHECK
    #define CG_CHECK_ERROR(e) \
    { \
        if((CGError)e != kCGErrorSuccess) \
        { \
            CGReleaseAllDisplays(); \
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("CG Error: " + StringConverter::toString(e) +  + \
                                        " Line # " + StringConverter::toString(__LINE__)), __PRETTY_FUNCTION__); \
        } \
    }
    #else
        #define CG_CHECK_ERROR(e) {}
    #endif
        
    #if ENABLE_CG_CHECK
    #define CGL_CHECK_ERROR(e) \
    { \
        if((CGLError)e != kCGLNoError) \
        { \
            CGReleaseAllDisplays(); \
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("CGL Error: " + String(CGLErrorString(e)) + \
                                      " Line # " + StringConverter::toString(__LINE__)), __PRETTY_FUNCTION__); \
        } \
    }
    #else
        #define CGL_CHECK_ERROR(e) {}
    #endif
}

#endif
