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
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

#define SGX_LIB_NORMALMAP "SGXLib_NormalMap"
#define SGX_FUNC_FETCHNORMAL "SGX_FetchNormal"

namespace Ogre
{
namespace RTShader
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
const String SRS_NORMALMAP = "NormalMap";

//-----------------------------------------------------------------------
NormalMapLighting::NormalMapLighting()
{
    mNormalMapSamplerIndex = -1;
    mVSTexCoordSetIndex = 0;
    mNormalMapSpace = NMS_TANGENT;
    mParallaxHeightScale = 0.04f;
}

//-----------------------------------------------------------------------
const String& NormalMapLighting::getType() const { return SRS_NORMALMAP; }
//-----------------------------------------------------------------------
bool NormalMapLighting::createCpuSubPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();

    vsProgram->addDependency(SGX_LIB_NORMALMAP);

    psProgram->addDependency(FFP_LIB_TEXTURING);
    psProgram->addDependency(SGX_LIB_NORMALMAP);

    if (mNormalMapSpace == NMS_PARALLAX_OCCLUSION)
        psProgram->addPreprocessorDefines("POM_LAYER_COUNT=32");

    // Resolve texture coordinates.
    auto vsInTexcoord = vsMain->resolveInputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);
    auto vsOutTexcoord = vsMain->resolveOutputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);
    auto psInTexcoord = psMain->resolveInputParameter(vsOutTexcoord);

    // Resolve normal.
    auto vsInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
    auto vsOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
    auto viewNormal = psMain->resolveInputParameter(vsOutNormal);
    auto newViewNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);

    // Resolve vertex tangent
    auto vsInTangent = vsMain->resolveInputParameter(Parameter::SPC_TANGENT_OBJECT_SPACE);
    auto vsOutTangent = vsMain->resolveOutputParameter(Parameter::SPC_TANGENT_OBJECT_SPACE);
    auto psInTangent = psMain->resolveInputParameter(vsOutTangent);

    // insert before lighting stage
    auto vstage = vsMain->getStage(FFP_PS_COLOUR_BEGIN + 1);
    auto fstage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1);

    // Output texture coordinates.
    vstage.assign(vsInTexcoord, vsOutTexcoord);

    auto normalMapSampler = psProgram->resolveParameter(GCT_SAMPLER2D, "gNormalMapSampler", mNormalMapSamplerIndex);

    auto psOutTBN = psMain->resolveLocalParameter(GpuConstantType::GCT_MATRIX_3X3, "TBN");
    fstage.callFunction("SGX_CalculateTBN", {In(viewNormal), In(psInTangent), Out(psOutTBN)});

    if (mNormalMapSpace == NMS_PARALLAX || mNormalMapSpace == NMS_PARALLAX_OCCLUSION)
    {
        // assuming: lighting stage computed this
        auto vsOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
        auto viewPos = psMain->resolveInputParameter(vsOutViewPos);

        fstage.callFunction("SGX_Generate_Parallax_Texcoord",
                            {In(normalMapSampler), In(psInTexcoord), In(viewPos), In(mParallaxHeightScale),
                             In(psOutTBN), Out(psInTexcoord)});

        // overwrite texcoord0 unconditionally, only one texcoord set is supported with parallax mapping
        // we are before FFP_PS_TEXTURING, so the new value will be used
        auto texcoord0 = psMain->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
        fstage.assign(psInTexcoord, texcoord0);
    }

    // Add the normal fetch function invocation
    fstage.callFunction(SGX_FUNC_FETCHNORMAL, normalMapSampler, psInTexcoord, newViewNormal);

    if (mNormalMapSpace & NMS_TANGENT)
    {
        // transform normal & tangent
        auto normalMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        vstage.callBuiltin("mul", normalMatrix, vsInNormal, vsOutNormal);
        vstage.callBuiltin("normalize", vsOutNormal, vsOutNormal);
        vstage.callBuiltin("mul", normalMatrix, In(vsInTangent).xyz(), Out(vsOutTangent).xyz());
        vstage.callBuiltin("normalize", In(vsOutTangent).xyz(), Out(vsOutTangent).xyz());
        vstage.assign(In(vsInTangent).w(), Out(vsOutTangent).w());

        // transform normal
        fstage.callBuiltin("mul", psOutTBN, newViewNormal, newViewNormal);
    }
    else if (mNormalMapSpace & NMS_OBJECT)
    {
        // transform normal in FS
        auto normalMatrix = psProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        fstage.callBuiltin("mul", normalMatrix, newViewNormal, newViewNormal);
    }

    return true;
}

//-----------------------------------------------------------------------
void NormalMapLighting::copyFrom(const SubRenderState& rhs)
{
    const NormalMapLighting& rhsLighting = static_cast<const NormalMapLighting&>(rhs);

    mNormalMapSpace = rhsLighting.mNormalMapSpace;
    mNormalMapSamplerIndex = rhsLighting.mNormalMapSamplerIndex;
    mParallaxHeightScale = rhsLighting.mParallaxHeightScale;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (mNormalMapSamplerIndex >= 0)
    {
        mVSTexCoordSetIndex = srcPass->getTextureUnitState(mNormalMapSamplerIndex)->getTextureCoordSet();
        return true;
    }

    return false;
}

bool NormalMapLighting::setParameter(const String& name, const String& value)
{
    if (name == "normalmap_space")
    {
        // Normal map defines normals in tangent space.
        if (value == "tangent_space")
        {
            setNormalMapSpace(NMS_TANGENT);
            return true;
        }
        // Normal map defines normals in object space.
        if (value == "object_space")
        {
            setNormalMapSpace(NMS_OBJECT);
            return true;
        }
        if (value == "parallax")
        {
            setNormalMapSpace(NMS_PARALLAX);
            return true;
        }
        if (value == "parallax_occlusion")
        {
            setNormalMapSpace(NMS_PARALLAX_OCCLUSION);
            return true;
        }
        return false;
    }

    if (name == "texture_index")
    {
        return StringConverter::parse(value, mNormalMapSamplerIndex);
    }

    if (name == "height_scale")
    {
        return StringConverter::parse(value, mParallaxHeightScale);
    }

    return false;
}

//-----------------------------------------------------------------------
const String& NormalMapLightingFactory::getType() const { return SRS_NORMALMAP; }

//-----------------------------------------------------------------------
SubRenderState* NormalMapLightingFactory::createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop,
                                                         Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "lighting_stage")
    {
        if (prop->values.size() >= 2)
        {
            AbstractNodeList::const_iterator it = prop->values.begin();

            // Case light model type is normal map
            if ((*it)->getString() == "normal_map")
            {
                ++it;
                SubRenderState* subRenderState = createOrRetrieveInstance(translator);

                TextureUnitState* normalMapTexture = pass->createTextureUnitState();
                uint16 texureIdx = pass->getNumTextureUnitStates() - 1;
                normalMapTexture->setTextureName((*it)->getString());
                subRenderState->setParameter("texture_index", std::to_string(texureIdx));

                ShaderGenerator::_markNonFFP(normalMapTexture);

                // Read normal map space type.
                if (prop->values.size() >= 3)
                {
                    ++it;
                    if (!subRenderState->setParameter("normalmap_space", (*it)->getString()))
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                }

                // Read texture coordinate index.
                if (prop->values.size() >= 4)
                {
                    unsigned int textureCoordinateIndex = 0;

                    ++it;
                    if (SGScriptTranslator::getUInt(*it, &textureCoordinateIndex))
                    {
                        normalMapTexture->setTextureCoordSet(textureCoordinateIndex);
                    }
                    compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                       "use the texture_unit format to specify tex_coord_set and sampler_ref");
                }

                // Read texture filtering format.
                if (prop->values.size() >= 5)
                {
                    ++it;
                    // sampler reference
                    if (auto sampler = TextureManager::getSingleton().getSampler((*it)->getString()))
                    {
                        normalMapTexture->setSampler(sampler);
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                }

                return subRenderState;
            }
        }
    }
    return NULL;
}

SubRenderState* NormalMapLightingFactory::createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop,
                                                         TextureUnitState* texState, SGScriptTranslator* translator)
{
    if (prop->name == "normal_map" && !prop->values.empty())
    {
        auto pass = texState->getParent();
        auto texureIdx = pass->getTextureUnitStateIndex(texState);

        // blacklist from FFP
        ShaderGenerator::_markNonFFP(texState);

        SubRenderState* subRenderState = createOrRetrieveInstance(translator);
        subRenderState->setParameter("texture_index", std::to_string(texureIdx));

        auto it = prop->values.begin();
        if (!subRenderState->setParameter("normalmap_space", (*it)->getString()))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return subRenderState;
        }

        if (prop->values.size() % 2 != 1) // parameters must come in pairs now
        {
            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
            return subRenderState;
        }

        it++;
        while(it != prop->values.end())
        {
            String paramName = (*it)->getString();
            String paramValue = (*++it)->getString();

            if (!subRenderState->setParameter(paramName, paramValue))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                return subRenderState;
            }
            it++;
        }

        return subRenderState;
    }

    return NULL;
}

//-----------------------------------------------------------------------
void NormalMapLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState,
                                             const TextureUnitState* srcTex, const TextureUnitState* dstTex)
{
    NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);

    auto textureIdx = srcTex->getParent()->getTextureUnitStateIndex(srcTex);
    if(textureIdx != normalMapSubRenderState->getNormalMapSamplerIndex())
        return;

    ser->writeAttribute(5, "normal_map");

    switch (normalMapSubRenderState->getNormalMapSpace())
    {
    case NormalMapLighting::NMS_TANGENT:
        ser->writeValue("tangent_space");
        break;
    case NormalMapLighting::NMS_OBJECT:
        ser->writeValue("object_space");
        break;
    case NormalMapLighting::NMS_PARALLAX:
        ser->writeValue("parallax");
        break;
    case NormalMapLighting::NMS_PARALLAX_OCCLUSION:
        ser->writeValue("parallax_occlusion");
        break;
    }
}

//-----------------------------------------------------------------------
SubRenderState* NormalMapLightingFactory::createInstanceImpl() { return OGRE_NEW NormalMapLighting; }

} // namespace RTShader
} // namespace Ogre

#endif