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
#include "OgreVolumeHalfFloatGridSource.h"
#include "OgreRoot.h"
#include "OgreDeflate.h"
#include "OgreStreamSerialiser.h"
#include "OgreBitwise.h"
#include "OgreMemoryAllocatorConfig.h"
#include "OgreLogManager.h"
#include "OgreTimer.h"

namespace Ogre {
namespace Volume {

    float HalfFloatGridSource::getVolumeGridValue(size_t x, size_t y, size_t z) const
    {
        x = x >= mWidth ? mWidth - 1 : x;
        y = y >= mHeight ? mHeight - 1 : y;
        z = z >= mDepth ? mDepth - 1 : z;
        return Bitwise::halfToFloat(mData[(mDepth - z - 1) * mDepthTimesHeight + x * mHeight + y]);
    }

    //-----------------------------------------------------------------------

    void HalfFloatGridSource::setVolumeGridValue(int x, int y, int z, float value)
    {

        // Clamp if wanted.
        if (mMaxClampedAbsoluteDensity != (Real)0.0 && Math::Abs(value) > mMaxClampedAbsoluteDensity)
        {
            value = mMaxClampedAbsoluteDensity;
        }
        
        mData[(mDepth - z - 1) * mDepthTimesHeight + x * mHeight + y] = Bitwise::floatToHalf(value);
    }

    //-----------------------------------------------------------------------
    
    HalfFloatGridSource::HalfFloatGridSource(const String &serializedVolumeFile, const bool trilinearValue, const bool trilinearGradient, const bool sobelGradient) :
        GridSource(trilinearValue, trilinearGradient, sobelGradient)
    {
    
        Timer t;
        DataStreamPtr streamRead = Root::getSingleton().openFileStream(serializedVolumeFile);
#if OGRE_NO_ZIP_ARCHIVE == 0
        DataStreamPtr uncompressStream(OGRE_NEW DeflateStream(serializedVolumeFile, streamRead));
        StreamSerialiser ser(uncompressStream);
#else
        StreamSerialiser ser(streamRead);
#endif
        if (!ser.readChunkBegin(VOLUME_CHUNK_ID, VOLUME_CHUNK_VERSION))
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Invalid volume file given!");
        }
        
        // Read header
        Vector3 readFrom, readTo;
        ser.read(&readFrom);
        ser.read(&readTo);
        float voxelWidth;
        ser.read<float>(&voxelWidth);
        size_t width, height, depth;
        ser.read<size_t>(&width);
        ser.read<size_t>(&height);
        ser.read<size_t>(&depth);
        mWidth = static_cast<int>(width);
        mHeight = static_cast<int>(height);
        mDepth = static_cast<int>(depth);
        mDepthTimesHeight = static_cast<int>(mDepth * mHeight);
        
        Vector3 worldDimension = readTo - readFrom;
        mPosXScale = (Real)1.0 / (Real)worldDimension.x * (Real)mWidth;
        mPosYScale = (Real)1.0 / (Real)worldDimension.y * (Real)mHeight;
        mPosZScale = (Real)1.0 / (Real)worldDimension.z * (Real)mDepth;

        mVolumeSpaceToWorldSpaceFactor = (Real)worldDimension.x * (Real)mWidth;
        mMaxClampedAbsoluteDensity = 0;

        // Read data
        size_t elementCount = mWidth * mHeight * mDepth;
        mData = OGRE_ALLOC_T(uint16, elementCount, MEMCATEGORY_GENERAL);
        ser.read(mData, elementCount);

        ser.readChunkEnd(VOLUME_CHUNK_ID);
        
        LogManager::getSingleton().stream() << "Processed serialization in " << t.getMilliseconds() << "ms.";
    }
        
    //-----------------------------------------------------------------------

    void HalfFloatGridSource::setMaxClampedAbsoluteDensity(Real maxClampedAbsoluteDensity)
    {
        mMaxClampedAbsoluteDensity = maxClampedAbsoluteDensity;
    }

    //-----------------------------------------------------------------------


    Real HalfFloatGridSource::getMaxClampedAbsoluteDensity(void) const
    {
        return mMaxClampedAbsoluteDensity;
    }

    //-----------------------------------------------------------------------

    HalfFloatGridSource::~HalfFloatGridSource(void)
    {
        OGRE_FREE(mData, MEMCATEGORY_GENERAL);
    }
}
}