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
OgreTerrainZonePage.cpp  -  based on OgreTerrainPage.cpp from Ogre3d )

-----------------------------------------------------------------------------
begin                : Thu May 3 2007
author               : Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/

#include "OgreTerrainZonePage.h"
#include "OgreTerrainZoneRenderable.h"

namespace Ogre {

    //-------------------------------------------------------------------------
    TerrainZonePage::TerrainZonePage(unsigned short numTiles)
    {
        tilesPerPage = numTiles;
        // Set up an empty array of TerrainZoneRenderable pointers
        int i, j;
        for ( i = 0; i < tilesPerPage; i++ )
        {
            tiles.push_back( TerrainZoneRow() );

            for ( j = 0; j < tilesPerPage; j++ )
            {
                tiles[ i ].push_back( 0 );
            }
        }

        pageSceneNode = 0;

    }
    //-------------------------------------------------------------------------
    TerrainZonePage::~TerrainZonePage()
    {
        TerrainZone2D::iterator i, iend;
        iend = tiles.end();
        for (i = tiles.begin(); i != iend; ++i)
        {
            TerrainZoneRow::iterator j, jend;
            jend = i->end();
            for (j = i->begin(); j != jend; ++j)
            {
                OGRE_DELETE *j;
                *j = 0;
            }
        }

    }
    //-------------------------------------------------------------------------
    void TerrainZonePage::linkNeighbours(void)
    {
        //setup the neighbor links.

        for ( unsigned short j = 0; j < tilesPerPage; j++ )
        {
            for ( unsigned short i = 0; i < tilesPerPage; i++ )
            {
                if ( j != tilesPerPage - 1 )
                {
                    tiles[ i ][ j ] -> _setNeighbor( TerrainZoneRenderable::SOUTH, tiles[ i ][ j + 1 ] );
                    tiles[ i ][ j + 1 ] -> _setNeighbor( TerrainZoneRenderable::NORTH, tiles[ i ][ j ] );
                }

                if ( i != tilesPerPage - 1 )
                {
                    tiles[ i ][ j ] -> _setNeighbor( TerrainZoneRenderable::EAST, tiles[ i + 1 ][ j ] );
                    tiles[ i + 1 ][ j ] -> _setNeighbor( TerrainZoneRenderable::WEST, tiles[ i ][ j ] );
                }

            }
        }
    }
    //-------------------------------------------------------------------------
    TerrainZoneRenderable * TerrainZonePage::getTerrainZoneTile( const Vector3 & pt )
    {
        /* Since we don't know if the terrain is square, or has holes, we use a line trace
        to find the containing tile...
        */

        TerrainZoneRenderable * tile = tiles[ 0 ][ 0 ];

        while ( tile != 0 )
        {
            AxisAlignedBox b = tile -> getBoundingBox();

            if ( pt.x < b.getMinimum().x )
                tile = tile -> _getNeighbor( TerrainZoneRenderable::WEST );
            else if ( pt.x > b.getMaximum().x )
                tile = tile -> _getNeighbor( TerrainZoneRenderable::EAST );
            else if ( pt.z < b.getMinimum().z )
                tile = tile -> _getNeighbor( TerrainZoneRenderable::NORTH );
            else if ( pt.z > b.getMaximum().z )
                tile = tile -> _getNeighbor( TerrainZoneRenderable::SOUTH );
            else
                return tile;
        }

        return 0;
    }
	//-------------------------------------------------------------------------
	void TerrainZonePage::setRenderQueue(uint8 qid)
	{
		for ( unsigned short j = 0; j < tilesPerPage; j++ )
		{
			for ( unsigned short i = 0; i < tilesPerPage; i++ )
			{
				if ( j != tilesPerPage - 1 )
				{
					tiles[ i ][ j ]->setRenderQueueGroup(qid);
				}
			}
		}
	}

}

