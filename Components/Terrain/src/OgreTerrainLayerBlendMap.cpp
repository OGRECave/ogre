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
#include "OgreTerrainLayerBlendMap.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTerrain.h"
#include "OgreImage.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif

namespace Ogre
{
	//---------------------------------------------------------------------
	TerrainLayerBlendMap::TerrainLayerBlendMap(Terrain* parent, uint8 layerIndex, 
		HardwarePixelBuffer* buf)
		: mParent(parent)
		, mLayerIdx(layerIndex)
		, mChannel((layerIndex-1) % 4)
		, mDirty(false)
		, mBuffer(buf)
		, mData(0)
	{
		mData = static_cast<float*>(OGRE_MALLOC(mBuffer->getWidth() * mBuffer->getHeight() * sizeof(float), MEMCATEGORY_RESOURCE));

		// we know which of RGBA we need to look at, now find it in the format
		// because we can't guarantee what precise format the RS gives us
		unsigned char rgbaShift[4];
		PixelFormat fmt = mBuffer->getFormat();
		PixelUtil::getBitShifts(fmt, rgbaShift);
		mChannelOffset = rgbaShift[mChannel] / 8; // /8 to convert to bytes
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		// invert (dealing bytewise)
		mChannelOffset = PixelUtil::getNumElemBytes(fmt) - mChannelOffset - 1;
#endif
		download();

	}
	//---------------------------------------------------------------------
	TerrainLayerBlendMap::~TerrainLayerBlendMap()
	{
		OGRE_FREE(mData, MEMCATEGORY_RESOURCE);
		mData = 0;
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::download()
	{
		float* pDst = mData;
		// Download data
		Image::Box box(0, 0, mBuffer->getWidth(), mBuffer->getHeight());
		uint8* pSrc = static_cast<uint8*>(mBuffer->lock(box, HardwareBuffer::HBL_READ_ONLY).data);
		pSrc += mChannelOffset;
		size_t srcInc = PixelUtil::getNumElemBytes(mBuffer->getFormat());
		for (size_t y = box.top; y < box.bottom; ++y)
		{
			for (size_t x = box.left; x < box.right; ++x)
			{
				*pDst++ = static_cast<float>(*pSrc) / 255.0f;
				pSrc += srcInc;
			}
		}
		mBuffer->unlock();

	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertWorldToUVSpace(const Vector3& worldPos, Real *outX, Real* outY)
	{
		Vector3 terrainSpace;
		mParent->getTerrainPosition(worldPos, &terrainSpace);
		*outX = terrainSpace.x;
		*outY = 1.0f - terrainSpace.y;
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertUVToWorldSpace(Real x, Real y, Vector3* outWorldPos)
	{
		mParent->getPosition(x, 1.0f - y, 0, outWorldPos);
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertUVToImageSpace(Real x, Real y, size_t* outX, size_t* outY)
	{
		*outX = (unsigned long)(x * (mBuffer->getWidth() - 1));
		*outY = (unsigned long)(y * (mBuffer->getHeight() - 1));
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertImageToUVSpace(size_t x, size_t y, Real* outX, Real* outY)
	{
		*outX = x / (Real)(mBuffer->getWidth() - 1);
		*outY = y / (Real)(mBuffer->getHeight() - 1);
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertImageToTerrainSpace(size_t x, size_t y, Real* outX, Real* outY)
	{
		convertImageToUVSpace(x, y, outX, outY);
		*outY = 1.0f - *outY;
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertTerrainToImageSpace(Real x, Real y, size_t* outX, size_t* outY)
	{
		convertUVToImageSpace(x, 1.0f - y, outX, outY);
	}
	//---------------------------------------------------------------------
	float TerrainLayerBlendMap::getBlendValue(size_t x, size_t y)
	{
		return *(mData + y * mBuffer->getWidth() + x);
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::setBlendValue(size_t x, size_t y, float val)
	{
		*(mData + y * mBuffer->getWidth() + x) = val;
		dirtyRect(Rect(x, y, x+1, y+1));

	}
	//---------------------------------------------------------------------
	float* TerrainLayerBlendMap::getBlendPointer()
	{
		return mData;
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::dirty()
	{
		Rect rect;
		rect.top = 0; rect.bottom = mBuffer->getHeight();
		rect.left = 0; rect.right = mBuffer->getWidth();
		dirtyRect(rect);

	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::dirtyRect(const Rect& rect)
	{
		if (mDirty)
		{
			mDirtyBox.left = std::min(mDirtyBox.left, static_cast<uint32>(rect.left));
			mDirtyBox.top = std::min(mDirtyBox.top, static_cast<uint32>(rect.top));
			mDirtyBox.right = std::max(mDirtyBox.right, static_cast<uint32>(rect.right));
			mDirtyBox.bottom = std::max(mDirtyBox.bottom, static_cast<uint32>(rect.bottom));
		}
		else
		{
			mDirtyBox.left = static_cast<uint32>(rect.left);
			mDirtyBox.right = static_cast<uint32>(rect.right);
			mDirtyBox.top = static_cast<uint32>(rect.top);
			mDirtyBox.bottom = static_cast<uint32>(rect.bottom);
			mDirty = true;
		}
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::update()
	{
		if (mData && mDirty)
		{
			// Upload data
			float* pSrcBase = mData + mDirtyBox.top * mBuffer->getWidth() + mDirtyBox.left;
			uint8* pDstBase = static_cast<uint8*>(mBuffer->lock(mDirtyBox, HardwarePixelBuffer::HBL_NORMAL).data);
			pDstBase += mChannelOffset;
			size_t dstInc = PixelUtil::getNumElemBytes(mBuffer->getFormat());
			for (size_t y = 0; y < mDirtyBox.getHeight(); ++y)
			{
				float* pSrc = pSrcBase + y * mBuffer->getWidth();
				uint8* pDst = pDstBase + y * mBuffer->getWidth() * dstInc;
				for (size_t x = 0; x < mDirtyBox.getWidth(); ++x)
				{
					*pDst = static_cast<uint8>(*pSrc++ * 255);
					pDst += dstInc;
				}
			}
			mBuffer->unlock();

			mDirty = false;

			// make sure composite map is updated
			// mDirtyBox is in image space, convert to terrain units
			Rect compositeMapRect;
			float blendToTerrain = (float)mParent->getSize() / (float)mBuffer->getWidth();
			compositeMapRect.left = (long)(mDirtyBox.left * blendToTerrain);
			compositeMapRect.right = (long)(mDirtyBox.right * blendToTerrain + 1);
			compositeMapRect.top = (long)((mBuffer->getHeight() - mDirtyBox.bottom) * blendToTerrain);
			compositeMapRect.bottom = (long)((mBuffer->getHeight() - mDirtyBox.top) * blendToTerrain + 1);
			mParent->_dirtyCompositeMapRect(compositeMapRect);
			mParent->updateCompositeMapWithDelay();

		}
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::blit(const PixelBox &src, const Box &dstBox)
	{
		const PixelBox* srcBox = &src;

		if (srcBox->getWidth() != dstBox.getWidth() || srcBox->getHeight() != dstBox.getHeight())
		{
			// we need to rescale src to dst size first (also confvert format)
			void* tmpData = OGRE_MALLOC(dstBox.getWidth() * dstBox.getHeight(), MEMCATEGORY_GENERAL);
			srcBox = OGRE_NEW PixelBox(dstBox.getWidth(), dstBox.getHeight(), 1, PF_L8, tmpData);

			Image::scale(src, *srcBox);
		}

		// pixel conversion
		PixelBox dstMemBox(dstBox, PF_L8, mData);
		PixelUtil::bulkPixelConversion(*srcBox, dstMemBox);

		if (srcBox != &src)
		{
			// free temp
			OGRE_FREE(srcBox->data, MEMCATEGORY_GENERAL);
			OGRE_DELETE srcBox;
			srcBox = 0;
		}

		Rect dRect(dstBox.left, dstBox.top, dstBox.right, dstBox.bottom);
		dirtyRect(dRect);

	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::blit(const PixelBox &src)
	{
		blit(src, Box(0,0,0,mBuffer->getWidth(),mBuffer->getHeight(),1));
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::loadImage(const Image& img)
	{
		blit(img.getPixelBox());
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::loadImage(DataStreamPtr& stream, const String& ext)
	{
		Image img;
		img.load(stream, ext);
		loadImage(img);
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::loadImage(const String& filename, const String& groupName)
	{
		Image img;
		img.load(filename, groupName);
		loadImage(img);
	}


}

