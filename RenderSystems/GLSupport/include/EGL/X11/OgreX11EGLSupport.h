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

#ifndef __X11EGLSupport_H__
#define __X11EGLSupport_H__

// Tell EGL that we are using X11 (to select the appropriate definitions)
#define USE_X11

#include "OgreEGLSupport.h"

#ifndef Status
#define Status int
#endif 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
//  #include <X11/extensions/Xrandr.h>
#include <sys/time.h>
#include <X11/keysym.h>

namespace Ogre {
    class _OgrePrivate X11EGLSupport : public EGLSupport
    {
        protected:

            //removed createEGLWindow because it was easier to just use
            // new X11EGLWindow in the code to get the native version.
            //virtual EGLWindow* createEGLWindow( EGLSupport * support);

        public:
            X11EGLSupport(int profile);
            virtual ~X11EGLSupport();

            String getDisplayName(void);

            NativeDisplayType getNativeDisplay(void);
            XVisualInfo *getVisualFromFBConfig(::EGLConfig glConfig);
            Atom mAtomDeleteWindow;
            Atom mAtomFullScreen;
            Atom mAtomState;

            // This just sets the native variables needed by EGLSupport::getGLDisplay
            //Then it calls EGLSupport::getDLDisplay to do the rest of the work.
            EGLDisplay getGLDisplay();

            //Moved this here from EGLSupport because maybe it should be more native.
                    RenderWindow* newWindow(const String& name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams = 0);

    };
}

#endif

