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
OgreTerrainZone.h  -  based on OgreTerrainSceneManager.h from Ogre3d 
-----------------------------------------------------------------------------
begin                : Thu May 3 2007
author               : Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/

#ifndef TERRAINZONE_H
#define TERRAINZONE_H

#include "OgreTerrainZonePrerequisites.h"
#include "OgreOctreeZone.h"
#include "OgrePCZSceneQuery.h"
#include "OgreTerrainZoneRenderable.h"
#include "OgreTerrainZonePageSource.h"
#include "OgreIteratorWrappers.h"


namespace Ogre
{

	class Image;

	typedef vector< TerrainZonePage * >::type TerrainZonePageRow;
	typedef vector< TerrainZonePageRow >::type TerrainZonePage2D;

	/** This is a basic PCZone for organizing TerrainRenderables into a total landscape.
	* It loads a terrain from a .cfg file that specifices what textures/scale/mipmaps/etc to use.
	*@author Jon Anderson
	*/

	class _OgreOctreeZonePluginExport TerrainZone : public OctreeZone
	{
	public:
		TerrainZone( PCZSceneManager *, const String& name);
		virtual ~TerrainZone( );

		/** Set the enclosure node for this TerrainZone
		*/
		virtual void setEnclosureNode(PCZSceneNode *);

		/** Sets the given option for the SceneManager.
		@remarks
			Options are (in addition to those supported by superclasses):
			"PageSize", int*;
			"TileSize", int*; 
			"PrimaryCamera, Camera*;
			"MaxMipMapLevel", int*;
			"Scale", Vector3 *;
			"MaxPixelError", int*;
			"UseTriStrips", bool*;
			"VertexProgramMorph", bool*;
			"DetailTile", int*;
			"LodMorphStart", Real*;
			"VertexNormals", bool*;
			"VertexColours", bool*;
			"MorphLODFactorParamName", String*;
			"MorphLODFactorParamIndex", size_t*;
			"CustomMaterialName", String*;
			"WorldTexture", String*;
			"DetailTexture", String*;
		*/
		virtual bool setOption( const String &, const void * );

		/** Loads the terrain using parameters int he given config file. */
		void setZoneGeometry( const String& filename, PCZSceneNode * parentNode );
		/** Loads the terrain using parameters in the given config file (contained 
			in a stream). */
		virtual void setZoneGeometry(DataStreamPtr& stream, 
									 PCZSceneNode * parentNode,
									 const String& typeName = StringUtil::BLANK);

		/** Returns the height at the given terrain coordinates. */
		virtual float getHeightAt( float x, float y );


		virtual bool intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result );

		/** Sets the texture to use for the main world texture. */
		virtual void setWorldTexture(const String& textureName);
		/** Sets the texture to use for the detail texture. */
		virtual void setDetailTexture(const String& textureName);
		/** Sets the number of times per tile the detail texture should be repeated. */
		virtual void setDetailTextureRepeat(int repeat);
		/** Sets the dimensions of each tile (must be power of 2 + 1) */
		virtual void setTileSize(int size); 
		/** Sets the dimensions of each page (must be power of 2 + 1) */
		virtual void setPageSize(int size); 
		/** Sets the maximum screen space pixel error.  */
		virtual void setMaxPixelError(int pixelError); 
		/** Sets how to scale the terrain data. */
		virtual void setScale(const Vector3& scale);
		/** Sets the maximum geomipmap level to allow. */
		virtual void setMaxGeoMipMapLevel(int maxMip);
	    
		/** Gets the texture to use for the main world texture. */
		virtual const String& getWorldTexture(void) { return mWorldTextureName; }
		/** Gets the texture to use for the detail texture. */
		virtual const String& getDetailTexture(void) { return mDetailTextureName; }
		/** Gets the number of times per tile the detail texture should be repeated. */
		virtual int getDetailTextureRepeat(void);
		/** Gets the dimensions of each tile (must be power of 2 + 1) */
		virtual int getTileSize(void); 
		/** Gets the dimensions of each page (must be power of 2 + 1) */
		virtual int getPageSize(void); 
		/** Gets the maximum screen space pixel error.  */
		virtual int getMaxPixelError(void); 
		/** Gets how to scale the terrain data. */
		virtual const Vector3& getScale(void);
		/** Gets the maximum geomipmap level to allow. */
		virtual int getMaxGeoMipMapLevel(void);



		/** Sets whether the terrain should use triangle strips or not.
		@remarks
			The default is not, in which case it uses triangle lists. 
		*/
		virtual void setUseTriStrips(bool useStrips);
		/** Sets whether or not terrain tiles should be morphed between LODs
		(NB requires vertex program support). */
		virtual void setUseLODMorph(bool useMorph);
		/** Sets whether vertex normals will be generated for the terrain. */
		virtual void setUseVertexNormals(bool useNormals);
		/** Sets whether vertex colours will be used. */
		virtual void setUseVertexColours(bool useColours);

		/** Sets the name of a custom material to use to shade the landcape.
		@remarks
			This method allows you to provide a custom material which will be
			used to render the landscape instead of the standard internal 
			material. This gives you a great deal of flexibility and allows you 
			to perform special effects if you wish. Note that because you determine
			every aspect of the material, this setting makes the use of setWorldTexture
			and setDetailTexture redundant.
		@par
			In your custom material, you can use all the usual features of Ogre's
			materials, including multiple passes if you wish. You can also use 
			the programmable pipeline to hook vertex and fragment programs into the
			terrain rendering. The plugin provides the following vertex components:
			<ul>
				<li>positions</li>
				<li>2 sets of texture coordinates (index 0 is world texture, 
				index 1 is detail texture)</li>
				<li>Normals, if enabled</li>
				<li>Per-vertex delta values, for morphing a higher LOD tile into 
					a lower LOD tile. This is one float per vertex bound as 'blend
					weight'. If you want to use this you also have to provide the
					name or index of the parameter you wish to receive the morph factor
					(@see setCustomMaterialMorphFactorParam)</li>
			</ul>
		*/
		virtual void setCustomMaterial(const String& materialName);
		/** Sets the name of the vertex program parameter to which to pass the
			LOD morph factor.
		@remarks
			When LOD morphing is enabled, and you are using a custom material to 
			shade the terrain, you need to inform this class of the parameter you
			wish the current LOD morph factor to be passed to. This is a simple
			float parameter value that the plugin will set from 0 to 1, depending on
			the morph stage of a tile. 0 represents no morphing, ie the vertices are
			all in the original position. 1 represents a complete morph such that
			the height of the vertices is the same as they are at the next lower LOD
			level. The vertex program must use this factor, in conjunction with the
			per-vertex height delta values (bound as 'blend weight'), to displace
			vertices.
		@note This version of the method lets you specify a parameter name, compatible
			with high-level vertex programs. There is an alternative signature method
			which allows you to set the parameter index for low-level assembler programs.
		@param paramName The name of the parameter which will receive the morph factor
		*/
		virtual void setCustomMaterialMorphFactorParam(const String& paramName);
		/** Sets the index of the vertex program parameter to which to pass the
			LOD morph factor.
		@remarks
			When LOD morphing is enabled, and you are using a custom material to 
			shade the terrain, you need to inform this class of the parameter you
			wish the current LOD morph factor to be passed to. This is a simple
			float parameter value that the plugin will set from 0 to 1, depending on
			the morph stage of a tile. 0 represents no morphing, ie the vertices are
			all in the original position. 1 represents a complete morph such that
			the height of the vertices is the same as they are at the next lower LOD
			level. The vertex program must use this factor, in conjunction with the
			per-vertex height delta values (bound as 'blend weight'), to displace
			vertices.
		@note This version of the method lets you specify a parameter index, compatible
			with low-level assembler vertex programs. There is an alternative signature method
			which allows you to set the parameter name for high-level programs.
		@param paramName The name of the parameter which will receive the morph factor
		*/
		virtual void setCustomMaterialMorphFactorParam(size_t paramIndex);
		/** Sets the distance at which the LOD will start to morph downwards, as
		a proportion of the distance between the LODs. */
		virtual void setLODMorphStart(Real morphStart);

		/** Returns the TerrainRenderable that contains the given pt.
			If no tile exists at the point, it returns 0;
		*/
		virtual TerrainZoneRenderable * getTerrainTile( const Vector3 & pt );

		/** Returns the TerrainZonePage that contains the given pt.
		If no page exists at the point, it returns 0;
		*/
		virtual TerrainZonePage* getTerrainZonePage( const Vector3 & pt );

		/** Return the TerrainZonePage according to page coordinates
		If page coordinates are outside of valid range, page 0,0 is returned
		*/
		virtual TerrainZonePage* getTerrainZonePage( unsigned short x, unsigned short z);

		/** called when the scene manager creates a camera in order to store the first camera created as the primary
			one, for determining error metrics and the 'home' terrain page.
		*/
		virtual void notifyCameraCreated( Camera* c );

		/// Gets the terrain options 
		virtual const TerrainZoneOptions& getOptions(void) { return mOptions; }

		/** Sets the 'primary' camera, i.e. the one which will be used to determine
			the 'home' terrain page, and to calculate the error metrics. 
		*/
		virtual void setPrimaryCamera(const Camera* cam);
		/// Internal map of page source name to page source
		typedef map<String, TerrainZonePageSource*>::type PageSourceMap;

		/// Iterator over all page sources
		typedef ConstMapIterator<PageSourceMap> PageSourceIterator;
		/// Get an iterator over all page sources
		virtual PageSourceIterator getPageSourceIterator(void);
		/** Registers a TerrainZonePageSource class and associates it with a named type 
			of source.
		@remarks
			This function allows external classes to register themselves as providers
			of terrain pages of a particular type. Only one page source can be
			active at once, and the active one is selected by calling 
			selectPageSource(typeName), which is part of plugin configuration.
		@note The terrain engine comes with a default page source which loads
			greyscale heightmap images, registered under the type name "Heightmap".
		@param typeName A unique String to associate with this type of source
		@param source Pointer to the class which will implement this source.
		*/
		virtual void registerPageSource(const String& typeName, TerrainZonePageSource* source);
		/** Selects a given page source based on its type name.
		@remarks
			This method activates a single page source based on its typename, 
			e.g. "Heightmap". It also passes to it a number of custom options
			which the source is able to interpret however it likes.
		@param typeName The type name of the page source to activate
		@param optionList A list of string parameters, which are expected to begin
			with 'typeName.' (e.g. "Heightmap.image"), with their appropriate
			values.
		*/
		virtual void selectPageSource(const String& typeName, 
			TerrainZonePageSourceOptionList& optionList);

		/** Attaches a previously built page to the list of available pages.
		@remarks
			TerrainZonePageSource subclasses will call this method once they have
			pages available to be added to the working set. Note that whilst you
			can build TerrainZonePage instances in another thread if you like, this
			method must be called in the same thread as the main rendering loop 
			in order to avoid concurrency issues. 
		@param pageX, pageZ The page index at which to attach the page
		@param page The page to attach
		*/
		virtual void attachPage(ushort pageX, ushort pageZ, TerrainZonePage* page);
		/// Get a pointer to the material being used for the terrain
		virtual MaterialPtr& getTerrainMaterial(void);
		// Called when a _renderScene is called in the SceneManager
		virtual void notifyBeginRenderScene(void);

		/// Get the SceneNode under which all terrain nodes are attached.
		virtual PCZSceneNode* getTerrainRootNode(void) const { return mTerrainRoot; }
		/** clear structures for the zone */
		virtual void clearZone(void);
		/* called by PCZSM during setWorldGeometryRenderQueue() */
		virtual void notifyWorldGeometryRenderQueue(uint8 qid);

		/// Get the shared list of indexes cached (internal use only)
		virtual TerrainBufferCache& _getIndexCache(void) {return mIndexCache;}

		/// Get the shared level index list (internal use only)
		virtual LevelArray& _getLevelIndex(void) { return mLevelIndex; }

		/// Get the current page count (internal use only)
		virtual size_t _getPageCount(void) { return mTerrainZonePages.size(); }

		/// Shutdown cleanly before we get destroyed
		virtual void shutdown(void);

	protected:

		/// Validates that the size picked for the terrain is acceptable
		bool _checkSize( int s )
		{
			for ( int i = 0; i < 16; i++ )
			{
				printf( "Checking...%d\n", ( 1 << i ) + 1 );

				if ( s == ( 1 << i ) + 1 )
					return true;
			}

			return false;

		}

		/// The node to which all terrain tiles are attached
		PCZSceneNode * mTerrainRoot;
		/// Terrain size, detail etc
		TerrainZoneOptions mOptions;
		/// Should we use an externally-defined custom material?
		bool mUseCustomMaterial;
		/// The name of the custom material to use
		String mCustomMaterialName;
		/// The name of the world texture
		String mWorldTextureName;
		/// The name of the detail texture
		String mDetailTextureName;
		/// Are we using a named parameter to hook up LOD morph?
		bool mUseNamedParameterLodMorph;
		/// The name of the parameter to send the LOD morph to
		String mLodMorphParamName;
		/// The index of the parameter to send the LOD morph to
		size_t mLodMorphParamIndex;
		/// Whether paging is enabled, or whether a single page will be used
		bool mPagingEnabled;
		/// The number of pages to render outside the 'home' page
		unsigned short mLivePageMargin;
		/// The number of pages to keep loaded outside the 'home' page
		unsigned short mBufferedPageMargin;
		/// Grid of buffered pages
		TerrainZonePage2D mTerrainZonePages;
		//-- attributes to share across tiles
		/// Shared list of index buffers
		TerrainBufferCache mIndexCache;
		/// Shared array of IndexData (reuse indexes across tiles)
		LevelArray mLevelIndex;
	    
		/// Internal method for loading configurations settings
		virtual void loadConfig(DataStreamPtr& stream);

		/// Sets up the terrain material
		virtual void setupTerrainMaterial(void);
		/// Sets up the terrain page slots
		virtual void setupTerrainZonePages(PCZSceneNode * parentNode);
		/// Initialise level indexes
		virtual void initLevelIndexes(void);
		/// Destroy level indexes
		virtual void destroyLevelIndexes(void);


		/// Map of source type -> TerrainZonePageSource
		PageSourceMap mPageSources;
		/// The currently active page source
		TerrainZonePageSource* mActivePageSource;

	};
	/// Factory for TerrainZone
	class TerrainZoneFactory : public PCZoneFactory
	{
	protected:
		typedef vector<TerrainZonePageSource*>::type TerrainZonePageSources;
		TerrainZonePageSources mTerrainZonePageSources;
	public:
		TerrainZoneFactory();
		virtual ~TerrainZoneFactory();
		/// Factory type name
		bool supportsPCZoneType(const String& zoneType);
		PCZone* createPCZone(PCZSceneManager * pczsm, const String& zoneName);
	};

}

#endif
