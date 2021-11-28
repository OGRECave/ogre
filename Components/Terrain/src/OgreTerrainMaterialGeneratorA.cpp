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
#include "OgreTerrainMaterialShaderHelpers.h"
#include "OgreTextureManager.h"

#include "OgreShaderGenerator.h"
#include "OgreTerrainShaderTransform.h"
#include "OgreShaderExIntegratedPSSM3.h"

#include <fstream>
#include <string>

namespace Ogre
{
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::TerrainMaterialGeneratorA()
    {
        // define the layers
        // We expect terrain textures to have no alpha, so we use the alpha channel
        // in the albedo texture to store specular reflection
        // similarly we double-up the normal and height (for parallax)
        mLayerDecl.samplers.push_back(TerrainLayerSampler("albedo_specular", PF_BYTE_RGBA));
        mLayerDecl.samplers.push_back(TerrainLayerSampler("normal_height", PF_BYTE_RGBA));
        
        mLayerDecl.elements.push_back(
            TerrainLayerSamplerElement(0, TLSS_ALBEDO, 0, 3));
        mLayerDecl.elements.push_back(
            TerrainLayerSamplerElement(0, TLSS_SPECULAR, 3, 1));
        mLayerDecl.elements.push_back(
            TerrainLayerSamplerElement(1, TLSS_NORMAL, 0, 3));
        mLayerDecl.elements.push_back(
            TerrainLayerSamplerElement(1, TLSS_HEIGHT, 3, 1));


        mProfiles.push_back(OGRE_NEW SM2Profile(this, "SM2", "Profile for rendering on Shader Model 2 capable cards"));

        // TODO - check hardware capabilities & use fallbacks if required (more profiles needed)
        setActiveProfile(mProfiles.back());
    }
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::~TerrainMaterialGeneratorA()
    {

    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::SM2Profile::SM2Profile(TerrainMaterialGenerator* parent, const String& name, const String& desc)
        : Profile(parent, name, desc)
        , mShaderGen(0)
        , mLayerNormalMappingEnabled(true)
        , mLayerParallaxMappingEnabled(true)
        , mLayerSpecularMappingEnabled(true)
        , mGlobalColourMapEnabled(true)
        , mLightmapEnabled(true)
        , mCompositeMapEnabled(true)
        , mReceiveDynamicShadows(true)
        , mPSSM(0)
        , mDepthShadows(false)
        , mLowLodShadows(false)
    {
        mShaderGen = OGRE_NEW ShaderHelperGLSL();
    }
    //---------------------------------------------------------------------
    TerrainMaterialGeneratorA::SM2Profile::~SM2Profile()
    {
        OGRE_DELETE mShaderGen;
    }   
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::requestOptions(Terrain* terrain)
    {
        terrain->_setMorphRequired(true);
        terrain->_setNormalMapRequired(true);
        terrain->_setLightMapRequired(mLightmapEnabled, true);
        terrain->_setCompositeMapRequired(mCompositeMapEnabled);
    }
    //---------------------------------------------------------------------
    bool TerrainMaterialGeneratorA::SM2Profile::isVertexCompressionSupported() const
    {
        return true;
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
    void TerrainMaterialGeneratorA::SM2Profile::setLayerSpecularMappingEnabled(bool enabled)
    {
        if (enabled != mLayerSpecularMappingEnabled)
        {
            mLayerSpecularMappingEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::SM2Profile::setGlobalColourMapEnabled(bool enabled)
    {
        if (enabled != mGlobalColourMapEnabled)
        {
            mGlobalColourMapEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::SM2Profile::setLightmapEnabled(bool enabled)
    {
        if (enabled != mLightmapEnabled)
        {
            mLightmapEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::SM2Profile::setCompositeMapEnabled(bool enabled)
    {
        if (enabled != mCompositeMapEnabled)
        {
            mCompositeMapEnabled = enabled;
            mParent->_markChanged();
        }
    }
    //---------------------------------------------------------------------
    void  TerrainMaterialGeneratorA::SM2Profile::setReceiveDynamicShadowsEnabled(bool enabled)
    {
        if (enabled != mReceiveDynamicShadows)
        {
            mReceiveDynamicShadows = enabled;
            mParent->_markChanged();
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
    void TerrainMaterialGeneratorA::SM2Profile::setReceiveDynamicShadowsDepth(bool enabled)
    {
        if (enabled != mDepthShadows)
        {
            mDepthShadows = enabled;
            mParent->_markChanged();
        }

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::setReceiveDynamicShadowsLowLod(bool enabled)
    {
        if (enabled != mLowLodShadows)
        {
            mLowLodShadows = enabled;
            mParent->_markChanged();
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
        if (rsc->getNumTextureUnits() < 9)
        {
            setLayerNormalMappingEnabled(false);
            setLayerParallaxMappingEnabled(false);
        }

        addTechnique(mat, terrain, HIGH_LOD);
        updateParams(mat, terrain);

        // LOD
        if(mCompositeMapEnabled)
        {
            static TerrainTransformFactory* factory = nullptr;
            if(!factory)
            {
                factory = new TerrainTransformFactory;
                RTShader::ShaderGenerator::getSingleton().addSubRenderStateFactory(factory);
            }

            Technique* tech = mat->createTechnique();
            tech->setLodIndex(1);

            Pass* pass = tech->createPass();
            TextureUnitState* tu = pass->createTextureUnitState();
            tu->setTexture(terrain->getCompositeMap());
            tu->setTextureAddressingMode(TAM_CLAMP);

            pass->getUserObjectBindings().setUserAny("Terrain", terrain);

            using namespace RTShader;
            auto lod1RenderState = std::make_shared<TargetRenderState>();
            try
            {
                lod1RenderState->link({"TerrainTransform", "FFP_Colour", "FFP_Texturing", "FFP_Fog"}, pass, pass);
                if(isShadowingEnabled(LOW_LOD, terrain))
                {
                    // light count needed to enable PSSM3
                    lod1RenderState->setLightCount(Vector3i(0, 1, 0));
                    auto pssm = ShaderGenerator::getSingleton().createSubRenderState<IntegratedPSSM3>();
                    pssm->setSplitPoints(mPSSM->getSplitPoints());
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

        return mat;
    }
    //---------------------------------------------------------------------
    MaterialPtr TerrainMaterialGeneratorA::SM2Profile::generateForCompositeMap(const Terrain* terrain)
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

        addTechnique(mat, terrain, RENDER_COMPOSITE_MAP);

        updateParamsForCompositeMap(mat, terrain);

        return mat;

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::addTechnique(
        const MaterialPtr& mat, const Terrain* terrain, TechniqueType tt)
    {
        Technique* tech = mat->createTechnique();

        // Only supporting one pass
        Pass* pass = tech->createPass();

        HighLevelGpuProgramPtr vprog = mShaderGen->generateVertexProgram(this, terrain, tt);
        HighLevelGpuProgramPtr fprog = mShaderGen->generateFragmentProgram(this, terrain, tt);

        pass->setGpuProgram(GPT_VERTEX_PROGRAM, vprog);
        pass->setGpuProgram(GPT_FRAGMENT_PROGRAM, fprog);

        SamplerPtr mapSampler = TextureManager::getSingleton().createSampler();
        mapSampler->setAddressingMode(TAM_CLAMP);

        if (tt == HIGH_LOD || tt == RENDER_COMPOSITE_MAP)
        {
            // global normal map
            TextureUnitState* tu = pass->createTextureUnitState();
            tu->setTexture(terrain->getTerrainNormalMap());
            tu->setSampler(mapSampler);

            // global colour map
            if (terrain->getGlobalColourMapEnabled() && isGlobalColourMapEnabled())
            {
                tu = pass->createTextureUnitState();
                tu->setTexture(terrain->getGlobalColourMap());
                tu->setSampler(mapSampler);
            }

            // light map
            if (isLightmapEnabled())
            {
                tu = pass->createTextureUnitState();
                tu->setTexture(terrain->getLightmap());
                tu->setSampler(mapSampler);
            }

            // blend maps
            uint maxLayers = getMaxLayers(terrain);
            uint numBlendTextures = std::min<uint8>(Terrain::getBlendTextureCount(maxLayers),
                                                    terrain->getBlendTextures().size());
            uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
            for (uint i = 0; i < numBlendTextures; ++i)
            {
                tu = pass->createTextureUnitState();
                tu->setTexture(terrain->getBlendTextures()[i]);
                tu->setSampler(mapSampler);
            }

            // layer textures
            for (uint i = 0; i < numLayers; ++i)
            {
                // diffuse / specular
                pass->createTextureUnitState(terrain->getLayerTextureName(i, 0));
                // normal / height
                if(mLayerNormalMappingEnabled)
                    pass->createTextureUnitState(terrain->getLayerTextureName(i, 1));
            }

        }

        // Add shadow textures (always at the end)
        if (isShadowingEnabled(tt, terrain))
        {
            uint numTextures = 1;
            if (getReceiveDynamicShadowsPSSM())
            {
                numTextures = (uint)getReceiveDynamicShadowsPSSM()->getSplitCount();
            }
            for (uint i = 0; i < numTextures; ++i)
            {
                TextureUnitState* tu = pass->createTextureUnitState();
                tu->setContentType(TextureUnitState::CONTENT_SHADOW);
                tu->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
                tu->setTextureBorderColour(ColourValue::White);
            }
        }

    }
    //---------------------------------------------------------------------
    bool TerrainMaterialGeneratorA::SM2Profile::isShadowingEnabled(TechniqueType tt, const Terrain* terrain) const
    {
        return getReceiveDynamicShadowsEnabled() && tt != RENDER_COMPOSITE_MAP && 
            (tt != LOW_LOD || mLowLodShadows) &&
            terrain->getSceneManager()->isShadowTechniqueTextureBased();

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::updateParams(const MaterialPtr& mat, const Terrain* terrain)
    {
        Pass* p = mat->getTechnique(0)->getPass(0);
        mShaderGen->updateVpParams(this, terrain, HIGH_LOD, p->getVertexProgramParameters());
        mShaderGen->updateFpParams(this, terrain, HIGH_LOD, p->getFragmentProgramParameters());
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain)
    {
        Pass* p = mat->getTechnique(0)->getPass(0);
        mShaderGen->updateVpParams(this, terrain, RENDER_COMPOSITE_MAP, p->getVertexProgramParameters());
        mShaderGen->updateFpParams(this, terrain, RENDER_COMPOSITE_MAP, p->getFragmentProgramParameters());
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr 
        ShaderHelper::generateVertexProgram(
            const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        HighLevelGpuProgramPtr ret = createVertexProgram(prof, terrain, tt);

        StringStream sourceStr;
        generateVertexProgramSource(prof, terrain, tt, sourceStr);

        ret->setSource(sourceStr.str());
        ret->load();
        defaultVpParams(prof, terrain, tt, ret);

#if OGRE_DEBUG_MODE
        LogManager::getSingleton().stream(LML_TRIVIAL) << "*** Terrain Vertex Program: " 
            << ret->getName() << " ***\n" << ret->getSource() << "\n***   ***";
#endif
        return ret;

    }
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr 
    ShaderHelper::generateFragmentProgram(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        HighLevelGpuProgramPtr ret = createFragmentProgram(prof, terrain, tt);

        StringStream sourceStr;
        generateFragmentProgramSource(prof, terrain, tt, sourceStr);
        ret->setSource(sourceStr.str());
        ret->load();
        defaultFpParams(prof, terrain, tt, ret);

#if OGRE_DEBUG_MODE
        LogManager::getSingleton().stream(LML_TRIVIAL) << "*** Terrain Fragment Program: " 
            << ret->getName() << " ***\n" << ret->getSource() << "\n*** ***";
#endif
        return ret;
    }
    //---------------------------------------------------------------------
    void ShaderHelper::generateFragmentProgramSource(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {
        generateFpHeader(prof, terrain, tt, outStream);

        if (tt != LOW_LOD)
        {
            uint maxLayers = prof->getMaxLayers(terrain);
            uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));

            for (uint i = 0; i < numLayers; ++i)
                generateFpLayer(prof, terrain, tt, i, outStream);
        }

        generateFpFooter(prof, terrain, tt, outStream);
    }
    //---------------------------------------------------------------------
    void ShaderHelper::defaultVpParams(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const HighLevelGpuProgramPtr& prog)
    {
        GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
        params->setIgnoreMissingParams(true);
        params->setNamedAutoConstant("worldMatrix", GpuProgramParameters::ACT_WORLD_MATRIX);
        params->setNamedAutoConstant("viewProjMatrix", GpuProgramParameters::ACT_VIEWPROJ_MATRIX);
        params->setNamedAutoConstant("lodMorph", GpuProgramParameters::ACT_CUSTOM, 
            Terrain::LOD_MORPH_CUSTOM_PARAM);
        params->setNamedAutoConstant("fogParams", GpuProgramParameters::ACT_FOG_PARAMS);

        if (prof->isShadowingEnabled(tt, terrain))
        {
            uint numTextures = 1;
            if (prof->getReceiveDynamicShadowsPSSM())
            {
                numTextures = (uint)prof->getReceiveDynamicShadowsPSSM()->getSplitCount();
            }
            for (uint i = 0; i < numTextures; ++i)
            {
                params->setNamedAutoConstant("texViewProjMatrix" + StringConverter::toString(i), 
                    GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, i);
            }
        }

        if (terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP)
        {
            params->setNamedConstant("posIndexToObjectSpace", terrain->getPointTransform());
        }

        
        
    }
    //---------------------------------------------------------------------
    void ShaderHelper::defaultFpParams(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const HighLevelGpuProgramPtr& prog)
    {
        GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
        params->setIgnoreMissingParams(true);

        params->setNamedAutoConstant("ambient", GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
        params->setNamedAutoConstant("lightPosObjSpace", GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE, 0);
        params->setNamedAutoConstant("lightDiffuseColour", GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
        params->setNamedAutoConstant("lightSpecularColour", GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
        params->setNamedAutoConstant("eyePosObjSpace", GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);
        params->setNamedAutoConstant("fogColour", GpuProgramParameters::ACT_FOG_COLOUR);

        if (prof->isShadowingEnabled(tt, terrain))
        {
            uint numTextures = 1;
            if (prof->getReceiveDynamicShadowsPSSM())
            {
                PSSMShadowCameraSetup* pssm = prof->getReceiveDynamicShadowsPSSM();
                numTextures = (uint)pssm->getSplitCount();
                Vector4 splitPoints;
                const PSSMShadowCameraSetup::SplitPointList& splitPointList = pssm->getSplitPoints();
                // Populate from split point 1, not 0, since split 0 isn't useful (usually 0)
                for (uint i = 1; i < numTextures; ++i)
                {
                    splitPoints[i-1] = splitPointList[i];
                }
                splitPoints[3] = splitPointList.back();
                params->setNamedConstant("pssmSplitPoints", splitPoints);
            }

            if (prof->getReceiveDynamicShadowsDepth())
            {
                uint32 samplerOffset = mShadowSamplerStartHi;
                for (uint i = 0; i < numTextures; ++i)
                {
                    params->setNamedAutoConstant("inverseShadowmapSize" + StringConverter::toString(i), 
                        GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE, i + samplerOffset);
                }
            }
        }

        // Explicitly bind samplers for GLSL
        if (mLang == "glsl" || mLang == "glsles")
        {
            int numSamplers = 0;
            if (tt == LOW_LOD)
            {
                params->setNamedConstant("compositeMap", (int)numSamplers++);
            }
            else
            {
                params->setNamedConstant("globalNormal", (int)numSamplers++);

                if (terrain->getGlobalColourMapEnabled() && prof->isGlobalColourMapEnabled())
                {
                    params->setNamedConstant("globalColourMap", (int)numSamplers++);
                }
                if (prof->isLightmapEnabled())
                {
                    params->setNamedConstant("lightMap", (int)numSamplers++);
                }

                uint maxLayers = prof->getMaxLayers(terrain);
                uint numBlendTextures = std::min<uint8>(Terrain::getBlendTextureCount(maxLayers),
                                                        terrain->getBlendTextures().size());
                uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
                // Blend textures - sampler definitions
                for (uint i = 0; i < numBlendTextures; ++i)
                {
                    params->setNamedConstant("blendTex" + StringConverter::toString(i), (int)numSamplers++);
                }

                // Layer textures - sampler definitions & UV multipliers
                for (uint i = 0; i < numLayers; ++i)
                {
                    params->setNamedConstant("difftex" + StringConverter::toString(i), (int)numSamplers++);
                    if(prof->isLayerNormalMappingEnabled())
                        params->setNamedConstant("normtex" + StringConverter::toString(i), (int)numSamplers++);
                }
            }

            if (prof->isShadowingEnabled(tt, terrain))
            {
                uint numShadowTextures = 1;
                if (prof->getReceiveDynamicShadowsPSSM())
                    numShadowTextures = (uint)prof->getReceiveDynamicShadowsPSSM()->getSplitCount();

                for (uint i = 0; i < numShadowTextures; ++i)
                {
                    if (prof->isShadowingEnabled(tt, terrain))
                        params->setNamedConstant("shadowMap" + StringConverter::toString(i), (int)numSamplers++);
                }
            }
        }
    }
    //---------------------------------------------------------------------
    void ShaderHelper::updateVpParams(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const GpuProgramParametersSharedPtr& params)
    {
        params->setIgnoreMissingParams(true);

        if (terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP)
        {
            Real baseUVScale = 1.0f / (terrain->getSize() - 1);
            params->setNamedConstant("baseUVScale", baseUVScale);
        }
    }
    //---------------------------------------------------------------------
    void ShaderHelper::updateFpParams(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const GpuProgramParametersSharedPtr& params)
    {
        params->setIgnoreMissingParams(true);

        uint maxLayers = prof->getMaxLayers(terrain);
        uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
        uint numUVMul = (numLayers + 3) / 4;
        for (uint i = 0; i < numUVMul; ++i)
        {
            Vector4 uvMul(
                terrain->getLayerUVMultiplier(i * 4), 
                terrain->getLayerUVMultiplier(i * 4 + 1), 
                terrain->getLayerUVMultiplier(i * 4 + 2), 
                terrain->getLayerUVMultiplier(i * 4 + 3) 
                );
            params->setNamedConstant("uvMul_" + StringConverter::toString(i), uvMul);
        }

        // TODO - parameterise this?
        Vector4 scaleBiasSpecular(0.03, -0.04, 32, 1);
        params->setNamedConstant("scaleBiasSpecular", scaleBiasSpecular);

    }
    //---------------------------------------------------------------------
    const char* ShaderHelper::getChannel(uint idx)
    {
        uint rem = idx % 4;
        switch(rem)
        {
        case 0:
        default:
            return "r";
        case 1:
            return "g";
        case 2:
            return "b";
        case 3:
            return "a";
        };
    }
    //---------------------------------------------------------------------
    String ShaderHelper::getVertexProgramName(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {
        String progName = terrain->getMaterialName() + "/sm2/vp";
        
        switch(tt)
        {
        case HIGH_LOD:
            progName += "/hlod";
            break;
        case LOW_LOD:
            progName += "/llod";
            break;
        case RENDER_COMPOSITE_MAP:
            progName += "/comp";
            break;
        }

        return progName;

    }
    //---------------------------------------------------------------------
    String ShaderHelper::getFragmentProgramName(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt)
    {

        String progName = terrain->getMaterialName() + "/sm2/fp";

        switch(tt)
        {
        case HIGH_LOD:
            progName += "/hlod";
            break;
        case LOW_LOD:
            progName += "/llod";
            break;
        case RENDER_COMPOSITE_MAP:
            progName += "/comp";
            break;
        }

        return progName;
    }
}
