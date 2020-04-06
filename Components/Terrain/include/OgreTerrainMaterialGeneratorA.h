/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#ifndef __Ogre_TerrainMaterialGeneratorA_H__
#define __Ogre_TerrainMaterialGeneratorA_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreTerrainMaterialGenerator.h"

namespace Ogre
{
    class PSSMShadowCameraSetup;
    class ShaderHelper;

    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Terrain
    *  Some details on the terrain component
    *  @{
    */

    enum TechniqueType
    {
        HIGH_LOD,
        LOW_LOD,
        RENDER_COMPOSITE_MAP
    };

    /** A TerrainMaterialGenerator which can cope with normal mapped, specular mapped
        terrain.
    */
    class _OgreTerrainExport TerrainMaterialGeneratorA : public TerrainMaterialGenerator
    {
    public:
        TerrainMaterialGeneratorA();
        virtual ~TerrainMaterialGeneratorA();

        /** Shader model 2 profile target. 
        */
        class _OgreTerrainExport SM2Profile : public TerrainMaterialGenerator::Profile
        {
        public:
            SM2Profile(TerrainMaterialGenerator* parent, const String& name, const String& desc);
            virtual ~SM2Profile();
            MaterialPtr generate(const Terrain* terrain);
            MaterialPtr generateForCompositeMap(const Terrain* terrain);
            uint8 getMaxLayers(const Terrain* terrain) const;
            void updateParams(const MaterialPtr& mat, const Terrain* terrain);
            void updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain);
            void requestOptions(Terrain* terrain);
            bool isVertexCompressionSupported() const;

            /** Whether to support normal mapping per layer in the shader (default true). 
            */
            bool isLayerNormalMappingEnabled() const  { return mLayerNormalMappingEnabled; }
            /** Whether to support normal mapping per layer in the shader (default true). 
            */
            void setLayerNormalMappingEnabled(bool enabled);
            /** Whether to support parallax mapping per layer in the shader (default true). 
            */
            bool isLayerParallaxMappingEnabled() const  { return mLayerParallaxMappingEnabled; }
            /** Whether to support parallax mapping per layer in the shader (default true). 
            */
            void setLayerParallaxMappingEnabled(bool enabled);
            /** Whether to support specular mapping per layer in the shader (default true). 
            */
            bool isLayerSpecularMappingEnabled() const  { return mLayerSpecularMappingEnabled; }
            /** Whether to support specular mapping per layer in the shader (default true). 
            */
            void setLayerSpecularMappingEnabled(bool enabled);
            /** Whether to support a global colour map over the terrain in the shader,
                if it's present (default true). 
            */
            bool isGlobalColourMapEnabled() const  { return mGlobalColourMapEnabled; }
            /** Whether to support a global colour map over the terrain in the shader,
            if it's present (default true). 
            */
            void setGlobalColourMapEnabled(bool enabled);
            /** Whether to support a light map over the terrain in the shader,
            if it's present (default true). 
            */
            bool isLightmapEnabled() const  { return mLightmapEnabled; }
            /** Whether to support a light map over the terrain in the shader,
            if it's present (default true). 
            */
            void setLightmapEnabled(bool enabled);
            /** Whether to use the composite map to provide a lower LOD technique
                in the distance (default true). 
            */
            bool isCompositeMapEnabled() const  { return mCompositeMapEnabled; }
            /** Whether to use the composite map to provide a lower LOD technique
            in the distance (default true). 
            */
            void setCompositeMapEnabled(bool enabled);
            /** Whether to support dynamic texture shadows received from other 
                objects, on the terrain (default true). 
            */
            bool getReceiveDynamicShadowsEnabled() const  { return mReceiveDynamicShadows; }
            /** Whether to support dynamic texture shadows received from other 
            objects, on the terrain (default true). 
            */
            void setReceiveDynamicShadowsEnabled(bool enabled);

            /** Whether to use PSSM support dynamic texture shadows, and if so the 
                settings to use (default 0). 
            */
            void setReceiveDynamicShadowsPSSM(PSSMShadowCameraSetup* pssmSettings);
            /** Whether to use PSSM support dynamic texture shadows, and if so the 
            settings to use (default 0). 
            */
            PSSMShadowCameraSetup* getReceiveDynamicShadowsPSSM() const { return mPSSM; }
            /** Whether to use depth shadows (default false). 
            */
            void setReceiveDynamicShadowsDepth(bool enabled);
            /** Whether to use depth shadows (default false). 
            */
            bool getReceiveDynamicShadowsDepth() const { return mDepthShadows; }
            /** Whether to use shadows on low LOD material rendering (when using composite map) (default false). 
            */
            void setReceiveDynamicShadowsLowLod(bool enabled);
            /** Whether to use shadows on low LOD material rendering (when using composite map) (default false). 
            */
            bool getReceiveDynamicShadowsLowLod() const { return mLowLodShadows; }

            bool isShadowingEnabled(TechniqueType tt, const Terrain* terrain) const;
        private:
            typedef StringStream stringstream;

            void addTechnique(const MaterialPtr& mat, const Terrain* terrain, TechniqueType tt);

            ShaderHelper* mShaderGen;
            bool mLayerNormalMappingEnabled;
            bool mLayerParallaxMappingEnabled;
            bool mLayerSpecularMappingEnabled;
            bool mGlobalColourMapEnabled;
            bool mLightmapEnabled;
            bool mCompositeMapEnabled;
            bool mReceiveDynamicShadows;
            PSSMShadowCameraSetup* mPSSM;
            bool mDepthShadows;
            bool mLowLodShadows;
        };
    };
    /** @} */
    /** @} */
}

#endif

