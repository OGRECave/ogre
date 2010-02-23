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
#include "OgreD3D10RenderWindow.h"
#include "OgreException.h"
#include "OgreD3D10RenderSystem.h"
#include "OgreWindowEventUtilities.h"
#include "OgreD3D10Driver.h"
#include "OgreRoot.h"
#include "OgreDepthBuffer.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	D3D10RenderWindow::D3D10RenderWindow(HINSTANCE instance, D3D10Device & device)
		: mInstance(instance)
		, mDevice(device)
		//, mpRenderSurface(0)
	{
		mIsFullScreen = false;
		mIsSwapChain = true;//(deviceIfSwapChain != NULL);
		mIsExternal = false;
		mHWnd = 0;
		mActive = false;
		mSizing = false;
		mClosed = false;
		mSwitchingFullscreen = false;
		mDisplayFrequency = 0;
		mRenderTargetView = 0;
		mDepthStencilView = 0;
		mpBackBuffer = 0;
	}
	//---------------------------------------------------------------------
	D3D10RenderWindow::~D3D10RenderWindow()
	{
		SAFE_RELEASE( mRenderTargetView );
		SAFE_RELEASE( mDepthStencilView );

		mpBackBuffer->Release();
		mpBackBuffer = NULL;

		destroy();
	}
	//---------------------------------------------------------------------
	bool D3D10RenderWindow::_checkMultiSampleQuality(UINT SampleCount, UINT *outQuality, DXGI_FORMAT format)
	{
		//TODO :CheckMultisampleQualityLevels
		if (SUCCEEDED(mDevice->CheckMultisampleQualityLevels(//CheckDeviceMultiSampleType(
			format,
			SampleCount,
			outQuality)))
		{
			return true;
		}
		else
			return false;
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		HINSTANCE hInst = mInstance;
		//D3D10Driver* driver = mDriver;

		HWND parentHWnd = 0;
		HWND externalHandle = 0;
		mFSAAType.Count = 1;
		mFSAAType.Quality = 0;
		mFSAA = 0;
		mFSAAHint = "";
		mVSync = false;
		mVSyncInterval = 1;
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
			// vsyncInterval	[parseUnsignedInt]
			opt = miscParams->find("vsyncInterval");
			if(opt != miscParams->end())
				mVSyncInterval = StringConverter::parseUnsignedInt(opt->second);
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
			}
			// FSAA quality
			opt = miscParams->find("FSAAHint");
			if(opt != miscParams->end())
				mFSAAHint = opt->second;
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
			// NB allow 4 bytes of window data for D3D10RenderWindow pointer
			WNDCLASS wc = { 0, WindowEventUtilities::_WndProc, 0, 0, hInst,
				LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "OgreD3D10Wnd" };
			RegisterClass(&wc);

			// Create our main window
			// Pass pointer to self
			mIsExternal = false;
			mHWnd = CreateWindow("OgreD3D10Wnd", title.c_str(), dwStyle,
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
		mDepthBufferPoolId = depthBuffer ? DepthBuffer::POOL_DEFAULT : DepthBuffer::POOL_NO_DEPTH;
		mIsFullScreen = fullScreen;
		mColourDepth = colourDepth;

		LogManager::getSingleton().stream()
			<< "D3D10 : Created D3D10 Rendering Window '"
			<< mName << "' : " << mWidth << "x" << mHeight 
			<< ", " << mColourDepth << "bpp";

		createD3DResources();

		mActive = true;
		mClosed = false;
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
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
			md3dpp.BufferDesc.RefreshRate.Numerator = 0;
			md3dpp.BufferDesc.RefreshRate.Denominator= 0;
			md3dpp.BufferDesc.Height = height;
			md3dpp.BufferDesc.Width = width;

			if ((oldFullscreen && fullScreen) || mIsExternal)
			{
				// Have to release & trigger device reset
				// NB don't use windowMovedOrResized since Win32 doesn't know
				// about the size change yet
				//	SAFE_RELEASE(mpRenderSurface);
				// Notify viewports of resize
				ViewportList::iterator it = mViewportList.begin();
				while(it != mViewportList.end()) (*it++).second->_updateDimensions();
			}
		}

	} 
	//---------------------------------------------------------------------
	void D3D10RenderWindow::_finishSwitchingFullscreen()
	{
		if(mIsFullScreen)
		{
			// Need to reset the region on the window sometimes, when the 
			// windowed mode was constrained by desktop 
			HRGN hRgn = CreateRectRgn(0,0,md3dpp.BufferDesc.Width, md3dpp.BufferDesc.Height);
			SetWindowRgn(mHWnd, hRgn, FALSE);
		}
		else
		{
			// When switching back to windowed mode, need to reset window size 
			// after device has been restored
			RECT rc;
			SetRect(&rc, 0, 0, md3dpp.BufferDesc.Width, md3dpp.BufferDesc.Height);
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
		mpSwapChain->SetFullscreenState(mIsFullScreen, NULL);
		mSwitchingFullscreen = false;
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::createD3DResources(void)
	{
		if (mIsSwapChain && mDevice.isNull())
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
				"Secondary window has not been given the device from the primary!",
				"D3D10RenderWindow::createD3DResources");
		}

		//SAFE_RELEASE(mpRenderSurface);

		// Set up the presentation parameters
		//		int pD3D = mDriver->getD3D();
		//	D3DDEVTYPE devType = D3DDEVTYPE_HAL;

		ZeroMemory( &md3dpp, sizeof(DXGI_SWAP_CHAIN_DESC) );
		md3dpp.Windowed				= !mIsFullScreen;
		md3dpp.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD ;
		// triple buffer if VSync is on
		md3dpp.BufferCount			= mVSync ? 2 : 1;
		md3dpp.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		md3dpp.OutputWindow 		= mHWnd;
		md3dpp.BufferDesc.Width		= mWidth;
		md3dpp.BufferDesc.Height	= mHeight;
		md3dpp.BufferDesc.RefreshRate.Numerator = 0;
		md3dpp.BufferDesc.RefreshRate.Denominator= 0;
		if (mIsFullScreen)
		{
			md3dpp.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			md3dpp.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			md3dpp.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ;
		}
		md3dpp.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D10RenderSystem* rsys = static_cast<D3D10RenderSystem*>(Root::getSingleton().getRenderSystem());
		rsys->determineFSAASettings(mFSAA, mFSAAHint, md3dpp.BufferDesc.Format, &mFSAAType);


		if (mVSync)
		{
			//	md3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
		else
		{
			// NB not using vsync in windowed mode in D3D10 can cause jerking at low 
			// frame rates no matter what buffering modes are used (odd - perhaps a
			// timer issue in D3D10 since GL doesn't suffer from this) 
			// low is < 200fps in this context
			if (!mIsFullScreen)
			{
				LogManager::getSingleton().logMessage("D3D10 : WARNING - "
					"disabling VSync in windowed mode can cause timing issues at lower "
					"frame rates, turn VSync on if you observe this problem.");
			}
			//	md3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}
		/*
		md3dpp.BufferDesc.Format= BackBufferFormat		= D3DFMT_R5G6B5;
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
		*/
		md3dpp.SampleDesc.Count = mFSAAType.Count;
		md3dpp.SampleDesc.Quality = mFSAAType.Quality;
		if (mIsSwapChain)
		{
			IDXGIFactory*	mpDXGIFactory;
			HRESULT hr;
			hr = CreateDXGIFactory( IID_IDXGIFactory, (void**)&mpDXGIFactory );
			if( FAILED(hr) )
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create a DXGIFactory for the swap chain",
					"D3D10RenderWindow::createD3DResources");
			}

			// get the dxgi device
			IDXGIDevice* pDXGIDevice = NULL;
			hr = mDevice->QueryInterface( IID_IDXGIDevice, (void**)&pDXGIDevice );
			if( FAILED(hr) )
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create a DXGIDevice for the swap chain",
					"D3D10RenderWindow::createD3DResources");
			}

			// Create swap chain			
			hr = mpDXGIFactory->CreateSwapChain( 
				pDXGIDevice,&md3dpp,&mpSwapChain);

			if (FAILED(hr))
			{
				// Try a second time, may fail the first time due to back buffer count,
				// which will be corrected by the runtime
				hr = mpDXGIFactory->CreateSwapChain(pDXGIDevice,&md3dpp,&mpSwapChain);
			}
			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create an additional swap chain",
					"D3D10RenderWindow::createD3DResources");
			}
		
			// Store references to buffers for convenience
			//mpSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &mpRenderSurface );
			// Additional swap chains need their own depth buffer
			// to support resizing them

			hr = mpSwapChain->GetBuffer( 0,  __uuidof( ID3D10Texture2D ), (LPVOID*)&mpBackBuffer  );
			if( FAILED(hr) )
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to Get Back Buffer for swap chain",
					"D3D10RenderWindow::createD3DResources");
			}

			// get the backbuffer desc
			D3D10_TEXTURE2D_DESC BBDesc;
			mpBackBuffer->GetDesc( &BBDesc );

			// create the render target view
			D3D10_RENDER_TARGET_VIEW_DESC RTVDesc;
			ZeroMemory( &RTVDesc, sizeof(RTVDesc) );

			RTVDesc.Format = BBDesc.Format;
			RTVDesc.ViewDimension = mFSAA ? D3D10_RTV_DIMENSION_TEXTURE2DMS : D3D10_RTV_DIMENSION_TEXTURE2D;
			RTVDesc.Texture2D.MipSlice = 0;
			hr = mDevice->CreateRenderTargetView( mpBackBuffer, &RTVDesc, &mRenderTargetView );

			if( FAILED(hr) )
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create rendertagert view\nError Description:" + errorDescription,
					"D3D10RenderWindow::createD3DResources");
			}


			if( mDepthBufferPoolId != DepthBuffer::POOL_NO_DEPTH )
			{
				/*	hr = mDevice->CreateDepthStencilSurface(
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
				"D3D10RenderWindow::createD3DResources");

				}
				*/
				// get the backbuffer

				// Create depth stencil texture
				ID3D10Texture2D* pDepthStencil = NULL;
				D3D10_TEXTURE2D_DESC descDepth;

				descDepth.Width = mWidth;
				descDepth.Height = mHeight;
				descDepth.MipLevels = 1;
				descDepth.ArraySize = 1;
				descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
				descDepth.SampleDesc.Count = mFSAAType.Count;
				descDepth.SampleDesc.Quality = mFSAAType.Quality;
				descDepth.Usage = D3D10_USAGE_DEFAULT;
				descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
				descDepth.CPUAccessFlags = 0;
				descDepth.MiscFlags = 0;

				hr = mDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil );
				if( FAILED(hr) || mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription(hr);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"Unable to create depth texture\nError Description:" + errorDescription,
						"D3D10RenderWindow::createD3DResources");
				}

				// Create the depth stencil view
				D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
				ZeroMemory( &descDSV, sizeof(D3D10_DEPTH_STENCIL_VIEW_DESC) );

				descDSV.Format = DXGI_FORMAT_D32_FLOAT;
				descDSV.ViewDimension = mFSAA ? D3D10_DSV_DIMENSION_TEXTURE2DMS : D3D10_DSV_DIMENSION_TEXTURE2D;
				descDSV.Texture2D.MipSlice = 0;
				hr = mDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &mDepthStencilView );
				SAFE_RELEASE( pDepthStencil );
				if( FAILED(hr) )
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"Unable to create depth stencil view\nError Description:" + errorDescription,
						"D3D10RenderWindow::createD3DResources");
				}

				DepthBuffer *depthBuf = rsys->_addManualDepthBuffer( mDepthStencilView, mWidth, mHeight,
																	 mFSAAType.Count, mFSAAType.Quality );

				//Don't forget we want this window to use _this_ depth buffer
				this->attachDepthBuffer( depthBuf );
			} 
			else 
			{
				//				mpRenderZBuffer = 0;
			}
		}
		/*else
		{
		if (!mDevice)
		{
		// We haven't created the device yet, this must be the first time

		// Do we want to preserve the FPU mode? Might be useful for scientific apps
		DWORD extraFlags = 0;
		ConfigOptionMap& options = Root::getSingleton().getRenderSystem()->getConfigOptions();
		ConfigOptionMap::iterator opti = options.find("Floating-point mode");
		if (opti != options.end() && opti->second.currentValue == "Consistent")
		extraFlags |= D3DCREATE_FPU_PRESERVE;

		#if OGRE_THREAD_SUPPORT
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
		D3DCREATE_HARDWARE_VERTEXPROCESSING | extraFlags, &md3dpp, &mDevice );
		if (FAILED(hr))
		{
		// Try a second time, may fail the first time due to back buffer count,
		// which will be corrected down to 1 by the runtime
		hr = pD3D->CreateDevice( adapterToUse, devType, mHWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | extraFlags, &md3dpp, &mDevice );
		}
		if( FAILED( hr ) )
		{
		hr = pD3D->CreateDevice( adapterToUse, devType, mHWnd,
		D3DCREATE_MIXED_VERTEXPROCESSING | extraFlags, &md3dpp, &mDevice );
		if( FAILED( hr ) )
		{
		hr = pD3D->CreateDevice( adapterToUse, devType, mHWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | extraFlags, &md3dpp, &mDevice );
		}
		}
		// TODO: make this a bit better e.g. go from pure vertex processing to software
		if( FAILED( hr ) )
		{
		destroy();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
		"Failed to create Direct3D9 Device: " + 
		Root::getSingleton().getErrorDescription(hr), 
		"D3D10RenderWindow::createD3DResources" );
		}
		}
		// update device in driver
		mDriver->setD3DDevice( mDevice );
		// Store references to buffers for convenience
		mDevice->GetRenderTarget( 0, &mpRenderSurface );
		mDevice->GetDepthStencilSurface( &mpRenderZBuffer );
		// release immediately so we don't hog them
		mpRenderZBuffer->Release();
		}
		*/
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::destroyD3DResources()
	{
		if (mIsSwapChain)
		{
			//	SAFE_RELEASE(mpRenderZBuffer);
			SAFE_RELEASE(mpSwapChain);
		}
		else
		{
			// ignore depth buffer, access device through driver
			//	mpRenderZBuffer = 0;
		}
		//		SAFE_RELEASE(mpRenderSurface);
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::destroy()
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
	//---------------------------------------------------------------------
	bool D3D10RenderWindow::isVisible() const
	{
		return (mHWnd && !IsIconic(mHWnd));
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::reposition(int top, int left)
	{
		if (mHWnd && !mIsFullScreen)
		{
			SetWindowPos(mHWnd, 0, top, left, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::resize(unsigned int width, unsigned int height)
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
	//---------------------------------------------------------------------
	void D3D10RenderWindow::windowMovedOrResized()
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

		md3dpp.Windowed				= !mIsFullScreen;
		md3dpp.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD ;
		// triple buffer if VSync is on
		md3dpp.BufferCount			= mVSync ? 2 : 1;
		md3dpp.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		md3dpp.OutputWindow 		= mHWnd;
		md3dpp.BufferDesc.Width		= mWidth;
		md3dpp.BufferDesc.Height	= mHeight;
		md3dpp.BufferDesc.RefreshRate.Numerator = 0;
		md3dpp.BufferDesc.RefreshRate.Denominator= 0;

		mWidth = width;
		mHeight = height;


		UINT Flags = 0;
		if( mIsFullScreen )
			Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		mpSwapChain->ResizeBuffers(md3dpp.BufferCount, width, height, md3dpp.BufferDesc.Format, Flags);
		/*
		SAFE_RELEASE( mpRenderSurface );

		if (mIsSwapChain) 
		{

		DXGI_SWAP_CHAIN_DESC pp = md3dpp;

		pp.BufferDesc.Height = width;
		pp.BufferDesc.Height = height;

		//SAFE_RELEASE( mpRenderZBuffer );
		SAFE_RELEASE( mpSwapChain );

		HRESULT hr = mDriver->mDevice->CreateAdditionalSwapChain(
		&pp,
		&mpSwapChain);

		if (FAILED(hr)) 
		{
		LogManager::getSingleton().stream(LML_CRITICAL)
		<< "D3D10RenderWindow: failed to reset device to new dimensions << "
		<< width << " x " << height << ". Trying to recover.";

		// try to recover
		hr = mDriver->mDevice->CreateAdditionalSwapChain(
		&md3dpp,
		&mpSwapChain);

		if (FAILED(hr))
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Reset window to last size failed", "D3D10RenderWindow::resize" );

		}		
		else 
		{
		md3dpp = pp;

		mWidth = width;
		mHeight = height;

		hr = mpSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &mpRenderSurface);
		hr = mDriver->mDevice->CreateDepthStencilSurface(
		mWidth, mHeight,
		md3dpp.AutoDepthStencilFormat,
		md3dpp.MultiSampleType,
		md3dpp.MultiSampleQuality, 
		(md3dpp.Flags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL),
		&mpRenderZBuffer, NULL
		);

		if (FAILED(hr)) 
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to create depth stencil surface for Swap Chain", "D3D10RenderWindow::resize" );
		}

		}
		}
		// primary windows must reset the device
		else 
		{
		md3dpp.BufferDesc.Width = mWidth = width;
		md3dpp.BufferDesc.Height = mHeight = height;
		static_cast<D3D10RenderSystem*>(
		Root::getSingleton().getRenderSystem())->_notifyDeviceLost();
		}

		// Notify viewports of resize
		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
		(*it++).second->_updateDimensions();
		*/
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::swapBuffers( bool waitForVSync )
	{
		if( !mDevice.isNull() )
		{
			HRESULT hr;
			if (mIsSwapChain)
			{
				hr = mpSwapChain->Present(waitForVSync ? mVSyncInterval : 0, 0);
			}
			else
			{
				//hr = mDevice->Present( 0,0);
			}
			/*if( D3DERR_DEVICELOST == hr )
			{
			SAFE_RELEASE(mpRenderSurface);

			static_cast<D3D10RenderSystem*>(
			Root::getSingleton().getRenderSystem())->_notifyDeviceLost();
			}
			else 
			*/
			if( FAILED(hr) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error Presenting surfaces", "D3D10RenderWindow::swapBuffers" );
		}
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::getCustomAttribute( const String& name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// WINDOW				: getWindowHandle

		if( name == "D3DDEVICE" )
		{
			ID3D10Device  **device = (ID3D10Device **)pData;
			*device = mDevice.get();
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
		else if( name == "ID3D10RenderTargetView" )
		{
			ID3D10RenderTargetView * *pRTView = (ID3D10RenderTargetView **)pData;
			*pRTView = mRenderTargetView;
			return;
		}
		else if( name == "ID3D10Texture2D" )
		{
			ID3D10Texture2D **pBackBuffer = (ID3D10Texture2D**)pData;
			*pBackBuffer = mpBackBuffer;
		}

		/*else if( name == "D3DZBUFFER" )
		{
		IDXGISurface * *pSurf = (IDXGISurface **)pData;
		*pSurf = mpRenderZBuffer;
		return;
		}
		else if( name == "DDBACKBUFFER" )
		{
		IDXGISurface * *pSurf = (IDXGISurface **)pData;
		*pSurf = mpRenderSurface;
		return;
		}
		else if( name == "DDFRONTBUFFER" )
		{
		IDXGISurface * *pSurf = (IDXGISurface **)pData;
		*pSurf = mpRenderSurface;
		return;
		}
		*/
	}
	//---------------------------------------------------------------------
	void D3D10RenderWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		

		// get the backbuffer desc
		D3D10_TEXTURE2D_DESC BBDesc;
		mpBackBuffer->GetDesc( &BBDesc );

		// change the parameters of the texture so we can read it
		BBDesc.Usage = D3D10_USAGE_STAGING;
		BBDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
		BBDesc.BindFlags = 0;

		// create a temp buffer to copy to
		ID3D10Texture2D * pTempTexture2D;
		mDevice->CreateTexture2D(
			&BBDesc,
			0,
			&pTempTexture2D);

		// copy the back buffer
		mDevice->CopyResource(pTempTexture2D, mpBackBuffer);


		// map the copied texture
		D3D10_MAPPED_TEXTURE2D mappedTex2D;
		pTempTexture2D->Map(0,D3D10_MAP_READ, 0, &mappedTex2D);

		// copy the the texture to the dest
		PixelUtil::bulkPixelConversion(
			PixelBox(mWidth, mHeight, 1, PF_A8B8G8R8, mappedTex2D.pData), 
			dst);

		// unmap the temp buffer
		pTempTexture2D->Unmap(0);

		// Release the temp buffer
		pTempTexture2D->Release();
		pTempTexture2D = NULL;
	}
	//-----------------------------------------------------------------------------
	void D3D10RenderWindow::update(bool swap)
	{

		/*D3D10RenderSystem* rs = static_cast<D3D10RenderSystem*>(
		Root::getSingleton().getRenderSystem());

		// access device through driver
		D3D10Device & mDevice = mDriver->mDevice;

		if (rs->isDeviceLost())
		{
		// Test the cooperative mode first
		HRESULT hr = mDevice->TestCooperativeLevel();
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
		mDevice->GetRenderTarget( 0, &mpRenderSurface );
		mDevice->GetDepthStencilSurface( &mpRenderZBuffer );
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
		// D3D10RenderSystem::restoreLostDevice when it calls
		// createD3DResources for each secondary window
		}
		}

		}
		*/
		RenderWindow::update(swap);
	}
	//---------------------------------------------------------------------
	bool D3D10RenderWindow::requiresTextureFlipping() const
	{
		return false;
	}
	//---------------------------------------------------------------------
	HWND D3D10RenderWindow::getWindowHandle() const
	{
		return mHWnd;
	}
	//---------------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC* D3D10RenderWindow::getPresentationParameters( void )
	{
		return &md3dpp;
	}
	//---------------------------------------------------------------------
	bool D3D10RenderWindow::_getSwitchingFullscreen() const
	{
		return mSwitchingFullscreen;
	}
	//---------------------------------------------------------------------
}
