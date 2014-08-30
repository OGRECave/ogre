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
#include "OgreVolumeGridSource.h"
#include "OgreVector3.h"
#include "OgreLogManager.h"
#include "OgreRay.h"
#include "OgreVolumeCSGSource.h"

namespace Ogre {
namespace Volume {
    
    Vector3 GridSource::getIntersectionStart(const Ray &ray, Real maxDistance) const
    {
        AxisAlignedBox box((Real)0, (Real)0, (Real)0, (Real)mWidth / mPosXScale, (Real)mHeight / mPosYScale, (Real)mDepth / mPosZScale);
        
        // Inside the grid
        if (box.intersects(ray.getOrigin()))
        {
            return ray.getOrigin();
        }
        
        // Outside the grid, ray intersects it
        std::pair<bool, Real> intersection = ray.intersects(box);
        if (intersection.first)
        {
            Vector3 direction = ray.getDirection().normalisedCopy();
            return ray.getOrigin() + direction * intersection.second;
        }

        // Outside the grid, ray doesn't intersect it
        return ray.getOrigin();
    }

    //-----------------------------------------------------------------------

    Vector3 GridSource::getIntersectionEnd(const Ray &ray, Real maxDistance) const
    {
        AxisAlignedBox box((Real)0, (Real)0, (Real)0, (Real)mWidth / mPosXScale, (Real)mHeight / mPosYScale, (Real)mDepth / mPosZScale);
        Vector3 direction = ray.getDirection().normalisedCopy();
        Vector3 invertedDirection = (Real)-1.0 * direction;
        Vector3 origin = ray.getOrigin() + direction * box.getSize().length();

        Ray inverted(origin, invertedDirection);
        std::pair<bool, Real> intersection = inverted.intersects(box);
        if (intersection.first)
        {
            return origin + invertedDirection * intersection.second;
        }
        return ray.getOrigin() + direction * maxDistance;
    }

    //-----------------------------------------------------------------------

    GridSource::GridSource(bool trilinearValue, bool trilinearGradient, bool sobelGradient) :
        mWidth(0), mHeight(0), mDepth(0), mPosXScale(0), mPosYScale(0), mPosZScale(0),
        mTrilinearValue(trilinearValue), mTrilinearGradient(trilinearGradient), mSobelGradient(sobelGradient),
        mVolumeSpaceToWorldSpaceFactor(0)
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
        Vector3 gradient;
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

            gradient = oneMinZ * (f000 * oneMinXoneMinY
                + f100 * dXOneMinY
                + f010 * oneMinX * dY)
                + dZ * (f001 * oneMinXoneMinY
                + f101 * dXOneMinY
                + f011 * oneMinX * dY)
                + dX * dY * (f110 * oneMinZ
                + f111 * dZ);

            gradient *= (Real)-1.0;
        }
        else
        {
            gradient = getGradient((size_t)(scaledPosition.x + (Real)0.5), (size_t)(scaledPosition.y + (Real)0.5), (size_t)(scaledPosition.z + (Real)0.5));
            gradient *= (Real)-1.0;
        }
        return Vector4(gradient.x, gradient.y, gradient.z, getValue(position));
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
    
    //-----------------------------------------------------------------------
    
    void GridSource::combineWithSource(CSGOperationSource *operation, Source *source, const Vector3 &center, Real radius)
    {
        Real worldWidthScale = (Real)1.0 / mPosXScale;
        Real worldHeightScale = (Real)1.0 / mPosYScale;
        Real worldDepthScale = (Real)1.0 / mPosZScale;

        operation->setSourceA(this);
        operation->setSourceB(source);
        // No need for trilineaer interpolation here as we iterate over the
        // cells anyway.
        bool oldTrilinearValue = mTrilinearValue;
        mTrilinearValue = false;
        float value;
        int x, y;
        Vector3 scaledCenter(center.x * mPosXScale, center.y * mPosYScale, center.z * mPosZScale);
        int xStart = Math::Clamp(static_cast<int>(scaledCenter.x - radius * mPosXScale), 0, static_cast<int>(mWidth));
        int xEnd = Math::Clamp(static_cast<int>(scaledCenter.x + radius * mPosXScale), 0, static_cast<int>(mWidth));
        int yStart = Math::Clamp(static_cast<int>(scaledCenter.y - radius * mPosYScale), 0, static_cast<int>(mHeight));
        int yEnd = Math::Clamp(static_cast<int>(scaledCenter.y + radius * mPosYScale), 0, static_cast<int>(mHeight));
        int zStart = Math::Clamp(static_cast<int>(scaledCenter.z - radius * mPosZScale), 0, static_cast<int>(mDepth));
        int zEnd = Math::Clamp(static_cast<int>(scaledCenter.z + radius * mPosZScale), 0, static_cast<int>(mDepth));
        Vector3 pos;
        for (int z = zStart; z < zEnd; ++z)
        {
            for (y = yStart; y < yEnd; ++y)
            {
                for (x = xStart; x < xEnd; ++x)
                {
                    pos.x = x * worldWidthScale;
                    pos.y = y * worldHeightScale;
                    pos.z = z * worldDepthScale;
                    value = operation->getValue(pos);
                    setVolumeGridValue(x, y, z, value);
                }
            }
        }

        mTrilinearValue = oldTrilinearValue;
    }
 
    //-----------------------------------------------------------------------

    Real GridSource::getVolumeSpaceToWorldSpaceFactor(void) const
    {
        return mVolumeSpaceToWorldSpaceFactor;
    }
}
}