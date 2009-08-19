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
		OGRE_LOCK_MUTEX(mResourcesMutex)

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
		OGRE_LOCK_MUTEX(mResourcesMutex)

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
		OGRE_LOCK_MUTEX(mResourcesMutex)

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
		OGRE_LOCK_MUTEX(mResourcesMutex)

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
		OGRE_LOCK_MUTEX(mResourcesMutex)		
		mResources.push_back(pResource);
	}
	
	//-----------------------------------------------------------------------
	void D3D9ResourceManager::_notifyResourceDestroyed(D3D9Resource* pResource)
	{		
		OGRE_LOCK_MUTEX(mResourcesMutex)

		ResourceContainerIterator it = mResources.begin();

		while (it != mResources.end())
		{
			if ((*it) == pResource)
			{
				mResources.erase(it);
				break;
			}			
			++it;
		}	
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
