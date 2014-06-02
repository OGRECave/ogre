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
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreShadowCameraSetupPSSM.h"
#include "OgreLogManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
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
        setActiveProfile("SM2");

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
        , mSM3Available(false)
        , mSM4Available(false)
    {
        HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();
        if (hmgr.isLanguageSupported("hlsl"))
        {
            mShaderLanguage = "hlsl";
        }
        else if (hmgr.isLanguageSupported("glsl")
                 && Root::getSingleton().getRenderSystem()->getNativeShadingLanguageVersion() >= 150)
        {
            mShaderLanguage = "glsl";
        }
        else if (hmgr.isLanguageSupported("glsles"))
        {
            mShaderLanguage = "glsles";
        }
        else if (hmgr.isLanguageSupported("cg"))
        {
            mShaderLanguage = "cg";
        }
        else
        {
            // todo
        }
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
        // FIXME: Not supporting compression on GLSL at the moment
        if ((mShaderLanguage == "glsl") || (mShaderLanguage == "glsles"))
            return false;
        else
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
        if (mat.isNull())
        {
            MaterialManager& matMgr = MaterialManager::getSingleton();

            // it's important that the names are deterministic for a given terrain, so
            // use the terrain pointer as an ID
            const String& matName = terrain->getMaterialName();
            mat = matMgr.getByName(matName);
            if (mat.isNull())
            {
                mat = matMgr.create(matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            }
        }
        // clear everything
        mat->removeAllTechniques();
        
        // Automatically disable normal & parallax mapping if card cannot handle it
        // We do this rather than having a specific technique for it since it's simpler
        GpuProgramManager& gmgr = GpuProgramManager::getSingleton();
        if (!gmgr.isSyntaxSupported("ps_4_0") && !gmgr.isSyntaxSupported("ps_3_0") && !gmgr.isSyntaxSupported("ps_2_x")
            && !gmgr.isSyntaxSupported("fp40") && !gmgr.isSyntaxSupported("arbfp1") && !gmgr.isSyntaxSupported("glsl")
            && !gmgr.isSyntaxSupported("glsles"))
        {
            setLayerNormalMappingEnabled(false);
            setLayerParallaxMappingEnabled(false);
        }

        addTechnique(mat, terrain, HIGH_LOD);

        // LOD
        if(mCompositeMapEnabled)
        {
            addTechnique(mat, terrain, LOW_LOD);
            Material::LodValueArray lodValues;
            lodValues.push_back(TerrainGlobalOptions::getSingleton().getCompositeMapDistance());
            mat->setLodLevels(lodValues);
            Technique* lowLodTechnique = mat->getTechnique(1);
            lowLodTechnique->setLodIndex(1);
        }

        updateParams(mat, terrain);

        return mat;

    }
    //---------------------------------------------------------------------
    MaterialPtr TerrainMaterialGeneratorA::SM2Profile::generateForCompositeMap(const Terrain* terrain)
    {
        // re-use old material if exists
        MaterialPtr mat = terrain->_getCompositeMapMaterial();
        if (mat.isNull())
        {
            MaterialManager& matMgr = MaterialManager::getSingleton();

            // it's important that the names are deterministic for a given terrain, so
            // use the terrain pointer as an ID
            const String& matName = terrain->getMaterialName() + "/comp";
            mat = matMgr.getByName(matName);
            if (mat.isNull())
            {
                mat = matMgr.create(matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
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

        GpuProgramManager& gmgr = GpuProgramManager::getSingleton();
        HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();
        if (!mShaderGen)
        {
            bool check2x = mLayerNormalMappingEnabled || mLayerParallaxMappingEnabled;
            if (hmgr.isLanguageSupported("hlsl") &&
                ((check2x && gmgr.isSyntaxSupported("ps_4_0")) ))
            {
                mShaderGen = OGRE_NEW ShaderHelperHLSL();
            }
            else if (hmgr.isLanguageSupported("glsl") &&
                     Root::getSingleton().getRenderSystem()->getNativeShadingLanguageVersion() >= 150)
            {
                mShaderGen = OGRE_NEW ShaderHelperGLSL();
            }
            else if (hmgr.isLanguageSupported("glsles"))
            {
                mShaderGen = OGRE_NEW ShaderHelperGLSLES();
            }
            else if (hmgr.isLanguageSupported("cg"))
            {
                mShaderGen = OGRE_NEW ShaderHelperCg();
            }
            
            // check SM3 features
            mSM3Available = GpuProgramManager::getSingleton().isSyntaxSupported("ps_3_0");
            mSM4Available = GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0");
        }

        HighLevelGpuProgramPtr vprog = mShaderGen->generateVertexProgram(this, terrain, tt);
        HighLevelGpuProgramPtr fprog = mShaderGen->generateFragmentProgram(this, terrain, tt);

        pass->setVertexProgram(vprog->getName());
        pass->setFragmentProgram(fprog->getName());

        if (tt == HIGH_LOD || tt == RENDER_COMPOSITE_MAP)
        {
            // global normal map
            TextureUnitState* tu = pass->createTextureUnitState();
            tu->setTextureName(terrain->getTerrainNormalMap()->getName());
            // Bugfix for D3D11 Render System
            // tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

            // global colour map
            if (terrain->getGlobalColourMapEnabled() && isGlobalColourMapEnabled())
            {
                tu = pass->createTextureUnitState(terrain->getGlobalColourMap()->getName());
                //tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            }

            // light map
            if (isLightmapEnabled())
            {
                tu = pass->createTextureUnitState(terrain->getLightmap()->getName());
                //tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            }

            // blend maps
            uint maxLayers = getMaxLayers(terrain);
            uint numBlendTextures = std::min(terrain->getBlendTextureCount(maxLayers), terrain->getBlendTextureCount());
            uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
            for (uint i = 0; i < numBlendTextures; ++i)
            {
                tu = pass->createTextureUnitState(terrain->getBlendTextureName(i));
                //tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            }

            // layer textures
            for (uint i = 0; i < numLayers; ++i)
            {
                // diffuse / specular
                pass->createTextureUnitState(terrain->getLayerTextureName(i, 0));
                // normal / height
                pass->createTextureUnitState(terrain->getLayerTextureName(i, 1));
            }

        }
        else
        {
            // LOW_LOD textures
            // composite map
            TextureUnitState* tu = pass->createTextureUnitState();
            tu->setTextureName(terrain->getCompositeMap()->getName());
            tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

            // That's it!

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
        (tt != LOW_LOD || mLowLodShadows);
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::updateParams(const MaterialPtr& mat, const Terrain* terrain)
    {
        mShaderGen->updateParams(this, mat, terrain, false);

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain)
    {
        mShaderGen->updateParams(this, mat, terrain, true);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    HighLevelGpuProgramPtr 
        TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateVertexProgram(
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
    TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateFragmentProgram(
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
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateVertexProgramSource(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream)
    {
        generateVpHeader(prof, terrain, tt, outStream);

        if (tt != LOW_LOD)
        {
            uint maxLayers = prof->getMaxLayers(terrain);
            uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));

            for (uint i = 0; i < numLayers; ++i)
                generateVpLayer(prof, terrain, tt, i, outStream);
        }

        generateVpFooter(prof, terrain, tt, outStream);

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::generateFragmentProgramSource(
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
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::defaultVpParams(
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
                if (prof->getReceiveDynamicShadowsDepth())
                {
                    params->setNamedAutoConstant("depthRange" + StringConverter::toString(i), 
                        GpuProgramParameters::ACT_SHADOW_SCENE_DEPTH_RANGE, i);
                }
            }
        }

        if (terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP)
        {
            Matrix4 posIndexToObjectSpace;
            terrain->getPointTransform(&posIndexToObjectSpace);
            params->setNamedConstant("posIndexToObjectSpace", posIndexToObjectSpace);
        }

        
        
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::defaultFpParams(
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
                params->setNamedConstant("pssmSplitPoints", splitPoints);
            }

            if (prof->getReceiveDynamicShadowsDepth())
            {
                size_t samplerOffset = (tt == HIGH_LOD) ? mShadowSamplerStartHi : mShadowSamplerStartLo;
                for (uint i = 0; i < numTextures; ++i)
                {
                    params->setNamedAutoConstant("inverseShadowmapSize" + StringConverter::toString(i), 
                        GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE, i + samplerOffset);
                }
            }
        }

        // Explicitly bind samplers for GLSL
        if ((prof->_getShaderLanguage() == "glsl") || (prof->_getShaderLanguage() == "glsles"))
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
                uint numBlendTextures = std::min(terrain->getBlendTextureCount(maxLayers), terrain->getBlendTextureCount());
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
                    params->setNamedConstant("normtex" + StringConverter::toString(i), (int)numSamplers++);
                }

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
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::updateParams(
        const SM2Profile* prof, const MaterialPtr& mat, const Terrain* terrain, bool compositeMap)
    {
        Pass* p = mat->getTechnique(0)->getPass(0);
        if (compositeMap)
        {
            updateVpParams(prof, terrain, RENDER_COMPOSITE_MAP, p->getVertexProgramParameters());
            updateFpParams(prof, terrain, RENDER_COMPOSITE_MAP, p->getFragmentProgramParameters());
        }
        else
        {
            // high lod
            updateVpParams(prof, terrain, HIGH_LOD, p->getVertexProgramParameters());
            updateFpParams(prof, terrain, HIGH_LOD, p->getFragmentProgramParameters());

            if(prof->isCompositeMapEnabled())
            {
                // low lod
                p = mat->getTechnique(1)->getPass(0);
                updateVpParams(prof, terrain, LOW_LOD, p->getVertexProgramParameters());
                updateFpParams(prof, terrain, LOW_LOD, p->getFragmentProgramParameters());
            }
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::updateVpParams(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const GpuProgramParametersSharedPtr& params)
    {
        params->setIgnoreMissingParams(true);
        uint maxLayers = prof->getMaxLayers(terrain);
        uint numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));
        uint numUVMul = numLayers / 4;
        if (numLayers % 4)
            ++numUVMul;
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
        
        if (terrain->_getUseVertexCompression() && tt != RENDER_COMPOSITE_MAP)
        {
            Real baseUVScale = 1.0f / (terrain->getSize() - 1);
            params->setNamedConstant("baseUVScale", baseUVScale);
        }

    }
    //---------------------------------------------------------------------
    void TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::updateFpParams(
        const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const GpuProgramParametersSharedPtr& params)
    {
        params->setIgnoreMissingParams(true);
        // TODO - parameterise this?
        Vector4 scaleBiasSpecular(0.03, -0.04, 32, 1);
        params->setNamedConstant("scaleBiasSpecular", scaleBiasSpecular);

    }
    //---------------------------------------------------------------------
    String TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::getChannel(uint idx)
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
    String TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::getVertexProgramName(
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
    String TerrainMaterialGeneratorA::SM2Profile::ShaderHelper::getFragmentProgramName(
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
