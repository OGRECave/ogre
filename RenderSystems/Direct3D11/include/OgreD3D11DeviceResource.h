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
#ifndef __D3D11Resource_H__
#define __D3D11Resource_H__

#include "OgreD3D11Prerequisites.h"

namespace Ogre {

    /** Represents a Direct3D rendering resource.
    Provide unified interface to handle various device states.
    This class is intended to be used as protected base.
    */
    class D3D11DeviceResource
    {
    public:

        // Called immediately after the Direct3D device has entered a removed state.
        // This is the place to release device depended resources.
        virtual void notifyDeviceLost(D3D11Device* device) = 0;

        // Called immediately after the Direct3D device has been reset.
        // This is the place to create device depended resources.
        virtual void notifyDeviceRestored(D3D11Device* device) = 0;

    protected:
        D3D11DeviceResource();
        ~D3D11DeviceResource(); // protected and non-virtual
    };


    /** Singleton that is used to propagate device state changed notifications.
    This class is intended to be used as protected base.
    */
    class D3D11DeviceResourceManager
    {
    public:

        void notifyResourceCreated(D3D11DeviceResource* deviceResource);
        void notifyResourceDestroyed(D3D11DeviceResource* deviceResource);

        void notifyDeviceLost(D3D11Device* device);
        void notifyDeviceRestored(D3D11Device* device);

        static D3D11DeviceResourceManager* get();

    protected:
        D3D11DeviceResourceManager();
        ~D3D11DeviceResourceManager(); // protected and non-virtual

    private:
        std::vector<D3D11DeviceResource*> mResources, mResourcesCopy;
    };

}
#endif
