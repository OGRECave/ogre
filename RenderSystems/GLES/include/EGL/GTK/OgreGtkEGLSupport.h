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

#ifndef __GTKEGLSupport_H__
#define __GTKEGLSupport_H__


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

	enum GtkBool
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
	Atom XInternAtom(Display* nativeDisplayType, char * name, GtkBool isTrue);;
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
    class _OgrePrivate GtkEGLSupport : public EGLSupport
    {
		protected:

			//removed createEGLWindow because it was easier to just use
			// new GtkEGLWindow in the code to get the native version.
			//virtual EGLWindow* createEGLWindow( EGLSupport * support);

        public:
            GtkEGLSupport();
            virtual ~GtkEGLSupport();

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
