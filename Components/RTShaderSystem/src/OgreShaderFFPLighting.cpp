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
#if defined(RTSHADER_SYSTEM_BUILD_CORE_SHADERS) || defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
const String SRS_PER_VERTEX_LIGHTING = "FFP_Lighting";

//-----------------------------------------------------------------------
FFPLighting::FFPLighting()
{
	mTrackVertexColourType			= TVC_NONE;
	mSpecularEnable					= false;
	mNormalisedEnable               = false;
	mTwoSidedLighting               = false;
}

//-----------------------------------------------------------------------
const String& FFPLighting::getType() const
{
	return SRS_PER_VERTEX_LIGHTING;
}


//-----------------------------------------------------------------------
int	FFPLighting::getExecutionOrder() const
{
	return FFP_LIGHTING;
}

//-----------------------------------------------------------------------
void FFPLighting::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
										  const LightList* pLightList)
{		
	if (mLightParamsList.empty())
		return;

	Light::LightTypes curLightType = Light::LT_DIRECTIONAL; 
	unsigned int curSearchLightIndex = 0;

	// Update per light parameters.
	for (auto & curParams : mLightParamsList)
	{
			if (curLightType != curParams.mType)
		{
			curLightType = curParams.mType;
			curSearchLightIndex = 0;
		}

		// Search a matching light from the current sorted lights of the given renderable.
		uint32 j;
		for (j = curSearchLightIndex; j < (pLightList ? pLightList->size() : 0); ++j)
		{
			if (pLightList->at(j)->getType() == curLightType)
			{
				curSearchLightIndex = j + 1;
				break;
			}			
		}

		switch (curParams.mType)
		{
		case Light::LT_DIRECTIONAL:
		    // update light index. data will be set by scene manager
			curParams.mDirection->updateExtraInfo(j);
			break;

		case Light::LT_POINT:
			// update light index. data will be set by scene manager
			curParams.mPosition->updateExtraInfo(j);
			curParams.mAttenuatParams->updateExtraInfo(j);
			break;

		case Light::LT_SPOTLIGHT:
		{						
			// update light index. data will be set by scene manager
			curParams.mPosition->updateExtraInfo(j);
            curParams.mAttenuatParams->updateExtraInfo(j);
            curParams.mSpotParams->updateExtraInfo(j);
			curParams.mDirection->updateExtraInfo(j);
		}
			break;
		}

		
		// Update diffuse colour.
        curParams.mDiffuseColour->updateExtraInfo(j);

		// Update specular colour if need to.
		if (mSpecularEnable)
		{
            curParams.mSpecularColour->updateExtraInfo(j);
		}																			
	}
}

//-----------------------------------------------------------------------
bool FFPLighting::resolveParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
	Function* vsMain = vsProgram->getEntryPointFunction();

	// Resolve world view IT matrix.
	mWorldViewITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
	mViewNormal = vsMain->resolveLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
	
	// Get surface ambient colour if need to.
	if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
	{		
		mDerivedAmbientLightColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
	}
	else
	{
		mLightAmbientColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
	}
	
	// Get surface emissive colour if need to.
	if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		mSurfaceEmissiveColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
	}

	// Get derived scene colour.
	mDerivedSceneColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);
	
	// Get surface shininess.
	mSurfaceShininess = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SHININESS);
	
	// Resolve input vertex shader normal.
    mVSInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
	
	if (mTrackVertexColourType != 0)
	{
		mInDiffuse = vsMain->resolveInputParameter(Parameter::SPC_COLOR_DIFFUSE);
	}
	

	// Resolve output vertex shader diffuse colour.
	mOutDiffuse = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
	
	// Resolve per light parameters.
	bool needViewPos = mSpecularEnable;
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		switch (mLightParamsList[i].mType)
		{
		case Light::LT_DIRECTIONAL:
			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE, i);
			mLightParamsList[i].mPSInDirection = mLightParamsList[i].mDirection;
			break;
		
		case Light::LT_POINT:
		    needViewPos = true;
			
			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, i);
			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);
			break;
		
		case Light::LT_SPOTLIGHT:
		    needViewPos = true;
			
			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, i);
            mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);

			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE, i);
			mLightParamsList[i].mPSInDirection = mLightParamsList[i].mDirection;

			mLightParamsList[i].mSpotParams = vsProgram->resolveParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS, i);

			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR, i);
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED, i);
		}		

		if (mSpecularEnable)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR, i);
			}
			else
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED, i);
			}

			if (mOutSpecular.get() == NULL)
			{
				mOutSpecular = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_SPECULAR);
			}
		}		
	}

	if(needViewPos)
	{
        mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
        mViewPos = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_VIEW_SPACE);
	}

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);

	vsProgram->addDependency(FFP_LIB_TRANSFORM);
	vsProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

	if(mNormalisedEnable)
	    vsProgram->addPreprocessorDefines("NORMALISED");

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);	
	Function* vsMain = vsProgram->getEntryPointFunction();	
    auto stage = vsMain->getStage(FFP_VS_LIGHTING);

	// Add the global illumination functions.
	addGlobalIlluminationInvocation(stage);

    // Add per light functions.
    for (const auto& lp : mLightParamsList)
    {
        addIlluminationInvocation(&lp, stage);
    }

    auto psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    auto psMain = psProgram->getMain();
    if (auto shadowFactor = psMain->getLocalParameter("lShadowFactor"))
    {
        auto ambient = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);
        auto psOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

        auto fstage = psMain->getStage(FFP_PS_COLOUR_BEGIN);
        fstage.callFunction("SGX_ApplyShadowFactor_Diffuse", {In(ambient), In(shadowFactor), InOut(psOutDiffuse)});
        if (mSpecularEnable)
        {
            auto psSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
            if (!psSpecular)
                psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
            fstage.mul(psSpecular, shadowFactor, psSpecular);
        }
    }

    return true;
}

//-----------------------------------------------------------------------
void FFPLighting::addGlobalIlluminationInvocation(const FunctionStageRef& stage)
{
    // Transform normal to view space
	if(!mLightParamsList.empty())
	    stage.callFunction(FFP_FUNC_TRANSFORM, mWorldViewITMatrix, mVSInNormal, mViewNormal);

    if(mViewPos)
    {
        // Transform position to view space.
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldViewMatrix, mVSInPosition, mViewPos);
    }

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
		    stage.assign(mDerivedAmbientLightColour, mOutDiffuse);
		}

		if (mTrackVertexColourType & TVC_EMISSIVE)
		{
			stage.add(In(mInDiffuse).xyz(), In(mOutDiffuse).xyz(), Out(mOutDiffuse).xyz());
		}
		else
		{
            stage.add(mSurfaceEmissiveColour, mOutDiffuse, mOutDiffuse);
		}		
	}
}

//-----------------------------------------------------------------------
void FFPLighting::addIlluminationInvocation(const LightParams* curLightParams, const FunctionStageRef& stage)
{
    // Merge diffuse colour with vertex colour if need to.
    if (mTrackVertexColourType & TVC_DIFFUSE)
    {
        stage.mul(In(mInDiffuse).xyz(), In(curLightParams->mDiffuseColour).xyz(),
                  Out(curLightParams->mDiffuseColour).xyz());
    }

    // Merge specular colour with vertex colour if need to.
    if (mSpecularEnable && mTrackVertexColourType & TVC_SPECULAR)
    {
        stage.mul(In(mInDiffuse).xyz(), In(curLightParams->mSpecularColour).xyz(),
                  Out(curLightParams->mSpecularColour).xyz());
    }

    switch (curLightParams->mType)
    {
    case Light::LT_DIRECTIONAL:
        if (mSpecularEnable)
        {
            stage.callFunction(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPSInDirection).xyz(),
                                In(curLightParams->mDiffuseColour).xyz(), In(curLightParams->mSpecularColour).xyz(),
                                In(mSurfaceShininess), InOut(mOutDiffuse).xyz(), InOut(mOutSpecular).xyz()});
        }

        else
        {
            stage.callFunction(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE,
                               {In(mViewNormal), In(curLightParams->mPSInDirection).xyz(),
                                In(curLightParams->mDiffuseColour).xyz(), InOut(mOutDiffuse).xyz()});
        }
        break;

    case Light::LT_POINT:
        if (mSpecularEnable)
        {
            stage.callFunction(SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mAttenuatParams), In(curLightParams->mDiffuseColour).xyz(),
                                In(curLightParams->mSpecularColour).xyz(), In(mSurfaceShininess),
                                InOut(mOutDiffuse).xyz(), InOut(mOutSpecular).xyz()});
        }
        else
        {
            stage.callFunction(SGX_FUNC_LIGHT_POINT_DIFFUSE,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mAttenuatParams), In(curLightParams->mDiffuseColour).xyz(),
                                InOut(mOutDiffuse).xyz()});
        }
				
        break;

    case Light::LT_SPOTLIGHT:
        if (mSpecularEnable)
        {
            stage.callFunction(SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mPSInDirection).xyz(), In(curLightParams->mAttenuatParams),
                                In(curLightParams->mSpotParams).xyz(), In(curLightParams->mDiffuseColour).xyz(),
                                In(curLightParams->mSpecularColour).xyz(), In(mSurfaceShininess),
                                InOut(mOutDiffuse).xyz(), InOut(mOutSpecular).xyz()});
        }
        else
        {
            stage.callFunction(SGX_FUNC_LIGHT_SPOT_DIFFUSE,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mPSInDirection).xyz(), In(curLightParams->mAttenuatParams),
                                In(curLightParams->mSpotParams).xyz(), In(curLightParams->mDiffuseColour).xyz(),
                                InOut(mOutDiffuse).xyz()});
        }
        break;
    }
}


//-----------------------------------------------------------------------
void FFPLighting::copyFrom(const SubRenderState& rhs)
{
	const FFPLighting& rhsLighting = static_cast<const FFPLighting&>(rhs);

	setLightCount(rhsLighting.getLightCount());
	mNormalisedEnable = rhsLighting.mNormalisedEnable;
	mTwoSidedLighting = rhsLighting.mTwoSidedLighting;
}

//-----------------------------------------------------------------------
bool FFPLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    //! [disable]
	if (!srcPass->getLightingEnabled())
		return false;
	//! [disable]

	auto lightCount = renderState->getLightCount();
	
	setTrackVertexColourType(srcPass->getVertexColourTracking());			

	if (srcPass->getShininess() > 0.0 &&
		srcPass->getSpecular() != ColourValue::Black)
	{
		setSpecularEnable(true);
	}
	else
	{
		setSpecularEnable(false);	
	}

	// Case this pass should run once per light(s) -> override the light policy.
	if (srcPass->getIteratePerLight())
	{		

		// This is the preferred case -> only one type of light is handled.
		if (srcPass->getRunOnlyForOneLightType())
		{
			if (srcPass->getOnlyLightType() == Light::LT_POINT)
			{
				lightCount[0] = srcPass->getLightCountPerIteration();
				lightCount[1] = 0;
				lightCount[2] = 0;
			}
			else if (srcPass->getOnlyLightType() == Light::LT_DIRECTIONAL)
			{
				lightCount[0] = 0;
				lightCount[1] = srcPass->getLightCountPerIteration();
				lightCount[2] = 0;
			}
			else if (srcPass->getOnlyLightType() == Light::LT_SPOTLIGHT)
			{
				lightCount[0] = 0;
				lightCount[1] = 0;
				lightCount[2] = srcPass->getLightCountPerIteration();
			}
		}

		// This is worse case -> all light types expected to be handled.
		// Can not handle this request in efficient way - throw an exception.
		else
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Using iterative lighting method with RT Shader System requires specifying explicit light type.",
				"FFPLighting::preAddToRenderState");			
		}
	}

	if(srcPass->getMaxSimultaneousLights() == 0)
	{
		lightCount = Vector3i(0);
	}

	setLightCount(lightCount);

	return true;
}

bool FFPLighting::setParameter(const String& name, const String& value)
{
	if(name == "normalise" || name == "normalised") // allow both spelling variations
	{
		return StringConverter::parse(value, mNormalisedEnable);
	}

	return false;
}

//-----------------------------------------------------------------------
void FFPLighting::setLightCount(const Vector3i& lightCount)
{
	for (int type : {1, 0, 2}) // directional first
	{
		for (int i=0; i < lightCount[type]; ++i)
		{
			LightParams curParams;

			if (type == 0)
				curParams.mType = Light::LT_POINT;
			else if (type == 1)
				curParams.mType = Light::LT_DIRECTIONAL;
			else if (type == 2)
				curParams.mType = Light::LT_SPOTLIGHT;

			mLightParamsList.push_back(curParams);
		}
	}			
}

//-----------------------------------------------------------------------
Vector3i FFPLighting::getLightCount() const
{
	Vector3i lightCount(0, 0, 0);

	for (const auto& curParams : mLightParamsList)
	{
			if (curParams.mType == Light::LT_POINT)
			lightCount[0]++;
		else if (curParams.mType == Light::LT_DIRECTIONAL)
			lightCount[1]++;
		else if (curParams.mType == Light::LT_SPOTLIGHT)
			lightCount[2]++;
	}

	return lightCount;
}

//-----------------------------------------------------------------------
const String& FFPLightingFactory::getType() const
{
	return SRS_PER_VERTEX_LIGHTING;
}

//-----------------------------------------------------------------------
SubRenderState*	FFPLightingFactory::createInstance(ScriptCompiler* compiler, 
												PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name != "lighting_stage" || prop->values.empty())
        return NULL;

    auto it = prop->values.begin();

    SubRenderState* ret = NULL;
    if ((*it++)->getString() == "ffp")
    {
        ret = createOrRetrieveInstance(translator);
    }

    if(ret && prop->values.size() >= 2)
    {
        ret->setParameter((*it)->getString(), "true"); // normalise
    }

    return ret;
}

//-----------------------------------------------------------------------
void FFPLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
											Pass* srcPass, Pass* dstPass)
{
	ser->writeAttribute(4, "lighting_stage");
	ser->writeValue("ffp");
}

//-----------------------------------------------------------------------
SubRenderState*	FFPLightingFactory::createInstanceImpl()
{
	return OGRE_NEW FFPLighting;
}

}
}

#endif
