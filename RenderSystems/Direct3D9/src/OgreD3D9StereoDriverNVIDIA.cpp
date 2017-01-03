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

#include "OgreD3D9StereoDriverNVIDIA.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#include "OgreD3D9RenderWindow.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	D3D9StereoDriverNVIDIA::D3D9StereoDriverNVIDIA()
	{
		mStereoMap.clear();

		// Assume NVAPI is already initialized by caller and enable direct mode
		NvAPI_Status nvStatus = NvAPI_Stereo_SetDriverMode(NVAPI_STEREO_DRIVER_MODE_DIRECT);
		if (!logErrorMessage(nvStatus))
			return;

		// If stereo is not enabled, enable it
		nvStatus = NvAPI_Stereo_IsEnabled(&mStereoEnabled);
		if (!logErrorMessage(nvStatus))
		{
			return;
		}
		else if (!mStereoEnabled)
		{
			nvStatus = NvAPI_Stereo_Enable();
			if (!logErrorMessage(nvStatus))
				return;
		}

		// Verify stereo is now enabled
		if (!mStereoEnabled)
		{
			nvStatus = NvAPI_Stereo_IsEnabled(&mStereoEnabled);
			if (!logErrorMessage(nvStatus) || !mStereoEnabled)
				return;
		}

		logErrorMessage(nvStatus);
	}
	//-----------------------------------------------------------------------------
	D3D9StereoDriverNVIDIA::~D3D9StereoDriverNVIDIA()
	{
		StereoHandleMap::iterator i = mStereoMap.begin();
		while (i != mStereoMap.end())
		{
			removeRenderWindow((*i).first);
			i = mStereoMap.begin();
		}
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverNVIDIA::addRenderWindow(D3D9RenderWindow* renderWindow)
	{
		// Initialize the stereo handle
		OgreStereoHandle stereoHandle;
		stereoHandle.renderWindow = renderWindow;
		stereoHandle.nvapiStereoHandle = 0;
		stereoHandle.isStereoOn = 0;

		// Create a new NVAPI stereo handle and verify it is activated
		NvAPI_Status nvStatus = NvAPI_Stereo_CreateHandleFromIUnknown(stereoHandle.renderWindow->getD3D9Device(), &stereoHandle.nvapiStereoHandle);
		if (logErrorMessage(nvStatus))
			nvStatus = NvAPI_Stereo_IsActivated(stereoHandle.nvapiStereoHandle, &stereoHandle.isStereoOn);

		logErrorMessage(nvStatus);

		// Add the stereo handle to the existing map
		mStereoMap[stereoHandle.renderWindow->getName()] = stereoHandle;

		return true;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverNVIDIA::removeRenderWindow(const String& renderWindowName)
	{
		OgreStereoHandle stereoHandle = mStereoMap[renderWindowName];
		NvAPI_Status nvStatus = NvAPI_Stereo_DestroyHandle(stereoHandle.nvapiStereoHandle);
		logErrorMessage(nvStatus);
		mStereoMap.erase(renderWindowName);

		return true;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverNVIDIA::isStereoEnabled(const String& renderWindowName)
	{
		// Verify stereo is supported
		if (!mStereoEnabled)
			return false;

		// If stereo was lost for the handle, re-create the handle
		OgreStereoHandle stereoHandle;
		stereoHandle = mStereoMap[renderWindowName];
		NvU8 isStereoOn;
		NvAPI_Status nvStatus = NvAPI_Stereo_IsActivated(stereoHandle.nvapiStereoHandle, &isStereoOn);
		if (NVAPI_OK != nvStatus || !isStereoOn)
		{
			// If stereo was lost for the window, log the error message
			if (stereoHandle.isStereoOn != isStereoOn)
			{
				stereoHandle.isStereoOn = isStereoOn;
				logErrorMessage(nvStatus);
			}

			nvStatus = NvAPI_Stereo_DestroyHandle(stereoHandle.nvapiStereoHandle);
			nvStatus = NvAPI_Stereo_CreateHandleFromIUnknown(stereoHandle.renderWindow->getD3D9Device(), &stereoHandle.nvapiStereoHandle);
			if (NVAPI_OK != nvStatus)
				return false;
		}

		// Verify that stereo is activated for the handle
		nvStatus = NvAPI_Stereo_IsActivated(stereoHandle.nvapiStereoHandle, &isStereoOn);

		// If stereo was enabled for the window, log the error message
		if (stereoHandle.isStereoOn != isStereoOn)
		{
			stereoHandle.isStereoOn = isStereoOn;
			logErrorMessage(nvStatus);
		}

		return true;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverNVIDIA::setDrawBuffer(ColourBufferType colourBuffer)
	{
		NvAPI_Status nvStatus;

		// Set the active eye for all render windows that have stereo enabled
		for (StereoHandleMap::iterator i = mStereoMap.begin(); i != mStereoMap.end(); ++i)
		{
			if ((*i).second.renderWindow->isStereoEnabled())
			{
				switch (colourBuffer)
				{
				  case CBT_BACK:
					nvStatus = NvAPI_Stereo_SetActiveEye((*i).second.nvapiStereoHandle, NVAPI_STEREO_EYE_MONO);
					break;
				  case CBT_BACK_LEFT:
					nvStatus = NvAPI_Stereo_SetActiveEye((*i).second.nvapiStereoHandle, NVAPI_STEREO_EYE_LEFT);
					break;
				  case CBT_BACK_RIGHT:
					nvStatus = NvAPI_Stereo_SetActiveEye((*i).second.nvapiStereoHandle, NVAPI_STEREO_EYE_RIGHT);
					break;
				  default:
					return false;
				}

				logErrorMessage(nvStatus);
			}
		}

		return true;
	}
	//-----------------------------------------------------------------------------
	bool D3D9StereoDriverNVIDIA::logErrorMessage(NvAPI_Status nvStatus)
	{
		if (NVAPI_OK != nvStatus)
		{
			NvAPI_ShortString nvapiStatusMessage;
			NvAPI_GetErrorMessage(nvStatus, nvapiStatusMessage);
			return false;
		}

		return true;
	}
	//-----------------------------------------------------------------------------
}
#endif

