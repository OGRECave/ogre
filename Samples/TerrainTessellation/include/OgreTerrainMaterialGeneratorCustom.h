#ifndef CUSTOMTERRAINMATERIAL_H
#define CUSTOMTERRAINMATERIAL_H

#include "Ogre.h"
#include "OgreTerrain.h"
#include "OgreTerrainMaterialGenerator.h"

class TerrainMaterial : public Ogre::TerrainMaterialGenerator
{
public:
  
  TerrainMaterial(Ogre::String materialName, bool addNormalmap = false, bool cloneMaterial = true);
  
  void setMaterialByName(const Ogre::String materialName); 
  void addNormalMapOnGenerate(bool addNormalMap) { mAddNormalMap = addNormalMap; };
  void cloneMaterialOnGenerate(bool cloneMaterial) { mCloneMaterial = cloneMaterial; };

  Ogre::String getMaterialName() { return mMaterialName; };
	
	class Profile : public Ogre::TerrainMaterialGenerator::Profile
	{
	public:
		Profile(Ogre::TerrainMaterialGenerator* parent, const Ogre::String& name, const Ogre::String& desc);
		~Profile();

		Ogre::MaterialPtr generate(const Ogre::Terrain* terrain);

		Ogre::MaterialPtr generateForCompositeMap(const Ogre::Terrain* terrain);

		Ogre::uint8 getMaxLayers(const Ogre::Terrain* terrain) const;

		void updateParams(const Ogre::MaterialPtr& mat, const Ogre::Terrain* terrain);

		void updateParamsForCompositeMap(const Ogre::MaterialPtr& mat, const Ogre::Terrain* terrain);

		void requestOptions(Ogre::Terrain* terrain);       

	};
protected:         
  Ogre::String mMaterialName; 
  bool mCloneMaterial;
  bool mAddNormalMap;
};
    
#endif