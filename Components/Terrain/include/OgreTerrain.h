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

#ifndef __Ogre_Terrain_H__
#define __Ogre_Terrain_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreCommon.h"
#include "OgreVector3.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSceneManager.h"
#include "OgreTerrainMaterialGenerator.h"
#include "OgreTerrainLayerBlendMap.h"
#include "OgreWorkQueue.h"

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
		<td>LayerDeclaration</td>
		<td>LayerDeclaration*</td>
		<td>The layer declaration for this terrain (see below)</td>
	</tr>
	<tr>
		<td>Layer count</td>
		<td>uint8</td>
		<td>The number of layers in this terrain</td>
	</tr>
	<tr>
		<td>LayerInstance list</td>
		<td>LayerInstance*</td>
		<td>A number of LayerInstance definitions based on layer count (see below)</td>
	</tr>
	<tr>
		<td>Layer blend map size</td>
		<td>uint16</td>
		<td>The size of the layer blend maps as stored in this file</td>
	</tr>
	<tr>
		<td>Packed blend texture data</td>
		<td>uint8*</td>
		<td>layerCount-1 sets of blend texture data interleaved as either RGB or RGBA 
			depending on layer count</td>
	</tr>
	<tr>
		<td>Optional derived map data</td>
		<td>TerrainDerivedMap list</td>
		<td>0 or more sets of map data derived from the original terrain</td>
	</tr>
	<tr>
		<td>Delta data</td>
		<td>float[size*size]</td>
		<td>At each vertex, delta information for the LOD at which this vertex disappears</td>
	</tr>
	<tr>
		<td>Quadtree delta data</td>
		<td>float[quadtrees*lods]</td>
		<td>At each quadtree node, for each lod a record of the max delta value in the region</td>
	</tr>
	</table>
	<b>TerrainLayerDeclaration (Identifier 'TDCL')</b>\n
	[Version 1]
	<table>
	<tr>
		<td><b>Name</b></td>
		<td><b>Type</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td>TerrainLayerSampler Count</td>
		<td>uint8</td>
		<td>Number of samplers in this declaration</td>
	</tr>
	<tr>
		<td>TerrainLayerSampler List</td>
		<td>TerrainLayerSampler*</td>
		<td>List of TerrainLayerSampler structures</td>
	</tr>
	<tr>
		<td>Sampler Element Count</td>
		<td>uint8</td>
		<td>Number of sampler elements in this declaration</td>
	</tr>
	<tr>
		<td>TerrainLayerSamplerElement List</td>
		<td>TerrainLayerSamplerElement*</td>
		<td>List of TerrainLayerSamplerElement structures</td>
	</tr>
	</table>
	<b>TerrainLayerSampler (Identifier 'TSAM')</b>\n
	[Version 1]
	<table>
	<tr>
		<td><b>Name</b></td>
		<td><b>Type</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td>Alias</td>
		<td>String</td>
		<td>Alias name of this sampler</td>
	</tr>
	<tr>
		<td>Format</td>
		<td>uint8</td>
		<td>Desired pixel format</td>
	</tr>
	</table>
	<b>TerrainLayerSamplerElement (Identifier 'TSEL')</b>\n
	[Version 1]
	<table>
	<tr>
		<td><b>Name</b></td>
		<td><b>Type</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td>Source</td>
		<td>uint8</td>
		<td>Sampler source index</td>
	</tr>
	<tr>
		<td>Semantic</td>
		<td>uint8</td>
		<td>Semantic interpretation of this element</td>
	</tr>
	<tr>
		<td>Element start</td>
		<td>uint8</td>
		<td>Start of this element in the sampler</td>
	</tr>
	<tr>
		<td>Element count</td>
		<td>uint8</td>
		<td>Number of elements in the sampler used by this entry</td>
	</tr>
	</table>
	<b>LayerInstance (Identifier 'TLIN')</b>\n
	[Version 1]
	<table>
	<tr>
		<td><b>Name</b></td>
		<td><b>Type</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td>World size</td>
		<td>Real</td>
		<td>The world size of this layer (determines UV scaling)</td>
	</tr>
	<tr>
		<td>Texture list</td>
		<td>String*</td>
		<td>List of texture names corresponding to the number of samplers in the layer declaration</td>
	</tr>
	</table>
	<b>TerrainDerivedData (Identifier 'TDDA')</b>\n
	[Version 1]
	<table>
	<tr>
		<td><b>Name</b></td>
		<td><b>Type</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td>Derived data type name</td>
		<td>String</td>
		<td>Name of the derived data type ('normalmap', 'lightmap', 'colourmap', 'compositemap')</td>
	</tr>
	<tr>
		<td>Size</td>
		<td>uint16</td>
		<td>Size of the data along one edge</td>
	</tr>
	<tr>
		<td>Data</td>
		<td>varies based on type</td>
		<td>The data</td>
	</tr>
	</table>
	*/
	class _OgreTerrainExport Terrain : public SceneManager::Listener, 
		public WorkQueue::RequestHandler, public WorkQueue::ResponseHandler, public TerrainAlloc
	{
	public:
		/** Constructor.
		@param sm The SceneManager to use.
		*/
		Terrain(SceneManager* sm);
		virtual ~Terrain();

		static const uint32 TERRAIN_CHUNK_ID;
		static const uint16 TERRAIN_CHUNK_VERSION;
		static const uint16 TERRAIN_MAX_BATCH_SIZE;

		static const uint32 TERRAINLAYERDECLARATION_CHUNK_ID;
		static const uint16 TERRAINLAYERDECLARATION_CHUNK_VERSION;
		static const uint32 TERRAINLAYERSAMPLER_CHUNK_ID;
		static const uint16 TERRAINLAYERSAMPLER_CHUNK_VERSION;
		static const uint32 TERRAINLAYERSAMPLERELEMENT_CHUNK_ID;
		static const uint16 TERRAINLAYERSAMPLERELEMENT_CHUNK_VERSION;
		static const uint32 TERRAINLAYERINSTANCE_CHUNK_ID;
		static const uint16 TERRAINLAYERINSTANCE_CHUNK_VERSION;
		static const uint32 TERRAINDERIVEDDATA_CHUNK_ID;
		static const uint16 TERRAINDERIVEDDATA_CHUNK_VERSION;

		static const size_t LOD_MORPH_CUSTOM_PARAM;

		typedef vector<Real>::type RealVector;

		/** An instance of a layer, with specific texture names
		*/
		struct _OgreTerrainExport LayerInstance
		{
			/// The world size of the texture to be applied in this layer
			Real worldSize;
			/// List of texture names to import; must match with TerrainLayerDeclaration
			StringVector textureNames;

			LayerInstance()
				: worldSize(100) {}
		};
		typedef vector<LayerInstance>::type LayerInstanceList;

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

			/** Optional list of terrainSize * terrainSize floats defining the terrain. 
				The list of floats wil be interpreted such that the first row
				in the array equates to the bottom row of vertices. 
			*/
			float* inputFloat;

			/** If neither inputImage or inputFloat are supplied, the constant
				height at which the initial terrain should be created (flat). 
			*/
			float constantHeight;

			/** Whether this structure should 'own' the input data (inputImage and
				inputFloat), and therefore delete it on destruction. 
				The default is false so you have to manage your own memory. If you
				set it to true, then you must have allocated the memory through
				OGRE_NEW (for Image) and OGRE_ALLOC_T (for inputFloat), the latter
				with the category MEMCATEGORY_GEOMETRY.
			*/
			bool deleteInputData;

			/// How to scale the input values provided (if any)
			Real inputScale;
			/// How to bias the input values provided (if any)
			Real inputBias;

			/** Definition of the contents of each layer (required).
			Most likely,  you will pull a declaration from a TerrainMaterialGenerator
			of your choice.
			*/
			TerrainLayerDeclaration layerDeclaration;
			/** List of layer structures, one for each layer required.
				Can be empty or underfilled if required, list will be padded with
				blank textures.
			*/
			LayerInstanceList layerList;

			ImportData() 
				: terrainAlign(ALIGN_X_Z)
				, terrainSize(1025)
				, maxBatchSize(65)
				, minBatchSize(17)
				, pos(Vector3::ZERO)
				, worldSize(1000)
				, inputImage(0)
				, inputFloat(0)
				, constantHeight(0)
				, deleteInputData(false)
				, inputScale(1.0)
				, inputBias(0.0)
			{

			}

			ImportData(const ImportData& rhs)
			{
				*this = rhs;
			}

			ImportData& operator=(const ImportData& rhs)
			{
				// basic copy
				terrainAlign = rhs.terrainAlign;
				terrainSize = rhs.terrainSize;
				maxBatchSize = rhs.maxBatchSize;
				minBatchSize = rhs.minBatchSize;
				pos = rhs.pos;
				worldSize = rhs.worldSize;
				constantHeight = rhs.constantHeight;
				deleteInputData = rhs.deleteInputData;
				inputScale = rhs.inputScale;
				inputBias = rhs.inputBias;
				layerDeclaration = rhs.layerDeclaration;
				layerList = rhs.layerList;

				// By-value copies in ownership cases
				if (rhs.deleteInputData)
				{
					if (rhs.inputImage)
						inputImage = OGRE_NEW Image(*rhs.inputImage);
					if (rhs.inputFloat)
					{
						inputFloat = OGRE_ALLOC_T(float, terrainSize*terrainSize, MEMCATEGORY_GEOMETRY);
						memcpy(inputFloat, rhs.inputFloat, sizeof(float) * terrainSize*terrainSize);
					}
				}
				else
				{
					// re-use pointers
					inputImage = rhs.inputImage;
					inputFloat = rhs.inputFloat;
				}
				return *this;
			}

			/// Delete any input data if this struct is set to do so
			void destroy()
			{
				if (deleteInputData)
				{
					OGRE_DELETE inputImage;
					OGRE_FREE(inputFloat, MEMCATEGORY_GEOMETRY);
					inputImage = 0;
					inputFloat = 0;
				}

			}

			~ImportData()
			{
				destroy();
			}

		};

		/// Neighbour index enumeration - indexed anticlockwise from East like angles
		enum NeighbourIndex
		{
			NEIGHBOUR_EAST = 0, 
			NEIGHBOUR_NORTHEAST = 1, 
			NEIGHBOUR_NORTH = 2, 
			NEIGHBOUR_NORTHWEST = 3, 
			NEIGHBOUR_WEST = 4, 
			NEIGHBOUR_SOUTHWEST = 5, 
			NEIGHBOUR_SOUTH = 6, 
			NEIGHBOUR_SOUTHEAST = 7,

			NEIGHBOUR_COUNT = 8
		};

		SceneManager* getSceneManager() const { return mSceneMgr; }

		/// Enumeration of relative spaces that you might want to use to address the terrain
		enum Space
		{
			/// Simple global world space, axes and positions are all in world space
			WORLD_SPACE = 0, 
			/// As world space, but positions are relative to the terrain world position
			LOCAL_SPACE = 1, 
			/** x & y are parametric values on the terrain from 0 to 1, with the
			origin at the bottom left. z is the world space height at that point.
			*/
			TERRAIN_SPACE = 2,
			/** x & y are integer points on the terrain from 0 to size-1, with the
			origin at the bottom left. z is the world space height at that point.
			*/
			POINT_SPACE = 3
		};

		/** Interface used to by the Terrain instance to allocate GPU buffers.
		@remarks This class exists to make it easier to re-use buffers between
			multiple instances of terrain.
		*/
		class _OgreTerrainExport GpuBufferAllocator : public TerrainAlloc
		{
		public:
			GpuBufferAllocator() {}
			virtual ~GpuBufferAllocator() {}

			/** Allocate (or reuse) vertex buffers for a terrain LOD. 
			@param numVertices The total number of vertices
			@param destPos Pointer to a vertex buffer for positions, to be bound
			@param destDelta Pointer to a vertex buffer for deltas, to be bound
			*/
			virtual void allocateVertexBuffers(Terrain* forTerrain, size_t numVertices, HardwareVertexBufferSharedPtr& destPos, HardwareVertexBufferSharedPtr& destDelta) = 0;
			/** Free (or return to the pool) vertex buffers for terrain. 
			*/
			virtual void freeVertexBuffers(const HardwareVertexBufferSharedPtr& posbuf, const HardwareVertexBufferSharedPtr& deltabuf) = 0;

			/** Get a shared index buffer for a given number of settings.
			@remarks
				Since all index structures are the same at the same LOD level and
				relative position, we can share index buffers. Therefore the 
				buffer returned from this method does not need to be 'freed' like
				the vertex buffers since it is never owned.
			@param batchSize The batch size along one edge
			@param vdatasize The size of the referenced vertex data along one edge
			@param vertexIncrement The number of vertices to increment for each new indexed row / column
			@param xoffset The x offset from the start of vdatasize, at that resolution
			@param yoffset The y offset from the start of vdatasize, at that resolution
			@param numSkirtRowsCols Number of rows and columns of skirts
			@param skirtRowColSkip The number of rows / cols to skip in between skirts
			*/
			virtual HardwareIndexBufferSharedPtr getSharedIndexBuffer(uint16 batchSize, 
				uint16 vdatasize, size_t vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols, 
				uint16 skirtRowColSkip) = 0;

			/// Free any buffers we're holding
			virtual void freeAllBuffers() = 0;

		};
		/// Standard implementation of a buffer allocator which re-uses buffers
		class _OgreTerrainExport DefaultGpuBufferAllocator : public GpuBufferAllocator
		{
		public:
			DefaultGpuBufferAllocator();
			~DefaultGpuBufferAllocator();
			void allocateVertexBuffers(Terrain* forTerrain, size_t numVertices, HardwareVertexBufferSharedPtr& destPos, HardwareVertexBufferSharedPtr& destDelta);
			void freeVertexBuffers(const HardwareVertexBufferSharedPtr& posbuf, const HardwareVertexBufferSharedPtr& deltabuf);
			HardwareIndexBufferSharedPtr getSharedIndexBuffer(uint16 batchSize, 
				uint16 vdatasize, size_t vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols, 
				uint16 skirtRowColSkip);
			void freeAllBuffers();

			/** 'Warm start' the allocator based on needing x instances of 
				terrain with the given configuration.
			*/
			void warmStart(size_t numInstances, uint16 terrainSize, uint16 maxBatchSize, 
				uint16 minBatchSize);

		protected:
			typedef list<HardwareVertexBufferSharedPtr>::type VBufList;
			VBufList mFreePosBufList;
			VBufList mFreeDeltaBufList;
			typedef map<uint32, HardwareIndexBufferSharedPtr>::type IBufMap;
			IBufMap mSharedIBufMap;

			uint32 hashIndexBuffer(uint16 batchSize, 
				uint16 vdatasize, size_t vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols, 
				uint16 skirtRowColSkip);
			HardwareVertexBufferSharedPtr getVertexBuffer(VBufList& list, size_t vertexSize, size_t numVertices);

		};

		/** Tell this instance to use the given GpuBufferAllocator. 
		@remarks
			May only be called when the terrain is not loaded.
		*/
		void setGpuBufferAllocator(GpuBufferAllocator* alloc);

		/// Get the current buffer allocator
		GpuBufferAllocator* getGpuBufferAllocator();

		/// Utility method to get the number of indexes required to render a given batch
		static size_t _getNumIndexesForBatchSize(uint16 batchSize);
		/** Utility method to populate a (locked) index buffer.
		@param pIndexes Pointer to an index buffer to populate
		@param batchSize The number of vertices down one side of the batch
		@param vdatasize The number of vertices down one side of the vertex data being referenced
		@param vertexIncrement The number of vertices to increment for each new indexed row / column
		@param xoffset The x offset from the start of the vertex data being referenced
		@param yoffset The y offset from the start of the vertex data being referenced
		@param numSkirtRowsCols Number of rows and columns of skirts
		@param skirtRowColSkip The number of rows / cols to skip in between skirts

		*/
		static void _populateIndexBuffer(uint16* pIndexes, uint16 batchSize, 
			uint16 vdatasize, size_t vertexIncrement, uint16 xoffset, uint16 yoffset, uint16 numSkirtRowsCols, 
			uint16 skirtRowColSkip);

		/** Utility method to calculate the skirt index for a given original vertex index. */
		static uint16 _calcSkirtVertexIndex(uint16 mainIndex, uint16 vdatasize, bool isCol, 
			uint16 numSkirtRowsCols, uint16 skirtRowColSkip);

		/** Convert a position from one space to another with respect to this terrain.
		@param inSpace The space that inPos is expressed as
		@param inPos The incoming position
		@param outSpace The space which outPos should be expressed as
		@param outPos The output position to be populated
		*/
		void convertPosition(Space inSpace, const Vector3& inPos, Space outSpace, Vector3& outPos) const;
		/** Convert a position from one space to another with respect to this terrain.
		@param inSpace The space that inPos is expressed as
		@param inPos The incoming position
		@param outSpace The space which outPos should be expressed as
		@returns The output position 
		*/
		Vector3 convertPosition(Space inSpace, const Vector3& inPos, Space outSpace) const;
		/** Convert a direction from one space to another with respect to this terrain.
		@param inSpace The space that inDir is expressed as
		@param inDir The incoming direction
		@param outSpace The space which outDir should be expressed as
		@param outDir The output direction to be populated
		*/
		void convertDirection(Space inSpace, const Vector3& inDir, Space outSpace, Vector3& outDir) const;
		/** Convert a direction from one space to another with respect to this terrain.
		@param inSpace The space that inDir is expressed as
		@param inDir The incoming direction
		@param outSpace The space which outDir should be expressed as
		@returns The output direction 
		*/
		Vector3 convertDirection(Space inSpace, const Vector3& inDir, Space outSpace) const;

		/** Set the resource group to use when loading / saving. 
		@param resGroup Resource group name - you can set this to blank to use
			the default in TerrainGlobalOptions.
		*/
		void setResourceGroup(const String& resGroup) { mResourceGroup = resGroup; }

		/** Get the resource group to use when loading / saving. 
			If this is blank, the default in TerrainGlobalOptions will be used.
		*/
		const String& getResourceGroup() const { return mResourceGroup; }

		/** Get the final resource group to use when loading / saving. 
		*/
		const String& _getDerivedResourceGroup() const;

		/** Save terrain data in native form to a standalone file
		@param filename The name of the file to save to. If this is a filename with
			no path elements, then it is saved in the first writeable location
			available in the resource group you have chosen to use for this
			terrain. If the filename includes path specifiers then it is saved
			directly instead (but note that it may not be reloadable via the
			resource system if the location is not on the path). 
		*/
		void save(const String& filename);
		/** Save terrain data in native form to a serializing stream.
		@remarks
			If you want complete control over where the terrain data goes, use
			this form.
		*/
		void save(StreamSerialiser& stream);

		/** Prepare the terrain from a standalone file.
		@note
		This is safe to do in a background thread as it creates no GPU resources.
		It reads data from a native terrain data chunk. For more advanced uses, 
		such as loading from a shared file, use the StreamSerialiser form.
		*/
		bool prepare(const String& filename);
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

		/** Prepare and load the terrain in one simple call from a standalone file.
		@note
			This method must be called from the primary render thread. To load data
			in a background thread, use the prepare() method.
		*/
		void load(const String& filename);

		/** Prepare and load the terrain in one simple call from a stream.
		@note
		This method must be called from the primary render thread. To load data
		in a background thread, use the prepare() method.
		*/
		void load(StreamSerialiser& stream);

		/** Load the terrain based on the data already populated via prepare methods. 
		@remarks
			This method must be called in the main render thread. 
		*/
		void load();

		/** Return whether the terrain is loaded. 
		@remarks
			Should only be called from the render thread really, since this is
			where the loaded state changes.
		*/
		bool isLoaded() const { return mIsLoaded; }

		/** Returns whether this terrain has been modified since it was first loaded / defined. 
		@remarks
			This flag is reset on save().
		*/
		bool isModified() const { return mModified; }


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
		float* getHeightData() const;

		/** Get a pointer to the height data for a given point. 
		*/
		float* getHeightData(long x, long y) const;

		/** Get the height data for a given terrain point. 
		@param x, y Discrete coordinates in terrain vertices, values from 0 to size-1,
			left/right bottom/top
		*/
		float getHeightAtPoint(long x, long y) const;

		/** Set the height data for a given terrain point. 
		@note this doesn't take effect until you call update()
		@param x, y Discrete coordinates in terrain vertices, values from 0 to size-1,
		left/right bottom/top
		@param h The new height
		*/
		void setHeightAtPoint(long x, long y, float h);

		/** Get the height data for a given terrain position. 
		@param x, y Position in terrain space, values from 0 to 1 left/right bottom/top
		*/
		float getHeightAtTerrainPosition(Real x, Real y);

		/** Get the height data for a given world position (projecting the point
			down on to the terrain). 
		@param x, y,z Position in world space. Positions will be clamped to the edge
			of the terrain
		*/
		float getHeightAtWorldPosition(Real x, Real y, Real z);

		/** Get the height data for a given world position (projecting the point
		down on to the terrain). 
		@param pos Position in world space. Positions will be clamped to the edge
		of the terrain
		*/
		float getHeightAtWorldPosition(const Vector3& pos);

		/** Get a pointer to all the delta data for this terrain.
		@remarks
			The delta data is a measure at a given vertex of by how much vertically
			a vertex will have to move to reach the point at which it will be
			removed in the next lower LOD.
		*/
		const float* getDeltaData();

		/** Get a pointer to the delta data for a given point. 
		*/
		const float* getDeltaData(long x, long y);

		/** Get a Vector3 of the world-space point on the terrain, aligned as per
			options.
		@note This point is relative to Terrain::getPosition
		*/
		void getPoint(long x, long y, Vector3* outpos);

		/** Get a Vector3 of the world-space point on the terrain, aligned as per
		options. Cascades into neighbours if out of bounds.
		@note This point is relative to Terrain::getPosition - neighbours are
			adjusted to be relative to this tile
		*/
		void getPointFromSelfOrNeighbour(long x, long y, Vector3* outpos);

		/** Get a Vector3 of the world-space point on the terrain, supplying the
			height data manually (can be more optimal). 
		@note This point is relative to Terrain::getPosition
		*/
		void getPoint(long x, long y, float height, Vector3* outpos);
		/** Translate a vector from world space to local terrain space based on the alignment options.
		@param inVec The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getTerrainVector(const Vector3& inVec, Vector3* outVec);
		/** Translate a vector from world space to local terrain space based on a specified alignment.
		@param inVec The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getTerrainVectorAlign(const Vector3& inVec, Alignment align, Vector3* outVec);

		/** Translate a vector from world space to local terrain space based on the alignment options.
		@param x, y, z The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getTerrainVector(Real x, Real y, Real z, Vector3* outVec);
		/** Translate a vector from world space to local terrain space based on a specified alignment.
		@param x, y, z The vector in world space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getTerrainVectorAlign(Real x, Real y, Real z, Alignment align, Vector3* outVec);

		/** Translate a vector into world space based on the alignment options.
		@param inVec The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getVector(const Vector3& inVec, Vector3* outVec);
		/** Translate a vector into world space based on a specified alignment.
		@param inVec The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getVectorAlign(const Vector3& inVec, Alignment align, Vector3* outVec);

		/** Translate a vector into world space based on the alignment options.
		@param x, y, z The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getVector(Real x, Real y, Real z, Vector3* outVec);
		/** Translate a vector into world space based on a specified alignment.
		@param x, y, z The vector in basis space, where x/y represents the 
		terrain plane and z represents the up vector
		*/
		void getVectorAlign(Real x, Real y, Real z, Alignment align, Vector3* outVec);


		/** Convert a position from terrain basis space to world space. 
		@param TSpos Terrain space position, where (0,0) is the bottom-left of the
			terrain, and (1,1) is the top-right. The Z coordinate is in absolute
			height units.
		@note This position is relative to Terrain::getPosition
		@param outWSpos World space output position (setup according to current alignment). 
		*/
		void getPosition(const Vector3& TSpos, Vector3* outWSpos);
		/** Convert a position from terrain basis space to world space. 
		@param x,y,z Terrain space position, where (0,0) is the bottom-left of the
		terrain, and (1,1) is the top-right. The Z coordinate is in absolute
		height units.
		@note This position is relative to Terrain::getPosition
		@param outWSpos World space output position (setup according to current alignment). 
		*/
		void getPosition(Real x, Real y, Real z, Vector3* outWSpos);

		/** Convert a position from world space to terrain basis space. 
		@param WSpos World space position (setup according to current alignment). 
		@param outTSpos Terrain space output position, where (0,0) is the bottom-left of the
		terrain, and (1,1) is the top-right. The Z coordinate is in absolute
		height units.
		*/
		void getTerrainPosition(const Vector3& WSpos, Vector3* outTSpos);
		/** Convert a position from world space to terrain basis space. 
		@param x,y,z World space position (setup according to current alignment). 
		@param outTSpos Terrain space output position, where (0,0) is the bottom-left of the
		terrain, and (1,1) is the top-right. The Z coordinate is in absolute
		height units.
		*/
		void getTerrainPosition(Real x, Real y, Real z, Vector3* outTSpos);
		/** Convert a position from terrain basis space to world space based on a specified alignment. 
		@param TSpos Terrain space position, where (0,0) is the bottom-left of the
			terrain, and (1,1) is the top-right. The Z coordinate is in absolute
			height units.
		@param outWSpos World space output position (setup according to alignment). 
		*/
		void getPositionAlign(const Vector3& TSpos, Alignment align, Vector3* outWSpos);
		/** Convert a position from terrain basis space to world space based on a specified alignment. 
		@param x,y,z Terrain space position, where (0,0) is the bottom-left of the
		terrain, and (1,1) is the top-right. The Z coordinate is in absolute
		height units.
		@param outWSpos World space output position (setup according to alignment). 
		*/
		void getPositionAlign(Real x, Real y, Real z, Alignment align, Vector3* outWSpos);

		/** Convert a position from world space to terrain basis space based on a specified alignment. 
		@param WSpos World space position (setup according to alignment). 
		@param outTSpos Terrain space output position, where (0,0) is the bottom-left of the
		terrain, and (1,1) is the top-right. The Z coordinate is in absolute
		height units.
		*/
		void getTerrainPositionAlign(const Vector3& WSpos, Alignment align, Vector3* outTSpos);
		/** Convert a position from world space to terrain basis space based on a specified alignment. 
		@param x,y,z World space position (setup according to alignment). 
		@param outTSpos Terrain space output position, where (0,0) is the bottom-left of the
		terrain, and (1,1) is the top-right. The Z coordinate is in absolute
		height units.
		*/
		void getTerrainPositionAlign(Real x, Real y, Real z, Alignment align, Vector3* outTSpos);


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

		/** Get the number of layers in this terrain. */
		uint8 getLayerCount() const { return static_cast<uint8>(mLayers.size()); }

		/** Get the declaration which describes the layers in this terrain. */
		const TerrainLayerDeclaration& getLayerDeclaration() const { return mLayerDecl; }

		/** Add a new layer to this terrain.
		@param worldSize The size of the texture in this layer in world units. Default
		to zero to use the default
		@param textureNames A list of textures to assign to the samplers in this
			layer. Leave blank to provide these later. 
		*/
		void addLayer(Real worldSize = 0, const StringVector* textureNames = 0);

		/** Remove a layer from the terrain.
		*/
		void removeLayer(uint8 index);

		/** Get the maximum number of layers supported with the current options. 
		@note When you change the options requested, this value can change. 
		*/
		uint8 getMaxLayers() const;

		/** How large an area in world space the texture in a terrain layer covers
		before repeating. 
		@param index The layer index.
		*/
		Real getLayerWorldSize(uint8 index) const;
		/** How large an area in world space the texture in a terrain layer covers
		before repeating. 
		@param index The layer index.
		@param size The world size of the texture before repeating
		*/
		void setLayerWorldSize(uint8 index, Real size);

		/** Get the layer UV multiplier. 
		@remarks
			This is derived from the texture world size. The base UVs in the 
			terrain vary from 0 to 1 and this multiplier is used (in a fixed-function 
			texture coord scaling or a shader parameter) to translate it to the
			final value.
		@param index The layer index.
		*/
		Real getLayerUVMultiplier(uint8 index) const;

		/** Get the name of the texture bound to a given index within a given layer.
		See the LayerDeclaration for a list of sampelrs within a layer.
		@param layerIndex The layer index.
		@param samplerIndex The sampler index within a layer
		*/
		const String& getLayerTextureName(uint8 layerIndex, uint8 samplerIndex) const;
		/** Set the name of the texture bound to a given index within a given layer.
		See the LayerDeclaration for a list of sampelrs within a layer.
		@param index The layer index.
		@param size The world size of the texture before repeating
		@param textureName The name of the texture to use
		*/
		void setLayerTextureName(uint8 layerIndex, uint8 samplerIndex, const String& textureName);

		/** Get the requested size of the blend maps used to blend between layers
			for this terrain. 
			Note that where hardware limits this, the actual blend maps may be lower
			resolution. This option is derived from TerrainGlobalOptions when the
			terrain is created.
		*/
		uint16 getLayerBlendMapSize() const { return mLayerBlendMapSize; }

		/** Get the requested size of lightmap for this terrain. 
		Note that where hardware limits this, the actual lightmap may be lower
		resolution. This option is derived from TerrainGlobalOptions when the
		terrain is created.
		*/
		uint16 getLightmapSize() const { return mLightmapSize; }

		/// Get access to the lightmap, if enabled (as requested by the material generator)
		const TexturePtr& getLightmap() const { return mLightmap; }

		/** Get the requested size of composite map for this terrain. 
		Note that where hardware limits this, the actual texture may be lower
		resolution. This option is derived from TerrainGlobalOptions when the
		terrain is created.
		*/
		uint16 getCompositeMapSize() const { return mCompositeMapSize; }

		/// Get access to the composite map, if enabled (as requested by the material generator)
		const TexturePtr& getCompositeMap() const { return mCompositeMap; }

		/// Get the world position of the terrain centre
		const Vector3& getPosition() const { return mPos; }
		/// Set the position of the terrain centre in world coordinates
		void setPosition(const Vector3& pos);
		/// Get the root scene node for the terrain (internal use only)
		SceneNode* _getRootSceneNode() const;
		/** Mark the entire terrain as dirty. 
		By marking a section of the terrain as dirty, you are stating that you have
		changed the height data within this rectangle. This rectangle will be merged with
		any existing outstanding changes. To finalise the changes, you must 
		call update(), updateGeometry(), or updateDerivedData().
		*/
		void dirty();

		/** Mark a region of the terrain as dirty. 
		By marking a section of the terrain as dirty, you are stating that you have
		changed the height data within this rectangle. This rectangle will be merged with
		any existing outstanding changes. To finalise the changes, you must 
		call update(), updateGeometry(), or updateDerivedData().
		@param rect A rectangle expressed in vertices describing the dirty region;
			left < right, top < bottom, left & top are inclusive, right & bottom exclusive
		*/
		void dirtyRect(const Rect& rect);

		/** Mark a region of the terrain composite map as dirty. 
		@remarks
			You don't usually need to call this directly, it is inferred from 
			changing the other data on the terrain.
		*/
		void _dirtyCompositeMapRect(const Rect& rect);

		/** Trigger the update process for the terrain.
		@remarks
			Updating the terrain will process any dirty sections of the terrain.
			This may affect many things:
			<ol><li>The terrain geometry</li>
			<li>The terrain error metrics which determine LOD transitions</li>
			<li>The terrain normal map, if present</li>
			<li>The terrain lighting map, if present</li>
			<li>The terrain composite map, if present</li>
			</ol>
			If threading is enabled, only item 1 (the geometry) will be updated
			synchronously, ie will be fully up to date when this method returns.
			The other elements are more expensive to compute, and will be queued
			for processing in a background thread, in the order shown above. As these
			updates complete, the effects will be shown.

			You can also separate the timing of updating the geometry, LOD and the lighting
			information if you want, by calling updateGeometry() and
			updateDerivedData() separately.
			@param synchronous If true, all updates will happen immediately and not
			in a separate thread.
		*/
		void update(bool synchronous = false);

		/** Performs an update on the terrain geometry based on the dirty region.
		@remarks
			Terrain geometry will be updated when this method returns.
		*/
		void updateGeometry();

		// Used as a type mask for updateDerivedData
		static const uint8 DERIVED_DATA_DELTAS;
		static const uint8 DERIVED_DATA_NORMALS;
		static const uint8 DERIVED_DATA_LIGHTMAP;
		static const uint8 DERIVED_DATA_ALL;

		/** Updates derived data for the terrain (LOD, lighting) to reflect changed height data, in a separate
		thread if threading is enabled (OGRE_THREAD_SUPPORT). 
		If threading is enabled, on return from this method the derived
		data will not necessarily be updated immediately, the calculation 
		may be done in the background. Only one update will run in the background
		at once. This derived data can typically survive being out of sync for a 
		few frames which is why it is not done synchronously
		@param synchronous If true, the update will happen immediately and not
			in a separate thread.
		@param typeMask Mask indicating the types of data we should generate
		*/
		void updateDerivedData(bool synchronous = false, uint8 typeMask = 0xFF);

		/** Performs an update on the terrain composite map based on its dirty region.
		@remarks
			Rather than calling this directly, call updateDerivedData, which will
			also call it after the other derived data has been updated (there is
			no point updating the composite map until lighting has been updated).
			However the blend maps may call this directly when only the blending 
			information has been updated.
		*/
		void updateCompositeMap();

		/** Performs an update on the terrain composite map based on its dirty region, 
			but only at a maximum frequency. 
		@remarks
		Rather than calling this directly, call updateDerivedData, which will
		also call it after the other derived data has been updated (there is
		no point updating the composite map until lighting has been updated).
		However the blend maps may call this directly when only the blending 
		information has been updated.
		@note
		This method will log the request for an update, but won't do it just yet 
		unless there are no further requests in the next 'delay' seconds. This means
		you can call it all the time but only pick up changes in quiet times.
		*/
		void updateCompositeMapWithDelay(Real delay = 2);


		/** The default size of 'skirts' used to hide terrain cracks
			(default 10, set for new Terrain using TerrainGlobalOptions)
		*/
		Real getSkirtSize() const { return mSkirtSize; }

		/// Get the total number of LOD levels in the terrain
		uint16 getNumLodLevels() const { return mNumLodLevels; }

		/// Get the number of LOD levels in a leaf of the terrain quadtree
		uint16 getNumLodLevelsPerLeaf() const { return mNumLodLevelsPerLeafNode; }

		/** Calculate (or recalculate) the delta values of heights between a vertex
			in its recorded position, and the place it will end up in the LOD
			in which it is removed. 
		@param rect Rectangle describing the area in which heights have altered 
		@returns A Rectangle describing the area which was updated (may be wider
			than the input rectangle)
		*/
		Rect calculateHeightDeltas(const Rect& rect);

		/** Finalise the height deltas. 
		Calculated height deltas are kept in a separate calculation field to make
		them safe to perform in a background thread. This call promotes those
		calculations to the runtime values, and must be called in the main thread.
		@param rect Rectangle describing the area to finalise 
		@param cpuData When updating vertex data, update the CPU copy (background)
		*/
		void finaliseHeightDeltas(const Rect& rect, bool cpuData);

		/** Calculate (or recalculate) the normals on the terrain
		@param rect Rectangle describing the area of heights that were changed
		@param outFinalRect Output rectangle describing the area updated
		@returns Pointer to a PixelBox full of normals (caller responsible for deletion)
		*/
		PixelBox* calculateNormals(const Rect& rect, Rect& outFinalRect);

		/** Finalise the normals. 
		Calculated normals are kept in a separate calculation area to make
		them safe to perform in a background thread. This call promotes those
		calculations to the runtime values, and must be called in the main thread.
		@param rect Rectangle describing the area to finalise 
		@param normalsBox Pointer to a PixelBox full of normals
		*/
		void finaliseNormals(const Rect& rect, PixelBox* normalsBox);

		/** Calculate (or recalculate) the terrain lightmap
		@param rect Rectangle describing the area of heights that were changed
		@param extraTargetRect Rectangle describing a target area of the terrain that
			needs to be calculated additionally (e.g. from a neighbour)
		@param outFinalRect Output rectangle describing the area updated in the lightmap
		@returns Pointer to a PixelBox full of lighting data (caller responsible for deletion)
		*/
		PixelBox* calculateLightmap(const Rect& rect, const Rect& extraTargetRect, Rect& outFinalRect);

		/** Finalise the lightmap. 
		Calculating lightmaps is kept in a separate calculation area to make
		it safe to perform in a background thread. This call promotes those
		calculations to the runtime values, and must be called in the main thread.
		@param rect Rectangle describing the area to finalise 
		@param normalsBox Pointer to a PixelBox full of normals
		*/
		void finaliseLightmap(const Rect& rect, PixelBox* lightmapBox);

		/** Gets the resolution of the entire terrain (down one edge) at a 
			given LOD level. 
		*/
		uint16 getResolutionAtLod(uint16 lodLevel);

		/** Test for intersection of a given ray with the terrain. If the ray hits
		 the terrain, the point of intersection is returned.
		 @param ray The ray to test for intersection
		 @param cascadeToNeighbours Whether the ray will be projected onto neighbours if
			no intersection is found
		 @param distanceLimit The distance from the ray origin at which we will stop looking,
			0 indicates no limit
		 @return A pair which contains whether the ray hit the terrain and, if so, where.
		 @remarks This can be called from any thread as long as no parallel write to
		 the heightmap data occurs.
		 */
		std::pair<bool, Vector3> rayIntersects(const Ray& ray, 
			bool cascadeToNeighbours = false, Real distanceLimit = 0); //const;
		
		/// Get the AABB (local coords) of the entire terrain
		const AxisAlignedBox& getAABB() const;
		/// Get the AABB (world coords) of the entire terrain
		AxisAlignedBox getWorldAABB() const;
		/// Get the minimum height of the terrain
		Real getMinHeight() const;
		/// Get the maximum height of the terrain
		Real getMaxHeight() const;
		/// Get the bounding radius of the entire terrain
		Real getBoundingRadius() const;

		/// Get the material being used for the terrain
		const MaterialPtr& getMaterial() const;
		/// Internal getting of material 
		const MaterialPtr& _getMaterial() const { return mMaterial; }
		/// Get the material being used for the terrain composite map
		const MaterialPtr& getCompositeMapMaterial() const;
		/// Internal getting of material  for the terrain composite map
		const MaterialPtr& _getCompositeMapMaterial() const { return mCompositeMapMaterial; }

		/// Get the name of the material being used for the terrain
		const String& getMaterialName() const { return mMaterialName; }

		/// Overridden from SceneManager::Listener
		void preFindVisibleObjects(SceneManager* source, 
			SceneManager::IlluminationRenderStage irs, Viewport* v);
		/// Overridden from SceneManager::Listener
		void sceneManagerDestroyed(SceneManager* source);

		/// Get the render queue group that this terrain will be rendered into
		uint8 getRenderQueueGroup(void) const { return mRenderQueueGroup; }
		/** Set the render queue group that this terrain will be rendered into.
		@remarks The default is specified in TerrainGlobalOptions
		*/
		void setRenderQueueGroup(uint8 grp) { mRenderQueueGroup = grp; }

		/// Get the visibility flags for this terrain.
		uint32 getVisibilityFlags(void) const { return mVisibilityFlags; }
		/** Set the visibility flags for this terrain.
		@remarks The default is specified in TerrainGlobalOptions
		*/
		void setVisibilityFlags(uint32 flags) { mVisibilityFlags = flags; }

		/// Get the query flags for this terrain.
		uint32 getQueryFlags(void) const { return mQueryFlags; }
		/** Set the query flags for this terrain.
		@remarks The default is specified in TerrainGlobalOptions
		*/
		void setQueryFlags(uint32 flags) { mQueryFlags = flags; }
		
		/** As setQueryFlags, except the flags passed as parameters are appended to the existing flags on this object. */
		void addQueryFlags(uint32 flags) { mQueryFlags |= flags; }
		
		/* As setQueryFlags, except the flags passed as parameters are removed from the existing flags on this object. */
		void removeQueryFlags(uint32 flags) { mQueryFlags &= ~flags; }
		

		/** Retrieve the layer blending map for a given layer, which may
			be used to edit the blending information for that layer.
		@note
			You can only do this after the terrain has been loaded. You may 
			edit the content of the blend layer in another thread, but you
			may only upload it in the main render thread.
		@param layerIndex The layer index, which should be 1 or higher (since 
			the bottom layer has no blending).
		@returns Pointer to the TerrainLayerBlendMap requested. The caller must
			not delete this instance, use freeTemporaryResources if you want
			to save the memory after completing your editing.
		*/
		TerrainLayerBlendMap* getLayerBlendMap(uint8 layerIndex);

		/** Get the index of the blend texture that a given layer uses.
		@param layerIndex The layer index, must be >= 1 and less than the number
			of layers
		@returns The index of the shared blend texture
		*/
		uint8 getBlendTextureIndex(uint8 layerIndex) const;

		/// Get the number of blend textures in use
		uint8 getBlendTextureCount() const;
		/// Get the number of blend textures needed for a given number of layers
		uint8 getBlendTextureCount(uint8 numLayers) const;


		/** Get the name of the packed blend texture at a specific index.
		@param textureIndex This is the blend texture index, not the layer index
			(multiple layers will share a blend texture)
		*/
		const String& getBlendTextureName(uint8 textureIndex) const;

		/** Set whether a global colour map is enabled. 
		@remarks
			A global colour map can add variation to your terrain and reduce the 
			perceived tiling effect you might get in areas of continuous lighting
			and the same texture. 
			The global colour map is only used when the material generator chooses
			to use it.
		@note You must only call this from the main render thread
		@param enabled Whether the global colour map is enabled or not
		@param size The resolution of the colour map. A value of zero means 'no change'
			and the default is set in TerrainGlobalOptions.
		*/
		void setGlobalColourMapEnabled(bool enabled, uint16 size = 0);
		/// Get whether a global colour map is enabled on this terrain
		bool getGlobalColourMapEnabled() const { return mGlobalColourMapEnabled; }
		/// Get the size of the global colour map (if used)
		uint16 getGlobalColourMapSize() const { return mGlobalColourMapSize; }
		/// Get access to the global colour map, if enabled
		const TexturePtr& getGlobalColourMap() const { return mColourMap; }

		/** Widen a rectangular area of terrain to take into account an extrusion vector.
		@param vec A vector in world space
		@param inRect Input rectangle
		@param inRect Output rectangle
		*/
		void widenRectByVector(const Vector3& vec, const Rect& inRect, Rect& outRect);

		/** Widen a rectangular area of terrain to take into account an extrusion vector, 
			but specify the min / max heights to extrude manually.
		@param vec A vector in world space
		@param inRect Input rectangle
		@param minHeight, maxHeight The extents of the height to extrude
		@param inRect Output rectangle
		*/
		void widenRectByVector(const Vector3& vec, const Rect& inRect, 
			Real minHeight, Real maxHeight, Rect& outRect);

		/** Free as many resources as possible for optimal run-time memory use.
		@remarks
			This class keeps some temporary storage around in order to make
			certain actions (such as editing) possible more quickly. Calling this
			method will cause as many of those resources as possible to be
			freed. You might want to do this for example when you are finished
			editing a particular terrain and want to have optimal runtime
			efficiency.
		*/
		void freeTemporaryResources();

		/** Get a blend texture with a given index.
		@param index The blend texture index (note: not layer index; derive
		the texture index from getLayerBlendTextureIndex)
		*/
		const TexturePtr& getLayerBlendTexture(uint8 index);

		/** Get the texture index and colour channel of the blend information for 
			a given layer. 
		@param layerIndex The index of the layer (1 or higher, layer 0 has no blend data)
		@returns A pair in which the first value is the texture index, and the 
			second value is the colour channel (RGBA)
		*/
		std::pair<uint8,uint8> getLayerBlendTextureIndex(uint8 layerIndex);

		/** Request internal implementation options for the terrain material to use, 
			in this case vertex morphing information. 
		The TerrainMaterialGenerator should call this method to specify the 
		options it would like to use when creating a material. Not all the data
		is guaranteed to be up to date on return from this method - for example som
		maps may be generated in the background. However, on return from this method
		all the features that are requested will be referenceable by materials, the
		data may just take a few frames to be fully populated.
		@param morph Whether LOD morphing information is required to be calculated
		*/
		void _setMorphRequired(bool morph) { mLodMorphRequired = morph; }
		/// Get whether LOD morphing is needed
		bool _getMorphRequired() const { return mLodMorphRequired; }

		/** Request internal implementation options for the terrain material to use, 
		in this case a terrain-wide normal map. 
		The TerrainMaterialGenerator should call this method to specify the 
		options it would like to use when creating a material. Not all the data
		is guaranteed to be up to date on return from this method - for example some
		maps may be generated in the background. However, on return from this method
		all the features that are requested will be referenceable by materials, the
		data may just take a few frames to be fully populated.
		@param normalMap Whether a terrain-wide normal map is requested. This is usually
			mutually exclusive with the lightmap option.
		*/
		void _setNormalMapRequired(bool normalMap);

		/** Request internal implementation options for the terrain material to use, 
		in this case a terrain-wide normal map. 
		The TerrainMaterialGenerator should call this method to specify the 
		options it would like to use when creating a material. Not all the data
		is guaranteed to be up to date on return from this method - for example some
		maps may be generated in the background. However, on return from this method
		all the features that are requested will be referenceable by materials, the
		data may just take a few frames to be fully populated.
		@param lightMap Whether a terrain-wide lightmap including precalculated 
			lighting is required (light direction in TerrainGlobalOptions)
		@param shadowsOnly If true, the lightmap contains only shadows, 
			no directional lighting intensity
		*/
		void _setLightMapRequired(bool lightMap, bool shadowsOnly = false);

		/** Request internal implementation options for the terrain material to use, 
		in this case a terrain-wide composite map. 
		The TerrainMaterialGenerator should call this method to specify the 
		options it would like to use when creating a material. Not all the data
		is guaranteed to be up to date on return from this method - for example some
		maps may be generated in the background. However, on return from this method
		all the features that are requested will be referenceable by materials, the
		data may just take a few frames to be fully populated.
		@param compositeMap Whether a terrain-wide composite map is needed. A composite
		map is a texture with all of the blending and lighting baked in, such that
		at distance this texture can be used as an approximation of the multi-layer
		blended material. It is actually up to the material generator to render this
		composite map, because obviously precisely what it looks like depends on what
		the main material looks like. For this reason, the composite map is one piece
		of derived terrain data that is always calculated in the render thread, and
		usually on the GPU. It is expected that if this option is requested, 
		the material generator will use it to construct distant LOD techniques.
		*/
		void _setCompositeMapRequired(bool compositeMap);

		/// WorkQueue::RequestHandler override
		bool canHandleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
		/// WorkQueue::RequestHandler override
		WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
		/// WorkQueue::ResponseHandler override
		bool canHandleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);
		/// WorkQueue::ResponseHandler override
		void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);

		static const uint16 WORKQUEUE_DERIVED_DATA_REQUEST;


		/// Utility method, get the first LOD Level at which this vertex is no longer included
		uint16 getLODLevelWhenVertexEliminated(long x, long y);
		/// Utility method, get the first LOD Level at which this vertex is no longer included
		uint16 getLODLevelWhenVertexEliminated(long rowOrColulmn);


		/// Get the top level of the quad tree which is used to divide up the terrain
		TerrainQuadTreeNode* getQuadTree() { return mQuadTree; }

		/// Get the (global) normal map texture
		TexturePtr getTerrainNormalMap() const { return mTerrainNormalMap; }

		/** Retrieve the terrain's neighbour, or null if not present.
		@remarks
			Terrains only know about their neighbours if they are notified via
			setNeighbour. This information is not saved with the terrain since every
			tile must be able to be independent.
		@param index The index of the neighbour
		*/
		Terrain* getNeighbour(NeighbourIndex index);

		/** Set a terrain's neighbour, or null to detach one. 
		@remarks
			This information is not saved with the terrain since every
			tile must be able to be independent. However if modifications are
			made to a tile which can affect its neighbours, while connected the
			changes will be propagated. 
		@param index The index of the neighbour
		@param neighbour The terrain instance to become the neighbour, or null to reset.
		@param recalculate If true, this terrain instance will recalculate elements
			that could be affected by the connection of this tile (e.g. matching 
			heights, calcaulting normals, calculating shadows crossing the boundary). 
			If false, this terrain's state is assumed to be up to date already 
			(e.g. was calculated with this tile present before and the state saved). 
		@param notifyOther Whether the neighbour should also be notified (recommended
			to leave this at the default so relationships are up to date before
			background updates are triggered)
		*/
		void setNeighbour(NeighbourIndex index, Terrain* neighbour, bool recalculate = false, bool notifyOther = true);

		/** Get the opposite neighbour relationship (useful for finding the 
			neighbour index from the perspective of the tile the other side of the
			boundary).
		*/
		static NeighbourIndex getOppositeNeighbour(NeighbourIndex index);

		/** Get the neighbour enum for a given offset in a grid (signed).
		*/
		static NeighbourIndex getNeighbourIndex(long offsetx, long offsety);

		/** Tell this instance to notify all neighbours that will be affected
			by a height change that has taken place. 
		@remarks
			This method will determine which neighbours need notification and call
			their neighbourModified method. It is called automatically by 
			updateGeometry().
		*/
		void notifyNeighbours();

		/** Notify that a neighbour has just finished updating and that this
			change affects this tile. 
		@param index The neighbour index (from this tile's perspective)
		@param edgerect The area at the edge of this tile that needs height / normal
			recalculation (may be null)
		@param shadowrect The area on this tile where shadows need recalculating (may be null)
		*/
		void neighbourModified(NeighbourIndex index, const Rect& edgerect, const Rect& shadowrect);

		/** Utility method to pick a neighbour based on a ray. 
		@param ray The ray in world space
		@param distanceLimit Limit beyond which we want to ignore neighbours (0 for infinite)
		@returns The first neighbour along this ray, or null
		*/
		Terrain* raySelectNeighbour(const Ray& ray, Real distanceLimit = 0);

		/** Dump textures to files.
		@remarks
			This is a debugging method.
		*/
		void _dumpTextures(const String& prefix, const String& suffix);

		/** Query whether a derived data update is in progress or not. */
		bool isDerivedDataUpdateInProgress() const { return mDerivedDataUpdateInProgress; }


		/// Utility method to convert axes from world space to terrain space (xy terrain, z up)
		static void convertWorldToTerrainAxes(Alignment align, const Vector3& worldVec, Vector3* terrainVec);
		/// Utility method to convert axes from terrain space (xy terrain, z up) tp world space
		static void convertTerrainToWorldAxes(Alignment align, const Vector3& terrainVec, Vector3* worldVec);

		/// Utility method to write a layer declaration to a stream
		static void writeLayerDeclaration(const TerrainLayerDeclaration& decl, StreamSerialiser& ser);
		/// Utility method to read a layer declaration from a stream
		static bool readLayerDeclaration(StreamSerialiser& ser, TerrainLayerDeclaration& targetdecl);
		/// Utility method to write a layer instance list to a stream
		static void writeLayerInstanceList(const Terrain::LayerInstanceList& lst, StreamSerialiser& ser);
		/// Utility method to read a layer instance list from a stream
		static bool readLayerInstanceList(StreamSerialiser& ser, size_t numSamplers, Terrain::LayerInstanceList& targetlst);
	protected:

		void freeCPUResources();
		void freeGPUResources();
		void determineLodLevels();
		void distributeVertexData();
		void updateBaseScale();
		void createGPUBlendTextures();
		void createLayerBlendMaps();
		void createOrDestroyGPUNormalMap();
		void createOrDestroyGPUColourMap();
		void createOrDestroyGPULightmap();
		void createOrDestroyGPUCompositeMap();
		void waitForDerivedProcesses();
		void convertSpace(Space inSpace, const Vector3& inVec, Space outSpace, Vector3& outVec, bool translation) const;
		Vector3 convertWorldToTerrainAxes(const Vector3& inVec) const;
		Vector3 convertTerrainToWorldAxes(const Vector3& inVec) const;
		/** Get a Vector3 of the world-space point on the terrain, aligned Y-up always.
		@note This point is relative to Terrain::getPosition
		*/
		void getPointAlign(long x, long y, Alignment align, Vector3* outpos);
		/** Get a Vector3 of the world-space point on the terrain, supplying the
		height data manually (can be more optimal). 
		@note This point is relative to Terrain::getPosition
		*/
		void getPointAlign(long x, long y, float height, Alignment align, Vector3* outpos);
		void calculateCurrentLod(Viewport* vp);
		/// Test a single quad of the terrain for ray intersection.
		std::pair<bool, Vector3> checkQuadIntersection(int x, int y, const Ray& ray); //const;

		void copyGlobalOptions();
		void checkLayers(bool includeGPUResources);
		void checkDeclaration();
		void deriveUVMultipliers();
		PixelFormat getBlendTextureFormat(uint8 textureIndex, uint8 numLayers);

		void updateDerivedDataImpl(const Rect& rect, const Rect& lightmapExtraRect, bool synchronous, uint8 typeMask);

		void getEdgeRect(NeighbourIndex index, long range, Rect* outRect);
		// get the equivalent of the passed in edge rectangle in neighbour
		void getNeighbourEdgeRect(NeighbourIndex index, const Rect& inRect, Rect* outRect);
		// get the equivalent of the passed in edge point in neighbour
		void getNeighbourPoint(NeighbourIndex index, long x, long y, long *outx, long *outy);
		// overflow a point into a neighbour index and point
		void getNeighbourPointOverflow(long x, long y, NeighbourIndex *outindex, long *outx, long *outy);

		

		uint16 mWorkQueueChannel;
		SceneManager* mSceneMgr;
		SceneNode* mRootNode;
		String mResourceGroup;
		bool mIsLoaded;
		bool mModified;
		
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
		uint16 mNumLodLevelsPerLeafNode;
		uint16 mTreeDepth;
		/// Base position in world space, relative to mPos
		Real mBase;
		/// Relationship between one point on the terrain and world size
		Real mScale;
		TerrainLayerDeclaration mLayerDecl;
		LayerInstanceList mLayers;
		RealVector mLayerUVMultiplier;

		Real mSkirtSize;
		uint8 mRenderQueueGroup;
		uint32 mVisibilityFlags;
		uint32 mQueryFlags;

		Rect mDirtyGeometryRect;
		Rect mDirtyDerivedDataRect;
		Rect mDirtyGeometryRectForNeighbours;
		Rect mDirtyLightmapFromNeighboursRect;
		bool mDerivedDataUpdateInProgress;
		uint8 mDerivedUpdatePendingMask; // if another update is requested while one is already running

		/// A data holder for communicating with the background derived data update
		struct DerivedDataRequest
		{
			Terrain* terrain;
			// types requested
			uint8 typeMask;
			Rect dirtyRect;
			Rect lightmapExtraDirtyRect;
			_OgreTerrainExport friend std::ostream& operator<<(std::ostream& o, const DerivedDataRequest& r)
			{ return o; }		
		};

		/// A data holder for communicating with the background derived data update
		struct DerivedDataResponse
		{
			Terrain* terrain;
			// remaining types not yet processed
			uint8 remainingTypeMask;
			// The area of deltas that was updated
			Rect deltaUpdateRect;
			// the area of normals that was updated
			Rect normalUpdateRect;
			// the area of lightmap that was updated
			Rect lightmapUpdateRect;
			// all CPU-side data, independent of textures; to be blitted in main thread
			PixelBox* normalMapBox;
			PixelBox* lightMapBox;
			_OgreTerrainExport friend std::ostream& operator<<(std::ostream& o, const DerivedDataResponse& r)
			{ return o; }		
		};

		String mMaterialName;
		mutable MaterialPtr mMaterial;
		mutable TerrainMaterialGeneratorPtr mMaterialGenerator;
		mutable unsigned long long int mMaterialGenerationCount;
		mutable bool mMaterialDirty;
		mutable bool mMaterialParamsDirty;

		uint16 mLayerBlendMapSize;
		uint16 mLayerBlendMapSizeActual;
		typedef vector<uint8*>::type BytePointerList;
		/// Staging post for blend map data
		BytePointerList mCpuBlendMapStorage;
		typedef vector<TexturePtr>::type TexturePtrList;
		TexturePtrList mBlendTextureList;
		TerrainLayerBlendMapList mLayerBlendMapList;

		uint16 mGlobalColourMapSize;
		bool mGlobalColourMapEnabled;
		TexturePtr mColourMap;
		uint8* mCpuColourMapStorage;

		uint16 mLightmapSize;
		uint16 mLightmapSizeActual;
		TexturePtr mLightmap;
		uint8* mCpuLightmapStorage;

		uint16 mCompositeMapSize;
		uint16 mCompositeMapSizeActual;
		TexturePtr mCompositeMap;
		uint8* mCpuCompositeMapStorage;
		Rect mCompositeMapDirtyRect;
		unsigned long mCompositeMapUpdateCountdown;
		unsigned long mLastMillis;
		/// true if the updates included lightmap changes (widen)
		bool mCompositeMapDirtyRectLightmapUpdate;
		mutable MaterialPtr mCompositeMapMaterial;


		static NameGenerator msBlendTextureGenerator;
		static NameGenerator msNormalMapNameGenerator;
		static NameGenerator msLightmapNameGenerator;
		static NameGenerator msCompositeMapNameGenerator;

		bool mLodMorphRequired;
		bool mNormalMapRequired;
		bool mLightMapRequired;
		bool mLightMapShadowsOnly;
		bool mCompositeMapRequired;
		/// Texture storing normals for the whole terrrain
		TexturePtr mTerrainNormalMap;

		/// Pending data 
		PixelBox* mCpuTerrainNormalMap;

		const Camera* mLastLODCamera;
		unsigned long mLastLODFrame;
		int mLastViewportHeight;

		Terrain* mNeighbours[NEIGHBOUR_COUNT];

		GpuBufferAllocator* mCustomGpuBufferAllocator;
		DefaultGpuBufferAllocator mDefaultGpuBufferAllocator;

		size_t getPositionBufVertexSize() const;
		size_t getDeltaBufVertexSize() const;

	};


	/** Options class which just stores default options for the terrain.
	@remarks
		None of these options are stored with the terrain when saved. They are
		options that you can use to modify the behaviour of the terrain when it
		is loaded or created. 
	*/
	class _OgreTerrainExport TerrainGlobalOptions
	{
	protected:
		// no instantiation
		TerrainGlobalOptions() {}

		static Real msSkirtSize;
		static Vector3 msLightMapDir;
		static bool msCastsShadows;
		static Real msMaxPixelError;
		static uint8 msRenderQueueGroup;
		static uint32 msVisibilityFlags;
		static uint32 msQueryFlags;
		static bool msUseRayBoxDistanceCalculation;
		static TerrainMaterialGeneratorPtr msDefaultMaterialGenerator;
		static uint16 msLayerBlendMapSize;
		static Real msDefaultLayerTextureWorldSize;
		static uint16 msDefaultGlobalColourMapSize;
		static uint16 msLightmapSize;
		static uint16 msCompositeMapSize;
		static ColourValue msCompositeMapAmbient;
		static ColourValue msCompositeMapDiffuse;
		static Real msCompositeMapDistance;
		static String msResourceGroup;

	public:


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
		/// Get the shadow map light direction to use (world space)
		static const Vector3& getLightMapDirection() { return msLightMapDir; }
		/** Set the shadow map light direction to use (world space). */
		static void setLightMapDirection(const Vector3& v) { msLightMapDir = v; }
		/// Get the composite map ambient light to use 
		static const ColourValue& getCompositeMapAmbient() { return msCompositeMapAmbient; }
		/// Set the composite map ambient light to use 
		static void setCompositeMapAmbient(const ColourValue& c) { msCompositeMapAmbient = c; }
		/// Get the composite map iffuse light to use 
		static const ColourValue& getCompositeMapDiffuse() { return msCompositeMapDiffuse; }
		/// Set the composite map diffuse light to use 
		static void setCompositeMapDiffuse(const ColourValue& c) { msCompositeMapDiffuse = c; }
		/// Get the distance at which to start using a composite map if present
		static Real getCompositeMapDistance() { return msCompositeMapDistance; }
		/// Set the distance at which to start using a composite map if present
		static void setCompositeMapDistance(Real c) { msCompositeMapDistance = c; }


		/** Whether the terrain will be able to cast shadows (texture shadows
		only are supported, and you must be using depth shadow maps).
		*/
		static bool getCastsDynamicShadows() { return msCastsShadows; }

		/** Whether the terrain will be able to cast shadows (texture shadows
		only are supported, and you must be using depth shadow maps).
		This value can be set dynamically, and affects all existing terrains.
		It defaults to false. 
		*/
		static void setCastsDynamicShadows(bool s) { msCastsShadows = s; }

		/** Get the maximum screen pixel error that should be allowed when rendering. */
		static Real getMaxPixelError() { return msMaxPixelError; }

		/** Set the maximum screen pixel error that should  be allowed when rendering. 
		@note
			This value can be varied dynamically and affects all existing terrains.
			It will be weighted by the LOD bias on viewports. 
		*/
		static void setMaxPixelError(Real pixerr) { msMaxPixelError = pixerr; }

		/// Get the render queue group that this terrain will be rendered into
		static uint8 getRenderQueueGroup(void) { return msRenderQueueGroup; }
		/** Set the render queue group that terrains will be rendered into.
		@remarks This applies to newly created terrains, after which they will
			maintain their own queue group settings
		*/
		static void setRenderQueueGroup(uint8 grp) { msRenderQueueGroup = grp; }

		/// Get the visbility flags that terrains will be rendered with
		static uint32 getVisibilityFlags(void) { return msVisibilityFlags; }
		/** Set the visbility flags that terrains will be rendered with
		@remarks This applies to newly created terrains, after which they will
		maintain their own settings
		*/
		static void setVisibilityFlags(uint32 flags) { msVisibilityFlags = flags; }

		/** Set the default query flags for terrains.
		@remarks This applies to newly created terrains, after which they will
		maintain their own settings
		*/
		static void  setQueryFlags(uint32 flags) { msQueryFlags = flags; }
		/** Get the default query flags for terrains.
		*/
		static uint32 getQueryFlags(void) { return msQueryFlags; }

		/** As setQueryFlags, except the flags passed as parameters are appended to the existing flags on this object. */
		static void addQueryFlags(uint32 flags) { msQueryFlags |= flags; }

		/* As setQueryFlags, except the flags passed as parameters are removed from the existing flags on this object. */
		static void removeQueryFlags(uint32 flags) { msQueryFlags &= ~flags; }

		/** Returns whether or not to use an accurate calculation of camera distance
			from a terrain tile (ray / AABB intersection) or whether to use the
			simpler distance from the tile centre. 
		*/
		static bool getUseRayBoxDistanceCalculation() { return msUseRayBoxDistanceCalculation; }

		/** Sets whether to use an accurate ray / box intersection to determine
			distance from a terrain tile, or whether to use the simple distance
			from the tile centre.
			Using ray/box intersection will result in higher detail terrain because 
			the LOD calculation is more conservative, assuming the 'worst case scenario' 
			of a large height difference at the edge of a tile. This is guaranteed to give you at least
			the max pixel error or better, but will often give you more detail than
			you need. Not using the ray/box method is cheaper but will only use
			the max pixel error as a guide, the actual error will vary above and
			below that. The default is not to use the ray/box approach.
		*/
		static void setUseRayBoxDistanceCalculation(bool rb) { msUseRayBoxDistanceCalculation = rb; }

		/** Get the default material generator.
		*/
		static TerrainMaterialGeneratorPtr getDefaultMaterialGenerator();

		/** Set the default material generator.
		*/
		static void setDefaultMaterialGenerator(TerrainMaterialGeneratorPtr gen);

		/** Get the default size of the blend maps for a new terrain. 
		*/
		static uint16 getLayerBlendMapSize() { return msLayerBlendMapSize; }

		/** Sets the default size of blend maps for a new terrain.
		This is the resolution of each blending layer for a new terrain. 
		Once created, this information will be stored with the terrain. 
		*/
		static void setLayerBlendMapSize(uint16 sz) { msLayerBlendMapSize = sz;}

		/** Get the default world size for a layer 'splat' texture to cover. 
		*/
		static Real getDefaultLayerTextureWorldSize() { return msDefaultLayerTextureWorldSize; }

		/** Set the default world size for a layer 'splat' texture to cover. 
		*/
		static void setDefaultLayerTextureWorldSize(Real sz) { msDefaultLayerTextureWorldSize = sz; }

		/** Get the default size of the terrain global colour map for a new terrain. 
		*/
		static uint16 getDefaultGlobalColourMapSize() { return msDefaultGlobalColourMapSize; }

		/** Set the default size of the terrain global colour map for a new terrain. 
		Once created, this information will be stored with the terrain. 
		*/
		static void setDefaultGlobalColourMapSize(uint16 sz) { msDefaultGlobalColourMapSize = sz;}


		/** Get the default size of the lightmaps for a new terrain. 
		*/
		static uint16 getLightMapSize() { return msLightmapSize; }

		/** Sets the default size of lightmaps for a new terrain.
		*/
		static void setLightMapSize(uint16 sz) { msLightmapSize = sz;}

		/** Get the default size of the composite maps for a new terrain. 
		*/
		static uint16 getCompositeMapSize() { return msCompositeMapSize; }

		/** Sets the default size of composite maps for a new terrain.
		*/
		static void setCompositeMapSize(uint16 sz) { msCompositeMapSize = sz;}

		/** Set the default resource group to use to load / save terrains.
		*/
		static void setDefaultResourceGroup(const String& grp) { msResourceGroup = grp; }

		/** Get the default resource group to use to load / save terrains.
		*/
		static const String& getDefaultResourceGroup() { return msResourceGroup; }

	};


	/** @} */
	/** @} */
}




#endif 
