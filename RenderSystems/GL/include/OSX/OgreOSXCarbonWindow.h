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

#ifndef __OSXCarbonWindow_H__
#define __OSXCarbonWindow_H__

#include "OgreOSXWindow.h"
#include "OgreOSXCarbonContext.h"
#include "OgreOSXCGLContext.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>

namespace Ogre 
{
	class OSXCarbonWindow : public OSXWindow
	{
	public:
		OSXCarbonWindow();
		virtual ~OSXCarbonWindow();
		
		virtual void create( const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams );
        /** Overridden - see RenderWindow */
		virtual void destroy( void );
        /** Overridden - see RenderWindow */
		virtual bool isActive( void ) const;
        /** Overridden - see RenderWindow */
		virtual bool isClosed( void ) const;
        /** Overridden - see RenderWindow */
		virtual void reposition( int left, int top );
        /** Overridden - see RenderWindow */
		virtual void resize( unsigned int width, unsigned int height );
        /** Overridden - see RenderWindow */
    	virtual void setVisible( bool visible );
        /** Overridden - see RenderWindow */
    	virtual bool isVisible(void) const;
        /** Overridden - see RenderWindow */
		virtual void swapBuffers( bool waitForVSync );
        /** Overridden - see RenderWindow */
        virtual void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
   	    /** Overridden - see RenderTarget */
		virtual void windowMovedOrResized();

		bool requiresTextureFlipping() const { return false; }
		
		void windowResized();
		void windowHasResized();
		
		void getCustomAttribute( const String& name, void* pData );

	private:
		void processEvents();
		
	private:
		WindowRef mWindow;
        EventHandlerRef mEventHandlerRef;
		HIViewRef mView;
		AGLContext mAGLContext;
		
    bool mActive;
    bool mClosed;
    bool mCreated;
    bool mHasResized;
    bool mIsExternal;
    bool mVisible;
	};
}

#endif
