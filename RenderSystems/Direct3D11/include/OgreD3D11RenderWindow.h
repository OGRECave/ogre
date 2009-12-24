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
#ifndef __D3D11RENDERWINDOW_H__
#define __D3D11RENDERWINDOW_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreRenderWindow.h"


namespace Ogre 
{
	class D3D11RenderWindow : public RenderWindow
	{
	public:
		/** Constructor.
		@param instance The application instance
		@param driver The root driver
		@param deviceIfSwapChain The existing D3D device to create an additional swap chain from, if this is not
		the first window.
		*/
		D3D11RenderWindow(HINSTANCE instance, D3D11Device & device, IDXGIFactory1*	pDXGIFactory);
		~D3D11RenderWindow();
		void create(const String& name, unsigned int width, unsigned int height,
			bool fullScreen, const NameValuePairList *miscParams);
		void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
		void destroy(void);
		bool isVisible() const;
		bool isClosed() const { return mClosed; }
		void reposition(int left, int top);
		void resize(unsigned int width, unsigned int height);
		void swapBuffers( bool waitForVSync = true );
		HWND getWindowHandle() const;

		void getCustomAttribute( const String& name, void* pData );
		/** Overridden - see RenderTarget.
		*/
		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		bool requiresTextureFlipping() const;

		// Method for dealing with resize / move & 3d library
		void windowMovedOrResized();

		/// Get the presentation parameters used with this window
		DXGI_SWAP_CHAIN_DESC* getPresentationParameters(void);

		/// @copydoc RenderTarget::update
		void update(bool swap);

		/** Create (or recreate) the D3D device or SwapChain for this window.
		*/
		void createD3DResources();

		/** Destroy the D3D device or SwapChain for this window.
		*/
		void destroyD3DResources();

		/// Are we in the middle of switching between fullscreen and windowed
		bool _getSwitchingFullscreen() const;
		/// Indicate that fullscreen / windowed switching has finished
		void _finishSwitchingFullscreen();


	protected:
		HINSTANCE mInstance;			// Process instance
		D3D11Device & mDevice;			// D3D11 driver
		IDXGIFactory1*	mpDXGIFactory;
		HWND	mHWnd;					// Win32 Window handle
		bool	mIsExternal;			// window not created by Ogre
		bool	mSizing;
		bool	mClosed;
		bool	mIsSwapChain;			// Is this a secondary window?
		bool	mSwitchingFullscreen;	// Are we switching from fullscreen to windowed or vice versa

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------

		// Pointer to swap chain, only valid if mIsSwapChain
		IDXGISwapChain * mpSwapChain;
		DXGI_SWAP_CHAIN_DESC md3dpp;
		DXGI_SAMPLE_DESC mFSAAType;
		//DWORD mFSAAQuality;
		UINT mDisplayFrequency;
		bool mVSync;
		unsigned int mVSyncInterval;
		bool mUseNVPerfHUD;
		ID3D11RenderTargetView*		mRenderTargetView;
		ID3D11DepthStencilView*		mDepthStencilView;
		ID3D11Texture2D*			mpBackBuffer;

		// just check if the multisampling requested is supported by the device
		bool _checkMultiSampleQuality(UINT SampleCount, UINT *outQuality, DXGI_FORMAT format);

	};
}
#endif
