/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __D3D9Resource_H__
#define __D3D9Resource_H__

#include "OgreD3D9Prerequisites.h"
#include "Threading/OgreThreadHeaders.h"

namespace Ogre {

	/** Represents a Direct3D rendering resource.
	Provide unified interface to
	handle various device states.
	*/
	class _OgreD3D9Export D3D9Resource
	{

	// Interface.
	public:

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device) {}

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device) {}

		// Called immediately after the Direct3D device has entered a lost state.
		// This is the place to release non-managed resources.
		virtual void notifyOnDeviceLost(IDirect3DDevice9* d3d9Device) {}

		// Called immediately after the Direct3D device has been reset.
		// This is the place to create non-managed resources.
		virtual void notifyOnDeviceReset(IDirect3DDevice9* d3d9Device) {}

		// Called when device state is changing. Access to any device should be locked.
		// Relevant for multi thread application.
		static void lockDeviceAccess();

		// Called when device state change completed. Access to any device is allowed.
		// Relevant for multi thread application.
		static void unlockDeviceAccess();


	public:
		D3D9Resource			();
		virtual ~D3D9Resource	();

	protected:
		OGRE_STATIC_MUTEX(msDeviceAccessMutex);
	};
}
#endif
