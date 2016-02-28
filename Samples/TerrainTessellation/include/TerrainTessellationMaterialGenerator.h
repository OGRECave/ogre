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

#ifndef CUSTOMTERRAINMATERIAL_H
#define CUSTOMTERRAINMATERIAL_H

#include "Ogre.h"
#include "OgreTerrain.h"
#include "OgreTerrainMaterialGenerator.h"
#include "OgreTerrainPrerequisites.h"
#include "OgreGpuProgramParams.h"

using namespace Ogre;

class TerrainTessellationMaterialGenerator : public TerrainMaterialGenerator
{
public:
      
    TerrainTessellationMaterialGenerator(Ogre::String materialName, bool addNormalmap=true, bool cloneMaterial=true);
      
    void setMaterialByName(const Ogre::String materialName); 
    void addNormalMapOnGenerate(bool set) { mAddNormalMap=set; };
    void cloneMaterialOnGenerate(bool set) { mCloneMaterial=set; };

    Ogre::String getMaterialName() { return mMaterialName; };
        
    class Profile : public TerrainMaterialGenerator::Profile
    {
    public:
        Profile(Ogre::TerrainMaterialGenerator* parent, const Ogre::String& name, const Ogre::String& desc);
        ~Profile();

        bool isVertexCompressionSupported() const { return false; }

        MaterialPtr generate(const Terrain* terrain);

        MaterialPtr generateForCompositeMap(const Terrain* terrain);

        Ogre::uint8 getMaxLayers(const Terrain* terrain) const;

        void updateParams(const MaterialPtr& mat, const Terrain* terrain);

        void updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain);

        void requestOptions(Terrain* terrain);

        bool isLightmapEnabled() const  { return mLightmapEnabled; }
        /** Whether to support a light map over the terrain in the shader,
        if it's present (default true). 
        */
        void setLightmapEnabled(bool enabled);

        bool mLightmapEnabled;
    };
protected:         
    Ogre::String mMaterialName; 
    bool mCloneMaterial;
    bool mAddNormalMap;
};
    
#endif