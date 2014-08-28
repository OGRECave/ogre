
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __D3D11RENDERWINDOW_H__
#define __D3D11RENDERWINDOW_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreRenderWindow.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT 
#pragma warning( disable : 4451 ) // http://social.msdn.microsoft.com/Forums/en-US/winappswithnativecode/thread/314b5826-0a66-4307-abfe-87b8052c3c07/

#    include <agile.h>
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PC_APP)
#    include <windows.ui.xaml.media.dxinterop.h>
#    endif
 
#endif

namespace Ogre 
{
	class D3D11RenderWindowBase 
		: public RenderWindow
	{
	public:
		D3D11RenderWindowBase(D3D11Device& device, IDXGIFactoryN*	pDXGIFactory);
		~D3D11RenderWindowBase();
		virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
		virtual void destroy(void);

		void reposition(int left, int top)						{}
		void resize(unsigned int width, unsigned int height)	{}

		bool isClosed() const									{ return mClosed; }
		bool isHidden() const									{ return mHidden; }

		void getCustomAttribute( const String& name, void* pData );
		/** Overridden - see RenderTarget.
		*/
		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		bool requiresTextureFlipping() const					{ return false; }

	protected:
		void _createSizeDependedD3DResources(); // assumes mpBackBuffer is already initialized
		void _destroySizeDependedD3DResources();

		IDXGIDeviceN* _queryDxgiDevice(); // release after use
	
		void _updateViewportsDimensions();

	protected:
		D3D11Device & mDevice;			// D3D11 driver
		IDXGIFactoryN*	mpDXGIFactory;
		bool	mIsExternal;			// window not created by Ogre
		bool	mSizing;
		bool	mClosed;
		bool	mHidden;

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------
		DXGI_SAMPLE_DESC mFSAAType;
		UINT mDisplayFrequency;
		bool mVSync;
		unsigned int mVSyncInterval;

		// Window size depended resources - must be released before swapchain resize and recreated later
		ID3D11Texture2D*			mpBackBuffer;
		ID3D11RenderTargetView*		mRenderTargetView;
		ID3D11DepthStencilView*		mDepthStencilView;
	};

	
	class D3D11RenderWindowSwapChainBased
		: public D3D11RenderWindowBase
	{
	public:
		D3D11RenderWindowSwapChainBased(D3D11Device& device, IDXGIFactoryN*	pDXGIFactory);
		~D3D11RenderWindowSwapChainBased()						{ destroy(); }
		virtual void destroy(void);

		/// Get the presentation parameters used with this window
		DXGI_SWAP_CHAIN_DESC_N* getPresentationParameters(void)	{ return &mSwapChainDesc; }

		void swapBuffers( );

	protected:
		void _createSizeDependedD3DResources(); // obtains mpBackBuffer from mpSwapChain
		void _createSwapChain();
		virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice) = 0;
		void _resizeSwapChainBuffers(unsigned width, unsigned height);

	protected:
		// Pointer to swap chain
		IDXGISwapChainN * mpSwapChain;
		DXGI_SWAP_CHAIN_DESC_N mSwapChainDesc;
	};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

	class D3D11RenderWindowHwnd 
		: public D3D11RenderWindowSwapChainBased
	{
	public:
		D3D11RenderWindowHwnd(D3D11Device& device, IDXGIFactoryN*	pDXGIFactory);
		~D3D11RenderWindowHwnd()								{ destroy(); }
		virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
		virtual void destroy(void);

		bool isVisible() const;
		void setHidden(bool hidden);
		void reposition(int left, int top);
		void resize(unsigned int width, unsigned int height);
		void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);

		// Method for dealing with resize / move & 3d library
		void windowMovedOrResized();
	
		HWND getWindowHandle() const							{ return mHWnd; }
		void getCustomAttribute( const String& name, void* pData );

	protected:
		/// Are we in the middle of switching between fullscreen and windowed
		bool _getSwitchingFullscreen() const					{ return mSwitchingFullscreen; }
		/// Indicate that fullscreen / windowed switching has finished
		void _finishSwitchingFullscreen();

		virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice);
		void setActive(bool state);
	protected:
		HWND	mHWnd;					// Win32 window handle
		bool	mSwitchingFullscreen;	// Are we switching from fullscreen to windowed or vice versa
	};

#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
	
	class D3D11RenderWindowCoreWindow
		: public D3D11RenderWindowSwapChainBased
	{
	public:
		D3D11RenderWindowCoreWindow(D3D11Device& device, IDXGIFactoryN*	pDXGIFactory);
		~D3D11RenderWindowCoreWindow()							{ destroy(); }
		virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
		virtual void destroy(void);

		Windows::UI::Core::CoreWindow^ getCoreWindow() const	{ return mCoreWindow.Get(); }

		bool isVisible() const;

		// Method for dealing with resize / move & 3d library
		void windowMovedOrResized();

	protected:
		virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice);

	protected:
		Platform::Agile<Windows::UI::Core::CoreWindow> mCoreWindow;
	};

#if (OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && (OGRE_WINRT_TARGET_TYPE == DESKTOP_APP)

	class D3D11RenderWindowImageSource
		: public D3D11RenderWindowBase
	{
	public:
		D3D11RenderWindowImageSource(D3D11Device& device, IDXGIFactoryN* pDXGIFactory);
		~D3D11RenderWindowImageSource()							{ destroy(); }
		virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
		virtual void destroy(void);

		virtual void resize(unsigned int width, unsigned int height);
		virtual void update(bool swapBuffers = true);
		virtual void swapBuffers();

		virtual bool isVisible() const							{ return mImageSourceNative != NULL; }

		Windows::UI::Xaml::Media::ImageBrush^ getImageBrush() const	{ return mBrush; }
		virtual void getCustomAttribute( const String& name, void* pData ); // "ImageBrush" -> Windows::UI::Xaml::Media::ImageBrush^

	protected:
		void _createSizeDependedD3DResources(); // creates mpBackBuffer

	protected:
		Windows::UI::Xaml::Media::ImageBrush^					mBrush;				// size independed
		Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^	mImageSource;		// size depended, can be NULL
		ISurfaceImageSourceNative*								mImageSourceNative;	// size depended, can be NULL
	};
#endif //  (OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && (OGRE_WINRT_TARGET_TYPE == DESKTOP_APP)

#endif

}
#endif
