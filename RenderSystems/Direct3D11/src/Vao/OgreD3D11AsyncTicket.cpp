/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreD3D11Prerequisites.h"

#include "Vao/OgreD3D11AsyncTicket.h"
#include "Vao/OgreD3D11VaoManager.h"

#include "Vao/OgreStagingBuffer.h"

#include "OgreD3D11Device.h"

namespace Ogre
{
    D3D11AsyncTicket::D3D11AsyncTicket( BufferPacked *creator,
                                        StagingBuffer *stagingBuffer,
                                        size_t elementStart,
                                        size_t elementCount,
                                        D3D11Device &device ) :
        AsyncTicket( creator, stagingBuffer, elementStart, elementCount ),
        mFenceName( 0 ),
        mDevice( device )
    {
        //Base constructor has already called _asyncDownload. We should now place a fence.
        mFenceName = D3D11VaoManager::createFence( mDevice );
    }
    //-----------------------------------------------------------------------------------
    D3D11AsyncTicket::~D3D11AsyncTicket()
    {
        if( mFenceName )
        {
            mFenceName->Release();
            mFenceName = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    const void* D3D11AsyncTicket::mapImpl(void)
    {
        if( mFenceName )
            mFenceName = D3D11VaoManager::waitFor( mFenceName, mDevice.GetImmediateContext() );

        return mStagingBuffer->_mapForRead( mStagingBufferMapOffset,
                                            mElementCount * mCreator->getBytesPerElement() );
    }
    //-----------------------------------------------------------------------------------
    bool D3D11AsyncTicket::queryIsTransferDone(void)
    {
        bool retVal = false;

        if( mFenceName )
        {
            HRESULT hr = mDevice.GetImmediateContext()->GetData( mFenceName, NULL, 0, 0 );

            if( FAILED( hr ) )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Failure while waiting for a D3D11 Fence. Could be out of GPU memory. "
                             "Update your video card drivers. If that doesn't help, "
                             "contact the developers.",
                             "D3D11VaoManager::queryIsTransferDone" );
            }

            retVal = hr != S_FALSE;
        }
        else
        {
            retVal = true;
        }

        return retVal;
    }
}
