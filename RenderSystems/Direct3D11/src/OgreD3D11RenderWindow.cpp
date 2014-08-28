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
#include "OgreD3D11RenderWindow.h"
#include "OgreException.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreWindowEventUtilities.h"
#include "OgreD3D11Driver.h"
#include "OgreRoot.h"
#include "OgreD3D11DepthBuffer.h"
#include "OgreD3D11Texture.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	// class D3D11RenderWindowBase
	//---------------------------------------------------------------------
#pragma region D3D11RenderWindowBase
	D3D11RenderWindowBase::D3D11RenderWindowBase(D3D11Device & device, IDXGIFactoryN*	pDXGIFactory)
		: mDevice(device)
		, mpDXGIFactory(pDXGIFactory)
	{
		mIsFullScreen = false;
		mIsExternal = false;
		mActive = false;
		mSizing = false;
		mClosed = false;
		mHidden = false;
		mDisplayFrequency = 0;
		mRenderTargetView = 0;
		mDepthStencilView = 0;
		mpBackBuffer = 0;
	}
    //---------------------------------------------------------------------
    D3D11RenderWindowBase::~D3D11RenderWindowBase()
    {
        destroy();
    }
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		mFSAAType.Count = 1;
		mFSAAType.Quality = 0;
		mFSAA = 0;
		mFSAAHint = "";
		mVSync = false;
		mVSyncInterval = 1;
		
		unsigned int colourDepth = 32;
		bool depthBuffer = true;
		
		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			// vsync	[parseBool]
			opt = miscParams->find("vsync");
			if(opt != miscParams->end())
				mVSync = StringConverter::parseBool(opt->second);
			// vsyncInterval	[parseUnsignedInt]
			opt = miscParams->find("vsyncInterval");
			if(opt != miscParams->end())
				mVSyncInterval = StringConverter::parseUnsignedInt(opt->second);
			// hidden	[parseBool]
			opt = miscParams->find("hidden");
			if(opt != miscParams->end())
				mHidden = StringConverter::parseBool(opt->second);
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
				mFSAA = StringConverter::parseUnsignedInt(opt->second);
			// FSAA quality
			opt = miscParams->find("FSAAHint");
			if(opt != miscParams->end())
				mFSAAHint = opt->second;
			// sRGB?
			opt = miscParams->find("gamma");
			if(opt != miscParams->end())
				mHwGamma = StringConverter::parseBool(opt->second);
		}

		mName = name;
		mDepthBufferPoolId = depthBuffer ? DepthBuffer::POOL_DEFAULT : DepthBuffer::POOL_NO_DEPTH;
		mIsFullScreen = fullScreen;
		mColourDepth = colourDepth;

		mWidth = mHeight = mLeft = mTop = 0;

		mActive = true;
		mClosed = false;
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::_createSizeDependedD3DResources(void)
	{
		assert(mpBackBuffer && !mRenderTargetView && !mDepthStencilView);

		HRESULT hr;

		// get the backbuffer desc
		D3D11_TEXTURE2D_DESC BBDesc;
		mpBackBuffer->GetDesc( &BBDesc );

		// create the render target view
		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
		ZeroMemory( &RTVDesc, sizeof(RTVDesc) );

		RTVDesc.Format = BBDesc.Format;
		RTVDesc.ViewDimension = mFSAA ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;
		hr = mDevice->CreateRenderTargetView( mpBackBuffer, &RTVDesc, &mRenderTargetView );

		if( FAILED(hr) )
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to create rendertagert view\nError Description:" + errorDescription,
				"D3D11RenderWindow::_createSizeDependedD3DResources");
		}


		if( mDepthBufferPoolId != DepthBuffer::POOL_NO_DEPTH )
		{
			// Create depth stencil texture
			ID3D11Texture2D* pDepthStencil = NULL;
			D3D11_TEXTURE2D_DESC descDepth;

			descDepth.Width = BBDesc.Width;
			descDepth.Height = BBDesc.Height;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDepth.SampleDesc.Count = mFSAAType.Count;
			descDepth.SampleDesc.Quality = mFSAAType.Quality;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;

			hr = mDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil );
			if( FAILED(hr) || mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Unable to create depth texture\nError Description:" + errorDescription,
					"D3D11RenderWindow::_createSizeDependedD3DResources");
			}

			// Create the depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			ZeroMemory( &descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );

			descDSV.Format =  descDepth.Format;
			descDSV.ViewDimension = mFSAA ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;
			hr = mDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &mDepthStencilView );

            SAFE_RELEASE(pDepthStencil);
                
			if( FAILED(hr) )
			{
				String errorDescription = mDevice.getErrorDescription(hr);
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
					"Unable to create depth stencil view\nError Description:" + errorDescription,
					"D3D11RenderWindow::_createSizeDependedD3DResources");
			}

			D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
			DepthBuffer *depthBuf = rsys->_addManualDepthBuffer( mDepthStencilView, mWidth, mHeight,
																 mFSAAType.Count, mFSAAType.Quality );

			//Don't forget we want this window to use _this_ depth buffer
			this->attachDepthBuffer( depthBuf );
		} 
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::_destroySizeDependedD3DResources()
	{
		SAFE_RELEASE(mpBackBuffer);
		SAFE_RELEASE(mRenderTargetView);

		// delete manual depth buffer (depth buffer view non-owning wrapper)
		DepthBuffer* depthBuf = this->getDepthBuffer();
		detachDepthBuffer();
		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		rsys->_removeManualDepthBuffer(depthBuf);
		delete depthBuf;

		SAFE_RELEASE(mDepthStencilView);
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::destroy()
	{
		_destroySizeDependedD3DResources();

		mActive = false;
		mClosed = true;
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::_updateViewportsDimensions()
	{
		// Notify viewports of resize
		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
			(*it++).second->_updateDimensions();			
	}
	//---------------------------------------------------------------------
	IDXGIDeviceN* D3D11RenderWindowBase::_queryDxgiDevice()
	{
		if (mDevice.isNull())
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
				"D3D11Device is NULL!",
				"D3D11RenderWindowBase::_queryDxgiDevice");
		}

		IDXGIDeviceN* pDXGIDevice = NULL;
		HRESULT hr = mDevice->QueryInterface( __uuidof(IDXGIDeviceN), (void**)&pDXGIDevice );
		if( FAILED(hr) )
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to query a DXGIDevice",
				"D3D11RenderWindowBase::_queryDxgiDevice");
		}

		return pDXGIDevice;
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::getCustomAttribute( const String& name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// WINDOW				: getWindowHandle

		if( name == "D3DDEVICE" )
		{
			ID3D11DeviceN  **device = (ID3D11DeviceN **)pData;
			*device = mDevice.get();
			return;
		}
		else if( name == "isTexture" )
		{
			bool *b = reinterpret_cast< bool * >( pData );
			*b = false;
			return;
		}
		else if( name == "ID3D11RenderTargetView" )
		{
			*static_cast<ID3D11RenderTargetView**>(pData) = mRenderTargetView;
			return;
		}
		else if( name == "ID3D11Texture2D" )
		{
			ID3D11Texture2D **pBackBuffer = (ID3D11Texture2D**)pData;
			*pBackBuffer = mpBackBuffer;
			return;
		}
		else if( name == "numberOfViews" )
		{
			unsigned int* n = reinterpret_cast<unsigned int*>(pData);
			*n = 1;
			return;
		}
		else if( name == "DDBACKBUFFER" )
		{
			ID3D11Texture2D **ppBackBuffer = (ID3D11Texture2D**) pData;
			ppBackBuffer[0] = NULL;
			return;
		}

		RenderWindow::getCustomAttribute(name, pData);
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowBase::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		if(mpBackBuffer == NULL)
			return;

		// get the backbuffer desc
		D3D11_TEXTURE2D_DESC BBDesc;
		mpBackBuffer->GetDesc( &BBDesc );

        ID3D11Texture2D *backbuffer = NULL;

        if(BBDesc.SampleDesc.Quality > 0)
        {
                D3D11_TEXTURE2D_DESC desc = BBDesc;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.CPUAccessFlags = 0;
                desc.BindFlags = 0;
                desc.SampleDesc.Quality = 0;
                desc.SampleDesc.Count = 1;

                HRESULT hr = mDevice->CreateTexture2D(
                        &desc,
                        NULL,
                        &backbuffer);

                if (FAILED(hr) || mDevice.isError())
                {
                        String errorDescription = mDevice.getErrorDescription(hr);
                        OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                                "Error creating texture\nError Description:" + errorDescription, 
                                "D3D11RenderWindow::copyContentsToMemory" );
                }

                mDevice.GetImmediateContext()->ResolveSubresource(backbuffer, D3D11CalcSubresource(0, 0, 1), mpBackBuffer, D3D11CalcSubresource(0, 0, 1), desc.Format);
        }


		// change the parameters of the texture so we can read it
		BBDesc.Usage = D3D11_USAGE_STAGING;
		BBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		BBDesc.BindFlags = 0;
		BBDesc.SampleDesc.Quality = 0;
        BBDesc.SampleDesc.Count = 1;

		// create a temp buffer to copy to
		ID3D11Texture2D * pTempTexture2D;
		HRESULT hr = mDevice->CreateTexture2D(
                        &BBDesc,
                        NULL,
                        &pTempTexture2D);

        if (FAILED(hr) || mDevice.isError())
        {
                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                        "Error creating texture\nError Description:" + errorDescription, 
                        "D3D11RenderWindow::copyContentsToMemory" );
        }
		// copy the back buffer
		mDevice.GetImmediateContext()->CopyResource(pTempTexture2D, backbuffer != NULL ? backbuffer : mpBackBuffer);

		// map the copied texture
		D3D11_MAPPED_SUBRESOURCE mappedTex2D;
		mDevice.GetImmediateContext()->Map(pTempTexture2D, 0,D3D11_MAP_READ, 0, &mappedTex2D);

		// copy the the texture to the dest
		PixelUtil::bulkPixelConversion(
			PixelBox(mWidth, mHeight, 1, D3D11Mappings::_getPF(BBDesc.Format), mappedTex2D.pData), 
			dst);

		// unmap the temp buffer
		mDevice.GetImmediateContext()->Unmap(pTempTexture2D, 0);

		// Release the temp buffer
		SAFE_RELEASE(pTempTexture2D);
		SAFE_RELEASE(backbuffer);
	}
#pragma endregion
	//---------------------------------------------------------------------
	// class D3D11RenderWindowSwapChainBased
	//---------------------------------------------------------------------
#pragma region D3D11RenderWindowSwapChainBased
	D3D11RenderWindowSwapChainBased::D3D11RenderWindowSwapChainBased(D3D11Device & device, IDXGIFactoryN*	pDXGIFactory)
		: D3D11RenderWindowBase(device, pDXGIFactory)
		, mpSwapChain(NULL)
	{
		ZeroMemory( &mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC_N) );
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowSwapChainBased::destroy()
	{
        if(mIsFullScreen && mpSwapChain != NULL)
        {
            mpSwapChain->SetFullscreenState(false, NULL); // get back from fullscreen
            mIsFullScreen = false;
        }

		SAFE_RELEASE(mpSwapChain);

		D3D11RenderWindowBase::destroy();
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowSwapChainBased::_createSwapChain(void)
	{
		ZeroMemory( &mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC_N) );

		// get the dxgi device
		IDXGIDeviceN* pDXGIDevice = _queryDxgiDevice();
		// here the mSwapChainDesc and mpSwapChain are initialized
		HRESULT hr = _createSwapChainImpl(pDXGIDevice);

		SAFE_RELEASE(pDXGIDevice);

        if (FAILED(hr))
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to create swap chain",
				"D3D11RenderWindowSwapChainBased::_createSwapChain");
		}
	}

	void D3D11RenderWindowSwapChainBased::_createSizeDependedD3DResources()
	{
		// obtain back buffer
		SAFE_RELEASE(mpBackBuffer);

		HRESULT hr = mpSwapChain->GetBuffer( 0,  __uuidof( ID3D11Texture2D ), (LPVOID*)&mpBackBuffer  );
		if( FAILED(hr) )
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to Get Back Buffer for swap chain",
				"D3D11RenderWindow::_createSizeDependedD3DResources");
		}

		// create all other size depended resources
		D3D11RenderWindowBase::_createSizeDependedD3DResources();
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowSwapChainBased::_resizeSwapChainBuffers(unsigned width, unsigned height)
	{
		_destroySizeDependedD3DResources();

		// width and height can be zero to autodetect size, therefore do not rely on them
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		UINT Flags = mIsFullScreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
		mpSwapChain->ResizeBuffers(mSwapChainDesc.BufferCount, width, height, mSwapChainDesc.BufferDesc.Format, Flags);
		mpSwapChain->GetDesc(&mSwapChainDesc);
		mWidth = mSwapChainDesc.BufferDesc.Width;
		mHeight = mSwapChainDesc.BufferDesc.Height;
		mIsFullScreen = (0 == mSwapChainDesc.Windowed); // Alt-Enter together with SetWindowAssociation() can change this state

#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		mpSwapChain->ResizeBuffers(mSwapChainDesc.BufferCount, width, height, mSwapChainDesc.Format, 0);
		mpSwapChain->GetDesc1(&mSwapChainDesc);
		mWidth = mSwapChainDesc.Width;
		mHeight = mSwapChainDesc.Height;
#endif

		_createSizeDependedD3DResources();

		// Notify viewports of resize
		_updateViewportsDimensions();
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowSwapChainBased::swapBuffers( )
	{
		if( !mDevice.isNull() )
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			HRESULT hr = mpSwapChain->Present(mVSync ? mVSyncInterval : 0, 0);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
			HRESULT hr = mpSwapChain->Present(1, 0); // flip presentation model swap chains have another semantic for first parameter
#endif
			if( FAILED(hr) )
				OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, "Error Presenting surfaces", "D3D11RenderWindowSwapChainBased::swapBuffers");
		}
	}
	//---------------------------------------------------------------------
#pragma endregion

#pragma region D3D11RenderWindowHwnd
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	//---------------------------------------------------------------------
	// class D3D11RenderWindowHwnd
	//---------------------------------------------------------------------
	D3D11RenderWindowHwnd::D3D11RenderWindowHwnd(D3D11Device & device, IDXGIFactoryN*	pDXGIFactory)
		: D3D11RenderWindowSwapChainBased(device, pDXGIFactory)
	{
		mHWnd = 0;
		mSwitchingFullscreen = false;
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		D3D11RenderWindowSwapChainBased::create(name, width, height, fullScreen, miscParams);

		HWND parentHWnd = 0;
		HWND externalHandle = 0;
		String title = name;
		int left = -1; // Defaults to screen center
		int top = -1; // Defaults to screen center
		String border = "";
		bool outerSize = false;
		bool enableDoubleClick = false;

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
				parentHWnd = (HWND)StringConverter::parseSizeT(opt->second);
			// externalWindowHandle		-> externalHandle
			opt = miscParams->find("externalWindowHandle");
			if(opt != miscParams->end())
				externalHandle = (HWND)StringConverter::parseSizeT(opt->second);
			// window border style
			opt = miscParams->find("border");
			if(opt != miscParams->end())
				border = opt->second;
			// set outer dimensions?
			opt = miscParams->find("outerDimensions");
			if(opt != miscParams->end())
				outerSize = StringConverter::parseBool(opt->second);
			// enable double click messages
			opt = miscParams->find("enableDoubleClick");
			if(opt != miscParams->end())
				enableDoubleClick = StringConverter::parseBool(opt->second);

		}

		// Destroy current window if any
		if( mHWnd )
			destroy();

		if (!externalHandle)
		{
			DWORD dwStyle = (mHidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN;
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

			UINT classStyle = 0;
			if (enableDoubleClick)
				classStyle |= CS_DBLCLKS;

			HINSTANCE hInst = NULL;

			// Register the window class
			// NB allow 4 bytes of window data for D3D11RenderWindow pointer
			WNDCLASS wc = { classStyle, WindowEventUtilities::_WndProc, 0, 0, hInst,
				LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "OgreD3D11Wnd" };	

 
			RegisterClass(&wc);

			// Create our main window
			// Pass pointer to self
			mIsExternal = false;
			mHWnd = CreateWindow("OgreD3D11Wnd", title.c_str(), dwStyle,
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

		LogManager::getSingleton().stream()
			<< "D3D11 : Created D3D11 Rendering Window '"
			<< mName << "' : " << mWidth << "x" << mHeight 
			<< ", " << mColourDepth << "bpp";

		_createSwapChain();
		_createSizeDependedD3DResources();
		mpDXGIFactory->MakeWindowAssociation(mHWnd, NULL);
		setHidden(mHidden);
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::destroy()
	{
		D3D11RenderWindowSwapChainBased::destroy();

		if (mHWnd && !mIsExternal)
		{
			WindowEventUtilities::_removeRenderWindow(this);
			DestroyWindow(mHWnd);
		}

		mHWnd = NULL;
	}
	//---------------------------------------------------------------------
	HRESULT D3D11RenderWindowHwnd::_createSwapChainImpl(IDXGIDeviceN* pDXGIDevice)
	{
		ZeroMemory( &mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC_N) );
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		mSwapChainDesc.BufferDesc.Width		= mWidth;
		mSwapChainDesc.BufferDesc.Height	= mHeight;
		mSwapChainDesc.BufferDesc.Format	= format;

		mSwapChainDesc.BufferDesc.RefreshRate.Numerator=0;
		mSwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		
		mSwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mSwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		mSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ;
		
		// triple buffer if VSync is on
		mSwapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		mSwapChainDesc.BufferCount			= mVSync ? 2 : 1;
		mSwapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD ;

		mSwapChainDesc.OutputWindow 		= mHWnd;
		mSwapChainDesc.Windowed				= !mIsFullScreen;

		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		rsys->determineFSAASettings(mFSAA, mFSAAHint, format, &mFSAAType);
		mSwapChainDesc.SampleDesc.Count = mFSAAType.Count;
		mSwapChainDesc.SampleDesc.Quality = mFSAAType.Quality;

		if (!mVSync && !mIsFullScreen)
		{
			// NB not using vsync in windowed mode in D3D11 can cause jerking at low 
			// frame rates no matter what buffering modes are used (odd - perhaps a
			// timer issue in D3D11 since GL doesn't suffer from this) 
			// low is < 200fps in this context
			LogManager::getSingleton().logMessage("D3D11 : WARNING - "
				"disabling VSync in windowed mode can cause timing issues at lower "
				"frame rates, turn VSync on if you observe this problem.");
		}

		HRESULT hr;

		// Create swap chain			
		hr = mpDXGIFactory->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, &mpSwapChain);
    
		if (FAILED(hr))
		{
			// Try a second time, may fail the first time due to back buffer count,
			// which will be corrected by the runtime
			hr = mpDXGIFactory->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, &mpSwapChain);
		}

		return hr;
	}
	//---------------------------------------------------------------------
	bool D3D11RenderWindowHwnd::isVisible() const
	{
		return (mHWnd && !IsIconic(mHWnd));
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::setHidden(bool hidden)
	{
		mHidden = hidden;
		if (!mIsExternal)
		{
			if (hidden)
				ShowWindow(mHWnd, SW_HIDE);
			else
				ShowWindow(mHWnd, SW_SHOWNORMAL);
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::reposition(int top, int left)
	{
		if (mHWnd && !mIsFullScreen)
		{
			SetWindowPos(mHWnd, 0, top, left, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::resize(unsigned int width, unsigned int height)
	{
		if (!mIsExternal)
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
		else
			windowMovedOrResized();
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::windowMovedOrResized()
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
		unsigned int width = rc.right - rc.left;
		unsigned int height = rc.bottom - rc.top;

		if (width == 0) 
			width = 1;
		if (height == 0)
			height = 1;

		if (mWidth == width && mHeight == height)
			return;

		_resizeSwapChainBuffers(width, height);
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::getCustomAttribute( const String& name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// WINDOW				: getWindowHandle

		if( name == "WINDOW" )
		{
			HWND *pWnd = (HWND*)pData;
			*pWnd = mHWnd;
			return;
		}

		D3D11RenderWindowSwapChainBased::getCustomAttribute(name, pData);
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
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

			mSwapChainDesc.Windowed = !fullScreen;
			mSwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			mSwapChainDesc.BufferDesc.RefreshRate.Denominator=0;
			mSwapChainDesc.BufferDesc.Height = height;
			mSwapChainDesc.BufferDesc.Width = width;

			if ((oldFullscreen && fullScreen) || mIsExternal)
			{
				// Notify viewports of resize
				_updateViewportsDimensions();
			}
		}
	} 
	//---------------------------------------------------------------------
	void D3D11RenderWindowHwnd::_finishSwitchingFullscreen()
	{
		if(mIsFullScreen)
		{
			// Need to reset the region on the window sometimes, when the 
			// windowed mode was constrained by desktop 
			HRGN hRgn = CreateRectRgn(0,0,mSwapChainDesc.BufferDesc.Width, mSwapChainDesc.BufferDesc.Height);
			SetWindowRgn(mHWnd, hRgn, FALSE);
		}
		else
		{
			// When switching back to windowed mode, need to reset window size 
			// after device has been restored
			RECT rc;
			SetRect(&rc, 0, 0, mSwapChainDesc.BufferDesc.Width, mSwapChainDesc.BufferDesc.Height);
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
    void D3D11RenderWindowHwnd::setActive(bool state)
    {
            if (mHWnd && mpSwapChain && mIsFullScreen)
            {
                    if (state)
                    {
                            ShowWindow(mHWnd, SW_RESTORE);
                            mpSwapChain->SetFullscreenState(mIsFullScreen, NULL);
                    }
                    else
                    {
                            ShowWindow(mHWnd, SW_SHOWMINIMIZED);
                            mpSwapChain->SetFullscreenState(FALSE, NULL);
                    }
            }

            RenderWindow::setActive(state);
    }
#endif
#pragma endregion
#pragma region D3D11RenderWindowCoreWindow
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
	//---------------------------------------------------------------------
	// class D3D11RenderWindowCoreWindow
	//---------------------------------------------------------------------
	D3D11RenderWindowCoreWindow::D3D11RenderWindowCoreWindow(D3D11Device & device, IDXGIFactoryN*	pDXGIFactory)
		: D3D11RenderWindowSwapChainBased(device, pDXGIFactory)
	{
	}
	void D3D11RenderWindowCoreWindow::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		D3D11RenderWindowSwapChainBased::create(name, width, height, fullScreen, miscParams);

		Windows::UI::Core::CoreWindow^ externalHandle = nullptr;

		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			// externalWindowHandle		-> externalHandle
			opt = miscParams->find("externalWindowHandle");
			if(opt != miscParams->end())
				externalHandle = reinterpret_cast<Windows::UI::Core::CoreWindow^>((void*)StringConverter::parseSizeT(opt->second));
		}

		// Reset current window if any
		mCoreWindow = nullptr;

		if (!externalHandle)
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "External window handle is not specified.", "D3D11RenderWindow::create" );
		}
		else
		{
			mCoreWindow = externalHandle;
			mIsExternal = true;
		}

		Windows::Foundation::Rect rc = mCoreWindow->Bounds;
		float scale = Windows::Graphics::Display::DisplayProperties::LogicalDpi / 96;
		mLeft = (int)(rc.X * scale);
		mTop = (int)(rc.Y * scale);
		mWidth = (int)(rc.Width * scale);
		mHeight = (int)(rc.Height * scale);

		LogManager::getSingleton().stream()
			<< "D3D11 : Created D3D11 Rendering Window '"
			<< mName << "' : " << mWidth << "x" << mHeight 
			<< ", " << mColourDepth << "bpp";

		_createSwapChain();
		_createSizeDependedD3DResources();
	}

	//---------------------------------------------------------------------
	void D3D11RenderWindowCoreWindow::destroy()
	{
		D3D11RenderWindowSwapChainBased::destroy();

		if (mCoreWindow.Get() && !mIsExternal)
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Only external window handles are supported."
				, "D3D11RenderWindow::destroy" );
		}

		mCoreWindow = nullptr;
	}
	//---------------------------------------------------------------------
	HRESULT D3D11RenderWindowCoreWindow::_createSwapChainImpl(IDXGIDeviceN* pDXGIDevice)
	{
		DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
		mSwapChainDesc.Width				= 0;									// Use automatic sizing.
		mSwapChainDesc.Height				= 0;
		mSwapChainDesc.Format				= format;
		mSwapChainDesc.Stereo				= false;

		// triple buffer if VSync is on
        mSwapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
#if (OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && (OGRE_WINRT_TARGET_TYPE == PHONE)
		mSwapChainDesc.BufferCount			= 1;									// WP8: One buffer.
		mSwapChainDesc.Scaling				= DXGI_SCALING_STRETCH;					// WP8: Must be stretch scaling mode.
		mSwapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;				// WP8: No swap effect.
#else
		mSwapChainDesc.BufferCount			= 2;									// Use two buffers to enable flip effect.
		mSwapChainDesc.Scaling				= DXGI_SCALING_NONE;					// Otherwise stretch would be used by default.
		mSwapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;		// MS recommends using this swap effect for all applications.
#endif
		mSwapChainDesc.AlphaMode			= DXGI_ALPHA_MODE_UNSPECIFIED;

		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		rsys->determineFSAASettings(mFSAA, mFSAAHint, format, &mFSAAType);
		mSwapChainDesc.SampleDesc.Count = mFSAAType.Count;
		mSwapChainDesc.SampleDesc.Quality = mFSAAType.Quality;

		// Create swap chain
		HRESULT hr = mpDXGIFactory->CreateSwapChainForCoreWindow(pDXGIDevice, reinterpret_cast<IUnknown*>(mCoreWindow.Get()), &mSwapChainDesc, NULL, &mpSwapChain);
    
		if (FAILED(hr))
		{
			// Try a second time, may fail the first time due to back buffer count,
			// which will be corrected by the runtime
			hr = mpDXGIFactory->CreateSwapChainForCoreWindow(pDXGIDevice, reinterpret_cast<IUnknown*>(mCoreWindow.Get()), &mSwapChainDesc, NULL, &mpSwapChain);
		}
		if (FAILED(hr))
			return hr;

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces 
        // latency and ensures that the application will only render after each VSync, minimizing 
        // power consumption.
        hr = pDXGIDevice->SetMaximumFrameLatency(1);
		return hr;
	}
	//---------------------------------------------------------------------
	bool D3D11RenderWindowCoreWindow::isVisible() const
	{
		return (mCoreWindow.Get() && Windows::UI::Core::CoreWindow::GetForCurrentThread() == mCoreWindow.Get());
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowCoreWindow::windowMovedOrResized()
	{
		Windows::Foundation::Rect rc = mCoreWindow->Bounds;
		float scale = Windows::Graphics::Display::DisplayProperties::LogicalDpi / 96;
		mLeft = (int)(rc.X * scale);
		mTop = (int)(rc.Y * scale);
		mWidth = (int)(rc.Width * scale);
		mHeight = (int)(rc.Height * scale);

		_resizeSwapChainBuffers(0, 0);		// pass zero to autodetect size
	}
	//---------------------------------------------------------------------
#endif
#pragma endregion

	//---------------------------------------------------------------------
	// class D3D11RenderWindowImageSource
	//---------------------------------------------------------------------
#pragma region D3D11RenderWindowImageSource
#if (OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && (OGRE_WINRT_TARGET_TYPE == DESKTOP_APP)
	//---------------------------------------------------------------------
	D3D11RenderWindowImageSource::D3D11RenderWindowImageSource(D3D11Device& device, IDXGIFactoryN* pDXGIFactory)
		: D3D11RenderWindowBase(device, pDXGIFactory)
		, mImageSourceNative(NULL)
	{
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::create(const String& name, unsigned width, unsigned height, bool fullScreen, const NameValuePairList *miscParams)
	{
		D3D11RenderWindowBase::create(name, width, height, fullScreen, miscParams);

		mWidth = width;
		mHeight = height;

		bool isOpaque = true;
		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			// isOpaque
			opt = miscParams->find("isOpaque");
			if(opt != miscParams->end())
				isOpaque = StringConverter::parseBool(opt->second);
		}

		// create brush
		// TODO: obtain from miscParams optional placeholder image and set inside the brush till first render???
		mBrush = ref new Windows::UI::Xaml::Media::ImageBrush;

		_createSizeDependedD3DResources();
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::destroy(void)
	{
		D3D11RenderWindowBase::destroy();

		SAFE_RELEASE(mImageSourceNative);
		mImageSource = nullptr;
		mBrush = nullptr;
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::_createSizeDependedD3DResources()
	{
		SAFE_RELEASE(mpBackBuffer);
		SAFE_RELEASE(mImageSourceNative);

		if(mWidth <= 0 || mHeight <= 0)
		{
			mImageSource = nullptr;
			mBrush->ImageSource = nullptr;
			return;
		}

		// create back buffer - ID3D11Texture2D
		D3D11_TEXTURE2D_DESC desc = {0};
		desc.Width = mWidth;
		desc.Height = mHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = mDevice->CreateTexture2D(&desc, NULL, &mpBackBuffer);
		if( FAILED(hr) )
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to Create Back Buffer",
				"D3D11RenderWindowImageSource::_createSizeDependedD3DResources");
		}

		// create front buffer - SurfaceImageSource
		mImageSource = ref new Windows::UI::Xaml::Media::Imaging::SurfaceImageSource(mWidth, mHeight, true);
		reinterpret_cast<IUnknown*>(mImageSource)->QueryInterface(__uuidof(ISurfaceImageSourceNative), (void **)&mImageSourceNative);

		// set DXGI device for the front buffer
		IDXGIDeviceN* pDXGIDevice = _queryDxgiDevice();
		mImageSourceNative->SetDevice(pDXGIDevice);
		SAFE_RELEASE(pDXGIDevice);

		// create all other size depended resources
		D3D11RenderWindowBase::_createSizeDependedD3DResources();

		mBrush->ImageSource = mImageSource;
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::update(bool swapBuffers)
	{
		if(mImageSourceNative == NULL)
			return;

		D3D11RenderWindowBase::update(swapBuffers);
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::swapBuffers()
	{
		if(mImageSourceNative == NULL)
			return;

		IDXGISurface* dxgiSurface = NULL;
		RECT updateRect = { 0, 0, mWidth, mHeight };
		POINT offset = { 0, 0 };

		HRESULT hr = mImageSourceNative->BeginDraw(updateRect, &dxgiSurface, &offset);
		if(hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			return;

		if(FAILED(hr))
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to Get DXGI surface for SurfaceImageSource",
				"D3D11RenderWindowImageSource::swapBuffers");
		}

		ID3D11Texture2D* destTexture = NULL;
		hr = dxgiSurface->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&destTexture);
		SAFE_RELEASE(dxgiSurface);
		if(FAILED(hr))
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Unable to convert DXGI surface to D3D11 texture",
				"D3D11RenderWindowImageSource::swapBuffers");
		}

		mDevice.GetImmediateContext()->CopySubresourceRegion1(destTexture, 0, offset.x, offset.y, 0, mpBackBuffer, 0, NULL, 0);

		hr = mImageSourceNative->EndDraw();

		SAFE_RELEASE(destTexture);

		if( FAILED(hr) )
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
				"Drawing into SurfaceImageSource failed",
				"D3D11RenderWindowImageSource::swapBuffers");
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::resize(unsigned width, unsigned height)
	{
		_destroySizeDependedD3DResources();

		mWidth = width;
		mHeight = height;

		_createSizeDependedD3DResources();

		// Notify viewports of resize
		_updateViewportsDimensions();
	}
	//---------------------------------------------------------------------
	void D3D11RenderWindowImageSource::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "ImageBrush" )
		{
			IUnknown** pUnk = (IUnknown **)pData;
			*pUnk = reinterpret_cast<IUnknown*>(mBrush);
			return;
		}

		D3D11RenderWindowBase::getCustomAttribute(name, pData);
	}
	//---------------------------------------------------------------------
#endif // (OGRE_PLATFORM == OGRE_PLATFORM_WINRT) && (OGRE_WINRT_TARGET_TYPE == DESKTOP_APP)
#pragma endregion
}
