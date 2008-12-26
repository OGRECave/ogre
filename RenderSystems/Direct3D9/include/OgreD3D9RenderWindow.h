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
#include "OgreD3D9Driver.h"

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
		D3D9RenderWindow(HINSTANCE instance, D3D9Driver *driver, bool isSwapChain);
		~D3D9RenderWindow();
		void create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams);
		void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
		void destroy(void);
		bool isActive() const;
		bool isVisible() const;
		bool isClosed() const { return mClosed; }
		void reposition(int left, int top);
		void resize(unsigned int width, unsigned int height);
		void swapBuffers( bool waitForVSync = true );
		HWND getWindowHandle() const { return mHWnd; }

		D3D9Driver* getDirectD3DDriver() { return mDriver; }
		// changed to access driver member
		LPDIRECT3DDEVICE9 getD3DDevice() { return mDriver->getD3DDevice(); }

		void getCustomAttribute( const String& name, void* pData );
		/** Overridden - see RenderTarget.
		*/
		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		bool requiresTextureFlipping() const { return false; }

		// Method for dealing with resize / move & 3d library
		void windowMovedOrResized();

		/// Build the presentation parameters used with this window
		void buildPresentParameters(void);

		/// Get the presentation parameters used with this window
		D3DPRESENT_PARAMETERS* getPresentationParameters(void) 
		{ return &md3dpp; }

		/// @copydoc RenderTarget::update
		void update(bool swap);
		
		/** Create (or recreate) the D3D device or SwapChain for this window.
		*/
		void createD3DResources();
	
		/** Destroy the D3D device or SwapChain for this window.
		*/
		void destroyD3DResources();
	
		/// Accessor for render surface
		LPDIRECT3DSURFACE9 getRenderSurface() { return mpRenderSurface; }

		/// Are we in the middle of switching between fullscreen and windowed
		bool _getSwitchingFullscreen() const { return mSwitchingFullscreen; }
		/// Indicate that fullscreen / windowed switching has finished
		void _finishSwitchingFullscreen();
	protected:
		HINSTANCE mInstance;			// Process instance
		D3D9Driver *mDriver;			// D3D9 driver
		HWND	mHWnd;					// Win32 Window handle
		int		mHeadIndex;				// Head adapater index.
		bool	mIsExternal;			// window not created by Ogre
		bool	mSizing;
		bool	mClosed;
		bool	mIsSwapChain;			// Is this a secondary window?
		bool	mSwitchingFullscreen;	// Are we switching from fullscreen to windowed or vice versa

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------

		// Pointer to swap chain, only valid if mIsSwapChain
		LPDIRECT3DSWAPCHAIN9 mpSwapChain;
		D3DPRESENT_PARAMETERS md3dpp;
		LPDIRECT3DSURFACE9 mpRenderSurface;
		LPDIRECT3DSURFACE9 mpRenderZBuffer;
		D3DMULTISAMPLE_TYPE mFSAAType;
		DWORD mFSAAQuality;
		UINT mDisplayFrequency;
		bool mVSync;
		bool mUseNVPerfHUD;


	};
}
#endif
