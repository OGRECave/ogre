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

#include "OgreD3D9Prerequisites.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#ifndef __D3D9StereoDriverAMD_H__
#define __D3D9StereoDriverAMD_H__

#include "OgreD3D9RenderSystem.h"
#include "OgreD3D9StereoDriverImpl.h"
#include "OgreRenderTargetListener.h"
#include "AtiDx9Stereo.h"

namespace Ogre {

	class D3D9RenderWindow;

	/** Virtual interface of the stereo driver*/
	class _OgreD3D9Export D3D9StereoDriverAMD : public D3D9StereoDriverImpl, public D3D9RenderSystem::Listener, public RenderTargetListener
	{
	// Interface
	public:
	  D3D9StereoDriverAMD();
	  virtual ~D3D9StereoDriverAMD();
	  virtual bool addRenderWindow(D3D9RenderWindow* renderWindow);
	  virtual bool removeRenderWindow(const String& renderWindowName);
	  virtual bool isStereoEnabled(const String& renderWindowName);
	  virtual bool setDrawBuffer(ColourBufferType colourBuffer);

	protected:
	  // D3D9RenderSystem and RenderTarget Listener Events
	  virtual void eventOccurred(const String& eventName, const NameValuePairList* parameters = 0);
	  void deviceCreatedEvent(const NameValuePairList* parameters);
	  void deviceLostEvent(const NameValuePairList* parameters);
	  void afterDeviceResetEvent(const NameValuePairList* parameters);
	  void beforeDevicePresentEvent(const NameValuePairList* parameters);
	  void postViewportUpdate(const RenderTargetViewportEvent& evt);

	  bool getPresentParamsAndVerifyStereoSettings();
	  bool sendStereoCommand(ATIDX9STEREOCOMMAND stereoCommand, BYTE* outBuffer, DWORD outBufferSize, BYTE* inBuffer, DWORD inBufferSize);
	  void releaseResources();

	  typedef map<String, D3D9RenderWindow*>::type RenderWindowMap;
	  RenderWindowMap mRenderWindowMap;

	  bool mStereoEnabled;
	  DWORD mLineOffset;
	  IDirect3DDevice9* mDevice;
	  IDirect3DSurface9* mDriverComSurface;
	  IDirect3DSurface9* mStereoBuffer;
	  IDirect3DSurface9* mLeftBuffer;
	  IDirect3DSurface9* mRightBuffer;
	  D3DPRESENT_PARAMETERS mSwapChainPresentParameters;
	};
}
#endif
#endif
