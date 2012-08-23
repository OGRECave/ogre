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
#include "OgreVolumeTextureSource.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreColourValue.h"
#include "OgreMemoryAllocatorConfig.h"
#include "OgreLogManager.h"
#include "OgreTimer.h"

namespace Ogre {
namespace Volume {

    TextureSource::TextureSource(const String &volumeTextureName, const Real worldWidth, const Real worldHeight, const Real worldDepth, const bool trilinearValue, const bool trilinearNormal, const bool sobelNormal) :
        mTrilinearValue(trilinearValue), mTrilinearNormal(trilinearNormal), mSobelNormal(sobelNormal)
    {
    
        Timer t;
        //we to load then read a 3d texture. we cannot load it directly as that will mean we might
        //not be able to read it. we need to change it's usage from dynamic (default) to a readable static.
        Ogre::ResourceManager::ResourceCreateOrRetrieveResult res =
            TextureManager::getSingleton().createOrRetrieve(volumeTextureName,
            Ogre::ResourceGroupManager::getSingleton().getWorldResourceGroupName(),
            false,0,0,Ogre::TEX_TYPE_3D);
        Ogre::TexturePtr tex = res.first;
        tex->setUsage(TU_DYNAMIC);
        tex->load();
       
        LogManager::getSingleton().stream() << "Loaded texture in " << t.getMilliseconds() << "ms.";
        t.reset();

        mWidth = tex->getSrcWidth();
        mHeight= tex->getSrcHeight();
        mWidthTimesHeight = mWidth * mHeight;
        mDepth = tex->getSrcDepth();

        mPosXScale = (Real)1.0 / (Real)worldWidth * (Real)mWidth;
        mPosYScale = (Real)1.0 / (Real)worldHeight * (Real)mHeight;
        mPosZScale = (Real)1.0 / (Real)worldDepth * (Real)mDepth;
    
        HardwarePixelBufferSharedPtr buffer = tex->getBuffer(0, 0);
        buffer->lock(HardwareBuffer::HBL_READ_ONLY);
        const PixelBox &pb = buffer->getCurrentLock();
        float *pbptr = static_cast<float*>(pb.data);
        mData = OGRE_ALLOC_T(float, mWidth * mHeight * mDepth, MEMCATEGORY_GENERAL);
        float * dataRunner = mData;
        ColourValue cv;
        size_t x, y;
        size_t zEnd = pb.back;
        size_t yStart = pb.top;
        size_t yEnd = pb.bottom;
        size_t xStart = pb.left;
        size_t xEnd = pb.right;
        size_t sliceSkip = pb.getSliceSkip();
        for (size_t z = pb.front; z < zEnd; ++z)
        {
            for (y = yStart; y < yEnd; ++y)
            {
                for (x = xStart; x < xEnd; ++x)
                {
                    *dataRunner++ = pbptr[x];
                }
                pbptr += pb.rowPitch;
            }
            pbptr += sliceSkip;
		}
		buffer->unlock();

		TextureManager::getSingleton().remove(tex->getHandle());

		LogManager::getSingleton().stream() << "Processed texture in " << t.getMilliseconds() << "ms.";
    }
    
    //-----------------------------------------------------------------------

    TextureSource::~TextureSource(void)
    {
        OGRE_FREE(mData, MEMCATEGORY_GENERAL);
    }
    
    //-----------------------------------------------------------------------

    Real TextureSource::getValueAndGradient(const Vector3 &position, Vector3 &normal) const
    {
        Vector3 scaledPosition(position.x * mPosXScale, position.y * mPosYScale, position.z * mPosZScale);
        if (mTrilinearNormal)
        {
            // http://en.wikipedia.org/wiki/Trilinear_interpolation
            size_t x0 = (size_t)scaledPosition.x;
            size_t x1 = (size_t)ceil(scaledPosition.x);
            size_t y0 = (size_t)scaledPosition.y;
            size_t y1 = (size_t)ceil(scaledPosition.y);
            size_t z0 = (size_t)scaledPosition.z;
            size_t z1 = (size_t)ceil(scaledPosition.z);
        
            Real dX = scaledPosition.x - (Real)x0;
            Real dY = scaledPosition.y - (Real)y0;
            Real dZ = scaledPosition.z - (Real)z0;
        
            Vector3 i1 = getGradient(x0, y0, z0) * ((Real)1.0 - dZ) + getGradient(x0, y0, z1) * dZ;
            Vector3 i2 = getGradient(x0, y1, z0) * ((Real)1.0 - dZ) + getGradient(x0, y1, z1) * dZ;
            Vector3 j1 = getGradient(x1, y0, z0) * ((Real)1.0 - dZ) + getGradient(x1, y0, z1) * dZ;
            Vector3 j2 = getGradient(x1, y1, z0) * ((Real)1.0 - dZ) + getGradient(x1, y1, z1) * dZ;
            Vector3 w1 = i1 * ((Real)1.0 - dY) + i2 * dY;
            Vector3 w2 = j1 * ((Real)1.0 - dY) + j2 * dY;
            normal = w1 * ((Real)1.0 - dX) + w2 * dX;
            normal *= (Real)-1.0;
        }
        else
        {
            normal = getGradient((size_t)scaledPosition.x, (size_t)scaledPosition.y, (size_t)scaledPosition.z);
            normal *= (Real)-1.0;
        }
        return getValue(position);
    }
    
    //-----------------------------------------------------------------------

    Real TextureSource::getValue(const Vector3 &position) const
    {
        Vector3 scaledPosition(position.x * mPosXScale, position.y * mPosYScale, position.z * mPosZScale);
        Real value;
        if (mTrilinearValue)
        {
            // http://en.wikipedia.org/wiki/Trilinear_interpolation
            size_t x0 = (size_t)scaledPosition.x;
            size_t x1 = (size_t)ceil(scaledPosition.x);
            size_t y0 = (size_t)scaledPosition.y;
            size_t y1 = (size_t)ceil(scaledPosition.y);
            size_t z0 = (size_t)scaledPosition.z;
            size_t z1 = (size_t)ceil(scaledPosition.z);
        
            Real dX = scaledPosition.x - (Real)x0;
            Real dY = scaledPosition.y - (Real)y0;
            Real dZ = scaledPosition.z - (Real)z0;

            Real i1 = getVolumeArrayValue(x0, y0, z0) * ((Real)1.0 - dZ) + getVolumeArrayValue(x0, y0, z1) * dZ;
            Real i2 = getVolumeArrayValue(x0, y1, z0) * ((Real)1.0 - dZ) + getVolumeArrayValue(x0, y1, z1) * dZ;
            Real j1 = getVolumeArrayValue(x1, y0, z0) * ((Real)1.0 - dZ) + getVolumeArrayValue(x1, y0, z1) * dZ;
            Real j2 = getVolumeArrayValue(x1, y1, z0) * ((Real)1.0 - dZ) + getVolumeArrayValue(x1, y1, z1) * dZ;
            Real w1 = i1 * ((Real)1.0 - dY) + i2 * dY;
            Real w2 = j1 * ((Real)1.0 - dY) + j2 * dY;
            value = w1 * ((Real)1.0 - dX) + w2 * dX;
        }
        else
        {
            // Nearest neighbour else
            size_t x = (size_t)(scaledPosition.x + (Real)0.5);
            size_t y = (size_t)(scaledPosition.y + (Real)0.5);
            size_t z = (size_t)(scaledPosition.z + (Real)0.5);
            value = (Real)getVolumeArrayValue(x, y, z);
        }
        return value;
    }
    
    //-----------------------------------------------------------------------

    size_t TextureSource::getWidth(void) const
    {
        return mWidth;
    }
    
    //-----------------------------------------------------------------------

    size_t TextureSource::getHeight(void) const
    {
        return mHeight;
    }
    
    //-----------------------------------------------------------------------

    size_t TextureSource::getDepth(void) const
    {
        return mDepth;
    }

}
}