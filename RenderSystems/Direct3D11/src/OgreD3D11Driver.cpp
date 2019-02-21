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
#include "OgreD3D11Driver.h"
#include "OgreD3D11VideoModeList.h"
#include "OgreD3D11VideoMode.h"
#include "OgreD3D11Device.h"
#include "OgreString.h"
#include "OgreStringConverter.h"

#ifndef DXGI_ADAPTER_FLAG_SOFTWARE
#define DXGI_ADAPTER_FLAG_SOFTWARE 2 // unavailable in June 2010 SDK
#endif

namespace Ogre
{
    //---------------------------------------------------------------------
    D3D11Driver::D3D11Driver() 
    {
        ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
        mSameNameAdapterIndex = 0;
    }
    //---------------------------------------------------------------------
    D3D11Driver::D3D11Driver(IDXGIAdapterN* pDXGIAdapter, const DXGI_ADAPTER_DESC1& desc, unsigned sameNameIndex)
    {
        mDXGIAdapter = pDXGIAdapter;
        mAdapterIdentifier = desc;
        mSameNameAdapterIndex = sameNameIndex;
    }
    //---------------------------------------------------------------------
    D3D11Driver::~D3D11Driver()
    {
    }
    //---------------------------------------------------------------------
    String D3D11Driver::DriverDescription() const
    {
        char str[sizeof(mAdapterIdentifier.Description) + 1];
        wcstombs(str, mAdapterIdentifier.Description, sizeof(str) - 1);
        str[sizeof(str) - 1] = '\0';
        String driverDescription=str;
        StringUtil::trim(driverDescription);
        if(mAdapterIdentifier.VendorId == 0x1414 && mAdapterIdentifier.DeviceId == 0x8c && (mAdapterIdentifier.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            driverDescription += " (software)"; // It`s software only variation of "Microsoft Basic Render Driver", always present since Win8
        else if(mSameNameAdapterIndex != 0)
            driverDescription += " (" + Ogre::StringConverter::toString(mSameNameAdapterIndex + 1) + ")";

        return driverDescription;
    }
    //---------------------------------------------------------------------
    D3D11VideoModeList* D3D11Driver::getVideoModeList()
    {
        if(!mVideoModeList)
            mVideoModeList = SharedPtr<D3D11VideoModeList>(OGRE_NEW_T(D3D11VideoModeList, MEMCATEGORY_GENERAL)(getDeviceAdapter()), SPFM_DELETE_T);

        return mVideoModeList.get();
    }
    //---------------------------------------------------------------------
    const DXGI_ADAPTER_DESC1& D3D11Driver::getAdapterIdentifier() const
    {
        return mAdapterIdentifier;
    }
    //---------------------------------------------------------------------
    IDXGIAdapterN* D3D11Driver::getDeviceAdapter() const
    {
        return mDXGIAdapter.Get();
    }
    //---------------------------------------------------------------------
}
