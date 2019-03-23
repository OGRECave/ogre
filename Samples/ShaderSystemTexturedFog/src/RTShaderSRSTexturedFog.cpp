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
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"
#include "OgreShaderGenerator.h"
#include "OgreTextureManager.h"

#define FFP_FUNC_PIXELFOG_POSITION_DEPTH                        "FFP_PixelFog_PositionDepth"

using namespace Ogre;
using namespace RTShader;
/************************************************************************/
/*                                                                      */
/************************************************************************/
String RTShaderSRSTexturedFog::Type = "TexturedFog";

//-----------------------------------------------------------------------
RTShaderSRSTexturedFog::RTShaderSRSTexturedFog(RTShaderSRSTexturedFogFactory* factory)
{
    mFactory = factory;
    mFogMode                = FOG_NONE;
    mPassOverrideParams     = false;
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
void RTShaderSRSTexturedFog::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
                                     const LightList* pLightList)
{   
    if (mFogMode == FOG_NONE)
        return;

    FogMode fogMode;
    Real newFogStart, newFogEnd, newFogDensity;

    //Check if this is an overlay element if so disable fog 
    if ((rend->getUseIdentityView() == true) && (rend->getUseIdentityProjection() == true))
    {
        fogMode         = FOG_NONE;
        newFogStart     = 100000000;
        newFogEnd       = 200000000;
        newFogDensity   = 0;
    }
    else
    {
        if (mPassOverrideParams)
        {
            fogMode         = pass->getFogMode();
            newFogStart     = pass->getFogStart();
            newFogEnd       = pass->getFogEnd();
            newFogDensity   = pass->getFogDensity();
        }
        else
        {
            SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
        
            fogMode         = sceneMgr->getFogMode();
            newFogStart     = sceneMgr->getFogStart();
            newFogEnd       = sceneMgr->getFogEnd();
            newFogDensity   = sceneMgr->getFogDensity();                
        }
    }

    // Set fog properties.
    setFogProperties(fogMode, newFogStart, newFogEnd, newFogDensity);

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
    if (mWorldMatrix.get() == NULL)
        return false;
    
    // Resolve world view matrix.
    mCameraPos = vsProgram->resolveParameter(GpuProgramParameters::ACT_CAMERA_POSITION);
    if (mCameraPos.get() == NULL)
        return false;
    
    // Resolve vertex shader input position.
    mVSInPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    if (mVSInPos.get() == NULL)
        return false;

        // Resolve fog colour.
    mFogColour = psMain->resolveLocalParameter("FogColor", GCT_FLOAT4);
    if (mFogColour.get() == NULL)
        return false;
        
    // Resolve pixel shader output diffuse color.
    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mPSOutDiffuse.get() == NULL)    
        return false;
    
    // Resolve fog params.      
    mFogParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
    if (mFogParams.get() == NULL)
        return false;

    // Resolve vertex shader output depth.      
    mVSOutPosView = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);
    if (mVSOutPosView.get() == NULL)
        return false;
    
    // Resolve pixel shader input depth.
    mPSInPosView = psMain->resolveInputParameter(mVSOutPosView);
    if (mPSInPosView.get() == NULL)
        return false;       
    
    // Resolve vertex shader output depth.      
    mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPC_DEPTH_VIEW_SPACE);
    if (mVSOutDepth.get() == NULL)
        return false;
    
    // Resolve pixel shader input depth.
    mPSInDepth = psMain->resolveInputParameter(mVSOutDepth);
    if (mPSInDepth.get() == NULL)
        return false;       

    // Resolve texture sampler parameter.       
    mBackgroundTextureSampler = psProgram->resolveParameter(GCT_SAMPLERCUBE, mBackgroundSamplerIndex, (uint16)GPV_GLOBAL, "FogBackgroundSampler");
    if (mBackgroundTextureSampler.get() == NULL)
        return false;
    
        
    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSTexturedFog::resolveDependencies(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_FOG);
    psProgram->addDependency(FFP_LIB_COMMON);

    psProgram->addDependency(FFP_LIB_FOG);
    psProgram->addDependency(FFP_LIB_TEXTURING);
    
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
        .callFunction(FFP_FUNC_PIXELFOG_POSITION_DEPTH,
                      {In(mWorldMatrix), In(mCameraPos), In(mVSInPos), Out(mVSOutPosView), Out(mVSOutDepth)});

    auto psStage = psMain->getStage(FFP_PS_FOG);
    psStage.sampleTexture(mBackgroundTextureSampler, mPSInPosView, mFogColour);

    const char* fogFunc = NULL;
    switch (mFogMode)
    {
    case FOG_LINEAR:
        fogFunc = FFP_FUNC_PIXELFOG_LINEAR;
        break;
    case FOG_EXP:
        fogFunc = FFP_FUNC_PIXELFOG_EXP;
        break;
    case FOG_EXP2:
        fogFunc = FFP_FUNC_PIXELFOG_EXP2;
        break;
    case FOG_NONE:
       break;
    }

    psStage.callFunction(fogFunc,
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
    if (mFactory == NULL)
        return false;

    FogMode fogMode;
    ColourValue newFogColour;
    Real newFogStart, newFogEnd, newFogDensity;

    if (srcPass->getFogOverride())
    {
        fogMode         = srcPass->getFogMode();
        newFogStart     = srcPass->getFogStart();
        newFogEnd       = srcPass->getFogEnd();
        newFogDensity   = srcPass->getFogDensity();
        mPassOverrideParams = true;
    }
    else
    {
        SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
        
        if (sceneMgr == NULL)
        {
            fogMode         = FOG_NONE;
            newFogStart     = 0.0;
            newFogEnd       = 0.0;
            newFogDensity   = 0.0;
        }
        else
        {
            fogMode         = sceneMgr->getFogMode();
            newFogStart     = sceneMgr->getFogStart();
            newFogEnd       = sceneMgr->getFogEnd();
            newFogDensity   = sceneMgr->getFogDensity();            
        }
        mPassOverrideParams = false;
    }

    // Set fog properties.
    setFogProperties(fogMode, newFogStart, newFogEnd, newFogDensity);
    
    
    // Override scene fog since it will happen in shader.
    dstPass->setFog(true, FOG_NONE, ColourValue::White, newFogDensity, newFogStart, newFogEnd); 

    TextureUnitState* tus = dstPass->createTextureUnitState();
    auto tex = TextureManager::getSingleton().load(
        mFactory->getBackgroundTextureName(), RGN_DEFAULT, TEX_TYPE_CUBE_MAP);
    tus->setTexture(tex);
    mBackgroundSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    return true;
}

//-----------------------------------------------------------------------
void RTShaderSRSTexturedFog::setFogProperties(FogMode fogMode, 
                             float fogStart, 
                             float fogEnd, 
                             float fogDensity)
{
    mFogMode            = fogMode;
    mFogParamsValue.x   = fogDensity;
    mFogParamsValue.y   = fogStart;
    mFogParamsValue.z   = fogEnd;
    mFogParamsValue.w   = fogEnd != fogStart ? 1 / (fogEnd - fogStart) : 0; 
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
