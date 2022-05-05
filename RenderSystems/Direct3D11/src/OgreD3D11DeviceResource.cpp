/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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
#include "OgreD3D11DeviceResource.h"


namespace Ogre
{
    D3D11DeviceResource::D3D11DeviceResource()
    {
        D3D11DeviceResourceManager::get()->notifyResourceCreated(this);
    }

    D3D11DeviceResource::~D3D11DeviceResource()
    {
        D3D11DeviceResourceManager::get()->notifyResourceDestroyed(this);
    }

    // ------------------------------------------------------------------------
    D3D11DeviceResourceManager* gs_D3D11DeviceResourceManager = NULL;

    D3D11DeviceResourceManager* D3D11DeviceResourceManager::get()
    {
        return gs_D3D11DeviceResourceManager;
    }

    D3D11DeviceResourceManager::D3D11DeviceResourceManager()
    {
        assert(gs_D3D11DeviceResourceManager == NULL);
        gs_D3D11DeviceResourceManager = this;
    }

    D3D11DeviceResourceManager::~D3D11DeviceResourceManager()
    {
        OgreAssertDbg(mResources.empty(),
                      "Not all resources have been released! Are you holding on to a reference somewhere?");
        assert(gs_D3D11DeviceResourceManager == this);
        gs_D3D11DeviceResourceManager = NULL;
    }

    void D3D11DeviceResourceManager::notifyResourceCreated(D3D11DeviceResource* deviceResource)
    {
        assert(std::find(mResources.begin(), mResources.end(), deviceResource) == mResources.end());
        mResources.push_back(deviceResource);
    }

    void D3D11DeviceResourceManager::notifyResourceDestroyed(D3D11DeviceResource* deviceResource)
    {
        std::vector<D3D11DeviceResource*>::iterator it = std::find(mResources.begin(), mResources.end(), deviceResource);
        assert(it != mResources.end());
        mResources.erase(it);

        std::vector<D3D11DeviceResource*>::iterator itCopy = std::find(mResourcesCopy.begin(), mResourcesCopy.end(), deviceResource);
        if(itCopy != mResourcesCopy.end())
            *itCopy = NULL;
    }

    void D3D11DeviceResourceManager::notifyDeviceLost(D3D11Device* device)
    {
        assert(mResourcesCopy.empty()); // reentrancy is not expected nor supported
        mResourcesCopy = mResources;
        for(std::vector<D3D11DeviceResource*>::iterator it = mResourcesCopy.begin(), it_end = mResourcesCopy.end(); it != it_end; ++it)
            if(D3D11DeviceResource* deviceResource = *it)
                deviceResource->notifyDeviceLost(device);
        mResourcesCopy.clear();
    }

    void D3D11DeviceResourceManager::notifyDeviceRestored(D3D11Device* device)
    {
        assert(mResourcesCopy.empty()); // reentrancy is not expected nor supported
        mResourcesCopy = mResources;
        for(std::vector<D3D11DeviceResource*>::iterator it = mResourcesCopy.begin(), it_end = mResourcesCopy.end(); it != it_end; ++it)
            if(D3D11DeviceResource* deviceResource = *it)
                deviceResource->notifyDeviceRestored(device);
        mResourcesCopy.clear();
    }

}
