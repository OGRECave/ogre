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
#include "OgreD3D9DriverList.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre 
{
    D3D9DriverList::D3D9DriverList()
    {
        enumerate();
    }

    D3D9DriverList::~D3D9DriverList(void)
    {
        mDriverList.clear();
    }

    BOOL D3D9DriverList::enumerate()
    {
        IDirect3D9* lpD3D9 = D3D9RenderSystem::getDirect3D9();

        LogManager::getSingleton().logMessage( "D3D9: Driver Detection Starts" );
        for( UINT iAdapter=0; iAdapter < lpD3D9->GetAdapterCount(); ++iAdapter )
        {
            D3DADAPTER_IDENTIFIER9 adapterIdentifier;
            D3DDISPLAYMODE d3ddm;
            D3DCAPS9 d3dcaps9;
            
            lpD3D9->GetAdapterIdentifier( iAdapter, 0, &adapterIdentifier );
            lpD3D9->GetAdapterDisplayMode( iAdapter, &d3ddm );
            lpD3D9->GetDeviceCaps( iAdapter, D3DDEVTYPE_HAL, &d3dcaps9 );

            mDriverList.push_back( D3D9Driver( iAdapter, d3dcaps9, adapterIdentifier, d3ddm ) );
        }

        LogManager::getSingleton().logMessage( "D3D9: Driver Detection Ends" );

        return TRUE;
    }

    size_t D3D9DriverList::count() const 
    {
        return mDriverList.size();
    }

    D3D9Driver* D3D9DriverList::item( size_t index )
    {
        return &mDriverList.at( index );
    }

    D3D9Driver* D3D9DriverList::item( const String &name )
    {
        vector<D3D9Driver>::type::iterator it = mDriverList.begin();
        if (it == mDriverList.end())
            return NULL;

        for (;it != mDriverList.end(); ++it)
        {
            if (it->DriverDescription() == name)
                return &(*it);
        }

        return NULL;
    }
}
