
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
#    if !__OGRE_WINRT_PHONE_80
#    include <windows.ui.xaml.media.dxinterop.h>
#    endif
 
#endif

namespace Ogre 
{
    class D3D11RenderWindowBase 
        : public RenderWindow
    {
    public:
        D3D11RenderWindowBase(D3D11Device& device, IDXGIFactoryN*   pDXGIFactory);
        ~D3D11RenderWindowBase();
        virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        void reposition(int left, int top)                      {}
        void resize(unsigned int width, unsigned int height)    {}
        /// @copydoc RenderTarget::setFSAA
        virtual void setFSAA(uint fsaa, const String& fsaaHint) { mFSAA = fsaa; mFSAAHint = fsaaHint; resize(mWidth, mHeight); }

        bool isClosed() const                                   { return mClosed; }
        bool isHidden() const                                   { return mHidden; }

        void getCustomAttribute( const String& name, void* pData );
        /** Overridden - see RenderTarget.
        */
        virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
        bool requiresTextureFlipping() const                    { return false; }

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		/** Validate the type of stereo that is enabled for this window.*/
		void _validateStereo();
#endif

    protected:
        void _createSizeDependedD3DResources(); // assumes mpBackBuffer is already initialized
        void _destroySizeDependedD3DResources();

        IDXGIDeviceN* _queryDxgiDevice(); // release after use

        void _updateViewportsDimensions();

    protected:
        D3D11Device & mDevice;          // D3D11 driver
        IDXGIFactoryN*  mpDXGIFactory;
        bool    mIsExternal;            // window not created by Ogre
        bool    mSizing;
        bool    mClosed;
        bool    mHidden;

        // -------------------------------------------------------
        // DirectX-specific
        // -------------------------------------------------------
        DXGI_SAMPLE_DESC mFSAAType;
        UINT mDisplayFrequency;
        bool mVSync;
        unsigned int mVSyncInterval;

        // Window size depended resources - must be released before swapchain resize and recreated later
        ID3D11Texture2D*            mpBackBuffer;
        ID3D11RenderTargetView*     mRenderTargetView;
        ID3D11DepthStencilView*     mDepthStencilView;
    };

    
    class D3D11RenderWindowSwapChainBased
        : public D3D11RenderWindowBase
    {
    public:
        D3D11RenderWindowSwapChainBased(D3D11Device& device, IDXGIFactoryN* pDXGIFactory);
        ~D3D11RenderWindowSwapChainBased()                      { destroy(); }
        virtual void destroy(void);

        /// Get the presentation parameters used with this window
        DXGI_SWAP_CHAIN_DESC_N* getPresentationParameters(void) { return &mSwapChainDesc; }

		IDXGISwapChainN * _getSwapChain() { return mpSwapChain; }

        /// @copydoc RenderTarget::setFSAA
        virtual void setFSAA(uint fsaa, const String& fsaaHint) { mFSAA = fsaa; mFSAAHint = fsaaHint; _recreateSwapChain(); }

        void swapBuffers( );
		void updateStats(void);

		bool IsWindows8OrGreater();
		
    protected:
        void _createSwapChain();
        virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice) = 0;
        void _destroySwapChain();
        void _recreateSwapChain(); // required to change FSAA
        void _resizeSwapChainBuffers(unsigned width, unsigned height);
        void _createSizeDependedD3DResources(); // obtains mpBackBuffer from mpSwapChain

		int getVBlankMissCount( );

    protected:
        // Pointer to swap chain
        IDXGISwapChainN * mpSwapChain;
        DXGI_SWAP_CHAIN_DESC_N mSwapChainDesc;

		DXGI_FRAME_STATISTICS		mPreviousPresentStats;			// We save the previous present stats - so we can detect a "vblank miss"
		bool						mPreviousPresentStatsIsValid;	// Does mLastPresentStats data is valid (it isn't if when you start or resize the window)
		uint						mVBlankMissCount;				// Number of times we missed the v sync blank
		bool						mUseFlipMode;					// Flag to determine if the swapchain flip model is enabled.

    };

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

    class D3D11RenderWindowHwnd 
        : public D3D11RenderWindowSwapChainBased
    {
    public:
        D3D11RenderWindowHwnd(D3D11Device& device, IDXGIFactoryN*   pDXGIFactory);
        ~D3D11RenderWindowHwnd()                                { destroy(); }
        virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        bool isVisible() const;
        void setHidden(bool hidden);
        void reposition(int left, int top);
        void resize(unsigned int width, unsigned int height);
        void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);

        // Method for dealing with resize / move & 3d library
        void windowMovedOrResized();
    
        HWND getWindowHandle() const                            { return mHWnd; }
        void getCustomAttribute( const String& name, void* pData );
		void adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
			unsigned int* winWidth, unsigned int* winHeight);

		DWORD getWindowStyle(bool fullScreen) const { if (fullScreen) return mFullscreenWinStyle; return mWindowedWinStyle; }
		void updateWindowRect();
		void _beginUpdate();

		void				setVSyncEnabled		(bool vsync);
		bool				isVSyncEnabled		() const;
		void				setVSyncInterval	(unsigned int interval);
		unsigned int		getVSyncInterval	() const;

    protected:

        /// Indicate that fullscreen / windowed switching has finished
        void _finishSwitchingFullscreen();

        virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice);
        void setActive(bool state);
    protected:
        HWND    mHWnd;                  // Win32 window handle
		DWORD						mWindowedWinStyle;		// Windowed mode window style flags.
		DWORD						mFullscreenWinStyle;	// Fullscreen mode window style flags.		 
		unsigned int				mDesiredWidth;			// Desired width after resizing
		unsigned int				mDesiredHeight;			// Desired height after resizing
		int							mLastSwitchingFullscreenCounter;	// the last value of the switching fullscreen counter when we switched
    };

#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    
    class D3D11RenderWindowCoreWindow
        : public D3D11RenderWindowSwapChainBased
    {
    public:
        D3D11RenderWindowCoreWindow(D3D11Device& device, IDXGIFactoryN* pDXGIFactory);
        ~D3D11RenderWindowCoreWindow()                          { destroy(); }

        virtual float getViewPointToPixelScale();
        virtual void create(const String& name, unsigned widthPt, unsigned heightPt, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        Windows::UI::Core::CoreWindow^ getCoreWindow() const    { return mCoreWindow.Get(); }

        bool isVisible() const;

        // Method for dealing with resize / move & 3d library
        void windowMovedOrResized();

    protected:
        virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice);

    protected:
        Platform::Agile<Windows::UI::Core::CoreWindow> mCoreWindow;
    };

#if !__OGRE_WINRT_PHONE_80

    class D3D11RenderWindowImageSource
        : public D3D11RenderWindowBase
    {
    public:
        D3D11RenderWindowImageSource(D3D11Device& device, IDXGIFactoryN* pDXGIFactory);
        ~D3D11RenderWindowImageSource()                         { destroy(); }
        virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        virtual void resize(unsigned int width, unsigned int height);
        virtual void update(bool swapBuffers = true);
        virtual void swapBuffers();

        virtual bool isVisible() const                          { return mImageSourceNative != NULL; }

        Windows::UI::Xaml::Media::ImageBrush^ getImageBrush() const { return mBrush; }
        virtual void getCustomAttribute( const String& name, void* pData ); // "ImageBrush" -> Windows::UI::Xaml::Media::ImageBrush^

    protected:
        void _createSizeDependedD3DResources(); // creates mpBackBuffer and optionally mpBackBufferNoMSAA

    protected:
        Windows::UI::Xaml::Media::ImageBrush^                   mBrush;             // size independed
        Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^  mImageSource;       // size depended, can be NULL
        ISurfaceImageSourceNative*                              mImageSourceNative; // size depended, can be NULL
        ID3D11Texture2D*                                        mpBackBufferNoMSAA; // size depended, optional
    };
#endif // !__OGRE_WINRT_PHONE_80

#endif

}
#endif
