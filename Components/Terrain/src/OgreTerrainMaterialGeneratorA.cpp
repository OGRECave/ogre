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
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreTerrain.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgramManager.h"
#include "OgreShadowCameraSetupPSSM.h"
#include "OgreLogManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreTextureManager.h"

#include "OgreShaderGenerator.h"
#include "OgreTerrainRTShaderSRS.h"

#include <fstream>
#include <string>

namespace Ogre
{
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::TerrainMaterialGeneratorA() :
        mLightmapEnabled(true),
        mCompositeMapEnabled(true),
        mReceiveDynamicShadows(true),
        mLowLodShadows(false)
    {
        // define the layers
        // We expect terrain textures to have no alpha, so we use the alpha channel
        // in the albedo texture to store specular reflection
        // similarly we double-up the normal and height (for parallax)
        mLayerDecl = {{"albedo_specular", PF_BYTE_RGBA}, {"normal_height", PF_BYTE_RGBA}};

        mActiveProfile.reset(new SM2Profile(this));

        using namespace RTShader;

        if (!ShaderGenerator::getSingletonPtr())
        {
            LogManager::getSingleton().logError(
                "TerrainMaterialGeneratorA - Shader generation not possible: RTSS is not initialized.");
            return;
        }

        static SubRenderStateFactory* factory = nullptr;
        if(!factory)
        {
            factory = new TerrainTransformFactory;
            ShaderGenerator::getSingleton().addSubRenderStateFactory(factory);
            factory = new TerrainSurfaceFactory;
            ShaderGenerator::getSingleton().addSubRenderStateFactory(factory);
        }

        mMainRenderState.reset(new RenderState());
        mMainRenderState->setLightCount(1);
        mMainRenderState->addTemplateSubRenderStates(
            {"TerrainTransform", "TerrainSurface", SRS_PER_PIXEL_LIGHTING, SRS_FOG});
    }
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::~TerrainMaterialGeneratorA()
    {

    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::SM2Profile::SM2Profile(TerrainMaterialGeneratorA* parent)
        : mParent(parent)
        , mLayerNormalMappingEnabled(true)
        , mLayerParallaxMappingEnabled(true)
        , mLayerParallaxOcclusionMappingEnabled(true)
        , mLayerSpecularMappingEnabled(true)
        , mPSSM(0)
    {
    }
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::SM2Profile::~SM2Profile()
    {
    }   
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::requestOptions(Terrain* terrain)
    {
        terrain->_setMorphRequired(true);
        terrain->_setNormalMapRequired(true);
        terrain->_setLightMapRequired(mLightmapEnabled, true);
        terrain->_setCompositeMapRequired(mCompositeMapEnabled);
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::setLayerNormalMappingEnabled(bool enabled)
    {
        if (enabled != mLayerNormalMappingEnabled)
        {
            mLayerNormalMappingEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::setLayerParallaxMappingEnabled(bool enabled)
    {
        if (enabled != mLayerParallaxMappingEnabled)
        {
            mLayerParallaxMappingEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::setLayerParallaxOcclusionMappingEnabled(bool enabled)
    {
        if (enabled != mLayerParallaxOcclusionMappingEnabled)
        {
            mLayerParallaxOcclusionMappingEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::setLayerSpecularMappingEnabled(bool enabled)
    {
        if (enabled != mLayerSpecularMappingEnabled)
        {
            mLayerSpecularMappingEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::setLightmapEnabled(bool enabled)
    {
        if (enabled != mLightmapEnabled)
        {
            mLightmapEnabled = enabled;
            _markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::setCompositeMapEnabled(bool enabled)
    {
        if (enabled != mCompositeMapEnabled)
        {
            mCompositeMapEnabled = enabled;
            _markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::setReceiveDynamicShadowsEnabled(bool enabled)
    {
        if (enabled != mReceiveDynamicShadows)
        {
            mReceiveDynamicShadows = enabled;
            _markChanged();
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::setReceiveDynamicShadowsPSSM(PSSMShadowCameraSetup* pssmSettings)
    {
        if (pssmSettings != mPSSM)
        {
            mPSSM = pssmSettings;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::setReceiveDynamicShadowsLowLod(bool enabled)
    {
        if (enabled != mLowLodShadows)
        {
            mLowLodShadows = enabled;
            _markChanged();
        }
    }
    //---------------------------------------------------------------------
    uint8 TerrainMaterialGeneratorA::SM2Profile::getMaxLayers(const Terrain* terrain) const
    {
        // count the texture units free
        uint8 freeTextureUnits = 16;
        // lightmap
        --freeTextureUnits;
        // normalmap
        --freeTextureUnits;
        // colourmap
        if (terrain->getGlobalColourMapEnabled())
            --freeTextureUnits;
        if (isShadowingEnabled(HIGH_LOD, terrain))
        {
            uint8 numShadowTextures = 1;
            if (getReceiveDynamicShadowsPSSM())
            {
                numShadowTextures = (uint8)getReceiveDynamicShadowsPSSM()->getSplitCount();
            }
            freeTextureUnits -= numShadowTextures;
        }

        // each layer needs 2.25 units (1xdiffusespec, 1xnormalheight, 0.25xblend)
        return static_cast<uint8>(freeTextureUnits / 2.25f);
        

    }
    //---------------------------------------------------------------------
    static int getRequiredLayers(const Terrain* terrain, bool pssm)
    {
        int layers = terrain->getLayerCount(); // layer diffusespec
        if (!terrain->getLayerTextureName(0, 1).empty())
            layers *= 2; // per layer normalheight
        layers += 1; // global normal
        layers += bool(terrain->getGlobalColourMap());
        layers += bool(terrain->getLightmap());
        layers += terrain->getBlendTextures().size();

        if(pssm)
            layers += 3; // 3 shadow textures

        return layers;
    }
    MaterialPtr TerrainMaterialGeneratorA::SM2Profile::generate(const Terrain* terrain)
    {
        // re-use old material if exists
        MaterialPtr mat = terrain->_getMaterial();
        if (!mat)
        {
            MaterialManager& matMgr = MaterialManager::getSingleton();
            const String& matName = terrain->getMaterialName();
            mat = matMgr.getByName(matName);
            if (!mat)
            {
                mat = matMgr.create(matName, terrain->_getDerivedResourceGroup());
            }
        }
        // clear everything
        mat->removeAllTechniques();
        
        // Automatically disable normal & parallax mapping if card cannot handle it
        // We do this rather than having a specific technique for it since it's simpler
        auto rsc = Root::getSingletonPtr()->getRenderSystem()->getCapabilities();
        if (getRequiredLayers(terrain, mPSSM) > rsc->getNumTextureUnits())
        {
            setLayerNormalMappingEnabled(false);
            setLayerParallaxMappingEnabled(false);
            LogManager::getSingleton().logWarning(
                "TerrainMaterialGeneratorA: Normal mapping disabled due to lack of texture units");
        }

        Pass* pass;
        pass = mat->createTechnique()->createPass();
        pass->getUserObjectBindings().setUserAny("Terrain", terrain);
        pass->setSpecular(ColourValue::White);
        pass->setShininess(32); // user param

        if(mLayerSpecularMappingEnabled)
        {
            // we use this to inject our specular map
            pass->setVertexColourTracking(TVC_SPECULAR);
        }

        using namespace RTShader;
        auto mainRenderState = std::make_shared<TargetRenderState>();
        auto tplRS = static_cast<TerrainMaterialGeneratorA*>(mParent)->getMainRenderState();
        mainRenderState->setLightCount(tplRS->getLightCount());
        mainRenderState->setHaveAreaLights(tplRS->haveAreaLights());

        if(auto surface = tplRS->getSubRenderState("TerrainSurface"))
            surface->setParameter("use_normal_mapping", std::to_string(mLayerNormalMappingEnabled));

        try
        {
            mainRenderState->link(*tplRS, pass, pass);
            auto surface = mainRenderState->getSubRenderState("TerrainSurface");
            OgreAssert(surface, "TerrainSurface SubRenderState not found");
            surface->setParameter("use_parallax_mapping", std::to_string(mLayerParallaxMappingEnabled));
            surface->setParameter("use_parallax_occlusion_mapping", std::to_string(mLayerParallaxOcclusionMappingEnabled));
            surface->setParameter("use_specular_mapping", std::to_string(mLayerSpecularMappingEnabled));
            if(isShadowingEnabled(HIGH_LOD, terrain))
            {
                auto pssm = ShaderGenerator::getSingleton().createSubRenderState(SRS_SHADOW_MAPPING);
                if(mPSSM)
                    pssm->setParameter("split_points", mPSSM->getSplitPoints());
                pssm->preAddToRenderState(mainRenderState.get(), pass, pass);
                mainRenderState->addSubRenderStateInstance(pssm);
            }
            mainRenderState->acquirePrograms(pass);
        }
        catch(const std::exception& e)
        {
            LogManager::getSingleton().logError(e.what());
            return nullptr;
        }

        pass->getUserObjectBindings().setUserAny(TargetRenderState::UserKey, mainRenderState);

        // LOD
        if(mParent->isCompositeMapEnabled())
        {
            Technique* tech = mat->createTechnique();
            tech->setLodIndex(1);

            pass = tech->createPass();
            TextureUnitState* tu = pass->createTextureUnitState();
            tu->setTexture(terrain->getCompositeMap());
            tu->setTextureAddressingMode(TAM_CLAMP);

            pass->getUserObjectBindings().setUserAny("Terrain", terrain);

            auto lod1RenderState = std::make_shared<TargetRenderState>();
            try
            {
                lod1RenderState->link({"TerrainTransform", SRS_VERTEX_COLOUR, SRS_TEXTURING, SRS_FOG}, pass, pass);
                if (isShadowingEnabled(LOW_LOD, terrain))
                {
                    // light count needed to enable PSSM3
                    lod1RenderState->setLightCount(1);
                    auto pssm = ShaderGenerator::getSingleton().createSubRenderState(SRS_SHADOW_MAPPING);
                    if(mPSSM)
                        pssm->setParameter("split_points", mPSSM->getSplitPoints());
                    pssm->preAddToRenderState(lod1RenderState.get(), pass, pass);
                    lod1RenderState->addSubRenderStateInstance(pssm);
                }
                lod1RenderState->acquirePrograms(pass);
            }
            catch(const Exception& e)
            {
                LogManager::getSingleton().logError(e.what());
                return nullptr;
            }

            pass->getUserObjectBindings().setUserAny(TargetRenderState::UserKey, lod1RenderState);

            mat->setLodLevels({TerrainGlobalOptions::getSingleton().getCompositeMapDistance()});
        }

        mParent->updateParams(mat, terrain);

        return mat;
    }
    //---------------------------------------------------------------------
    MaterialPtr TerrainMaterialGeneratorA::generateForCompositeMap(const Terrain* terrain)
    {
        // re-use old material if exists
        MaterialPtr mat = terrain->_getCompositeMapMaterial();
        if (!mat)
        {
            MaterialManager& matMgr = MaterialManager::getSingleton();

            // it's important that the names are deterministic for a given terrain, so
            // use the terrain pointer as an ID
            const String& matName = terrain->getMaterialName() + "/comp";
            mat = matMgr.getByName(matName);
            if (!mat)
            {
                mat = matMgr.create(matName, terrain->_getDerivedResourceGroup());
            }
        }
        // clear everything
        mat->removeAllTechniques();

        using namespace RTShader;
        auto pass = mat->createTechnique()->createPass();
        pass->getUserObjectBindings().setUserAny("Terrain", terrain);

        auto compRenderState = std::make_shared<TargetRenderState>();
        compRenderState->setLightCount(1);

        try
        {
            compRenderState->link({SRS_TRANSFORM, "TerrainSurface", SRS_PER_PIXEL_LIGHTING}, pass, pass);
            auto terrainSurface = compRenderState->getSubRenderState("TerrainSurface");
            terrainSurface->setParameter("for_composite_map", "true");
            compRenderState->acquirePrograms(pass);
            terrainSurface->updateGpuProgramsParams(NULL, NULL, NULL, NULL); // composite map scene manager not registered
        }
        catch(const std::exception& e)
        {
            LogManager::getSingleton().logError(e.what());
            return nullptr;
        }

        pass->getUserObjectBindings().setUserAny(TargetRenderState::UserKey, compRenderState);
        updateParamsForCompositeMap(mat, terrain);

        return mat;

    }
    //---------------------------------------------------------------------
    bool TerrainMaterialGeneratorA::SM2Profile::isShadowingEnabled(TechniqueType tt, const Terrain* terrain) const
    {
        return mParent->getReceiveDynamicShadowsEnabled() && tt != RENDER_COMPOSITE_MAP &&
               (tt != LOW_LOD || mParent->getReceiveDynamicShadowsLowLod()) &&
               terrain->getSceneManager()->isShadowTechniqueTextureBased();
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::updateParams(const MaterialPtr& mat, const Terrain* terrain)
    {
        using namespace RTShader;
        auto mainRenderState = any_cast<TargetRenderStatePtr>(
            mat->getTechnique(0)->getPass(0)->getUserObjectBindings().getUserAny(TargetRenderState::UserKey));

        for (auto srs : mainRenderState->getSubRenderStates())
        {
            if (auto transform = dynamic_cast<TerrainTransform*>(srs))
            {
                transform->updateParams();
            }
            if (auto surface = dynamic_cast<TerrainSurface*>(srs))
            {
                surface->updateParams();
            }
        }

        if(!isCompositeMapEnabled())
            return;

        auto lod1RenderState = any_cast<TargetRenderStatePtr>(
            mat->getTechnique(1)->getPass(0)->getUserObjectBindings().getUserAny(TargetRenderState::UserKey));
        if (auto transform = lod1RenderState->getSubRenderState("TerrainTransform"))
        {
            static_cast<TerrainTransform*>(transform)->updateParams();
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain)
    {
        using namespace RTShader;
        auto mainRenderState = any_cast<TargetRenderStatePtr>(
            mat->getTechnique(0)->getPass(0)->getUserObjectBindings().getUserAny(TargetRenderState::UserKey));

        if (auto surface = mainRenderState->getSubRenderState("TerrainSurface"))
        {
            static_cast<TerrainSurface*>(surface)->updateParams();
        }
    }
}
