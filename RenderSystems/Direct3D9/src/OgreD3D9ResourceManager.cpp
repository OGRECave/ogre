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
#include "OgreD3D9Resource.h"
#include "OgreD3D9ResourceManager.h"
#include "OgreD3D9HardwarePixelBuffer.h"

namespace Ogre
{
	
	//-----------------------------------------------------------------------
	D3D9ResourceManager::D3D9ResourceManager()
	{
		mResourceCreationPolicy = RCP_CREATE_ON_ALL_DEVICES;
		mDeviceAccessLockCount = 0;
		mAutoHardwareBufferManagement = false;
	}

	//-----------------------------------------------------------------------
	D3D9ResourceManager::~D3D9ResourceManager()
	{
	
	}
 
	//-----------------------------------------------------------------------
	void D3D9ResourceManager::setCreationPolicy(D3D9ResourceCreationPolicy creationPolicy)
	{
		mResourceCreationPolicy = creationPolicy;
	}

	//-----------------------------------------------------------------------
	D3D9ResourceCreationPolicy D3D9ResourceManager::getCreationPolicy() const
	{
		return mResourceCreationPolicy;
	}

	 //-----------------------------------------------------------------------
	void D3D9ResourceManager::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{				
            OGRE_LOCK_MUTEX(mResourcesMutex);

		ResourceContainerIterator it = mResources.begin();
		while (it != mResources.end())
		{
			(*it)->notifyOnDeviceCreate(d3d9Device);
			++it;
		}				
	}

	 //-----------------------------------------------------------------------
	void D3D9ResourceManager::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
            OGRE_LOCK_MUTEX(mResourcesMutex);

		ResourceContainerIterator it = mResources.begin();
		while (it != mResources.end())
		{
			(*it)->notifyOnDeviceDestroy(d3d9Device);
			++it;
		}	
	}

	 //-----------------------------------------------------------------------
	void D3D9ResourceManager::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device)
	{
            OGRE_LOCK_MUTEX(mResourcesMutex);

		ResourceContainerIterator it = mResources.begin();
		while (it != mResources.end())
		{
			(*it)->notifyOnDeviceLost(d3d9Device);
			++it;
		}	
	}

	 //-----------------------------------------------------------------------
	void D3D9ResourceManager::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device)
	{		
            OGRE_LOCK_MUTEX(mResourcesMutex);

		ResourceContainerIterator it = mResources.begin();
		while (it != mResources.end())
		{
			(*it)->notifyOnDeviceReset(d3d9Device);
			++it;			
		}		
	}

	//-----------------------------------------------------------------------
	void D3D9ResourceManager::_notifyResourceCreated(D3D9Resource* pResource)
	{		
            OGRE_LOCK_MUTEX(mResourcesMutex);
		mResources.insert(pResource);
	}
	
	//-----------------------------------------------------------------------
	void D3D9ResourceManager::_notifyResourceDestroyed(D3D9Resource* pResource)
	{		
            OGRE_LOCK_MUTEX(mResourcesMutex);

		mResources.erase(pResource);
	}
	
	//-----------------------------------------------------------------------
	void D3D9ResourceManager::lockDeviceAccess()
	{	
		assert(mDeviceAccessLockCount >= 0);
		mDeviceAccessLockCount++;
		if (mDeviceAccessLockCount == 1)
		{					
			OGRE_LOCK_RECURSIVE_MUTEX(mResourcesMutex);
			D3D9Resource::lockDeviceAccess();
			D3D9HardwarePixelBuffer::lockDeviceAccess();
		}
	}

	//-----------------------------------------------------------------------
	void D3D9ResourceManager::unlockDeviceAccess()
	{
		assert(mDeviceAccessLockCount > 0);		
		mDeviceAccessLockCount--;				
		if (mDeviceAccessLockCount == 0)
		{						
			D3D9HardwarePixelBuffer::unlockDeviceAccess();
			D3D9Resource::unlockDeviceAccess();			
			OGRE_UNLOCK_RECURSIVE_MUTEX(mResourcesMutex);			
		}
	}
}
