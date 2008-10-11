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
#include "OgreStableHeaders.h"
#include "OgreWindowEventUtilities.h"
#include "OgreRenderWindow.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#include <X11/Xlib.h>
void GLXProc( Ogre::RenderWindow *win, const XEvent &event );
#endif

using namespace Ogre;

WindowEventUtilities::WindowEventListeners WindowEventUtilities::_msListeners;
WindowEventUtilities::Windows WindowEventUtilities::_msWindows;

//--------------------------------------------------------------------------------//
void WindowEventUtilities::messagePump()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	// Windows Message Loop (NULL means check all HWNDs belonging to this context)
	MSG  msg;
	while( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	//GLX Message Pump
	Windows::iterator win = _msWindows.begin();
	Windows::iterator end = _msWindows.end();

	Display* xDisplay = 0; // same for all windows
	
	for (; win != end; win++)
	{
	    XID xid;
	    XEvent event;

	    if (!xDisplay)
		(*win)->getCustomAttribute("XDISPLAY", &xDisplay);

	    (*win)->getCustomAttribute("WINDOW", &xid);

	    while (XCheckWindowEvent (xDisplay, xid, StructureNotifyMask | VisibilityChangeMask | FocusChangeMask, &event))
	    {
		GLXProc(*win, event);
	    }

	    // The ClientMessage event does not appear under any Event Mask
	    while (XCheckTypedWindowEvent (xDisplay, xid, ClientMessage, &event))
	    {
		GLXProc(*win, event);
	    }
	}
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !defined __OBJC__
	// OSX Message Pump
	EventRef event = NULL;
	EventTargetRef targetWindow;
	targetWindow = GetEventDispatcherTarget();
    
    // If we are unable to get the target then we no longer care about events.
    if( !targetWindow ) return;
    
    // Grab the next event, process it if it is a window event
	if( ReceiveNextEvent( 0, NULL, kEventDurationNoWait, true, &event ) == noErr )
	{
        // Dispatch the event
		SendEventToEventTarget( event, targetWindow );
   		ReleaseEvent( event );
	}
#endif
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::addWindowEventListener( RenderWindow* window, WindowEventListener* listener )
{
	_msListeners.insert(std::make_pair(window, listener));
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::removeWindowEventListener( RenderWindow* window, WindowEventListener* listener )
{
	WindowEventListeners::iterator i = _msListeners.begin(), e = _msListeners.end();

	for( ; i != e; ++i )
	{
		if( i->first == window && i->second == listener )
		{
			_msListeners.erase(i);
			break;
		}
	}
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::_addRenderWindow(RenderWindow* window)
{
	_msWindows.push_back(window);
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::_removeRenderWindow(RenderWindow* window)
{
	Windows::iterator i = std::find(_msWindows.begin(), _msWindows.end(), window);
	if( i != _msWindows.end() )
		_msWindows.erase( i );
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//--------------------------------------------------------------------------------//
LRESULT CALLBACK WindowEventUtilities::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{	// Store pointer to Win32Window in user data area
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(((LPCREATESTRUCT)lParam)->lpCreateParams));
		return 0;
	}

	// look up window instance
	// note: it is possible to get a WM_SIZE before WM_CREATE
	RenderWindow* win = (RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!win)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	//LogManager* log = LogManager::getSingletonPtr();
	//Iterator of all listeners registered to this RenderWindow
	WindowEventListeners::iterator index,
        start = _msListeners.lower_bound(win),
        end = _msListeners.upper_bound(win);

	switch( uMsg )
	{
	case WM_ACTIVATE:
	{
        bool active = (LOWORD(wParam) != WA_INACTIVE);
        if( active )
        {
		    win->setActive( true );
        }
        else
        {
            if( win->isDeactivatedOnFocusChange() )
            {
    		    win->setActive( false );
            }
        }

	    for( ; start != end; ++start )
		    (start->second)->windowFocusChange(win);
		break;
	}
	case WM_SYSKEYDOWN:
		switch( wParam )
		{
		case VK_CONTROL:
		case VK_SHIFT:
		case VK_MENU: //ALT
			//return zero to bypass defProc and signal we processed the message
			return 0;
		}
		break;
	case WM_SYSKEYUP:
		switch( wParam )
		{
		case VK_CONTROL:
		case VK_SHIFT:
		case VK_MENU: //ALT
		case VK_F10:
			//return zero to bypass defProc and signal we processed the message
			return 0;
		}
		break;
	case WM_SYSCHAR:
		// return zero to bypass defProc and signal we processed the message, unless it's an ALT-space
		if (wParam != VK_SPACE)
			return 0;
		break;
	case WM_ENTERSIZEMOVE:
		//log->logMessage("WM_ENTERSIZEMOVE");
		break;
	case WM_EXITSIZEMOVE:
		//log->logMessage("WM_EXITSIZEMOVE");
		break;
	case WM_MOVE:
		//log->logMessage("WM_MOVE");
		win->windowMovedOrResized();
		for(index = start; index != end; ++index)
			(index->second)->windowMoved(win);
		break;
	case WM_DISPLAYCHANGE:
		win->windowMovedOrResized();
		for(index = start; index != end; ++index)
			(index->second)->windowResized(win);
		break;
	case WM_SIZE:
		//log->logMessage("WM_SIZE");
		win->windowMovedOrResized();
		for(index = start; index != end; ++index)
			(index->second)->windowResized(win);
		break;
	case WM_GETMINMAXINFO:
		// Prevent the window from going smaller than some minimu size
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
		break;
	case WM_CLOSE:
	{
		//log->logMessage("WM_CLOSE");
		bool close = true;
		for(index = start; index != end; ++index)
		{
			if (!(index->second)->windowClosing(win))
				close = false;
		}
		if (!close) return 0;

		for(index = _msListeners.lower_bound(win); index != end; ++index)
			(index->second)->windowClosed(win);
		win->destroy();
		return 0;
	}
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
//--------------------------------------------------------------------------------//
void GLXProc( RenderWindow *win, const XEvent &event )
{
	//An iterator for the window listeners
	WindowEventUtilities::WindowEventListeners::iterator index,
		start = WindowEventUtilities::_msListeners.lower_bound(win),
		end   = WindowEventUtilities::_msListeners.upper_bound(win);

	switch(event.type)
	{
	case ClientMessage:
	{
		::Atom atom;
		win->getCustomAttribute("ATOM", &atom);
		if(event.xclient.format == 32 && event.xclient.data.l[0] == (long)atom)
		{	//Window closed by window manager
			//Send message first, to allow app chance to unregister things that need done before
			//window is shutdown
			bool close = true;
            for(index = start ; index != end; ++index)
			{
				if (!(index->second)->windowClosing(win))
					close = false;
			}
			if (!close) return;

            for(index = start ; index != end; ++index)
                (index->second)->windowClosed(win);
			win->destroy();
		}
		break;
	}
	case DestroyNotify:
	{
		if (!win->isClosed())
		{
			// Window closed without window manager warning.
            for(index = start ; index != end; ++index)
                (index->second)->windowClosed(win);
			win->destroy();
		}
		break;
	}
	case ConfigureNotify:
	{    
        // This could be slightly more efficient if windowMovedOrResized took arguments:
		unsigned int oldWidth, oldHeight, oldDepth;
		int oldLeft, oldTop;
		win->getMetrics(oldWidth, oldHeight, oldDepth, oldLeft, oldTop);
		win->windowMovedOrResized();

		unsigned int newWidth, newHeight, newDepth;
		int newLeft, newTop;
		win->getMetrics(newWidth, newHeight, newDepth, newLeft, newTop);

		if (newLeft != oldLeft || newTop != oldTop)
		{
            for(index = start ; index != end; ++index)
                (index->second)->windowMoved(win);
		}

		if (newWidth != oldWidth || newHeight != oldHeight)
		{
            for(index = start ; index != end; ++index)
                (index->second)->windowResized(win);
		}
		break;
	}
	case FocusIn:     // Gained keyboard focus
	case FocusOut:    // Lost keyboard focus
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	case MapNotify:   //Restored
		win->setActive( true );
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	case UnmapNotify: //Minimised
		win->setActive( false );
		win->setVisible( false );
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	case VisibilityNotify:
		switch(event.xvisibility.state)
		{
		case VisibilityUnobscured:
			win->setActive( true );
			win->setVisible( true );
			break;
		case VisibilityPartiallyObscured:
			win->setActive( true );
			win->setVisible( true );
			break;
		case VisibilityFullyObscured:
			win->setActive( false );
			win->setVisible( false );
			break;
		}
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	default:
		break;
	} //End switch event.type
}
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !defined __OBJC__
//--------------------------------------------------------------------------------//
OSStatus WindowEventUtilities::_CarbonWindowHandler(EventHandlerCallRef nextHandler, EventRef event, void* wnd)
{
    OSStatus status = noErr;
    
    // Only events from our window should make it here
    // This ensures that our user data is our WindowRef
    RenderWindow* curWindow = (RenderWindow*)wnd;
    if(!curWindow) return eventNotHandledErr;
    
    //Iterator of all listeners registered to this RenderWindow
	WindowEventListeners::iterator index,
        start = _msListeners.lower_bound(curWindow),
        end = _msListeners.upper_bound(curWindow);
    
    // We only get called if a window event happens
    UInt32 eventKind = GetEventKind( event );
    
    LogManager* log = LogManager::getSingletonPtr();
    
    switch( eventKind )
    {
        case kEventWindowActivated:
            curWindow->setActive( true );
            for( ; start != end; ++start )
                (start->second)->windowFocusChange(curWindow);
            break;
        case kEventWindowDeactivated:
            if( curWindow->isDeactivatedOnFocusChange() )
            {
                curWindow->setActive( false );
            }

            for( ; start != end; ++start )
                (start->second)->windowFocusChange(curWindow);

            break;
        case kEventWindowShown:
        case kEventWindowExpanded:
            curWindow->setActive( true );
            curWindow->setVisible( true );
            for( ; start != end; ++start )
                (start->second)->windowFocusChange(curWindow);
            break;
        case kEventWindowHidden:
        case kEventWindowCollapsed:
            curWindow->setActive( false );
            curWindow->setVisible( false );
            for( ; start != end; ++start )
                (start->second)->windowFocusChange(curWindow);
            break;            
        case kEventWindowDragCompleted:
            curWindow->windowMovedOrResized();
            for( ; start != end; ++start )
				(start->second)->windowMoved(curWindow);
            break;
        case kEventWindowBoundsChanged:
            curWindow->windowMovedOrResized();
            for( ; start != end; ++start )
				(start->second)->windowResized(curWindow);
            break;
		case kEventWindowClose:
		{
			bool close = true;
			for( ; start != end; ++start )
			{
				if (!(start->second)->windowClosing(curWindow))
					close = false;
			}
			if (close)
				// This will cause event handling to continue on to the standard handler, which calls
				// DisposeWindow(), which leads to the 'kEventWindowClosed' event
				status = eventNotHandledErr;
			break;
		}
        case kEventWindowClosed:
            curWindow->destroy();
            for( ; start != end; ++start )
				(start->second)->windowClosed(curWindow);
            break;
        default:
            status = eventNotHandledErr;
            break;
    }
    
    return status;
}
#endif
