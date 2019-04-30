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

namespace Ogre
{
namespace RTShader
{

/// ITU BT.601 for gamma compressed colors
static float luminance(const ColourValue& col)
{
    static Vector3 grayXfer(0.3f, 0.59f, 0.11f);
    return grayXfer.dotProduct(Vector3(col.r, col.g, col.b));
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
String CookTorranceLighting::Type = "SGX_CookTorranceLighting";

//-----------------------------------------------------------------------
const String& CookTorranceLighting::getType() const
{
    return Type;
}
//-----------------------------------------------------------------------
void CookTorranceLighting::updateGpuProgramsParams(Renderable* rend, const Pass* pass,
                                                   const AutoParamDataSource* source,
                                                   const LightList* pLightList)
{
    mSurfaceRoughness->setGpuParameter(Math::saturate(1.0f - source->getSurfaceShininess()/128.0f));
    mSurfaceMetallness->setGpuParameter(luminance(source->getSurfaceSpecularColour()));
    
    NormalMapLighting::updateGpuProgramsParams(rend, pass, source, pLightList);
}
//-----------------------------------------------------------------------
bool CookTorranceLighting::resolveParameters(ProgramSet* programSet)
{
    mSpecularEnable = true;
    mNormalMapSpace = NMS_TANGENT;

    if (false == resolveGlobalParameters(programSet))
        return false;

    if (false == resolvePerLightParameters(programSet))
        return false;

    return true;
}

//-----------------------------------------------------------------------
bool CookTorranceLighting::resolveGlobalParameters(ProgramSet* programSet)
{

    NormalMapLighting::resolveGlobalParameters(programSet);

    // Get surface shininess.
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    mSurfaceRoughness = psProgram->resolveParameter(GCT_FLOAT1, -1, GPV_GLOBAL, "roughness");
    mSurfaceMetallness = psProgram->resolveParameter(GCT_FLOAT1, -1, GPV_GLOBAL, "metallness");
    mSurfaceDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);

    return true;
}

//-----------------------------------------------------------------------
bool CookTorranceLighting::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(FFP_LIB_TRANSFORM);
    vsProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);
    vsProgram->addDependency(SGX_LIB_NORMALMAP);

    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

    psProgram->addPreprocessorDefines("COOK_TORRANCE");

    return true;
}

//-----------------------------------------------------------------------
bool CookTorranceLighting::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();

    // Add the global illumination functions.
    addVSInvocation(vsMain->getStage(FFP_VS_LIGHTING));

    auto stage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1);

    if(!mNormalMapTextureName.empty())
        stage.callFunction(SGX_FUNC_FETCHNORMAL, mPSNormalMapSampler, mPSInTexcoord, mViewNormal);

    // Add the global illumination functions.
    addPSGlobalIlluminationInvocation(stage);

    // Add per light functions.
    for (const auto& lp : mLightParamsList)
    {
        addPSIlluminationInvocation(&lp, stage);
    }

    // Assign back temporary variables to the ps diffuse and specular components.
    stage.assign(mOutDiffuse, mInDiffuse);

    return true;
}

//-----------------------------------------------------------------------
void CookTorranceLighting::addPSIlluminationInvocation(const LightParams* curLightParams,
                                                       const FunctionStageRef& stage)
{
    switch (curLightParams->mType)
    {
    case Light::LT_DIRECTIONAL:
        stage.callFunction(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR,
                           {In(mViewNormal), In(mToView), In(curLightParams->mPSInDirection).xyz(),
                            In(curLightParams->mDiffuseColour).xyz(),
                            In(curLightParams->mSpecularColour).xyz(), In(mSurfaceRoughness),
                            In(mSurfaceDiffuseColour).xyz(), In(mSurfaceMetallness),
                            InOut(mOutDiffuse).xyz(), InOut(mOutSpecular).xyz()});
        break;

    case Light::LT_POINT:
        stage.callFunction(SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR,
                           {In(mViewNormal), In(mToView), In(curLightParams->mToLight),
                            In(curLightParams->mAttenuatParams), In(curLightParams->mDiffuseColour).xyz(),
                            In(curLightParams->mSpecularColour).xyz(), In(mSurfaceRoughness),
                            In(mSurfaceDiffuseColour).xyz(), In(mSurfaceMetallness),
                            InOut(mOutDiffuse).xyz(), InOut(mOutSpecular).xyz()});

        break;

    case Light::LT_SPOTLIGHT:
        stage.callFunction(SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR,
                           {In(mViewNormal), In(mToView), In(curLightParams->mToLight),
                            In(curLightParams->mPSInDirection).xyz(), In(curLightParams->mAttenuatParams),
                            In(curLightParams->mSpotParams).xyz(), In(curLightParams->mDiffuseColour).xyz(),
                            In(curLightParams->mSpecularColour).xyz(), In(mSurfaceRoughness),
                            In(mSurfaceDiffuseColour).xyz(), In(mSurfaceMetallness),
                            InOut(mOutDiffuse).xyz(), InOut(mOutSpecular).xyz()});
        break;
    }
}

//-----------------------------------------------------------------------
const String& CookTorranceLightingFactory::getType() const
{
    return CookTorranceLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState* CookTorranceLightingFactory::createInstance(ScriptCompiler* compiler,
                                                            PropertyAbstractNode* prop, Pass* pass,
                                                            SGScriptTranslator* translator)
{
    if (prop->name != "lighting_stage" || prop->values.empty())
        return NULL;

    auto it = prop->values.begin();
    String val;

    if (!SGScriptTranslator::getString(*it, &val))
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        return NULL;
    }

    CookTorranceLighting* ret = NULL;
    if (val == "cook_torrance")
    {
        ret = static_cast<CookTorranceLighting*>(createOrRetrieveInstance(translator));
    }

    if(prop->values.size() >= 2)
    {
        SGScriptTranslator::getString(*(it++), &val);

    }

    return ret;
}

//-----------------------------------------------------------------------
void CookTorranceLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState,
                                                Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("cook_torrance");
}

//-----------------------------------------------------------------------
SubRenderState* CookTorranceLightingFactory::createInstanceImpl()
{
    return OGRE_NEW CookTorranceLighting;
}

} // namespace RTShader
} // namespace Ogre

#endif
