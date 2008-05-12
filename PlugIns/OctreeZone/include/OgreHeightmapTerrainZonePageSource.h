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
