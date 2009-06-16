/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreTerrainLayerBlendMap.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTerrain.h"
#include "OgreImage.h"

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
		mData = static_cast<uint8*>(OGRE_MALLOC(mBuffer->getWidth() * mBuffer->getHeight(), MEMCATEGORY_RESOURCE));

		// we know which of RGBA we need to look at, now find it in the format
		// because we can't guarantee what precise format the RS gives us
		unsigned char rgbaShift[4];
		PixelFormat fmt = mBuffer->getFormat();
		PixelUtil::getBitShifts(fmt, rgbaShift);
		mChannelOffset = rgbaShift[mChannel] / 8; // /8 to convert to bytes
		// now invert since we're dealing with this in a bytewise, not uint32 fashion
		mChannelOffset = PixelUtil::getNumElemBytes(fmt) - mChannelOffset;

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
		uint8* pDst = mData;
		// Download data
		Image::Box box(0, 0, mBuffer->getWidth(), mBuffer->getHeight());
		uint8* pSrc = static_cast<uint8*>(mBuffer->lock(box, HardwareBuffer::HBL_READ_ONLY).data);
		pSrc += mChannelOffset;
		size_t srcInc = PixelUtil::getNumElemBytes(mBuffer->getFormat());
		for (size_t y = box.top; y < box.bottom; ++y)
		{
			for (size_t x = box.left; x < box.right; ++x)
			{
				*pDst++ = *pSrc;
				pSrc += srcInc;
			}
		}
		mBuffer->unlock();

	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::upload()
	{
		if (mData && mDirty)
		{
			// Upload data
			uint8* pSrcBase = mData + mDirtyBox.top * mBuffer->getWidth() + mDirtyBox.left;
			uint8* pDstBase = static_cast<uint8*>(mBuffer->lock(mDirtyBox, HardwarePixelBuffer::HBL_NORMAL).data);
			pDstBase += mChannelOffset;
			size_t dstInc = PixelUtil::getNumElemBytes(mBuffer->getFormat());
			for (size_t y = 0; y < mDirtyBox.getHeight(); ++y)
			{
				uint8* pSrc = pSrcBase + y * mBuffer->getWidth();
				uint8* pDst = pDstBase + y * mBuffer->getWidth() * dstInc;
				for (size_t x = 0; x < mDirtyBox.getWidth(); ++x)
				{
					*pDst = *pSrc++;
					pDst += dstInc;
				}
			}
			mBuffer->unlock();

			mDirty = false;

		}

	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertWorldToUVSpace(const Vector3& worldPos, Real *outX, Real* outY)
	{
		Vector3 terrainSpace;
		mParent->getTerrainPosition(worldPos, &terrainSpace);
		*outX = terrainSpace.x;
		*outY = 1.0 - terrainSpace.y;
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertUVToWorldSpace(Real x, Real y, Vector3* outWorldPos)
	{
		mParent->getPosition(x, 1.0 - y, 0, outWorldPos);
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertUVToImageSpace(Real x, Real y, size_t* outX, size_t* outY)
	{
		*outX = x * (mBuffer->getWidth() - 1);
		*outY = y * (mBuffer->getHeight() - 1);
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
		*outY = 1.0 - *outY;
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::convertTerrainToImageSpace(Real x, Real y, size_t* outX, size_t* outY)
	{
		convertUVToImageSpace(x, 1.0 - y, outX, outY);
	}
	//---------------------------------------------------------------------
	uint8 TerrainLayerBlendMap::getBlendValue(size_t x, size_t y)
	{
		return *(mData + y * mBuffer->getWidth() + x);
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::setBlendValue(size_t x, size_t y, uint8 val)
	{
		*(mData + y * mBuffer->getWidth() + x) = val;
		dirtyRect(Rect(x, y, x+1, y+1));

	}
	//---------------------------------------------------------------------
	uint8* TerrainLayerBlendMap::getBlendPointer()
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
			mDirtyBox.left = std::min(mDirtyBox.left, (size_t)rect.left);
			mDirtyBox.top = std::min(mDirtyBox.top, (size_t)rect.top);
			mDirtyBox.right = std::max(mDirtyBox.right, (size_t)rect.right);
			mDirtyBox.bottom = std::max(mDirtyBox.bottom, (size_t)rect.bottom);
		}
		else
		{
			mDirtyBox.left = rect.left;
			mDirtyBox.right = rect.right;
			mDirtyBox.top = rect.top;
			mDirtyBox.bottom = rect.bottom;
			mDirty = true;
		}
	}
	//---------------------------------------------------------------------
	void TerrainLayerBlendMap::update()
	{
		upload();
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

