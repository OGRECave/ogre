/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
		bool				isVSync				() const { return mVSync; }
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
		

		/// @copydoc RenderTarget::_beginUpdate
		void _beginUpdate();
	
		/// @copydoc RenderTarget::_updateViewport
		void _updateViewport(Viewport* viewport, bool updateStatistics = true);

		/// @copydoc RenderTarget::_endUpdate
		void _endUpdate();

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

		/** Validate the device for this window. */
		bool _validateDevice();

		void adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
			unsigned int* winWidth, unsigned int* winHeight);

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
		unsigned int				mVSyncInterval;		
		bool						mUseNVPerfHUD;			// Use NV Perf HUD.

		// Desired width / height after resizing
		unsigned int mDesiredWidth;
		unsigned int mDesiredHeight;


		void updateWindowRect();
	};
}
#endif
