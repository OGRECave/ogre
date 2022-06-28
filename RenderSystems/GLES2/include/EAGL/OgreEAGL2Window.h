/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#ifndef __EAGL2Window_H__
#define __EAGL2Window_H__

#include "OgreRenderWindow.h"
#include "OgreGLRenderTarget.h"
#include "OgreEAGLES2Context.h"

#ifdef __OBJC__
#import "OgreEAGL2View.h"
#import "OgreEAGL2ViewController.h"

// Forward declarations
@class CAEAGLLayer;

// Define the native window type
typedef UIWindow *NativeWindowType;

#endif

namespace Ogre {
    class EAGL2Support;

    class _OgrePrivate EAGL2Window : public RenderWindow, public GLRenderTarget
    {
        protected:
            bool mVisible;
            bool mHidden;
            /// Is this using an external window handle?
            bool mIsExternal;
            /// Is this using an external view handle?
            bool mUsingExternalView;
            /// Is this using an external view controller handle?
            bool mUsingExternalViewController;

            // iOS 4 content scaling
            bool mIsContentScalingSupported;
            float mContentScalingFactor;

            EAGL2Support* mGLSupport;
            EAGLES2Context* mContext;
#ifdef __OBJC__
            NativeWindowType mWindow;
            EAGL2View *mView;
            EAGL2ViewController *mViewController;
#else
            void *mWindowPlaceholder;
            void *mViewPlaceholder;
            void *mViewControllerPlaceholder;
#endif

            void createNativeWindow(uint widthPt, uint heightPt, const NameValuePairList *miscParams);
            void resize(unsigned int widthPt, unsigned int heightPt);
            void windowMovedOrResized();
            int _getPixelFromPoint(float viewPt) { return mIsContentScalingSupported ? (int)viewPt * mContentScalingFactor : (int)viewPt; }

    public:
            EAGL2Window(EAGL2Support* glsupport);
            virtual ~EAGL2Window();

            float getViewPointToPixelScale() { return mIsContentScalingSupported ? mContentScalingFactor : 1.0f; }
            void create(const String& name, unsigned int widthPt, unsigned int heightPt,
                        bool fullScreen, const NameValuePairList *miscParams);

            void destroy(void);
            bool isVisible(void) const { return mVisible; }
            void setVisible(bool visible) { mVisible = visible; }
            bool isHidden(void) const { return mHidden; }
            void setHidden(bool hidden);
            void setClosed(bool closed) { mClosed = closed; }
            void swapBuffers();
            void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer);
            PixelFormat suggestPixelFormat() const { return PF_BYTE_RGBA; }

            /**

               * Get custom attribute; the following attributes are valid:
               * WINDOW         The NativeWindowType target for rendering.
               * VIEW           The EAGLView object that is drawn into.
               * VIEWCONTROLLER The UIViewController used for handling view rotation.
               * GLCONTEXT      The Ogre GLContext used for rendering.
               * SHAREGROUP     The EAGLShareGroup object associated with the main context.
               */
            virtual void getCustomAttribute(const String& name, void* pData);

            bool requiresTextureFlipping() const { return false; }

            virtual GLContext* getContext() const { return mContext; }
    };
}

#endif
