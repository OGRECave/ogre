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

#ifndef __EAGLWindow_H__
#define __EAGLWindow_H__

#include "OgreRenderWindow.h"
#include "OgreEAGLSupport.h"
#include "OgreEAGLESContext.h"

#ifdef __OBJC__

// Forward declarations
@class CAEAGLLayer;

#import <UIKit/UIKit.h>

@interface EAGLView : UIView {
}

@end

#endif

namespace Ogre {
    class EAGLSupport;
    class EAGLESContext;

    class _OgrePrivate EAGLWindow : public RenderWindow
    {
        private:
        protected:
            bool mClosed;
            bool mVisible;
            bool mIsTopLevel;
            bool mIsExternal;
            bool mIsExternalGLControl;
            Viewport::Orientation mCurrentOrientation;
            /// The iPhone OS doesn't like rendering too quickly and will throw GL errors because the context is still in use
            /// This timer will be reset every 16 ms to simulate VSync at 60 Hz
            Timer *mAnimationTimer; 

            EAGLSupport* mGLSupport;
            EAGLESContext* mContext;
#ifdef __OBJC__
			NativeWindowType mWindow;
            EAGLView *mView;
#endif

            void switchFullScreen(bool fullscreen);
			void getLeftAndTopFromNativeWindow(int & left, int & top, uint width, uint height);
			void initNativeCreatedWindow(const NameValuePairList *miscParams);
			void createNativeWindow(int &left, int &top, uint &width, uint &height, String &title);
			void reposition(int left, int top);
			void resize(unsigned int width, unsigned int height);
			void windowMovedOrResized();

	public:
            EAGLWindow(EAGLSupport* glsupport);
            virtual ~EAGLWindow();

            void create(const String& name, unsigned int width, unsigned int height,
                        bool fullScreen, const NameValuePairList *miscParams);

			virtual void setFullscreen(bool fullscreen, uint width, uint height);
            void destroy(void);
            bool isClosed(void) const;
            bool isVisible(void) const;

            void setVisible(bool visible);
            void setClosed(bool closed);
            void swapBuffers(bool waitForVSync);
            void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);

            /**
               @remarks
               * Get custom attribute; the following attributes are valid:
               * WINDOW         The NativeWindowType target for rendering.
               * GLCONTEXT      The Ogre GLESContext used for rendering.
               * DISPLAY        EAGLDisplay connection behind that context.
               * DISPLAYNAME    The name for the connected display.
               */
            virtual void getCustomAttribute(const String& name, void* pData);

            bool requiresTextureFlipping() const;
    };
}

#endif
