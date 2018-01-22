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

#include "OgreStableHeaders.h"

#include "OgreIrradianceVolume.h"
#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    IrradianceVolume::IrradianceVolume( HlmsManager *hlmsManager ) :
        mHlmsManager( hlmsManager ),
        mNumBlocksX( 0 ),
        mNumBlocksY( 0 ),
        mNumBlocksZ( 0 ),
        mFadeAttenuationOverDistace( false ),
        mPowerScale( 1.0f ),
        mIrradianceMaxPower( 1 ),
        mIrradianceOrigin( Vector3::ZERO ),
        mIrradianceCellSize( Vector3::UNIT_SCALE ),
        mIrradianceSamplerblock( 0 ),
        mVolumeData( 0 ),
        mBlurredVolumeData( 0 ),
        mRowPitch( 0 ),
        mSlicePitch( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    IrradianceVolume::~IrradianceVolume()
    {
        destroyIrradianceVolumeTexture();
        freeMemory();
    }
    //-----------------------------------------------------------------------------------
    void IrradianceVolume::gaussFilter( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                        size_t texWidth, size_t texHeight, size_t texDepth )
    {
        /*const float c_kernel[17] =
        {
            0.000078f, 0.000489f, 0.002403f, 0.009245f, 0.027835f, 0.065592f, 0.12098f, 0.17467f,
            0.197417f,
            0.17467f, 0.12098f, 0.065592f, 0.027835f, 0.009245f, 0.002403f, 0.000489f, 0.000078f
        };

        const int kernelStart = -8;
        const int kernelEnd   =  8;*/
        const float c_kernel[9] =
        {
            0.028532f, 0.067234f, 0.124009f, 0.179044f,
            0.20236f,
            0.179044f, 0.124009f, 0.067234f, 0.028532f
        };

        const int kernelStart = -4;
        const int kernelEnd   =  4;

        gaussFilterX( dstData, srcData, texWidth, texHeight, texDepth,
                      c_kernel, kernelStart, kernelEnd );
        gaussFilterY( srcData, dstData, texWidth, texHeight, texDepth,
                      c_kernel, kernelStart, kernelEnd );
        gaussFilterZ( dstData, srcData, texWidth, texHeight, texDepth,
                      c_kernel, kernelStart, kernelEnd );
    }
    //-----------------------------------------------------------------------------------
    void IrradianceVolume::gaussFilterX( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                         size_t texWidth, size_t texHeight, size_t texDepth,
                                         const float * RESTRICT_ALIAS kernel,
                                         int kernelStart, int kernelEnd )
    {
        const size_t rowPitch = texWidth * 3u;
        const size_t slicePitch = rowPitch * texHeight;

        //X filter
        for( size_t z=0; z<texDepth; ++z )
        {
            for( size_t y=0; y<texHeight; y += 6u )
            {
                for( size_t x=0; x<texWidth; ++x )
                {
                    const int kStart    = std::max<int>( -(int)x, kernelStart );
                    const int kEnd      = std::min<int>( texWidth - 1 - x, kernelEnd );

                    for( int i=0; i<6; ++i )
                    {
                        float accumR = 0;
                        float accumG = 0;
                        float accumB = 0;

                        float divisor = 0;

                        size_t srcIdx = z * slicePitch + (y + i) * rowPitch + (x + kStart) * 3u;

                        for( int k=kStart; k<=kEnd; ++k )
                        {
                            const float kernelVal = kernel[k+kernelEnd];

                            accumR += srcData[srcIdx+0] * kernelVal;
                            accumG += srcData[srcIdx+1] * kernelVal;
                            accumB += srcData[srcIdx+2] * kernelVal;

                            divisor += kernelVal;
                            srcIdx += 3;
                        }

                        float invDivisor = 1.0f / divisor;
                        const size_t dstIdx = z * slicePitch + (y + i) * rowPitch + x * 3u;

                        dstData[dstIdx+0] = accumR * invDivisor;
                        dstData[dstIdx+1] = accumG * invDivisor;
                        dstData[dstIdx+2] = accumB * invDivisor;
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void IrradianceVolume::gaussFilterY( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                         size_t texWidth, size_t texHeight, size_t texDepth,
                                         const float * RESTRICT_ALIAS kernel,
                                         int kernelStart, int kernelEnd )
    {
        const size_t rowPitch = texWidth * 3u;
        const size_t slicePitch = rowPitch * texHeight;

        //Y filter
        for( size_t z=0; z<texDepth; ++z )
        {
            for( size_t y=0; y<texHeight; y += 6u )
            {
                const int kStart    = std::max<int>( -(int)(y / 6u), kernelStart );
                const int kEnd      = std::min<int>( (texHeight - 6u - y) / 6u, kernelEnd );

                for( size_t x=0; x<texWidth; ++x )
                {
                    for( int i=0; i<6; ++i )
                    {
                        float accumR = 0;
                        float accumG = 0;
                        float accumB = 0;

                        float divisor = 0;

                        size_t srcIdx = z * slicePitch + (y + i + kStart * 6) * rowPitch + x * 3u;

                        for( int k=kStart; k<=kEnd; ++k )
                        {
                            const float kernelVal = kernel[k+kernelEnd];

                            accumR += srcData[srcIdx+0] * kernelVal;
                            accumG += srcData[srcIdx+1] * kernelVal;
                            accumB += srcData[srcIdx+2] * kernelVal;

                            divisor += kernelVal;
                            srcIdx += rowPitch * 6u;
                        }

                        float invDivisor = 1.0f / divisor;
                        const size_t dstIdx = z * slicePitch + (y + i) * rowPitch + x * 3u;

                        dstData[dstIdx+0] = accumR * invDivisor;
                        dstData[dstIdx+1] = accumG * invDivisor;
                        dstData[dstIdx+2] = accumB * invDivisor;
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void IrradianceVolume::gaussFilterZ( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                         size_t texWidth, size_t texHeight, size_t texDepth,
                                         const float * RESTRICT_ALIAS kernel,
                                         int kernelStart, int kernelEnd )
    {
        const size_t rowPitch = texWidth * 3u;
        const size_t slicePitch = rowPitch * texHeight;

        //Z filter
        for( size_t z=0; z<texDepth; ++z )
        {
            const int kStart    = std::max<int>( -(int)z, kernelStart );
            const int kEnd      = std::min<int>( texDepth - 1u - z, kernelEnd );

            for( size_t y=0; y<texHeight; y += 6u )
            {
                for( size_t x=0; x<texWidth; ++x )
                {
                    for( int i=0; i<6; ++i )
                    {
                        float accumR = 0;
                        float accumG = 0;
                        float accumB = 0;

                        float divisor = 0;

                        size_t srcIdx = (z + kStart) * slicePitch + (y + i) * rowPitch + x * 3u;

                        for( int k=kStart; k<=kEnd; ++k )
                        {
                            const float kernelVal = kernel[k+kernelEnd];

                            accumR += srcData[srcIdx+0] * kernelVal;
                            accumG += srcData[srcIdx+1] * kernelVal;
                            accumB += srcData[srcIdx+2] * kernelVal;

                            divisor += kernelVal;
                            srcIdx += slicePitch;
                        }

                        float invDivisor = 1.0f / divisor;
                        const size_t dstIdx = z * slicePitch + (y + i) * rowPitch + x * 3u;

                        dstData[dstIdx+0] = accumR * invDivisor;
                        dstData[dstIdx+1] = accumG * invDivisor;
                        dstData[dstIdx+2] = accumB * invDivisor;
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void IrradianceVolume::createIrradianceVolumeTexture( uint32 numBlocksX, uint32 numBlocksY, uint32 numBlocksZ )
    {
        destroyIrradianceVolumeTexture();

        mNumBlocksX = numBlocksX;
        mNumBlocksY = numBlocksY;
        mNumBlocksZ = numBlocksZ;

        uint32 width = numBlocksX;
        uint32 height = numBlocksY * 6;
        uint32 depth = numBlocksZ;

        mRowPitch = width * 3u;
        mSlicePitch = mRowPitch * height;

        //const uint32 maxMipCount = PixelUtil::getMaxMipmapCount( width, height, depth );
        const uint32 maxMipCount = 0; //TODO?

        mIrradianceVolume = TextureManager::getSingleton().createManual(
                    "InstantRadiosity_IrradianceVolume",
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_3D, width, height, depth, maxMipCount, PF_A2R10G10B10, TU_DEFAULT );

        HlmsSamplerblock samplerblock;
        samplerblock.mMinFilter = FO_LINEAR;
        samplerblock.mMagFilter = FO_LINEAR;
        samplerblock.mMipFilter = FO_LINEAR;
        samplerblock.setAddressingMode( TAM_BORDER );
        samplerblock.mBorderColour = ColourValue::ZERO;
        mIrradianceSamplerblock = mHlmsManager->getSamplerblock( samplerblock );
    }
    //-----------------------------------------------------------------------------------
    void IrradianceVolume::destroyIrradianceVolumeTexture()
    {
        if( !mIrradianceVolume.isNull() )
        {
            TextureManager::getSingleton().remove( mIrradianceVolume->getHandle() );
            mIrradianceVolume.setNull();
        }

        if( mIrradianceSamplerblock )
        {
            mHlmsManager->destroySamplerblock( mIrradianceSamplerblock );
            mIrradianceSamplerblock = 0;
        }
    }

    void IrradianceVolume::freeMemory()
    {
        if( mVolumeData )
        {
            OGRE_FREE( mVolumeData, MEMCATEGORY_GENERAL );
            OGRE_FREE( mBlurredVolumeData, MEMCATEGORY_GENERAL );
            mVolumeData = 0;
            mBlurredVolumeData = 0;
        }
    }

    void IrradianceVolume::changeVolumeData(uint32 x, uint32 y, uint32 z, uint32 direction_id, const Vector3& delta)
    {
        assert( mVolumeData );
        assert( direction_id < 6 );

        const size_t idx = z * mSlicePitch + (y * 6 + direction_id) * mRowPitch + x * 3u;
        mVolumeData[idx + 0] += delta.x;
        mVolumeData[idx + 1] += delta.y;
        mVolumeData[idx + 2] += delta.z;
    }

    void IrradianceVolume::clearVolumeData()
    {
        freeMemory();

        if (mIrradianceVolume)
        {
            const int32 texWidth = static_cast<int32>(mIrradianceVolume->getWidth());
            const int32 texHeight = static_cast<int32>( mIrradianceVolume->getHeight() );
            const int32 texDepth  = static_cast<int32>( mIrradianceVolume->getDepth() );

            mVolumeData = reinterpret_cast<float*>( OGRE_MALLOC( texWidth * texHeight *
                                                                 texDepth * 3u * sizeof(float),
                                                                 MEMCATEGORY_GENERAL ) );
            mBlurredVolumeData = reinterpret_cast<float*>( OGRE_MALLOC( texWidth * texHeight *
                                                                        texDepth * 3u * sizeof(float),
                                                                        MEMCATEGORY_GENERAL ) );

            memset( mVolumeData, 0, texWidth * texHeight * texDepth * 3u * sizeof(float) );
        }
    }

    void IrradianceVolume::updateIrradianceVolumeTexture()
    {
        const int32 texWidth  = static_cast<int32>( mIrradianceVolume->getWidth() );
        const int32 texHeight = static_cast<int32>( mIrradianceVolume->getHeight() );
        const int32 texDepth  = static_cast<int32>( mIrradianceVolume->getDepth() );

        gaussFilter( mBlurredVolumeData, mVolumeData, texWidth, texHeight, texDepth );

        const PixelBox &lockBox = mIrradianceVolume->getBuffer()->lock(
                            Box( 0, 0, 0, texWidth, texHeight, texDepth ), v1::HardwareBuffer::HBL_NORMAL );

        const size_t bytesPerPixel = PixelUtil::getNumElemBytes( mIrradianceVolume->getFormat() );
        const size_t texRowPitch = lockBox.rowPitchAlwaysBytes();
        const size_t texSlicePitch = lockBox.slicePitchAlwaysBytes();

        uint8 * RESTRICT_ALIAS dstData = reinterpret_cast<uint8 * RESTRICT_ALIAS>( lockBox.data );

        const size_t rowPitch = texWidth * 3u;
        const size_t slicePitch = rowPitch * texHeight;

        for (size_t z = 0; z<(size_t)texDepth; ++z)
        {
            for( size_t y=0; y<(size_t)texHeight; ++y )
            {
                for( size_t x=0; x<(size_t)texWidth; ++x )
                {
                    const size_t srcIdx = z * slicePitch + y * rowPitch + x * 3u;
                    const size_t dstIdx = z * texSlicePitch + y * texRowPitch + x * bytesPerPixel;
                    PixelUtil::packColour( mBlurredVolumeData[srcIdx+0], mBlurredVolumeData[srcIdx+1],
                                           mBlurredVolumeData[srcIdx+2], 1.0f,
                                           PF_A2R10G10B10, &dstData[dstIdx] );
                }
            }
        }

        mIrradianceVolume->getBuffer()->unlock();
    }
}
