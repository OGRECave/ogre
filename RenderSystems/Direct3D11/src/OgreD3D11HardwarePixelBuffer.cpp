/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#include "OgreD3D11HardwarePixelBuffer.h"
#include "OgreD3D11Texture.h"
#include "OgreD3D11Mappings.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreD3D11Texture.h"
#include "OgreD3D11Device.h"
#include "OgreD3D11RenderSystem.h"

#include <algorithm>

namespace Ogre {

	namespace
	{
		const char* toString(TextureType textureType)
		{
			switch(textureType)
			{
			case TEX_TYPE_1D:       return "1D texture";
			case TEX_TYPE_CUBE_MAP: return "cube map texture";
			case TEX_TYPE_2D:       return "2D texture";
			case TEX_TYPE_2D_ARRAY: return "2D texture array";
			case TEX_TYPE_3D:       return "3D texture";
			default:                return "texture";
			}
		}
	}

    //-----------------------------------------------------------------------------  

    D3D11HardwarePixelBuffer::D3D11HardwarePixelBuffer(D3D11Texture * parentTexture, D3D11Device & device, UINT mipLevel,
        size_t width, size_t height, size_t depth, UINT face, PixelFormat format, HardwareBuffer::Usage usage):
    HardwarePixelBuffer(width, height, depth, format, usage, false, false),
        mParentTexture(parentTexture),
        mDevice(device),
        mFace(face),
        mMipLevel(mipLevel)
    {
        if(mUsage & TU_RENDERTARGET)
        {
            // Create render target for each slice
            mSliceTRT.reserve(mDepth);
            for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
            {
                String name;
                name = "rtt/"+StringConverter::toString((size_t)mParentTexture) + "/" + StringConverter::toString(mMipLevel) + "/" + parentTexture->getName();

                RenderTexture *trt = new D3D11RenderTexture(name, this, zoffset, mDevice);
                mSliceTRT.push_back(trt);
                Root::getSingleton().getRenderSystem()->attachRenderTarget(*trt);
            }
        }
		
		mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
		
    }
    D3D11HardwarePixelBuffer::~D3D11HardwarePixelBuffer()
    {
        if(!mSliceTRT.empty())
        {   
            // Delete all render targets that are not yet deleted via _clearSliceRTT
            for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
            {
                if(mSliceTRT[zoffset])
                    Root::getSingleton().getRenderSystem()->destroyRenderTarget(mSliceTRT[zoffset]->getName());
            }
        }
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::_map(ID3D11Resource *res, D3D11_MAP flags, PixelBox & box)
    {
        assert(mLockBox.getDepth() == 1 || mParentTexture->getTextureType() == TEX_TYPE_3D);

        D3D11_MAPPED_SUBRESOURCE pMappedResource = { 0 };

        HRESULT hr = mDevice.GetImmediateContext()->Map(res, getSubresourceIndex(mLockBox.front), flags, 0, &pMappedResource);
        if(FAILED(hr) || mDevice.isError())
        {
            String errorDescription; errorDescription
                .append("D3D11 device cannot map ").append(toString(mParentTexture->getTextureType()))
                .append("\nError Description:").append(mDevice.getErrorDescription(hr));
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, errorDescription, "D3D11HardwarePixelBuffer::_map");
        }

        D3D11Mappings::setPixelBoxMapping(box, pMappedResource);
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::_mapstagingbuffer(D3D11_MAP flags, PixelBox &box)
    {
        if(!mStagingBuffer)
            createStagingBuffer();

        if(flags == D3D11_MAP_READ_WRITE || flags == D3D11_MAP_READ || flags == D3D11_MAP_WRITE)  
        {
            if(mLockBox.getHeight() == mParentTexture->getHeight() && mLockBox.getWidth() == mParentTexture->getWidth())
                mDevice.GetImmediateContext()->CopyResource(mStagingBuffer.Get(), mParentTexture->getTextureResource());
            else
            {
                D3D11_BOX box = getSubresourceBox(mLockBox);
                UINT subresource = getSubresourceIndex(mLockBox.front);
                mDevice.GetImmediateContext()->CopySubresourceRegion(mStagingBuffer.Get(), subresource, box.left, box.top, box.front, mParentTexture->getTextureResource(), subresource, &box);
            }
        }
        else if(flags == D3D11_MAP_WRITE_DISCARD)
            flags = D3D11_MAP_WRITE; // stagingbuffer doesn't support discarding

        _map(mStagingBuffer.Get(), flags, box);
    }
    //-----------------------------------------------------------------------------  
    PixelBox D3D11HardwarePixelBuffer::lockImpl(const Image::Box &lockBox, LockOptions options)
    {
        // Check for misuse
        if(mUsage & TU_RENDERTARGET)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "DirectX does not allow locking of or directly writing to RenderTargets. Use blitFromMemory if you need the contents.",
            "D3D11HardwarePixelBuffer::lockImpl");  

        mLockBox = lockBox;

        // Set extents and format
        // Note that we do not carry over the left/top/front here, since the returned
        // PixelBox will be re-based from the locking point onwards
        PixelBox rval(lockBox.getWidth(), lockBox.getHeight(), lockBox.getDepth(), mFormat);
        // Set locking flags according to options
        D3D11_MAP  flags = D3D11_MAP_WRITE_DISCARD ;
        switch(options)
        {
        case HBL_NO_OVERWRITE:
            flags = D3D11_MAP_WRITE_NO_OVERWRITE;
            break;
        case HBL_NORMAL:
            flags = D3D11_MAP_READ_WRITE;
            break;
        case HBL_DISCARD:
            flags = D3D11_MAP_WRITE_DISCARD;
            break;
        case HBL_READ_ONLY:
            flags = D3D11_MAP_READ;
            break;
        case HBL_WRITE_ONLY:
            flags = D3D11_MAP_WRITE;
            break;
        default: 
            break;
        };

        size_t offset = 0;

        if(mUsage == HBU_STATIC || mUsage & HBU_DYNAMIC)
        {
            if(mUsage == HBU_STATIC || options == HBL_READ_ONLY || options == HBL_NORMAL || options == HBL_WRITE_ONLY)
                _mapstagingbuffer(flags, rval);
            else
                _map(mParentTexture->getTextureResource(), flags, rval);

            // calculate the offset in bytes
			offset = PixelUtil::getMemorySize(rval.left, rval.front, 1, rval.format);
            // add the offset, so the right memory will be changed
            //rval.data = static_cast<int*>(rval.data) + offset;
        }
        else
        {
            mDataForStaticUsageLock.resize(rval.getConsecutiveSize());
            rval.data = mDataForStaticUsageLock.data();
        }
        // save without offset
        mCurrentLock = rval;
        mCurrentLockOptions = options;

        // add the offset, so the right memory will be changed
		rval.data = static_cast<int*>(rval.data) + offset;	// TODO: why offsetInBytes is added to (int*) pointer ???

        return rval;
    }
    //-----------------------------------------------------------------------------
    void D3D11HardwarePixelBuffer::_unmap(ID3D11Resource *res)
    {
        mDevice.GetImmediateContext()->Unmap(res, getSubresourceIndex(mLockBox.front));
        if (mDevice.isError())
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "D3D11 device unmap resource\nError Description:" + errorDescription,
                "D3D11HardwarePixelBuffer::_unmap");
        }
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::_unmapstaticbuffer()
    {
        D3D11_BOX box = getSubresourceBox(mLockBox);
        UINT subresource = getSubresourceIndex(mLockBox.front);
        UINT srcRowPitch = PixelUtil::getMemorySize(mCurrentLock.getWidth(), 1, 1, mCurrentLock.format);
        UINT srcDepthPitch = PixelUtil::getMemorySize(mCurrentLock.getWidth(), mCurrentLock.getHeight(), 1, mCurrentLock.format); // H * rowPitch is invalid for compressed formats

        mDevice.GetImmediateContext()->UpdateSubresource(mParentTexture->getTextureResource(), subresource, &box, mDataForStaticUsageLock.data(), srcRowPitch, srcDepthPitch);
        if (mDevice.isError())
        {
            String errorDescription; errorDescription
                .append("D3D11 device cannot update staging ").append(toString(mParentTexture->getTextureType()))
                .append("\nError Description:").append(mDevice.getErrorDescription());
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, errorDescription, "D3D11HardwarePixelBuffer::_unmapstaticbuffer");
        }

        mDataForStaticUsageLock.swap(vector<int8>::type()); // i.e. shrink_to_fit
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::_unmapstagingbuffer(bool copyback)
    {
        _unmap(mStagingBuffer.Get());

        if(copyback)
        {
            if(mLockBox.getHeight() == mParentTexture->getHeight() && mLockBox.getWidth() == mParentTexture->getWidth())
                mDevice.GetImmediateContext()->CopyResource(mParentTexture->getTextureResource(), mStagingBuffer.Get());
            else
            {
                D3D11_BOX box = getSubresourceBox(mLockBox);
                UINT subresource = getSubresourceIndex(mLockBox.front);
                mDevice.GetImmediateContext()->CopySubresourceRegion(mParentTexture->getTextureResource(), subresource, box.left, box.top, box.front, mStagingBuffer.Get(), subresource, &box);
            }
        }
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::unlockImpl(void)
    {
        if(mUsage == HBU_STATIC)
            _unmapstagingbuffer();
        else if(mUsage & HBU_DYNAMIC)
        {
            if(mCurrentLockOptions == HBL_READ_ONLY || mCurrentLockOptions == HBL_NORMAL || mCurrentLockOptions == HBL_WRITE_ONLY)
            {
                PixelBox box;
                box.format = mFormat;
                _map(mParentTexture->getTextureResource(), D3D11_MAP_WRITE_DISCARD, box);
                void *data = box.data; 
				memcpy(data, mCurrentLock.data, mSizeInBytes);
                // unmap the texture and the staging buffer
                _unmap(mParentTexture->getTextureResource());
                _unmapstagingbuffer(false);
            }
            else
                _unmap(mParentTexture->getTextureResource());
        }
        else
            _unmapstaticbuffer();

        _genMipmaps();
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox)
    {
        if (srcBox.getWidth() != dstBox.getWidth()
            || srcBox.getHeight() != dstBox.getHeight()
            || srcBox.getDepth() != dstBox.getDepth())
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "D3D11 device cannot copy a subresource - source and dest size are not the same and they have to be the same in DX11.",
                "D3D11HardwarePixelBuffer::blit");
        }

        D3D11HardwarePixelBuffer * srcDx11 = static_cast<D3D11HardwarePixelBuffer *>(src.get());

        // We should blit TEX_TYPE_2D_ARRAY with depth > 1 by iterating over subresources.
        if (srcBox.getDepth() > 1 &&
            (mParentTexture->getTextureType() == TEX_TYPE_2D_ARRAY || srcDx11->mParentTexture->getTextureType() == TEX_TYPE_2D_ARRAY))
        {
            Image::Box srcSlice = srcBox, dstSlice = dstBox;
            srcSlice.back = srcSlice.front + 1;
            dstSlice.back = dstSlice.front + 1;
            for(uint32 slice = srcBox.front; slice < srcBox.back; ++slice)
            {
                blit(src, srcSlice, dstSlice); // recursive call
                ++srcSlice.front; ++srcSlice.back;
                ++dstSlice.front; ++dstSlice.back;
            }
            return;
        }

        // Do real work without extra checking - debug layer will catch erroneous parameters.
        D3D11_BOX srcBoxDx11 = srcDx11->getSubresourceBox(srcBox);
        UINT srcSubresource = srcDx11->getSubresourceIndex(srcBox.front);
        D3D11_BOX dstBoxDx11 = getSubresourceBox(dstBox);
        UINT dstSubresource = getSubresourceIndex(dstBox.front);

        mDevice.GetImmediateContext()->CopySubresourceRegion(
            mParentTexture->getTextureResource(), dstSubresource, dstBoxDx11.left, dstBoxDx11.top, dstBoxDx11.front,
            srcDx11->mParentTexture->getTextureResource(), srcSubresource, &srcBoxDx11);

        if(mDevice.isError())
        {
            String errorDescription; errorDescription
                .append("D3D11 device cannot copy to ").append(toString(mParentTexture->getTextureType()))
                .append(" subresource region from ").append(toString(srcDx11->mParentTexture->getTextureType()))
                .append("\nError Description:").append(mDevice.getErrorDescription());
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, errorDescription, "D3D11HardwarePixelBuffer::blit");
        }

        _genMipmaps();
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dst)
    {
        if (src.getWidth() != dst.getWidth()
            || src.getHeight() != dst.getHeight()
            || src.getDepth() != dst.getDepth())
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "D3D11 device cannot copy a subresource - source and dest size are not the same and they have to be the same in DX11.",
                "D3D11HardwarePixelBuffer::blitFromMemory");
        }

        // convert to pixelbuffer's native format if necessary
        if(src.format != mFormat)
        {
            vector<uint8>::type buffer;
            buffer.resize(PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), src.getDepth(), mFormat));
            PixelBox converted = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), mFormat, buffer.data());
            PixelUtil::bulkPixelConversion(src, converted);
            blitFromMemory(converted, dst); // recursive call
            return;
        }

        // We should blit TEX_TYPE_2D_ARRAY with depth > 1 by iterating over subresources.
        if (src.getDepth() > 1 && mParentTexture->getTextureType() == TEX_TYPE_2D_ARRAY)
        {
            PixelBox srcSlice = src;
            Image::Box dstSlice = dst;
            srcSlice.back = srcSlice.front + 1;
            dstSlice.back = dstSlice.front + 1;
            for(uint32 slice = src.front; slice < src.back; ++slice)
            {
                blitFromMemory(srcSlice, dstSlice); // recursive call
                ++srcSlice.front; ++srcSlice.back;
                ++dstSlice.front; ++dstSlice.back;
            }
            return;
        }

        // Do the real work
        if (mUsage & HBU_DYNAMIC) // i.e. UpdateSubresource can not be used
        {
            Ogre::PixelBox locked = lock(dst, HBL_DISCARD);
            PixelUtil::bulkPixelConversion(src, locked); // compressed formats are handled using per slice granularity, pitches are honoured
            unlock();
        }
        else
        {
            D3D11_BOX dstBox = getSubresourceBox(dst);
            UINT dstSubresource = getSubresourceIndex(dst.front);
            UINT srcRowPitch = PixelUtil::getMemorySize(src.getWidth(), 1, 1, src.format);
            UINT srcDepthPitch = PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), 1, src.format); // H * rowPitch is invalid for compressed formats

            mDevice.GetImmediateContext()->UpdateSubresource(
                mParentTexture->getTextureResource(), dstSubresource, &dstBox,
                src.getTopLeftFrontPixelPtr(), srcRowPitch, srcDepthPitch);
        }

        if(mDevice.isError())
        {
            String errorDescription; errorDescription
                .append("D3D11 device cannot update ").append(toString(mParentTexture->getTextureType()))
                .append("\nError Description:").append(mDevice.getErrorDescription());
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, errorDescription, "D3D11HardwarePixelBuffer::blitFromMemory");
        }

        _genMipmaps();
    }
    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
    {
        assert(srcBox.getDepth() == 1 && dst.getDepth() == 1);

        //This is a pointer to the texture we're trying to copy
        //Only implemented for 2D at the moment...
        ID3D11Texture2D *textureResource = mParentTexture->GetTex2D();

        // get the description of the texture
        D3D11_TEXTURE2D_DESC desc = {0};
        textureResource->GetDesc( &desc );
        //Alter the description to set up a staging texture
        desc.Usage = D3D11_USAGE_STAGING;
        //This texture is not bound to any part of the pipeline
        desc.BindFlags = 0;
        //Allow CPU Access
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        //No Misc Flags
        desc.MiscFlags = 0;
        //Create the staging texture
        ID3D11Texture2D* pStagingTexture = NULL;
        mDevice->CreateTexture2D( &desc, NULL, &pStagingTexture );
        //Copy our texture into the staging texture
        mDevice.GetImmediateContext()->CopyResource( pStagingTexture, textureResource );
        //Create a mapped resource and map the staging texture to the resource
        D3D11_MAPPED_SUBRESOURCE mapped = {0};
        mDevice.GetImmediateContext()->Map( pStagingTexture, 0, D3D11_MAP_READ , 0, &mapped );
        
        // read the data out of the texture.
        PixelBox locked = D3D11Mappings::getPixelBoxWithMapping(dst.getWidth(), dst.getHeight(), dst.getDepth(), D3D11Mappings::_getPF(desc.Format), mapped);
        PixelUtil::bulkPixelConversion(locked, dst);

        //Release the staging texture
        mDevice.GetImmediateContext()->Unmap( pStagingTexture, 0 );
        pStagingTexture->Release();
    }

    //-----------------------------------------------------------------------------  
    void D3D11HardwarePixelBuffer::_genMipmaps()
    {
        if(mParentTexture->HasAutoMipMapGenerationEnabled())
        {
            ID3D11ShaderResourceView *pShaderResourceView = mParentTexture->getTexture();
            ID3D11DeviceContextN * context =  mDevice.GetImmediateContext();
            context->GenerateMips(pShaderResourceView);
            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "D3D11 device cannot generate mips\nError Description:" + errorDescription,
                    "D3D11HardwarePixelBuffer::_genMipmaps");
            }   
        }

    }
    //-----------------------------------------------------------------------------    
    RenderTexture *D3D11HardwarePixelBuffer::getRenderTarget(size_t zoffset)
    {
        assert(mUsage & TU_RENDERTARGET);
        assert(zoffset < mDepth);
        return mSliceTRT[zoffset];
    }
    //-----------------------------------------------------------------------------    
    D3D11Texture * D3D11HardwarePixelBuffer::getParentTexture() const
    {
        return mParentTexture;
    }
    //-----------------------------------------------------------------------------    
    UINT D3D11HardwarePixelBuffer::getSubresourceIndex(size_t box_front) const
    {
        switch(mParentTexture->getTextureType())
        {
        case TEX_TYPE_CUBE_MAP: return D3D11CalcSubresource(mMipLevel, mFace, mParentTexture->getNumMipmaps() + 1);
        case TEX_TYPE_2D_ARRAY: return D3D11CalcSubresource(mMipLevel, box_front, mParentTexture->getNumMipmaps() + 1);
        }
        return mMipLevel;
    }
    //-----------------------------------------------------------------------------  
    D3D11_BOX D3D11HardwarePixelBuffer::getSubresourceBox(const Image::Box &inBox) const
    {
        // Ogre index Tex2DArray using Z component of the box, but Direct3D expect 
        // this index to be in subresource, and Z component should be sanitized
        bool is2DArray = (mParentTexture->getTextureType() == TEX_TYPE_2D_ARRAY);

        D3D11_BOX res;
        res.left    = static_cast<UINT>(inBox.left);
        res.top     = static_cast<UINT>(inBox.top);
        res.front   = is2DArray ? 0 : static_cast<UINT>(inBox.front);
        res.right   = static_cast<UINT>(inBox.right);
        res.bottom  = static_cast<UINT>(inBox.bottom);
        res.back    = is2DArray ? 1 : static_cast<UINT>(inBox.back);
        return res;
    }
    //-----------------------------------------------------------------------------    
    UINT D3D11HardwarePixelBuffer::getFace() const
    {
        return mFace;
    }
    //-----------------------------------------------------------------------------    
    void D3D11HardwarePixelBuffer::createStagingBuffer()
    {
        D3D11Texture *tex = static_cast<D3D11Texture*>(mParentTexture);

        switch (mParentTexture->getTextureType())
        {
        case TEX_TYPE_1D:
            {
                D3D11_TEXTURE1D_DESC desc;
                tex->GetTex1D()->GetDesc(&desc);

                desc.BindFlags = 0;
                desc.MiscFlags = 0;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
                desc.Usage = D3D11_USAGE_STAGING;

                mDevice->CreateTexture1D(&desc, NULL, (ID3D11Texture1D**)mStagingBuffer.ReleaseAndGetAddressOf());
            }                   
            break;
        case TEX_TYPE_2D:
        case TEX_TYPE_CUBE_MAP:
        case TEX_TYPE_2D_ARRAY:
            {
                D3D11_TEXTURE2D_DESC desc;
                tex->GetTex2D()->GetDesc(&desc);

                desc.BindFlags = 0;
                desc.MiscFlags = 0;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
                desc.Usage = D3D11_USAGE_STAGING;

                mDevice->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)mStagingBuffer.ReleaseAndGetAddressOf());
            }
            break;
        case TEX_TYPE_3D:
            {
                D3D11_TEXTURE3D_DESC desc;
                tex->GetTex3D()->GetDesc(&desc);

                desc.BindFlags = 0;
                desc.MiscFlags = 0;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
                desc.Usage = D3D11_USAGE_STAGING;

                mDevice->CreateTexture3D(&desc, NULL, (ID3D11Texture3D**)mStagingBuffer.ReleaseAndGetAddressOf());
            }
            break;
        }
    }
};
