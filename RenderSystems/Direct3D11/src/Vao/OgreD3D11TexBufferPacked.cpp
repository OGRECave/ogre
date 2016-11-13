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

#include "Vao/OgreD3D11TexBufferPacked.h"
#include "Vao/OgreD3D11BufferInterface.h"
#include "Vao/OgreD3D11CompatBufferInterface.h"
#include "Vao/OgreD3D11VaoManager.h"

#include "OgreD3D11Mappings.h"
#include "OgreD3D11RenderSystem.h"

namespace Ogre
{
    D3D11TexBufferPacked::D3D11TexBufferPacked(
            size_t internalBufStartBytes, size_t numElements, uint32 bytesPerElement,
            uint32 numElementsPadding, BufferType bufferType, void *initialData, bool keepAsShadow,
            VaoManager *vaoManager, BufferInterface *bufferInterface, PixelFormat pf,
            D3D11Device &device ) :
        TexBufferPacked( internalBufStartBytes, numElements, bytesPerElement, numElementsPadding,
                         bufferType, initialData, keepAsShadow, vaoManager, bufferInterface, pf ),
        mInternalFormat( DXGI_FORMAT_UNKNOWN ),
        mDevice( device ),
        mCurrentCacheCursor( 0 )
    {
        memset( mCachedResourceViews, 0, sizeof( mCachedResourceViews ) );

        mInternalFormat = D3D11Mappings::_getPF( pf );
    }
    //-----------------------------------------------------------------------------------
    D3D11TexBufferPacked::~D3D11TexBufferPacked()
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
    ID3D11ShaderResourceView* D3D11TexBufferPacked::createResourceView( int cacheIdx, uint32 offset,
                                                                        uint32 sizeBytes )
    {
        assert( cacheIdx < 16 );

        const size_t formatSize = PixelUtil::getNumElemBytes( mPixelFormat );

        if( mCachedResourceViews[cacheIdx].mResourceView )
            mCachedResourceViews[cacheIdx].mResourceView->Release();

        mCachedResourceViews[cacheIdx].mOffset  = mFinalBufferStart + offset;
        mCachedResourceViews[cacheIdx].mSize    = sizeBytes;

        D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;

        srDesc.Format               = mInternalFormat;
        srDesc.ViewDimension        = D3D11_SRV_DIMENSION_BUFFER;
        srDesc.Buffer.FirstElement  = (mFinalBufferStart + offset) / formatSize;
        srDesc.Buffer.NumElements   = sizeBytes / formatSize;

        D3D11RenderSystem *rs = static_cast<D3D11VaoManager*>(mVaoManager)->getD3D11RenderSystem();
        ID3D11Buffer *vboName = 0;

        if( rs->_getFeatureLevel() > D3D_FEATURE_LEVEL_11_0 )
        {
            assert( dynamic_cast<D3D11BufferInterface*>( mBufferInterface ) );
            D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                        mBufferInterface );
            vboName = bufferInterface->getVboName();
        }
        else
        {
            assert( dynamic_cast<D3D11CompatBufferInterface*>( mBufferInterface ) );
            D3D11CompatBufferInterface *bufferInterface = static_cast<D3D11CompatBufferInterface*>(
                        mBufferInterface );
            vboName = bufferInterface->getVboName();
        }

        mDevice.get()->CreateShaderResourceView( vboName, &srDesc,
                                                 &mCachedResourceViews[cacheIdx].mResourceView );

        mCurrentCacheCursor = (cacheIdx + 1) % 16;

        return mCachedResourceViews[cacheIdx].mResourceView;
    }
    //-----------------------------------------------------------------------------------
    ID3D11ShaderResourceView* D3D11TexBufferPacked::bindBufferCommon( size_t offset, size_t sizeBytes )
    {
        assert( offset < (mNumElements - 1) );
        assert( (offset + sizeBytes) <= mNumElements );

        sizeBytes = !sizeBytes ? (mNumElements - offset) : sizeBytes;

        ID3D11ShaderResourceView *resourceView = 0;
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
    void D3D11TexBufferPacked::bindBufferVS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11ShaderResourceView *resourceView = bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->VSSetShaderResources( slot, 1, &resourceView );
    }
    //-----------------------------------------------------------------------------------
    void D3D11TexBufferPacked::bindBufferPS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11ShaderResourceView *resourceView = bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->PSSetShaderResources( slot, 1, &resourceView );
    }
    //-----------------------------------------------------------------------------------
    void D3D11TexBufferPacked::bindBufferGS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11ShaderResourceView *resourceView = bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->GSSetShaderResources( slot, 1, &resourceView );
    }
    //-----------------------------------------------------------------------------------
    void D3D11TexBufferPacked::bindBufferHS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11ShaderResourceView *resourceView = bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->HSSetShaderResources( slot, 1, &resourceView );
    }
    //-----------------------------------------------------------------------------------
    void D3D11TexBufferPacked::bindBufferDS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11ShaderResourceView *resourceView = bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->DSSetShaderResources( slot, 1, &resourceView );
    }
    //-----------------------------------------------------------------------------------
    void D3D11TexBufferPacked::bindBufferCS( uint16 slot, size_t offset, size_t sizeBytes )
    {
        ID3D11ShaderResourceView *resourceView = bindBufferCommon( offset, sizeBytes );
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
        deviceContext->CSSetShaderResources( slot, 1, &resourceView );
    }
    //-----------------------------------------------------------------------------------
}
