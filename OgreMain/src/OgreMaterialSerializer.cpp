/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreStableHeaders.h"

#include "OgreTextureUnitState.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MaterialSerializer::MaterialSerializer()
    {
        mDefaults = false;
        mBuffer.clear();
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::exportMaterial(const MaterialPtr& pMat, const String &fileName, bool exportDefaults,
        const bool includeProgDef, const String& programFilename, const String& materialName)
    {
        clearQueue();
        mDefaults = exportDefaults;
        writeMaterial(pMat, materialName);
        exportQueued(fileName, includeProgDef, programFilename);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::exportQueued(const String &fileName, const bool includeProgDef, const String& programFilename)
    {
        // write out gpu program definitions to the buffer
        writeGpuPrograms();

        if (mBuffer.empty())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Queue is empty !", "MaterialSerializer::exportQueued");

        LogManager::getSingleton().logMessage("MaterialSerializer : writing material(s) to material script : " + fileName, LML_NORMAL);
        FILE *fp;
        fp = fopen(fileName.c_str(), "w");
        if (!fp)
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create material file.",
            "MaterialSerializer::export");

        // output gpu program definitions to material script file if includeProgDef is true
        if (includeProgDef && !mGpuProgramBuffer.empty())
        {
            fputs(mGpuProgramBuffer.c_str(), fp);
        }

        // output main buffer holding material script
        fputs(mBuffer.c_str(), fp);
        fclose(fp);

        // write program script if program filename and program definitions
        // were not included in material script
        if (!includeProgDef && !mGpuProgramBuffer.empty() && !programFilename.empty())
        {
            FILE *locFp;
            locFp = fopen(programFilename.c_str(), "w");
            if (!locFp)
                OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create program material file.",
                "MaterialSerializer::export");
            fputs(mGpuProgramBuffer.c_str(), locFp);
            fclose(locFp);
        }

        LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_NORMAL);
        clearQueue();
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::queueForExport(const MaterialPtr& pMat,
        bool clearQueued, bool exportDefaults, const String& materialName)
    {
        if (clearQueued)
            clearQueue();

        mDefaults = exportDefaults;
        writeMaterial(pMat, materialName);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::clearQueue()
    {
        mBuffer.clear();
        mGpuProgramBuffer.clear();
        mGpuProgramDefinitionContainer.clear();
    }
    //-----------------------------------------------------------------------
    const String &MaterialSerializer::getQueuedAsString() const
    {
        return mBuffer;
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeMaterial(const MaterialPtr& pMat, const String& materialName)
    {
        String outMaterialName;

        if (materialName.length() > 0)
        {
            outMaterialName = materialName;
        }
        else
        {
            outMaterialName = pMat->getName();
        }

        LogManager::getSingleton().logMessage("MaterialSerializer : writing material " + outMaterialName + " to queue.", LML_NORMAL);

        bool skipWriting = false;

        // Fire pre-write event.
        fireMaterialEvent(MSE_PRE_WRITE, skipWriting, pMat.get());
        if (skipWriting)        
            return;     

        // Material name
        writeAttribute(0, "material");
        writeValue(quoteWord(outMaterialName));
        
        beginSection(0);
        {
            // Fire write begin event.
            fireMaterialEvent(MSE_WRITE_BEGIN, skipWriting, pMat.get());

            // Write LOD information
            Material::LodValueList::const_iterator valueIt = pMat->getUserLodValues().begin();
            // Skip zero value
            if (!pMat->getUserLodValues().empty())
                valueIt++;
            String attributeVal;
            while (valueIt != pMat->getUserLodValues().end())
            {
                attributeVal.append(StringConverter::toString(*valueIt++));
                if (valueIt != pMat->getUserLodValues().end())
                    attributeVal.append(" ");
            }
            if (!attributeVal.empty())
            {
                writeAttribute(1, "lod_values");
                writeValue(attributeVal);
            }


            // Shadow receive
            if (mDefaults ||
                pMat->getReceiveShadows() != true)
            {
                writeAttribute(1, "receive_shadows");
                writeValue(pMat->getReceiveShadows() ? "on" : "off");
            }

            // When rendering shadows, treat transparent things as opaque?
            if (mDefaults ||
                pMat->getTransparencyCastsShadows() == true)
            {
                writeAttribute(1, "transparency_casts_shadows");
                writeValue(pMat->getTransparencyCastsShadows() ? "on" : "off");
            }

            // Iterate over techniques
            for(auto t : pMat->getTechniques())
            {
                // skip RTSS generated techniques
                if(!mDefaults && t->getSchemeName() == "ShaderGeneratorDefaultScheme")
                    continue;
                writeTechnique(t);
                mBuffer += "\n";
            }

            // Fire write end event.
            fireMaterialEvent(MSE_WRITE_END, skipWriting, pMat.get());
        }
        endSection(0);
        mBuffer += "\n";

        // Fire post section write event.
        fireMaterialEvent(MSE_POST_WRITE, skipWriting, pMat.get());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTechnique(const Technique* pTech)
    {
        bool skipWriting = false;

        // Fire pre-write event.
        fireTechniqueEvent(MSE_PRE_WRITE, skipWriting, pTech);
        if (skipWriting)        
            return; 
        
        // Technique header
        writeAttribute(1, "technique");
        // only output technique name if it exists.
        if (!pTech->getName().empty())
            writeValue(quoteWord(pTech->getName()));

        beginSection(1);
        {
            // Fire write begin event.
            fireTechniqueEvent(MSE_WRITE_BEGIN, skipWriting, pTech);

            // LOD index
            if (mDefaults ||
                pTech->getLodIndex() != 0)
            {
                writeAttribute(2, "lod_index");
                writeValue(StringConverter::toString(pTech->getLodIndex()));
            }

            // Scheme name
            if (mDefaults || pTech->getSchemeName() != MSN_DEFAULT)
            {
                writeAttribute(2, "scheme");
                writeValue(quoteWord(pTech->getSchemeName()));
            }

            // ShadowCasterMaterial name
            if (pTech->getShadowCasterMaterial())
            {
                writeAttribute(2, "shadow_caster_material");
                writeValue(quoteWord(pTech->getShadowCasterMaterial()->getName()));
            }
            // ShadowReceiverMaterial name
            if (pTech->getShadowReceiverMaterial())
            {
                writeAttribute(2, "shadow_receiver_material");
                writeValue(quoteWord(pTech->getShadowReceiverMaterial()->getName()));
            }
            // GPU vendor rules
            Technique::GPUVendorRuleList::const_iterator vrit;
            for (vrit = pTech->getGPUVendorRules().begin(); vrit != pTech->getGPUVendorRules().end(); ++vrit)
            {
                const Technique::GPUVendorRule& rule = *vrit;
                writeAttribute(2, "gpu_vendor_rule");
                if (rule.includeOrExclude == Technique::INCLUDE)
                    writeValue("include");
                else
                    writeValue("exclude");
                writeValue(quoteWord(RenderSystemCapabilities::vendorToString(rule.vendor)));
            }
            // GPU device rules
            Technique::GPUDeviceNameRuleList::const_iterator dnit;
            for (dnit = pTech->getGPUDeviceNameRules().begin(); dnit != pTech->getGPUDeviceNameRules().end(); ++dnit)
            {
                const Technique::GPUDeviceNameRule& rule = *dnit;
                writeAttribute(2, "gpu_device_rule");
                if (rule.includeOrExclude == Technique::INCLUDE)
                    writeValue("include");
                else
                    writeValue("exclude");
                writeValue(quoteWord(rule.devicePattern));
                writeValue(StringConverter::toString(rule.caseSensitive));
            }
            // Iterate over passes
            for(auto& p : pTech->getPasses())
            {
                writePass(p);
                mBuffer += "\n";
            }

            // Fire write end event.
            fireTechniqueEvent(MSE_WRITE_END, skipWriting, pTech);
        }
        endSection(1);

        // Fire post section write event.
        fireTechniqueEvent(MSE_POST_WRITE, skipWriting, pTech);

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writePass(const Pass* pPass)
    {
        bool skipWriting = false;

        // Fire pre-write event.
        firePassEvent(MSE_PRE_WRITE, skipWriting, pPass);
        if (skipWriting)        
            return;
        
        writeAttribute(2, "pass");
        // only output pass name if its not the default name
        if (pPass->getName() != StringConverter::toString(pPass->getIndex()))
            writeValue(quoteWord(pPass->getName()));

        beginSection(2);
        {
            // Fire write begin event.
            firePassEvent(MSE_WRITE_BEGIN, skipWriting, pPass);

            //lighting
            if (mDefaults ||
                pPass->getLightingEnabled() != true)
            {
                writeAttribute(3, "lighting");
                writeValue(pPass->getLightingEnabled() ? "on" : "off");
            }
            // max_lights
            if (mDefaults ||
                pPass->getMaxSimultaneousLights() != OGRE_MAX_SIMULTANEOUS_LIGHTS)
            {
                writeAttribute(3, "max_lights");
                writeValue(StringConverter::toString(pPass->getMaxSimultaneousLights()));
            }
            // start_light
            if (mDefaults ||
                pPass->getStartLight() != 0)
            {
                writeAttribute(3, "start_light");
                writeValue(StringConverter::toString(pPass->getStartLight()));
            }
            // iteration
            if (mDefaults ||
                pPass->getIteratePerLight() || (pPass->getPassIterationCount() > 1))
            {
                writeAttribute(3, "iteration");
                // pass iteration count
                if (pPass->getPassIterationCount() > 1 || pPass->getLightCountPerIteration() > 1)
                {
                    writeValue(StringConverter::toString(pPass->getPassIterationCount()));
                    if (pPass->getIteratePerLight())
                    {
                        if (pPass->getLightCountPerIteration() > 1)
                        {
                            writeValue("per_n_lights");
                            writeValue(StringConverter::toString(
                                pPass->getLightCountPerIteration()));
                        }
                        else
                        {
                            writeValue("per_light");
                        }
                    }
                }
                else
                {
                    writeValue(pPass->getIteratePerLight() ? "once_per_light" : "once");
                }

                if (pPass->getIteratePerLight() && pPass->getRunOnlyForOneLightType())
                {
                    switch (pPass->getOnlyLightType())
                    {
                    case Light::LT_DIRECTIONAL:
                        writeValue("directional");
                        break;
                    case Light::LT_POINT:
                        writeValue("point");
                        break;
                    case Light::LT_SPOTLIGHT:
                        writeValue("spot");
                        break;
                    case Light::LT_RECTLIGHT:
                        writeValue("rect");
                        break;
                    };
                }
            }

            if(mDefaults || pPass->getLightMask() != 0xFFFFFFFF)
            {
                writeAttribute(3, "light_mask");
                writeValue(StringConverter::toString(pPass->getLightMask()));
            }

            if (pPass->getLightingEnabled())
            {
                // Ambient
                if (mDefaults ||
                    pPass->getAmbient().r != 1 ||
                    pPass->getAmbient().g != 1 ||
                    pPass->getAmbient().b != 1 ||
                    pPass->getAmbient().a != 1 ||
                    (pPass->getVertexColourTracking() & TVC_AMBIENT))
                {
                    writeAttribute(3, "ambient");
                    if (pPass->getVertexColourTracking() & TVC_AMBIENT)
                        writeValue("vertexcolour");
                    else
                        writeColourValue(pPass->getAmbient(), true);
                }

                // Diffuse
                if (mDefaults ||
                    pPass->getDiffuse().r != 1 ||
                    pPass->getDiffuse().g != 1 ||
                    pPass->getDiffuse().b != 1 ||
                    pPass->getDiffuse().a != 1 ||
                    (pPass->getVertexColourTracking() & TVC_DIFFUSE))
                {
                    writeAttribute(3, "diffuse");
                    if (pPass->getVertexColourTracking() & TVC_DIFFUSE)
                        writeValue("vertexcolour");
                    else
                        writeColourValue(pPass->getDiffuse(), true);
                }

                // Specular
                if (mDefaults ||
                    pPass->getSpecular().r != 0 ||
                    pPass->getSpecular().g != 0 ||
                    pPass->getSpecular().b != 0 ||
                    pPass->getSpecular().a != 1 ||
                    pPass->getShininess() != 0 ||
                    (pPass->getVertexColourTracking() & TVC_SPECULAR))
                {
                    writeAttribute(3, "specular");
                    if (pPass->getVertexColourTracking() & TVC_SPECULAR)
                    {
                        writeValue("vertexcolour");
                    }
                    else
                    {
                        writeColourValue(pPass->getSpecular(), true);
                    }
                    writeValue(StringConverter::toString(pPass->getShininess()));

                }

                // Emissive
                if (mDefaults ||
                    pPass->getSelfIllumination().r != 0 ||
                    pPass->getSelfIllumination().g != 0 ||
                    pPass->getSelfIllumination().b != 0 ||
                    pPass->getSelfIllumination().a != 1 ||
                    (pPass->getVertexColourTracking() & TVC_EMISSIVE))
                {
                    writeAttribute(3, "emissive");
                    if (pPass->getVertexColourTracking() & TVC_EMISSIVE)
                        writeValue("vertexcolour");
                    else
                        writeColourValue(pPass->getSelfIllumination(), true);
                }
            }

            // Point size
            if (mDefaults ||
                pPass->getPointSize() != 1.0)
            {
                writeAttribute(3, "point_size");
                writeValue(StringConverter::toString(pPass->getPointSize()));
            }

            // Point sprites
            if (mDefaults ||
                pPass->getPointSpritesEnabled())
            {
                writeAttribute(3, "point_sprites");
                writeValue(pPass->getPointSpritesEnabled() ? "on" : "off");
            }

            // Point attenuation
            if (mDefaults ||
                pPass->isPointAttenuationEnabled())
            {
                writeAttribute(3, "point_size_attenuation");
                writeValue(pPass->isPointAttenuationEnabled() ? "on" : "off");
                if (pPass->isPointAttenuationEnabled() &&
                    (pPass->getPointAttenuationConstant() != 0.0 ||
                     pPass->getPointAttenuationLinear() != 1.0 ||
                     pPass->getPointAttenuationQuadratic() != 0.0))
                {
                    writeValue(StringConverter::toString(pPass->getPointAttenuationConstant()));
                    writeValue(StringConverter::toString(pPass->getPointAttenuationLinear()));
                    writeValue(StringConverter::toString(pPass->getPointAttenuationQuadratic()));
                }
            }

            // Point min size
            if (mDefaults ||
                pPass->getPointMinSize() != 0.0)
            {
                writeAttribute(3, "point_size_min");
                writeValue(StringConverter::toString(pPass->getPointMinSize()));
            }

            // Point max size
            if (mDefaults ||
                pPass->getPointMaxSize() != 0.0)
            {
                writeAttribute(3, "point_size_max");
                writeValue(StringConverter::toString(pPass->getPointMaxSize()));
            }

            // scene blend factor
            if (mDefaults ||
                pPass->getSourceBlendFactor() != SBF_ONE ||
                pPass->getDestBlendFactor() != SBF_ZERO ||
                pPass->getSourceBlendFactorAlpha() != SBF_ONE ||
                pPass->getDestBlendFactorAlpha() != SBF_ZERO)
            {
                writeAttribute(3, "separate_scene_blend");
                writeSceneBlendFactor(pPass->getSourceBlendFactor(), pPass->getDestBlendFactor(),
                    pPass->getSourceBlendFactorAlpha(), pPass->getDestBlendFactorAlpha());
            }


            //depth check
            if (mDefaults ||
                pPass->getDepthCheckEnabled() != true)
            {
                writeAttribute(3, "depth_check");
                writeValue(pPass->getDepthCheckEnabled() ? "on" : "off");
            }
            // alpha_rejection
            if (mDefaults ||
                pPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS ||
                pPass->getAlphaRejectValue() != 0)
            {
                writeAttribute(3, "alpha_rejection");
                writeCompareFunction(pPass->getAlphaRejectFunction());
                writeValue(StringConverter::toString(pPass->getAlphaRejectValue()));
            }
            // alpha_to_coverage
            if (mDefaults ||
                pPass->isAlphaToCoverageEnabled())
            {
                writeAttribute(3, "alpha_to_coverage");
                writeValue(pPass->isAlphaToCoverageEnabled() ? "on" : "off");
            }
            // transparent_sorting
            if (mDefaults ||
                pPass->getTransparentSortingForced() == true ||
                pPass->getTransparentSortingEnabled() != true)
            {
                writeAttribute(3, "transparent_sorting");
                writeValue(pPass->getTransparentSortingForced() ? "force" :
                    (pPass->getTransparentSortingEnabled() ? "on" : "off"));
            }


            //depth write
            if (mDefaults ||
                pPass->getDepthWriteEnabled() != true)
            {
                writeAttribute(3, "depth_write");
                writeValue(pPass->getDepthWriteEnabled() ? "on" : "off");
            }

            //depth function
            if (mDefaults ||
                pPass->getDepthFunction() != CMPF_LESS_EQUAL)
            {
                writeAttribute(3, "depth_func");
                writeCompareFunction(pPass->getDepthFunction());
            }

            //depth bias
            if (mDefaults ||
                pPass->getDepthBiasConstant() != 0 ||
                pPass->getDepthBiasSlopeScale() != 0)
            {
                writeAttribute(3, "depth_bias");
                writeValue(StringConverter::toString(pPass->getDepthBiasConstant()));
                writeValue(StringConverter::toString(pPass->getDepthBiasSlopeScale()));
            }
            //iteration depth bias
            if (mDefaults ||
                pPass->getIterationDepthBias() != 0)
            {
                writeAttribute(3, "iteration_depth_bias");
                writeValue(StringConverter::toString(pPass->getIterationDepthBias()));
            }

            //light scissor
            if (mDefaults ||
                pPass->getLightScissoringEnabled() != false)
            {
                writeAttribute(3, "light_scissor");
                writeValue(pPass->getLightScissoringEnabled() ? "on" : "off");
            }

            //light clip planes
            if (mDefaults ||
                pPass->getLightClipPlanesEnabled() != false)
            {
                writeAttribute(3, "light_clip_planes");
                writeValue(pPass->getLightClipPlanesEnabled() ? "on" : "off");
            }

            // illumination stage
            if (pPass->getIlluminationStage() != IS_UNKNOWN)
            {
                writeAttribute(3, "illumination_stage");
                switch(pPass->getIlluminationStage())
                {
                case IS_AMBIENT:
                    writeValue("ambient");
                    break;
                case IS_PER_LIGHT:
                    writeValue("per_light");
                    break;
                case IS_DECAL:
                    writeValue("decal");
                    break;
                case IS_UNKNOWN:
                    break;
                };
            }

            // hardware culling mode
            if (mDefaults ||
                pPass->getCullingMode() != CULL_CLOCKWISE)
            {
                CullingMode hcm = pPass->getCullingMode();
                writeAttribute(3, "cull_hardware");
                switch (hcm)
                {
                case CULL_NONE :
                    writeValue("none");
                    break;
                case CULL_CLOCKWISE :
                    writeValue("clockwise");
                    break;
                case CULL_ANTICLOCKWISE :
                    writeValue("anticlockwise");
                    break;
                }
            }

            // software culling mode
            if (mDefaults ||
                pPass->getManualCullingMode() != MANUAL_CULL_BACK)
            {
                ManualCullingMode scm = pPass->getManualCullingMode();
                writeAttribute(3, "cull_software");
                switch (scm)
                {
                case MANUAL_CULL_NONE :
                    writeValue("none");
                    break;
                case MANUAL_CULL_BACK :
                    writeValue("back");
                    break;
                case MANUAL_CULL_FRONT :
                    writeValue("front");
                    break;
                }
            }

            //shading
            if (mDefaults ||
                pPass->getShadingMode() != SO_GOURAUD)
            {
                writeAttribute(3, "shading");
                switch (pPass->getShadingMode())
                {
                case SO_FLAT:
                    writeValue("flat");
                    break;
                case SO_GOURAUD:
                    writeValue("gouraud");
                    break;
                case SO_PHONG:
                    writeValue("phong");
                    break;
                }
            }


            if (mDefaults ||
                pPass->getPolygonMode() != PM_SOLID)
            {
                writeAttribute(3, "polygon_mode");
                switch (pPass->getPolygonMode())
                {
                case PM_POINTS:
                    writeValue("points");
                    break;
                case PM_WIREFRAME:
                    writeValue("wireframe");
                    break;
                case PM_SOLID:
                    writeValue("solid");
                    break;
                }
            }

            // polygon mode overrideable
            if (mDefaults ||
                !pPass->getPolygonModeOverrideable())
            {
                writeAttribute(3, "polygon_mode_overrideable");
                writeValue(pPass->getPolygonModeOverrideable() ? "on" : "off");
            }

            //fog override
            if (mDefaults ||
                pPass->getFogOverride() != false)
            {
                writeAttribute(3, "fog_override");
                writeValue(pPass->getFogOverride() ? "true" : "false");
                if (pPass->getFogOverride())
                {
                    switch (pPass->getFogMode())
                    {
                    case FOG_NONE:
                        writeValue("none");
                        break;
                    case FOG_LINEAR:
                        writeValue("linear");
                        break;
                    case FOG_EXP2:
                        writeValue("exp2");
                        break;
                    case FOG_EXP:
                        writeValue("exp");
                        break;
                    }

                    if (pPass->getFogMode() != FOG_NONE)
                    {
                        writeColourValue(pPass->getFogColour());
                        writeValue(StringConverter::toString(pPass->getFogDensity()));
                        writeValue(StringConverter::toString(pPass->getFogStart()));
                        writeValue(StringConverter::toString(pPass->getFogEnd()));
                    }
                }
            }

            //  GPU Vertex and Fragment program references and parameters
            if (pPass->hasVertexProgram())
            {
                writeVertexProgramRef(pPass);
            }

            if (pPass->hasFragmentProgram())
            {
                writeFragmentProgramRef(pPass);
            }

            if(pPass->hasTessellationHullProgram())
            {
                writeTesselationHullProgramRef(pPass);
            }

            if(pPass->hasTessellationDomainProgram())
            {
                writeTesselationDomainProgramRef(pPass);
            }
            
            if (pPass->hasGeometryProgram())
            {
                writeGeometryProgramRef(pPass);
            }

            // Nested texture layers
            for(auto& s : pPass->getTextureUnitStates())
            {
                writeTextureUnit(s);
            }

            // Fire write end event.
            firePassEvent(MSE_WRITE_END, skipWriting, pPass);
        }
        endSection(2);
        
        // Fire post section write event.
        firePassEvent(MSE_POST_WRITE, skipWriting, pPass);
        
        LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_NORMAL);
    }
    //-----------------------------------------------------------------------
    String MaterialSerializer::convertFiltering(FilterOptions fo)
    {
        switch (fo)
        {
        case FO_NONE:
            return "none";
        case FO_POINT:
            return "point";
        case FO_LINEAR:
            return "linear";
        case FO_ANISOTROPIC:
            return "anisotropic";
        }

        return "point";
    }
    //-----------------------------------------------------------------------
    static String convertTexAddressMode(TextureAddressingMode tam)
    {
        switch (tam)
        {
        case TextureUnitState::TAM_BORDER:
            return "border";
        case TextureUnitState::TAM_CLAMP:
            return "clamp";
        case TextureUnitState::TAM_MIRROR:
            return "mirror";
        case TextureUnitState::TAM_WRAP:
            return "wrap";
        }

        return "wrap";
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTextureUnit(const TextureUnitState *pTex)
    {
        bool skipWriting = false;

        // Fire pre-write event.
        fireTextureUnitStateEvent(MSE_PRE_WRITE, skipWriting, pTex);
        if (skipWriting)        
            return;
    
        LogManager::getSingleton().logMessage("MaterialSerializer : parsing texture layer.", LML_NORMAL);
        mBuffer += "\n";
        writeAttribute(3, "texture_unit");
        // only write out name if its not equal to the default name
        if (pTex->getName() != StringConverter::toString(pTex->getParent()->getTextureUnitStateIndex(pTex)))
            writeValue(quoteWord(pTex->getName()));

        beginSection(3);
        {
            // Fire write begin event.
            fireTextureUnitStateEvent(MSE_WRITE_BEGIN, skipWriting, pTex);

            OGRE_IGNORE_DEPRECATED_BEGIN
            // texture_alias
            if (!pTex->getTextureNameAlias().empty() && pTex->getTextureNameAlias() != pTex->getName())
            {
                writeAttribute(4, "texture_alias");
                writeValue(quoteWord(pTex->getTextureNameAlias()));
            }
            OGRE_IGNORE_DEPRECATED_END

            //texture name
            if (pTex->getNumFrames() == 1 && !pTex->getTextureName().empty())
            {
                writeAttribute(4, "texture");
                writeValue(quoteWord(pTex->getTextureName()));

                switch (pTex->getTextureType())
                {
                case TEX_TYPE_1D:
                    writeValue("1d");
                    break;
                case TEX_TYPE_2D:
                    // nothing, this is the default
                    break;
                case TEX_TYPE_2D_ARRAY:
                    writeValue("2darray");
                    break;
                case TEX_TYPE_3D:
                    writeValue("3d");
                    break;
                case TEX_TYPE_CUBE_MAP:
                    writeValue("cubic");
                    break;
                default:
                    break;
                };

                if (uint32(pTex->getNumMipmaps()) != TextureManager::getSingleton().getDefaultNumMipmaps())
                {
                    writeValue(StringConverter::toString(pTex->getNumMipmaps()));
                }

                if (pTex->getDesiredFormat() != PF_UNKNOWN)
                {
                    writeValue(PixelUtil::getFormatName(pTex->getDesiredFormat()));
                }
            }

            //anim. texture
            if (pTex->getNumFrames() > 1)
            {
                writeAttribute(4, "anim_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(quoteWord(pTex->getFrameTextureName(n)));
                writeValue(StringConverter::toString(pTex->getAnimationDuration()));
            }

            //anisotropy level
            if (mDefaults ||
                pTex->getTextureAnisotropy() != 1)
            {
                writeAttribute(4, "max_anisotropy");
                writeValue(StringConverter::toString(pTex->getTextureAnisotropy()));
            }

            //texture coordinate set
            if (mDefaults ||
                pTex->getTextureCoordSet() != 0)
            {
                writeAttribute(4, "tex_coord_set");
                writeValue(StringConverter::toString(pTex->getTextureCoordSet()));
            }

            //addressing mode
            const Sampler::UVWAddressingMode& uvw =
                pTex->getTextureAddressingMode();
            if (mDefaults ||
                uvw.u != Ogre::TextureUnitState::TAM_WRAP ||
                uvw.v != Ogre::TextureUnitState::TAM_WRAP ||
                uvw.w != Ogre::TextureUnitState::TAM_WRAP )
            {
                writeAttribute(4, "tex_address_mode");
                if (uvw.u == uvw.v && uvw.u == uvw.w)
                {
                    writeValue(convertTexAddressMode(uvw.u));
                }
                else
                {
                    writeValue(convertTexAddressMode(uvw.u));
                    writeValue(convertTexAddressMode(uvw.v));
                    if (uvw.w != TextureUnitState::TAM_WRAP)
                    {
                        writeValue(convertTexAddressMode(uvw.w));
                    }
                }
            }

            //border colour
            const ColourValue& borderColour =
                pTex->getTextureBorderColour();
            if (mDefaults ||
                borderColour != ColourValue::Black)
            {
                writeAttribute(4, "tex_border_colour");
                writeColourValue(borderColour, true);
            }

            //filtering
            if (TextureManager::getSingletonPtr() && (mDefaults || !pTex->isDefaultFiltering()))
            {
                writeAttribute(4, "filtering");
                writeValue(
                    convertFiltering(pTex->getTextureFiltering(FT_MIN))
                    + " "
                    + convertFiltering(pTex->getTextureFiltering(FT_MAG))
                    + " "
                    + convertFiltering(pTex->getTextureFiltering(FT_MIP)));
            }

            // Mip biasing
            if (mDefaults ||
                pTex->getTextureMipmapBias() != 0.0f)
            {
                writeAttribute(4, "mipmap_bias");
                writeValue(
                    StringConverter::toString(pTex->getTextureMipmapBias()));
            }

            // colour_op_ex
            if (mDefaults ||
                pTex->getColourBlendMode().operation != LBX_MODULATE ||
                pTex->getColourBlendMode().source1 != LBS_TEXTURE ||
                pTex->getColourBlendMode().source2 != LBS_CURRENT)
            {
                writeAttribute(4, "colour_op_ex");
                writeLayerBlendOperationEx(pTex->getColourBlendMode().operation);
                writeLayerBlendSource(pTex->getColourBlendMode().source1);
                writeLayerBlendSource(pTex->getColourBlendMode().source2);
                if (pTex->getColourBlendMode().operation == LBX_BLEND_MANUAL)
                    writeValue(StringConverter::toString(pTex->getColourBlendMode().factor));
                if (pTex->getColourBlendMode().source1 == LBS_MANUAL)
                    writeColourValue(pTex->getColourBlendMode().colourArg1, false);
                if (pTex->getColourBlendMode().source2 == LBS_MANUAL)
                    writeColourValue(pTex->getColourBlendMode().colourArg2, false);

                //colour_op_multipass_fallback
                writeAttribute(4, "colour_op_multipass_fallback");
                writeSceneBlendFactor(pTex->getColourBlendFallbackSrc());
                writeSceneBlendFactor(pTex->getColourBlendFallbackDest());
            }

            // alpha_op_ex
            if (mDefaults ||
                pTex->getAlphaBlendMode().operation != LBX_MODULATE ||
                pTex->getAlphaBlendMode().source1 != LBS_TEXTURE ||
                pTex->getAlphaBlendMode().source2 != LBS_CURRENT)
            {
                writeAttribute(4, "alpha_op_ex");
                writeLayerBlendOperationEx(pTex->getAlphaBlendMode().operation);
                writeLayerBlendSource(pTex->getAlphaBlendMode().source1);
                writeLayerBlendSource(pTex->getAlphaBlendMode().source2);
                if (pTex->getAlphaBlendMode().operation == LBX_BLEND_MANUAL)
                    writeValue(StringConverter::toString(pTex->getAlphaBlendMode().factor));
                else if (pTex->getAlphaBlendMode().source1 == LBS_MANUAL)
                    writeValue(StringConverter::toString(pTex->getAlphaBlendMode().alphaArg1));
                else if (pTex->getAlphaBlendMode().source2 == LBS_MANUAL)
                    writeValue(StringConverter::toString(pTex->getAlphaBlendMode().alphaArg2));
            }

            bool individualTransformElems = false;
            // rotate
            if (mDefaults ||
                pTex->getTextureRotate() != Radian(0))
            {
                writeAttribute(4, "rotate");
                writeValue(StringConverter::toString(pTex->getTextureRotate().valueDegrees()));
                individualTransformElems = true;
            }

            // scroll
            if (mDefaults ||
                pTex->getTextureUScroll() != 0 ||
                pTex->getTextureVScroll() != 0 )
            {
                writeAttribute(4, "scroll");
                writeValue(StringConverter::toString(pTex->getTextureUScroll()));
                writeValue(StringConverter::toString(pTex->getTextureVScroll()));
                individualTransformElems = true;
            }
            // scale
            if (mDefaults ||
                pTex->getTextureUScale() != 1.0 ||
                pTex->getTextureVScale() != 1.0 )
            {
                writeAttribute(4, "scale");
                writeValue(StringConverter::toString(pTex->getTextureUScale()));
                writeValue(StringConverter::toString(pTex->getTextureVScale()));
                individualTransformElems = true;
            }

            // free transform
            if (!individualTransformElems &&
                (mDefaults ||
                pTex->getTextureTransform() != Matrix4::IDENTITY))
            {
                writeAttribute(4, "transform");
                const Matrix4& xform = pTex->getTextureTransform();
                for (int row = 0; row < 4; ++row)
                {
                    for (int col = 0; col < 4; ++col)
                    {
                        writeValue(StringConverter::toString(xform[row][col]));
                    }
                }
            }

            // Used to store the u and v speeds of scroll animation effects
            float scrollAnimU = 0;
            float scrollAnimV = 0;

            EffectMap effMap = pTex->getEffects();
            if (!effMap.empty())
            {
                EffectMap::const_iterator it;
                for (it = effMap.begin(); it != effMap.end(); ++it)
                {
                    const TextureUnitState::TextureEffect& ef = it->second;
                    switch (ef.type)
                    {
                    case TextureUnitState::ET_ENVIRONMENT_MAP :
                        writeEnvironmentMapEffect(ef, pTex);
                        break;
                    case TextureUnitState::ET_ROTATE :
                        writeRotationEffect(ef, pTex);
                        break;
                    case TextureUnitState::ET_UVSCROLL :
                        scrollAnimU = scrollAnimV = ef.arg1;
                        break;
                    case TextureUnitState::ET_USCROLL :
                        scrollAnimU = ef.arg1;
                        break;
                    case TextureUnitState::ET_VSCROLL :
                        scrollAnimV = ef.arg1;
                        break;
                    case TextureUnitState::ET_TRANSFORM :
                        writeTransformEffect(ef, pTex);
                        break;
                    default:
                        break;
                    }
                }
            }

            // u and v scroll animation speeds merged, if present serialize scroll_anim
            if(scrollAnimU || scrollAnimV) {
                TextureUnitState::TextureEffect texEffect;
                texEffect.arg1 = scrollAnimU;
                texEffect.arg2 = scrollAnimV;
                writeScrollEffect(texEffect, pTex);
            }

            // Content type
            if (mDefaults ||
                pTex->getContentType() != TextureUnitState::CONTENT_NAMED)
            {
                writeAttribute(4, "content_type");
                switch(pTex->getContentType())
                {
                case TextureUnitState::CONTENT_NAMED:
                    writeValue("named");
                    break;
                case TextureUnitState::CONTENT_SHADOW:
                    writeValue("shadow");
                    break;
                case TextureUnitState::CONTENT_COMPOSITOR:
                    writeValue("compositor");
                    writeValue(quoteWord(pTex->getReferencedCompositorName()));
                    writeValue(quoteWord(pTex->getReferencedTextureName()));
                    writeValue(StringConverter::toString(pTex->getReferencedMRTIndex()));
                    break;
                };
            }

            // Fire write end event.
            fireTextureUnitStateEvent(MSE_WRITE_END, skipWriting, pTex);
        }
        endSection(3);

        // Fire post section write event.
        fireTextureUnitStateEvent(MSE_POST_WRITE, skipWriting, pTex);

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeEnvironmentMapEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        writeAttribute(4, "env_map");
        switch (effect.subtype)
        {
        case TextureUnitState::ENV_PLANAR:
            writeValue("planar");
            break;
        case TextureUnitState::ENV_CURVED:
            writeValue("spherical");
            break;
        case TextureUnitState::ENV_NORMAL:
            writeValue("cubic_normal");
            break;
        case TextureUnitState::ENV_REFLECTION:
            writeValue("cubic_reflection");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeRotationEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        if (effect.arg1)
        {
            writeAttribute(4, "rotate_anim");
            writeValue(StringConverter::toString(effect.arg1));
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTransformEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        writeAttribute(4, "wave_xform");

        switch (effect.subtype)
        {
        case TextureUnitState::TT_ROTATE:
            writeValue("rotate");
            break;
        case TextureUnitState::TT_SCALE_U:
            writeValue("scale_x");
            break;
        case TextureUnitState::TT_SCALE_V:
            writeValue("scale_y");
            break;
        case TextureUnitState::TT_TRANSLATE_U:
            writeValue("scroll_x");
            break;
        case TextureUnitState::TT_TRANSLATE_V:
            writeValue("scroll_y");
            break;
        }

        switch (effect.waveType)
        {
        case WFT_INVERSE_SAWTOOTH:
            writeValue("inverse_sawtooth");
            break;
        case WFT_SAWTOOTH:
            writeValue("sawtooth");
            break;
        case WFT_SINE:
            writeValue("sine");
            break;
        case WFT_SQUARE:
            writeValue("square");
            break;
        case WFT_TRIANGLE:
            writeValue("triangle");
            break;
        case WFT_PWM:
            writeValue("pwm");
            break;
        }

        writeValue(StringConverter::toString(effect.base));
        writeValue(StringConverter::toString(effect.frequency));
        writeValue(StringConverter::toString(effect.phase));
        writeValue(StringConverter::toString(effect.amplitude));
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeScrollEffect(
        const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        if (effect.arg1 || effect.arg2)
        {
            writeAttribute(4, "scroll_anim");
            writeValue(StringConverter::toString(effect.arg1));
            writeValue(StringConverter::toString(effect.arg2));
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf)
    {
        switch (sbf)
        {
        case SBF_DEST_ALPHA:
            writeValue("dest_alpha");
            break;
        case SBF_DEST_COLOUR:
            writeValue("dest_colour");
            break;
        case SBF_ONE:
            writeValue("one");
            break;
        case SBF_ONE_MINUS_DEST_ALPHA:
            writeValue("one_minus_dest_alpha");
            break;
        case SBF_ONE_MINUS_DEST_COLOUR:
            writeValue("one_minus_dest_colour");
            break;
        case SBF_ONE_MINUS_SOURCE_ALPHA:
            writeValue("one_minus_src_alpha");
            break;
        case SBF_ONE_MINUS_SOURCE_COLOUR:
            writeValue("one_minus_src_colour");
            break;
        case SBF_SOURCE_ALPHA:
            writeValue("src_alpha");
            break;
        case SBF_SOURCE_COLOUR:
            writeValue("src_colour");
            break;
        case SBF_ZERO:
            writeValue("zero");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dst)
    {
        if (sbf_src == SBF_ONE && sbf_dst == SBF_ONE )
            writeValue("add");
        else if (sbf_src == SBF_DEST_COLOUR && sbf_dst == SBF_ZERO)
            writeValue("modulate");
        else if (sbf_src == SBF_SOURCE_COLOUR && sbf_dst == SBF_ONE_MINUS_SOURCE_COLOUR)
            writeValue("colour_blend");
        else if (sbf_src == SBF_SOURCE_ALPHA && sbf_dst == SBF_ONE_MINUS_SOURCE_ALPHA)
            writeValue("alpha_blend");
        else
        {
            writeSceneBlendFactor(sbf_src);
            writeSceneBlendFactor(sbf_dst);
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeSceneBlendFactor(
        const SceneBlendFactor c_src, const SceneBlendFactor c_dest, 
        const SceneBlendFactor a_src, const SceneBlendFactor a_dest)
    {
        writeSceneBlendFactor(c_src, c_dest);
        writeSceneBlendFactor(a_src, a_dest);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeCompareFunction(const CompareFunction cf)
    {
        switch (cf)
        {
        case CMPF_ALWAYS_FAIL:
            writeValue("always_fail");
            break;
        case CMPF_ALWAYS_PASS:
            writeValue("always_pass");
            break;
        case CMPF_EQUAL:
            writeValue("equal");
            break;
        case CMPF_GREATER:
            writeValue("greater");
            break;
        case CMPF_GREATER_EQUAL:
            writeValue("greater_equal");
            break;
        case CMPF_LESS:
            writeValue("less");
            break;
        case CMPF_LESS_EQUAL:
            writeValue("less_equal");
            break;
        case CMPF_NOT_EQUAL:
            writeValue("not_equal");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeColourValue(const ColourValue &colour, bool writeAlpha)
    {
        writeValue(StringConverter::toString(colour.r));
        writeValue(StringConverter::toString(colour.g));
        writeValue(StringConverter::toString(colour.b));
        if (writeAlpha)
            writeValue(StringConverter::toString(colour.a));
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeLayerBlendOperationEx(const LayerBlendOperationEx op)
    {
        switch (op)
        {
        case LBX_ADD:
            writeValue("add");
            break;
        case LBX_ADD_SIGNED:
            writeValue("add_signed");
            break;
        case LBX_ADD_SMOOTH:
            writeValue("add_smooth");
            break;
        case LBX_BLEND_CURRENT_ALPHA:
            writeValue("blend_current_alpha");
            break;
        case LBX_BLEND_DIFFUSE_COLOUR:
            writeValue("blend_diffuse_colour");
            break;
        case LBX_BLEND_DIFFUSE_ALPHA:
            writeValue("blend_diffuse_alpha");
            break;
        case LBX_BLEND_MANUAL:
            writeValue("blend_manual");
            break;
        case LBX_BLEND_TEXTURE_ALPHA:
            writeValue("blend_texture_alpha");
            break;
        case LBX_MODULATE:
            writeValue("modulate");
            break;
        case LBX_MODULATE_X2:
            writeValue("modulate_x2");
            break;
        case LBX_MODULATE_X4:
            writeValue("modulate_x4");
            break;
        case LBX_SOURCE1:
            writeValue("source1");
            break;
        case LBX_SOURCE2:
            writeValue("source2");
            break;
        case LBX_SUBTRACT:
            writeValue("subtract");
            break;
        case LBX_DOTPRODUCT:
            writeValue("dotproduct");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeLayerBlendSource(const LayerBlendSource lbs)
    {
        switch (lbs)
        {
        case LBS_CURRENT:
            writeValue("src_current");
            break;
        case LBS_DIFFUSE:
            writeValue("src_diffuse");
            break;
        case LBS_MANUAL:
            writeValue("src_manual");
            break;
        case LBS_SPECULAR:
            writeValue("src_specular");
            break;
        case LBS_TEXTURE:
            writeValue("src_texture");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeVertexProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("vertex_program_ref",
            pPass->getVertexProgram(), pPass->getVertexProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTesselationHullProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("tesselation_hull_program_ref",
            pPass->getTessellationHullProgram(), pPass->getTessellationHullProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTesselationDomainProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("tesselation_domain_program_ref",
            pPass->getTessellationDomainProgram(), pPass->getTessellationDomainProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGeometryProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("geometry_program_ref",
            pPass->getGeometryProgram(), pPass->getGeometryProgramParameters());
    }
    //--
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeFragmentProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("fragment_program_ref",
            pPass->getFragmentProgram(), pPass->getFragmentProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuProgramRef(const String& attrib,
                                                const GpuProgramPtr& program, const GpuProgramParametersSharedPtr& params)
    {       
        bool skipWriting = false;

        // Fire pre-write event.
        fireGpuProgramRefEvent(MSE_PRE_WRITE, skipWriting, attrib, program, params, NULL);
        if (skipWriting)        
            return;

        mBuffer += "\n";
        writeAttribute(3, attrib);
        writeValue(quoteWord(program->getName()));
        beginSection(3);
        {
            // write out parameters
            GpuProgramParameters* defaultParams = 0;
            // does the GPU program have default parameters?
            if (program->hasDefaultParameters())
                defaultParams = program->getDefaultParameters().get();

            // Fire write begin event.
            fireGpuProgramRefEvent(MSE_WRITE_BEGIN, skipWriting, attrib, program, params, defaultParams);

            writeGPUProgramParameters(params, defaultParams);

            // Fire write end event.
            fireGpuProgramRefEvent(MSE_WRITE_END, skipWriting, attrib, program, params, defaultParams);
        }
        endSection(3);

        // add to GpuProgram container
        mGpuProgramDefinitionContainer.insert(program->getName());

        // Fire post section write event.
        fireGpuProgramRefEvent(MSE_POST_WRITE, skipWriting, attrib, program, params, NULL);     
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGPUProgramParameters(
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams, unsigned short level,
        const bool useMainBuffer)
    {
        // iterate through the constant definitions
        if (params->hasNamedParameters())
        {
            writeNamedGpuProgramParameters(params, defaultParams, level, useMainBuffer);
        }
        else
        {
            writeLowLevelGpuProgramParameters(params, defaultParams, level, useMainBuffer);
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeNamedGpuProgramParameters(
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams, unsigned short level,
        const bool useMainBuffer)
    {
        for(auto& it : params->getConstantDefinitions().map)
        {
            // get the constant definition
            const String& paramName = it.first;
            const GpuConstantDefinition& def = it.second;

            // get any auto-link
            const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                params->findAutoConstantEntry(paramName);
            const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
            if (defaultParams)
            {
                defaultAutoEntry = 
                    defaultParams->findAutoConstantEntry(paramName);
            }

            writeGpuProgramParameter("param_named", 
                                     paramName, autoEntry, defaultAutoEntry, 
                                     def.isFloat(), def.isDouble(), def.isInt(), def.isUnsignedInt(), def.isSampler(),
                                     def.physicalIndex, def.elementSize * def.arraySize,
                                     params, defaultParams, level, useMainBuffer);
        }

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeLowLevelGpuProgramParameters(
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams, unsigned short level,
        const bool useMainBuffer)
    {
        // Iterate over the logical->physical mappings
        // This will represent the values which have been set

        // float params
        GpuLogicalBufferStructPtr floatLogical = params->getLogicalBufferStruct();
        if( floatLogical )
        {
            OGRE_LOCK_MUTEX(floatLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = floatLogical->map.begin();
                i != floatLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                    params->findFloatAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findFloatAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed", 
                                         StringConverter::toString(logicalIndex), autoEntry, 
                                         defaultAutoEntry, true, false, false, false, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuProgramParameter(
        const String& commandName, const String& identifier, 
        const GpuProgramParameters::AutoConstantEntry* autoEntry, 
        const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry, 
        bool isFloat, bool isDouble, bool isInt, bool isUnsignedInt, bool isRegister,
        size_t physicalIndex, size_t physicalSize,
        const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
        const ushort level, const bool useMainBuffer)
    {
        // Skip any params with array qualifiers
        // These are only for convenience of setters, the full array will be
        // written using the base, non-array identifier
        if (identifier.find('[') != String::npos)
        {
            return;
        }

        // get any auto-link
        // don't duplicate constants that are defined as a default parameter
        bool different = false;
        if (defaultParams)
        {
            // if default is auto but we're not or vice versa
            if ((autoEntry == 0) != (defaultAutoEntry == 0))
            {
                different = true;
            }
            else if (autoEntry)
            {
                // both must be auto
                // compare the auto values
                different = (autoEntry->paramType != defaultAutoEntry->paramType
                             || autoEntry->data != defaultAutoEntry->data);
            }
            else
            {
                // compare the non-auto (raw buffer) values
                // param buffers are always initialised with all zeros
                // so unset == unset
                if (isRegister) {
                    different = memcmp(
                            params->getRegPointer(physicalIndex),
                            defaultParams->getRegPointer(physicalIndex),
                            sizeof(int) * physicalSize) != 0;
                }
                else if (isFloat)
                {
                    different = memcmp(
                        params->getFloatPointer(physicalIndex), 
                        defaultParams->getFloatPointer(physicalIndex),
                        sizeof(float) * physicalSize) != 0;
                }
                else if (isDouble)
                {
                    different = memcmp(
                        params->getDoublePointer(physicalIndex),
                        defaultParams->getDoublePointer(physicalIndex),
                        sizeof(double) * physicalSize) != 0;
                }
                else if (isInt)
                {
                    different = memcmp(
                        params->getIntPointer(physicalIndex), 
                        defaultParams->getIntPointer(physicalIndex),
                        sizeof(int) * physicalSize) != 0;
                }
                else if (isUnsignedInt)
                {
                    different = memcmp(
                        params->getUnsignedIntPointer(physicalIndex),
                        defaultParams->getUnsignedIntPointer(physicalIndex),
                        sizeof(uint) * physicalSize) != 0;
                }
                //else if (isBool)
                //{
                //    // different = memcmp(
                //    //     params->getBoolPointer(physicalIndex), 
                //    //     defaultParams->getBoolPointer(physicalIndex),
                //    //     sizeof(bool) * physicalSize) != 0;
                //    different = memcmp(
                //        params->getUnsignedIntPointer(physicalIndex), 
                //        defaultParams->getUnsignedIntPointer(physicalIndex),
                //        sizeof(uint) * physicalSize) != 0;
                //}
            }
        }

        if (!defaultParams || different)
        {
            String label = commandName;

            // is it auto
            if (autoEntry)
                label += "_auto";

            writeAttribute(level, label, useMainBuffer);
            // output param index / name
            writeValue(quoteWord(identifier), useMainBuffer);

            // if auto output auto type name and data if needed
            if (autoEntry)
            {
                const GpuProgramParameters::AutoConstantDefinition* autoConstDef =
                    GpuProgramParameters::getAutoConstantDefinition(autoEntry->paramType);

                // output auto constant name
                writeValue(quoteWord(autoConstDef->name), useMainBuffer);
                // output data if it uses it
                switch(autoConstDef->dataType)
                {
                case GpuProgramParameters::ACDT_REAL:
                    writeValue(StringConverter::toString(autoEntry->fData), useMainBuffer);
                    break;

                case GpuProgramParameters::ACDT_INT:
                    writeValue(StringConverter::toString(autoEntry->data), useMainBuffer);
                    break;

                default:
                    break;
                }
            }
            else // not auto so output all the values used
            {
                String countLabel;

                // only write a number if > 1
                if (physicalSize > 1)
                    countLabel = StringConverter::toString(physicalSize);

                if (isRegister)
                {
                    // Get pointer to start of values
                    const int* pInt = params->getRegPointer(physicalIndex);

                    writeValue("int" + countLabel, useMainBuffer);
                    // iterate through real constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pInt++), useMainBuffer);
                    }
                }
                else if (isFloat)
                {
                    // Get pointer to start of values
                    const float* pFloat = params->getFloatPointer(physicalIndex);

                    writeValue("float" + countLabel, useMainBuffer);
                    // iterate through real constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pFloat++), useMainBuffer);
                    }
                }
                else if (isDouble)
                {
                    // Get pointer to start of values
                    const double* pDouble = params->getDoublePointer(physicalIndex);

                    writeValue("double" + countLabel, useMainBuffer);
                    // iterate through double constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pDouble++), useMainBuffer);
                    }
                }
                else if (isInt)
                {
                    // Get pointer to start of values
                    const int* pInt = params->getIntPointer(physicalIndex);

                    writeValue("int" + countLabel, useMainBuffer);
                    // iterate through int constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pInt++), useMainBuffer);
                    }
                }
                else if (isUnsignedInt) 
                {
                    // Get pointer to start of values
                    const uint* pUInt = params->getUnsignedIntPointer(physicalIndex);

                    writeValue("uint" + countLabel, useMainBuffer);
                    // iterate through uint constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pUInt++), useMainBuffer);
                    }
                }
                //else if (isBool)
                //{
                //    // Get pointer to start of values
                //    // const bool* pBool = params->getBoolPointer(physicalIndex);
                //    const uint* pBool = params->getUnsignedIntPointer(physicalIndex);

                //    writeValue("bool" + countLabel, useMainBuffer);
                //    // iterate through bool constants
                //    for (size_t f = 0; f < physicalSize; ++f)
                //    {
                //        writeValue(StringConverter::toString(*pBool++), useMainBuffer);
                //    }
                //}
            }
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuPrograms(void)
    {
        // iterate through gpu program names in container
        GpuProgramDefIterator currentDef = mGpuProgramDefinitionContainer.begin();
        GpuProgramDefIterator endDef = mGpuProgramDefinitionContainer.end();

        while (currentDef != endDef)
        {
            // get gpu program from gpu program manager
            GpuProgramPtr program = GpuProgramManager::getSingleton().getByName(
                *currentDef, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
            // write gpu program definition type to buffer
            // check program type for vertex program
            // write program type
            mGpuProgramBuffer += "\n";
            writeAttribute(0, program->getParameter("type"), false);

            // write program name
            writeValue( quoteWord(program->getName()), false);
            // write program language
            const String language = program->getLanguage();
            writeValue( language, false );
            // write opening braces
            beginSection(0, false);
            {
                // write program source + filename
                writeAttribute(1, "source", false);
                writeValue(quoteWord(program->getSourceFile()), false);
                // write special parameters based on language
                for (const auto& name : program->getParameters())
                {
                    if (name != "type" &&
                        name != "assemble_code" &&
                        name != "micro_code" &&
                        name != "external_micro_code")
                    {
                        String paramstr = program->getParameter(name);
                        if ((name == "includes_skeletal_animation") && (paramstr == "false"))
                            paramstr.clear();
                        if ((name == "includes_morph_animation") && (paramstr == "false"))
                            paramstr.clear();
                        if ((name == "includes_pose_animation") && (paramstr == "0"))
                            paramstr.clear();
                        if ((name == "uses_vertex_texture_fetch") && (paramstr == "false"))
                            paramstr.clear();

                        if ((language != "asm") && (name == "syntax"))
                            paramstr.clear();

                        if (!paramstr.empty())
                        {
                            writeAttribute(1, name, false);
                            writeValue(paramstr, false);
                        }
                    }
                }

                // write default parameters
                if (program->hasDefaultParameters())
                {
                    mGpuProgramBuffer += "\n";
                    GpuProgramParametersSharedPtr gpuDefaultParams = program->getDefaultParameters();
                    writeAttribute(1, "default_params", false);
                    beginSection(1, false);
                    writeGPUProgramParameters(gpuDefaultParams, 0, 2, false);
                    endSection(1, false);
                }
            }
            // write closing braces
            endSection(0, false);

            ++currentDef;

        }

        mGpuProgramBuffer += "\n";
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::addListener(Listener* listener)
    {
        mListeners.push_back(listener);
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::removeListener(Listener* listener)
    {
        ListenerList::iterator i = std::find(mListeners.begin(), mListeners.end(), listener);
        if (i != mListeners.end())
            mListeners.erase(i);
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::fireMaterialEvent(SerializeEvent event, bool& skip, const Material* mat)
    {
        for (auto *l : mListeners)
        {
            l->materialEventRaised(this, event, skip, mat);
            if (skip)
                break;
        }       
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::fireTechniqueEvent(SerializeEvent event, bool& skip, const Technique* tech)
    {
        for (auto *l : mListeners)
        {
            l->techniqueEventRaised(this, event, skip, tech);
            if (skip)
                break;
        }
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::firePassEvent(SerializeEvent event, bool& skip, const Pass* pass)
    {
        for (auto *l : mListeners)
        {
            l->passEventRaised(this, event, skip, pass);
            if (skip)
                break;
        }
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::fireGpuProgramRefEvent(SerializeEvent event, bool& skip,
        const String& attrib, 
        const GpuProgramPtr& program, 
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams)
    {
        for (auto *l : mListeners)
        {
            l->gpuProgramRefEventRaised(this, event, skip, attrib, program, params, defaultParams);
            if (skip)
                break;
        }
    }   

    //---------------------------------------------------------------------
    void MaterialSerializer::fireTextureUnitStateEvent(SerializeEvent event, bool& skip,
        const TextureUnitState* textureUnit)
    {
        for (auto *l : mListeners)
        {
            l->textureUnitStateEventRaised(this, event, skip, textureUnit);
            if (skip)
                break;
        }
    }   
}
