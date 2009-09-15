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
*/
#ifndef __HeightmapTerrainPageSource_H__
#define __HeightmapTerrainPageSource_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreTerrainPageSource.h"
#include "OgreImage.h"

namespace Ogre {

    /** Specialisation of the TerrainPageSource class to provide tiles loaded
        from a 2D greyscale image.
    @remarks
        This is a simple tile provider that does not support paging; it is
        assumed that the entire heightmap is loaded as one page.
    */
    class _OgreOctreePluginExport HeightmapTerrainPageSource : public TerrainPageSource
    {
    protected:
        /// Is this input RAW?
        bool mIsRaw;
        /// Should we flip terrain vertically?
        bool mFlipTerrain;
        /// Image containing the source heightmap if loaded from non-RAW
        Image mImage;
        /// Arbitrary data loaded from RAW
        MemoryDataStreamPtr mRawData;
        /// The (single) terrain page this source will provide
        TerrainPage* mPage;
        /// Source file name
        String mSource;
        /// Manual size if source is RAW
        size_t mRawSize;
        /// Manual bpp if source is RAW
        uchar mRawBpp;
        
        /// Load a heightmap
        void loadHeightmap(void);
    public:
        HeightmapTerrainPageSource();
        ~HeightmapTerrainPageSource();
        /// @see TerrainPageSource
        void shutdown(void);
        /// @see TerrainPageSource
        void requestPage(ushort x, ushort y);
        /// @see TerrainPageSource
        void expirePage(ushort x, ushort y);
        /// @see TerrainPageSource
        void initialise(TerrainSceneManager* tsm, 
            ushort tileSize, ushort pageSize, bool asyncLoading, 
            TerrainPageSourceOptionList& optionList);
    };
}

#endif
