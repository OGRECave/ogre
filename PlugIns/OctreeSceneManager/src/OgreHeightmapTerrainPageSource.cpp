/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

#include "OgreHeightmapTerrainPageSource.h"
#include "OgreTerrainPage.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreTerrainSceneManager.h"
#include "OgreResourceManager.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-------------------------------------------------------------------------
    HeightmapTerrainPageSource::HeightmapTerrainPageSource()
        : mIsRaw(false), mFlipTerrain(false), mPage(0)
    {
    }
    //-------------------------------------------------------------------------
    HeightmapTerrainPageSource::~HeightmapTerrainPageSource()
    {
        shutdown();
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainPageSource::shutdown(void)
    {
        // Image will destroy itself
        OGRE_DELETE mPage;
        mPage = 0;
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainPageSource::loadHeightmap(void)
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
                    "HeightmapTerrainPageSource::loadHeightmap");
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
                    "HeightmapTerrainPageSource::loadHeightmap");
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
                "HeightmapTerrainPageSource::loadHeightmap" );
        }

    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainPageSource::initialise(TerrainSceneManager* tsm, 
        ushort tileSize, ushort pageSize, bool asyncLoading, 
        TerrainPageSourceOptionList& optionList)
    {
        // Shutdown to clear any previous data
        shutdown();

        TerrainPageSource::initialise(tsm, tileSize, pageSize, asyncLoading, optionList);

        // Get source image
        TerrainPageSourceOptionList::iterator ti, tiend;
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
                        "HeightmapTerrainPageSource::initialise");
                }
                rawBppFound = true;
            }
            else if (StringUtil::startsWith(val, "Heightmap.flip", false))
            {
                mFlipTerrain = StringConverter::parseBool(ti->second);
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
                "HeightmapTerrainPageSource::initialise");
        }
        if (mIsRaw && 
            (!rawSizeFound || !rawBppFound))
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Options 'Heightmap.raw.size' and 'Heightmap.raw.bpp' must "
                "be specified for RAW heightmap sources", 
                "HeightmapTerrainPageSource::initialise");
        }
        // Load it!
        loadHeightmap();
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainPageSource::requestPage(ushort x, ushort y)
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
                        "HeightmapTerrainPageSource::requestPage" );
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
                if (mFlipTerrain)
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
            // Now turn into TerrainPage
            // Note that we're using a single material for now
            if (mSceneManager)
            {
                mPage = buildPage(heightData, 
                    mSceneManager->getOptions().terrainMaterial);
                mSceneManager->attachPage(0, 0, mPage);
            }

            // Free temp store
            OGRE_FREE(heightData, MEMCATEGORY_RESOURCE);
        }
    }
    //-------------------------------------------------------------------------
    void HeightmapTerrainPageSource::expirePage(ushort x, ushort y)
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
