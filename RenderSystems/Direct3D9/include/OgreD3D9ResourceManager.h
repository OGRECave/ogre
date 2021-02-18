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
#ifndef __D3D9ResourceManager_H__
#define __D3D9ResourceManager_H__

#include "OgreD3D9Prerequisites.h"

namespace Ogre {

    enum D3D9ResourceCreationPolicy
    {
        /// Creates the rendering resource on the current active device that needs it.
        /// This policy should be used when video memory consumption should be minimized but
        /// it might cause some performance issues when using loader resource thread since
        /// a resource that was not created on specified device will be created on demand during
        /// the rendering process and might cause the FPS to drop down.
        RCP_CREATE_ON_ACTIVE_DEVICE,

        /// Create the rendering resource on every existing device.
        /// This policy should be used when working intensively with a background loader thread.
        /// In that case when multiple devices exist, the resource will be created on each device
        /// and will be ready to use in the rendering thread.
        /// The draw back can be some video memory waste in case that each device render different
        /// scene and doesn't really need all the resources.
        RCP_CREATE_ON_ALL_DEVICES
    };
    
    class _OgreD3D9Export D3D9ResourceManager : public ResourceAlloc
    {

    // Interface.
    public:

        // Called immediately after the Direct3D device has been created.
        void notifyOnDeviceCreate   (IDirect3DDevice9* d3d9Device);

        // Called before the Direct3D device is going to be destroyed.
        void notifyOnDeviceDestroy  (IDirect3DDevice9* d3d9Device);

        // Called immediately after the Direct3D device has entered a lost state.
        void notifyOnDeviceLost     (IDirect3DDevice9* d3d9Device);

        // Called immediately after the Direct3D device has been reset.
        void notifyOnDeviceReset    (IDirect3DDevice9* d3d9Device);

        // Called when device state is changing. Access to any device should be locked.
        // Relevant for multi thread application.
        void lockDeviceAccess       ();

        // Called when device state change completed. Access to any device is allowed.
        // Relevant for multi thread application.
        void unlockDeviceAccess     ();
        
        D3D9ResourceManager         ();
        ~D3D9ResourceManager        ();     

        void                        setCreationPolicy   (D3D9ResourceCreationPolicy creationPolicy);
        D3D9ResourceCreationPolicy  getCreationPolicy   () const;

    // Friends.
    protected:
        friend class D3D9Resource;
    
    // Types.
    protected:
        typedef std::set<D3D9Resource*>        ResourceContainer;
        typedef ResourceContainer::iterator     ResourceContainerIterator;

    // Protected methods.
    protected:
        
        // Called when new resource created.
        void _notifyResourceCreated     (D3D9Resource* pResource);

        // Called when resource is about to be destroyed.
        void _notifyResourceDestroyed   (D3D9Resource* pResource);
                

    // Attributes.
    protected:      
        OGRE_MUTEX(mResourcesMutex);
        ResourceContainer           mResources;
        D3D9ResourceCreationPolicy  mResourceCreationPolicy;
        long                        mDeviceAccessLockCount;     
    };
}

#endif
