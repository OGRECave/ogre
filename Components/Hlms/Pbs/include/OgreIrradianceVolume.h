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
#ifndef _OgreIrradianceVolume_H_
#define _OgreIrradianceVolume_H_

#include "OgreHlmsPbsPrerequisites.h"
#include "OgreHlmsBufferManager.h"
#include "OgreConstBufferPool.h"
#include "OgreRay.h"
#include "OgreRawPtr.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class _OgreHlmsPbsExport IrradianceVolume
    {
    private:
        HlmsManager             *mHlmsManager;

        uint32                  mNumBlocksX;
        uint32                  mNumBlocksY;
        uint32                  mNumBlocksZ;

        /// Value kept for storing original creation parameters. It's not used after creation.
        bool mFadeAttenuationOverDistace;

        /// Tweaks how strong Irradiance Volume should be.
        /// In range (0; inf)
        float                   mPowerScale;
        /// Value kept for storing original creation parameters;
        /// as in every frame it just gets multiplied against mPowerScale
        float                   mIrradianceMaxPower;
        Vector3                 mIrradianceOrigin;
        Vector3                 mIrradianceCellSize;
        TexturePtr              mIrradianceVolume;
        HlmsSamplerblock const  *mIrradianceSamplerblock;

        float*                  mVolumeData;
        float*                  mBlurredVolumeData;

        /// Cached data for faster changeVolumeData()
        size_t                  mRowPitch;
        size_t                  mSlicePitch;

    public:
        void createIrradianceVolumeTexture( uint32 numBlocksX, uint32 numBlocksY, uint32 numBlocksZ );
        void destroyIrradianceVolumeTexture();

        void clearVolumeData();
        void updateIrradianceVolumeTexture();
        void freeMemory();

        void changeVolumeData(uint32 x, uint32 y, uint32 z, uint32 direction_id, const Vector3& delta);

        static void gaussFilter( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                 size_t texWidth, size_t texHeight, size_t texDepth );
        static void gaussFilterX( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                  size_t texWidth, size_t texHeight, size_t texDepth,
                                  const float * RESTRICT_ALIAS kernel, int kernelStart, int kernelEnd );
        static void gaussFilterY( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                  size_t texWidth, size_t texHeight, size_t texDepth,
                                  const float * RESTRICT_ALIAS kernel, int kernelStart, int kernelEnd );
        static void gaussFilterZ( float * RESTRICT_ALIAS dstData, float * RESTRICT_ALIAS srcData,
                                  size_t texWidth, size_t texHeight, size_t texDepth,
                                  const float * RESTRICT_ALIAS kernel, int kernelStart, int kernelEnd );

    public:
        IrradianceVolume( HlmsManager *hlmsManager );
        ~IrradianceVolume();

        float getIrradianceMaxPower(void) const             { return mIrradianceMaxPower; }
        /// Not really used. It's only use is keeping track of creation parameters.
        void setIrradianceMaxPower(float power)             { mIrradianceMaxPower = power; }

        const Vector3& getIrradianceOrigin(void) const      { return mIrradianceOrigin; }
        void setIrradianceOrigin(const Vector3& origin)     { mIrradianceOrigin = origin; }

        const Vector3& getIrradianceCellSize(void) const    { return mIrradianceCellSize; }
        void setIrradianceCellSize(const Vector3& cellSize) { mIrradianceCellSize = cellSize; }

        /// Not really used. It's only use is keeping track of creation parameters.
        void setFadeAttenuationOverDistace( bool fade )     { mFadeAttenuationOverDistace = fade; }
        bool getFadeAttenuationOverDistace(void) const      { return mFadeAttenuationOverDistace; }

        float getPowerScale(void) const  { return mPowerScale; }
        void setPowerScale(float power)  { mPowerScale = power; }

        uint32 getNumBlocksX(void) const { return mNumBlocksX; }
        uint32 getNumBlocksY(void) const { return mNumBlocksY; }
        uint32 getNumBlocksZ(void) const { return mNumBlocksZ; }

        const TexturePtr& getIrradianceVolumeTexture(void) const    { return mIrradianceVolume; }
        const HlmsSamplerblock* getIrradSamplerblock(void) const    { return mIrradianceSamplerblock; }


    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
