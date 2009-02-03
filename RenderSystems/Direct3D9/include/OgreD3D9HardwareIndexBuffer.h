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
#ifndef __D3D9HARDWAREINDEXBUFFER_H__
#define __D3D9HARDWAREINDEXBUFFER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreD3D9Resource.h"

namespace Ogre { 


    class D3D9HardwareIndexBuffer : public HardwareIndexBuffer, public D3D9Resource
    {
  
    public:
		D3D9HardwareIndexBuffer(IndexType idxType, size_t numIndexes, 
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
		// updates buffer resources from system memory buffer.
		bool updateBufferResources(const char* systemMemoryBuffer, BufferResources* bufferResources);

	protected:		
		typedef map<IDirect3DDevice9*, BufferResources*>::type	DeviceToBufferResourcesMap;
		typedef DeviceToBufferResourcesMap::iterator			DeviceToBufferResourcesIterator;

		DeviceToBufferResourcesMap	mMapDeviceToBufferResources;	// Map between device to buffer resources.	
		D3DINDEXBUFFER_DESC			mBufferDesc;					// Buffer description.		
		char*						mSystemMemoryBuffer;			// Consistent system memory buffer for multiple devices support.
    };
}



#endif

