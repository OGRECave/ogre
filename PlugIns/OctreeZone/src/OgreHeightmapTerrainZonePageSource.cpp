/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
OgreHeightmapTerrainZonePage.cpp  -  Almost same as OgreHeightmapTerrainPage.cpp 
from TerrainSM just slightly modified & renamed to avoid confusion 
with the TerrainSM version.

-----------------------------------------------------------------------------
begin                : Thu May 3 2007
author               : Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/

#include "OgreHeightmapTerrainZonePageSource.h"
#include "OgreTerrainZonePage.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreTerrainZone.h"
#include "OgreResourceManager.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-------------------------------------------------------------------------
    HeightmapTerrainZonePageSource::HeightmapTerrainZonePageSource()
        : mIsRaw(false), mFlipTerrainZone(false), mPage(0)
    {
    }
    //-------------------------------------------------------------------------
    HeightmapTerrainZonePageSource::~HeightmapTerrainZonePageSource()
    {
        shutdown();
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainZonePageSource::shutdown(void)
    {
        // Image will destroy itself
        OGRE_DELETE mPage;
        mPage = 0;
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainZonePageSource::loadHeightmap(void)
    {
        size_t imgSize;
        // Special-case RAW format
        if (mIsRaw)
        {
            // Image size comes from setting (since RAW is not self-describing)
            imgSize = mRawSize;
            
            // Load data
            mRawData.setNull();
            DataStreamPtr stream = 
                ResourceGroupManager::getSingleton().openResource(
                    mSource, ResourceGroupManager::getSingleton().getWorldResourceGroupName());
            mRawData = MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(mSource, stream));

            // Validate size
            size_t numBytes = imgSize * imgSize * mRawBpp;
            if (mRawData->size() != numBytes)
            {
                shutdown();
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "RAW size (" + StringConverter::toString(mRawData->size()) + 
                    ") does not agree with configuration settings.", 
                    "HeightmapTerrainZonePageSource::loadHeightmap");
            }
        }
        else
        {
            mImage.load(mSource, ResourceGroupManager::getSingleton().getWorldResourceGroupName());
            // Must be square (dimensions checked later)
            if ( mImage.getWidth() != mImage.getHeight())
            {
                shutdown();
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Heightmap must be square",
                    "HeightmapTerrainZonePageSource::loadHeightmap");
            }
            imgSize = mImage.getWidth();
        }
        //check to make sure it's the expected size
        if ( imgSize != mPageSize)
        {
            shutdown();
            String err = "Error: Invalid heightmap size : " +
                StringConverter::toString( imgSize ) +
                ". Should be " + StringConverter::toString(mPageSize);
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, err, 
                "HeightmapTerrainZonePageSource::loadHeightmap" );
        }

    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainZonePageSource::initialise(TerrainZone* tsm, 
        ushort tileSize, ushort pageSize, bool asyncLoading, 
        TerrainZonePageSourceOptionList& optionList)
    {
        // Shutdown to clear any previous data
        shutdown();

        TerrainZonePageSource::initialise(tsm, tileSize, pageSize, asyncLoading, optionList);

        // Get source image
        TerrainZonePageSourceOptionList::iterator ti, tiend;
        tiend = optionList.end();
        bool imageFound = false;
        mIsRaw = false;
        bool rawSizeFound = false;
        bool rawBppFound = false;
        for (ti = optionList.begin(); ti != tiend; ++ti)
        {
            String val = ti->first;
            StringUtil::trim(val);
            if (StringUtil::startsWith(val, "Heightmap.image", false))
            {
                mSource = ti->second;
                imageFound = true;
                // is it a raw?
                if (StringUtil::endsWith(mSource, "raw"))
                {
                    mIsRaw = true;
                }
            }
            else if (StringUtil::startsWith(val, "Heightmap.raw.size", false))
            {
                mRawSize = atoi(ti->second.c_str());
                rawSizeFound = true;
            }
            else if (StringUtil::startsWith(val, "Heightmap.raw.bpp", false))
            {
                mRawBpp = atoi(ti->second.c_str());
                if (mRawBpp < 1 || mRawBpp > 2)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                        "Invalid value for 'Heightmap.raw.bpp', must be 1 or 2",
                        "HeightmapTerrainZonePageSource::initialise");
                }
                rawBppFound = true;
            }
            else if (StringUtil::startsWith(val, "Heightmap.flip", false))
            {
                mFlipTerrainZone = StringConverter::parseBool(ti->second);
            }
            else
            {
                LogManager::getSingleton().logMessage("Warning: ignoring unknown Heightmap option '"
                    + val + "'");
            }
        }
        if (!imageFound)
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Missing option 'Heightmap.image'", 
                "HeightmapTerrainZonePageSource::initialise");
        }
        if (mIsRaw && 
            (!rawSizeFound || !rawBppFound))
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Options 'Heightmap.raw.size' and 'Heightmap.raw.bpp' must "
                "be specified for RAW heightmap sources", 
                "HeightmapTerrainZonePageSource::initialise");
        }
        // Load it!
        loadHeightmap();
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainZonePageSource::requestPage(ushort x, ushort y)
    {
        // Only 1 page provided
        if (x == 0 && y == 0 && !mPage)
        {
            // Convert the image data to unscaled floats
            ulong totalPageSize = mPageSize * mPageSize; 
            Real *heightData = OGRE_ALLOC_T(Real, totalPageSize, MEMCATEGORY_RESOURCE);
            const uchar* pOrigSrc, *pSrc;
            Real* pDest = heightData;
            Real invScale;
            bool is16bit = false;
            
            if (mIsRaw)
            {
                pOrigSrc = mRawData->getPtr();
                is16bit = (mRawBpp == 2);
            }
            else
            {
                PixelFormat pf = mImage.getFormat();
                if (pf != PF_L8 && pf != PF_L16)
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                        "Error: Image is not a grayscale image.",
                        "HeightmapTerrainZonePageSource::requestPage" );
                }

                pOrigSrc = mImage.getData();
                is16bit = (pf == PF_L16);
            }
            // Determine mapping from fixed to floating
            ulong rowSize;
            if ( is16bit )
            {
                invScale = 1.0f / 65535.0f; 
                rowSize =  mPageSize * 2;
            }
            else 
            {
                invScale = 1.0f / 255.0f; 
                rowSize =  mPageSize;
            }
            // Read the data
            pSrc = pOrigSrc;
            for (ulong j = 0; j < mPageSize; ++j)
            {
                if (mFlipTerrainZone)
                {
                    // Work backwards 
                    pSrc = pOrigSrc + (rowSize * (mPageSize - j - 1));
                }
                for (ulong i = 0; i < mPageSize; ++i)
                {
                    if (is16bit)
                    {
                        #if OGRE_ENDIAN == OGRE_ENDIAN_BIG
                            ushort val = *pSrc++ << 8;
                            val += *pSrc++;
                        #else
                            ushort val = *pSrc++;
                            val += *pSrc++ << 8;
                        #endif
                        *pDest++ = Real(val) * invScale;
                    }
                    else
                    {
                        *pDest++ = Real(*pSrc++) * invScale;
                    }
                }
            }

            // Call listeners
            firePageConstructed(0, 0, heightData);
            // Now turn into TerrainZonePage
            // Note that we're using a single material for now
            if (mTerrainZone)
            {
                mPage = buildPage(heightData, 
                    mTerrainZone->getOptions().terrainMaterial);
                mTerrainZone->attachPage(0, 0, mPage);
            }

            // Free temp store
            OGRE_FREE(heightData, MEMCATEGORY_RESOURCE);
        }
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainZonePageSource::expirePage(ushort x, ushort y)
    {
        // Single page
        if (x == 0 && y == 0 && mPage)
        {
            OGRE_DELETE mPage;
            mPage = 0;
        }

    }
    //-------------------------------------------------------------------------


}
