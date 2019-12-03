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

#ifndef __GLXWindow_H__
#define __GLXWindow_H__

#include "OgreGLWindow.h"

#include <X11/X.h>

namespace Ogre 
{
    class GLXGLSupport;

    class _OgrePrivate GLXWindow : public GLWindow
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
        
        /** @copydoc see RenderWindow::setHidden */
        void setHidden(bool hidden);

        /** @copydoc see RenderWindow::setVSyncEnabled */
        void setVSyncEnabled(bool vsync);
        
        /** @copydoc see RenderWindow::reposition */
        void reposition(int left, int top);
        
        /** @copydoc see RenderWindow::resize */
        void resize(unsigned int width, unsigned int height);

        /** @copydoc see RenderWindow::windowMovedOrResized */
        void windowMovedOrResized();
        
        /** @copydoc see RenderWindow::swapBuffers */
        void swapBuffers();
        
        /**
           @remarks
           * Get custom attribute; the following attributes are valid:
           * WINDOW      The X Window target for rendering.
           * GLCONTEXT    The Ogre GLContext used for rendering.
           * DISPLAY        The X Display connection behind that context.
           * DISPLAYNAME    The X Server name for the connected display.
           * ATOM          The X Atom used in client delete events.
           */
        void getCustomAttribute(const String& name, void* pData);
        
        PixelFormat suggestPixelFormat() const;

    private:
        GLXGLSupport* mGLSupport;
        ::Window      mWindow;
        void switchFullScreen(bool fullscreen);
    };
}

#endif
