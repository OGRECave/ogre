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

#ifndef __Ogre_Terrain_H__
#define __Ogre_Terrain_H__

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


	/** The main containing class for a chunk of terrain.
	@par
		Terrain can be edited and stored.
	The data format for this in a file is:<br/>
	<b>TerrainData (Identifier 'TERR')</b>\n
	[Version 1]
	<table>
	<tr>
	<td><b>Name</b></td>
	<td><b>Type</b></td>
	<td><b>Description</b></td>
	</tr>
	<tr>
	<td>Terrain orientation</td>
	<td>uint8</td>
	<td>The orientation of the terrain; XZ = 0, XY = 1, YZ = 2</td>
	</tr>
	</table>
	*/
	class _OgreTerrainExport Terrain : public TerrainAlloc
	{
	public:
		Terrain();
		virtual ~Terrain();

		static const uint32 TERRAIN_CHUNK_ID;
		static const uint16 TERRAIN_CHUNK_VERSION;

		/// The alignment of the terrain
		enum Alignment
		{
			/// Terrain is in the X/Z plane
			ALIGN_X_Z = 0, 
			/// Terrain is in the X/Y plane
			ALIGN_X_Y = 1, 
			/// Terrain is in the Y/Z plane
			ALIGN_Y_Z = 2
		};

		/** Structure encapsulating import data that you may use to bootstrap 
			the terrain without loading from a native data stream. 
		*/
		struct ImportData
		{
			/// The alignment of the terrain
			Alignment terrainAlign;
			/// Terrain size (along one edge) in vertices; must be 2^n+1
			uint16 terrainSize;
			/** Maximum batch size (along one edge) in vertices; must be 2^n+1 and <= 65
			@remarks
				The terrain will be divided into hierarchical tiles, and this is the maximum
				size of one tile in vertices (at any LOD).
			*/
			uint16 maxBatchSize;
			/** Minimum batch size (along one edge) in vertices; must be 2^n+1.
			@remarks
			The terrain will be divided into tiles, and this is the minimum
			size of one tile in vertices (at any LOD). Adjacent tiles will be
			collected together into one batch once they are individually at this minimum,
			so setting this value higher means greater batching at the expense
			of making adjacent tiles use a common LOD.
			Only when the entire terrain is collected together into one batch can 
			the batch size become smaller than this.
			*/
			uint16 minBatchSize;

			/** Where a height of zero is positioned in world space. */
			Real baseHeight;

			/** Optional heightmap providing the initial heights for the terrain. 
			@remarks
				If supplied, should ideally be terrainSize * terrainSize, but if
				it isn't it will be resized.
			*/
			Image* inputImage;

			/// Optional list of terrainSize * terrainSize floats defining the terrain. 
			float* inputFloat;

			/// How to scale the input values provided (if any)
			Real inputScale;
			/// How to bias the input values provided (if any)
			Real inputBias;

			// TODO - add options for UV generation, tangents etc

			ImportData() 
				: terrainAlign(ALIGN_X_Z)
				, terrainSize(1025)
				, maxBatchSize(65)
				, minBatchSize(17)
				, baseHeight(0)
				, inputImage(0)
				, inputFloat(0)
				, inputScale(1.0)
				, inputBias(0.0)
			{
			}

		};

		/// Save terrain data in native form
		void save(StreamSerialiser& stream);
		/** Prepare terrain data from saved data.
		@remarks
			This is safe to do in a background thread as it creates no GPU resources.
			It reads data from a native terrain data chunk. 
		@returns true if the preparation was successful
		*/
		bool prepare(StreamSerialiser& stream);

		/** Prepare the terrain from some import data rather than loading from 
			native data. 
		@remarks
			This method may be called in a background thread.
		*/
		bool prepare(const ImportData& importData);

		/** Load the terrain based on the data already populated via prepare methods. 
		@remarks
			This method must be called in the main render thread. 
		*/
		void load();


		/** Unload the terrain and free GPU resources. 
		@remarks
			This method must be called in the main render thread.
		*/
		void unload();

		/** Free CPU resources created during prepare methods.
		@remarks
			This is safe to do in a background thread after calling unload().
		*/
		void unprepare();
	protected:


	};





	/** @} */
	/** @} */
}




#endif 