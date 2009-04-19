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

#ifndef __Ogre_TerrainQuadTreeNode_H__
#define __Ogre_TerrainQuadTreeNode_H__

#include "OgreTerrainPrerequisites.h"



namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** A node in a quad tree used to store a patch of terrain.
	@remarks
		<b>Algorithm overview:</b>
	@par
		Our goal is to perform traditional chunked LOD with geomorphing. But, 
		instead of just dividing the terrain into tiles, we will divide them into
		a hierarchy of tiles, a quadtree, where any level of the quadtree can 
		be a rendered tile (to the exclusion of its children). The idea is to 
		collect together children into a larger batch with their siblings as LOD 
		decreases, to improve performance.
	@par
		The minBatchSize and maxBatchSize parameters on Terrain a key to 
		defining this behaviour. Both values are expressed in vertices down one axis.
		maxBatchSize determines the number of tiles on one side of the terrain,
		which is numTiles = (terrainSize-1) / (maxBatchSize-1). This in turn determines the depth
		of the quad tree, which is sqrt(numTiles). The minBatchSize determines
		the 'floor' of how low the number of vertices can go in a tile before it
		has to be grouped together with its siblings to drop any lower. We also do not group 
		a tile with its siblings unless all of them are at this minimum batch size, 
		rather than trying to group them when they all end up on the same 'middle' LOD;
		this is for several reasons; firstly, tiles hitting the same 'middle' LOD is
		less likely and more transient if they have different levels of 'roughness',
		and secondly since we're sharing a vertex / index pool between all tiles, 
		only grouping at the min level means that the number of combinations of 
		buffer sizes for any one tile is greatly simplified, making it easier to 
		pool data. To be more specific, any tile / quadtree node can only have
		log2(maxBatchSize-1) - log2(minBatchSize-1) + 1 LOD levels (and if you set them 
		to the same value, LOD can only change by going up/down the quadtree).
		The numbers of vertices / indices in each of these levels is constant for
		the same (relative) LOD index no matter where you are in the tree, therefore
		buffers can potentially be reused more easily.
	*/
	class _OgreTerrainExport TerrainQuadTreeNode : public TerrainAlloc
	{
	public:
		/** Constructor.
		@param terrain The ultimate parent terrain
		@param parent Optional parent node (in which case xoff, yoff are 0 and size must be entire terrain)
		@param xoff,off Offsets from the start of the terrain data in 2D
		@param size The size of the node in vertices at the highest LOD
		*/
		TerrainQuadTreeNode(Terrain* terrain, TerrainQuadTreeNode* parent, 
			uint16 xoff, uint16 yoff, uint16 size);
		virtual ~TerrainQuadTreeNode();


		/// Is this a leaf node (no children)
		bool isLeaf() const;
		/// Get child node
		TerrainQuadTreeNode* getChild(unsigned short child) const;
		/// Get parent node
		TerrainQuadTreeNode* getParent() const;

		/// Prepare node and children (perform CPU tasks, may be background thread)
		void prepare();
		/// Load node and children (perform GPU tasks, will be render thread)
		void load();
		/// Unload node and children (perform GPU tasks, will be render thread)
		void unload();
		/// Unprepare node and children (perform CPU tasks, may be background thread)
		void unprepare();

	protected:
		Terrain* mTerrain;
		TerrainQuadTreeNode* mParent;
		TerrainQuadTreeNode* mChildren[4];

		uint16 mOffsetX, mOffsetY;
		uint16 mSize;

	};





	/** @} */
	/** @} */
}




#endif 