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
#ifndef __TerrainPage_H__
#define __TerrainPage_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreRenderQueue.h"

namespace Ogre {

    /** Groups a number of TerrainRenderables (tiles) into a page, which is
        the unit of loading / unloading.
    @remarks
        Note that this class, whilst holding onto TerrainRenderable instances, 
        does not actually process or initialise them itself - this is 
        intentional so the TerrainPageSource which is providing the tiles
        is able to load and prepare each renderable incrementally if required, 
        thus avoiding any 'single hit' load methods for the page.
    @par
        All this class does do is pre-create a 2D vector of 'slots' in which 
        to place the TerrainRenderable pointers, which it does on 
        construction. Note that this structure is public to allow completely
        free access to users of this class.
    */
	class _OgreOctreePluginExport TerrainPage : public GeometryAllocatedObject
    {
    public:
        typedef vector< TerrainRenderable * >::type TerrainRow;
        typedef vector< TerrainRow >::type Terrain2D;
        
        /// 2-dimensional vector of tiles, pre-allocated to the correct size
        Terrain2D tiles;
        /// The number of tiles across a page
        unsigned short tilesPerPage;
        /// The scene node to which all the tiles for this page are attached
        SceneNode* pageSceneNode;

        /** The main constructor. 
        @param numTiles The number of terrain tiles (TerrainRenderable)
            across (and down) a page
        */
        TerrainPage(unsigned short numTiles);

        /** Destructor, will organise the deletion of pages
        */
        virtual ~TerrainPage();
        /** After TerrainRenderables have been populated, this method
            adds the neighbour links. 
        @remarks
            Should be called before adding the page to the scene manager.
        */
        void linkNeighbours(void);

        /** Returns the TerrainRenderable that contains the given pt.
        If no tile exists at the point, it returns 0;
        */
        TerrainRenderable * getTerrainTile( const Vector3 & pt );

		/** Sets the render queue group which the tiles should be rendered in. */
		void setRenderQueue(uint8 qid);


    };


}

#endif
