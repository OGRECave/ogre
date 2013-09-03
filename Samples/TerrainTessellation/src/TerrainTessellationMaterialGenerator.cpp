/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "TerrainTessellationMaterialGenerator.h"

   TerrainMaterial::TerrainMaterial(Ogre::String materialName, bool addNormalmap, bool cloneMaterial) 
      : mMaterialName(materialName), mAddNormalMap(addNormalmap), mCloneMaterial(cloneMaterial)
    {
        mProfiles.push_back(OGRE_NEW Profile(this, "Ogre/TerrainTessellation", "Profile for rendering Ogre standard material"));
        setActiveProfile("OgreMaterial");
    }
    
    void TerrainMaterial::setMaterialByName(const Ogre::String materialName) {
      mMaterialName = materialName;
      _markChanged();
    };

    // -----------------------------------------------------------------------------------------------------------------------

    TerrainMaterial::Profile::Profile(Ogre::TerrainMaterialGenerator* parent, const Ogre::String& name, const Ogre::String& desc)
		: Ogre::TerrainMaterialGenerator::Profile(parent, name, desc), mLightmapEnabled(false) 
    {
    };

    TerrainMaterial::Profile::~Profile()
    {
    };

    Ogre::MaterialPtr TerrainMaterial::Profile::generate(const Ogre::Terrain* terrain)
    {
      const Ogre::String& matName = terrain->getMaterialName();        

	  MaterialPtr mat = MaterialManager::getSingleton().getByName( matName ).staticCast<Material>();

      if (!mat.isNull()) 
         Ogre::MaterialManager::getSingleton().remove(matName);

      TerrainMaterial* parent = (TerrainMaterial*)getParent();

      // Set Ogre material 
      mat = Ogre::MaterialManager::getSingleton().getByName(parent->mMaterialName);

      // Clone material
      if(parent->mCloneMaterial) {
         mat = mat->clone(matName);
         parent->mMaterialName = matName;
      }
      
      // Add normalmap
      if(parent->mAddNormalMap) {
         // Get default pass
         Ogre::Pass *p = mat->getTechnique(0)->getPass(0);      

         // Add terrain's global normalmap to renderpass so the fragment program can find it.
         Ogre::TextureUnitState *tu = p->createTextureUnitState(matName+"/nm");

         Ogre::TexturePtr nmtx = terrain->getTerrainNormalMap();
         tu->_setTexturePtr(nmtx);   
      }
      
      return mat;
    };
    
    Ogre::MaterialPtr TerrainMaterial::Profile::generateForCompositeMap(const Ogre::Terrain* terrain)
    {
      return terrain->_getCompositeMapMaterial();
    };

    Ogre::uint8 TerrainMaterial::Profile::getMaxLayers(const Ogre::Terrain* terrain) const
    {
        return 0;
    };

    void TerrainMaterial::Profile::updateParams(const Ogre::MaterialPtr& mat, const Ogre::Terrain* terrain)
    {
    };

    void TerrainMaterial::Profile::updateParamsForCompositeMap(const Ogre::MaterialPtr& mat, const Ogre::Terrain* terrain)
    {
    };

    void TerrainMaterial::Profile::requestOptions(Ogre::Terrain* terrain)
    {
        terrain->_setMorphRequired(false);
        terrain->_setNormalMapRequired(true); // enable global normal map
        terrain->_setLightMapRequired(false);
        terrain->_setCompositeMapRequired(false);
    };
	void  TerrainMaterial::Profile::setLightmapEnabled(bool enabled)
	{
		if (enabled != mLightmapEnabled)
		{
			mLightmapEnabled = enabled;
			mParent->_markChanged();
		}
	};