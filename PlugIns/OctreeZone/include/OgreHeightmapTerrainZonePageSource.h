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
OgreHeightmapTerrainZonePage.h  -  Almost same as OgreHeightmapTerrainPage.h from 
Ogre3d just slightly modified & renamed to avoid confusion with the TerrainSM version.

-----------------------------------------------------------------------------
begin                : Thu May 3 2007
author               : Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/

#ifndef __HeightmapTerrainZonePageSource_H__
#define __HeightmapTerrainZonePageSource_H__

#include "OgreTerrainZonePrerequisites.h"
#include "OgreTerrainZonePageSource.h"
#include "OgreImage.h"

namespace Ogre {

    /** Specialisation of the TerrainZonePageSource class to provide tiles loaded
        from a 2D greyscale image.
    @remarks
        This is a simple tile provider that does not support paging; it is
        assumed that the entire heightmap is loaded as one page.
    */
    class _OgreOctreeZonePluginExport HeightmapTerrainZonePageSource : public TerrainZonePageSource
    {
    protected:
        /// Is this input RAW?
        bool mIsRaw;
        /// Should we flip terrain vertically?
        bool mFlipTerrainZone;
        /// Image containing the source heightmap if loaded from non-RAW
        Image mImage;
        /// Arbitrary data loaded from RAW
        MemoryDataStreamPtr mRawData;
        /// The (single) terrain page this source will provide
        TerrainZonePage* mPage;
        /// Source file name
        String mSource;
        /// Manual size if source is RAW
        size_t mRawSize;
        /// Manual bpp if source is RAW
        uchar mRawBpp;
        
        /// Load a heightmap
        void loadHeightmap(void);
    public:
        HeightmapTerrainZonePageSource();
        ~HeightmapTerrainZonePageSource();
        /// @see TerrainZonePageSource
        void shutdown(void);
        /// @see TerrainZonePageSource
        void requestPage(ushort x, ushort y);
        /// @see TerrainZonePageSource
        void expirePage(ushort x, ushort y);
        /// @see TerrainZonePageSource
        void initialise(TerrainZone* tsm, 
            ushort tileSize, ushort pageSize, bool asyncLoading, 
            TerrainZonePageSourceOptionList& optionList);
    };
}

#endif
