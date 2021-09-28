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
#include "OgreD3D11Driver.h"
#include "OgreRoot.h"
#include "OgreD3D11DepthBuffer.h"
#include "OgreD3D11Texture.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"
#include "OgreHardwarePixelBuffer.h"
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#include "OgreD3D11StereoDriverBridge.h"
#endif
#include <iomanip>
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT && defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#include <dxgi1_3.h> // for IDXGISwapChain2::SetMatrixTransform used in D3D11RenderWindowSwapChainPanel
#endif

#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
#include <Windows.h>
#include <VersionHelpers.h>
#endif

#define OGRE_D3D11_WIN_CLASS_NAME "OgreD3D11Wnd"

namespace Ogre
{
    //---------------------------------------------------------------------
    // class D3D11RenderWindowBase
    //---------------------------------------------------------------------
#pragma region D3D11RenderWindowBase
    D3D11RenderWindowBase::D3D11RenderWindowBase(D3D11Device & device)
        : mDevice(device)
    {
        mIsFullScreen = false;
        mIsExternal = false;
        mActive = false;
        mSizing = false;
        mHidden = false;
    }
    //---------------------------------------------------------------------
    D3D11RenderWindowBase::~D3D11RenderWindowBase()
    {
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->fireDeviceEvent(&mDevice,"RenderWindowDestroyed",this);

        destroy();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::create(const String& name, unsigned int width, unsigned int height,
        bool fullScreen, const NameValuePairList *miscParams)
    {
        mFSAA = 0;
        mFSAAHint = "";
        mFSAAType.Count = 1;
        mFSAAType.Quality = 0;
        
        unsigned int colourDepth = 32;
        bool depthBuffer = true;
        
        if(miscParams)
        {
            // Get variable-length params
            NameValuePairList::const_iterator opt;
            // hidden   [parseBool]
            opt = miscParams->find("hidden");
            if(opt != miscParams->end())
                mHidden = StringConverter::parseBool(opt->second);
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

		if(mIsFullScreen)
		{
			D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
			rsys->addToSwitchingFullscreenCounter();
		}

        mWidth = mHeight = mLeft = mTop = 0;

        mActive = true;
        mClosed = false;
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::_createSizeDependedD3DResources(void)
    {
        assert(mpBackBuffer && !mRenderTargetView && !mDepthStencilView);

        // get the backbuffer desc
        D3D11_TEXTURE2D_DESC BBDesc;
        mpBackBuffer->GetDesc( &BBDesc );

        // mFSAA is an external request that may be even not supported by hardware, but mFSAAType should be always in sync with reality
        assert(BBDesc.SampleDesc.Count == mFSAAType.Count && BBDesc.SampleDesc.Quality == mFSAAType.Quality);

        // create the render target view
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
        ZeroMemory( &RTVDesc, sizeof(RTVDesc) );

        RTVDesc.Format = _getRenderFormat(); // if BB is from swapchain than RTV format can have extra _SRGB suffix not present in BB format
        RTVDesc.ViewDimension = mFSAAType.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
        OGRE_CHECK_DX_ERROR(mDevice->CreateRenderTargetView(mpBackBuffer.Get(), &RTVDesc,
                                                            mRenderTargetView.ReleaseAndGetAddressOf()));

        if( mDepthBufferPoolId != DepthBuffer::POOL_NO_DEPTH )
        {
            // Create depth stencil texture
            ComPtr<ID3D11Texture2D> pDepthStencil;
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

            OGRE_CHECK_DX_ERROR(
                mDevice->CreateTexture2D(&descDepth, NULL, pDepthStencil.ReleaseAndGetAddressOf()));

            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
            ZeroMemory( &descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );

            descDSV.Format =  descDepth.Format;
            descDSV.ViewDimension = mFSAAType.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            OGRE_CHECK_DX_ERROR(mDevice->CreateDepthStencilView(
                pDepthStencil.Get(), &descDSV, mDepthStencilView.ReleaseAndGetAddressOf()));

            D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
            DepthBuffer *depthBuf = rsys->_addManualDepthBuffer( mDepthStencilView.Get(), mWidth, mHeight,
                                                                 mFSAAType.Count, mFSAAType.Quality );

            //Don't forget we want this window to use _this_ depth buffer
            this->attachDepthBuffer( depthBuf );
        } 
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::_destroySizeDependedD3DResources()
    {
        mpBackBuffer.Reset();
        mpBackBufferNoMSAA.Reset();
        mRenderTargetView.Reset();

        // delete manual depth buffer (depth buffer view non-owning wrapper)
        DepthBuffer* depthBuf = this->getDepthBuffer();
        detachDepthBuffer();
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->_removeManualDepthBuffer(depthBuf);
        delete depthBuf;

        mDepthStencilView.Reset();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::destroy()
    {
        _destroySizeDependedD3DResources();

        mActive = false;
        mClosed = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::updateImpl()
	{
		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		rsys->validateDevice();

		RenderWindow::updateImpl();
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
    void D3D11RenderWindowBase::_queryDxgiDeviceImpl(IDXGIDeviceN** dxgiDevice)
    {
        if (mDevice.isNull())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "D3D11Device is NULL!",
                "D3D11RenderWindowBase::_queryDxgiDevice");
        }

        OGRE_CHECK_DX_ERROR(mDevice->QueryInterface( __uuidof(IDXGIDeviceN), (void**)dxgiDevice ));
    }

    uint D3D11RenderWindowBase::getNumberOfViews() const { return 1; }

    ID3D11Texture2D* D3D11RenderWindowBase::getSurface(uint index) const
    {
        return index == 0 ? mpBackBuffer.Get() : NULL;
    }

    ID3D11RenderTargetView* D3D11RenderWindowBase::getRenderTargetView(uint index) const
    {
        return index == 0 ? mRenderTargetView.Get() : NULL;
    }

    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::getCustomAttribute( const String& name, void* pData )
    {
        // Valid attributes and their equvalent native functions:
        // D3DDEVICE            : getD3DDevice
        // WINDOW               : getWindowHandle

        if (name == "D3DDEVICE")
        {
            *(ID3D11DeviceN**)pData = mDevice.get();
        }
        else
        {
            RenderWindow::getCustomAttribute(name, pData);
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowBase::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        if(src.right > mWidth || src.bottom > mHeight || src.front != 0 || src.back != 1
        || dst.getWidth() != src.getWidth() || dst.getHeight() != src.getHeight() || dst.getDepth() != 1)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid box.", "D3D11RenderWindowBase::copyContentsToMemory");
        }

        if(!mpBackBuffer)
            return;

        // get the backbuffer desc
        D3D11_TEXTURE2D_DESC BBDesc;
        mpBackBuffer->GetDesc( &BBDesc );
        UINT srcSubresource = 0;
        D3D11_BOX srcBoxDx11 = { src.left, src.top, 0, src.right, src.bottom, 1 };

        // We need data from backbuffer without MSAA
        ComPtr<ID3D11Texture2D> backbufferNoMSAA;
        if(BBDesc.SampleDesc.Count == 1)
        {
            backbufferNoMSAA = mpBackBuffer;
        }
        else if(mpBackBufferNoMSAA)
        {
            backbufferNoMSAA = mpBackBufferNoMSAA;
            mDevice.GetImmediateContext()->ResolveSubresource(backbufferNoMSAA.Get(), 0, mpBackBuffer.Get(), 0, BBDesc.Format);
            mDevice.throwIfFailed("Error resolving MSAA subresource", "D3D11RenderWindowBase::copyContentsToMemory");
        }
        else
        {
            D3D11_TEXTURE2D_DESC desc = BBDesc;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = 0;
            desc.CPUAccessFlags = 0;

            HRESULT hr = mDevice->CreateTexture2D(&desc, NULL, backbufferNoMSAA.ReleaseAndGetAddressOf());
            mDevice.throwIfFailed(hr, "Error creating texture without MSAA", "D3D11RenderWindowBase::copyContentsToMemory");

            mDevice.GetImmediateContext()->ResolveSubresource(backbufferNoMSAA.Get(), 0, mpBackBuffer.Get(), 0, BBDesc.Format);
            mDevice.throwIfFailed("Error resolving MSAA subresource", "D3D11RenderWindowBase::copyContentsToMemory");
        }

        // change the parameters of the texture so we can read it
        BBDesc.SampleDesc.Count = 1;
        BBDesc.SampleDesc.Quality = 0;
        BBDesc.Usage = D3D11_USAGE_STAGING;
        BBDesc.BindFlags = 0;
        BBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        // Create the staging texture
        ComPtr<ID3D11Texture2D> stagingTexture;
        HRESULT hr = mDevice->CreateTexture2D(&BBDesc, NULL, stagingTexture.ReleaseAndGetAddressOf());
        mDevice.throwIfFailed(hr, "Error creating staging texture", "D3D11RenderWindowBase::copyContentsToMemory");

        // Copy the back buffer into the staging texture
        mDevice.GetImmediateContext()->CopySubresourceRegion(
            stagingTexture.Get(), srcSubresource, srcBoxDx11.left, srcBoxDx11.top, srcBoxDx11.front,
            backbufferNoMSAA.Get(), srcSubresource, &srcBoxDx11);
        mDevice.throwIfFailed("Error while copying to staging texture", "D3D11RenderWindowBase::copyContentsToMemory");

        // Map the subresource of the staging texture
        D3D11_MAPPED_SUBRESOURCE mapped = {0};
        hr = mDevice.GetImmediateContext()->Map(stagingTexture.Get(), srcSubresource, D3D11_MAP_READ, 0, &mapped);
        mDevice.throwIfFailed(hr, "Error while mapping staging texture", "D3D11RenderWindowBase::copyContentsToMemory");

        // Read the data out of the texture
        PixelBox locked = D3D11Mappings::getPixelBoxWithMapping(srcBoxDx11, BBDesc.Format, mapped);
        PixelUtil::bulkPixelConversion(locked, dst);

        // Release the staging texture
        mDevice.GetImmediateContext()->Unmap(stagingTexture.Get(), srcSubresource);
    }
	//---------------------------------------------------------------------
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
	void D3D11RenderWindowBase::_validateStereo()
	{
		mStereoEnabled = D3D11StereoDriverBridge::getSingleton().isStereoEnabled(this->getName());
	}
#endif
#pragma endregion

    //---------------------------------------------------------------------
    // class D3D11RenderWindowSwapChainBased
    //---------------------------------------------------------------------
#pragma region D3D11RenderWindowSwapChainBased
    D3D11RenderWindowSwapChainBased::D3D11RenderWindowSwapChainBased(D3D11Device & device)
        : D3D11RenderWindowBase(device)
    {
        ZeroMemory( &mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC_N) );
        mUseFlipMode = false;
        mVSync = false;
        mVSyncInterval = 1;

        memset(&mPreviousPresentStats, 0, sizeof(mPreviousPresentStats));
        mPreviousPresentStatsIsValid = false; 
        mVBlankMissCount = 0; 
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::destroy()
    {
        _destroySwapChain();
        D3D11RenderWindowBase::destroy();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::notifyDeviceLost(D3D11Device* device)
    {
        _destroySizeDependedD3DResources();
        _destroySwapChain();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::notifyDeviceRestored(D3D11Device* device)
    {
        _createSwapChain();
        _createSizeDependedD3DResources();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::_destroySwapChain()
    {
        if(mIsFullScreen && mpSwapChain)
            mpSwapChain->SetFullscreenState(false, NULL); // get back from fullscreen

        mpSwapChain.Reset();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::_createSwapChain(void)
    {
        ZeroMemory( &mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC_N) );

        // here the mSwapChainDesc and mpSwapChain are initialized
        OGRE_CHECK_DX_ERROR(_createSwapChainImpl(_queryDxgiDevice().Get()));
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::_createSizeDependedD3DResources()
    {
        mpBackBuffer.Reset();
        mpBackBufferNoMSAA.Reset();

        HRESULT hr = S_OK;
        if(mUseFlipMode && mFSAAType.Count > 1)
        {
            // Swapchain does not support FSAA in FlipMode, therefore create separate back buffer with FSAA
            // Swapchain(FSAA=0, SRGB=0) <-ResolveSubresource- Buffer(FSAA=4x, SRGB=0) <= RenderTargetView(FSAA=4x, SRGB=0)
            D3D11_TEXTURE2D_DESC desc = { 0 };
            desc.Width               = mWidth;
            desc.Height              = mHeight;
            desc.MipLevels           = 1;
            desc.ArraySize           = 1;
            desc.Format              = _getRenderFormat();
            desc.SampleDesc.Count    = mFSAAType.Count;
            desc.SampleDesc.Quality  = mFSAAType.Quality;
            desc.Usage               = D3D11_USAGE_DEFAULT;
            desc.BindFlags           = D3D11_BIND_RENDER_TARGET;
            desc.CPUAccessFlags      = 0;
            desc.MiscFlags           = 0;

            hr = mDevice->CreateTexture2D(&desc, NULL, mpBackBuffer.ReleaseAndGetAddressOf());

            if(SUCCEEDED(hr) && isHardwareGammaEnabled())
            {
                // The worst possible combo, additional buffer needed
                // Swapchain(FSAA=0, SRGB=0) <-CopyResource- Buffer2(FSAA=0, SRGB=1) <-ResolveSubresource- Buffer(FSAA=4x, SRGB=1) <= RenderTargetView(FSAA=4x, SRGB=1)
                desc.SampleDesc.Count    = 1;
                desc.SampleDesc.Quality  = 0;

                hr = mDevice->CreateTexture2D(&desc, NULL, mpBackBufferNoMSAA.ReleaseAndGetAddressOf());
            }
        }
        else
        {
            // Obtain back buffer from swapchain
            hr = mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)mpBackBuffer.ReleaseAndGetAddressOf());
        }

        if(FAILED(hr))
        {
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to Get Back Buffer for swap chain",
                "D3D11RenderWindow::_createSizeDependedD3DResources");
        }

        // create all other size depended resources
        D3D11RenderWindowBase::_createSizeDependedD3DResources();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::_changeBuffersFSAA()
    {
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->fireDeviceEvent(&mDevice,"RenderWindowBeforeResize",this);

        _destroySizeDependedD3DResources();

        if(mUseFlipMode)
        {
            // swapchain is not multisampled in flip sequential mode, so we reuse it
            D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
            rsys->determineFSAASettings(mFSAA, mFSAAHint, _getRenderFormat(), &mFSAAType);
        }
        else
        {
            _destroySwapChain();
            _createSwapChain();
        }

        _createSizeDependedD3DResources();

        // Notify viewports of resize
        _updateViewportsDimensions();
        rsys->fireDeviceEvent(&mDevice,"RenderWindowResized",this);
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::_resizeSwapChainBuffers(unsigned width, unsigned height)
    {
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->validateDevice();
        rsys->fireDeviceEvent(&mDevice,"RenderWindowBeforeResize",this);

        _destroySizeDependedD3DResources();
        
        // Call flush before resize buffers to ensure destruction of resources.
        // not doing so may result in 'Out of memory' exception.
        mDevice.GetImmediateContext()->Flush();

        // width and height can be zero to autodetect size, therefore do not rely on them
        HRESULT hr = mpSwapChain->ResizeBuffers(mSwapChainDesc.BufferCount, width, height, _getSwapChainFormat(), 0);
        if(hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            rsys->handleDeviceLost();
            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else if(FAILED(hr))
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, "Unable to resize swap chain", "D3D11RenderWindowSwapChainBased::_resizeSwapChainBuffers");

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        mpSwapChain->GetDesc(&mSwapChainDesc);
        mWidth = mSwapChainDesc.BufferDesc.Width;
        mHeight = mSwapChainDesc.BufferDesc.Height;
        mIsFullScreen = (0 == mSwapChainDesc.Windowed); // Alt-Enter together with SetWindowAssociation() can change this state
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        mpSwapChain->GetDesc1(&mSwapChainDesc);
        mWidth = mSwapChainDesc.Width;
        mHeight = mSwapChainDesc.Height;
#endif

        _createSizeDependedD3DResources();

        // Notify viewports of resize
        _updateViewportsDimensions();
        rsys->fireDeviceEvent(&mDevice,"RenderWindowResized",this);
    }

    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::swapBuffers( )
    {
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->fireDeviceEvent(&mDevice,"BeforeDevicePresent",this);

        if( !mDevice.isNull() )
        {
            // Step of resolving MSAA resource for swap chains in FlipSequentialMode should be done by application rather than by OS.
            if(mUseFlipMode && mFSAAType.Count > 1)
            {
                ComPtr<ID3D11Texture2D> swapChainBackBuffer;
                HRESULT hr = mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)swapChainBackBuffer.ReleaseAndGetAddressOf());
                if(FAILED(hr))
                    OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, "Error obtaining backbuffer", "D3D11RenderWindowSwapChainBased::swapBuffers");
                if(!isHardwareGammaEnabled())
                {
                    assert(_getRenderFormat() == _getSwapChainFormat());
                    mDevice.GetImmediateContext()->ResolveSubresource(swapChainBackBuffer.Get(), 0, mpBackBuffer.Get(), 0, _getRenderFormat());
				}
				else
                {
                    assert(mpBackBufferNoMSAA);
                    mDevice.GetImmediateContext()->ResolveSubresource(mpBackBufferNoMSAA.Get(), 0, mpBackBuffer.Get(), 0, _getRenderFormat());
                    mDevice.GetImmediateContext()->CopyResource(swapChainBackBuffer.Get(), mpBackBufferNoMSAA.Get());
                }
			}

            // flip presentation model swap chains have another semantic for first parameter
            UINT syncInterval = mUseFlipMode ? std::max(1U, mVSyncInterval) : (mVSync ? mVSyncInterval : 0);
            HRESULT hr = mpSwapChain->Present(syncInterval, 0);
            if( FAILED(hr) )
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, "Error Presenting surfaces", "D3D11RenderWindowSwapChainBased::swapBuffers");
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainBased::updateStats( void )
	{
		RenderTarget::updateStats();
		mStats.vBlankMissCount = getVBlankMissCount();
	}
	//---------------------------------------------------------------------
	int D3D11RenderWindowSwapChainBased::getVBlankMissCount()
	{
		if (!(mIsFullScreen || (!mIsFullScreen && isVSyncEnabled() && mUseFlipMode == true && mFSAA == 0)))
		{
			return -1;
		}

		DXGI_FRAME_STATISTICS currentPresentStats;
		ZeroMemory(&currentPresentStats, sizeof(currentPresentStats));
		HRESULT hr = mpSwapChain->GetFrameStatistics(&currentPresentStats);
		if(FAILED(hr) || currentPresentStats.PresentRefreshCount == 0)
		{
			mPreviousPresentStatsIsValid = false;
			return -1;
		}
		else
		{
			if(mPreviousPresentStatsIsValid == true)
			{
				int currentVBlankMissCount = (currentPresentStats.PresentRefreshCount - mPreviousPresentStats.PresentRefreshCount) 
					- (currentPresentStats.PresentCount - mPreviousPresentStats.PresentCount);
				mVBlankMissCount +=  std::max(0, currentVBlankMissCount);
			}
			mPreviousPresentStats			= currentPresentStats;
			mPreviousPresentStatsIsValid	= true;
		}			
		return mVBlankMissCount;
	}
	
#pragma endregion

    //---------------------------------------------------------------------
    // class D3D11RenderWindowHwnd
    //---------------------------------------------------------------------
#pragma region D3D11RenderWindowHwnd
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    D3D11RenderWindowHwnd::D3D11RenderWindowHwnd(D3D11Device & device)
        : D3D11RenderWindowSwapChainBased(device)
    {
        mHWnd = 0;
		mWindowedWinStyle = 0;
		mFullscreenWinStyle = 0;
		mDesiredWidth = 0;
		mDesiredHeight = 0;
		mLastSwitchingFullscreenCounter = 0;
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowHwnd::create(const String& name, unsigned int width, unsigned int height,
        bool fullScreen, const NameValuePairList *miscParams)
    {
        D3D11RenderWindowSwapChainBased::create(name, width, height, fullScreen, miscParams);

        WNDPROC windowProc = DefWindowProc;
        HWND externalHandle = 0;
        String title = name;

		unsigned int colourDepth = 32;
		int left = INT_MAX; // Defaults to screen center
		int top = INT_MAX;  // Defaults to screen center
		bool depthBuffer = true;
		int monitorIndex = -1;	//Default by detecting the adapter from left / top position	
		size_t fsaaSamples = 0;
		String fsaaHint;

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
            opt = miscParams->find("windowProc");
            if (opt != miscParams->end())
                windowProc = reinterpret_cast<WNDPROC>(StringConverter::parseSizeT(opt->second));
            // externalWindowHandle     -> externalHandle
            opt = miscParams->find("externalWindowHandle");
            if (opt == miscParams->end())
                opt = miscParams->find("parentWindowHandle");
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
            opt = miscParams->find("monitorIndex");
            if(opt != miscParams->end())
                monitorIndex = StringConverter::parseInt(opt->second);

#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
            // useFlipMode    [parseBool]
            opt = miscParams->find("useFlipMode");
            if(opt != miscParams->end())
                mUseFlipMode = IsWindows8OrGreater() && StringConverter::parseBool(opt->second);
#endif
            // vsync    [parseBool]
            opt = miscParams->find("vsync");
            if(opt != miscParams->end())
                mVSync = StringConverter::parseBool(opt->second);
            // vsyncInterval    [parseUnsignedInt]
            opt = miscParams->find("vsyncInterval");
            if(opt != miscParams->end())
                mVSyncInterval = StringConverter::parseUnsignedInt(opt->second);

            // enable double click messages
            opt = miscParams->find("enableDoubleClick");
            if(opt != miscParams->end())
                enableDoubleClick = StringConverter::parseBool(opt->second);

        }
		
		mIsFullScreen = fullScreen;
		

        // Destroy current window if any
        if( mHWnd )
            destroy();

		if (!externalHandle)
		{
			DWORD		dwStyleEx = 0;
			HMONITOR    hMonitor = NULL;		
			MONITORINFO monitorInfo;
			RECT		rc;
			if (hMonitor == NULL)
			{
				POINT windowAnchorPoint;
				windowAnchorPoint.x = left;
				windowAnchorPoint.y = top;
				hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTONEAREST);
			}
			memset(&monitorInfo, 0, sizeof(MONITORINFO));
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &monitorInfo);
			mFullscreenWinStyle = WS_CLIPCHILDREN | WS_POPUP;
			mWindowedWinStyle   = WS_CLIPCHILDREN;
			if (!mHidden)
			{
				mFullscreenWinStyle |= WS_VISIBLE;
				mWindowedWinStyle |= WS_VISIBLE;
			}
            if (border == "none")
                mWindowedWinStyle |= WS_POPUP;
            else if (border == "fixed")
                mWindowedWinStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
                WS_SYSMENU | WS_MINIMIZEBOX;
            else
                mWindowedWinStyle |= WS_OVERLAPPEDWINDOW;
			unsigned int winWidth, winHeight;
			winWidth = width;
			winHeight = height;
			if (left == INT_MAX || top == INT_MAX)
			{				
				uint32 screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
				uint32 screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
				uint32 outerw = (winWidth < screenw)? winWidth : screenw;
				uint32 outerh = (winHeight < screenh)? winHeight : screenh;
				if (left == INT_MAX)
					left = monitorInfo.rcWork.left + (screenw - outerw) / 2;
				else if (monitorIndex != -1)
					left += monitorInfo.rcWork.left;
				if (top == INT_MAX)
					top = monitorInfo.rcWork.top + (screenh - outerh) / 2;
				else if (monitorIndex != -1)
					top += monitorInfo.rcWork.top;
			}
			else if (monitorIndex != -1)
			{
				left += monitorInfo.rcWork.left;
				top += monitorInfo.rcWork.top;
			}
			mWidth = mDesiredWidth = width;
			mHeight = mDesiredHeight = height;
			mTop = top;
			mLeft = left;
			if (fullScreen)
			{
				dwStyleEx |= WS_EX_TOPMOST;				
				mTop = monitorInfo.rcMonitor.top;
				mLeft = monitorInfo.rcMonitor.left;		
			}
			else
			{				
				adjustWindow(width, height, &winWidth, &winHeight);
				if (!outerSize)
				{
					SetRect(&rc, 0, 0, mWidth, mHeight);
					AdjustWindowRect(&rc, getWindowStyle(fullScreen), false);
					mWidth = rc.right - rc.left;
					mHeight = rc.bottom - rc.top;
					if (mLeft < monitorInfo.rcWork.left)
						mLeft = monitorInfo.rcWork.left;		
					if (mTop < monitorInfo.rcWork.top)					
						mTop = monitorInfo.rcWork.top;					
					if (static_cast<int>(winWidth) > monitorInfo.rcWork.right - mLeft)					
						winWidth = monitorInfo.rcWork.right - mLeft;	
					if (static_cast<int>(winHeight) > monitorInfo.rcWork.bottom - mTop)					
						winHeight = monitorInfo.rcWork.bottom - mTop;										
				}
			}
			UINT classStyle = 0;
			if (enableDoubleClick)
				classStyle |= CS_DBLCLKS;

			HINSTANCE hInst = NULL;
			static TCHAR staticVar;
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &staticVar, &hInst);

			WNDCLASS wc = { classStyle, windowProc, 0, 0, hInst,
				LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, OGRE_D3D11_WIN_CLASS_NAME };
			RegisterClass(&wc);
			mIsExternal = false;
			mHWnd = CreateWindowEx(dwStyleEx, OGRE_D3D11_WIN_CLASS_NAME, title.c_str(), getWindowStyle(fullScreen),
				mLeft, mTop, winWidth, winHeight, 0, 0, hInst, this);
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
            << "D3D11: Created D3D11 Rendering Window '"
            << mName << "' : " << mWidth << "x" << mHeight;

        _createSwapChain();
        _createSizeDependedD3DResources();
        mDevice.GetDXGIFactory()->MakeWindowAssociation(mHWnd, NULL);
        setHidden(mHidden);

        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->fireDeviceEvent(&mDevice,"RenderWindowCreated",this);
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowHwnd::destroy()
    {
        D3D11RenderWindowSwapChainBased::destroy();

        if (mHWnd && !mIsExternal)
        {
            DestroyWindow(mHWnd);
        }

        mHWnd = NULL;
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowHwnd::notifyDeviceRestored(D3D11Device* device)
    {
        D3D11RenderWindowSwapChainBased::notifyDeviceRestored(device);
        mDevice.GetDXGIFactory()->MakeWindowAssociation(mHWnd, NULL);
    }
    //---------------------------------------------------------------------
	HRESULT D3D11RenderWindowHwnd::_createSwapChainImpl(IDXGIDeviceN* pDXGIDevice)
	{
		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		rsys->determineFSAASettings(mFSAA, mFSAAHint, _getRenderFormat(), &mFSAAType);

		ZeroMemory(&mSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC_N));
		mSwapChainDesc.BufferDesc.Width = mWidth;
		mSwapChainDesc.BufferDesc.Height = mHeight;
		mSwapChainDesc.BufferDesc.Format = _getSwapChainFormat();

		mSwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		mSwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;

		mSwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mSwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
		if(mUseFlipMode)
		{
			mSwapChainDesc.SampleDesc.Count = 1;
			mSwapChainDesc.SampleDesc.Quality = 0;
			mSwapChainDesc.BufferCount = 2;
#if defined(_WIN32_WINNT_WIN10) // we want DXGI_SWAP_EFFECT_FLIP_DISCARD even if _WIN32_WINNT < _WIN32_WINNT_WIN10 but runtime is Win10
			mSwapChainDesc.SwapEffect = IsWindows10OrGreater() ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
#else
			mSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
#endif
		}
		else
#endif	
		{
			assert(!mUseFlipMode);
			mSwapChainDesc.SampleDesc.Count = mFSAAType.Count;
			mSwapChainDesc.SampleDesc.Quality = mFSAAType.Quality;
			mSwapChainDesc.BufferCount = 1;
			mSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		}

        mSwapChainDesc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        mSwapChainDesc.OutputWindow         = mHWnd;
        mSwapChainDesc.Windowed             = !mIsFullScreen;
        mSwapChainDesc.Flags                = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        if (!mVSync && !mIsFullScreen)
        {
            // NB not using vsync in windowed mode in D3D11 can cause jerking at low 
            // frame rates no matter what buffering modes are used (odd - perhaps a
            // timer issue in D3D11 since GL doesn't suffer from this) 
            // low is < 200fps in this context
            LogManager::getSingleton().logWarning(
                "D3D11: disabling VSync in windowed mode can cause timing issues at lower "
                "frame rates, turn VSync on if you observe this problem.");
        }

        // Create swap chain            
        HRESULT hr = mDevice.GetDXGIFactory()->CreateSwapChain(pDXGIDevice, &mSwapChainDesc, mpSwapChain.ReleaseAndGetAddressOf());

        return hr;
    }
    //---------------------------------------------------------------------
    bool D3D11RenderWindowHwnd::isVisible() const
    {
        HWND currentWindowHandle = mHWnd;
        bool visible;
        while ((visible = (IsIconic(currentWindowHandle) == false)) &&
            (GetWindowLong(currentWindowHandle, GWL_STYLE) & WS_CHILD) != 0)
        {
            currentWindowHandle = GetParent(currentWindowHandle);
        } 
        return visible;
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
				unsigned int winWidth, winHeight;
				adjustWindow(width, height, &winWidth, &winHeight);
				SetWindowPos(mHWnd, 0, 0, 0, winWidth, winHeight,
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		else
			updateWindowRect();
	}
    //---------------------------------------------------------------------
    void D3D11RenderWindowHwnd::windowMovedOrResized()
    {
        if (!mHWnd || IsIconic(mHWnd) || !mpSwapChain)
            return;

		updateWindowRect();		
		
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
        // D3DDEVICE            : getD3DDevice
        // WINDOW               : getWindowHandle

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
			{
				D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
				rsys->addToSwitchingFullscreenCounter();
			}

            DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;

            bool oldFullscreen = mIsFullScreen;
            mIsFullScreen = fullScreen;

            if (fullScreen)
            {
				HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO monitorInfo;
				memset(&monitorInfo, 0, sizeof(MONITORINFO));
				monitorInfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &monitorInfo);
				mTop = monitorInfo.rcMonitor.top;
				mLeft = monitorInfo.rcMonitor.left;				
				
                // need different ordering here

                if (oldFullscreen)
                {
                    // was previously fullscreen, just changing the resolution
                    SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE);
                }
                else
                {
                    SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOACTIVATE);
                    SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
                    SetWindowPos(mHWnd, 0, 0,0, 0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
                }
            }
            else
            {
				unsigned int winWidth, winHeight;
				winWidth = mWidth;
				winHeight = mHeight;
				adjustWindow(mWidth, mHeight, &winWidth, &winHeight);
				SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
				SetWindowPos(mHWnd, HWND_NOTOPMOST, 0, 0, winWidth, winHeight,
					SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOACTIVATE);
				updateWindowRect();
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
	void D3D11RenderWindowHwnd::adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
		unsigned int* winWidth, unsigned int* winHeight)
	{
		RECT rc;
		SetRect(&rc, 0, 0, clientWidth, clientHeight);
		AdjustWindowRect(&rc, getWindowStyle(mIsFullScreen), false);
		*winWidth = rc.right - rc.left;
		*winHeight = rc.bottom - rc.top;
	}

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
			if (mWidth != mDesiredWidth ||
				mHeight != mDesiredHeight)
			{
				mWidth = mDesiredWidth;
				mHeight = mDesiredHeight;				
			}
			unsigned int winWidth, winHeight;
			adjustWindow(mWidth, mHeight, &winWidth, &winHeight);
			HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo;
			memset(&monitorInfo, 0, sizeof(MONITORINFO));
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &monitorInfo);
			ULONG screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
			ULONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
			int left = screenw > winWidth ? ((screenw - winWidth) / 2) : 0;
			int top = screenh > winHeight ? ((screenh - winHeight) / 2) : 0;
			SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
				SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);
			updateWindowRect();
			

        }

		DXGI_SWAP_CHAIN_DESC dsc;
		ZeroMemory(&dsc, sizeof(dsc));
		mpSwapChain->GetDesc(&dsc);
		if((dsc.Windowed != 0) == mIsFullScreen)
		{
            mpSwapChain->SetFullscreenState(mIsFullScreen, NULL);
		}
		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		mLastSwitchingFullscreenCounter = rsys->getSwitchingFullscreenCounter();

    }
	
	void D3D11RenderWindowHwnd::updateWindowRect()
	{
		RECT rc;
		BOOL result;
		result = GetWindowRect(mHWnd, &rc);
		if (result == FALSE)
		{
			mTop = 0;
			mLeft = 0;
			mWidth = 0;
			mHeight = 0;
			return;
		}
		mTop = rc.top;
		mLeft = rc.left;
		result = GetClientRect(mHWnd, &rc);
		if (result == FALSE)
		{
			mTop = 0;
			mLeft = 0;
			mWidth = 0;
			mHeight = 0;
			return;
		}
		unsigned int width = rc.right - rc.left;
		unsigned int height = rc.bottom - rc.top;
		if (width != mWidth || height != mHeight)
		{
			mWidth  = rc.right - rc.left;
			mHeight = rc.bottom - rc.top;
			_resizeSwapChainBuffers(mWidth, mHeight);
		}	
	}
	void D3D11RenderWindowHwnd::_beginUpdate()
	{		
		if (mIsExternal)
		{		
			updateWindowRect();
		}
		if (mWidth == 0 || mHeight == 0)
		{
			return;
		}
		DXGI_SWAP_CHAIN_DESC dsc;
		ZeroMemory(&dsc, sizeof(dsc));
		mpSwapChain->GetDesc(&dsc);
		D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
		if(rsys->getSwitchingFullscreenCounter() > mLastSwitchingFullscreenCounter 
			|| GetFocus() == mHWnd && ((dsc.Windowed != 0) == mIsFullScreen))
		{
			_finishSwitchingFullscreen();		
		}
		RenderWindow::_beginUpdate();
	}
	void D3D11RenderWindowHwnd::setActive(bool state)
	{
		if (mHWnd && mpSwapChain && mIsFullScreen && state)
		{
			DXGI_SWAP_CHAIN_DESC dsc;
			ZeroMemory(&dsc, sizeof(dsc));
			mpSwapChain->GetDesc(&dsc);
			if(dsc.Windowed)
			{
				D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
				rsys->addToSwitchingFullscreenCounter();
			}

		}
		RenderWindow::setActive(state);
	}
#endif
#pragma endregion

    //---------------------------------------------------------------------
    // class D3D11RenderWindowCoreWindow
    //---------------------------------------------------------------------
#pragma region D3D11RenderWindowCoreWindow
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
    D3D11RenderWindowCoreWindow::D3D11RenderWindowCoreWindow(D3D11Device& device)
        : D3D11RenderWindowSwapChainBased(device)
    {
        mUseFlipMode = true;
    }

    float D3D11RenderWindowCoreWindow::getViewPointToPixelScale()
    {
#if _WIN32_WINNT > _WIN32_WINNT_WIN8
        return Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi / 96;
#else
        return Windows::Graphics::Display::DisplayProperties::LogicalDpi / 96;
#endif
    }

    void D3D11RenderWindowCoreWindow::create(const String& name, unsigned int widthPt, unsigned int heightPt,
        bool fullScreen, const NameValuePairList *miscParams)
    {
        D3D11RenderWindowSwapChainBased::create(name, widthPt, heightPt, fullScreen, miscParams);

        Windows::UI::Core::CoreWindow^ externalHandle = nullptr;

        if(miscParams)
        {
            // Get variable-length params
            NameValuePairList::const_iterator opt;
            // externalWindowHandle     -> externalHandle
            opt = miscParams->find("externalWindowHandle");
            if(opt != miscParams->end())
                externalHandle = reinterpret_cast<Windows::UI::Core::CoreWindow^>((void*)StringConverter::parseSizeT(opt->second));
            else
                externalHandle = Windows::UI::Core::CoreWindow::GetForCurrentThread();
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

        float scale = getViewPointToPixelScale();
        Windows::Foundation::Rect rc = mCoreWindow->Bounds;
        mLeft = (int)(rc.X * scale + 0.5f);
        mTop = (int)(rc.Y * scale + 0.5f);
        mWidth = (int)(rc.Width * scale + 0.5f);
        mHeight = (int)(rc.Height * scale + 0.5f);

        LogManager::getSingleton().stream() << std::fixed << std::setprecision(1) 
            << "D3D11: Created D3D11 Rendering Window \"" << mName << "\", " << rc.Width << " x " << rc.Height
            << ", with backing store " << mWidth << "x" << mHeight << " "
            << "using content scaling factor " << scale;

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
#if !__OGRE_WINRT_PHONE
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->determineFSAASettings(mFSAA, mFSAAHint, _getRenderFormat(), &mFSAAType);
#endif

        mSwapChainDesc.Width                = 0;                                    // Use automatic sizing.
        mSwapChainDesc.Height               = 0;
        mSwapChainDesc.Format               = _getSwapChainFormat();
        mSwapChainDesc.Stereo               = false;

        assert(mUseFlipMode);                                                       // i.e. no FSAA for swapchain, but can be enabled in separate backbuffer
        mSwapChainDesc.SampleDesc.Count     = 1;
        mSwapChainDesc.SampleDesc.Quality   = 0;

        mSwapChainDesc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
#if __OGRE_WINRT_PHONE_80
        mSwapChainDesc.BufferCount          = 1;                                    // WP8: One buffer.
        mSwapChainDesc.Scaling              = DXGI_SCALING_STRETCH;                 // WP8: Must be stretch scaling mode.
        mSwapChainDesc.SwapEffect           = DXGI_SWAP_EFFECT_DISCARD;             // WP8: No swap effect.
#else
        mSwapChainDesc.BufferCount          = 2;                                    // Use two buffers to enable flip effect.
        mSwapChainDesc.Scaling              = DXGI_SCALING_NONE;                    // Otherwise stretch would be used by default.
        mSwapChainDesc.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;     // MS recommends using this swap effect for all applications.
#endif
        mSwapChainDesc.AlphaMode            = DXGI_ALPHA_MODE_UNSPECIFIED;

        // Create swap chain
        HRESULT hr = mDevice.GetDXGIFactory()->CreateSwapChainForCoreWindow(pDXGIDevice, reinterpret_cast<IUnknown*>(mCoreWindow.Get()), &mSwapChainDesc, NULL, mpSwapChain.ReleaseAndGetAddressOf());
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
        float scale = getViewPointToPixelScale();
        Windows::Foundation::Rect rc = mCoreWindow->Bounds;
        mLeft = (int)(rc.X * scale + 0.5f);
        mTop = (int)(rc.Y * scale + 0.5f);
        mWidth = (int)(rc.Width * scale + 0.5f);
        mHeight = (int)(rc.Height * scale + 0.5f);

        _resizeSwapChainBuffers(0, 0);      // pass zero to autodetect size
    }

#endif
#pragma endregion

    //---------------------------------------------------------------------
    // class D3D11RenderWindowSwapChainPanel
    //---------------------------------------------------------------------
#pragma region D3D11RenderWindowSwapChainPanel
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT && defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
	D3D11RenderWindowSwapChainPanel::D3D11RenderWindowSwapChainPanel(D3D11Device& device)
        : D3D11RenderWindowSwapChainBased(device)
        , mCompositionScale(1.0f, 1.0f)
    {
        mUseFlipMode = true;
    }

    float D3D11RenderWindowSwapChainPanel::getViewPointToPixelScale()
    {
        return std::max(mCompositionScale.Width, mCompositionScale.Height);
    }

    void D3D11RenderWindowSwapChainPanel::create(const String& name, unsigned int widthPt, unsigned int heightPt,
        bool fullScreen, const NameValuePairList *miscParams)
    {
        D3D11RenderWindowSwapChainBased::create(name, widthPt, heightPt, fullScreen, miscParams);

        Windows::UI::Xaml::Controls::SwapChainPanel^ externalHandle = nullptr;

        if(miscParams)
        {
            // Get variable-length params
            NameValuePairList::const_iterator opt;
            // externalWindowHandle     -> externalHandle
            opt = miscParams->find("externalWindowHandle");
            if(opt != miscParams->end())
                externalHandle = reinterpret_cast<Windows::UI::Xaml::Controls::SwapChainPanel^>((void*)StringConverter::parseSizeT(opt->second));
        }

        // Reset current control if any
        mSwapChainPanel = nullptr;

        if (!externalHandle)
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "External window handle is not specified.", "D3D11RenderWindow::create" );
        }
        else
        {
            mSwapChainPanel = externalHandle;
            mIsExternal = true;

            // subscribe to important notifications
            compositionScaleChangedToken = (mSwapChainPanel->CompositionScaleChanged +=
                ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::SwapChainPanel^, Platform::Object^>([this](Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Platform::Object^ e)
            {
                windowMovedOrResized();
            }));
            sizeChangedToken = (mSwapChainPanel->SizeChanged +=
                ref new Windows::UI::Xaml::SizeChangedEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
            {
                windowMovedOrResized();
            }));
        }

        Windows::Foundation::Size sz = Windows::Foundation::Size(static_cast<float>(mSwapChainPanel->ActualWidth), static_cast<float>(mSwapChainPanel->ActualHeight));
        mCompositionScale = Windows::Foundation::Size(mSwapChainPanel->CompositionScaleX, mSwapChainPanel->CompositionScaleY);
        mLeft = 0;
        mTop = 0;
        mWidth = (int)(sz.Width * mCompositionScale.Width + 0.5f);
        mHeight = (int)(sz.Height * mCompositionScale.Height + 0.5f);

        // Prevent zero size DirectX content from being created.
        mWidth = std::max(mWidth, 1U);
        mHeight = std::max(mHeight, 1U);

        LogManager::getSingleton().stream() << std::fixed << std::setprecision(1) 
            << "D3D11: Created D3D11 SwapChainPanel Rendering Window \"" << mName << "\", " << sz.Width << " x " << sz.Height
            << ", with backing store " << mWidth << "x" << mHeight << " "
            << "using content scaling factor { " << mCompositionScale.Width << ", " << mCompositionScale.Height << " }";

        _createSwapChain();
        _createSizeDependedD3DResources();
    }

    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainPanel::destroy()
    {
        D3D11RenderWindowSwapChainBased::destroy();

        if (mSwapChainPanel && !mIsExternal)
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Only external window handles are supported."
                , "D3D11RenderWindow::destroy" );
        }
        mSwapChainPanel->CompositionScaleChanged -= compositionScaleChangedToken; compositionScaleChangedToken.Value = 0;
        mSwapChainPanel->SizeChanged -= sizeChangedToken; sizeChangedToken.Value = 0;
        mSwapChainPanel = nullptr;
    }
    //---------------------------------------------------------------------
    HRESULT D3D11RenderWindowSwapChainPanel::_createSwapChainImpl(IDXGIDeviceN* pDXGIDevice)
    {
#if !__OGRE_WINRT_PHONE
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->determineFSAASettings(mFSAA, mFSAAHint, _getRenderFormat(), &mFSAAType);
#endif

        mSwapChainDesc.Width                = mWidth;                               // Use automatic sizing.
        mSwapChainDesc.Height               = mHeight;
        mSwapChainDesc.Format               = _getSwapChainFormat();
        mSwapChainDesc.Stereo               = false;

        assert(mUseFlipMode);                                                       // i.e. no FSAA for swapchain, but can be enabled in separate backbuffer
        mSwapChainDesc.SampleDesc.Count     = 1;
        mSwapChainDesc.SampleDesc.Quality   = 0;

        mSwapChainDesc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        mSwapChainDesc.BufferCount          = 2;                                    // Use two buffers to enable flip effect.
        mSwapChainDesc.Scaling              = DXGI_SCALING_STRETCH;                 // Required for CreateSwapChainForComposition.
        mSwapChainDesc.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;     // MS recommends using this swap effect for all applications.
        mSwapChainDesc.AlphaMode            = DXGI_ALPHA_MODE_UNSPECIFIED;

        // Create swap chain
        HRESULT hr = mDevice.GetDXGIFactory()->CreateSwapChainForComposition(pDXGIDevice, &mSwapChainDesc, NULL, mpSwapChain.ReleaseAndGetAddressOf());
        if (FAILED(hr))
            return hr;

        // Associate swap chain with SwapChainPanel
        // Get backing native interface for SwapChainPanel
        ComPtr<ISwapChainPanelNative> panelNative;
        hr = reinterpret_cast<IUnknown*>(mSwapChainPanel)->QueryInterface(IID_PPV_ARGS(panelNative.ReleaseAndGetAddressOf()));
        if(FAILED(hr))
            return hr;
        hr = panelNative->SetSwapChain(mpSwapChain.Get());
        if(FAILED(hr))
            return hr;

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces 
        // latency and ensures that the application will only render after each VSync, minimizing 
        // power consumption.
        hr = pDXGIDevice->SetMaximumFrameLatency(1);
        if(FAILED(hr))
            return hr;

        hr = _compensateSwapChainCompositionScale();
        return hr;
    }
    //---------------------------------------------------------------------
    bool D3D11RenderWindowSwapChainPanel::isVisible() const
    {
        return (mSwapChainPanel && mSwapChainPanel->Visibility == Windows::UI::Xaml::Visibility::Visible);
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowSwapChainPanel::windowMovedOrResized()
    {
        Windows::Foundation::Size sz = Windows::Foundation::Size(static_cast<float>(mSwapChainPanel->ActualWidth), static_cast<float>(mSwapChainPanel->ActualHeight));
        mCompositionScale = Windows::Foundation::Size(mSwapChainPanel->CompositionScaleX, mSwapChainPanel->CompositionScaleY);
        mLeft = 0;
        mTop = 0;
        mWidth = (int)(sz.Width * mCompositionScale.Width + 0.5f);
        mHeight = (int)(sz.Height * mCompositionScale.Height + 0.5f);

        // Prevent zero size DirectX content from being created.
        mWidth = std::max(mWidth, 1U);
        mHeight = std::max(mHeight, 1U);

        _resizeSwapChainBuffers(mWidth, mHeight);

        _compensateSwapChainCompositionScale();
    }

    HRESULT D3D11RenderWindowSwapChainPanel::_compensateSwapChainCompositionScale()
    {
        // Setup inverse scale on the swap chain
        DXGI_MATRIX_3X2_F inverseScale = { 0 };
        inverseScale._11 = 1.0f / mCompositionScale.Width;
        inverseScale._22 = 1.0f / mCompositionScale.Height;
        ComPtr<IDXGISwapChain2> spSwapChain2;
        HRESULT hr = mpSwapChain.As<IDXGISwapChain2>(&spSwapChain2);
        if(FAILED(hr))
            return hr;

        hr = spSwapChain2->SetMatrixTransform(&inverseScale);
        return hr;
    }

#endif
#pragma endregion

    //---------------------------------------------------------------------
    // class D3D11RenderWindowImageSource
    //---------------------------------------------------------------------
#pragma region D3D11RenderWindowImageSource
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT && !__OGRE_WINRT_PHONE_80

    D3D11RenderWindowImageSource::D3D11RenderWindowImageSource(D3D11Device& device)
        : D3D11RenderWindowBase(device)
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

        mImageSourceNative.Reset();
        mImageSource = nullptr;
        mBrush = nullptr;
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowImageSource::notifyDeviceLost(D3D11Device* device)
    {
        _destroySizeDependedD3DResources();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowImageSource::notifyDeviceRestored(D3D11Device* device)
    {
        _createSizeDependedD3DResources();
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowImageSource::_createSizeDependedD3DResources()
    {
        mpBackBuffer.Reset();
        mpBackBufferNoMSAA.Reset();
        mImageSourceNative.Reset();

        if(mWidth <= 0 || mHeight <= 0)
        {
            mImageSource = nullptr;
            mBrush->ImageSource = nullptr;
            return;
        }

        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->determineFSAASettings(mFSAA, mFSAAHint, _getRenderFormat(), &mFSAAType);

        // create back buffer - ID3D11Texture2D
        D3D11_TEXTURE2D_DESC desc = {0};
        desc.Width = mWidth;
        desc.Height = mHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = _getRenderFormat();
        desc.SampleDesc.Count = mFSAAType.Count;
        desc.SampleDesc.Quality = mFSAAType.Quality;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        // Create back buffer, maybe with FSAA
        HRESULT hr = mDevice->CreateTexture2D(&desc, NULL, mpBackBuffer.ReleaseAndGetAddressOf());
        if(FAILED(hr) && mFSAAType.Count > 1)
        {
            // Second chance - try without FSAA, keep mFSAAType synchronized.
            LogManager::getSingleton().logMessage("Unable to Create MSAA Back Buffer, retry without MSAA support");
            desc.SampleDesc.Count = mFSAAType.Count = 1;
            desc.SampleDesc.Quality = mFSAAType.Quality = 0;
            hr = mDevice->CreateTexture2D(&desc, NULL, mpBackBuffer.ReleaseAndGetAddressOf());
        }
        if( FAILED(hr) )
        {
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to Create Back Buffer",
                "D3D11RenderWindowImageSource::_createSizeDependedD3DResources");
        }

        // Create optional back buffer without FSAA if needed
        if(mFSAAType.Count > 1)
        {
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            hr = mDevice->CreateTexture2D(&desc, NULL, mpBackBufferNoMSAA.ReleaseAndGetAddressOf());
            if( FAILED(hr) )
            {
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                    "Unable to Create Back Buffer without MSAA",
                    "D3D11RenderWindowImageSource::_createSizeDependedD3DResources");
            }
        }

        // create front buffer - SurfaceImageSource
        mImageSource = ref new Windows::UI::Xaml::Media::Imaging::SurfaceImageSource(mWidth, mHeight, true);
        reinterpret_cast<IUnknown*>(mImageSource)->QueryInterface(__uuidof(ISurfaceImageSourceNative), (void **)mImageSourceNative.ReleaseAndGetAddressOf());

        // set DXGI device for the front buffer
        mImageSourceNative->SetDevice(_queryDxgiDevice().Get());

        // create all other size depended resources
        D3D11RenderWindowBase::_createSizeDependedD3DResources();

        mBrush->ImageSource = mImageSource;
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowImageSource::update(bool swapBuffers)
    {
        if(!mImageSourceNative)
            return;

        D3D11RenderWindowBase::update(swapBuffers);
    }
    //---------------------------------------------------------------------
    void D3D11RenderWindowImageSource::swapBuffers()
    {
        if(!mImageSourceNative)
            return;

        ComPtr<IDXGISurface> dxgiSurface;
        RECT updateRect = { 0, 0, (LONG)mWidth, (LONG)mHeight };
        POINT offset = { 0, 0 };

        HRESULT hr = mImageSourceNative->BeginDraw(updateRect, dxgiSurface.ReleaseAndGetAddressOf(), &offset);
        if(hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            return;

        if(FAILED(hr))
        {
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to Get DXGI surface for SurfaceImageSource",
                "D3D11RenderWindowImageSource::swapBuffers");
        }

        ComPtr<ID3D11Texture2D> destTexture;
        hr = dxgiSurface.As(&destTexture);
        if(FAILED(hr))
        {
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to convert DXGI surface to D3D11 texture",
                "D3D11RenderWindowImageSource::swapBuffers");
        }

        // resolve multi-sample texture into single-sample texture if needed
        if(mpBackBufferNoMSAA)
        {
            mDevice.GetImmediateContext()->ResolveSubresource(mpBackBufferNoMSAA.Get(), 0, mpBackBuffer.Get(), 0, _getRenderFormat());
            mDevice.GetImmediateContext()->CopySubresourceRegion1(destTexture.Get(), 0, offset.x, offset.y, 0, mpBackBufferNoMSAA.Get(), 0, NULL, 0);
        }
        else
            mDevice.GetImmediateContext()->CopySubresourceRegion1(destTexture.Get(), 0, offset.x, offset.y, 0, mpBackBuffer.Get(), 0, NULL, 0);

        hr = mImageSourceNative->EndDraw();
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
        D3D11RenderSystem* rsys = static_cast<D3D11RenderSystem*>(Root::getSingleton().getRenderSystem());
        rsys->validateDevice();

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
#endif // !__OGRE_WINRT_PHONE_80
#pragma endregion
}
