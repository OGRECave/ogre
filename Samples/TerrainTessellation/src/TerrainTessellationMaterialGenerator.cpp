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

#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrain.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreShadowCameraSetupPSSM.h"
#include "TerrainTessellationMaterialGenerator.h"

using namespace Ogre;
TerrainTessellationMaterialGenerator::TerrainTessellationMaterialGenerator(Ogre::String materialName, bool addNormalmap, bool cloneMaterial) 
    : mMaterialName(materialName)
    , mCloneMaterial(cloneMaterial)
    , mAddNormalMap(addNormalmap)
{
    mProfiles.push_back(OGRE_NEW Profile(this, materialName, "Profile for rendering Ogre standard material"));
    setActiveProfile(materialName);
}
// -----------------------------------------------------------------------------------------------------------------------
void TerrainTessellationMaterialGenerator::setMaterialByName(const Ogre::String materialName) 
{
    mMaterialName = materialName;
    _markChanged();
}
// -----------------------------------------------------------------------------------------------------------------------
TerrainTessellationMaterialGenerator::Profile::Profile(TerrainMaterialGenerator* parent, const Ogre::String& name, const Ogre::String& desc)
    : TerrainMaterialGenerator::Profile(parent, name, desc)
    , mLightmapEnabled(false) 
{

}
// -----------------------------------------------------------------------------------------------------------------------
TerrainTessellationMaterialGenerator::Profile::~Profile()
{

}
// -----------------------------------------------------------------------------------------------------------------------
MaterialPtr TerrainTessellationMaterialGenerator::Profile::generate(const Terrain* terrain)
{
    const Ogre::String& matName = terrain->getMaterialName();        
    MaterialPtr mat = MaterialManager::getSingleton().getByName( matName ).staticCast<Material>();

    if (mat) 
        MaterialManager::getSingleton().remove(matName);

    TerrainTessellationMaterialGenerator* parent = (TerrainTessellationMaterialGenerator*)getParent();
    
    // Set Ogre material 
    mat = MaterialManager::getSingleton().getByName( parent->mMaterialName ).staticCast<Material>();

    // Clone material
    if(parent->mCloneMaterial) 
    {
        mat = mat->clone(matName);
        parent->mMaterialName = matName;
    }
      
    // Add normalmap
    if(parent->mAddNormalMap) 
    {
        // Get default pass
        Pass *p = mat->getTechnique(0)->getPass(0);      

        // Add terrain's global normalmap to renderpass so the fragment program can find it.
        TextureUnitState *tu = p->createTextureUnitState(matName+"/nm");

        TexturePtr nmtx = terrain->getTerrainNormalMap();
        tu->_setTexturePtr(nmtx);   
    }
      
    return mat;
}
// -----------------------------------------------------------------------------------------------------------------------
MaterialPtr TerrainTessellationMaterialGenerator::Profile::generateForCompositeMap(const Terrain* terrain)
{
    return terrain->_getCompositeMapMaterial();
}
// -----------------------------------------------------------------------------------------------------------------------
Ogre::uint8 TerrainTessellationMaterialGenerator::Profile::getMaxLayers(const Terrain* terrain) const
{
    return 0;
}
// -----------------------------------------------------------------------------------------------------------------------
void TerrainTessellationMaterialGenerator::Profile::updateParams(const MaterialPtr& mat, const Terrain* terrain)
{

}
// -----------------------------------------------------------------------------------------------------------------------
void TerrainTessellationMaterialGenerator::Profile::updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain)
{

}
// -----------------------------------------------------------------------------------------------------------------------
void TerrainTessellationMaterialGenerator::Profile::requestOptions(Terrain* terrain)
{
    terrain->_setMorphRequired(false);
    terrain->_setNormalMapRequired(true); // enable global normal map
    terrain->_setLightMapRequired(false);
    terrain->_setCompositeMapRequired(false);
};
// -----------------------------------------------------------------------------------------------------------------------
void  TerrainTessellationMaterialGenerator::Profile::setLightmapEnabled(bool enabled)
{
    if (enabled != mLightmapEnabled)
    {
        mLightmapEnabled = enabled;
        mParent->_markChanged();
    }
};