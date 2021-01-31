/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreMetalHardwarePixelBuffer.h"
#include "OgreMetalRenderTexture.h"
#include "OgreMetalRenderSystem.h"
#include "OgreRoot.h"

#include "OgreMetalMappings.h"

#import <Metal/MTLBlitCommandEncoder.h>

namespace Ogre {
    MetalHardwarePixelBuffer::MetalHardwarePixelBuffer( uint32 width, uint32 height,
                                                        uint32 depth, PixelFormat format,
                                                        bool hwGamma, HardwareBuffer::Usage usage )
        : HardwarePixelBuffer(width, height, depth, format, usage, false, false),
          mBuffer(width, height, depth, format)
    {
    }

    MetalHardwarePixelBuffer::~MetalHardwarePixelBuffer()
    {
        // Force free buffer
        delete [] (uint8*)mBuffer.data;
    }

    void MetalHardwarePixelBuffer::allocateBuffer()
    {
        if (mBuffer.data)
            // Already allocated
            return;

        mBuffer.data = new uint8[mSizeInBytes];
    }

    void MetalHardwarePixelBuffer::freeBuffer()
    {
        // Free buffer if we're STATIC to save memory
        if (mUsage & HBU_STATIC)
        {
            delete [] (uint8*)mBuffer.data;
            mBuffer.data = 0;
        }
    }

    PixelBox MetalHardwarePixelBuffer::lockImpl(const Box &lockBox,  LockOptions options)
    {
        allocateBuffer();
        if (options != HardwareBuffer::HBL_DISCARD)
        {
            // Download the old contents of the texture
            download(mBuffer);
        }
        return mBuffer.getSubVolume(lockBox);
    }

    void MetalHardwarePixelBuffer::unlockImpl(void)
    {
        if (mCurrentLockOptions != HardwareBuffer::HBL_READ_ONLY)
        {
            // From buffer to card, only upload if was locked for writing
            upload(mCurrentLock, mLockedBox);
        }
        freeBuffer();
    }

    void MetalHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Box &dstBox)
    {
        if (!mBuffer.contains(dstBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Destination box out of range",
                        "MetalHardwarePixelBuffer::blitFromMemory");
        }

        bool freeScaledBuffer = false;
        PixelBox scaled;

        if (src.getWidth() != dstBox.getWidth() ||
            src.getHeight() != dstBox.getHeight() ||
            src.getDepth() != dstBox.getDepth())
        {
            // Scale to destination size.
            // This also does pixel format conversion if needed
            allocateBuffer();
            scaled = mBuffer.getSubVolume(dstBox);
            Image::scale(src, scaled, Image::FILTER_BILINEAR);
        }
        else
        {
            allocateBuffer();

            // No scaling or conversion needed
            scaled = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), src.format, src.data);

            if (src.format == PF_R8G8B8 || src.format == PF_B8G8R8)
            {
                const PixelFormat newFormat = src.format == PF_R8G8B8 ? PF_X8R8G8B8 : PF_X8B8G8R8;
                freeScaledBuffer = true;
                size_t scaledSize = PixelUtil::getMemorySize( src.getWidth(), src.getHeight(),
                                                              src.getDepth(), newFormat );
                scaled.format = newFormat;
                scaled.data = new uint8[scaledSize];
                scaled.setConsecutive();
                PixelUtil::bulkPixelConversion(src, scaled);
            }
            else if (src.format == PF_BYTE_LA)
            {
                freeScaledBuffer = true;
                size_t scaledSize = PixelUtil::getMemorySize( src.getWidth(), src.getHeight(),
                                                              src.getDepth(), PF_A8R8G8B8 );
                scaled.format = PF_A8R8G8B8;
                scaled.data = new uint8[scaledSize];
                scaled.setConsecutive();
                PixelUtil::bulkPixelConversion(src, scaled);
            }
        }

        upload(scaled, dstBox);
        freeBuffer();

        if (freeScaledBuffer)
        {
            delete[] (uint8*)scaled.data;
        }
    }

    void MetalHardwarePixelBuffer::blitToMemory(const Box &srcBox, const PixelBox &dst)
    {
        if (!mBuffer.contains(srcBox))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "source box out of range",
                        "MetalHardwarePixelBuffer::blitToMemory");
        }

        if( srcBox.left == 0 && srcBox.right == getWidth() &&
            srcBox.top == 0 && srcBox.bottom == getHeight() &&
            srcBox.front == 0 && srcBox.back == getDepth() &&
            dst.getWidth() == getWidth() &&
            dst.getHeight() == getHeight() &&
            dst.getDepth() == getDepth() )
        {
            // The direct case: the user wants the entire texture in a format supported by Metal
            // so we don't need an intermediate buffer
            download(dst);
        }
        else
        {
            // Use buffer for intermediate copy
            allocateBuffer();
            // Download entire buffer
            download(mBuffer);
            if(srcBox.getWidth() != dst.getWidth() ||
                srcBox.getHeight() != dst.getHeight() ||
                srcBox.getDepth() != dst.getDepth())
            {
                // We need scaling
                Image::scale(mBuffer.getSubVolume(srcBox), dst, Image::FILTER_BILINEAR);
            }
            else
            {
                // Just copy the bit that we need
                PixelUtil::bulkPixelConversion(mBuffer.getSubVolume(srcBox), dst);
            }
            freeBuffer();
        }
    }

    void MetalHardwarePixelBuffer::upload(const PixelBox &data, const Box &dest)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Upload not possible for this pixelbuffer type",
                    "MetalHardwarePixelBuffer::upload");
    }

    void MetalHardwarePixelBuffer::download(const PixelBox &data)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Download not possible for this pixelbuffer type",
                    "MetalHardwarePixelBuffer::download");
    }

    void MetalHardwarePixelBuffer::bindToFramebuffer(uint32 attachment, size_t zoffset)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "Framebuffer bind not possible for this pixelbuffer type",
                    "MetalHardwarePixelBuffer::bindToFramebuffer");
    }

    // TextureBuffer
    MetalTextureBuffer::MetalTextureBuffer( __unsafe_unretained id<MTLTexture> renderTexture,
                                            __unsafe_unretained id<MTLTexture> resolveTexture,
                                            MetalDevice *device,
                                            const String &baseName, MTLTextureType target,
                                            int width, int height, int depth, PixelFormat format,
                                            int face, int level, Usage usage,
                                            bool writeGamma, uint fsaa ) :
        MetalHardwarePixelBuffer(0, 0, 0, format, writeGamma, usage),
        mTexture( renderTexture ? renderTexture : resolveTexture ),
        mTarget( target ),
        mBufferId( 0 ),
        mFace( face ),
        mLevel( level )
    {
        // Calculate the width and height of the texture at this mip level
        mWidth = mLevel == 0 ? width : width / static_cast<size_t>(pow(2.0f, level));
        mHeight = mLevel == 0 ? height : height / static_cast<size_t>(pow(2.0f, level));
        if(mWidth < 1)
            mWidth = 1;
        if(mHeight < 1)
            mHeight = 1;

        if(target != MTLTextureType3D && target != MTLTextureType2DArray)
            mDepth = 1; // Depth always 1 for non-3D textures
        else
            mDepth = depth;

#if OGRE_DEBUG_MODE
        // Log a message
        std::stringstream str;
        str << "MetalHardwarePixelBuffer constructed for texture " << baseName
            << " face " << mFace << " level " << mLevel << ":"
            << " width=" << mWidth << " height="<< mHeight << " depth=" << mDepth
            << " format=" << PixelUtil::getFormatName(mFormat)
            << " MTLformat=" << format
            << " rowPitch=" << mRowPitch;
        LogManager::getSingleton().logMessage(LML_NORMAL, str.str());
#endif

        // Set up a pixel box
        mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);

        if (mWidth == 0 || mHeight == 0 || mDepth == 0)
            // We are invalid, do not allocate a buffer
            return;

        // Is this a render target?
        if (mUsage & TU_RENDERTARGET)
        {
            // Create render target for each slice
            mSliceTRT.reserve(mDepth);
            for(uint32 zoffset=0; zoffset<mDepth; ++zoffset)
            {
                String name;
                name = "rtt/" + StringConverter::toString((size_t)this) + "/" + baseName;
                RenderTexture *trt = OGRE_NEW MetalRenderTexture( device, name, this,
                                                                  renderTexture, resolveTexture,
                                                                  mFormat, zoffset, mFace, fsaa, level,
                                                                  mHwGamma );
                mSliceTRT.push_back(trt);
                // RenderTexture** val = &mSliceTRT[0];
                Root::getSingleton().getRenderSystem()->attachRenderTarget(*mSliceTRT[zoffset]);
            }
        }
    }

    MetalTextureBuffer::~MetalTextureBuffer()
    {
        if (mUsage & TU_RENDERTARGET)
        {
            // Delete all render targets that are not yet deleted via _clearSliceRTT because the rendertarget
            // was deleted by the user.
            for (SliceTRT::const_iterator it = mSliceTRT.begin(); it != mSliceTRT.end(); ++it)
            {
                if( *it )
                    Root::getSingleton().getRenderSystem()->destroyRenderTarget((*it)->getName());
            }

            mSliceTRT.clear();
        }
    }

    void MetalTextureBuffer::upload(const PixelBox &data, const Box &dest)
    {
        if(PixelUtil::isCompressed(data.format) && (NSUInteger)mLevel == mTexture.mipmapLevelCount)
            return;

        // Calculate size for all mip levels of the texture
        size_t bytesPerImage = PixelUtil::getMemorySize( dest.getWidth(), dest.getHeight(),
                                                         1, data.format );

        if (!PixelUtil::isCompressed(data.format))
        {
            if (data.getWidth() != data.rowPitch)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unsupported texture format",
                            "MetalTextureBuffer::upload");
            }

            if (data.getHeight() * data.getWidth() != data.slicePitch)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Unsupported texture format",
                            "MetalTextureBuffer::upload");
            }
        }

        NSUInteger rowPitch = PixelUtil::getMemorySize(data.getWidth(), 1, 1, data.format);

        // PVR textures should have 0 row size and data size
        if( data.format == PF_PVRTC2_2BPP ||
            data.format == PF_PVRTC2_4BPP ||
            data.format == PF_PVRTC_RGB2 ||
            data.format == PF_PVRTC_RGB4 ||
            data.format == PF_PVRTC_RGBA2 ||
            data.format == PF_PVRTC_RGBA4 )
        {
            rowPitch = 0;
            bytesPerImage = 0;
        }

        switch(mTarget)
        {
            case MTLTextureType1D:
                [mTexture replaceRegion:MTLRegionMake1D( dest.left, dest.getWidth() )
                            mipmapLevel:mLevel
                              withBytes:data.data
                            bytesPerRow:0];
                break;
            case MTLTextureType1DArray:
                [mTexture replaceRegion:MTLRegionMake1D( dest.left, dest.getWidth() )
                            mipmapLevel:mLevel
                              withBytes:data.data
                            bytesPerRow:rowPitch];
                break;
            case MTLTextureType2D:
                [mTexture replaceRegion:MTLRegionMake2D( dest.left, dest.top,
                                                         dest.getWidth(), dest.getHeight() )
                            mipmapLevel:mLevel
                              withBytes:data.data
                            bytesPerRow:rowPitch];
                break;
            case MTLTextureType2DArray:
                [mTexture replaceRegion:MTLRegionMake2D( dest.left, dest.top,
                                                         dest.getWidth(), dest.getHeight() )
                            mipmapLevel:mLevel
                                  slice:dest.front
                              withBytes:data.data
                            bytesPerRow:rowPitch
                          bytesPerImage:bytesPerImage];
                break;
            case MTLTextureType2DMultisample:
                [mTexture replaceRegion:MTLRegionMake2D( dest.left, dest.top,
                                                         dest.getWidth(), dest.getHeight() )
                            mipmapLevel:mLevel
                              withBytes:data.data
                            bytesPerRow:rowPitch];
                break;
            case MTLTextureTypeCube:
                [mTexture replaceRegion:MTLRegionMake2D( dest.left, dest.top,
                                                         dest.getWidth(), dest.getHeight() )
                            mipmapLevel:mLevel
                                  slice:mFace
                              withBytes:data.data
                            bytesPerRow:rowPitch
                          bytesPerImage:bytesPerImage];
                break;
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
            case MTLTextureTypeCubeArray:
                [mTexture replaceRegion:MTLRegionMake3D( dest.left, dest.top, dest.front,
                                                         dest.getWidth(),
                                                         dest.getHeight(),
                                                         dest.getDepth() )
                            mipmapLevel:mLevel
                                  slice:dest.front
                              withBytes:data.data
                            bytesPerRow:rowPitch
                          bytesPerImage:bytesPerImage];
#endif
            case MTLTextureType3D:
                [mTexture replaceRegion:MTLRegionMake3D( dest.left, dest.top, dest.front,
                                                         dest.getWidth(),
                                                         dest.getHeight(),
                                                         dest.getDepth() )
                            mipmapLevel:mLevel
                                  slice:0
                              withBytes:data.data
                            bytesPerRow:rowPitch
                          bytesPerImage:bytesPerImage];
                break;
            default:
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "unhandled case");
        }

#if OGRE_DEBUG_MODE
        std::stringstream str;
        str << "MetalTextureBuffer::upload: "
        << " pixel buffer: " << mBufferId
        << " bytes: " << mSizeInBytes
        << " dest depth: " << dest.getDepth()
        << " dest front: " << dest.front
        << " bytesPerImage: " << bytesPerImage
        << " face: " << mFace << " level: " << mLevel
        << " width: " << mWidth << " height: "<< mHeight << " depth: " << mDepth
        << " format: " << PixelUtil::getFormatName(mFormat)
        << " data format: " << PixelUtil::getFormatName(data.format);
        LogManager::getSingleton().logMessage(LML_NORMAL, str.str());
#endif
    }

    //-----------------------------------------------------------------------------------
    void MetalTextureBuffer::download(const PixelBox &data)
    {
        if(data.getWidth() != getWidth() ||
           data.getHeight() != getHeight() ||
           data.getDepth() != getDepth())
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "only download of entire buffer is supported by Metal",
                         "MetalTextureBuffer::download" );

        // Sleep for 0.01s to allow render operations to finish
        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = 10000000L;

        nanosleep(&tim, &tim2);

        NSUInteger bytesPerPixel = PixelUtil::getNumElemBytes(data.format);

        // Construct a temp PixelBox
        size_t sizeInBytes = PixelUtil::getMemorySize( data.getWidth(), data.getHeight(),
                                                       data.getDepth(), mFormat );

        PixelBox tempBox = PixelBox( data.getWidth(), data.getHeight(), data.getDepth(), mFormat );
        tempBox.data = new uint8[sizeInBytes];

        if( mTexture.storageMode == MTLStorageModePrivate )
        {
            MetalRenderSystem *renderSystem =
                    static_cast<MetalRenderSystem*>( Root::getSingleton().getRenderSystem() );
            MetalDevice *device = renderSystem->getActiveDevice();

            MTLResourceOptions resourceOptions = MTLResourceCPUCacheModeDefaultCache|
                                                 MTLResourceStorageModeShared;
            id<MTLBuffer> tmpBuffer = [device->mDevice newBufferWithLength:sizeInBytes
                                                                   options:resourceOptions];
            __unsafe_unretained id<MTLBlitCommandEncoder> blitEncoder = device->getBlitEncoder();

            [blitEncoder copyFromTexture:mTexture
                             sourceSlice:0
                             sourceLevel:0
                            sourceOrigin:MTLOriginMake( data.left, data.top, data.front )
                              sourceSize:MTLSizeMake( data.getWidth(), data.getHeight(),
                                                      data.getDepth() )
                                toBuffer:tmpBuffer
                       destinationOffset:0
                  destinationBytesPerRow:bytesPerPixel * data.getWidth()
                destinationBytesPerImage:bytesPerPixel * data.getWidth() * data.getHeight()];

            device->stall();

            memcpy( tempBox.data, [tmpBuffer contents], sizeInBytes );
        }
        else
        {
            [mTexture getBytes:tempBox.data
                   bytesPerRow:bytesPerPixel * data.getWidth()
                    fromRegion:MTLRegionMake2D(data.left, data.top, data.getWidth(), data.getHeight())
                   mipmapLevel:0];
        }

        // Convert to the target pixel format and vertically flip
        PixelUtil::bulkPixelConversion(tempBox, data);

        delete[] (uint8*) tempBox.data;
        tempBox.data = 0;
    }
    //-----------------------------------------------------------------------------------
    void MetalTextureBuffer::bindToFramebuffer(uint32 attachment, size_t zoffset)
    {
        assert(zoffset < mDepth);
    }
    //-----------------------------------------------------------------------------------
    void MetalTextureBuffer::copyFromFramebuffer(size_t zoffset)
    {
    }
    //-----------------------------------------------------------------------------------
    void MetalTextureBuffer::blit( const HardwarePixelBufferSharedPtr &src,
                                   const Box &srcBox, const Box &dstBox )
    {
        MetalTextureBuffer *srct = static_cast<MetalTextureBuffer *>(src.get());
        // Destination texture must be 2D or Cube
        // Source texture must be 2D
        if(((src->getUsage() & TU_RENDERTARGET) == 0 && (srct->mTarget == MTLTextureType2D))
           || ((srct->mTarget == MTLTextureType3D) && (mTarget != MTLTextureType2DArray)))
        {
            blitFromTexture(srct, srcBox, dstBox);
        }
        else
        {
            MetalHardwarePixelBuffer::blit(src, srcBox, dstBox);
        }
    }

    //-----------------------------------------------------------------------------------
    // Very fast texture-to-texture blitter and hardware bi/trilinear scaling implementation using FBO
    // Destination texture must be 1D, 2D, 3D, or Cube
    // Source texture must be 1D, 2D or 3D
    // Supports compressed formats as both source and destination format, it will use the hardware DXT compressor
    // if available.
    // @author W.J. van der Laan
    void MetalTextureBuffer::blitFromTexture(MetalTextureBuffer *src, const Box &srcBox, const Box &dstBox)
    {
        // Implement?
    }
    //-----------------------------------------------------------------------------------
    // blitFromMemory doing hardware trilinear scaling
    void MetalTextureBuffer::blitFromMemory(const PixelBox &src_orig, const Box &dstBox)
    {
        // Fall back to normal MetalHardwarePixelBuffer::blitFromMemory in case
        // - Either source or target is luminance due doesn't looks like supported by hardware
        // - the source dimensions match the destination ones, in which case no scaling is needed
        if(PixelUtil::isLuminance(src_orig.format) ||
           PixelUtil::isLuminance(mFormat) ||
           (src_orig.getWidth() == dstBox.getWidth() &&
            src_orig.getHeight() == dstBox.getHeight() &&
            src_orig.getDepth() == dstBox.getDepth()))
        {
            MetalHardwarePixelBuffer::blitFromMemory(src_orig, dstBox);
            return;
        }
        if(!mBuffer.contains(dstBox))
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Destination box out of range",
                        "MetalTextureBuffer::blitFromMemory");
        // For scoped deletion of conversion buffer
        MemoryDataStreamPtr buf;
        PixelBox src;

        // Create temporary texture to store source data
        MTLTextureType target = (src.getDepth() != 1) ? MTLTextureType3D : MTLTextureType2D;

        int width = src.getWidth();
        int height = src.getHeight();
        int depth = src.getDepth();

        // Metal texture buffer
        MetalTextureBuffer tex( mTexture, 0, 0, BLANKSTRING, target, width, height, depth,
                                src.format,
                                0, 0, (Usage)(TU_AUTOMIPMAP|HBU_STATIC_WRITE_ONLY),
                                src.format == mFormat ? mHwGamma : false, 0 );

        // Upload data to 0,0,0 in temporary texture
        Box tempTarget(0, 0, 0, src.getWidth(), src.getHeight(), src.getDepth());
        tex.upload(src, tempTarget);

        // Blit
        blitFromTexture(&tex, tempTarget, dstBox);
    }

    RenderTexture *MetalTextureBuffer::getRenderTarget(size_t zoffset)
    {
        assert(mUsage & TU_RENDERTARGET);
        assert(zoffset < mDepth);
        return mSliceTRT[zoffset];
    }
}
