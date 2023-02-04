/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __EGLWindow_H__
#define __EGLWindow_H__

#include "OgreGLWindow.h"
#include "OgreEGLSupport.h"
#include "OgreEGLContext.h"

namespace Ogre {
    class _OgrePrivate EGLWindow : public GLWindow
    {
    private:
        protected:
            EGLSupport* mGLSupport;
            NativeWindowType mWindow;
            NativeDisplayType mNativeDisplay;
            ::EGLDisplay mEglDisplay;


            ::EGLConfig mEglConfig;
            ::EGLSurface mEglSurface;

            ::EGLSurface createSurfaceFromWindow(::EGLDisplay display, NativeWindowType win);

            virtual void switchFullScreen(bool fullscreen) {}
            EGLContext * createEGLContext(::EGLContext external = NULL) const {
                return new EGLContext(mEglDisplay, mGLSupport, mEglConfig, mEglSurface, external);
            }

            void windowMovedOrResized() override {}

            void finaliseWindow();
    public:
            EGLWindow(EGLSupport* glsupport);
            virtual ~EGLWindow();

            // default, PBuffer based, implementation
            void create(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                        const NameValuePairList* miscParams) override;

            void resize(unsigned int width, unsigned int height) override {}

            void setFullscreen (bool fullscreen, uint width, uint height) override;
            void destroy(void) override;
            void swapBuffers() override;
            void setVSyncEnabled(bool vsync) override;

            /**

               * Get custom attribute; the following attributes are valid:
               * WINDOW         The X NativeWindowType target for rendering.
               * GLCONTEXT      The Ogre GLContext used for rendering.
               * DISPLAY        EGLDisplay connection behind that context.
               */
            void getCustomAttribute(const String& name, void* pData) override;

            PixelFormat suggestPixelFormat() const override;
    };
}

#endif
