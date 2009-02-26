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

#ifndef __GtkEGLWindow_H__
#define __GtkEGLWindow_H__

#include "OgreEGLWindow.h"

namespace Ogre {
    class _OgrePrivate GtkEGLWindow : public EGLWindow
    {
	protected:
		Window mParentWindow ;
		virtual EGLContext * createEGLContext() const;
		virtual void getLeftAndTopFromNativeWindow(int & left, int & top);
		virtual void initNativeCreatedWindow();
		virtual void createNativeWindow( int &left, int &top, uint &width, uint &height, String &title );
		virtual void reposition(int left, int top);
		virtual void resize(unsigned int width, unsigned int height);
		virtual void windowMovedOrResized();
		virtual void switchFullScreen(bool fullscreen);

	public:
            GtkEGLWindow(EGLSupport* glsupport);
           virtual  ~GtkEGLWindow();

			/**
			@remarks
			* Get custom attribute; the following attributes are valid:
			* XDISPLAY        The X Display connection behind that context.
			* XWINDOW        The X Window connection behind that context.
			* ATOM           The X Atom used in client delete events.
			*/
			virtual void getCustomAttribute(const String& name, void* pData);

			virtual void setFullscreen (bool fullscreen, uint width, uint height);
	};
}

#endif
