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
    bool hasError = false;
    // Resolve world view IT matrix.
    mWorldViewITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX);
    hasError |= !(mWorldViewITMatrix.get());    
    
    // Get surface ambient colour if need to.
    if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
    {       
        mDerivedAmbientLightColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
        hasError |= !(mDerivedAmbientLightColour.get());        
    }
    else
    {
        mLightAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
        mSurfaceAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
        
        hasError |= !(mSurfaceAmbientColour.get()) || !(mLightAmbientColour.get()); 
    }

    // Get surface diffuse colour if need to.
    if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
    {
        mSurfaceDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
        hasError |= !(mSurfaceDiffuseColour.get()); 
    }

    // Get surface specular colour if need to.
    if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
    {
        mSurfaceSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
        hasError |= !(mSurfaceSpecularColour.get());    
    }


    // Get surface emissive colour if need to.
    if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        mSurfaceEmissiveColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
        hasError |= !(mSurfaceEmissiveColour.get());    
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

    mOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    hasError |= !(mDerivedSceneColour.get()) || !(mSurfaceShininess.get()) || !(mVSInNormal.get()) || !(mVSOutNormal.get()) || !(mViewNormal.get()) || !(
        mInDiffuse.get()) || !(mOutDiffuse.get());
    
    if (mSpecularEnable)
    {
        mPSSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
        if (mPSSpecular.get() == NULL)
        {
            mPSSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);
        }

        mOutSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);

        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

        mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);

        mViewPos = psMain->resolveInputParameter(mVSOutViewPos);

        mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);

        hasError |= !(mPSSpecular.get()) || !(mOutSpecular.get()) || !(mVSInPosition.get()) || !(mVSOutViewPos.get()) ||
            !(mViewPos.get()) || !(mWorldViewMatrix.get());
    }

    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "PerPixelLighting::resolveGlobalParameters" );
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
    bool hasError = false;

    // Resolve per light parameters.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        switch (mLightParamsList[i].mType)
        {
        case Light::LT_DIRECTIONAL:
            mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
            break;

        case Light::LT_POINT:
            mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            mLightParamsList[i].mPosition = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
            
            if (mVSOutViewPos.get() == NULL)
            {
                mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);

                mViewPos = psMain->resolveInputParameter(mVSOutViewPos);
            }   
            
            hasError |= !(mWorldViewMatrix.get()) || !(mVSInPosition.get()) || !(mLightParamsList[i].mPosition.get()) || 
                !(mLightParamsList[i].mAttenuatParams.get()) || !(mVSOutViewPos.get()) || !(mViewPos.get());
        
            break;

        case Light::LT_SPOTLIGHT:
            mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);

            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            mLightParamsList[i].mPosition = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
            mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");

            mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");

            if (mVSOutViewPos.get() == NULL)
            {
                mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_VIEW_SPACE);

                mViewPos = psMain->resolveInputParameter(mVSOutViewPos);
            }   

            hasError |= !(mWorldViewMatrix.get()) || !(mVSInPosition.get()) || !(mLightParamsList[i].mPosition.get()) || 
                !(mLightParamsList[i].mDirection.get()) || !(mLightParamsList[i].mAttenuatParams.get()) || 
                !(mLightParamsList[i].mSpotParams.get()) || !(mVSOutViewPos.get()) || !(mViewPos.get());
            
            break;
        }

        // Resolve diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_diffuse");
        }
        else
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_diffuse");
        }   

        hasError |= !(mLightParamsList[i].mDiffuseColour.get());

        if (mSpecularEnable)
        {
            // Resolve specular colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_specular");
            }
            else
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_specular");
            }   
            hasError |= !(mLightParamsList[i].mSpecularColour.get());
        }       
    }
        
    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "PerPixelLighting::resolvePerLightParameters" );
    }
    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

    psProgram->addDependency(FFP_LIB_COMMON);
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
    if (false == addVSInvocation(vsMain, FFP_VS_LIGHTING))
        return false;

    // Add the global illumination functions.
    if (false == addPSGlobalIlluminationInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    // Add per light functions.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        if (false == addIlluminationInvocation(&mLightParamsList[i], psMain, FFP_PS_COLOUR_BEGIN + 1))
            return false;
    }

    // Assign back temporary variables to the ps diffuse and specular components.
    if (false == addPSFinalAssignmentInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addVSInvocation(Function* vsMain, const int groupOrder)
{
    auto stage = vsMain->getStage(groupOrder);

    // Transform normal in view space.
    if(!mLightParamsList.empty())
        stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mWorldViewITMatrix, mVSInNormal, mVSOutNormal);

    // Transform view space position if need to.
    if (mVSOutViewPos)
    {
        stage.callFunction(SGX_FUNC_TRANSFORMPOSITION, mWorldViewMatrix, mVSInPosition, mVSOutViewPos);
    }

    return true;
}


//-----------------------------------------------------------------------
bool PerPixelLighting::addPSGlobalIlluminationInvocation(Function* psMain, const int groupOrder)
{
    auto stage = psMain->getStage(groupOrder);

    if ((mTrackVertexColourType & TVC_AMBIENT) == 0 && 
        (mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        stage.assign(mDerivedSceneColour, mOutDiffuse);
    }
    else
    {
        if (mTrackVertexColourType & TVC_AMBIENT)
        {
            stage.callFunction(FFP_FUNC_MODULATE, mLightAmbientColour, mInDiffuse, mOutDiffuse);
        }
        else
        {
            stage.assign(In(mDerivedAmbientLightColour).xyz(), Out(mOutDiffuse).xyz());
        }

        if (mTrackVertexColourType & TVC_EMISSIVE)
        {
            stage.callFunction(FFP_FUNC_ADD, mInDiffuse, mOutDiffuse, mOutDiffuse);
        }
        else
        {
            stage.callFunction(FFP_FUNC_ADD, mSurfaceEmissiveColour, mOutDiffuse, mOutDiffuse);
        }       
    }

    if (mSpecularEnable)
    {
        stage.assign(mPSSpecular, mOutSpecular);
    }
    
    return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addPSFinalAssignmentInvocation( Function* psMain, const int groupOrder)
{
    auto stage = psMain->getStage(groupOrder);
    stage.assign(mOutDiffuse, mInDiffuse);

    if (mSpecularEnable)
    {
        stage.assign(mOutSpecular, mPSSpecular);
    }

    return true;
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
