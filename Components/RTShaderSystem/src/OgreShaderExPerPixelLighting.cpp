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

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String PerPixelLighting::Type = "SGX_PerPixelLighting";

//-----------------------------------------------------------------------
const String& PerPixelLighting::getType() const
{
    return Type;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolveParameters(ProgramSet* programSet)
{
    if (false == resolveGlobalParameters(programSet))
        return false;
    
    if (false == resolvePerLightParameters(programSet))
        return false;
    
    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolveGlobalParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    // Resolve world view IT matrix.
    mWorldViewITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);

    // Get surface ambient colour if need to.
    if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
    {       
        mDerivedAmbientLightColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
    }
    else
    {
        mLightAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
    }

    // Get surface emissive colour if need to.
    if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        mSurfaceEmissiveColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
    }

    // Get derived scene colour.
    mDerivedSceneColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);

    // Get surface shininess.
    mSurfaceShininess = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SHININESS);

    // Resolve input vertex shader normal.
    mVSInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);

    // Resolve output vertex shader normal.
    mVSOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_VIEW_SPACE);

    // Resolve input pixel shader normal.
    mViewNormal = psMain->resolveInputParameter(mVSOutNormal);

    mInDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mInDiffuse.get() == NULL)
    {
        mInDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    }

    OgreAssert(mInDiffuse, "mInDiffuse is NULL");

    mOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (mSpecularEnable)
    {
        mOutSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);

        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

        mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);

        mViewPos = psMain->resolveInputParameter(mVSOutViewPos);

        mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
    }

    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolvePerLightParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    bool needViewPos = false;

    // Resolve per light parameters.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        switch (mLightParamsList[i].mType)
        {
        case Light::LT_DIRECTIONAL:
            mLightParamsList[i].mDirection = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, i);
            mLightParamsList[i].mPSInDirection = mLightParamsList[i].mDirection;
            needViewPos = mSpecularEnable || needViewPos;
            break;

        case Light::LT_POINT:
            mLightParamsList[i].mPosition = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, i);
            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);
            
            needViewPos = true;
            break;

        case Light::LT_SPOTLIGHT:
            mLightParamsList[i].mPosition = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, i);
            mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
            mLightParamsList[i].mPSInDirection = mLightParamsList[i].mDirection;
            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);
            mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS, i);

            needViewPos = true;
            break;
        }

        // Resolve diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR, i);
        }
        else
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED, i);
        }   

        if (mSpecularEnable)
        {
            // Resolve specular colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR, i);
            }
            else
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED, i);
            }   
        }       
    }

    if (needViewPos)
    {
        mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
        mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);

        mViewPos = psMain->resolveInputParameter(mVSOutViewPos);
        mToLight = psMain->resolveLocalParameter(Parameter::SPC_LIGHTDIRECTION_VIEW_SPACE0);
        mToView = psMain->resolveLocalParameter(Parameter::SPC_POSTOCAMERA_VIEW_SPACE);

        for (auto& l : mLightParamsList)
        {
            if(l.mType != Light::LT_POINT && l.mType != Light::LT_SPOTLIGHT)
                continue;
            l.mToLight = mToLight;
        }
    }

    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_TRANSFORM);
    vsProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

    psProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

    if(mNormalisedEnable)
        psProgram->addPreprocessorDefines("NORMALISED");

    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();  

    // Add the global illumination functions.
    addVSInvocation(vsMain->getStage(FFP_VS_LIGHTING));

    auto stage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1);
    // Add the global illumination functions.
    addPSGlobalIlluminationInvocation(stage);

    if (mToView)
        stage.mul(Vector3(-1), mViewPos, mToView);

    // Add per light functions.
    for (const auto& lp : mLightParamsList)
    {
        addIlluminationInvocation(&lp, stage);
    }

    // Assign back temporary variables
    stage.assign(mOutDiffuse, mInDiffuse);

    return true;
}

//-----------------------------------------------------------------------
void PerPixelLighting::addVSInvocation(const FunctionStageRef& stage)
{
    // Transform normal in view space.
    if(!mLightParamsList.empty())
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldViewITMatrix, mVSInNormal, mVSOutNormal);

    // Transform view space position if need to.
    if (mVSOutViewPos)
    {
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldViewMatrix, mVSInPosition, mVSOutViewPos);
    }
}


//-----------------------------------------------------------------------
void PerPixelLighting::addPSGlobalIlluminationInvocation(const FunctionStageRef& stage)
{
    if ((mTrackVertexColourType & TVC_AMBIENT) == 0 && 
        (mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        stage.assign(mDerivedSceneColour, mOutDiffuse);
    }
    else
    {
        if (mTrackVertexColourType & TVC_AMBIENT)
        {
            stage.mul(mLightAmbientColour, mInDiffuse, mOutDiffuse);
        }
        else
        {
            stage.assign(In(mDerivedAmbientLightColour).xyz(), Out(mOutDiffuse).xyz());
        }

        if (mTrackVertexColourType & TVC_EMISSIVE)
        {
            stage.add(mInDiffuse, mOutDiffuse, mOutDiffuse);
        }
        else
        {
            stage.add(mSurfaceEmissiveColour, mOutDiffuse, mOutDiffuse);
        }       
    }
}

//-----------------------------------------------------------------------
const String& PerPixelLightingFactory::getType() const
{
    return PerPixelLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState* PerPixelLightingFactory::createInstance(ScriptCompiler* compiler, 
                                                        PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name != "lighting_stage" || prop->values.empty())
        return NULL;

    auto it = prop->values.begin();
    String val;

    if(!SGScriptTranslator::getString(*it, &val))
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        return NULL;
    }

    SubRenderState* ret = NULL;
    if (val == "per_pixel")
    {
        ret = createOrRetrieveInstance(translator);
    }

    if(ret && prop->values.size() >= 2)
    {
        if(!SGScriptTranslator::getString(*it, &val))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return NULL;
        }

        static_cast<PerPixelLighting*>(ret)->setNormaliseEnabled(val == "normalised");
    }

    return ret;
}

//-----------------------------------------------------------------------
void PerPixelLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
                                            Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("per_pixel");
}

//-----------------------------------------------------------------------
SubRenderState* PerPixelLightingFactory::createInstanceImpl()
{
    return OGRE_NEW PerPixelLighting;
}

}
}

#endif
