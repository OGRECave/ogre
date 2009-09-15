/*
-----------------------------------------------------------------------------
This source file is part of OGRE
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#ifndef __GLXWindow_H__
#define __GLXWindow_H__

#include "OgreRenderWindow.h"
#include "OgreGLXContext.h"
#include "OgreGLXGLSupport.h"
#include <X11/Xlib.h>

namespace Ogre 
{
	class _OgrePrivate GLXWindow : public RenderWindow
	{
	public:
		GLXWindow(GLXGLSupport* glsupport);
		~GLXWindow();
		
		void create(const String& name, unsigned int width, unsigned int height,
					bool fullScreen, const NameValuePairList *miscParams);
		
		/** @copydoc see RenderWindow::setFullscreen */
		void setFullscreen (bool fullscreen, uint width, uint height);
		
		/** @copydoc see RenderWindow::destroy */
		void destroy(void);
		
		/** @copydoc see RenderWindow::isClosed */
		bool isClosed(void) const;
		
		/** @copydoc see RenderWindow::isVisible */
		bool isVisible(void) const;
		
		/** @copydoc see RenderWindow::setVisible */
		void setVisible(bool visible);
		
		/** @copydoc see RenderWindow::reposition */
		void reposition(int left, int top);
		
		/** @copydoc see RenderWindow::resize */
		void resize(unsigned int width, unsigned int height);

		/** @copydoc see RenderWindow::windowMovedOrResized */
		void windowMovedOrResized();
		
		/** @copydoc see RenderWindow::swapBuffers */
		void swapBuffers(bool waitForVSync);
		
		/** @copydoc see RenderTarget::copyContentsToMemory */
		void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		
		/**
		   @remarks
		   * Get custom attribute; the following attributes are valid:
		   * WINDOW		 The X Window target for rendering.
		   * GLCONTEXT	  The Ogre GLContext used for rendering.
		   * DISPLAY		The X Display connection behind that context.
		   * DISPLAYNAME	The X Server name for the connected display.
		   * ATOM		   The X Atom used in client delete events.
		   */
		void getCustomAttribute(const String& name, void* pData);
		
		bool requiresTextureFlipping() const { return false; }

	private:
		bool mClosed;
		bool mVisible;
		bool mIsTopLevel;
		bool mIsExternal;
		bool mIsExternalGLControl;
		
		GLXGLSupport* mGLSupport;
		::Window	  mWindow;
		GLXContext*   mContext;
		void switchFullScreen(bool fullscreen);
	};
}

#endif
