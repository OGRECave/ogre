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

#ifndef __X11EGLWindow_H__
#define __X11EGLWindow_H__

#include "OgreEGLWindow.h"
#include "OgreX11EGLSupport.h"

namespace Ogre {
    class _OgrePrivate X11EGLWindow : public EGLWindow
    {
    protected:
        X11EGLSupport* mGLSupport;

        //Changed these variables back to Window type because
        //it seems they are not used outside this class.
        Window mParentWindow;
        Window mExternalWindow;
        virtual EGLContext * createEGLContext() const;
        virtual void getLeftAndTopFromNativeWindow(int & left, int & top, uint width, uint height);
        virtual void initNativeCreatedWindow(const NameValuePairList *miscParams);
        virtual void createNativeWindow( int &left, int &top, uint &width, uint &height, String &title );
        virtual void reposition(int left, int top);
        virtual void resize(unsigned int width, unsigned int height);
        virtual void windowMovedOrResized();
        virtual void switchFullScreen(bool fullscreen);


    public:
            X11EGLWindow(X11EGLSupport* glsupport);
           virtual  ~X11EGLWindow();

            /**
            @remarks
            * Get custom attribute; the following attributes are valid:
            * XDISPLAY        The X Display connection behind that context.
            * XWINDOW        The X NativeWindowType connection behind that context.
            * ATOM           The X Atom used in client delete events.
            */
            virtual void getCustomAttribute(const String& name, void* pData);

            virtual void setFullscreen (bool fullscreen, uint width, uint height);

        //Moved this from EGLWindow because it has some native calls.
            void create(const String& name, unsigned int width, unsigned int height,
                        bool fullScreen, const NameValuePairList *miscParams);
    };
}

#endif
