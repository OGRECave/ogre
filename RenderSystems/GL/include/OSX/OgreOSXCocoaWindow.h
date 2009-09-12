/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
-----------------------------------------------------------------------------
*/

#ifndef __OSXCocoaWindow_H__
#define __OSXCocoaWindow_H__

#include "OgreOSXWindow.h"
#include "OgreOSXCocoaContext.h"

#include <Cocoa/Cocoa.h>
#include "OgreOSXCocoaView.h"

namespace Ogre {
    class OSXCocoaWindow : public OSXWindow
    {
    private:
		NSWindow *mWindow;
		NSView *mView;
		NSOpenGLContext *glContext;
		
        bool mActive;
        bool mClosed;
		bool mHasResized;
            
        // Process pending events
        void processEvents(void);
    public:
        OSXCocoaWindow();
        ~OSXCocoaWindow();
		
		NSView* ogreView() const { return mView; };
		NSWindow* ogreWindow() const { return mWindow; };
		NSOpenGLContext* nsopenGLContext() const { return glContext; };
		void createWithView(OgreView *view);

		void create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams);
        /** Overridden - see RenderWindow */
        void destroy(void);
        /** Overridden - see RenderWindow */
        bool isActive(void) const;
        /** Overridden - see RenderWindow */
        bool isClosed(void) const;
        /** Overridden - see RenderWindow */
        void reposition(int left, int top);
        /** Overridden - see RenderWindow */
        void resize(unsigned int width, unsigned int height);
        /** Overridden - see RenderWindow */
        void swapBuffers(bool waitForVSync);
        /** Overridden - see RenderWindow */
        virtual void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
        /** Overridden - see RenderWindow */
		void windowMovedOrResized();

		bool requiresTextureFlipping() const { return false; }		
		void getCustomAttribute( const String& name, void* pData );
    };
}

#endif

