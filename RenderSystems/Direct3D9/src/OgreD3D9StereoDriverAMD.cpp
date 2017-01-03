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

#include "OgreD3D9StereoDriverAMD.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreD3D9RenderWindow.h"
#include "OgreD3D9DeviceManager.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	D3D9StereoDriverAMD::D3D9StereoDriverAMD()
	{
		mStereoEnabled = false;
		mLineOffset = 0;
		mDevice = NULL;
		mDriverComSurface = NULL;
		mStereoBuffer = NULL;
		mLeftBuffer = NULL;
		mRightBuffer = NULL;
		Root::getSingleton().getRenderSystem()->addListener(this);
	}
	//-----------------------------------------------------------------------------
	D3D9StereoDriverAMD::~D3D9StereoDriverAMD()
	{
	    Root::getSingleton().getRenderSystem()->removeListener(this);
		releaseResources();
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverAMD::addRenderWindow(D3D9RenderWindow* renderWindow)
	{
	    // Add the window to the existing map
		mRenderWindowMap[renderWindow->getName()] = renderWindow;
		renderWindow->addListener(this);
		
		return false;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverAMD::removeRenderWindow(const String& renderWindowName)
	{
	    // Remove the window from the existing map
		D3D9RenderWindow* renderWindow = mRenderWindowMap[renderWindowName];
		renderWindow->removeListener(this);
		mRenderWindowMap.erase(renderWindowName);
	
		return false;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverAMD::isStereoEnabled(const String& renderWindowName)
	{
		return mStereoEnabled;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverAMD::setDrawBuffer(ColourBufferType colourBuffer)
	{
		// Note, setDrawBuffer is normally called at the beginning of rendering to the viewport
		// but for this implementation, the related logic is in the postViewportUpdate() method
		if (!mStereoEnabled)
			return false;

		return true;
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::eventOccurred(const String& eventName, const NameValuePairList* parameters)
	{
		if (eventName.compare("DeviceCreated") == 0)
			deviceCreatedEvent(parameters);
		else if (eventName.compare("DeviceLost") == 0)
			deviceLostEvent(parameters);
		else if (eventName.compare("AfterDeviceReset") == 0)
			afterDeviceResetEvent(parameters);
		else if (eventName.compare("BeforeDevicePresent") == 0)
			beforeDevicePresentEvent(parameters);
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::deviceCreatedEvent(const NameValuePairList* parameters)
	{
		NameValuePairList::const_iterator iter = parameters->find("D3DDEVICE");
		mDevice = reinterpret_cast<IDirect3DDevice9*>(StringConverter::parseSizeT(iter->second));

		// Cache the parameters and verify the settings are valid for stereo
		if (!getPresentParamsAndVerifyStereoSettings())
			return;

		// Send the enable stereo command to the driver
		if (!sendStereoCommand(ATI_STEREO_ENABLESTEREO, NULL, 0, 0, 0))
		{
			releaseResources();
			return;
		}

		// Send the enable anti-aliasing command to the driver
		DWORD multiSampleAntiAlias;
		mDevice->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &multiSampleAntiAlias);
		if (1 == multiSampleAntiAlias && !sendStereoCommand(ATI_STEREO_ENABLEPRIMARYAA, NULL, 0, 0, 0))
		{
			releaseResources();
			return;
		}

		// Get the available valid stereo display modes
		ATIDX9GETDISPLAYMODES validStereoDisplayModes;
		validStereoDisplayModes.dwNumModes = 0;
		validStereoDisplayModes.pStereoModes = NULL;

		// Send stereo command to get the number of available stereo modes
		if (!sendStereoCommand(ATI_STEREO_GETDISPLAYMODES, (BYTE*)(&validStereoDisplayModes), sizeof(ATIDX9GETDISPLAYMODES), 0, 0))
		{
			releaseResources();
			return;
		}

		// Store the list of available stereo modes
		if (validStereoDisplayModes.dwNumModes != 0)
		{
			// Allocate memory to store the list of available stereo modes
			validStereoDisplayModes.pStereoModes = new D3DDISPLAYMODE[validStereoDisplayModes.dwNumModes];

			// Send stereo command to get the list of stereo modes
			if (!sendStereoCommand(ATI_STEREO_GETDISPLAYMODES, (BYTE*)(&validStereoDisplayModes), sizeof(ATIDX9GETDISPLAYMODES), 0, 0))
			{
				delete[] validStereoDisplayModes.pStereoModes;
				releaseResources();
				return;
			}
		}

		// Get the display mode from the device
		D3DDISPLAYMODE mode;
		mDevice->GetDisplayMode(0, &mode);

		// Verify the display mode from the device matches one of the valid stereo display modes
		int displayModeMatch = -1;
		int i = 0;
		while (displayModeMatch < 0 && i < (int)validStereoDisplayModes.dwNumModes)
		{
			if (validStereoDisplayModes.pStereoModes[i].Width == mode.Width &&
			validStereoDisplayModes.pStereoModes[i].Height == mode.Height &&
			validStereoDisplayModes.pStereoModes[i].Format == mode.Format &&
			validStereoDisplayModes.pStereoModes[i].RefreshRate == mode.RefreshRate)
			displayModeMatch = i;

			i++;
		}

		// Release memory used to store the list of available stereo modes
		delete[] validStereoDisplayModes.pStereoModes;

		// Validate the device matches one of the stereo display modes
		if (displayModeMatch < 0)
		{
			releaseResources();
			return;
		}

		// Create the resources
		afterDeviceResetEvent(parameters);
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::deviceLostEvent(const NameValuePairList* parameters)
	{
		releaseResources();
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::afterDeviceResetEvent(const NameValuePairList* parameters)
	{
		NameValuePairList::const_iterator iter = parameters->find("D3DDEVICE");
		mDevice = reinterpret_cast<IDirect3DDevice9*>(StringConverter::parseSizeT(iter->second));

		// Verify fullscreen since AMD stereo only works in fullscreen
		D3D9Device* ogreDevice = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(mDevice);
		if (NULL == ogreDevice || !ogreDevice->isFullScreen())
		{
			releaseResources();
			return;
		}

		// Cache the parameters and verify the settings are valid for stereo
		if (!getPresentParamsAndVerifyStereoSettings())
			return;

		// Get the line offset
		if (!sendStereoCommand(ATI_STEREO_GETLINEOFFSET, (BYTE*)(&mLineOffset), sizeof(DWORD), 0, 0))
		{
			releaseResources();
			return;
		}

		// Verify the line offset is valid
		if (0 == mLineOffset)
		{
			releaseResources();
			return;
		}

		// Cache the back buffer as the final stereo buffer
		if (FAILED(mDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &mStereoBuffer)))
		{
			releaseResources();
			return;
		}

		// Create the left buffer
		if (FAILED(mDevice->CreateRenderTarget(mSwapChainPresentParameters.BackBufferWidth, mSwapChainPresentParameters.BackBufferHeight,
										   mSwapChainPresentParameters.BackBufferFormat, mSwapChainPresentParameters.MultiSampleType,
										   mSwapChainPresentParameters.MultiSampleQuality, false, &mLeftBuffer, NULL)))
		{
			releaseResources();
			return;
		}

		// Create the right buffer
		if (FAILED(mDevice->CreateRenderTarget(mSwapChainPresentParameters.BackBufferWidth, mSwapChainPresentParameters.BackBufferHeight,
										   mSwapChainPresentParameters.BackBufferFormat, mSwapChainPresentParameters.MultiSampleType,
										   mSwapChainPresentParameters.MultiSampleQuality, false, &mRightBuffer, NULL)))
		{
			releaseResources();
			return;
		}

		mStereoEnabled = true;
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::beforeDevicePresentEvent(const NameValuePairList* parameters)
	{
		if (!mStereoEnabled)
			return;

		// Use the stereo buffer render target
		mDevice->SetRenderTarget(0, mStereoBuffer);

		// Update the quad buffer with the right render target
		D3DSURFACE_DESC rightBufferDesc;
		mRightBuffer->GetDesc(&rightBufferDesc);
		D3DVIEWPORT9 viewPort;
		viewPort.X = 0;
		viewPort.Y = mLineOffset;
		viewPort.Width = rightBufferDesc.Width;
		viewPort.Height = rightBufferDesc.Height;
		viewPort.MinZ = 0;
		viewPort.MaxZ = 1;
		mDevice->SetViewport(&viewPort);

		// Set the right quad buffer as the destination for StretchRect to the stereo buffer
		DWORD eye = ATI_STEREO_RIGHTEYE;
		sendStereoCommand(ATI_STEREO_SETDSTEYE, NULL, 0, (BYTE*)&eye, sizeof(eye));
		mDevice->StretchRect(mRightBuffer, NULL, mStereoBuffer, NULL, D3DTEXF_LINEAR);

		// Set the left buffer as the destination for StretchRect to the stereo buffer
		viewPort.Y = 0;
		mDevice->SetViewport(&viewPort);
		eye = ATI_STEREO_LEFTEYE;
		sendStereoCommand(ATI_STEREO_SETDSTEYE, NULL, 0, (BYTE*)&eye, sizeof(eye));
		mDevice->StretchRect(mLeftBuffer, NULL, mStereoBuffer, NULL, D3DTEXF_LINEAR);
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverAMD::getPresentParamsAndVerifyStereoSettings()
	{
		// Verify the swap chain exists
		UINT numberOfSwapChains = mDevice->GetNumberOfSwapChains();
		if (numberOfSwapChains < 0)
			return false;

		// Get the first swap chain
		IDirect3DSwapChain9* swapChain = NULL;
		if (FAILED(mDevice->GetSwapChain(0, &swapChain)))
			return false;

		// Cache the swap chain presentation parameters and release the swap chain
		swapChain->GetPresentParameters(&mSwapChainPresentParameters);
		swapChain->Release();

		// Check if multi sample type must be set to a value greater than 1 to enable stereo
		if (D3DMULTISAMPLE_NONE == mSwapChainPresentParameters.MultiSampleType || D3DMULTISAMPLE_NONMASKABLE == mSwapChainPresentParameters.MultiSampleType)
			return false;

		return true;
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::postViewportUpdate(const RenderTargetViewportEvent& evt)
	{
		if (!mStereoEnabled)
			return;

		// Set the correct destination buffer
		IDirect3DSurface9* destSurface = NULL;
		switch (evt.source->getDrawBuffer())
		{
		case CBT_BACK:
		case CBT_BACK_LEFT:
			destSurface = mLeftBuffer;
			break;
		case CBT_BACK_RIGHT:
			destSurface = mRightBuffer;
			break;
		}

		// Set the source rectangle
		RECT sourceRect;
		sourceRect.left = evt.source->getActualLeft();
		sourceRect.top = evt.source->getActualTop();
		sourceRect.right = evt.source->getActualWidth();
		sourceRect.bottom = evt.source->getActualHeight();

		// Copy the back buffer to the appropriate buffer
		mDevice->StretchRect(mStereoBuffer, &sourceRect, destSurface, &sourceRect, D3DTEXF_LINEAR);
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverAMD::sendStereoCommand(ATIDX9STEREOCOMMAND stereoCommand, BYTE* outBuffer, DWORD outBufferSize, BYTE* inBuffer, DWORD inBufferSize)
	{
		ATIDX9STEREOCOMMPACKET* stereoCommPacket;
		D3DLOCKED_RECT lockedRect;
		HRESULT stereoPacketResult;

		// If the input buffer exists, verfiy the size is non-zero
		if (inBuffer && inBufferSize == 0)
			return false;

		// If the output buffer exists, verfiy the size is non-zero
		if (outBuffer && outBufferSize == 0)
			return false;

		// If not already created, create a surface to be used to communicate with the driver
		if (NULL == mDriverComSurface)
		{
			// Get the active device from the render system
			D3D9RenderSystem* renderSystem = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
			IDirect3DDevice9* device = renderSystem->getActiveD3D9Device();
			if (FAILED(device->CreateOffscreenPlainSurface(10, 10, (D3DFORMAT)FOURCC_AQBS, D3DPOOL_DEFAULT, &mDriverComSurface, NULL)))
				return false;
		}

		// Lock the surface and the driver will allocate and return a pointer to a stereo packet
		if (FAILED(mDriverComSurface->LockRect(&lockedRect, 0, 0)))
			return false;

		// Assign the data to the stereo packet
		stereoCommPacket = static_cast<ATIDX9STEREOCOMMPACKET*>(lockedRect.pBits);
		stereoCommPacket->dwSignature = 'STER';
		stereoCommPacket->pResult = &stereoPacketResult;
		stereoCommPacket->stereoCommand = stereoCommand;
		stereoCommPacket->pOutBuffer = outBuffer;
		stereoCommPacket->dwOutBufferSize = outBufferSize;
		stereoCommPacket->pInBuffer = inBuffer;
		stereoCommPacket->dwInBufferSize = inBufferSize;

		// After all bits have been set, unlock the surface
		if (FAILED(mDriverComSurface->UnlockRect()))
			return false;

		// Verify the stereo packet success
		if (FAILED(stereoPacketResult))
			return false;

		return true;
	}
	//-----------------------------------------------------------------------------
	void D3D9StereoDriverAMD::releaseResources()
	{
		mLineOffset = 0;
		mStereoEnabled = false;
		mDevice = NULL;

		if (NULL != mDriverComSurface)
		{
			mDriverComSurface->Release();
			mDriverComSurface = NULL;
		}

		if (NULL != mStereoBuffer)
		{
			mStereoBuffer->Release();
			mStereoBuffer = NULL;
		}

		if (NULL != mLeftBuffer)
		{
			mLeftBuffer->Release();
			mLeftBuffer = NULL;
		}

		if (NULL != mRightBuffer)
		{
			mRightBuffer->Release();
			mRightBuffer = NULL;
		}
	}  
	//-----------------------------------------------------------------------------
}
#endif
