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
#include "OgreAxisAlignedBox.h"
#include "OgreSceneManager.h"
#include "OgreTerrainMaterialGenerator.h"
#include "OgreTerrainLayerBlendMap.h"

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
		<td><b>TerrainLayerSampler Count</b></td>
		<td><b>uint8</b></td>
		<td><b>Number of samplers in this declaration</b></td>
	</tr>
	<tr>
		<td><b>TerrainLayerSampler List</b></td>
		<td><b>TerrainLayerSampler*</b></td>
		<td><b>List of TerrainLayerSampler structures</b></td>
	</tr>
	<tr>
		<td><b>Sampler Element Count</b></td>
		<td><b>uint8</b></td>
		<td><b>Number of sampler elements in this declaration</b></td>
	</tr>
	<tr>
		<td><b>TerrainLayerSamplerElement List</b></td>
		<td><b>TerrainLayerSamplerElement*</b></td>
		<td><b>List of TerrainLayerSamplerElement structures</b></td>
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
		<td><b>Alias</b></td>
		<td><b>String</b></td>
		<td><b>Alias name of this sampler</b></td>
	</tr>
	<tr>
		<td><b>Format</b></td>
		<td><b>uint8</b></td>
		<td><b>Desired pixel format</b></td>
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
		<td><b>Source</b></td>
		<td><b>uint8</b></td>
		<td><b>Sampler source index</b></td>
	</tr>
	<tr>
		<td><b>Semantic</b></td>
		<td><b>uint8</b></td>
		<td><b>Semantic interpretation of this element</b></td>
	</tr>
	<tr>
		<td><b>Element start</b></td>
		<td><b>uint8</b></td>
		<td><b>Start of this element in the sampler</b></td>
	</tr>
	<tr>
		<td><b>Element count</b></td>
		<td><b>uint8</b></td>
		<td><b>Number of elements in the sampler used by this entry</b></td>
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
		<td><b>World size</b></td>
		<td><b>Real</b></td>
		<td><b>The world size of this layer (determines UV scaling)</b></td>
	</tr>
	<tr>
		<td><b>Texture list</b></td>
		<td><b>String*</b></td>
		<td><b>List of texture names corresponding to the number of samplers in the layer declaration</b></td>
	</tr>
	</table>
	*/
	class _OgreTerrainExport Terrain : public SceneManager::Listener, public TerrainAlloc
	{
	public:
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

		/** Get the height data for a given point. 
		*/
		float getHeight(long x, long y);

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

		/** Get a Vector3 of the world-space point on the terrain, supplying the
			height data manually (can be more optimal). 
		@note This point is relative to Terrain::getPosition
		*/
		void getPoint(long x, long y, float height, Vector3* outpos);

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
		Real getLayerUVMultipler(uint8 index) const;

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

		/// Get the world position of the terrain centre
		const Vector3& getPosition() const { return mPos; }
		/// Set the position of the terrain centre in world coordinates
		void setPosition(const Vector3& pos);
		/// Get the root scene node for the terrain (internal use only)
		SceneNode* _getRootSceneNode() const;
		/** Mark the entire terrain as dirty, so that the geometry is
			updated the next time it is accessed. 
		*/
		void dirty();

		/** Mark a region of the terrain as dirty, so that the geometry is 
			updated the next time it is accessed. 
		@param rect A rectangle expressed in vertices describing the dirty region;
			left < right, top < bottom, left & top are inclusive, right & bottom exclusive
		*/
		void dirtyRect(const Rect& rect);


		/** Whether to morph between LODs using a vertex program. 
			(default true, set for new Terrain using TerrainGlobalOptions)
		*/
		bool getUseLodMorph() const { return mUseLodMorph; }
		/** The default size of 'skirts' used to hide terrain cracks
			(default 10, set for new Terrain using TerrainGlobalOptions)
		*/
		Real getSkirtSize() const { return mSkirtSize; }
		/** Whether to include normals in the terrain geometry to support basic dynamic
		lighting. 
		@note Including normals in the terrain geometry is usually not recommended, 
		because as LOD decreases, the vertex lighting becomes approximated and
		will start to look wrong. Instead, it is better to generate a texture which
		encodes the normals as an object-space normal map which keeps lighting
		fidelity constant at all LODs, but does require shaders to use. Even if you're
		targetting the fixed-function pipeline you're better not to use vertex normals
		and instead use baked lightmaps.
		(default false, set for new Terrain using TerrainGlobalOptions)
		*/
		bool getGenerateVertexNormals() const { return mGenerateVertexNormals; }

		/** Whether to generate a texture containing the normals of the terrain. 
		(default true, set for new Terrain using TerrainGlobalOptions)
		*/
		bool getGenerateNormalMap() const { return mGenerateNormalMap; }
		/** Whether to generate a texture containing the shadows of the terrain at
			a particular time of day.
		(default false, set for new Terrain using TerrainGlobalOptions)
		*/
		bool getGenerateShadowMap() const { return mGenerateShadowMap; }

		/** Whether to generate a horizon map containing the angles of the horizon
			along a given linear sun track, which allows real-time shadows to be
			generated for the terrain very cheaply without using shadow textures.
			(default false, set for new Terrain using TerrainGlobalOptions)		
		*/
		bool getGenerateHorizonMap() const { return mGenerateHorizonMap; }

		/// Get the total number of LOD levels in the terrain
		uint16 getNumLodLevels() const { return mNumLodLevels; }

		/// Get the number of LOD levels in a leaf of the terrain quadtree
		uint16 getNumLodLevelsPerLeaf() const { return mNumLodLevelsPerLeafNode; }

		/** Calculate (or recalculate) the delta values of heights between a vertex
			in its recorded position, and the place it will end up in the LOD
			in which it is removed. 
		@param rect Rectangle describing the area to calculate (left <= right, bottom <= top)
			
		*/
		void calculateHeightDeltas(const Rect& rect);

		/** Gets the resolution of the entire terrain (down one edge) at a 
			given LOD level. 
		*/
		uint16 getResolutionAtLod(uint16 lodLevel);

		/** Test for intersection of a given ray with the terrain. If the ray hits
		 the terrain, the point of intersection is returned.
		 @param ray The ray to test for intersection
		 @return A pair which contains whether the ray hit the terrain and, if so, where.
		 @remarks This can be called from any thread as long as no parallel write to
		 the heightmap data occurs.
		 */
		std::pair<bool, Vector3> rayIntersects(const Ray& ray); //const;
		
		/// Get the AABB (local coords) of the entire terrain
		const AxisAlignedBox& getAABB() const;
		/// Get the bounding radius of the entire terrain
		Real getBoundingRadius() const;

		/// Get the material being used for the terrain
		const MaterialPtr& getMaterial() const;

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


		/** Retrieve the layer blending map for a given layer, which may
			be used to edit the blending information for that layer.
		@param layerIndex The layer index, which should be 1 or higher (since 
			the bottom layer has no blending).
		@returns Pointer to the TerrainLayerBlendMap requested. The caller must
			not delete this instance, use freeTemporaryResources if you want
			to save the memory after completing your editing.
		*/
		TerrainLayerBlendMap* getLayerBlendMap(uint8 layerIndex);

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


	protected:

		void freeCPUResources();
		void freeGPUResources();
		void determineLodLevels();
		void distributeVertexData();
		void updateBaseScale();
		void createGPUBlendTextures();
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
		std::pair<bool, Vector3> checkQuadIntersection(int x, int z, const Ray& ray); //const;

		void copyGlobalOptions();
		void checkLayers();
		void deriveUVMultipliers();
		uint8 getBlendTextureCount(uint8 numLayers);
		PixelFormat getBlendTextureFormat(uint8 textureIndex, uint8 numLayers);


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
		uint16 mNumLodLevelsPerLeafNode;
		uint16 mTreeDepth;
		Real mBase;
		Real mScale;
		TerrainLayerDeclaration mLayerDecl;
		LayerInstanceList mLayers;
		RealVector mLayerUVMultiplier;


		bool mUseLodMorph;
		Real mSkirtSize;
		bool mGenerateVertexNormals;
		bool mGenerateNormalMap;
		bool mGenerateShadowMap;
		bool mGenerateHorizonMap;
		uint8 mRenderQueueGroup;

		mutable MaterialPtr mMaterial;
		mutable TerrainMaterialGenerator* mMaterialGenerator;
		mutable unsigned long long int mMaterialGenerationCount;

		uint16 mLayerBlendMapSize;
		uint16 mLayerBlendMapSizeActual;
		typedef vector<uint8*>::type BytePointerList;
		/// Staging post for blend map data
		BytePointerList mCpuBlendMapStorage;
		typedef vector<TexturePtr>::type TexturePtrList;
		TexturePtrList mBlendTextureList;
		TerrainLayerBlendMapList mLayerBlendMapList;

		static NameGenerator msBlendTextureGenerator;


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

		static bool msUseLodMorph;
		static Real msSkirtSize;
		static bool msGenerateVertexNormals;
		static bool msGenerateNormalMap;
		static bool msGenerateShadowMap;
		static Vector3 msShadowMapDir;
		static bool msGenerateHorizonMap;
		static Radian msHorizonMapAzimuth;
		static Radian msHorizonMapZenith;
		static bool msCastsShadows;
		static Real msMaxPixelError;
		static uint8 msRenderQueueGroup;
		static bool msUseRayBoxDistanceCalculation;
		static TerrainMaterialGeneratorList msMatGeneratorList;
		static uint16 msLayerBlendMapSize;
	public:


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
		/** Whether to include normals in the terrain geometry to support basic dynamic
		lighting. 
		@note Including normals in the terrain geometry is usually not recommended, 
		because as LOD decreases, the vertex lighting becomes approximated and
		will start to look wrong. Instead, it is better to generate a texture which
		encodes the normals as an object-space normal map which keeps lighting
		fidelity constant at all LODs, but does require shaders to use. Even if you're
		targetting the fixed-function pipeline you're better not to use vertex normals
		and instead use baked lightmaps.
		(default false)
		*/
		static bool getGenerateVertexNormals() { return msGenerateVertexNormals; }
		/** Static method - whether to generate Normals on terrain
		(default true)
		@remarks
		Changing this value only applies to Terrain instances loaded / reloaded afterwards.
		*/
		static void setGenerateVertexNormals(bool nm) { msGenerateVertexNormals = nm; }
		/** Whether to generate a texture containing the normals of the terrain.
		@remarks
			Because of the variable LOD of the terrain, storing normals in vertices
			doesn't produce attractive results. Instead, a terrain-wide normal map
			is typically used, storing the object-space normals of the terrain
			(default true)
		*/
		static bool getGenerateNormalMap() { return msGenerateNormalMap; }
		/** Whether to generate a texture containing the normals of the terrain. */
		static void setGenerateNormalMap(bool nm) { msGenerateNormalMap = nm; }
		/** Whether to generate a texture containing the shadows of the terrain at
			a particular time of day.
		@remarks
			This can be a useful feature for older hardware, or indeed any 
			situation where dynamic shadows are not required. It requires that
			the light direction is set too.
		*/
		static bool getGenerateShadowMap() { return msGenerateShadowMap; }
		/** Whether to generate a texture containing the shadows of the terrain. */
		static void setGenerateShadowMap(bool sm) { msGenerateShadowMap = sm; }
		/// Get the shadow map light direction to use (world space)
		static const Vector3& getShadowMapDirection() { return msShadowMapDir; }
		/** Set the shadow map light direction to use (world space). */
		static void setShadowMapDirection(const Vector3& v) { msShadowMapDir = v; }

		/** Whether to generate a horizon map containing the angles of the horizon
			along a given linear sun track, which allows real-time shadows to be
			generated for the terrain very cheaply without using shadow textures.
		@remarks
			This can be a useful feature for moderate hardware, giving you the ability
			to self-shadow the terrain without rendering depth shadow maps. Each point
			in the texture encodes 2 values - the highest point on the terrain that
			can be seen from that point in the direction of the sun in the morning, 
			and the same thing in the evening (ie the opposite direction). With these
			two values and a single 'time of day' value, you can calculate terrain
			shadowing. 
		@note This option uses the azimuth setting to detemine the track of the sun.
			At runtime, it uses the zenith value which represents the time of day.
		*/
		static bool getGenerateHorizonMap() { return msGenerateHorizonMap; }
		/** Whether to generate a texture containing the shadows of the terrain. */
		static void setGenerateHorizonMap(bool hm) { msGenerateHorizonMap = hm; }
		/** Get the azimuth angle of the sun track across the terrain for use in 
			generating the horizon map; zero is  east/west (default)
		*/
		static Radian getHorizonMapAzimuth() { return msHorizonMapAzimuth; }
		/** Set the azimuth angle of the sun track across the terrain; zero is 
		east/west (default)
		*/
		static void setHorizonMapAzimuth(Radian az) { msHorizonMapAzimuth = az; }
		/** Get the current zenith angle of the sun; zero is midday (default), 
		    -HALF_PI is sunrise, HALF_PI is sunset. This doesn't affect the 
			calculation of the horizon map, it controls the current state in 
			which it is used.
		*/
		static Radian getHorizonMapZenith() { return msHorizonMapZenith; }
		/** Set the current zenith angle of the sun; zero is midday (default), 
		    -HALF_PI is sunrise, HALF_PI is sunset. This doesn't affect the 
			calculation of the horizon map, it controls the current state in 
			which it is used, therefore this can be varied dynamically
		*/
		static void setHorizonMapZenith(Radian z) { msHorizonMapZenith = z; }

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

		/** Get the list of material generators that are currently available.
		*/
		static const TerrainMaterialGeneratorList& getMaterialGeneratorList() { return msMatGeneratorList; }

		/** Add a new material generator to the list. Generators added later
			take precedence over those added earlier. 
		*/
		static void addMaterialGenerator(TerrainMaterialGenerator* gen);
		/** Remove a material generator from the list 
		*/
		static void removeMaterialGenerator(TerrainMaterialGenerator* gen);

		/** Get the first material generator that can generate materials for the
			given layer declaration.
		*/
		static TerrainMaterialGenerator* getMaterialGenerator(const TerrainLayerDeclaration& decl);

		/** Get the default size of the blend maps for a new terrain. 
		*/
		static uint16 getLayerBlendMapSize() { return msLayerBlendMapSize; }

		/** Sets the default size of blend maps for a new terrain.
		This is the resolution of each blending layer for a new terrain. 
		Once created, this information will be stored with the terrain. 
		*/
		static void setLayerBlendMapSize(uint16 sz) { msLayerBlendMapSize = sz;}


	};


	/** @} */
	/** @} */
}




#endif 
