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

#include "Vao/OgreD3D11UavBufferPacked.h"
#include "Vao/OgreD3D11BufferInterface.h"
#include "Vao/OgreD3D11CompatBufferInterface.h"
#include "Vao/OgreD3D11VaoManager.h"
#include "Vao/OgreD3D11TexBufferPacked.h"

#include "OgreD3D11Mappings.h"
#include "OgreD3D11RenderSystem.h"

namespace Ogre
{
    D3D11UavBufferPacked::D3D11UavBufferPacked(
            size_t internalBufStartBytes, size_t numElements, uint32 bytesPerElement,
            uint32 bindFlags, void *initialData, bool keepAsShadow,
            VaoManager *vaoManager, BufferInterface *bufferInterface,
            D3D11Device &device ) :
        UavBufferPacked( internalBufStartBytes, numElements, bytesPerElement, bindFlags,
                         initialData, keepAsShadow, vaoManager, bufferInterface ),
        mDevice( device ),
        mCurrentCacheCursor( 0 )
    {
        memset( mCachedResourceViews, 0, sizeof( mCachedResourceViews ) );
    }
    //-----------------------------------------------------------------------------------
    D3D11UavBufferPacked::~D3D11UavBufferPacked()
    {
        for( int i=0; i<16; ++i )
        {
            if( mCachedResourceViews[i].mResourceView )
            {
                mCachedResourceViews[i].mResourceView->Release();
                mCachedResourceViews[i].mResourceView = 0;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    TexBufferPacked* D3D11UavBufferPacked::getAsTexBufferImpl( PixelFormat pixelFormat )
    {
        assert( dynamic_cast<D3D11CompatBufferInterface*>( mBufferInterface ) );

        D3D11CompatBufferInterface *bufferInterface = static_cast<D3D11CompatBufferInterface*>(
                                                                            mBufferInterface );


        TexBufferPacked *retVal = OGRE_NEW D3D11TexBufferPacked(
                    mInternalBufferStart * mBytesPerElement, mNumElements, mBytesPerElement, 0,
                    mBufferType, (void*)0, false, (VaoManager*)0, bufferInterface,
                    pixelFormat, mDevice );

        mTexBufferViews.push_back( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    ID3D11UnorderedAccessView* D3D11UavBufferPacked::createResourceView( int cacheIdx, uint32 offset,
                                                                         uint32 sizeBytes )
    {
        assert( cacheIdx < 16 );

        if( mCachedResourceViews[cacheIdx].mResourceView )
            mCachedResourceViews[cacheIdx].mResourceView->Release();

        mCachedResourceViews[cacheIdx].mOffset  = mFinalBufferStart + offset;
        mCachedResourceViews[cacheIdx].mSize    = sizeBytes;

        D3D11_UNORDERED_ACCESS_VIEW_DESC srDesc;

        // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
        // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        srDesc.Format               = DXGI_FORMAT_UNKNOWN;
        srDesc.ViewDimension        = D3D11_UAV_DIMENSION_BUFFER;
        srDesc.Buffer.FirstElement  = (mFinalBufferStart + offset) / mBytesPerElement;
        srDesc.Buffer.NumElements   = sizeBytes / mBytesPerElement;
        srDesc.Buffer.Flags         = 0;

        assert( dynamic_cast<D3D11CompatBufferInterface*>( mBufferInterface ) );
        D3D11CompatBufferInterface *bufferInterface = static_cast<D3D11CompatBufferInterface*>(
                    mBufferInterface );
        ID3D11Buffer *vboName = bufferInterface->getVboName();

        mDevice.get()->CreateUnorderedAccessView( vboName, &srDesc,
                                                  &mCachedResourceViews[cacheIdx].mResourceView );
        mCurrentCacheCursor = (cacheIdx + 1) % 16;

        return mCachedResourceViews[cacheIdx].mResourceView;
    }
    //-----------------------------------------------------------------------------------
    ID3D11UnorderedAccessView* D3D11UavBufferPacked::_bindBufferCommon( size_t offset, size_t sizeBytes )
    {
        assert( offset < (mNumElements - 1) );
        assert( sizeBytes < mNumElements );

        sizeBytes = !sizeBytes ? (mNumElements * mBytesPerElement - offset) : sizeBytes;

        ID3D11UnorderedAccessView *resourceView = 0;
        for( int i=0; i<16; ++i )
        {
            //Reuse resource views. Reuse res. views that are bigger than what's requested too.
            if( mFinalBufferStart + offset == mCachedResourceViews[i].mOffset &&
                sizeBytes <= mCachedResourceViews[i].mSize )
            {
                resourceView = mCachedResourceViews[i].mResourceView;
                break;
            }
            else if( !mCachedResourceViews[i].mResourceView )
            {
                //We create in-order. If we hit here, the next ones are also null pointers.
                resourceView = createResourceView( i, offset, sizeBytes );
                break;
            }
        }

        if( !resourceView )
        {
            //If we hit here, the cache is full and couldn't find a match.
            resourceView = createResourceView( mCurrentCacheCursor, offset, sizeBytes );
        }

        return resourceView;
    }
    //-----------------------------------------------------------------------------------
//    void D3D11UavBufferPacked::bindBufferVS( uint16 slot, size_t offset, size_t sizeBytes )
//    {
//        ID3D11UnorderedAccessView *resourceView = _bindBufferCommon( offset, sizeBytes );
//        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
//        deviceContext->VSSetShaderResources( slot, 1, &resourceView );
//    }
//    //-----------------------------------------------------------------------------------
//    void D3D11UavBufferPacked::bindBufferPS( uint16 slot, size_t offset, size_t sizeBytes )
//    {
//        ID3D11UnorderedAccessView *resourceView = _bindBufferCommon( offset, sizeBytes );
//        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
//        deviceContext->PSSetShaderResources( slot, 1, &resourceView );
//    }
//    //-----------------------------------------------------------------------------------
//    void D3D11UavBufferPacked::bindBufferGS( uint16 slot, size_t offset, size_t sizeBytes )
//    {
//        ID3D11UnorderedAccessView *resourceView = _bindBufferCommon( offset, sizeBytes );
//        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
//        deviceContext->GSSetShaderResources( slot, 1, &resourceView );
//    }
//    //-----------------------------------------------------------------------------------
//    void D3D11UavBufferPacked::bindBufferHS( uint16 slot, size_t offset, size_t sizeBytes )
//    {
//        ID3D11UnorderedAccessView *resourceView = _bindBufferCommon( offset, sizeBytes );
//        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
//        deviceContext->HSSetShaderResources( slot, 1, &resourceView );
//    }
//    //-----------------------------------------------------------------------------------
//    void D3D11UavBufferPacked::bindBufferDS( uint16 slot, size_t offset, size_t sizeBytes )
//    {
//        ID3D11UnorderedAccessView *resourceView = _bindBufferCommon( offset, sizeBytes );
//        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
//        deviceContext->DSSetShaderResources( slot, 1, &resourceView );
//    }
    //-----------------------------------------------------------------------------------
    void D3D11UavBufferPacked::bindBufferCS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11UnorderedAccessView *resourceView = _bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->CSSetUnorderedAccessViews( slot, 1, &resourceView, 0 );
    }
    //-----------------------------------------------------------------------------------
}
