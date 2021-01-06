
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
#include "OgreD3D11DeviceResource.h"
#include "OgreD3D11Mappings.h"
#include "OgreD3D11RenderTarget.h"
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
    class _OgreD3D11Export D3D11RenderWindowBase
        : public RenderWindow, public D3D11RenderTarget
        , protected D3D11DeviceResource
    {
    public:
        D3D11RenderWindowBase(D3D11Device& device);
        ~D3D11RenderWindowBase();
        virtual void create(const String& name, unsigned widthPt, unsigned heightPt, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        void reposition(int left, int top)                      {}
        void resize(unsigned int width, unsigned int height)    {}
        /// @copydoc RenderTarget::setFSAA
        virtual void setFSAA(uint fsaa, const String& fsaaHint) { mFSAA = fsaa; mFSAAHint = fsaaHint; resize(mWidth, mHeight); }

        bool isClosed() const                                   { return mClosed; }
        bool isHidden() const                                   { return mHidden; }

        virtual uint getNumberOfViews() const;
        virtual ID3D11Texture2D* getSurface(uint index = 0) const;
        virtual ID3D11RenderTargetView* getRenderTargetView(uint index = 0) const;

        void getCustomAttribute( const String& name, void* pData );
        /** Overridden - see RenderTarget. */
        virtual void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer);
        bool requiresTextureFlipping() const                    { return false; }

        virtual bool _shouldRebindBackBuffer()                  { return false; }
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		/** Validate the type of stereo that is enabled for this window.*/
		void _validateStereo();
#endif

    protected:
        /** Overridden - see RenderTarget. */
        virtual void updateImpl();

        virtual DXGI_FORMAT _getBasicFormat()                   { return DXGI_FORMAT_B8G8R8A8_UNORM; } // preferred since Win8
        DXGI_FORMAT _getRenderFormat()                          { return D3D11Mappings::_getGammaFormat(_getBasicFormat(), isHardwareGammaEnabled()); }
        void _createSizeDependedD3DResources();                 // assumes mpBackBuffer is already initialized
        void _destroySizeDependedD3DResources();

        ComPtr<IDXGIDeviceN> _queryDxgiDevice()                 { ComPtr<IDXGIDeviceN> res; _queryDxgiDeviceImpl(res.GetAddressOf()); return res; }
        void _queryDxgiDeviceImpl(IDXGIDeviceN** dxgiDevice);   // release after use

        void _updateViewportsDimensions();

    protected:
        D3D11Device & mDevice;          // D3D11 driver
        bool    mIsExternal;            // window not created by Ogre
        bool    mSizing;
        bool    mClosed;
        bool    mHidden;

        DXGI_SAMPLE_DESC mFSAAType;     // Effective FSAA mode, limited by hardware capabilities

        // Window size depended resources - must be released before swapchain resize and recreated later
        ComPtr<ID3D11Texture2D>         mpBackBuffer;
        ComPtr<ID3D11Texture2D>         mpBackBufferNoMSAA;     // optional, always holds up-to-date copy data from mpBackBuffer if not NULL
        ComPtr<ID3D11RenderTargetView>  mRenderTargetView;
        ComPtr<ID3D11DepthStencilView>  mDepthStencilView;
    };

    
    class _OgreD3D11Export D3D11RenderWindowSwapChainBased
        : public D3D11RenderWindowBase
    {
    public:
        D3D11RenderWindowSwapChainBased(D3D11Device& device);
        ~D3D11RenderWindowSwapChainBased()                      { destroy(); }
        virtual void destroy(void);

        /// Get the swapchain details.
        IDXGISwapChainN* _getSwapChain()                        { return mpSwapChain.Get(); }
        DXGI_SWAP_CHAIN_DESC_N* _getSwapChainDescription(void)  { return &mSwapChainDesc; }
        virtual bool _shouldRebindBackBuffer()                  { return mUseFlipMode; }

        /// @copydoc RenderTarget::setFSAA
        virtual void setFSAA(uint fsaa, const String& fsaaHint) { mFSAA = fsaa; mFSAAHint = fsaaHint; _changeBuffersFSAA(); }

        void setVSyncEnabled(bool vsync)                        { mVSync = vsync; }
        bool isVSyncEnabled() const                             { return mVSync || mUseFlipMode; }
        void setVSyncInterval(unsigned interval)                { mVSyncInterval = interval; }

        void swapBuffers();
        void updateStats(void);

    protected:
        void notifyDeviceLost(D3D11Device* device);
        void notifyDeviceRestored(D3D11Device* device);

        DXGI_FORMAT _getSwapChainFormat()                       { return D3D11Mappings::_getGammaFormat(_getBasicFormat(), isHardwareGammaEnabled() && !mUseFlipMode); }
        void _createSwapChain();
        virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice) = 0;
        void _destroySwapChain();
        void _changeBuffersFSAA();
        void _resizeSwapChainBuffers(unsigned width, unsigned height);
        void _createSizeDependedD3DResources();                 // obtains mpBackBuffer and optionally mpBackBufferNoMSAA, former can be from mpSwapChain or standalone

        int getVBlankMissCount();

    protected:
        // Pointer to swap chain
        ComPtr<IDXGISwapChainN> mpSwapChain;
        DXGI_SWAP_CHAIN_DESC_N  mSwapChainDesc;

        bool                    mUseFlipMode;                   // Flag to determine if the swapchain flip model is enabled. Not supported before Win8.0, required for WinRT.
        bool                    mVSync;                         // mVSync assumed to be true if mUseFlipMode

        DXGI_FRAME_STATISTICS   mPreviousPresentStats;          // We save the previous present stats - so we can detect a "vblank miss"
        bool                    mPreviousPresentStatsIsValid;   // Does mLastPresentStats data is valid (it isn't if when you start or resize the window)
        uint                    mVBlankMissCount;               // Number of times we missed the v sync blank
    };

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

    class _OgreD3D11Export D3D11RenderWindowHwnd
        : public D3D11RenderWindowSwapChainBased
    {
    public:
        D3D11RenderWindowHwnd(D3D11Device& device);
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
        DWORD getWindowStyle(bool fullScreen) const             { return fullScreen ? mFullscreenWinStyle : mWindowedWinStyle; }
        void getCustomAttribute( const String& name, void* pData );

        void adjustWindow(unsigned int clientWidth, unsigned int clientHeight, unsigned int* winWidth, unsigned int* winHeight);
        void updateWindowRect();
        void _beginUpdate();

    protected:
        void notifyDeviceRestored(D3D11Device* device);

        DXGI_FORMAT _getBasicFormat()                           { return DXGI_FORMAT_R8G8B8A8_UNORM; } // be compatible with pre-Win8 D3D11
        virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice);

        /// Indicate that fullscreen / windowed switching has finished
        void _finishSwitchingFullscreen();
        void setActive(bool state);

    protected:
        HWND                    mHWnd;                          // Win32 window handle
        DWORD                   mWindowedWinStyle;              // Windowed mode window style flags.
        DWORD                   mFullscreenWinStyle;            // Fullscreen mode window style flags.
        unsigned int            mDesiredWidth;                  // Desired width after resizing
        unsigned int            mDesiredHeight;                 // Desired height after resizing
        int                     mLastSwitchingFullscreenCounter;// the last value of the switching fullscreen counter when we switched
    };

#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    
    class _OgreD3D11Export D3D11RenderWindowCoreWindow
        : public D3D11RenderWindowSwapChainBased
    {
    public:
        D3D11RenderWindowCoreWindow(D3D11Device& device);
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

#if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
    class _OgreD3D11Export D3D11RenderWindowSwapChainPanel
        : public D3D11RenderWindowSwapChainBased
    {
    public:
        D3D11RenderWindowSwapChainPanel(D3D11Device& device);
        ~D3D11RenderWindowSwapChainPanel()                      { destroy(); }

        virtual float getViewPointToPixelScale();
        virtual void create(const String& name, unsigned widthPt, unsigned heightPt, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        Windows::UI::Xaml::Controls::SwapChainPanel^ getSwapChainPanel() const    { return mSwapChainPanel; }

        bool isVisible() const;

        // Method for dealing with resize / move & 3d library
        void windowMovedOrResized();

    protected:
        virtual HRESULT _createSwapChainImpl(IDXGIDeviceN* pDXGIDevice);
        HRESULT _compensateSwapChainCompositionScale();

    protected:
        Windows::UI::Xaml::Controls::SwapChainPanel^ mSwapChainPanel;
        Windows::Foundation::Size mCompositionScale;
        Windows::Foundation::EventRegistrationToken sizeChangedToken, compositionScaleChangedToken;
    };
#endif

#if !__OGRE_WINRT_PHONE_80

    class _OgreD3D11Export D3D11RenderWindowImageSource
        : public D3D11RenderWindowBase
    {
    public:
        D3D11RenderWindowImageSource(D3D11Device& device);
        ~D3D11RenderWindowImageSource()                         { destroy(); }
        virtual void create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams);
        virtual void destroy(void);

        virtual void resize(unsigned int width, unsigned int height);
        virtual void update(bool swapBuffers = true);
        virtual void swapBuffers();

        virtual bool isVisible() const                          { return mImageSourceNative.Get() != NULL; }

        Windows::UI::Xaml::Media::ImageBrush^ getImageBrush() const { return mBrush; }
        virtual void getCustomAttribute( const String& name, void* pData ); // "ImageBrush" -> Windows::UI::Xaml::Media::ImageBrush^

    protected:
        void notifyDeviceLost(D3D11Device* device);
        void notifyDeviceRestored(D3D11Device* device);
        void _createSizeDependedD3DResources();                 // creates mpBackBuffer and optionally mpBackBufferNoMSAA

    protected:
        Windows::UI::Xaml::Media::ImageBrush^                   mBrush;             // size independed
        Windows::UI::Xaml::Media::Imaging::SurfaceImageSource^  mImageSource;       // size depended, can be NULL
        ComPtr<ISurfaceImageSourceNative>                       mImageSourceNative; // size depended, can be NULL
    };
#endif // !__OGRE_WINRT_PHONE_80

#endif

}
#endif
