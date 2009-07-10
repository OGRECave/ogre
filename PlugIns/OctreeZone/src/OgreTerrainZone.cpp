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
OgreTerrainZone.cpp  -  based on OgreTerrainZone.h from Ogre3d

-----------------------------------------------------------------------------
begin                : Thu May 3 2007
copyright            : (C) 2007 by Eric Cha
email                : ericcATxenopiDOTcom

-----------------------------------------------------------------------------
*/
#include "OgreTerrainZone.h"
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
#include "OgreTerrainZonePage.h"
#include "OgreLogManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreMaterialManager.h"
#include "OgreHeightmapTerrainZonePageSource.h"
#include "OgrePCZSceneManager.h"
#include <fstream>

#define TERRAIN_MATERIAL_NAME "TerrainSceneManager/Terrain"

namespace Ogre
{
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    TerrainZone::TerrainZone(PCZSceneManager * pczsm,const String& name) 
		: OctreeZone(pczsm, name)
    {
		mZoneTypeName = "ZoneType_Terrain";
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
	void TerrainZone::shutdown(void)
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
    TerrainZone::~TerrainZone()
    {
		shutdown();
    }
	/** Set the enclosure node for this TerrainZone
	*/
	void TerrainZone::setEnclosureNode(PCZSceneNode * node)
	{
		mEnclosureNode = node;
		if (node)
		{
			// anchor the node to this zone
			node->anchorToHomeZone(this);
			// make sure node world bounds are up to date
			node->_updateBounds();
			// DON'T resize the octree to the same size as the enclosure node bounding box
			// resize(node->_getWorldAABB());
			// EXPERIMENTAL - prevent terrain zone enclosure node from visiting other zones
			node->allowToVisit(false);
		}
	}

    //-------------------------------------------------------------------------
    void TerrainZone::loadConfig(DataStreamPtr& stream)
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
            "TerrainZone::loadConfig");


        val = config.getSetting( "TileSize" );
        if ( !val.empty() )
            setTileSize(atoi( val.c_str() ));
        else
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Missing option 'TileSize'",
            "TerrainZone::loadConfig");

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
            "TerrainZone::loadConfig");
        }
        TerrainZonePageSourceOptionList optlist;
        ConfigFile::SettingsIterator setIt = config.getSettingsIterator();
        while (setIt.hasMoreElements())
        {
            String name = setIt.peekNextKey();
            String value = setIt.getNext();
            if (StringUtil::startsWith(name, pageSourceName, false))
            {
                optlist.push_back(TerrainZonePageSourceOption(name, value));
            }
        }
        // set the page source
        selectPageSource(pageSourceName, optlist);


    }
    //-------------------------------------------------------------------------
    void TerrainZone::setupTerrainMaterial(void)
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
                mPCZSM->getDestinationRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM) &&
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
                FogMode fm = mPCZSM->getFogMode();
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
                    params->setConstant(5, Vector3(mPCZSM->getFogDensity(), 0, 0));
                    // Override scene fog since otherwise it's applied twice
                    // Set to linear and we derive [0,1] fog value in the shader
                    pass->setFog(true, FOG_LINEAR, mPCZSM->getFogColour(), 0, 1, 0);
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
    void TerrainZone::setupTerrainZonePages(PCZSceneNode * parentNode)
    {

        //create a root terrain node.
        if (!mTerrainRoot)
		{
			mTerrainRoot = (PCZSceneNode*)(parentNode->createChildSceneNode(this->getName()+"_Node"));
			setEnclosureNode(mTerrainRoot);
		}
        //setup the page array.
        unsigned short pageSlots = 1 + (mBufferedPageMargin * 2);
        unsigned short i, j;
        for (i = 0; i < pageSlots; ++i)
        {
            mTerrainZonePages.push_back(TerrainZonePageRow());
            for (j = 0; j < pageSlots; ++j)
            {
                mTerrainZonePages[i].push_back(0);
            }
        }

		// If we're not paging, load immediate for convenience
		if ( mActivePageSource && !mPagingEnabled )
			mActivePageSource->requestPage(0,0);


    }
    //-------------------------------------------------------------------------
    void TerrainZone::setZoneGeometry( const String& filename, PCZSceneNode * parentNode )
    {
		// try to open in the current folder first
		std::ifstream fs;
		fs.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (fs)
		{
			// Wrap as a stream
			DataStreamPtr stream(
				OGRE_NEW FileStreamDataStream(filename, &fs, false));
			setZoneGeometry(stream, parentNode);
		}
		else
		{
			// otherwise try resource system
			DataStreamPtr stream = 
				ResourceGroupManager::getSingleton().openResource(filename, 
					ResourceGroupManager::getSingleton().getWorldResourceGroupName());
				
			setZoneGeometry(stream, parentNode);
		}
	}
    //-------------------------------------------------------------------------
    void TerrainZone::setZoneGeometry(DataStreamPtr& stream, PCZSceneNode * parentNode, const String& typeName )
    {
        // Clear out any existing world resources (if not default)
        if (ResourceGroupManager::getSingleton().getWorldResourceGroupName() != 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME)
        {
            ResourceGroupManager::getSingleton().clearResourceGroup(
                ResourceGroupManager::getSingleton().getWorldResourceGroupName());
        }
		destroyLevelIndexes();
        mTerrainZonePages.clear();
        // Load the configuration
        loadConfig(stream);
		initLevelIndexes();


        setupTerrainMaterial();

        setupTerrainZonePages(parentNode);

        // Resize the octree allow for 1 page for now
        float max_x = mOptions.scale.x * mOptions.pageSize;
        float max_y = mOptions.scale.y;
        float max_z = mOptions.scale.z * mOptions.pageSize;
        resize( AxisAlignedBox( 0, 0, 0, max_x, max_y, max_z ) );

    }
    //-------------------------------------------------------------------------
    void TerrainZone::clearZone(void)
    {
        mTerrainZonePages.clear();
		destroyLevelIndexes();
        // Octree has destroyed our root
        mTerrainRoot = 0;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::notifyBeginRenderScene(void)
    {
        // For now, no paging and expect immediate response
        if (!mTerrainZonePages.empty() && mTerrainZonePages[0][0] == 0)
        {
            mActivePageSource->requestPage(0, 0);
        }

    }
    //-------------------------------------------------------------------------
    void TerrainZone::attachPage(ushort pageX, ushort pageZ, TerrainZonePage* page)
    {
        assert(pageX == 0 && pageZ == 0 && "Multiple pages not yet supported");

        assert(mTerrainZonePages[pageX][pageZ] == 0 && "Page at that index not yet expired!");
        // Insert page into list
        mTerrainZonePages[pageX][pageZ] = page;
        // Attach page to terrain root
		if (page->pageSceneNode->getParentSceneNode() != mTerrainRoot)
			mTerrainRoot->addChild(page->pageSceneNode);

    }
    //-------------------------------------------------------------------------
    float TerrainZone::getHeightAt( float x, float z )
    {


        Vector3 pt( x, 0, z );

        TerrainZoneRenderable * t = getTerrainTile( pt );

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
    TerrainZonePage* TerrainZone::getTerrainZonePage( const Vector3 & pt )
    {
        if (mPagingEnabled)
        {
            // TODO
            return 0;
        }
        else
        {
            // Single page
            if (mTerrainZonePages.empty() || mTerrainZonePages[0].empty())
                return 0;
            return mTerrainZonePages[0][0];
        }
    }
    //-------------------------------------------------------------------------
    TerrainZonePage* TerrainZone::getTerrainZonePage( unsigned short x, unsigned short z)
    {
        if (mPagingEnabled)
        {
            // TODO
            return 0;
        }
        else
        {
            // Single page
            if (mTerrainZonePages.empty() || mTerrainZonePages[0].empty())
                return 0;
			if (x > mOptions.pageSize || z > mOptions.pageSize)
			{
				return mTerrainZonePages[0][0];
			}
            return mTerrainZonePages[x][z];
        }
    }

    //-------------------------------------------------------------------------
    TerrainZoneRenderable * TerrainZone::getTerrainTile( const Vector3 & pt )
    {
		TerrainZonePage* tp = getTerrainZonePage(pt);
		if (!tp)
			return NULL;
		else
        	return tp->getTerrainZoneTile(pt);
    }
    //-------------------------------------------------------------------------
    bool TerrainZone::intersectSegment( const Vector3 & start, 
        const Vector3 & end, Vector3 * result )
    {
        TerrainZoneRenderable * t = getTerrainTile( start );

        if ( t == 0 )
        {
            *result = Vector3( -1, -1, -1 );
            return false;
        }

        return t -> intersectSegment( start, end, result );
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setUseTriStrips(bool useStrips)
    {
        mOptions.useTriStrips = useStrips;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setUseLODMorph(bool morph)
    {
        // Set true only if vertex programs are supported
        mOptions.lodMorph = morph && 
            mPCZSM->getDestinationRenderSystem()->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM);
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setUseVertexNormals(bool useNormals)
    {
        mOptions.lit = useNormals;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setUseVertexColours(bool useColours)
    {
        mOptions.coloured = useColours;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setWorldTexture(const String& textureName)
    {
        mWorldTextureName = textureName;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setDetailTexture(const String& textureName)
    {
        mDetailTextureName = textureName;

    }
    //-------------------------------------------------------------------------
    void TerrainZone::setDetailTextureRepeat(int repeat)
    {
        mOptions.detailTile = repeat;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setTileSize(int size) 
    {
        mOptions.tileSize = size;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setPageSize(int size)
    {
        mOptions.pageSize = size;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setMaxPixelError(int pixelError) 
    {
        mOptions.maxPixelError = pixelError;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setScale(const Vector3& scale)
    {
        mOptions.scale = scale;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setMaxGeoMipMapLevel(int maxMip)
    {
        mOptions.maxGeoMipMapLevel = maxMip;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setCustomMaterial(const String& materialName)
    {
        mCustomMaterialName = materialName;
        if (materialName != "")
            mUseCustomMaterial = true;
        else
            mUseCustomMaterial = false;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setCustomMaterialMorphFactorParam(const String& paramName)
    {
        mUseNamedParameterLodMorph = true;
        mLodMorphParamName = paramName;

    }
    //-------------------------------------------------------------------------
    void TerrainZone::setCustomMaterialMorphFactorParam(size_t paramIndex)
    {
        mUseNamedParameterLodMorph = false;
        mLodMorphParamIndex = paramIndex;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setLODMorphStart(Real morphStart)
    {
        mOptions.lodMorphStart = morphStart;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::notifyCameraCreated( Camera* c )
    {
        // Set primary camera, if none
        if (!mOptions.primaryCamera)
            setPrimaryCamera(c);

        return;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::setPrimaryCamera(const Camera* cam)
    {
        mOptions.primaryCamera = cam;
    }
    //-------------------------------------------------------------------------
    bool TerrainZone::setOption( const String & name, const void *value )
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
            return OctreeZone::setOption(name, value);
        }

        return false;
    }
    //-------------------------------------------------------------------------
    void TerrainZone::registerPageSource(const String& typeName, 
        TerrainZonePageSource* source)
    {
		std::pair<PageSourceMap::iterator, bool> retPair = 
			mPageSources.insert(
				PageSourceMap::value_type(typeName, source));
		if (!retPair.second)
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"The page source " + typeName + " is already registered",
				"TerrainZone::registerPageSource");
		}
        LogManager::getSingleton().logMessage(
            "TerrainZone: Registered a new PageSource for "
            "type " + typeName);
    }
    //-------------------------------------------------------------------------
    void TerrainZone::selectPageSource(const String& typeName, 
        TerrainZonePageSourceOptionList& optionList)
    {
        PageSourceMap::iterator i = mPageSources.find(typeName);
        if (i == mPageSources.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot locate a TerrainZonePageSource for type " + typeName,
                "TerrainZone::selectPageSource");
        }

        if (mActivePageSource)
        {
            mActivePageSource->shutdown();
        }
        mActivePageSource = i->second;
        mActivePageSource->initialise(this, mOptions.tileSize, mOptions.pageSize,
            mPagingEnabled, optionList);

        LogManager::getSingleton().logMessage(
            "TerrainZone: Activated PageSource " + typeName);

    }
    //-------------------------------------------------------------------------
    int TerrainZone::getDetailTextureRepeat(void)
    {
        return mOptions.detailTile;
    }
    //-------------------------------------------------------------------------
    int TerrainZone::getTileSize(void)
    {
        return mOptions.tileSize;
    }
    //-------------------------------------------------------------------------
    int TerrainZone::getPageSize(void)
    {
        return mOptions.pageSize;
    }
    //-------------------------------------------------------------------------
    int TerrainZone::getMaxPixelError(void)
    {
        return mOptions.maxPixelError;
    }
    //-------------------------------------------------------------------------
    const Vector3& TerrainZone::getScale(void)
    {
        return mOptions.scale;
    }
    //-------------------------------------------------------------------------
    int TerrainZone::getMaxGeoMipMapLevel(void)
    {
        return mOptions.maxGeoMipMapLevel;
    }
	//-----------------------------------------------------------------------
	void TerrainZone::initLevelIndexes()
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
	void TerrainZone::destroyLevelIndexes()
	{
		for ( unsigned int i = 0; i < mLevelIndex.size(); i++ )
		{
			OGRE_DELETE_T(mLevelIndex[i], IndexMap, MEMCATEGORY_GEOMETRY);
		}
		mLevelIndex.clear();
	}
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
/*    RaySceneQuery* 
        TerrainZone::createRayQuery(const Ray& ray, unsigned long mask)
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
            Real height = static_cast<TerrainZone*>(mParentSceneMgr)->getHeightAt(
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
            if (static_cast<TerrainZone*>(mParentSceneMgr)->intersectSegment(
                origin, origin + (dir * 100000), &mWorldFrag.singleIntersection))
            {
                if (!listener->queryResult(&mWorldFrag, 
                    (mWorldFrag.singleIntersection - origin).length()))
					return;
            }


        }
        OctreeRaySceneQuery::execute(listener);

    }
	*/
    //-------------------------------------------------------------------------
    MaterialPtr& TerrainZone::getTerrainMaterial(void)
    {
        return mOptions.terrainMaterial;
    }
    //-------------------------------------------------------------------------
    TerrainZone::PageSourceIterator TerrainZone::getPageSourceIterator(void)
    {
        return PageSourceIterator(mPageSources.begin(), mPageSources.end());
    }
	//-------------------------------------------------------------------------
	void TerrainZone::notifyWorldGeometryRenderQueue(uint8 qid)
	{
		for (TerrainZonePage2D::iterator pi = mTerrainZonePages.begin(); 
			pi != mTerrainZonePages.end(); ++pi)
		{
			TerrainZonePageRow& row = *pi;
			for (TerrainZonePageRow::iterator ri = row.begin(); ri != row.end(); ++ri)
			{
				TerrainZonePage* page = *ri;
				if (page)
				{
					page->setRenderQueue(qid);
				}
			}
		}

	}

	//-------------------------------------------------------------------------
	// TerrainZoneFactory functions
	//-----------------------------------------------------------------------
	//String terrainZoneString = String("ZoneType_Terrain"); 

	TerrainZoneFactory::TerrainZoneFactory() : PCZoneFactory(String("ZoneType_Terrain"))
	{
	}
	//-----------------------------------------------------------------------
	TerrainZoneFactory::~TerrainZoneFactory()
	{
		for (TerrainZonePageSources::iterator i = mTerrainZonePageSources.begin();
			i != mTerrainZonePageSources.end(); ++i)
		{
			OGRE_DELETE *i;
		}
		mTerrainZonePageSources.clear();
	}
	bool TerrainZoneFactory::supportsPCZoneType(const String& zoneType)
	{
		if (mFactoryTypeName == zoneType)
		{
			return true;
		}
		return false;
	}
	PCZone* TerrainZoneFactory::createPCZone(PCZSceneManager * pczsm, const String& zoneName)
	{
		TerrainZone * tz = OGRE_NEW TerrainZone(pczsm, zoneName);
		// Create & register default sources (one per zone)
		HeightmapTerrainZonePageSource* ps = OGRE_NEW HeightmapTerrainZonePageSource();
		mTerrainZonePageSources.push_back(ps);
		tz->registerPageSource("Heightmap", ps);
		return tz;
	}


} //namespace
