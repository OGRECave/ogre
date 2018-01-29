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
#include "OgreVolumeTextureSource.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreColourValue.h"
#include "OgreMemoryAllocatorConfig.h"
#include "OgreLogManager.h"
#include "OgreTimer.h"

namespace Ogre {
namespace Volume {

    float TextureSource::getVolumeGridValue(size_t x, size_t y, size_t z) const
    {
        x = x >= mWidth ? mWidth - 1 : x;
        y = y >= mHeight ? mHeight - 1 : y;
        z = z >= mDepth ? mDepth - 1 : z;
        return mData[(mDepth - z - 1) * mWidthTimesHeight + y * mWidth + x];
    }
    
    //-----------------------------------------------------------------------

    void TextureSource::setVolumeGridValue(int x, int y, int z, float value)
    {
        mData[(mDepth - z - 1) * mWidthTimesHeight + y * mWidth + x] = value;
    }

    //-----------------------------------------------------------------------
    
    TextureSource::TextureSource(const String &volumeTextureName, const Real worldWidth, const Real worldHeight, const Real worldDepth, const bool trilinearValue, const bool trilinearGradient, const bool sobelGradient) :
        GridSource(trilinearValue, trilinearGradient, sobelGradient)
    {
    
        Timer t;
        //we to load then read a 3d texture. we cannot load it directly as that will mean we might
        //not be able to read it. we need to change it's usage from dynamic (default) to a readable static.
        Ogre::ResourceManager::ResourceCreateOrRetrieveResult res =
            TextureManager::getSingleton().createOrRetrieve(volumeTextureName,
            Ogre::ResourceGroupManager::getSingleton().getWorldResourceGroupName(),
            false, 0, 0, Ogre::TEX_TYPE_3D, 0);
        Ogre::TexturePtr tex = static_pointer_cast<Texture>(res.first);
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

        mVolumeSpaceToWorldSpaceFactor = (Real)worldWidth * (Real)mWidth;

        HardwarePixelBufferSharedPtr buffer = tex->getBuffer(0, 0);
        buffer->lock(HardwareBuffer::HBL_READ_ONLY);
        const PixelBox &pb = buffer->getCurrentLock();
        float *pbptr = reinterpret_cast<float*>(pb.data);
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

}
}
