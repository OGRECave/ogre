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
#ifndef __D3D9HARDWAREINDEXBUFFER_H__
#define __D3D9HARDWAREINDEXBUFFER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreD3D9Resource.h"

namespace Ogre { 


    class _OgreD3D9Export D3D9HardwareIndexBuffer : public HardwareIndexBuffer, public D3D9Resource
    {
  
    public:
		D3D9HardwareIndexBuffer(HardwareBufferManagerBase* mgr, IndexType idxType, size_t numIndexes, 
			HardwareBuffer::Usage usage, bool useSystemMem, bool useShadowBuffer);
        ~D3D9HardwareIndexBuffer();
        /** See HardwareBuffer. */
        void readData(size_t offset, size_t length, void* pDest);
        /** See HardwareBuffer. */
        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);

		// Called immediately after the Direct3D device has been created.
		virtual void notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device);

		// Called before the Direct3D device is going to be destroyed.
		virtual void notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device);

		// Called immediately after the Direct3D device has entered a lost state.
		virtual void notifyOnDeviceLost(IDirect3DDevice9* d3d9Device);

		// Called immediately after the Direct3D device has been reset
		virtual void notifyOnDeviceReset(IDirect3DDevice9* d3d9Device);

		// Create the actual index buffer.
		void createBuffer(IDirect3DDevice9* d3d9Device, D3DPOOL ePool);
	
		/// Get the D3D-specific index buffer
        IDirect3DIndexBuffer9* getD3DIndexBuffer(void);		

	protected:
		struct BufferResources
		{
			IDirect3DIndexBuffer9*		mBuffer;
			bool						mOutOfDate;
			size_t						mLockOffset;
			size_t						mLockLength;
			LockOptions					mLockOptions;
			uint						mLastUsedFrame;
		};

	protected:
		/** See HardwareBuffer. */
		void* lockImpl(size_t offset, size_t length, LockOptions options);
		
		/** See HardwareBuffer. */
		void unlockImpl(void);
		
		/** Update the given buffer content.*/
		void updateBufferContent(BufferResources* bufferResources);

		// updates buffer resources from system memory buffer.
		bool updateBufferResources(const char* systemMemoryBuffer, BufferResources* bufferResources);

		/** Internal buffer lock method. */
		char* _lockBuffer(BufferResources* bufferResources, size_t offset, size_t length);

		/** Internal buffer unlock method. */
		void _unlockBuffer(BufferResources* bufferResources);

	protected:		
		typedef map<IDirect3DDevice9*, BufferResources*>::type	DeviceToBufferResourcesMap;
		typedef DeviceToBufferResourcesMap::iterator			DeviceToBufferResourcesIterator;

		DeviceToBufferResourcesMap	mMapDeviceToBufferResources;	// Map between device to buffer resources.	
		D3DINDEXBUFFER_DESC			mBufferDesc;					// Buffer description.		
		BufferResources*			mSourceBuffer;					// Source buffer resources when working with multiple devices.
		char*						mSourceLockedBytes;				// Source buffer locked bytes.
    };
}
#endif

