/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __X11EGLSupport_H__
#define __X11EGLSupport_H__


#include "OgreEGLSupport.h"

#if (OGRE_PLATFORM == OGRE_PLATFORM_LINUX)
	#include <X11/Xutil.h>
	#include <X11/extensions/Xrandr.h>
	#include <X11/Xutil.h>
	#include <sys/time.h>
	#include <X11/Xlib.h>
	#include <X11/keysym.h>
	#include <X11/extensions/Xrandr.h>
#else
	#define StructureNotifyMask 0
	#define VisibilityChangeMask 0
	#define FocusChangeMask 0
	#define CWBackPixel 0
	#define CWBorderPixel 0
	#define CWColormap 0
	#define CWEventMask 0
	#define NotUseful 0
	#define CWSaveUnder 0
	#define CWBackingStore 0
	#define CWOverrideRedirect 0
	#define InputOutput 0
	#define NormalState 0
	#define StateHint 0
	#define InputHint 0
	#define USPosition 0
	#define ClientMessage 0
	#define SubstructureRedirectMask 0
	#define SubstructureNotifyMask 0
	#define CurrentTime 0
	#define VisualIDMask 0
	#define AllocNone 0

	enum X11Bool
	{
		False
		,
		True
	};
	class NotPointerAtom{};
	typedef NotPointerAtom * Atom;

	class Display{};
	class XErrorEvent{};
	class XRRScreenConfiguration{};
	class Rotation{};
	class Visual{};
	#define Window  NativeWindowType

	struct XVisualInfo{int visualid, visual, depth;};
	struct XRRScreenSize{int width, height;};
	struct XWMHints{int initial_state, input, flags;};
	struct XSizeHints{int flags;};
	struct XWindowAttributes{NativeWindowType root; int x, y, width, height;};
	struct XSetWindowAttributes{int background_pixel, border_pixel, colormap, event_mask, backing_store, save_under, override_redirect;};
	struct XTextProperty{int * value;};
	struct StrangeData{StrangeData(){}StrangeData(Atom atom){}StrangeData(int num){}};
	struct XClientMessageEventData{StrangeData l[3];};
	struct XClientMessageEvent{int type, serial, send_event, format; XClientMessageEventData data ; NativeWindowType window; Atom message_type;};
	struct XEvent{};


	void XStringListToTextProperty(char ** prop, int num, XTextProperty * textProp);
	Window DefaultRootWindow(Display* nativeDisplayType);
	bool XQueryExtension(Display* nativeDisplayType, char * name, int * dummy0, int * dummy2, int * dummy3);
	XRRScreenConfiguration * XRRGetScreenInfo(Display* nativeDisplayType, Window window );
	int XRRConfigCurrentConfiguration(XRRScreenConfiguration * config, Rotation * rotation);
	XRRScreenSize * XRRConfigSizes(XRRScreenConfiguration * config, int * nSizes);
	int XRRConfigCurrentRate(XRRScreenConfiguration * config);
	short * XRRConfigRates(XRRScreenConfiguration * config, int sizeID, int * nRates);
	void XRRFreeScreenConfigInfo(XRRScreenConfiguration * config);
	int DefaultScreen(NativeDisplayType nativeDisplayType);
	int DisplayWidth(Display* nativeDisplayType, int screen);
	int DisplayHeight(Display* nativeDisplayType, int screen);
	Display* XOpenDisplay(int num);
	void XCloseDisplay(Display* nativeDisplayType);
	Atom XInternAtom(Display* nativeDisplayType, char * name, X11Bool isTrue);;
	char * DisplayString(NativeDisplayType nativeDisplayType);
	const char * XDisplayName(char * name);
	Visual * DefaultVisual(Display* nativeDisplayType,  int screen);
	int XVisualIDFromVisual(Visual *v);
	void XRRSetScreenConfigAndRate(Display* nativeDisplayType, XRRScreenConfiguration * config, Window window, int size, Rotation rotation, int mode, int currentTime );
	XVisualInfo * XGetVisualInfo(Display* nativeDisplayType,  int mask, XVisualInfo * info, int * n);
	typedef int (*XErrorHandler)(Display *, XErrorEvent*);
	XErrorHandler XSetErrorHandler(XErrorHandler xErrorHandler);
	void XDestroyWindow(Display* nativeDisplayType, Window nativeWindowType);
	bool XGetWindowAttributes(Display* nativeDisplayType, Window nativeWindowType, XWindowAttributes * xWindowAttributes);
	int XCreateColormap(Display* nativeDisplayType, Window nativeWindowType, int visual, int allocNone);
	Window XCreateWindow(Display* nativeDisplayType, Window nativeWindowType, int left, int top, int width, int height, int dummy1, int depth, int inputOutput, int visual, int mask, XSetWindowAttributes * xSetWindowAttributes);
	void XFree(void *data);
	XWMHints * XAllocWMHints();
	XSizeHints * XAllocSizeHints();
	void XSetWMProperties(Display* nativeDisplayType, Window nativeWindowType,XTextProperty * titleprop, char * dummy1, char * dummy2, int num, XSizeHints *sizeHints, XWMHints *wmHints, char * dummy3);
	void XSetWMProtocols(Display* nativeDisplayType, Window nativeWindowType, Atom * atom, int num);
	void XMapWindow(Display* nativeDisplayType, Window nativeWindowType);
	void XFlush(Display* nativeDisplayType);
	void XMoveWindow(Display* nativeDisplayType, Window nativeWindowType, int left, int top);
	void XResizeWindow(Display* nativeDisplayType, Window nativeWindowType, int left, int top);
	void XQueryTree(Display* nativeDisplayType, Window nativeWindowType, Window * root, Window *parent, Window **children, unsigned int * nChildren);
	void XSendEvent(Display* nativeDisplayType, Window nativeWindowType, int dummy1, int mask, XEvent* xevent);
#endif


namespace Ogre {
    class _OgrePrivate X11EGLSupport : public EGLSupport
    {
		protected:

			//removed createEGLWindow because it was easier to just use
			// new X11EGLWindow in the code to get the native version.
			//virtual EGLWindow* createEGLWindow( EGLSupport * support);

        public:
            X11EGLSupport();
            virtual ~X11EGLSupport();

			virtual GLESPBuffer* createPBuffer(PixelComponentType format,
				size_t width, size_t height);

			virtual void switchMode(uint& width, uint& height, short& frequency);
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
