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

#include "Vao/OgreD3D11CompatBufferInterface.h"
#include "Vao/OgreD3D11VaoManager.h"
#include "Vao/OgreD3D11StagingBuffer.h"
#include "Vao/OgreD3D11DynamicBuffer.h"

#include "OgreD3D11Device.h"
#include "OgreD3D11RenderSystem.h"

namespace Ogre
{
    D3D11CompatBufferInterface::D3D11CompatBufferInterface( size_t vboPoolIdx, ID3D11Buffer *d3dBuffer,
                                                          D3D11Device &device ) :
        mVboPoolIdx( vboPoolIdx ),
        mVboName( d3dBuffer ),
        mMappedPtr( 0 ),
        mDevice( device )
    {
    }
    //-----------------------------------------------------------------------------------
    D3D11CompatBufferInterface::~D3D11CompatBufferInterface()
    {
    }
    //-----------------------------------------------------------------------------------
    void* RESTRICT_ALIAS_RETURN D3D11CompatBufferInterface::map( size_t elementStart,
                                                                 size_t elementCount,
                                                                 MappingState prevMappingState,
                                                                 bool bAdvanceFrame )
    {
        assert( elementStart <= mBuffer->mNumElements &&
                elementStart + elementCount <= mBuffer->mNumElements );

        D3D11RenderSystem *renderSystem = static_cast<D3D11VaoManager*>( mBuffer->mVaoManager )->
                getD3D11RenderSystem();

        D3D11_MAP mapFlag = D3D11_MAP_WRITE_NO_OVERWRITE;

        if( renderSystem->_getFeatureLevel() <= D3D_FEATURE_LEVEL_11_0 ||
            (renderSystem->_getFeatureLevel() > D3D_FEATURE_LEVEL_11_0 &&
            elementStart <= mBuffer->mLastMappingStart + mBuffer->mLastMappingCount) )
        {
            //D3D 11.0 and below doesn't support NO_OVERWRITE
            //On D3D 11.1, only use discard when wrapping back.
            mapFlag = D3D11_MAP_WRITE_DISCARD;
        }

        D3D11_MAPPED_SUBRESOURCE mappedSubres;
        mDevice.GetImmediateContext()->Map( mVboName, 0, mapFlag, 0, &mappedSubres );
        mMappedPtr = reinterpret_cast<uint8*>( mappedSubres.pData ) +
                elementStart * mBuffer->mBytesPerElement;

        //Store the last map start so we know when to discard on the next map.
        mBuffer->mLastMappingStart = elementStart + elementCount;
        //Store the count for the assert in unmap
        mBuffer->mLastMappingCount = elementCount;

        return mMappedPtr;
    }
    //-----------------------------------------------------------------------------------
    void D3D11CompatBufferInterface::unmap( UnmapOptions unmapOption,
                                            size_t flushStartElem, size_t flushSizeElem )
    {
        //All arguments aren't really used by D3D11, these asserts are for the other APIs.
        assert( flushStartElem <= mBuffer->mLastMappingCount &&
                "Flush starts after the end of the mapped region!" );
        assert( flushStartElem + flushSizeElem <= mBuffer->mLastMappingCount &&
                "Flush region out of bounds!" );

        mDevice.GetImmediateContext()->Unmap( mVboName, 0 );
        mMappedPtr = 0;
    }
    //-----------------------------------------------------------------------------------
    void D3D11CompatBufferInterface::advanceFrame(void)
    {
    }
    //-----------------------------------------------------------------------------------
    void D3D11CompatBufferInterface::regressFrame(void)
    {
    }
}
