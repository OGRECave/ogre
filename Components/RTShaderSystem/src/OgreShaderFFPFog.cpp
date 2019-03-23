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
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPFog::Type = "FFP_Fog";

//-----------------------------------------------------------------------
FFPFog::FFPFog()
{
    mFogMode                = FOG_NONE;
    mCalcMode               = CM_PER_VERTEX;    
    mPassOverrideParams     = false;
}

//-----------------------------------------------------------------------
const String& FFPFog::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int FFPFog::getExecutionOrder() const
{
    return FFP_FOG;
}
//-----------------------------------------------------------------------
void FFPFog::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
                                     const LightList* pLightList)
{   
    if (mFogMode == FOG_NONE)
        return;

    FogMode fogMode;
    ColourValue newFogColour;
    Real newFogStart, newFogEnd, newFogDensity;

    if (mPassOverrideParams)
    {
        fogMode         = pass->getFogMode();
        newFogColour    = pass->getFogColour();
        newFogStart     = pass->getFogStart();
        newFogEnd       = pass->getFogEnd();
        newFogDensity   = pass->getFogDensity();
    }
    else
    {
        SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();
        
        fogMode         = sceneMgr->getFogMode();
        newFogColour    = sceneMgr->getFogColour();
        newFogStart     = sceneMgr->getFogStart();
        newFogEnd       = sceneMgr->getFogEnd();
        newFogDensity   = sceneMgr->getFogDensity();                
    }

    // Set fog properties.
    setFogProperties(fogMode, newFogColour, newFogStart, newFogEnd, newFogDensity);

    mFogParams->setGpuParameter(mFogParamsValue);
    mFogColour->setGpuParameter(mFogColourValue);
}

//-----------------------------------------------------------------------
bool FFPFog::resolveParameters(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    // Resolve world view matrix.
    mWorldViewProjMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
    
    // Resolve vertex shader input position.
    mVSInPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    
    // Resolve fog colour.
    mFogColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogColor");
    
    // Resolve pixel shader output diffuse color.
    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
    
    // Per pixel fog.
    if (mCalcMode == CM_PER_PIXEL)
    {
        // Resolve fog params.      
        mFogParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
        
        // Resolve vertex shader output depth.      
        mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPC_DEPTH_VIEW_SPACE);
        
        // Resolve pixel shader input depth.
        mPSInDepth = psMain->resolveInputParameter(mVSOutDepth);
    }
    // Per vertex fog.
    else
    {       
        // Resolve fog params.      
        mFogParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "gFogParams");
        
        // Resolve vertex shader output fog factor.
        mVSOutFogFactor = vsMain->resolveOutputParameter(Parameter::SPC_UNKNOWN, GCT_FLOAT1);

        // Resolve pixel shader input fog factor.
        mPSInFogFactor = psMain->resolveInputParameter(mVSOutFogFactor);
    }

    return true;
}

//-----------------------------------------------------------------------
bool FFPFog::resolveDependencies(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_FOG);
    psProgram->addDependency(FFP_LIB_COMMON);

    // Per pixel fog.
    if (mCalcMode == CM_PER_PIXEL)
    {
        psProgram->addDependency(FFP_LIB_FOG);

    }   

    return true;
}

//-----------------------------------------------------------------------
bool FFPFog::addFunctionInvocations(ProgramSet* programSet)
{
    if (mFogMode == FOG_NONE)
        return true;

    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    const char* fogfunc = NULL;
    
    // Per pixel fog.
    if (mCalcMode == CM_PER_PIXEL)
    {
        //! [func_invoc]
        auto vsFogStage = vsMain->getStage(FFP_VS_FOG);
        vsFogStage.callFunction(FFP_FUNC_PIXELFOG_DEPTH, mWorldViewProjMatrix, mVSInPos, mVSOutDepth);
        //! [func_invoc]

        switch (mFogMode)
        {
        case FOG_LINEAR:
            fogfunc = FFP_FUNC_PIXELFOG_LINEAR;
            break;
        case FOG_EXP:
            fogfunc = FFP_FUNC_PIXELFOG_EXP;
            break;
        case FOG_EXP2:
            fogfunc = FFP_FUNC_PIXELFOG_EXP2;
            break;
        case FOG_NONE:
            return true;
        }
        psMain->getStage(FFP_PS_FOG)
            .callFunction(fogfunc,
                          {In(mPSInDepth), In(mFogParams), In(mFogColour), In(mPSOutDiffuse), Out(mPSOutDiffuse)});
    }

    // Per vertex fog.
    else
    {
        switch (mFogMode)
        {
        case FOG_LINEAR:
            fogfunc = FFP_FUNC_VERTEXFOG_LINEAR;
            break;
        case FOG_EXP:
            fogfunc = FFP_FUNC_VERTEXFOG_EXP;
            break;
        case FOG_EXP2:
            fogfunc = FFP_FUNC_VERTEXFOG_EXP2;
            break;
        case FOG_NONE:
            return true;
        }

        vsMain->getStage(FFP_VS_FOG)
            .callFunction(fogfunc, {In(mWorldViewProjMatrix), In(mVSInPos), In(mFogParams), Out(mVSOutFogFactor)});
        psMain->getStage(FFP_VS_FOG)
            .callFunction(FFP_FUNC_LERP, {In(mFogColour), In(mPSOutDiffuse), In(mPSInFogFactor), Out(mPSOutDiffuse)});
    }



    return true;
}

//-----------------------------------------------------------------------
void FFPFog::copyFrom(const SubRenderState& rhs)
{
    const FFPFog& rhsFog = static_cast<const FFPFog&>(rhs);

    mFogMode            = rhsFog.mFogMode;
    mFogColourValue     = rhsFog.mFogColourValue;
    mFogParamsValue     = rhsFog.mFogParamsValue;

    setCalcMode(rhsFog.mCalcMode);
}

//-----------------------------------------------------------------------
bool FFPFog::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{   
    FogMode fogMode;
    ColourValue newFogColour;
    Real newFogStart, newFogEnd, newFogDensity;

    if (srcPass->getFogOverride())
    {
        fogMode         = srcPass->getFogMode();
        newFogColour    = srcPass->getFogColour();
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
            newFogColour    = ColourValue::White;
            newFogStart     = 0.0;
            newFogEnd       = 0.0;
            newFogDensity   = 0.0;
        }
        else
        {
            fogMode         = sceneMgr->getFogMode();
            newFogColour    = sceneMgr->getFogColour();
            newFogStart     = sceneMgr->getFogStart();
            newFogEnd       = sceneMgr->getFogEnd();
            newFogDensity   = sceneMgr->getFogDensity();            
        }
        mPassOverrideParams = false;
    }

    // Set fog properties.
    setFogProperties(fogMode, newFogColour, newFogStart, newFogEnd, newFogDensity);
    
    
    // Override scene fog since it will happen in shader.
    dstPass->setFog(true, FOG_NONE, newFogColour, newFogDensity, newFogStart, newFogEnd);   

    return true;
}

//-----------------------------------------------------------------------
void FFPFog::setFogProperties(FogMode fogMode, 
                             const ColourValue& fogColour, 
                             float fogStart, 
                             float fogEnd, 
                             float fogDensity)
{
    mFogMode            = fogMode;
    mFogColourValue     = fogColour;
    mFogParamsValue.x   = fogDensity;
    mFogParamsValue.y   = fogStart;
    mFogParamsValue.z   = fogEnd;
    mFogParamsValue.w   = fogEnd != fogStart ? 1 / (fogEnd - fogStart) : 0; 
}

//-----------------------------------------------------------------------
const String& FFPFogFactory::getType() const
{
    return FFPFog::Type;
}

//-----------------------------------------------------------------------
SubRenderState* FFPFogFactory::createInstance(ScriptCompiler* compiler, 
                                                    PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "fog_stage")
    {
        if(prop->values.size() >= 1)
        {
            String strValue;

            if(false == SGScriptTranslator::getString(prop->values.front(), &strValue))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                return NULL;
            }

            if (strValue == "ffp")
            {
                SubRenderState* subRenderState = createOrRetrieveInstance(translator);
                FFPFog* fogSubRenderState = static_cast<FFPFog*>(subRenderState);
                AbstractNodeList::const_iterator it = prop->values.begin();

                if(prop->values.size() >= 2)
                {
                    ++it;
                    if (false == SGScriptTranslator::getString(*it, &strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return NULL;
                    }

                    if (strValue == "per_vertex")
                    {
                        fogSubRenderState->setCalcMode(FFPFog::CM_PER_VERTEX);
                    }
                    else if (strValue == "per_pixel")
                    {
                        fogSubRenderState->setCalcMode(FFPFog::CM_PER_PIXEL);
                    }
                }
                
                return subRenderState;
            }
        }       
    }

    return NULL;
}

//-----------------------------------------------------------------------
void FFPFogFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
                                        Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "fog_stage");
    ser->writeValue("ffp");

    FFPFog* fogSubRenderState = static_cast<FFPFog*>(subRenderState);


    if (fogSubRenderState->getCalcMode() == FFPFog::CM_PER_VERTEX)
    {
        ser->writeValue("per_vertex");
    }
    else if (fogSubRenderState->getCalcMode() == FFPFog::CM_PER_PIXEL)
    {
        ser->writeValue("per_pixel");
    }
}

//-----------------------------------------------------------------------
SubRenderState* FFPFogFactory::createInstanceImpl()
{
    return OGRE_NEW FFPFog;
}

}
}

#endif
