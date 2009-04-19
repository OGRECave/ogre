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
#include "OgreCommon.h"
#include "OgreVector3.h"


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
	<tr>
		<td>Terrain size</td>
		<td>uint16</td>
		<td>The number of vertices along one side of the terrain</td>
	</tr>
	<tr>
		<td>Terrain world size</td>
		<td>Real</td>
		<td>The world size of one side of the terrain</td>
	</tr>
	<tr>
		<td>Max batch size</td>
		<td>uint16</td>
		<td>The maximum batch size in vertices along one side</td>
	</tr>
	<tr>
		<td>Min batch size</td>
		<td>uint16</td>
		<td>The minimum batch size in vertices along one side</td>
	</tr>
	<tr>
		<td>Position</td>
		<td>Vector3</td>
		<td>The location of the centre of the terrain</td>
	</tr>
	<tr>
		<td>Height data</td>
		<td>float[size*size]</td>
		<td>List of floating point heights</td>
	</tr>
	<tr>
		<td>Delta data</td>
		<td>float[size*size]</td>
		<td>List of floating point delta values for each vertex, storing the difference
			between its real height, and the height it will become on the first
			LOD level in which it is eliminated.</td>
	</tr>
	</table>
	*/
	class _OgreTerrainExport Terrain : public TerrainAlloc
	{
	public:
		Terrain(SceneManager* sm);
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
			collected together into one batch to drop LOD levels once they are individually at this minimum,
			so setting this value higher means greater batching at the expense
			of making adjacent tiles use a common LOD.
			Once the entire terrain is collected together into one batch this 
			effectively sets the minimum LOD.
			*/
			uint16 minBatchSize;

			/** Position of the terrain.
			@remarks
				Represents the position of the centre of the terrain. 
			*/
			Vector3 pos;

			/** The world size of the terrain. */
			Real worldSize;

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
				, pos(Vector3::ZERO)
				, worldSize(1000)
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


		/** Get a pointer to all the height data for this terrain.
		@remarks
			The height data is in world coordinates, relative to the position 
			of the terrain.
		@par
			This pointer is not const, so you can update the height data if you
			wish. However, changes will not be propagated until you call 
			Terrain::dirty or Terrain::dirtyRect.
		*/
		float* getHeightData();

		/** Get a pointer to the height data for a given point. 
		*/
		float* getHeightData(long x, long y);

		/** Get a Vector3 of the world-space point on the terrain, aligned as per
			options.
		@note This point is relative to Terrain::getPosition
		*/
		void getPoint(long x, long y, Vector3* outpos);

		/// Get the alignment of the terrain
		Alignment getAlignment() const;
		/// Get the size of the terrain in vertices along one side
		uint16 getSize() const;
		/// Get the maximum size in vertices along one side of a batch 
		uint16 getMaxBatchSize() const;
		/// Get the minimum size in vertices along one side of a batch 
		uint16 getMinBatchSize() const;
		/// Get the size of the terrain in world units
		Real getWorldSize() const;

		/// Get the world position of the terrain centre
		const Vector3& getPosition() const { return mPos; }
		/// Set the position of the terrain centre in world coordinates
		void setPosition(const Vector3& pos);
		/** Mark the entire terrain as dirty, so that the geometry is
			updated the next time it is accessed. 
		*/
		void dirty();

		/** Mark a region of the terrain as dirty, so that the geometry is 
			updated the next time it is accessed. 
		@param rect A rectangle expressed in vertices describing the dirty region;
			left <= right, bottom <= top
		*/
		void dirtyRect(const Rect& rect);


		/** Whether to use triangle strips or not then rendering terrain
			(default true, set for new Terrain using TerrainGlobalOptions)
		*/
		bool getUseTriangleStrips() { return mUseTriangleStrips; }
		/** Whether to morph between LODs using a vertex program. 
			(default true, set for new Terrain using TerrainGlobalOptions)
		*/
		bool getUseLodMorph() { return mUseLodMorph; }
		/** The default size of 'skirts' used to hide terrain cracks
			(default 10, set for new Terrain using TerrainGlobalOptions)
		*/
		Real getSkirtSize() { return mSkirtSize; }

		/** Calculate (or recalculate) the delta values of heights between a vertex
			in its recorded position, and the place it will end up in the LOD
			in which it is removed. 
		@param rect Rectangle describing the area to calculate (left <= right, bottom <= top)
			
		*/
		void calculateHeightDeltas(const Rect& rect);

		/** Get the maximum vertical difference between any vertex in one 
			LOD level and the implicit position it would be in the level below it
			because it has been removed. 
		@note
			This method assumes that height deltas are up to date (@see calculateHeightDeltas)
		@param rect Rectangle describing the (inclusive) set of points to 
			be calculated (left <= right, bottom <= top)
		@param srcLOD The level of detail from which to reduce from (0 is the 
			highest level)
		*/
		Real getMaxHeightDelta(const Rect& rect, unsigned short srcLOD);
	protected:

		void freeCPUResources();
		void freeGPUResources();
		void determineLodLevels();
		void updateBaseScale();
		/** Get a Vector3 of the world-space point on the terrain, aligned Y-up always.
		@note This point is relative to Terrain::getPosition
		*/
		void getPointAlign(long x, long y, Alignment align, Vector3* outpos);

		SceneManager* mSceneMgr;
		SceneNode* mRootNode;
		
		/// The height data (world coords relative to mPos)
		float* mHeightData;
		/// The delta information defining how a vertex moves before it is removed at a lower LOD
		float* mDeltaData;
		Alignment mAlign;
		Real mWorldSize;
		uint16 mSize;
		uint16 mMaxBatchSize;
		uint16 mMinBatchSize;
		Vector3 mPos;
		TerrainQuadTreeNode* mQuadTree;
		uint16 mNumLodLevels;
		uint16 mTreeDepth;
		Real mBase;
		Real mScale;

		bool mUseTriangleStrips;
		bool mUseLodMorph;
		Real mSkirtSize;

	};



	class _OgreTerrainExport TerrainGlobalOptions
	{
	protected:
		// no instantiation
		TerrainGlobalOptions() {}

		static bool msUseTriangleStrips;
		static bool msUseLodMorph;
		static Real msSkirtSize;

	public:


		/** Static method - whether to use triangle strips or not then rendering terrain
		(default true)
		*/
		static bool getUseTriangleStrips() { return msUseTriangleStrips; }
		/** Static method - whether to use triangle strips or not then rendering terrain
		(default true)
		@remarks
			Changing this value only applies to Terrain instances loaded / reloaded afterwards.
		*/
		static void setUseTriangleStrips(bool useStrips) { msUseTriangleStrips = useStrips; }
		/** Static method - whether to morph between LODs using a vertex program. 
		(default true)
		*/
		static bool getUseLodMorph() { return msUseLodMorph; }
		/** Static method - whether to morph between LODs using a vertex program. 
		(default true)
		@remarks
			Changing this value only applies to Terrain instances loaded / reloaded afterwards.
		*/
		static void setUseLodMorph(bool useMorph) { msUseLodMorph = useMorph; }
		/** Static method - the default size of 'skirts' used to hide terrain cracks
		(default 10)
		*/
		static Real getSkirtSize() { return msSkirtSize; }
		/** Static method - the default size of 'skirts' used to hide terrain cracks
		(default 10)
		@remarks
			Changing this value only applies to Terrain instances loaded / reloaded afterwards.
		*/
		static void setSkirtSize(Real skirtSz) { msSkirtSize = skirtSz; }
	};


	/** @} */
	/** @} */
}




#endif 