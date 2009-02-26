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

#ifndef __EGLWindow_H__
#define __EGLWindow_H__

#include "OgreRenderWindow.h"
#include "OgreEGLSupport.h"
#include "OgreEGLContext.h"

namespace Ogre {
    class _OgrePrivate EGLWindow : public RenderWindow
    {
        protected:
            bool mClosed;
            bool mVisible;
            bool mIsTopLevel;
            bool mIsExternal;
            bool mIsExternalGLControl;

            EGLSupport* mGLSupport;
            EGLContext* mContext;
			NativeWindowType mWindow;
			NativeDisplayType mNativeDisplay;
			::EGLDisplay mEglDisplay;


			::EGLConfig mEglConfig;
			::EGLSurface mEglSurface;

            ::EGLSurface createSurfaceFromWindow(::EGLDisplay display, NativeWindowType win);

			virtual void switchFullScreen(bool fullscreen) = 0;
			virtual EGLContext * createEGLContext() const = 0;
			virtual void getLeftAndTopFromNativeWindow(int & left, int & top) = 0;
			virtual void initNativeCreatedWindow() = 0;
			virtual void createNativeWindow( int &left, int &top, uint &width, uint &height, String &title ) = 0;
			virtual void reposition(int left, int top) = 0;
			virtual void resize(unsigned int width, unsigned int height) = 0;
			virtual void windowMovedOrResized() = 0;

	public:
            EGLWindow(EGLSupport* glsupport);
            virtual ~EGLWindow();

            void create(const String& name, unsigned int width, unsigned int height,
                        bool fullScreen, const NameValuePairList *miscParams);

			virtual void setFullscreen (bool fullscreen, uint width, uint height);
            void destroy(void);
            bool isClosed(void) const;
            bool isVisible(void) const;
            void setVisible(bool visible);
            void swapBuffers(bool waitForVSync);
            void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);

            /**
               @remarks
               * Get custom attribute; the following attributes are valid:
               * WINDOW         The X Window target for rendering.
               * GLCONTEXT      The Ogre GLContext used for rendering.
               * DISPLAY        EGLDisplay connection behind that context.
               * DISPLAYNAME    The name for the connected display.
               */
            virtual void getCustomAttribute(const String& name, void* pData);

            bool requiresTextureFlipping() const;
    };
}

#endif
