#ifdef SWIGPYTHON
%module(package="Ogre") Terrain
#else
%module OgreTerrain
#endif
%{
/* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreDefaultDebugDrawer.h"
#include "OgreTerrainLayerBlendMap.h"
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrain.h"
#include "OgreTerrainQuadTreeNode.h"
#include "OgreTerrainAutoUpdateLod.h"
#include "OgreTerrainGroup.h"
%}

%include std_string.i
%include std_vector.i
%include exception.i
%import "Ogre.i"

#define _OgreTerrainExport
#define __inline
#define __forceinline

%ignore Ogre::TerrainMaterialGenerator::setActiveProfile;
%include "OgreTerrainMaterialGenerator.h"
%include "OgreTerrainMaterialGeneratorA.h"

%include "OgreTerrainLayerBlendMap.h"
%include "OgreTerrainQuadTreeNode.h"

%template(LayerInstanceList) std::vector<Ogre::Terrain::LayerInstance>;
%template(TerrainRayResult) std::pair<bool, Ogre::Vector3>;
%ignore Ogre::Terrain::getBlendTextureCount;
%ignore Ogre::Terrain::getBlendTextureName;
%include "OgreTerrain.h"

%ignore Ogre::TerrainGroup::rayIntersects;
%ignore Ogre::TerrainGroup::getTerrainIterator; // deprecated
%include "OgreTerrainGroup.h"

%include "OgreTerrainLodManager.h"
%include "OgreTerrainPrerequisites.h"
