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

#include "OgreMaterialSerializer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsSamplerblock.h"

namespace Ogre
{
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
            Material::LodValueIterator valueIt = pMat->getUserLodValueIterator();
            // Skip zero value
            if (valueIt.hasMoreElements())
                valueIt.getNext();
            String attributeVal;
            while (valueIt.hasMoreElements())
            {
                attributeVal.append(StringConverter::toString(valueIt.getNext()));
                if (valueIt.hasMoreElements())
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
            Material::TechniqueIterator it = pMat->getTechniqueIterator();
            while (it.hasMoreElements())
            {
                writeTechnique(it.getNext());
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
            if (mDefaults ||
                pTech->getSchemeName() != MaterialManager::DEFAULT_SCHEME_NAME)
            {
                writeAttribute(2, "scheme");
                writeValue(quoteWord(pTech->getSchemeName()));
            }

            // ShadowCasterMaterial name
            if (!pTech->getShadowCasterMaterial().isNull())
            {
                writeAttribute(2, "shadow_caster_material");
                writeValue(quoteWord(pTech->getShadowCasterMaterial()->getName()));
            }
            // GPU vendor rules
            Technique::GPUVendorRuleIterator vrit = pTech->getGPUVendorRuleIterator();
            while (vrit.hasMoreElements())
            {
                const Technique::GPUVendorRule& rule = vrit.getNext();
                writeAttribute(2, "gpu_vendor_rule");
                if (rule.includeOrExclude == Technique::INCLUDE)
                    writeValue("include");
                else
                    writeValue("exclude");
                writeValue(quoteWord(RenderSystemCapabilities::vendorToString(rule.vendor)));
            }
            // GPU device rules
            Technique::GPUDeviceNameRuleIterator dnit = pTech->getGPUDeviceNameRuleIterator();
            while (dnit.hasMoreElements())
            {
                const Technique::GPUDeviceNameRule& rule = dnit.getNext();
                writeAttribute(2, "gpu_device_rule");
                if (rule.includeOrExclude == Technique::INCLUDE)
                    writeValue("include");
                else
                    writeValue("exclude");
                writeValue(quoteWord(rule.devicePattern));
                writeValue(StringConverter::toString(rule.caseSensitive));
            }
            // Iterate over passes
            Technique::PassIterator it = const_cast<Technique*>(pTech)->getPassIterator();
            while (it.hasMoreElements())
            {
                writePass(it.getNext());
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
                    case Light::LT_VPL:
                    case Light::NUM_LIGHT_TYPES: //Keep compiler happy
                        break;
                    };
                }
            }

            if(mDefaults || pPass->getLightMask() != 0xFFFFFFFF)
            {
                writeAttribute(3, "light_mask");
                writeValue(StringConverter::toString(pPass->getLightMask()));
            }

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

            const HlmsBlendblock *blendblock = pPass->getBlendblock();
            // scene blend factor
            if( blendblock->mSeparateBlend )
            {
                if (mDefaults ||
                    blendblock->mSourceBlendFactor != SBF_ONE ||
                    blendblock->mDestBlendFactor != SBF_ZERO ||
                    blendblock->mSourceBlendFactorAlpha != SBF_ONE ||
                    blendblock->mDestBlendFactorAlpha != SBF_ZERO)
                {
                    writeAttribute(3, "separate_scene_blend");
                    writeSceneBlendFactor( blendblock->mSourceBlendFactor, blendblock->mDestBlendFactor,
                        blendblock->mSourceBlendFactorAlpha, blendblock->mDestBlendFactorAlpha );
                }
            }
            else
            {
                if (mDefaults ||
                    blendblock->mSourceBlendFactor != SBF_ONE ||
                    blendblock->mDestBlendFactor != SBF_ZERO )
                {
                    writeAttribute(3, "scene_blend");
                    writeSceneBlendFactor( blendblock->mSourceBlendFactor, blendblock->mDestBlendFactor );
                }
            }

            // alpha_to_coverage
            if (mDefaults ||blendblock->mAlphaToCoverageEnabled)
            {
                writeAttribute(3, "alpha_to_coverage");
                writeValue(blendblock->mAlphaToCoverageEnabled ? "on" : "off");
            }

            const HlmsMacroblock *macroblock = pPass->getMacroblock();

            // alpha_rejection
            if (mDefaults ||
                pPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS ||
                pPass->getAlphaRejectValue() != 0)
            {
                writeAttribute(3, "alpha_rejection");
                writeCompareFunction(pPass->getAlphaRejectFunction());
                writeValue(StringConverter::toString(pPass->getAlphaRejectValue()));
            }

            //depth check
            if (mDefaults || macroblock->mDepthCheck != true)
            {
                writeAttribute(3, "depth_check");
                writeValue(macroblock->mDepthCheck ? "on" : "off");
            }
            //depth write
            if (mDefaults || macroblock->mDepthWrite != true)
            {
                writeAttribute(3, "depth_write");
                writeValue(macroblock->mDepthWrite ? "on" : "off");
            }
            //depth function
            if (mDefaults || macroblock->mDepthFunc != CMPF_LESS_EQUAL)
            {
                writeAttribute(3, "depth_func");
                writeCompareFunction( macroblock->mDepthFunc );
            }

            //depth bias
            if (mDefaults ||
                macroblock->mDepthBiasConstant != 0 ||
                macroblock->mDepthBiasSlopeScale != 0)
            {
                writeAttribute(3, "depth_bias");
                writeValue(StringConverter::toString(macroblock->mDepthBiasConstant));
                writeValue(StringConverter::toString(macroblock->mDepthBiasSlopeScale));
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

            // hardware culling mode
            if (mDefaults || macroblock->mCullMode != CULL_CLOCKWISE)
            {
                CullingMode hcm = macroblock->mCullMode;
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


            if (mDefaults || macroblock->mPolygonMode != PM_SOLID)
            {
                writeAttribute(3, "polygon_mode");
                switch (macroblock->mPolygonMode)
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
                writeTessellationHullProgramRef(pPass);
            }

            if(pPass->hasTessellationHullProgram())
            {
                writeTessellationDomainProgramRef(pPass);
            }
            
            if (pPass->hasGeometryProgram())
            {
                writeGeometryProgramRef(pPass);
            }

            if (pPass->hasShadowCasterVertexProgram())
            {
                writeShadowCasterVertexProgramRef(pPass);
            }

            // Nested texture layers
            Pass::TextureUnitStateIterator it = const_cast<Pass*>(pPass)->getTextureUnitStateIterator();
            while(it.hasMoreElements())
            {
                writeTextureUnit(it.getNext());
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
    String convertTexAddressMode( TextureAddressingMode tam)
    {
        switch (tam)
        {
        case TAM_BORDER:
            return "border";
        case TAM_CLAMP:
            return "clamp";
        case TAM_MIRROR:
            return "mirror";
        case TAM_WRAP:
        case TAM_UNKNOWN:
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

            // texture_alias
            if (!pTex->getTextureNameAlias().empty())
            {
                writeAttribute(4, "texture_alias");
                writeValue(quoteWord(pTex->getTextureNameAlias()));
            }

            //texture name
            if (pTex->getNumFrames() == 1 && !pTex->getTextureName().empty() && !pTex->isCubic())
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
                case TEX_TYPE_3D:
                    writeValue("3d");
                    break;
                case TEX_TYPE_CUBE_MAP:
                    // nothing, deal with this as cubic_texture since it copes with all variants
                    break;
                default:
                    break;
                };

                if (pTex->getNumMipmaps() != MIP_DEFAULT)
                {
                    writeValue(StringConverter::toString(pTex->getNumMipmaps()));
                }

                if (pTex->getIsAlpha())
                {
                    writeValue("alpha");
                }

                if (pTex->getDesiredFormat() != PF_UNKNOWN)
                {
                    writeValue(PixelUtil::getFormatName(pTex->getDesiredFormat()));
                }
            }

            //anim. texture
            if (pTex->getNumFrames() > 1 && !pTex->isCubic())
            {
                writeAttribute(4, "anim_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(quoteWord(pTex->getFrameTextureName(n)));
                writeValue(StringConverter::toString(pTex->getAnimationDuration()));
            }

            //cubic texture
            if (pTex->isCubic())
            {
                writeAttribute(4, "cubic_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(quoteWord(pTex->getFrameTextureName(n)));

                //combinedUVW/separateUW
                if (pTex->getTextureType() == TEX_TYPE_CUBE_MAP)
                    writeValue("combinedUVW");
                else
                    writeValue("separateUV");
            }

            const HlmsSamplerblock *samplerblock = pTex->getSamplerblock();

            //anisotropy level
            writeAttribute(4, "max_anisotropy");
            writeValue(StringConverter::toString( samplerblock->mMaxAnisotropy ));

            //texture coordinate set
            if (mDefaults ||
                pTex->getTextureCoordSet() != 0)
            {
                writeAttribute(4, "tex_coord_set");
                writeValue(StringConverter::toString(pTex->getTextureCoordSet()));
            }

            //addressing mode
            if (mDefaults ||
                samplerblock->mU != Ogre::TAM_WRAP ||
                samplerblock->mV != Ogre::TAM_WRAP ||
                samplerblock->mW != Ogre::TAM_WRAP )
            {
                writeAttribute(4, "tex_address_mode");
                if( samplerblock->mU == samplerblock->mV &&
                    samplerblock->mV == samplerblock->mW )
                {
                    writeValue(convertTexAddressMode(samplerblock->mU));
                }
                else
                {
                    writeValue(convertTexAddressMode(samplerblock->mU));
                    writeValue(convertTexAddressMode(samplerblock->mV));
                    if (samplerblock->mW != TAM_WRAP)
                    {
                        writeValue(convertTexAddressMode(samplerblock->mW));
                    }
                }
            }

            //border colour
            if (mDefaults || samplerblock->mBorderColour != ColourValue::White)
            {
                writeAttribute(4, "tex_border_colour");
                writeColourValue(samplerblock->mBorderColour, true);
            }

            //filtering
            writeAttribute(4, "filtering");
            writeValue(
                convertFiltering( samplerblock->mMinFilter )
                + " "
                + convertFiltering( samplerblock->mMagFilter )
                + " "
                + convertFiltering( samplerblock->mMipFilter ) );

            // Mip biasing
            if (mDefaults || samplerblock->mMipLodBias != 0.0f)
            {
                writeAttribute(4, "mipmap_bias");
                writeValue(
                    StringConverter::toString( samplerblock->mMipLodBias ));
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

            // Binding type
            TextureUnitState::BindingType bt = pTex->getBindingType();
            if (mDefaults ||
                bt != TextureUnitState::BT_FRAGMENT)
            {
                writeAttribute(4, "binding_type");
                switch(bt)
                {
                case TextureUnitState::BT_FRAGMENT:
                    writeValue("fragment");
                    break;
                case TextureUnitState::BT_VERTEX:
                    writeValue("vertex");
                    break;
                case TextureUnitState::BT_GEOMETRY:
                    writeValue("geometry");
                    break;
                case TextureUnitState::BT_TESSELLATION_DOMAIN:
                    writeValue("tessellation_domain");
                    break;
                case TextureUnitState::BT_TESSELLATION_HULL:
                    writeValue("tessellation_hull");
                    break;
                };
        
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
                    writeValue(quoteWord(pTex->getTextureName()));
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
        case NUM_COMPARE_FUNCTIONS: //keep compiler happy
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
    void MaterialSerializer::writeTessellationHullProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("tessellation_hull_program_ref",
            pPass->getTessellationHullProgram(), pPass->getTessellationHullProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTessellationDomainProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("tessellation_domain_program_ref",
            pPass->getTessellationDomainProgram(), pPass->getTessellationDomainProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeShadowCasterVertexProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("shadow_caster_vertex_program_ref",
            pPass->getShadowCasterVertexProgram(), pPass->getShadowCasterVertexProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeShadowCasterFragmentProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("shadow_caster_fragment_program_ref",
            pPass->getShadowCasterFragmentProgram(), pPass->getShadowCasterFragmentProgramParameters());
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
                defaultParams = program->getDefaultParameters().getPointer();

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
        GpuConstantDefinitionIterator constIt = params->getConstantDefinitionIterator();
        while(constIt.hasMoreElements())
        {
            // get the constant definition
            const String& paramName = constIt.peekNextKey();
            const GpuConstantDefinition& def =
                constIt.getNext();

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
                                     def.isFloat(), def.isDouble(), (def.isInt() || def.isSampler()), def.isUnsignedInt(),
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
        GpuLogicalBufferStructPtr floatLogical = params->getFloatLogicalBufferStruct();
        if( !floatLogical.isNull() )
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
                                         defaultAutoEntry, true, false, false, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }
        }

        // double params
        GpuLogicalBufferStructPtr doubleLogical = params->getDoubleLogicalBufferStruct();
        if( !doubleLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(floatLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = doubleLogical->map.begin();
                i != doubleLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry =
                    params->findDoubleAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findDoubleAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed",
                                         StringConverter::toString(logicalIndex), autoEntry,
                                         defaultAutoEntry, false, true, false, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }
        }

        // int params
        GpuLogicalBufferStructPtr intLogical = params->getIntLogicalBufferStruct();
        if( !intLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(intLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = intLogical->map.begin();
                i != intLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                    params->findIntAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findIntAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed", 
                                         StringConverter::toString(logicalIndex), autoEntry, 
                                         defaultAutoEntry, false, false, true, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }

        }

        // uint params
        GpuLogicalBufferStructPtr uintLogical = params->getUnsignedIntLogicalBufferStruct();
        if( !uintLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(uintLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = uintLogical->map.begin();
                i != uintLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                    params->findUnsignedIntAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findUnsignedIntAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed", 
                                         StringConverter::toString(logicalIndex), autoEntry, 
                                         defaultAutoEntry, false, false, false, true,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }

        }

        // // bool params
        // GpuLogicalBufferStructPtr boolLogical = params->getBoolLogicalBufferStruct();
        // if( !boolLogical.isNull() )
        // {
        //     OGRE_LOCK_MUTEX(boolLogical->mutex);

        //     for(GpuLogicalIndexUseMap::const_iterator i = boolLogical->map.begin();
        //         i != boolLogical->map.end(); ++i)
        //     {
        //         size_t logicalIndex = i->first;
        //         const GpuLogicalIndexUse& logicalUse = i->second;

        //         const GpuProgramParameters::AutoConstantEntry* autoEntry = 
        //             params->findBoolAutoConstantEntry(logicalIndex);
        //         const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
        //         if (defaultParams)
        //         {
        //             defaultAutoEntry = defaultParams->findBoolAutoConstantEntry(logicalIndex);
        //         }

        //         writeGpuProgramParameter("param_indexed", 
        //                                  StringConverter::toString(logicalIndex), autoEntry, 
        //                                  defaultAutoEntry, false, false, false, false,
        //                                  logicalUse.physicalIndex, logicalUse.currentSize,
        //                                  params, defaultParams, level, useMainBuffer);
        //     }

        // }

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuProgramParameter(
        const String& commandName, const String& identifier, 
        const GpuProgramParameters::AutoConstantEntry* autoEntry, 
        const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry, 
        bool isFloat, bool isDouble, bool isInt, bool isUnsignedInt,
        size_t physicalIndex, size_t physicalSize,
        const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
        const ushort level, const bool useMainBuffer)
    {
        // Skip any params with array qualifiers
        // These are only for convenience of setters, the full array will be
        // written using the base, non-array identifier
        if (identifier.find("[") != String::npos)
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
                if (isFloat)
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

                assert(autoConstDef && "Bad auto constant Definition Table");
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

                if (isFloat)
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
            GpuProgramPtr program = GpuProgramManager::getSingleton().getByName((*currentDef));
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
                const ParameterList& params = program->getParameters();
                ParameterList::const_iterator currentParam = params.begin();
                ParameterList::const_iterator endParam = params.end();

                while (currentParam != endParam)
                {
                    if (currentParam->name != "type" &&
                        currentParam->name != "assemble_code" &&
                        currentParam->name != "micro_code" &&
                        currentParam->name != "external_micro_code")
                    {
                        String paramstr = program->getParameter(currentParam->name);
                        if ((currentParam->name == "includes_skeletal_animation")
                            && (paramstr == "false"))
                            paramstr.clear();
                        if ((currentParam->name == "includes_morph_animation")
                            && (paramstr == "false"))
                            paramstr.clear();
                        if ((currentParam->name == "includes_pose_animation")
                            && (paramstr == "0"))
                            paramstr.clear();
                        if ((currentParam->name == "uses_vertex_texture_fetch")
                            && (paramstr == "false"))
                            paramstr.clear();

                        if ((language != "asm") && (currentParam->name == "syntax"))
                            paramstr.clear();

                        if (!paramstr.empty())
                        {
                            writeAttribute(1, currentParam->name, false);
                            writeValue(paramstr, false);
                        }
                    }
                    ++currentParam;
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
        ListenerListIterator i, iend;
        iend = mListeners.end();
        for (i = mListeners.begin(); i != iend; ++i)
        {
            if (*i == listener)
            {
                mListeners.erase(i);
                break;
            }
        }
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::fireMaterialEvent(SerializeEvent event, bool& skip, const Material* mat)
    {
        ListenerListIterator it    = mListeners.begin();
        ListenerListIterator itEnd = mListeners.end();

        while (it != itEnd)
        {
            (*it)->materialEventRaised(this, event, skip, mat);         
            if (skip)
                break;
            ++it;
        }       
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::fireTechniqueEvent(SerializeEvent event, bool& skip, const Technique* tech)
    {
        ListenerListIterator it    = mListeners.begin();
        ListenerListIterator itEnd = mListeners.end();

        while (it != itEnd)
        {
            (*it)->techniqueEventRaised(this, event, skip, tech);
            if (skip)
                break;
            ++it;
        }
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::firePassEvent(SerializeEvent event, bool& skip, const Pass* pass)
    {
        ListenerListIterator it    = mListeners.begin();
        ListenerListIterator itEnd = mListeners.end();

        while (it != itEnd)
        {
            (*it)->passEventRaised(this, event, skip, pass);
            if (skip)
                break;
            ++it;
        }
    }

    //---------------------------------------------------------------------
    void MaterialSerializer::fireGpuProgramRefEvent(SerializeEvent event, bool& skip,
        const String& attrib, 
        const GpuProgramPtr& program, 
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams)
    {
        ListenerListIterator it    = mListeners.begin();
        ListenerListIterator itEnd = mListeners.end();

        while (it != itEnd)
        {
            (*it)->gpuProgramRefEventRaised(this, event, skip, attrib, program, params, defaultParams);
            if (skip)
                break;
            ++it;
        }
    }   

    //---------------------------------------------------------------------
    void MaterialSerializer::fireTextureUnitStateEvent(SerializeEvent event, bool& skip,
        const TextureUnitState* textureUnit)
    {
        ListenerListIterator it    = mListeners.begin();
        ListenerListIterator itEnd = mListeners.end();

        while (it != itEnd)
        {
            (*it)->textureUnitStateEventRaised(this, event, skip, textureUnit);
            if (skip)
                break;
            ++it;
        }
    }   
}
