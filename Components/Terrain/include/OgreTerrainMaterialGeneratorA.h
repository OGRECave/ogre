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

    namespace RTShader
    {
    class RenderState;
    }

    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Terrain
    *  @{
    */

    /** A TerrainMaterialGenerator which can cope with normal mapped, specular mapped
        terrain.
    */
    class _OgreTerrainExport TerrainMaterialGeneratorA : public TerrainMaterialGenerator
    {
    public:
        class SM2Profile;
    private:
        std::unique_ptr<RTShader::RenderState> mMainRenderState;
        std::unique_ptr<SM2Profile> mActiveProfile;
        bool mLightmapEnabled;
        bool mCompositeMapEnabled;
        bool mReceiveDynamicShadows;
        bool mLowLodShadows;
    public:
        TerrainMaterialGeneratorA();
        virtual ~TerrainMaterialGeneratorA();

        RTShader::RenderState* getMainRenderState() const { return mMainRenderState.get(); }

        /** Shader model 2 profile target. 
        */
        class _OgreTerrainExport SM2Profile : public Profile
        {
        public:
            SM2Profile(TerrainMaterialGeneratorA* parent);
            virtual ~SM2Profile();
            /// Generate / reuse a material for the terrain
            MaterialPtr generate(const Terrain* terrain);
            /// Get the number of layers supported
            uint8 getMaxLayers(const Terrain* terrain) const;
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
            /** Whether to support steep parallax mapping per layer in the shader (default true).
             */
            void setLayerParallaxOcclusionMappingEnabled(bool enabled);
            /** Whether to support steep parallax mapping per layer in the shader (default true).
             */
            bool isLayerOcclusionMappingEnabled() const { return mLayerParallaxOcclusionMappingEnabled; }
            /** Whether to support specular mapping per layer in the shader (default true). 
            */
            bool isLayerSpecularMappingEnabled() const  { return mLayerSpecularMappingEnabled; }
            /** Whether to support specular mapping per layer in the shader (default true). 
            */
            void setLayerSpecularMappingEnabled(bool enabled);


            /** Whether to use PSSM support dynamic texture shadows, and if so the 
                settings to use (default 0). 
            */
            void setReceiveDynamicShadowsPSSM(PSSMShadowCameraSetup* pssmSettings);
            /** Whether to use PSSM support dynamic texture shadows, and if so the 
            settings to use (default 0). 
            */
            PSSMShadowCameraSetup* getReceiveDynamicShadowsPSSM() const { return mPSSM; }

            void setLightmapEnabled(bool enabled) { mParent->setLightmapEnabled(enabled); }
            void setCompositeMapEnabled(bool enabled) { mParent->setCompositeMapEnabled(enabled); }
            void setReceiveDynamicShadowsEnabled(bool enabled) { mParent->setReceiveDynamicShadowsEnabled(enabled); }
            void setReceiveDynamicShadowsLowLod(bool enabled) { mParent->setReceiveDynamicShadowsLowLod(enabled); }
        private:
            enum TechniqueType
            {
                HIGH_LOD,
                LOW_LOD,
                RENDER_COMPOSITE_MAP
            };
            bool isShadowingEnabled(TechniqueType tt, const Terrain* terrain) const;
            TerrainMaterialGeneratorA* mParent;
            bool mLayerNormalMappingEnabled;
            bool mLayerParallaxMappingEnabled;
            bool mLayerParallaxOcclusionMappingEnabled;
            bool mLayerSpecularMappingEnabled;
            PSSMShadowCameraSetup* mPSSM;
        };

        /** Whether to support normal mapping per layer in the shader (default true).
        */
        void setLayerNormalMappingEnabled(bool enabled) { mActiveProfile->setLayerNormalMappingEnabled(enabled); }

        /** Whether to support specular mapping per layer in the shader (default true).
        */
        void setLayerSpecularMappingEnabled(bool enabled) { mActiveProfile->setLayerSpecularMappingEnabled(enabled); }

        /** Whether to support dynamic texture shadows received from other
            objects, on the terrain (default true).
        */
        bool getReceiveDynamicShadowsEnabled() const  { return mReceiveDynamicShadows; }
        /** Whether to support dynamic texture shadows received from other
        objects, on the terrain (default true).
        */
        void setReceiveDynamicShadowsEnabled(bool enabled);

        /** Whether to use shadows on low LOD material rendering (when using composite map) (default false).
        */
        void setReceiveDynamicShadowsLowLod(bool enabled);

        /** Whether to use shadows on low LOD material rendering (when using composite map) (default false).
        */
        bool getReceiveDynamicShadowsLowLod() const { return mLowLodShadows; }

        bool isLightmapEnabled() const  { return mLightmapEnabled; }
        /** Whether to use the composite map to provide a lower LOD technique
        in the distance (default true).
        */
        void setCompositeMapEnabled(bool enabled);
        bool isCompositeMapEnabled() const  { return mCompositeMapEnabled; }

        /// Get the active profile
        Profile* getActiveProfile() const override { return mActiveProfile.get(); }

        bool isVertexCompressionSupported() const override { return true; }
        void requestOptions(Terrain* terrain) override;
        MaterialPtr generate(const Terrain* terrain) override { return mActiveProfile->generate(terrain); }
        MaterialPtr generateForCompositeMap(const Terrain* terrain) override;
        void setLightmapEnabled(bool enabled) override;
        uint8 getMaxLayers(const Terrain* terrain) const override { return mActiveProfile->getMaxLayers(terrain); }
        void updateParams(const MaterialPtr& mat, const Terrain* terrain) override;
        void updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain) override;
    };
    /** @} */
    /** @} */
}

#endif

