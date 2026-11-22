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
#include "RTShaderSRSTexturedFog.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderGenerator.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramSet.h"
#include "OgreTechnique.h"
#include "OgreTextureManager.h"


using namespace Ogre;
using namespace RTShader;
/************************************************************************/
/*                                                                      */
/************************************************************************/
String RTShaderSRSTexturedFog::Type = "TexturedFog";

//-----------------------------------------------------------------------
RTShaderSRSTexturedFog::RTShaderSRSTexturedFog(RTShaderSRSTexturedFogFactory* factory)
    : mFactory(factory), mFogMode(FOG_NONE), mPassOverrideParams(false), mBackgroundSamplerIndex(0)
{
}

//-----------------------------------------------------------------------
const String& RTShaderSRSTexturedFog::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int RTShaderSRSTexturedFog::getExecutionOrder() const
{
    return FFP_FOG;
}
//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::updateGpuProgramsParams(Renderable* rend, const Pass* pass,
                                                     const AutoParamDataSource* source,
                                                     const LightList* pLightList)
{
    if (mFogMode == FOG_NONE)
        return;

    if (rend->getUseIdentityView() && rend->getUseIdentityProjection())
    {
        // overlays are never fogged
        setFogProperties(0, -1, 0);
    }
    else if (mPassOverrideParams)
    {
        setFogProperties(pass->getFogStart(), pass->getFogEnd(), pass->getFogDensity());
    }
    else if (SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager())
    {
        if (sceneMgr->getFogMode() != FOG_NONE)
            setFogProperties(sceneMgr->getFogStart(), sceneMgr->getFogEnd(), sceneMgr->getFogDensity());
        else
            // the shaders are shared with the scene managers that do fog: stay neutral
            setFogProperties(0, -1, 0);
    }
    else
    {
        setFogProperties(0, -1, 0);
    }

    mFogParams->setGpuParameter(mFogParamsValue);
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::resolveParameters(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();


    // Resolve world view matrix.
    mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
    // Resolve world view matrix.
    mCameraPos = vsProgram->resolveParameter(GpuProgramParameters::ACT_CAMERA_POSITION);
    // Resolve vertex shader input position.
    mVSInPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

    // Resolve fog colour.
    mFogColour = psMain->resolveLocalParameter(GCT_FLOAT4, "FogColor");

    // Resolve pixel shader output diffuse color.
    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    // Resolve fog params.      
    mFogParams = psProgram->resolveParameter(GCT_FLOAT4, "gFogParams");

    // Resolve vertex shader output depth.      
    mVSOutPosView = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
    // Resolve pixel shader input depth.
    mPSInPosView = psMain->resolveInputParameter(mVSOutPosView);
    // Resolve vertex shader output depth.      
    mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPC_DEPTH_VIEW_SPACE);
    // Resolve pixel shader input depth.
    mPSInDepth = psMain->resolveInputParameter(mVSOutDepth);
    // Resolve texture sampler parameter.       
    mBackgroundTextureSampler = psProgram->resolveParameter(GCT_SAMPLERCUBE, "FogBackgroundSampler", mBackgroundSamplerIndex);

    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::resolveDependencies(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency("FFPLib_Fog");

    psProgram->addDependency("FFPLib_Fog");
    
    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::addFunctionInvocations(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    vsMain->getStage(FFP_VS_FOG)
        .callFunction("FFP_PixelFog_PositionDepth",
                      {In(mWorldMatrix), In(mCameraPos), In(mVSInPos), Out(mVSOutPosView), Out(mVSOutDepth)});

    auto psStage = psMain->getStage(FFP_PS_FOG);
    psStage.sampleTexture(mBackgroundTextureSampler, mPSInPosView, mFogColour);

    psProgram->addPreprocessorDefines(StringUtil::format("FOG_TYPE=%d", mFogMode));
    vsProgram->addPreprocessorDefines(StringUtil::format("FOG_TYPE=%d", mFogMode));

    psStage.callFunction("FFP_PixelFog",
                         {In(mPSInDepth), In(mFogParams), In(mFogColour), In(mPSOutDiffuse), Out(mPSOutDiffuse)});
    return true;
}

//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::copyFrom(const SubRenderState& rhs)
{
    const RTShaderSRSTexturedFog& rhsFog = static_cast<const RTShaderSRSTexturedFog&>(rhs);

    mFogMode            = rhsFog.mFogMode;
    mFogParamsValue     = rhsFog.mFogParamsValue;
    mFactory            = rhsFog.mFactory;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (!mFactory)
        return false;

    // neutral by default, so a scene manager without fog leaves the colour as is
    Real fogStart = 0, fogEnd = -1, fogDensity = 0;

    mPassOverrideParams = srcPass->getFogOverride();
    if (mPassOverrideParams)
    {
        mFogMode = srcPass->getFogMode();
        fogStart = srcPass->getFogStart();
        fogEnd = srcPass->getFogEnd();
        fogDensity = srcPass->getFogDensity();
    }
    else
    {
        // the mode is shared by all scene managers, see SGScheme::synchronizeWithFogSettings;
        // the values are taken from the rendered one in updateGpuProgramsParams
        mFogMode = ShaderGenerator::getSingleton().getFogMode(dstPass->getParent()->getSchemeName());
    }

    setFogProperties(fogStart, fogEnd, fogDensity);

    // the fog happens in the shader now
    dstPass->setFog(true, FOG_NONE, ColourValue::White, fogDensity, fogStart, fogEnd);

    TextureUnitState* tus = dstPass->createTextureUnitState();
    auto tex = TextureManager::getSingleton().load(mFactory->getBackgroundTextureName(), RGN_DEFAULT,
                                                   TEX_TYPE_CUBE_MAP);
    tus->setTexture(tex);
    mBackgroundSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    return true;
}

//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::setFogProperties(Real fogStart, Real fogEnd, Real fogDensity)
{
    mFogParamsValue.x = fogDensity;
    mFogParamsValue.y = fogStart;
    mFogParamsValue.z = fogEnd;
    mFogParamsValue.w = fogEnd != fogStart ? 1 / (fogEnd - fogStart) : 0;
}

//-----------------------------------------------------------------------
const String& RTShaderSRSTexturedFogFactory::getType() const
{
    return RTShaderSRSTexturedFog::Type;
}

//-----------------------------------------------------------------------
SubRenderState* RTShaderSRSTexturedFogFactory::createInstanceImpl()
{
    return OGRE_NEW RTShaderSRSTexturedFog(this);
}
