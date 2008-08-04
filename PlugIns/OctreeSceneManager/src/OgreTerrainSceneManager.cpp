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
/***************************************************************************
terrainscenemanager.cpp  -  description
-------------------
begin                : Mon Sep 23 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team

***************************************************************************/
#include <OgreTerrainSceneManager.h>
#include <OgreImage.h>
#include <OgreConfigFile.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreCamera.h>
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreTerrainVertexProgram.h"
#include "OgreTerrainPage.h"
#include "OgreLogManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreMaterialManager.h"
#include "OgreHeightmapTerrainPageSource.h"
#include <fstream>

#define TERRAIN_MATERIAL_NAME "TerrainSceneManager/Terrain"

namespace Ogre
{
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    TerrainSceneManager::TerrainSceneManager(const String& name) 
		: OctreeSceneManager(name)
    {
        //setDisplaySceneNodes( true );
        //setShowBoxes( true );

        mUseCustomMaterial = false;
        mUseNamedParameterLodMorph = false;
        mLodMorphParamIndex = 3;
        mTerrainRoot = 0;
        mActivePageSource = 0;
        mPagingEnabled = false;
        mLivePageMargin = 0;
        mBufferedPageMargin = 0;		


    }
	//-------------------------------------------------------------------------
	const String& TerrainSceneManager::getTypeName(void) const
	{
		return TerrainSceneManagerFactory::FACTORY_TYPE_NAME;
	}
	//-------------------------------------------------------------------------
	void TerrainSceneManager::shutdown(void)
	{
		// Make sure the indexes are destroyed during orderly shutdown
		// and not when statics are destroyed (may be too late)
		mIndexCache.shutdown();
		destroyLevelIndexes();

		// Make sure we free up material (static)
		mOptions.terrainMaterial.setNull();

		// Shut down page source to free terrain pages
		if (mActivePageSource)
		{
			mActivePageSource->shutdown();
		}

	}
    //-------------------------------------------------------------------------
    TerrainSceneManager::~TerrainSceneManager()
    {
		shutdown();
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::loadConfig(DataStreamPtr& stream)
    {
        /* Set up the options */
        ConfigFile config;
        String val;

        config.load( stream );

        val = config.getSetting( "DetailTile" );
        if ( !val.empty() )
            setDetailTextureRepeat(atoi(val.c_str()));

        val = config.getSetting( "MaxMipMapLevel" );
        if ( !val.empty() )
            setMaxGeoMipMapLevel(atoi( val.c_str() ));


        val = config.getSetting( "PageSize" );
        if ( !val.empty() )
            setPageSize(atoi( val.c_str() ));
        else
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing option 'PageSize'",
            "TerrainSceneManager::loadConfig");


        val = config.getSetting( "TileSize" );
        if ( !val.empty() )
            setTileSize(atoi( val.c_str() ));
        else
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing option 'TileSize'",
            "TerrainSceneManager::loadConfig");

        Vector3 v = Vector3::UNIT_SCALE;

        val = config.getSetting( "PageWorldX" );
        if ( !val.empty() )
            v.x = atof( val.c_str() );

        val = config.getSetting( "MaxHeight" );
        if ( !val.empty() )
            v.y = atof( val.c_str() );

        val = config.getSetting( "PageWorldZ" );
        if ( !val.empty() )
            v.z = atof( val.c_str() );

        // Scale x/z relative to pagesize
        v.x /= mOptions.pageSize - 1;
        v.z /= mOptions.pageSize - 1;
        setScale(v);

        val = config.getSetting( "MaxPixelError" );
        if ( !val.empty() )
            setMaxPixelError(atoi( val.c_str() ));

        mDetailTextureName = config.getSetting( "DetailTexture" );

        mWorldTextureName = config.getSetting( "WorldTexture" );

        if ( config.getSetting( "VertexColours" ) == "yes" )
            mOptions.coloured = true;

        if ( config.getSetting( "VertexNormals" ) == "yes" )
            mOptions.lit = true;

        if ( config.getSetting( "UseTriStrips" ) == "yes" )
            setUseTriStrips(true);

        if ( config.getSetting( "VertexProgramMorph" ) == "yes" )
            setUseLODMorph(true);

        val = config.getSetting( "LODMorphStart");
        if ( !val.empty() )
            setLODMorphStart(atof(val.c_str()));

        val = config.getSetting( "CustomMaterialName" );
        if ( !val.empty() )
            setCustomMaterial(val);

        val = config.getSetting( "MorphLODFactorParamName" );
        if ( !val.empty() )
            setCustomMaterialMorphFactorParam(val);

        val = config.getSetting( "MorphLODFactorParamIndex" );
        if ( !val.empty() )
            setCustomMaterialMorphFactorParam(atoi(val.c_str()));

        // Now scan through the remaining settings, looking for any PageSource
        // prefixed items
        String pageSourceName = config.getSetting("PageSource");
        if (pageSourceName == "")
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing option 'PageSource'",
            "TerrainSceneManager::loadConfig");
        }
        TerrainPageSourceOptionList optlist;
        ConfigFile::SettingsIterator setIt = config.getSettingsIterator();
        while (setIt.hasMoreElements())
        {
            String name = setIt.peekNextKey();
            String value = setIt.getNext();
            if (StringUtil::startsWith(name, pageSourceName, false))
            {
                optlist.push_back(TerrainPageSourceOption(name, value));
            }
        }
        // set the page source
        selectPageSource(pageSourceName, optlist);


    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setupTerrainMaterial(void)
    {
        if (mCustomMaterialName == "")
        {
            // define our own material
            mOptions.terrainMaterial = 
                MaterialManager::getSingleton().getByName(TERRAIN_MATERIAL_NAME);
			// Make unique terrain material name
			StringUtil::StrStreamType s;
			s << mName << "/Terrain";
			mOptions.terrainMaterial = MaterialManager::getSingleton().getByName(s.str());
            if (mOptions.terrainMaterial.isNull())
            {
                mOptions.terrainMaterial = MaterialManager::getSingleton().create(
                    s.str(),
                    ResourceGroupManager::getSingleton().getWorldResourceGroupName());

            }
            else
            {
                mOptions.terrainMaterial->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
            }

            Pass* pass = mOptions.terrainMaterial->getTechnique(0)->getPass(0);

            if ( mWorldTextureName != "" )
            {
                pass->createTextureUnitState( mWorldTextureName, 0 );
            }
            if ( mDetailTextureName != "" )
            {
                pass->createTextureUnitState( mDetailTextureName, 1 );
            }

            mOptions.terrainMaterial -> setLightingEnabled( mOptions.lit );

            if (mOptions.lodMorph && 
                mDestRenderSystem->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM) &&
				GpuProgramManager::getSingleton().getByName("Terrain/VertexMorph").isNull())
            {
                // Create & assign LOD morphing vertex program
                String syntax;
                if (GpuProgramManager::getSingleton().isSyntaxSupported("arbvp1"))
                {
                    syntax = "arbvp1";
                }
                else
                {
                    syntax = "vs_1_1";
                }

                // Get source, and take into account current fog mode
                FogMode fm = getFogMode();
                const String& source = TerrainVertexProgram::getProgramSource(
                    fm, syntax);

                GpuProgramPtr prog = GpuProgramManager::getSingleton().createProgramFromString(
                    "Terrain/VertexMorph", ResourceGroupManager::getSingleton().getWorldResourceGroupName(), 
                    source, GPT_VERTEX_PROGRAM, syntax);

                // Attach
                pass->setVertexProgram("Terrain/VertexMorph");

                // Get params
                GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();

                // worldviewproj
                params->setAutoConstant(0, GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
                // morph factor
                params->setAutoConstant(4, GpuProgramParameters::ACT_CUSTOM, MORPH_CUSTOM_PARAM_ID);
                // fog exp density(if relevant)
                if (fm == FOG_EXP || fm == FOG_EXP2)
                {
                    params->setConstant(5, Vector3(getFogDensity(), 0, 0));
                    // Override scene fog since otherwise it's applied twice
                    // Set to linear and we derive [0,1] fog value in the shader
                    pass->setFog(true, FOG_LINEAR, getFogColour(), 0, 1, 0);
                }

				// Also set shadow receiver program
				const String& source2 = TerrainVertexProgram::getProgramSource(
					fm, syntax, true);

				prog = GpuProgramManager::getSingleton().createProgramFromString(
					"Terrain/VertexMorphShadowReceive", 
					ResourceGroupManager::getSingleton().getWorldResourceGroupName(), 
					source2, GPT_VERTEX_PROGRAM, syntax);
				pass->setShadowReceiverVertexProgram("Terrain/VertexMorphShadowReceive");
				params = pass->getShadowReceiverVertexProgramParameters();
				// worldviewproj
				params->setAutoConstant(0, GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
				// world
				params->setAutoConstant(4, GpuProgramParameters::ACT_WORLD_MATRIX);
				// texture view / proj
				params->setAutoConstant(8, GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				// morph factor
				params->setAutoConstant(12, GpuProgramParameters::ACT_CUSTOM, MORPH_CUSTOM_PARAM_ID);
				
				if (mDetailTextureName != "")
				{
					pass->getTextureUnitState(1)->setTextureCoordSet(1);
				}


                // Set param index
                mLodMorphParamName = "";
                mLodMorphParamIndex = 4;
            }

            mOptions.terrainMaterial->load();

        }
        else
        {
            // Custom material
            mOptions.terrainMaterial = 
                MaterialManager::getSingleton().getByName(mCustomMaterialName);
			if (mOptions.terrainMaterial.isNull()){
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Material " + mCustomMaterialName + " not found.", 
					"TerrainSceneManager::setupTerrainMaterial");
			}
            mOptions.terrainMaterial->load();

        }

        // now set up the linkage between vertex program and LOD morph param
        if (mOptions.lodMorph)
        {
            Technique* t = mOptions.terrainMaterial->getBestTechnique();
            for (ushort i = 0; i < t->getNumPasses(); ++i)
            {
                Pass* p = t->getPass(i);
                if (p->hasVertexProgram())
                {
                    // we have to assume vertex program includes LOD morph capability
                    GpuProgramParametersSharedPtr params = 
                        p->getVertexProgramParameters();
                    // Check to see if custom param is already there
                    GpuProgramParameters::AutoConstantIterator aci = params->getAutoConstantIterator();
                    bool found = false;
                    while (aci.hasMoreElements())
                    {
                        const GpuProgramParameters::AutoConstantEntry& ace = aci.getNext();
                        if (ace.paramType == GpuProgramParameters::ACT_CUSTOM && 
                            ace.data == MORPH_CUSTOM_PARAM_ID)
                        {
                            found = true;
                        }
                    }
                    if (!found)
                    {
                        if(mLodMorphParamName != "")
                        {
                            params->setNamedAutoConstant(mLodMorphParamName, 
                                GpuProgramParameters::ACT_CUSTOM, MORPH_CUSTOM_PARAM_ID);
                        }
                        else
                        {
                            params->setAutoConstant(mLodMorphParamIndex, 
                                GpuProgramParameters::ACT_CUSTOM, MORPH_CUSTOM_PARAM_ID);
                        }
                    }

                }
            }
        }

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setupTerrainPages(void)
    {

        //create a root terrain node.
        if (!mTerrainRoot)
            mTerrainRoot = getRootSceneNode() -> createChildSceneNode( "Terrain" );

        //setup the page array.
        unsigned short pageSlots = 1 + (mBufferedPageMargin * 2);
        unsigned short i, j;
        for (i = 0; i < pageSlots; ++i)
        {
            mTerrainPages.push_back(TerrainPageRow());
            for (j = 0; j < pageSlots; ++j)
            {
                mTerrainPages[i].push_back(0);
            }
        }

		// If we're not paging, load immediate for convenience
		if ( mActivePageSource && !mPagingEnabled )
			mActivePageSource->requestPage(0,0);


    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setWorldGeometry( const String& filename )
    {
		// try to open in the current folder first
		std::ifstream fs;
		fs.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (fs)
		{
			// Wrap as a stream
			DataStreamPtr stream(
				OGRE_NEW FileStreamDataStream(filename, &fs, false));
			setWorldGeometry(stream);
		}
		else
		{
			// otherwise try resource system
			DataStreamPtr stream = 
				ResourceGroupManager::getSingleton().openResource(filename, 
					ResourceGroupManager::getSingleton().getWorldResourceGroupName());
				
			setWorldGeometry(stream);
		}
	}
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setWorldGeometry(DataStreamPtr& stream, const String& typeName )
    {
        // Clear out any existing world resources (if not default)
        if (ResourceGroupManager::getSingleton().getWorldResourceGroupName() != 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
        {
            ResourceGroupManager::getSingleton().clearResourceGroup(
                ResourceGroupManager::getSingleton().getWorldResourceGroupName());
        }
		destroyLevelIndexes();
        mTerrainPages.clear();
        // Load the configuration
        loadConfig(stream);
		initLevelIndexes();

        // Resize the octree, allow for 1 page for now
        float max_x = mOptions.scale.x * mOptions.pageSize;
        float max_y = mOptions.scale.y;
        float max_z = mOptions.scale.z * mOptions.pageSize;
        resize( AxisAlignedBox( 0, 0, 0, max_x, max_y, max_z ) );

        setupTerrainMaterial();

        setupTerrainPages();

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::clearScene(void)
    {
        OctreeSceneManager::clearScene();
        mTerrainPages.clear();
		destroyLevelIndexes();
        // Octree has destroyed our root
        mTerrainRoot = 0;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::_renderScene(Camera* cam, Viewport *vp, bool includeOverlays)
    {
        // For now, no paging and expect immediate response
        if (!mTerrainPages.empty() && mTerrainPages[0][0] == 0)
        {
            mActivePageSource->requestPage(0, 0);
        }
        SceneManager::_renderScene(cam, vp, includeOverlays);

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::attachPage(ushort pageX, ushort pageZ, TerrainPage* page)
    {
        assert(pageX == 0 && pageZ == 0 && "Multiple pages not yet supported");

        assert(mTerrainPages[pageX][pageZ] == 0 && "Page at that index not yet expired!");
        // Insert page into list
        mTerrainPages[pageX][pageZ] = page;
        // Attach page to terrain root
		if (page->pageSceneNode->getParentSceneNode() != mTerrainRoot)
			mTerrainRoot->addChild(page->pageSceneNode);

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::_renderVisibleObjects( void )
    {

        OctreeSceneManager::_renderVisibleObjects();

    }
    //-------------------------------------------------------------------------
    float TerrainSceneManager::getHeightAt( float x, float z )
    {


        Vector3 pt( x, 0, z );

        TerrainRenderable * t = getTerrainTile( pt );

        if ( t == 0 )
        {
            //  printf( "No tile found for point\n" );
            return -1;
        }

        float h = t -> getHeightAt( x, z );

        // printf( "Height is %f\n", h );
        return h;

    }
    //-------------------------------------------------------------------------
    TerrainPage* TerrainSceneManager::getTerrainPage( const Vector3 & pt )
    {
        if (mPagingEnabled)
        {
            // TODO
            return 0;
        }
        else
        {
            // Single page
            if (mTerrainPages.empty() || mTerrainPages[0].empty())
                return 0;
            return mTerrainPages[0][0];
        }
    }
    //-------------------------------------------------------------------------
    TerrainRenderable * TerrainSceneManager::getTerrainTile( const Vector3 & pt )
    {
		TerrainPage* tp = getTerrainPage(pt);
		if (!tp)
			return NULL;
		else
        	return tp->getTerrainTile(pt);
    }
    //-------------------------------------------------------------------------
    bool TerrainSceneManager::intersectSegment( const Vector3 & start, 
        const Vector3 & end, Vector3 * result )
    {
        TerrainRenderable * t = getTerrainTile( start );

        if ( t == 0 )
        {
            *result = Vector3( -1, -1, -1 );
            return false;
        }

        return t -> intersectSegment( start, end, result );
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setUseTriStrips(bool useStrips)
    {
        mOptions.useTriStrips = useStrips;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setUseLODMorph(bool morph)
    {
        // Set true only if vertex programs are supported
        mOptions.lodMorph = morph && 
            mDestRenderSystem->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM);
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setUseVertexNormals(bool useNormals)
    {
        mOptions.lit = useNormals;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setUseVertexColours(bool useColours)
    {
        mOptions.coloured = useColours;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setWorldTexture(const String& textureName)
    {
        mWorldTextureName = textureName;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setDetailTexture(const String& textureName)
    {
        mDetailTextureName = textureName;

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setDetailTextureRepeat(int repeat)
    {
        mOptions.detailTile = repeat;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setTileSize(int size) 
    {
        mOptions.tileSize = size;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setPageSize(int size)
    {
        mOptions.pageSize = size;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setMaxPixelError(int pixelError) 
    {
        mOptions.maxPixelError = pixelError;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setScale(const Vector3& scale)
    {
        mOptions.scale = scale;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setMaxGeoMipMapLevel(int maxMip)
    {
        mOptions.maxGeoMipMapLevel = maxMip;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setCustomMaterial(const String& materialName)
    {
        mCustomMaterialName = materialName;
        if (materialName != "")
            mUseCustomMaterial = true;
        else
            mUseCustomMaterial = false;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setCustomMaterialMorphFactorParam(const String& paramName)
    {
        mUseNamedParameterLodMorph = true;
        mLodMorphParamName = paramName;

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setCustomMaterialMorphFactorParam(size_t paramIndex)
    {
        mUseNamedParameterLodMorph = false;
        mLodMorphParamIndex = paramIndex;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setLODMorphStart(Real morphStart)
    {
        mOptions.lodMorphStart = morphStart;
    }
    //-------------------------------------------------------------------------
    Camera* TerrainSceneManager::createCamera( const String &name )
    {
        Camera* c = OctreeSceneManager::createCamera(name);

        // Set primary camera, if none
        if (!mOptions.primaryCamera)
            setPrimaryCamera(c);

        return c;

    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::setPrimaryCamera(const Camera* cam)
    {
        mOptions.primaryCamera = cam;
    }
    //-------------------------------------------------------------------------
    bool TerrainSceneManager::setOption( const String & name, const void *value )
    {
        if (name == "PageSize")
        {
            setPageSize(*static_cast<const int*>(value));
            return true;
        } 
        else if (name == "TileSize")
        {
            setTileSize(*static_cast<const int*>(value));
            return true;
        }
        else if (name == "PrimaryCamera")
        {
            setPrimaryCamera(static_cast<const Camera*>(value));
            return true;
        }
        else if (name == "MaxMipMapLevel")
        {
            setMaxGeoMipMapLevel(*static_cast<const int*>(value));
            return true;
        }
        else if (name == "Scale")
        {
            setScale(*static_cast<const Vector3*>(value));
            return true;
        }
        else if (name == "MaxPixelError")
        {
            setMaxPixelError(*static_cast<const int*>(value));
            return true;
        }
        else if (name == "UseTriStrips")
        {
            setUseTriStrips(*static_cast<const bool*>(value));
            return true;
        }
        else if (name == "VertexProgramMorph")
        {
            setUseLODMorph(*static_cast<const bool*>(value));
            return true;
        }
        else if (name == "DetailTile")
        {
            setDetailTextureRepeat(*static_cast<const int*>(value));
            return true;
        }
        else if (name == "LodMorphStart")
        {
            setLODMorphStart(*static_cast<const Real*>(value));
            return true;
        }
        else if (name == "VertexNormals")
        {
            setUseVertexNormals(*static_cast<const bool*>(value));
            return true;
        }
        else if (name == "VertexColours")
        {
            setUseVertexColours(*static_cast<const bool*>(value));
            return true;
        }
        else if (name == "MorphLODFactorParamName")
        {
            setCustomMaterialMorphFactorParam(*static_cast<const String*>(value));
            return true;
        }
        else if (name == "MorphLODFactorParamIndex")
        {
            setCustomMaterialMorphFactorParam(*static_cast<const size_t*>(value));
            return true;
        }
        else if (name == "CustomMaterialName")
        {
            setCustomMaterial(*static_cast<const String*>(value));
            return true;
        }
        else if (name == "WorldTexture")
        {
            setWorldTexture(*static_cast<const String*>(value));
            return true;
        }
        else if (name == "DetailTexture")
        {
            setDetailTexture(*static_cast<const String*>(value));
            return true;
        }
        else
        {
            return OctreeSceneManager::setOption(name, value);
        }

        return false;
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::registerPageSource(const String& typeName, 
        TerrainPageSource* source)
    {
		std::pair<PageSourceMap::iterator, bool> retPair = 
			mPageSources.insert(
				PageSourceMap::value_type(typeName, source));
		if (!retPair.second)
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"The page source " + typeName + " is already registered",
				"TerrainSceneManager::registerPageSource");
		}
        LogManager::getSingleton().logMessage(
            "TerrainSceneManager: Registered a new PageSource for "
            "type " + typeName);
    }
    //-------------------------------------------------------------------------
    void TerrainSceneManager::selectPageSource(const String& typeName, 
        TerrainPageSourceOptionList& optionList)
    {
        PageSourceMap::iterator i = mPageSources.find(typeName);
        if (i == mPageSources.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot locate a TerrainPageSource for type " + typeName,
                "TerrainSceneManager::selectPageSource");
        }

        if (mActivePageSource)
        {
            mActivePageSource->shutdown();
        }
        mActivePageSource = i->second;
        mActivePageSource->initialise(this, mOptions.tileSize, mOptions.pageSize,
            mPagingEnabled, optionList);

        LogManager::getSingleton().logMessage(
            "TerrainSceneManager: Activated PageSource " + typeName);

    }
    //-------------------------------------------------------------------------
    int TerrainSceneManager::getDetailTextureRepeat(void)
    {
        return mOptions.detailTile;
    }
    //-------------------------------------------------------------------------
    int TerrainSceneManager::getTileSize(void)
    {
        return mOptions.tileSize;
    }
    //-------------------------------------------------------------------------
    int TerrainSceneManager::getPageSize(void)
    {
        return mOptions.pageSize;
    }
    //-------------------------------------------------------------------------
    int TerrainSceneManager::getMaxPixelError(void)
    {
        return mOptions.maxPixelError;
    }
    //-------------------------------------------------------------------------
    const Vector3& TerrainSceneManager::getScale(void)
    {
        return mOptions.scale;
    }
    //-------------------------------------------------------------------------
    int TerrainSceneManager::getMaxGeoMipMapLevel(void)
    {
        return mOptions.maxGeoMipMapLevel;
    }
	//-----------------------------------------------------------------------
	void TerrainSceneManager::initLevelIndexes()
	{
		if ( mLevelIndex.size() == 0 )
		{
			for ( int i = 0; i < 16; i++ )
			{

				mLevelIndex.push_back( OGRE_NEW_T(IndexMap, MEMCATEGORY_GEOMETRY)() );

			}

		}
	}
	//-----------------------------------------------------------------------
	void TerrainSceneManager::destroyLevelIndexes()
	{
		for ( int i = 0; i < mLevelIndex.size(); i++ )
		{
			OGRE_DELETE_T(mLevelIndex[i], IndexMap, MEMCATEGORY_GEOMETRY);
		}
		mLevelIndex.clear();
	}
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    RaySceneQuery* 
        TerrainSceneManager::createRayQuery(const Ray& ray, unsigned long mask)
    {
        TerrainRaySceneQuery *trsq = OGRE_NEW TerrainRaySceneQuery(this);
        trsq->setRay(ray);
        trsq->setQueryMask(mask);
        return trsq;
    }
    //-------------------------------------------------------------------------
    TerrainRaySceneQuery::TerrainRaySceneQuery(SceneManager* creator)
        :OctreeRaySceneQuery(creator)
    {
      mSupportedWorldFragments.insert(SceneQuery::WFT_SINGLE_INTERSECTION);
    }
    //-------------------------------------------------------------------------
    TerrainRaySceneQuery::~TerrainRaySceneQuery()
    {
    }
    //-------------------------------------------------------------------------
    void TerrainRaySceneQuery::execute(RaySceneQueryListener* listener)
    {
        mWorldFrag.fragmentType = SceneQuery::WFT_SINGLE_INTERSECTION;

        const Vector3& dir = mRay.getDirection();
        const Vector3& origin = mRay.getOrigin();
        // Straight up / down?
        if (dir == Vector3::UNIT_Y || dir == Vector3::NEGATIVE_UNIT_Y)
        {
            Real height = static_cast<TerrainSceneManager*>(mParentSceneMgr)->getHeightAt(
                origin.x, origin.z);
            if (height != -1 && (height <= origin.y && dir.y < 0) || (height >= origin.y && dir.y > 0))
            {
                mWorldFrag.singleIntersection.x = origin.x;
                mWorldFrag.singleIntersection.z = origin.z;
                mWorldFrag.singleIntersection.y = height;
                if (!listener->queryResult(&mWorldFrag, 
                    (mWorldFrag.singleIntersection - origin).length()))
					return;
            }
        }
        else
        {
            // Perform arbitrary query
            if (static_cast<TerrainSceneManager*>(mParentSceneMgr)->intersectSegment(
                origin, origin + (dir * 100000), &mWorldFrag.singleIntersection))
            {
                if (!listener->queryResult(&mWorldFrag, 
                    (mWorldFrag.singleIntersection - origin).length()))
					return;
            }


        }
        OctreeRaySceneQuery::execute(listener);

    }
    //-------------------------------------------------------------------------
    MaterialPtr& TerrainSceneManager::getTerrainMaterial(void)
    {
        return mOptions.terrainMaterial;
    }
    //-------------------------------------------------------------------------
    TerrainSceneManager::PageSourceIterator TerrainSceneManager::getPageSourceIterator(void)
    {
        return PageSourceIterator(mPageSources.begin(), mPageSources.end());
    }
	//-------------------------------------------------------------------------
	void TerrainSceneManager::setWorldGeometryRenderQueue(uint8 qid)
	{
		OctreeSceneManager::setWorldGeometryRenderQueue(qid);

		for (TerrainPage2D::iterator pi = mTerrainPages.begin(); 
			pi != mTerrainPages.end(); ++pi)
		{
			TerrainPageRow& row = *pi;
			for (TerrainPageRow::iterator ri = row.begin(); ri != row.end(); ++ri)
			{
				TerrainPage* page = *ri;
				if (page)
				{
					page->setRenderQueue(qid);
				}
			}
		}

	}

	//-----------------------------------------------------------------------
	const String TerrainSceneManagerFactory::FACTORY_TYPE_NAME = "TerrainSceneManager";
	//-----------------------------------------------------------------------
	TerrainSceneManagerFactory::TerrainSceneManagerFactory()
	{
	}
	//-----------------------------------------------------------------------
	TerrainSceneManagerFactory::~TerrainSceneManagerFactory()
	{
		for (TerrainPageSources::iterator i = mTerrainPageSources.begin();
			i != mTerrainPageSources.end(); ++i)
		{
			OGRE_DELETE *i;
		}
		mTerrainPageSources.clear();
	}
	//-----------------------------------------------------------------------
	void TerrainSceneManagerFactory::initMetaData(void) const
	{
		mMetaData.typeName = FACTORY_TYPE_NAME;
		mMetaData.description = "Scene manager which generally organises the scene on "
			"the basis of an octree, but also supports terrain world geometry. ";
		mMetaData.sceneTypeMask = ST_EXTERIOR_CLOSE; // previous compatiblity
		mMetaData.worldGeometrySupported = true;
	}
	//-----------------------------------------------------------------------
	SceneManager* TerrainSceneManagerFactory::createInstance(
		const String& instanceName)
	{
		TerrainSceneManager* tsm = OGRE_NEW TerrainSceneManager(instanceName);
		// Create & register default sources (one per manager)
		HeightmapTerrainPageSource* ps = OGRE_NEW HeightmapTerrainPageSource();
		mTerrainPageSources.push_back(ps);
		tsm->registerPageSource("Heightmap", ps);

		return tsm;

	}
	//-----------------------------------------------------------------------
	void TerrainSceneManagerFactory::destroyInstance(SceneManager* instance)
	{
		OGRE_DELETE instance;
	}


} //namespace
