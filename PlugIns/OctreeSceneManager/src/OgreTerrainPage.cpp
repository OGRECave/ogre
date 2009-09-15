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
#include "OgreTerrainPage.h"
#include "OgreTerrainRenderable.h"

namespace Ogre {

    //-------------------------------------------------------------------------
    TerrainPage::TerrainPage(unsigned short numTiles)
    {
        tilesPerPage = numTiles;
        // Set up an empty array of TerrainRenderable pointers
        int i, j;
        for ( i = 0; i < tilesPerPage; i++ )
        {
            tiles.push_back( TerrainRow() );

            for ( j = 0; j < tilesPerPage; j++ )
            {
                tiles[ i ].push_back( 0 );
            }
        }

        pageSceneNode = 0;

    }
    //-------------------------------------------------------------------------
    TerrainPage::~TerrainPage()
    {
        Terrain2D::iterator i, iend;
        iend = tiles.end();
        for (i = tiles.begin(); i != iend; ++i)
        {
            TerrainRow::iterator j, jend;
            jend = i->end();
            for (j = i->begin(); j != jend; ++j)
            {
                OGRE_DELETE *j;
                *j = 0;
            }
        }

    }
    //-------------------------------------------------------------------------
    void TerrainPage::linkNeighbours(void)
    {
        //setup the neighbor links.

        for ( ushort j = 0; j < tilesPerPage; j++ )
        {
            for ( ushort i = 0; i < tilesPerPage; i++ )
            {
                if ( j != tilesPerPage - 1 )
                {
                    tiles[ i ][ j ] -> _setNeighbor( TerrainRenderable::SOUTH, tiles[ i ][ j + 1 ] );
                    tiles[ i ][ j + 1 ] -> _setNeighbor( TerrainRenderable::NORTH, tiles[ i ][ j ] );
                }

                if ( i != tilesPerPage - 1 )
                {
                    tiles[ i ][ j ] -> _setNeighbor( TerrainRenderable::EAST, tiles[ i + 1 ][ j ] );
                    tiles[ i + 1 ][ j ] -> _setNeighbor( TerrainRenderable::WEST, tiles[ i ][ j ] );
                }

            }
        }
    }
    //-------------------------------------------------------------------------
    TerrainRenderable * TerrainPage::getTerrainTile( const Vector3 & pt )
    {
        /* Since we don't know if the terrain is square, or has holes, we use a line trace
        to find the containing tile...
        */

        TerrainRenderable * tile = tiles[ 0 ][ 0 ];

        while ( tile != 0 )
        {
            AxisAlignedBox b = tile -> getBoundingBox();

            if ( pt.x < b.getMinimum().x )
                tile = tile -> _getNeighbor( TerrainRenderable::WEST );
            else if ( pt.x > b.getMaximum().x )
                tile = tile -> _getNeighbor( TerrainRenderable::EAST );
            else if ( pt.z < b.getMinimum().z )
                tile = tile -> _getNeighbor( TerrainRenderable::NORTH );
            else if ( pt.z > b.getMaximum().z )
                tile = tile -> _getNeighbor( TerrainRenderable::SOUTH );
            else
                return tile;
        }

        return 0;
    }
	//-------------------------------------------------------------------------
	void TerrainPage::setRenderQueue(uint8 qid)
	{
		for ( ushort j = 0; j < tilesPerPage; j++ )
		{
			for ( ushort i = 0; i < tilesPerPage; i++ )
			{
				if ( j != tilesPerPage - 1 )
				{
					tiles[ i ][ j ]->setRenderQueueGroup(qid);
				}
			}
		}
	}

}

