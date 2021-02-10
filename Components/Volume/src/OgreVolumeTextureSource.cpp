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
        Image img;
        img.load(volumeTextureName, ResourceGroupManager::getSingleton().getWorldResourceGroupName());

        LogManager::getSingleton().stream() << "Loaded texture in " << t.getMilliseconds() << "ms.";
        t.reset();

        mWidth = img.getWidth();
        mHeight= img.getHeight();
        mWidthTimesHeight = mWidth * mHeight;
        mDepth = img.getDepth();

        mPosXScale = (Real)1.0 / (Real)worldWidth * (Real)mWidth;
        mPosYScale = (Real)1.0 / (Real)worldHeight * (Real)mHeight;
        mPosZScale = (Real)1.0 / (Real)worldDepth * (Real)mDepth;

        mVolumeSpaceToWorldSpaceFactor = (Real)worldWidth * (Real)mWidth;

        auto srcBox = img.getPixelBox();

        mData = OGRE_ALLOC_T(float, mWidth * mHeight * mDepth, MEMCATEGORY_GENERAL);
        auto dstBox = srcBox;
        dstBox.data = (uchar*)mData;
        dstBox.format = PF_FLOAT32_R;
        PixelUtil::bulkPixelConversion(srcBox, dstBox);

        LogManager::getSingleton().stream() << "Processed texture in " << t.getMilliseconds() << "ms.";
    }
        
    //-----------------------------------------------------------------------

    TextureSource::~TextureSource(void)
    {
        OGRE_FREE(mData, MEMCATEGORY_GENERAL);
    }

}
}
