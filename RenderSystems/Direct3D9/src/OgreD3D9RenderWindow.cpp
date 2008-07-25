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
#include "OgreD3D9RenderWindow.h"
#include "OgreLogManager.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"

#include <d3d9.h>
#include "OgreRoot.h"
#include "OgreWindowEventUtilities.h"

namespace Ogre
{
	D3D9RenderWindow::D3D9RenderWindow(HINSTANCE instance, D3D9Driver *driver, LPDIRECT3DDEVICE9 deviceIfSwapChain)
        : mInstance(instance)
        , mDriver(driver)
        , mpRenderSurface(0)
		, mpRenderZBuffer(0)
	{
		mIsFullScreen = false;
		mIsSwapChain = (deviceIfSwapChain != NULL);
		mIsExternal = false;
		mHWnd = 0;
		mActive = false;
		mSizing = false;
		mClosed = false;
		mSwitchingFullscreen = false;
		mDisplayFrequency = 0;
	}

	D3D9RenderWindow::~D3D9RenderWindow()
	{
		destroy();
	}

	bool D3D9RenderWindow::_checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen)
	{
		LPDIRECT3D9 pD3D = mDriver->getD3D();

		if (SUCCEEDED(pD3D->CheckDeviceMultiSampleType(
			adapterNum, 
			deviceType, format, 
			fullScreen, type, outQuality)))
			return true;
		else
			return false;
	}

	void D3D9RenderWindow::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		HINSTANCE hInst = mInstance;
		D3D9Driver* driver = mDriver;

		HWND parentHWnd = 0;
		HWND externalHandle = 0;
		mFSAAType = D3DMULTISAMPLE_NONE;
		mFSAAQuality = 0;
		mVSync = false;
		String title = name;
		unsigned int colourDepth = 32;
		int left = -1; // Defaults to screen center
		int top = -1; // Defaults to screen center
		bool depthBuffer = true;
		String border = "";
		bool outerSize = false;
		mUseNVPerfHUD = false;

		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			// left (x)
			opt = miscParams->find("left");
			if(opt != miscParams->end())
				left = StringConverter::parseInt(opt->second);
			// top (y)
			opt = miscParams->find("top");
			if(opt != miscParams->end())
				top = StringConverter::parseInt(opt->second);
			// Window title
			opt = miscParams->find("title");
			if(opt != miscParams->end())
				title = opt->second;
			// parentWindowHandle		-> parentHWnd
			opt = miscParams->find("parentWindowHandle");
			if(opt != miscParams->end())
				parentHWnd = (HWND)StringConverter::parseUnsignedInt(opt->second);
			// externalWindowHandle		-> externalHandle
			opt = miscParams->find("externalWindowHandle");
			if(opt != miscParams->end())
				externalHandle = (HWND)StringConverter::parseUnsignedInt(opt->second);
			// vsync	[parseBool]
			opt = miscParams->find("vsync");
			if(opt != miscParams->end())
				mVSync = StringConverter::parseBool(opt->second);
			// displayFrequency
			opt = miscParams->find("displayFrequency");
			if(opt != miscParams->end())
				mDisplayFrequency = StringConverter::parseUnsignedInt(opt->second);
			// colourDepth
			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end())
				colourDepth = StringConverter::parseUnsignedInt(opt->second);
			// depthBuffer [parseBool]
			opt = miscParams->find("depthBuffer");
			if(opt != miscParams->end())
				depthBuffer = StringConverter::parseBool(opt->second);
			// FSAA type
			opt = miscParams->find("FSAA");
			if(opt != miscParams->end())
			{
				mFSAA = StringConverter::parseUnsignedInt(opt->second);
				mFSAAType = (D3DMULTISAMPLE_TYPE)mFSAA;
			}
				
			// FSAA quality
			opt = miscParams->find("FSAAQuality");
			if(opt != miscParams->end())
				mFSAAQuality = StringConverter::parseUnsignedInt(opt->second);
			// window border style
			opt = miscParams->find("border");
			if(opt != miscParams->end())
				border = opt->second;
			// set outer dimensions?
			opt = miscParams->find("outerDimensions");
			if(opt != miscParams->end())
				outerSize = StringConverter::parseBool(opt->second);
			// NV perf HUD?
			opt = miscParams->find("useNVPerfHUD");
			if(opt != miscParams->end())
				mUseNVPerfHUD = StringConverter::parseBool(opt->second);
			// sRGB?
			opt = miscParams->find("gamma");
			if(opt != miscParams->end())
				mHwGamma = StringConverter::parseBool(opt->second);

			 
		}

		// Destroy current window if any
		if( mHWnd )
			destroy();

		if (!externalHandle)
		{
			DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;
			RECT rc;

			mWidth = width;
			mHeight = height;
			mTop = top;
			mLeft = left;

			if (!fullScreen)
			{
				if (parentHWnd)
				{
					dwStyle |= WS_CHILD;
				}
				else
				{
					if (border == "none")
						dwStyle |= WS_POPUP;
					else if (border == "fixed")
						dwStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
						WS_SYSMENU | WS_MINIMIZEBOX;
					else
						dwStyle |= WS_OVERLAPPEDWINDOW;
				}

				if (!outerSize)
				{
					// Calculate window dimensions required
					// to get the requested client area
					SetRect(&rc, 0, 0, mWidth, mHeight);
					AdjustWindowRect(&rc, dwStyle, false);
					mWidth = rc.right - rc.left;
					mHeight = rc.bottom - rc.top;

					// Clamp width and height to the desktop dimensions
					int screenw = GetSystemMetrics(SM_CXSCREEN);
					int screenh = GetSystemMetrics(SM_CYSCREEN);
					if ((int)mWidth > screenw)
						mWidth = screenw;
					if ((int)mHeight > screenh)
						mHeight = screenh;
					if (mLeft < 0)
						mLeft = (screenw - mWidth) / 2;
					if (mTop < 0)
						mTop = (screenh - mHeight) / 2;
				}
			}
			else
			{
				dwStyle |= WS_POPUP;
				mTop = mLeft = 0;
			}

			// Register the window class
			// NB allow 4 bytes of window data for D3D9RenderWindow pointer
			WNDCLASS wc = { 0, WindowEventUtilities::_WndProc, 0, 0, hInst,
				LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "OgreD3D9Wnd" };
			RegisterClass(&wc);

			// Create our main window
			// Pass pointer to self
			mIsExternal = false;
			mHWnd = CreateWindow("OgreD3D9Wnd", title.c_str(), dwStyle,
				mLeft, mTop, mWidth, mHeight, parentHWnd, 0, hInst, this);
			
			WindowEventUtilities::_addRenderWindow(this);
		}
		else
		{
			mHWnd = externalHandle;
			mIsExternal = true;
		}

		RECT rc;
		// top and left represent outer window coordinates
		GetWindowRect(mHWnd, &rc);
		mTop = rc.top;
		mLeft = rc.left;
		// width and height represent interior drawable area
		GetClientRect(mHWnd, &rc);
		mWidth = rc.right;
		mHeight = rc.bottom;

		mName = name;
		mIsDepthBuffered = depthBuffer;
		mIsFullScreen = fullScreen;
		mColourDepth = colourDepth;

		LogManager::getSingleton().stream()
			<< "D3D9 : Created D3D9 Rendering Window '"
			<< mName << "' : " << mWidth << "x" << mHeight 
			<< ", " << mColourDepth << "bpp";

		createD3DResources();

		mActive = true;
		mClosed = false;
	}

	void D3D9RenderWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
	{
		if (fullScreen != mIsFullScreen || width != mWidth || height != mHeight)
		{

			if (fullScreen != mIsFullScreen)
				mSwitchingFullscreen = true;

			DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;

			bool oldFullscreen = mIsFullScreen;
			mIsFullScreen = fullScreen;

			if (fullScreen)
			{
				dwStyle |= WS_POPUP;
				mTop = mLeft = 0;
				mWidth = width;
				mHeight = height;
				// need different ordering here

				if (oldFullscreen)
				{
					// was previously fullscreen, just changing the resolution
					SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE);
				}
				else
				{
					SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE);
					//MoveWindow(mHWnd, mLeft, mTop, mWidth, mHeight, FALSE);
					SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
					SetWindowPos(mHWnd, 0, 0,0, 0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			else
			{
				dwStyle |= WS_OVERLAPPEDWINDOW;
				// Calculate window dimensions required
				// to get the requested client area
				RECT rc;
				SetRect(&rc, 0, 0, width, height);
				AdjustWindowRect(&rc, dwStyle, false);
				unsigned int winWidth = rc.right - rc.left;
				unsigned int winHeight = rc.bottom - rc.top;

				SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
				SetWindowPos(mHWnd, HWND_NOTOPMOST, 0, 0, winWidth, winHeight,
					SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOACTIVATE);
				// Note that we also set the position in the restoreLostDevice method
				// via _finishSwitchingFullScreen
			}

			md3dpp.Windowed = !fullScreen;
			md3dpp.FullScreen_RefreshRateInHz = mIsFullScreen ? mDisplayFrequency : 0;
			md3dpp.BackBufferHeight = height;
			md3dpp.BackBufferWidth = width;

			if ((oldFullscreen && fullScreen) || mIsExternal)
			{
				// Have to release & trigger device reset
				// NB don't use windowMovedOrResized since Win32 doesn't know
				// about the size change yet
				SAFE_RELEASE(mpRenderSurface);
				static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem())->_notifyDeviceLost();
				// Notify viewports of resize
				ViewportList::iterator it = mViewportList.begin();
				while(it != mViewportList.end()) (*it++).second->_updateDimensions();
			}
		}
	} 

	void D3D9RenderWindow::_finishSwitchingFullscreen()
	{
		if(mIsFullScreen)
		{
			// Need to reset the region on the window sometimes, when the 
			// windowed mode was constrained by desktop 
			HRGN hRgn = CreateRectRgn(0,0,md3dpp.BackBufferWidth, md3dpp.BackBufferHeight);
			SetWindowRgn(mHWnd, hRgn, FALSE);
		}
		else
		{
			// When switching back to windowed mode, need to reset window size 
			// after device has been restored
			RECT rc;
			SetRect(&rc, 0, 0, md3dpp.BackBufferWidth, md3dpp.BackBufferHeight);
			AdjustWindowRect(&rc, GetWindowLong(mHWnd, GWL_STYLE), false);
			unsigned int winWidth = rc.right - rc.left;
			unsigned int winHeight = rc.bottom - rc.top;
			int screenw = GetSystemMetrics(SM_CXSCREEN);
			int screenh = GetSystemMetrics(SM_CYSCREEN);
			int left = (screenw - winWidth) / 2;
			int top = (screenh - winHeight) / 2;
			SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
				SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

		}
		mSwitchingFullscreen = false;
	}

	void D3D9RenderWindow::createD3DResources(void)
	{
		// access device via driver
		LPDIRECT3DDEVICE9 mpD3DDevice = mDriver->getD3DDevice();

		if (mIsSwapChain && !mpD3DDevice)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
				"Secondary window has not been given the device from the primary!",
				"D3D9RenderWindow::createD3DResources");
		}

		SAFE_RELEASE(mpRenderSurface);

		// Set up the presentation parameters
		HRESULT hr;
		LPDIRECT3D9 pD3D = mDriver->getD3D();
		D3DDEVTYPE devType = D3DDEVTYPE_HAL;

		ZeroMemory( &md3dpp, sizeof(D3DPRESENT_PARAMETERS) );
		md3dpp.Windowed					= !mIsFullScreen;
		md3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
		// triple buffer if VSync is on
		md3dpp.BackBufferCount			= mVSync ? 2 : 1;
		md3dpp.EnableAutoDepthStencil	= mIsDepthBuffered;
		md3dpp.hDeviceWindow			= mHWnd;
		md3dpp.BackBufferWidth			= mWidth;
		md3dpp.BackBufferHeight			= mHeight;
		md3dpp.FullScreen_RefreshRateInHz = mIsFullScreen ? mDisplayFrequency : 0;
		

		if (mVSync)
		{
			md3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
		else
		{
			// NB not using vsync in windowed mode in D3D9 can cause jerking at low 
			// frame rates no matter what buffering modes are used (odd - perhaps a
			// timer issue in D3D9 since GL doesn't suffer from this) 
			// low is < 200fps in this context
			if (!mIsFullScreen)
			{
				LogManager::getSingleton().logMessage("D3D9 : WARNING - "
					"disabling VSync in windowed mode can cause timing issues at lower "
					"frame rates, turn VSync on if you observe this problem.");
			}
			md3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		md3dpp.BackBufferFormat		= D3DFMT_R5G6B5;
		if( mColourDepth > 16 )
			md3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

		if (mColourDepth > 16 )
		{
			// Try to create a 32-bit depth, 8-bit stencil
			if( FAILED( pD3D->CheckDeviceFormat(mDriver->getAdapterNumber(),
				devType,  md3dpp.BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
				D3DRTYPE_SURFACE, D3DFMT_D24S8 )))
			{
				// Bugger, no 8-bit hardware stencil, just try 32-bit zbuffer 
				if( FAILED( pD3D->CheckDeviceFormat(mDriver->getAdapterNumber(),
					devType,  md3dpp.BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
					D3DRTYPE_SURFACE, D3DFMT_D32 )))
				{
					// Jeez, what a naff card. Fall back on 16-bit depth buffering
					md3dpp.AutoDepthStencilFormat = D3DFMT_D16;
				}
				else
					md3dpp.AutoDepthStencilFormat = D3DFMT_D32;
			}
			else
			{
				// Woohoo!
				if( SUCCEEDED( pD3D->CheckDepthStencilMatch( mDriver->getAdapterNumber(), devType,
					md3dpp.BackBufferFormat, md3dpp.BackBufferFormat, D3DFMT_D24S8 ) ) )
				{
					md3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; 
				} 
				else 
					md3dpp.AutoDepthStencilFormat = D3DFMT_D24X8; 
			}
		}
		else
			// 16-bit depth, software stencil
			md3dpp.AutoDepthStencilFormat	= D3DFMT_D16;

		md3dpp.MultiSampleType = mFSAAType;
		md3dpp.MultiSampleQuality = (mFSAAQuality == 0) ? 0 : mFSAAQuality;

		// Check sRGB
		if (mHwGamma)
		{
			/* hmm, this never succeeds even when device does support??
			if(FAILED(pD3D->CheckDeviceFormat(mDriver->getAdapterNumber(),
				devType, md3dpp.BackBufferFormat, D3DUSAGE_QUERY_SRGBWRITE, 
				D3DRTYPE_SURFACE, md3dpp.BackBufferFormat )))
			{
				// disable - not supported
				mHwGamma = false;
			}
			*/

		}


		if (mIsSwapChain)
		{
			// Create swap chain			
			hr = mpD3DDevice->CreateAdditionalSwapChain(
				&md3dpp, &mpSwapChain);
			if (FAILED(hr))
			{
				// Try a second time, may fail the first time due to back buffer count,
				// which will be corrected by the runtime
				hr = mpD3DDevice->CreateAdditionalSwapChain(
					&md3dpp, &mpSwapChain);
			}
			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create an additional swap chain",
					"D3D9RenderWindow::createD3DResources");
			}
			// Store references to buffers for convenience
			SAFE_RELEASE(mpRenderSurface);
			mpSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &mpRenderSurface );
			// Additional swap chains need their own depth buffer
			// to support resizing them
			if (mIsDepthBuffered) 
			{
				SAFE_RELEASE(mpRenderZBuffer);
				hr = mpD3DDevice->CreateDepthStencilSurface(
					mWidth, mHeight,
					md3dpp.AutoDepthStencilFormat,
					md3dpp.MultiSampleType,
					md3dpp.MultiSampleQuality, 
					(md3dpp.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL),
					&mpRenderZBuffer, NULL
					);

				if (FAILED(hr)) 
				{
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"Unable to create a depth buffer for the swap chain",
						"D3D9RenderWindow::createD3DResources");

				}
			} 
			else 
			{
				mpRenderZBuffer = 0;
			}
		}
		else
		{
			if (!mpD3DDevice)
			{
				// We haven't created the device yet, this must be the first time

				// Do we want to preserve the FPU mode? Might be useful for scientific apps
				DWORD extraFlags = 0;
				ConfigOptionMap& options = Root::getSingleton().getRenderSystem()->getConfigOptions();
				ConfigOptionMap::iterator opti = options.find("Floating-point mode");
				if (opti != options.end() && opti->second.currentValue == "Consistent")
					extraFlags |= D3DCREATE_FPU_PRESERVE;

#if OGRE_THREAD_SUPPORT == 1
				extraFlags |= D3DCREATE_MULTITHREADED;
#endif
				// Set default settings (use the one Ogre discovered as a default)
				UINT adapterToUse = mDriver->getAdapterNumber();

				if (mUseNVPerfHUD)
				{
					// Look for 'NVIDIA NVPerfHUD' adapter (<= v4)
					// or 'NVIDIA PerfHUD' (v5)
					// If it is present, override default settings
					for (UINT adapter=0; adapter < mDriver->getD3D()->GetAdapterCount(); ++adapter)
					{
						D3DADAPTER_IDENTIFIER9 identifier;
						HRESULT res;
						res = mDriver->getD3D()->GetAdapterIdentifier(adapter,0,&identifier);
						if (strstr(identifier.Description,"PerfHUD") != 0)
						{
							adapterToUse = adapter;
							devType = D3DDEVTYPE_REF;
							break;
						}
					}
				}

				hr = pD3D->CreateDevice(adapterToUse, devType, mHWnd,
					D3DCREATE_HARDWARE_VERTEXPROCESSING | extraFlags, &md3dpp, &mpD3DDevice );
				if (FAILED(hr))
				{
					// Try a second time, may fail the first time due to back buffer count,
					// which will be corrected down to 1 by the runtime
					hr = pD3D->CreateDevice( adapterToUse, devType, mHWnd,
						D3DCREATE_HARDWARE_VERTEXPROCESSING | extraFlags, &md3dpp, &mpD3DDevice );
				}
				if( FAILED( hr ) )
				{
					hr = pD3D->CreateDevice( adapterToUse, devType, mHWnd,
						D3DCREATE_MIXED_VERTEXPROCESSING | extraFlags, &md3dpp, &mpD3DDevice );
					if( FAILED( hr ) )
					{
						hr = pD3D->CreateDevice( adapterToUse, devType, mHWnd,
							D3DCREATE_SOFTWARE_VERTEXPROCESSING | extraFlags, &md3dpp, &mpD3DDevice );
					}
				}
				// TODO: make this a bit better e.g. go from pure vertex processing to software
				if( FAILED( hr ) )
				{
					destroy();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"Failed to create Direct3D9 Device: " + 
						Root::getSingleton().getErrorDescription(hr), 
						"D3D9RenderWindow::createD3DResources" );
				}
			}
			// update device in driver
			mDriver->setD3DDevice( mpD3DDevice );
			// Store references to buffers for convenience
			SAFE_RELEASE(mpRenderSurface);
			mpD3DDevice->GetRenderTarget( 0, &mpRenderSurface );
			SAFE_RELEASE(mpRenderZBuffer);
			mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
			// release immediately so we don't hog them
			mpRenderZBuffer->Release();
		}

	}

	void D3D9RenderWindow::destroyD3DResources()
	{
		if (mIsSwapChain)
		{
			SAFE_RELEASE(mpRenderZBuffer);
			SAFE_RELEASE(mpSwapChain);
		}
		else
		{
			// ignore depth buffer, access device through driver
			mpRenderZBuffer = 0;
		}
		SAFE_RELEASE(mpRenderSurface);
	}

	void D3D9RenderWindow::destroy()
	{
		destroyD3DResources();

		if (mHWnd && !mIsExternal)
		{
			WindowEventUtilities::_removeRenderWindow(this);
			DestroyWindow(mHWnd);
		}

		mHWnd = 0;
		mActive = false;
		mClosed = true;
	}

	bool D3D9RenderWindow::isVisible() const
	{
		return (mHWnd && !IsIconic(mHWnd));
	}

	void D3D9RenderWindow::reposition(int top, int left)
	{
		if (mHWnd && !mIsFullScreen)
		{
			SetWindowPos(mHWnd, 0, top, left, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	void D3D9RenderWindow::resize(unsigned int width, unsigned int height)
	{
		if (mHWnd && !mIsFullScreen)
		{
			RECT rc = { 0, 0, width, height };
			AdjustWindowRect(&rc, GetWindowLong(mHWnd, GWL_STYLE), false);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			SetWindowPos(mHWnd, 0, 0, 0, width, height,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	void D3D9RenderWindow::windowMovedOrResized()
	{
		if (!mHWnd || IsIconic(mHWnd))
			return;

		RECT rc;
		// top and left represent outer window position
		GetWindowRect(mHWnd, &rc);
		mTop = rc.top;
		mLeft = rc.left;
		// width and height represent drawable area only
		GetClientRect(mHWnd, &rc);
		unsigned int width = rc.right;
		unsigned int height = rc.bottom;
		if (mWidth == width && mHeight == height)
			return;

		SAFE_RELEASE( mpRenderSurface );

		if (mIsSwapChain) 
		{

			D3DPRESENT_PARAMETERS pp = md3dpp;

			pp.BackBufferWidth = width;
			pp.BackBufferHeight = height;

			SAFE_RELEASE( mpRenderZBuffer );
			SAFE_RELEASE( mpSwapChain );

			HRESULT hr = mDriver->getD3DDevice()->CreateAdditionalSwapChain(
				&pp,
				&mpSwapChain);

			if (FAILED(hr)) 
			{
				LogManager::getSingleton().stream(LML_CRITICAL)
					<< "D3D9RenderWindow: failed to reset device to new dimensions << "
					<< width << " x " << height << ". Trying to recover.";

				// try to recover
				hr = mDriver->getD3DDevice()->CreateAdditionalSwapChain(
					&md3dpp,
					&mpSwapChain);

				if (FAILED(hr))
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Reset window to last size failed", "D3D9RenderWindow::resize" );

			}		
			else 
			{
				md3dpp = pp;

				mWidth = width;
				mHeight = height;
				
				SAFE_RELEASE(mpRenderSurface);
				hr = mpSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &mpRenderSurface);
				SAFE_RELEASE(mpRenderZBuffer);
				hr = mDriver->getD3DDevice()->CreateDepthStencilSurface(
					mWidth, mHeight,
					md3dpp.AutoDepthStencilFormat,
					md3dpp.MultiSampleType,
					md3dpp.MultiSampleQuality, 
					(md3dpp.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL),
					&mpRenderZBuffer, NULL
					);

				if (FAILED(hr)) 
				{
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to create depth stencil surface for Swap Chain", "D3D9RenderWindow::resize" );
				}

			}
		}
		// primary windows must reset the device
		else 
		{
			md3dpp.BackBufferWidth = mWidth = width;
			md3dpp.BackBufferHeight = mHeight = height;
			static_cast<D3D9RenderSystem*>(
				Root::getSingleton().getRenderSystem())->_notifyDeviceLost();
		}

		// Notify viewports of resize
		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
			(*it++).second->_updateDimensions();
	}

	void D3D9RenderWindow::swapBuffers( bool waitForVSync )
	{
		// access device through driver
		LPDIRECT3DDEVICE9 mpD3DDevice = mDriver->getD3DDevice();
		if( mpD3DDevice )
		{
			HRESULT hr;
			if (mIsSwapChain)
			{
				hr = mpSwapChain->Present(NULL, NULL, NULL, NULL, 0);
			}
			else
			{
				hr = mpD3DDevice->Present( NULL, NULL, 0, NULL );
			}
			if( D3DERR_DEVICELOST == hr )
			{
				SAFE_RELEASE(mpRenderSurface);

				static_cast<D3D9RenderSystem*>(
					Root::getSingleton().getRenderSystem())->_notifyDeviceLost();
			}
			else if( FAILED(hr) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error Presenting surfaces", "D3D9RenderWindow::swapBuffers" );
		}
	}

	void D3D9RenderWindow::getCustomAttribute( const String& name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// WINDOW				: getWindowHandle

		if( name == "D3DDEVICE" )
		{
			LPDIRECT3DDEVICE9 *pDev = (LPDIRECT3DDEVICE9*)pData;
			*pDev = getD3DDevice();
			return;
		}
		else if( name == "WINDOW" )
		{
			HWND *pHwnd = (HWND*)pData;
			*pHwnd = getWindowHandle();
			return;
		}
		else if( name == "isTexture" )
		{
			bool *b = reinterpret_cast< bool * >( pData );
			*b = false;

			return;
		}
		else if( name == "D3DZBUFFER" )
		{
			LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
			*pSurf = mpRenderZBuffer;
			return;
		}
		else if( name == "DDBACKBUFFER" )
		{
			LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
			*pSurf = mpRenderSurface;
			return;
		}
		else if( name == "DDFRONTBUFFER" )
		{
			LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
			*pSurf = mpRenderSurface;
			return;
		}
	}

	void D3D9RenderWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		if ((dst.left < 0) || (dst.right > mWidth) ||
			(dst.top < 0) || (dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Invalid box.",
						"D3D9RenderWindow::copyContentsToMemory" );
		}

		HRESULT hr;
		LPDIRECT3DSURFACE9 pSurf = 0, pTempSurf = 0;
		D3DSURFACE_DESC desc;
		D3DLOCKED_RECT lockedRect;

		LPDIRECT3DDEVICE9 mpD3DDevice = mDriver->getD3DDevice();

		if (buffer == FB_AUTO)
		{
			//buffer = mIsFullScreen? FB_FRONT : FB_BACK;
			buffer = FB_FRONT;
		}

		if (buffer == FB_FRONT)
		{
			D3DDISPLAYMODE dm;

			LPDIRECT3DDEVICE9 mpD3DDevice = mDriver->getD3DDevice();

			if (FAILED(hr = mpD3DDevice->GetDisplayMode(0, &dm)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get display mode: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			desc.Width = dm.Width;
			desc.Height = dm.Height;
			desc.Format = D3DFMT_A8R8G8B8;
			if (FAILED(hr = mpD3DDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
																	 desc.Format,
																	 D3DPOOL_SYSTEMMEM,
																	 &pTempSurf,
																	 0)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't create offscreen buffer: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if (FAILED(hr = mpD3DDevice->GetFrontBufferData(0, pTempSurf)))
			{
				SAFE_RELEASE(pTempSurf);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get front buffer: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if(mIsFullScreen)
			{
				if ((dst.left == 0) && (dst.right == mWidth) && (dst.top == 0) && (dst.bottom == mHeight))
				{
					hr = pTempSurf->LockRect(&lockedRect, 0, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
				}
				else
				{
					RECT rect;

					rect.left = (LONG)dst.left;
					rect.right = (LONG)dst.right;
					rect.top = (LONG)dst.top;
					rect.bottom = (LONG)dst.bottom;

					hr = pTempSurf->LockRect(&lockedRect, &rect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
				}
				if (FAILED(hr))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't lock rect: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				} 
			}
			else
			{
				RECT srcRect;
				//GetClientRect(mHWnd, &srcRect);
				srcRect.left = (LONG)dst.left;
				srcRect.top = (LONG)dst.top;
				srcRect.right = (LONG)dst.right;
				srcRect.bottom = (LONG)dst.bottom;
				POINT point;
				point.x = srcRect.left;
				point.y = srcRect.top;
				ClientToScreen(mHWnd, &point);
				srcRect.top = point.y;
				srcRect.left = point.x;
				srcRect.bottom += point.y;
				srcRect.right += point.x;

				desc.Width = srcRect.right - srcRect.left;
				desc.Height = srcRect.bottom - srcRect.top;

				if (FAILED(hr = pTempSurf->LockRect(&lockedRect, &srcRect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK)))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't lock rect: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				} 
			}
		}
		else
		{
			SAFE_RELEASE(pSurf);
			if(FAILED(hr = mpD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurf)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get back buffer: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if(FAILED(hr = pSurf->GetDesc(&desc)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get description: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if (FAILED(hr = mpD3DDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
																	 desc.Format,
																	 D3DPOOL_SYSTEMMEM,
																	 &pTempSurf,
																	 0)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't create offscreen surface: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if (desc.MultiSampleType == D3DMULTISAMPLE_NONE)
			{
				if (FAILED(hr = mpD3DDevice->GetRenderTargetData(pSurf, pTempSurf)))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't get render target data: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}
			}
			else
			{
				LPDIRECT3DSURFACE9 pStretchSurf = 0;

				if (FAILED(hr = mpD3DDevice->CreateRenderTarget(desc.Width, desc.Height,
																desc.Format,
																D3DMULTISAMPLE_NONE,
																0,
																false,
																&pStretchSurf,
																0)))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't create render target: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}

				if (FAILED(hr = mpD3DDevice->StretchRect(pSurf, 0, pStretchSurf, 0, D3DTEXF_NONE)))
				{
					SAFE_RELEASE(pTempSurf);
					SAFE_RELEASE(pStretchSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't stretch rect: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}
				if (FAILED(hr = mpD3DDevice->GetRenderTargetData(pStretchSurf, pTempSurf)))
				{
					SAFE_RELEASE(pTempSurf);
					SAFE_RELEASE(pStretchSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't get render target data: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}
				SAFE_RELEASE(pStretchSurf);
			}

			if ((dst.left == 0) && (dst.right == mWidth) && (dst.top == 0) && (dst.bottom == mHeight))
			{
				hr = pTempSurf->LockRect(&lockedRect, 0, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
			}
			else
			{
				RECT rect;

				rect.left = (LONG)dst.left;
				rect.right = (LONG)dst.right;
				rect.top = (LONG)dst.top;
				rect.bottom = (LONG)dst.bottom;

				hr = pTempSurf->LockRect(&lockedRect, &rect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
			}
			if (FAILED(hr))
			{
				SAFE_RELEASE(pTempSurf);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't lock rect: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}
		}

		PixelFormat format = Ogre::D3D9Mappings::_getPF(desc.Format);

		if (format == PF_UNKNOWN)
		{
			SAFE_RELEASE(pTempSurf);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
						"Unsupported format", "D3D9RenderWindow::copyContentsToMemory");
		}

		PixelBox src(dst.getWidth(), dst.getHeight(), 1, format, lockedRect.pBits);
		src.rowPitch = lockedRect.Pitch / PixelUtil::getNumElemBytes(format);
		src.slicePitch = desc.Height * src.rowPitch;

		PixelUtil::bulkPixelConversion(src, dst);

		SAFE_RELEASE(pTempSurf);
		SAFE_RELEASE(pSurf);
	}
	//-----------------------------------------------------------------------------
	void D3D9RenderWindow::update(bool swap)
	{

		D3D9RenderSystem* rs = static_cast<D3D9RenderSystem*>(
			Root::getSingleton().getRenderSystem());

		// access device through driver
		LPDIRECT3DDEVICE9 mpD3DDevice = mDriver->getD3DDevice();

		if (rs->isDeviceLost())
		{
			// Test the cooperative mode first
			HRESULT hr = mpD3DDevice->TestCooperativeLevel();
			if (hr == D3DERR_DEVICELOST)
			{
				// device lost, and we can't reset
				// can't do anything about it here, wait until we get 
				// D3DERR_DEVICENOTRESET; rendering calls will silently fail until 
				// then (except Present, but we ignore device lost there too)
				SAFE_RELEASE(mpRenderSurface);
				// need to release if swap chain
				if (!mIsSwapChain)
					mpRenderZBuffer = 0;
				else
					SAFE_RELEASE (mpRenderZBuffer);
				Sleep(50);
				return;
			}
			else
			{
				// device lost, and we can reset
				rs->restoreLostDevice();

				// Still lost?
				if (rs->isDeviceLost())
				{
					// Wait a while
					Sleep(50);
					return;
				}

				if (!mIsSwapChain) 
				{
					// re-qeuery buffers
					SAFE_RELEASE(mpRenderSurface);
					mpD3DDevice->GetRenderTarget( 0, &mpRenderSurface );
					SAFE_RELEASE(mpRenderZBuffer);
					mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
					// release immediately so we don't hog them
					mpRenderZBuffer->Release();
				}
				else 
				{
				    // Update dimensions incase changed
		            ViewportList::iterator it = mViewportList.begin();
		            while( it != mViewportList.end() )
			            (*it++).second->_updateDimensions();
					// Actual restoration of surfaces will happen in 
					// D3D9RenderSystem::restoreLostDevice when it calls
					// createD3DResources for each secondary window
				}
			}

		}
		RenderWindow::update(swap);
	}
}
