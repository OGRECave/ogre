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
#include "OgreD3D9DepthBuffer.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
    D3D9DepthBuffer::D3D9DepthBuffer( uint16 poolId, D3D9RenderSystem *renderSystem,
                                      IDirect3DDevice9 *creator, IDirect3DSurface9 *depthBufferSurf,
                                      D3DFORMAT fmt, uint32 width, uint32 height,
                                      uint32 fsaa, uint32 multiSampleQuality, bool isManual ) :
                DepthBuffer( poolId, width, height, fsaa, isManual ),
                mDepthBuffer( depthBufferSurf ),
                mCreator( creator ),
                mMultiSampleQuality( multiSampleQuality ),
                mD3DFormat( fmt ),
                mRenderSystem( renderSystem )
    {
    }

    D3D9DepthBuffer::~D3D9DepthBuffer()
    {
        if( !mManual )
            mDepthBuffer->Release();
        mDepthBuffer = 0;

        mCreator = 0;
    }
    //---------------------------------------------------------------------
    bool D3D9DepthBuffer::isCompatible( RenderTarget *renderTarget ) const
    {
        IDirect3DSurface9* pBack[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        memset( pBack, 0, sizeof(pBack) );
        renderTarget->getCustomAttribute( "DDBACKBUFFER", &pBack );
        if( !pBack[0] )
            return false;

        D3DSURFACE_DESC srfDesc;
        if( SUCCEEDED(pBack[0]->GetDesc(&srfDesc)) )
        {
            //RenderSystem will determine if bitdepths match (i.e. 32 bit RT don't like 16 bit Depth)
            //This is the same function used to create them. Note results are usually cached so this should
            //be quick
            D3DFORMAT fmt = mRenderSystem->_getDepthStencilFormatFor( srfDesc.Format );
            IDirect3DDevice9 *activeDevice = mRenderSystem->getActiveD3D9Device();

            if( mCreator == activeDevice &&
                fmt == mD3DFormat &&
                mFsaa == srfDesc.MultiSampleType &&
                mMultiSampleQuality == srfDesc.MultiSampleQuality &&
                this->getWidth() >= renderTarget->getWidth() &&
                this->getHeight() >= renderTarget->getHeight() )
            {
                return true;
            }
        }

        return false;
    }
    //---------------------------------------------------------------------
    IDirect3DDevice9* D3D9DepthBuffer::getDeviceCreator() const
    {
        return mCreator;
    }
    //---------------------------------------------------------------------
    IDirect3DSurface9* D3D9DepthBuffer::getDepthBufferSurface() const
    {
        return mDepthBuffer;
    }
}
