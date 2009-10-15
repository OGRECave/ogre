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
            void changeOrientation(Viewport::Orientation orient);

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
