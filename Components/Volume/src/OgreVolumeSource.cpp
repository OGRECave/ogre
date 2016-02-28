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
#include "OgreVolumeSource.h"

#include "OgreRoot.h"
#include "OgreDeflate.h"
#include "OgreStreamSerialiser.h"
#include "OgreBitwise.h"

namespace Ogre {
namespace Volume {
    
    const uint32 Source::VOLUME_CHUNK_ID = StreamSerialiser::makeIdentifier("VOLU");
    const uint16 Source::VOLUME_CHUNK_VERSION = 1;
    const size_t Source::SERIALIZATION_CHUNK_SIZE = 1000;

    //-----------------------------------------------------------------------

    Vector3 Source::getIntersectionStart(const Ray &ray, Real maxDistance) const
    {
        return ray.getOrigin();
    }

    //-----------------------------------------------------------------------

    Vector3 Source::getIntersectionEnd(const Ray &ray, Real maxDistance) const
    {
        Vector3 dir = ray.getDirection().normalisedCopy();
        return ray.getOrigin() + dir * maxDistance;
    }

    //-----------------------------------------------------------------------

    Source::~Source(void)
    {
    }

    //-----------------------------------------------------------------------

    void Source::serialize(const Vector3 &from, const Vector3 &to, float voxelWidth, const String &file)
    {
        Real maxClampedAbsoluteDensity = (from - to).length() / (Real)16.0;
        serialize(from, to, voxelWidth, maxClampedAbsoluteDensity, file);
    }

    //-----------------------------------------------------------------------

    void Source::serialize(const Vector3 &from, const Vector3 &to, float voxelWidth, Real maxClampedAbsoluteDensity, const String &file)
    {
        // Compress
        DataStreamPtr stream = Root::getSingleton().createFileStream(file);
#if OGRE_NO_ZIP_ARCHIVE == 0
        DataStreamPtr compressStream(OGRE_NEW DeflateStream(file, stream));
        StreamSerialiser ser(compressStream);
#else
        StreamSerialiser ser(stream);
#endif
        ser.writeChunkBegin(VOLUME_CHUNK_ID, VOLUME_CHUNK_VERSION);

        // Write Metadata
        ser.write(&from);
        ser.write(&to);
        ser.write<float>(&voxelWidth);
        Vector3 diagonal = to - from;
        size_t gridWidth = (size_t)(diagonal.x / voxelWidth);
        size_t gridHeight = (size_t)(diagonal.y / voxelWidth);
        size_t gridDepth = (size_t)(diagonal.z / voxelWidth);
        ser.write<size_t>(&gridWidth);
        ser.write<size_t>(&gridHeight);
        ser.write<size_t>(&gridDepth);

        // Go over the volume and write the density data.
        Vector3 pos;
        Real realVal;
        size_t x;
        size_t y;
        uint16 buffer[SERIALIZATION_CHUNK_SIZE];
        size_t bufferI = 0;
        for (size_t z = 0; z < gridDepth; ++z)
        {
            for (x = 0; x < gridWidth; ++x)
            {
                for (y = 0; y < gridHeight; ++y)
                {
                    pos.x = x * voxelWidth + from.x;
                    pos.y = y * voxelWidth + from.y;
                    pos.z = z * voxelWidth + from.z;
                    realVal = Math::Clamp<Real>(getValue(pos), -maxClampedAbsoluteDensity, maxClampedAbsoluteDensity);
                    buffer[bufferI] = Bitwise::floatToHalf(realVal);
                    bufferI++;
                    if (bufferI == SERIALIZATION_CHUNK_SIZE)
                    {
                        ser.write<uint16>(buffer, SERIALIZATION_CHUNK_SIZE);
                        bufferI = 0;
                    }
                }
            }
        }
        if (bufferI > 0)
        {
            ser.write<uint16>(buffer, bufferI);
        }
        ser.writeChunkEnd(VOLUME_CHUNK_ID);
        
    }

    //-----------------------------------------------------------------------

    bool Source::getFirstRayIntersection(const Ray &ray, Vector3 &result, Real scale, size_t maxIterations, Real maxDistance) const
    {
        Ray scaledRay(ray.getOrigin() / scale, ray.getDirection());
        Vector3 start = getIntersectionStart(scaledRay, maxDistance);
        Vector3 cur = start;
        Vector3 end = getIntersectionEnd(scaledRay, maxDistance);
     
        Vector4 startVal = getValueAndGradient(start);
        Vector3 scaleSampleGradient(startVal.x, startVal.y, startVal.z);
        Vector3 scaleSampleEnd = start + scaleSampleGradient.normalisedCopy();
        Real scaleSample = getValue(scaleSampleEnd);
        Real densityScale = (Real)1.0 / Math::Abs(scaleSample - startVal.w) * (Real)2.0;

        Real densityCur = getValue(cur);
        Vector3 dir = scaledRay.getDirection().normalisedCopy();

        size_t count = 0;
        Vector3 prev = Vector3::ZERO, prevPrev = Vector3::ZERO;
        bool atEnd = false;
        Real totalLength = (start - end).length();
        while (Math::Abs(densityCur) > (Real)0.01 && !atEnd)
        {
            cur += dir * (Real)-1.0 * (densityCur / densityScale);
            
            // Increase the scaling a bit if we jump forth and back due to bad depth data.
            if ((cur - prevPrev).length() < (Real)0.0001)
            {
                densityScale *= (Real)2.0;
            }
            prevPrev = prev;
            prev = cur;

            densityCur = getValue(cur);

            // Check if we are out of range
            if ((start - cur).length() >= totalLength) {
                atEnd = true;
            }

            // We have a limit here...
            count++;
            if (count > maxIterations)
            {
                break;
            }
        }

        if (Math::Abs(densityCur) <= (Real)0.01)
        {
            result = cur;
            return true;
        }
        return false;
    }
    
    //-----------------------------------------------------------------------

    Real Source::getVolumeSpaceToWorldSpaceFactor(void) const
    {
        return (Real)1.0;
    }
}
}