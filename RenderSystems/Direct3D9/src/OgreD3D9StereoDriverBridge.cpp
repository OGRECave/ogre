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

#include "OgreD3D9StereoDriverBridge.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0

#include "OgreD3D9StereoDriverAMD.h"
#include "OgreD3D9StereoDriverNVIDIA.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	D3D9StereoDriverBridge::D3D9StereoDriverBridge(StereoModeType stereoMode)
	{
		mPimpl = NULL;
		mIsNvapiInitialized = false;
		mStereoMode = stereoMode;

		if (!mStereoMode)
			return;

		// Both initialize and set driver mode need to be called to determine whether NVIDIA stereo is available
		NvAPI_Status nvStatus = NvAPI_Initialize();
		if (NVAPI_OK == nvStatus)
			nvStatus = NvAPI_Stereo_SetDriverMode(NVAPI_STEREO_DRIVER_MODE_DIRECT);

		if (NVAPI_OK == nvStatus)
		{
			NvAPI_ShortString nvapiStatusMessage;
			NvAPI_GetErrorMessage(nvStatus, nvapiStatusMessage);
			mIsNvapiInitialized = true;
			mPimpl = new D3D9StereoDriverNVIDIA();
		}
		else
		{
			// Assume the AMD implementation, since the device must be created before verifying AMD QBS
			mPimpl = new D3D9StereoDriverAMD();
		}
	}
	//-----------------------------------------------------------------------------
	D3D9StereoDriverBridge::~D3D9StereoDriverBridge()
	{
		if (NULL != mPimpl)
		{
			delete mPimpl;
			mPimpl = NULL;
		}

		if (mIsNvapiInitialized)
		{
			NvAPI_Status nvStatus = NvAPI_Unload();
			NvAPI_ShortString nvapiStatusMessage;
			NvAPI_GetErrorMessage(nvStatus, nvapiStatusMessage);
		}
	}
	//-----------------------------------------------------------------------------
	template<> D3D9StereoDriverBridge* Ogre::Singleton<D3D9StereoDriverBridge>::msSingleton = 0;
	D3D9StereoDriverBridge& D3D9StereoDriverBridge::getSingleton(void)
	{
		assert(msSingleton);
		return (*msSingleton);
	}
	//-----------------------------------------------------------------------------
	D3D9StereoDriverBridge* D3D9StereoDriverBridge::getSingletonPtr(void)
	{
		return msSingleton;
	}
	//-----------------------------------------------------------------------------
	StereoModeType D3D9StereoDriverBridge::getStereoMode() const
	{
		return mStereoMode;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverBridge::addRenderWindow(D3D9RenderWindow* renderWindow) const
	{
		if (NULL != mPimpl)
			return mPimpl->addRenderWindow(renderWindow);

		return false;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverBridge::removeRenderWindow(const String& name) const
	{
		if (NULL != mPimpl)
			return mPimpl->removeRenderWindow(name);

		return false;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverBridge::isStereoEnabled(const String& renderWindowName) const
	{
		if (NULL != mPimpl)
			return mPimpl->isStereoEnabled(renderWindowName);

		return false;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverBridge::setDrawBuffer(ColourBufferType colourBuffer) const
	{
		if (NULL != mPimpl)
			return mPimpl->setDrawBuffer(colourBuffer);

		return false;
	}
	//-----------------------------------------------------------------------------
}
#endif

