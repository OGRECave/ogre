/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#include "OgreVolumeGridSource.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreVector3.h"
#include "OgreColourValue.h"
#include "OgreMemoryAllocatorConfig.h"
#include "OgreLogManager.h"
#include "OgreTimer.h"

namespace Ogre {
namespace Volume {
    
    GridSource::GridSource(bool trilinearValue, bool trilinearGradient, bool sobelGradient) :
        mTrilinearValue(trilinearValue), mTrilinearGradient(trilinearGradient), mSobelGradient(sobelGradient)
    {
    }

    //-----------------------------------------------------------------------

    GridSource::~GridSource(void)
    {
    }
    
    //-----------------------------------------------------------------------
    
    Vector4 GridSource::getValueAndGradient(const Vector3 &position) const
    {
        Vector3 scaledPosition(position.x * mPosXScale, position.y * mPosYScale, position.z * mPosZScale);
        Vector3 normal;
        if (mTrilinearGradient)
        {
            size_t x0 = (size_t)scaledPosition.x;
            size_t x1 = (size_t)ceil(scaledPosition.x);
            size_t y0 = (size_t)scaledPosition.y;
            size_t y1 = (size_t)ceil(scaledPosition.y);
            size_t z0 = (size_t)scaledPosition.z;
            size_t z1 = (size_t)ceil(scaledPosition.z);
        
            Real dX = scaledPosition.x - (Real)x0;
            Real dY = scaledPosition.y - (Real)y0;
            Real dZ = scaledPosition.z - (Real)z0;
        
            Vector3 f000 = getGradient(x0, y0, z0);
            Vector3 f100 = getGradient(x1, y0, z0);
            Vector3 f010 = getGradient(x0, y1, z0);
            Vector3 f001 = getGradient(x0, y0, z1);
            Vector3 f101 = getGradient(x1, y0, z1);
            Vector3 f011 = getGradient(x0, y1, z1);
            Vector3 f110 = getGradient(x1, y1, z0);
            Vector3 f111 = getGradient(x1, y1, z1);

            Real oneMinX = (Real)1.0 - dX;
            Real oneMinY = (Real)1.0 - dY;
            Real oneMinZ = (Real)1.0 - dZ;
            Real oneMinXoneMinY = oneMinX * oneMinY;
            Real dXOneMinY = dX * oneMinY;

            normal = oneMinZ * (f000 * oneMinXoneMinY
                + f100 * dXOneMinY
                + f010 * oneMinX * dY)
                + dZ * (f001 * oneMinXoneMinY
                + f101 * dXOneMinY
                + f011 * oneMinX * dY)
                + dX * dY * (f110 * oneMinZ
                + f111 * dZ);

            normal *= (Real)-1.0;
        }
        else
        {
            normal = getGradient((size_t)(scaledPosition.x + (Real)0.5), (size_t)(scaledPosition.y + (Real)0.5), (size_t)(scaledPosition.z + (Real)0.5));
            normal *= (Real)-1.0;
        }
        return Vector4(normal.x, normal.y, normal.z, getValue(position));
    }
    
    //-----------------------------------------------------------------------
    
    Real GridSource::getValue(const Vector3 &position) const
    {
        Vector3 scaledPosition(position.x * mPosXScale, position.y * mPosYScale, position.z * mPosZScale);
        Real value;
        if (mTrilinearValue)
        {
            size_t x0 = (size_t)scaledPosition.x;
            size_t x1 = (size_t)ceil(scaledPosition.x);
            size_t y0 = (size_t)scaledPosition.y;
            size_t y1 = (size_t)ceil(scaledPosition.y);
            size_t z0 = (size_t)scaledPosition.z;
            size_t z1 = (size_t)ceil(scaledPosition.z);

            Real dX = scaledPosition.x - (Real)x0;
            Real dY = scaledPosition.y - (Real)y0;
            Real dZ = scaledPosition.z - (Real)z0;

            Real f000 = getVolumeGridValue(x0, y0, z0);
            Real f100 = getVolumeGridValue(x1, y0, z0);
            Real f010 = getVolumeGridValue(x0, y1, z0);
            Real f001 = getVolumeGridValue(x0, y0, z1);
            Real f101 = getVolumeGridValue(x1, y0, z1);
            Real f011 = getVolumeGridValue(x0, y1, z1);
            Real f110 = getVolumeGridValue(x1, y1, z0);
            Real f111 = getVolumeGridValue(x1, y1, z1);

            Real oneMinX = (Real)1.0 - dX;
            Real oneMinY = (Real)1.0 - dY;
            Real oneMinZ = (Real)1.0 - dZ;
            Real oneMinXoneMinY = oneMinX * oneMinY;
            Real dXOneMinY = dX * oneMinY;

            value = oneMinZ * (f000 * oneMinXoneMinY
                + f100 * dXOneMinY
                + f010 * oneMinX * dY)
                + dZ * (f001 * oneMinXoneMinY
                + f101 * dXOneMinY
                + f011 * oneMinX * dY)
                + dX * dY * (f110 * oneMinZ
                + f111 * dZ);
        
        }
        else
        {
            // Nearest neighbour else
            size_t x = (size_t)(scaledPosition.x + (Real)0.5);
            size_t y = (size_t)(scaledPosition.y + (Real)0.5);
            size_t z = (size_t)(scaledPosition.z + (Real)0.5);
            value = (Real)getVolumeGridValue(x, y, z);
        }
        return value;
    }
    
    //-----------------------------------------------------------------------
    
    size_t GridSource::getWidth(void) const
    {
        return mWidth;
    }
    
    //-----------------------------------------------------------------------
    
    size_t GridSource::getHeight(void) const
    {
        return mHeight;
    }
    
    //-----------------------------------------------------------------------
    
    size_t GridSource::getDepth(void) const
    {
        return mDepth;
    }

}
}