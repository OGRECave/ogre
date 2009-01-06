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
#ifndef __OgreWindowEventUtils_H__
#define __OgreWindowEventUtils_H__

#include "OgrePrerequisites.h"
#include "OgrePlatform.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef NOMINMAX
#  define NOMINMAX // required to stop windows.h screwing up std::min definition
#endif

namespace Ogre
{
	/**
	@Remarks
		Callback class used to send out window events to client app
	*/
	class _OgreExport WindowEventListener
	{
	public:
		virtual ~WindowEventListener() {};

		/**
		@Remarks
			Window has moved position
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowMoved(RenderWindow* rw)   {}

		/**
		@Remarks
			Window has resized
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowResized(RenderWindow* rw) {}

		/**
		@Remarks
			Window is closing (Only triggered if user pressed the [X] button)
		@param rw
			The RenderWindow which created this events
		@return True will close the window(default).
		*/
		virtual bool windowClosing(RenderWindow* rw)
		{ return true; }

		/**
		@Remarks
			Window has been closed (Only triggered if user pressed the [X] button)
		@param rw
			The RenderWindow which created this events
		@note
			The window has not actually close yet when this event triggers. It's only closed after
			all windowClosed events are triggered. This allows apps to deinitialise properly if they
			have services that needs the window to exist when deinitialising.
		*/
		virtual void windowClosed(RenderWindow* rw)  {}

		/**
		@Remarks
			Window has lost/gained focus
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowFocusChange(RenderWindow* rw) {}
	};

	/**
	@Remarks
		Utility class to handle Window Events/Pumping/Messages
	*/
	class _OgreExport WindowEventUtilities
	{
	public:
		/**
		@Remarks
			Call this once per frame if not using Root:startRendering(). This will update all registered
			RenderWindows (If using external Windows, you can optionally register those yourself)
		*/
		static void messagePump();

		/**
		@Remarks
			Add a listener to listen to renderwindow events (multiple listener's per renderwindow is fine)
			The same listener can listen to multiple windows, as the Window Pointer is sent along with
			any messages.
		@param window
			The RenderWindow you are interested in monitoring
		@param listner
			Your callback listener
		*/
		static void addWindowEventListener( RenderWindow* window, WindowEventListener* listener );

		/**
		@Remarks
			Remove previously added listener
		@param window
			The RenderWindow you registered with
		@param listner
			The listener registered
		*/
		static void removeWindowEventListener( RenderWindow* window, WindowEventListener* listener );

		/**
		@Remarks
			Called by RenderWindows upon creation for Ogre generated windows. You are free to add your
			external windows here too if needed.
		@param window
			The RenderWindow to monitor
		*/
		static void _addRenderWindow(RenderWindow* window);

		/**
		@Remarks
			Called by RenderWindows upon creation for Ogre generated windows. You are free to add your
			external windows here too if needed.
		@param window
			The RenderWindow to remove from list
		*/
		static void _removeRenderWindow(RenderWindow* window);

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		//! Internal winProc (RenderWindow's use this when creating the Win32 Window)
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !defined __OBJC__
        //! Internal UPP Window Handler (RenderWindow's use this when creating the OS X Carbon Window
        static OSStatus _CarbonWindowHandler(EventHandlerCallRef nextHandler, EventRef event, void* wnd);
#endif

		//These are public only so GLXProc can access them without adding Xlib headers header
		typedef multimap<RenderWindow*, WindowEventListener*>::type WindowEventListeners;
		static WindowEventListeners _msListeners;

		typedef vector<RenderWindow*>::type Windows;
		static Windows _msWindows;
	};
}
#endif
