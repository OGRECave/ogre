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
#ifndef __D3D9RENDERWINDOW_H__
#define __D3D9RENDERWINDOW_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreRenderWindow.h"
#include "OgreD3D9Device.h"

namespace Ogre 
{
	class D3D9RenderWindow : public RenderWindow
	{
	public:
		/** Constructor.
		@param instance The application instance
		@param driver The root driver
		@param deviceIfSwapChain The existing D3D device to create an additional swap chain from, if this is not
			the first window.
		*/
		D3D9RenderWindow					(HINSTANCE instance);
		~D3D9RenderWindow					();
		
		
		void				create				(const String& name, unsigned int width, unsigned int height,
												 bool fullScreen, const NameValuePairList *miscParams);
		void				setFullscreen		(bool fullScreen, unsigned int width, unsigned int height);
		void				destroy				(void);
		bool				isActive			() const;
		bool				isVisible			() const;
		bool 				isClosed			() const { return mClosed; }
		void 				reposition			(int left, int top);
		void 				resize				(unsigned int width, unsigned int height);
		void 				swapBuffers			( bool waitForVSync = true );
		HWND 				getWindowHandle		() const { return mHWnd; }				
		IDirect3DDevice9*	getD3D9Device		();
		D3D9Device*			getDevice			();
		void				setDevice			(D3D9Device* device);

		void				getCustomAttribute	(const String& name, void* pData);
		
		/** Overridden - see RenderTarget.
		*/
		void				copyContentsToMemory	(const PixelBox &dst, FrameBuffer buffer);
		bool				requiresTextureFlipping	() const { return false; }

		// Method for dealing with resize / move & 3d library
		void				windowMovedOrResized	();
	
		/// Build the presentation parameters used with this window
		void				buildPresentParameters	(D3DPRESENT_PARAMETERS* presentParams);
		
		/// @copydoc RenderTarget::update
		void update(bool swap);				
	
		/// Accessor for render surface
		IDirect3DSurface9* getRenderSurface();

		/// Are we in the middle of switching between fullscreen and windowed
		bool _getSwitchingFullscreen() const;
		
		/// Indicate that fullscreen / windowed switching has finished
		void _finishSwitchingFullscreen();
	
		/// Returns true if this window use depth buffer.
		bool isDepthBuffered() const;

		/// Returns true if this window should use NV perf hud adapter.
		bool isNvPerfHUDEnable() const;

	protected:
		HINSTANCE					mInstance;				// Process instance
		D3D9Device* 				mDevice;				// D3D9 device wrapper class.
		bool						mDeviceValid;			// Device was validation succeeded.
		HWND						mHWnd;					// Win32 Window handle		
		bool						mIsExternal;			// window not created by Ogre
		bool						mClosed;				// Is this window destroyed.		
		bool						mSwitchingFullscreen;	// Are we switching from fullscreen to windowed or vice versa		
		D3DMULTISAMPLE_TYPE			mFSAAType;				// AA type.
		DWORD						mFSAAQuality;			// AA quality.
		UINT						mDisplayFrequency;		// Display frequency.
		bool						mVSync;					// Use vertical sync or not.
		bool						mUseNVPerfHUD;			// Use NV Perf HUD.

		void updateWindowRect();
	};
}
#endif
