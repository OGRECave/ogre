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
        typedef std::vector < TerrainRenderable * > TerrainRow;
        typedef std::vector < TerrainRow > Terrain2D;
        
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
