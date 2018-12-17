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
String FFPLighting::Type = "FFP_Lighting";

//-----------------------------------------------------------------------
FFPLighting::FFPLighting()
{
	mTrackVertexColourType			= TVC_NONE;
	mSpecularEnable					= false;
	mNormalisedEnable               = false;
}

//-----------------------------------------------------------------------
const String& FFPLighting::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	FFPLighting::getExecutionOrder() const
{
	return FFP_LIGHTING;
}

//-----------------------------------------------------------------------
void FFPLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
										  const LightList* pLightList)
{		
	if (mLightParamsList.empty())
		return;

	const Affine3& matView = source->getViewMatrix();
	Light::LightTypes curLightType = Light::LT_DIRECTIONAL; 
	unsigned int curSearchLightIndex = 0;

	// Update per light parameters.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{
		const LightParams& curParams = mLightParamsList[i];

		if (curLightType != curParams.mType)
		{
			curLightType = curParams.mType;
			curSearchLightIndex = 0;
		}

		Vector4		vParameter;
		ColourValue colour;

		// Search a matching light from the current sorted lights of the given renderable.
		size_t j;
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

			// Update light direction.
			vParameter = matView * source->getLightAs4DVector(j);
			curParams.mDirection->setGpuParameter(vParameter);
			break;

		case Light::LT_POINT:
			// Update light position.
			vParameter = matView * source->getLightAs4DVector(j);
			curParams.mPosition->setGpuParameter(vParameter);
			// Update light attenuation parameters.
			vParameter = source->getLightAttenuation(j);
			curParams.mAttenuatParams->setGpuParameter(vParameter);
			break;

		case Light::LT_SPOTLIGHT:
		{						
			Vector3 vec3;
			// Update light position.
			vParameter = matView * source->getLightAs4DVector(j);
			curParams.mPosition->setGpuParameter(vParameter);
			
							
			vec3 = source->getInverseTransposeViewMatrix().linear() * source->getLightDirection(j);
			vec3.normalise();

			vParameter.x = -vec3.x;
			vParameter.y = -vec3.y;
			vParameter.z = -vec3.z;
			vParameter.w = 0.0;
			curParams.mDirection->setGpuParameter(vParameter);

			// Update light attenuation parameters.
			vParameter = source->getLightAttenuation(j);
			curParams.mAttenuatParams->setGpuParameter(vParameter);

			// Update spotlight parameters.
			vec3 = source->getSpotlightParams(j).xyz();
			curParams.mSpotParams->setGpuParameter(vec3);
		}
			break;
		}

		
		// Update diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			colour = pass->getDiffuse() * source->getLightDiffuseColourWithPower(j);
			curParams.mDiffuseColour->setGpuParameter(colour);					
		}
		else
		{					
			colour = source->getLightDiffuseColourWithPower(j);
			curParams.mDiffuseColour->setGpuParameter(colour);	
		}

		// Update specular colour if need to.
		if (mSpecularEnable)
		{
			// Update diffuse colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				colour = pass->getSpecular() * source->getLightSpecularColourWithPower(j);
				curParams.mSpecularColour->setGpuParameter(colour);					
			}
			else
			{					
				colour = source->getLightSpecularColourWithPower(j);
				curParams.mSpecularColour->setGpuParameter(colour);	
			}
		}																			
	}
}

//-----------------------------------------------------------------------
bool FFPLighting::resolveParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
	Function* vsMain = vsProgram->getEntryPointFunction();
	bool hasError = false;

	// Resolve world view IT matrix.
	mWorldViewITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX);
	mViewNormal = vsMain->resolveLocalParameter(Parameter::SPC_NORMAL_VIEW_SPACE);
	
	// Get surface ambient colour if need to.
	if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
	{		
		mDerivedAmbientLightColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
		hasError |= !(mDerivedAmbientLightColour.get());
	}
	else
	{
		mLightAmbientColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
		mSurfaceAmbientColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
		
		hasError |= !(mLightAmbientColour.get()) || !(mSurfaceAmbientColour.get());
	}

	// Get surface diffuse colour if need to.
	if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
	{
		mSurfaceDiffuseColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
		hasError |= !(mSurfaceDiffuseColour.get());
	}

	// Get surface specular colour if need to.
	if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
	{
		mSurfaceSpecularColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
		hasError |= !(mSurfaceSpecularColour.get());
	}
		 
	
	// Get surface emissive colour if need to.
	if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		mSurfaceEmissiveColour = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
		hasError |= !(mSurfaceEmissiveColour.get());
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
		hasError |= !(mInDiffuse.get());
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
			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			hasError |= !(mLightParamsList[i].mDirection.get());
			break;
		
		case Light::LT_POINT:
		    needViewPos = true;
			
			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			
			hasError |= !(mLightParamsList[i].mPosition.get()) || !(mLightParamsList[i].mAttenuatParams.get());
			break;
		
		case Light::LT_SPOTLIGHT:
		    needViewPos = true;
			
			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			
			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
			
			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			
			mLightParamsList[i].mSpotParams = vsProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
			
			hasError |=  !(mLightParamsList[i].mPosition.get()) ||
				!(mLightParamsList[i].mDirection.get()) || !(mLightParamsList[i].mAttenuatParams.get()) || !(mLightParamsList[i].mSpotParams.get());
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL | (uint16)GPV_LIGHTS, "derived_light_diffuse");
			hasError |= !(mLightParamsList[i].mDiffuseColour.get());
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_diffuse");
			hasError |= !(mLightParamsList[i].mDiffuseColour.get());
		}		

		if (mSpecularEnable)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL | (uint16)GPV_LIGHTS, "derived_light_specular");
				hasError |= !(mLightParamsList[i].mSpecularColour.get());
			}
			else
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_specular");
				hasError |= !(mLightParamsList[i].mSpecularColour.get());
			}

			if (mOutSpecular.get() == NULL)
			{
				mOutSpecular = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_SPECULAR);
				hasError |= !(mOutSpecular.get());
			}
			
			if (mVSInPosition.get() == NULL)
			{
				mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
				hasError |= !(mVSInPosition.get());
			}

			if (mWorldViewMatrix.get() == NULL)
			{
				mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
				hasError |= !(mWorldViewMatrix.get());
			}
		}		
	}

	if(needViewPos)
	{
        mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
        mViewPos = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_VIEW_SPACE);
        hasError |= !(mWorldViewMatrix.get()) || !(mVSInPosition.get()) || !(mViewPos.get());
	}

	hasError |= !(mWorldViewITMatrix.get()) || !(mDerivedSceneColour.get()) || !(mSurfaceShininess.get()) || 
		!(mVSInNormal.get()) || !(mOutDiffuse.get());
	
	
	if (hasError)
	{
		OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Not all parameters could be constructed for the sub-render state.",
				"PerPixelLighting::resolveGlobalParameters" );
	}

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);

	vsProgram->addDependency(FFP_LIB_COMMON);
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

	// Add the global illumination functions.
	if (false == addGlobalIlluminationInvocation(vsMain, FFP_VS_LIGHTING))
		return false;

	// Add per light functions.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		if (false == addIlluminationInvocation(&mLightParamsList[i], vsMain, FFP_VS_LIGHTING))
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::addGlobalIlluminationInvocation(Function* vsMain, const int groupOrder)
{
	auto stage = vsMain->getStage(groupOrder);

    // Transform normal to view space
	if(!mLightParamsList.empty())
	    stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mWorldViewITMatrix, mVSInNormal, mViewNormal);

    if(mViewPos)
    {
        // Transform position to view space.
        stage.callFunction(SGX_FUNC_TRANSFORMPOSITION, mWorldViewMatrix, mVSInPosition, mViewPos);
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
            stage.callFunction(FFP_FUNC_MODULATE, mLightAmbientColour, mInDiffuse, mOutDiffuse);
		}
		else
		{
		    stage.assign(mDerivedAmbientLightColour, mOutDiffuse);
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

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::addIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder)
{
    auto stage = vsMain->getStage(groupOrder);

    // Merge diffuse colour with vertex colour if need to.
    if (mTrackVertexColourType & TVC_DIFFUSE)
    {
        stage.callFunction(FFP_FUNC_MODULATE, In(mInDiffuse).xyz(), In(curLightParams->mDiffuseColour).xyz(),
                           Out(curLightParams->mDiffuseColour).xyz());
    }

    // Merge specular colour with vertex colour if need to.
    if (mSpecularEnable && mTrackVertexColourType & TVC_SPECULAR)
    {
        stage.callFunction(FFP_FUNC_MODULATE, In(mInDiffuse).xyz(), In(curLightParams->mSpecularColour).xyz(),
                           Out(curLightParams->mSpecularColour).xyz());
    }

    switch (curLightParams->mType)
    {
    case Light::LT_DIRECTIONAL:
        if (mSpecularEnable)
        {
            stage.callFunction(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mDirection).xyz(),
                                In(curLightParams->mDiffuseColour).xyz(), In(curLightParams->mSpecularColour).xyz(),
                                In(mSurfaceShininess), In(mOutDiffuse).xyz(), In(mOutSpecular).xyz(),
                                Out(mOutDiffuse).xyz(), Out(mOutSpecular).xyz()});
        }

        else
        {
            stage.callFunction(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE,
                               {In(mViewNormal), In(curLightParams->mDirection).xyz(),
                                In(curLightParams->mDiffuseColour).xyz(), In(mOutDiffuse).xyz(),
                                Out(mOutDiffuse).xyz()});
        }
        break;

    case Light::LT_POINT:
        if (mSpecularEnable)
        {
            stage.callFunction(SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mAttenuatParams), In(curLightParams->mDiffuseColour).xyz(),
                                In(curLightParams->mSpecularColour).xyz(), In(mSurfaceShininess),
                                In(mOutDiffuse).xyz(), In(mOutSpecular).xyz(), Out(mOutDiffuse).xyz(),
                                Out(mOutSpecular).xyz()});
        }
        else
        {
            stage.callFunction(SGX_FUNC_LIGHT_POINT_DIFFUSE,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mAttenuatParams), In(curLightParams->mDiffuseColour).xyz(),
                                In(mOutDiffuse).xyz(), Out(mOutDiffuse).xyz()});
        }
				
        break;

    case Light::LT_SPOTLIGHT:
        if (mSpecularEnable)
        {
            stage.callFunction(SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mDirection).xyz(), In(curLightParams->mAttenuatParams),
                                In(curLightParams->mSpotParams), In(curLightParams->mDiffuseColour).xyz(),
                                In(curLightParams->mSpecularColour).xyz(), In(mSurfaceShininess),
                                In(mOutDiffuse).xyz(), In(mOutSpecular).xyz(), Out(mOutDiffuse).xyz(),
                                Out(mOutSpecular).xyz()});
        }
        else
        {
            stage.callFunction(SGX_FUNC_LIGHT_SPOT_DIFFUSE,
                               {In(mViewNormal), In(mViewPos), In(curLightParams->mPosition).xyz(),
                                In(curLightParams->mDirection).xyz(), In(curLightParams->mAttenuatParams),
                                In(curLightParams->mSpotParams), In(curLightParams->mDiffuseColour).xyz(),
                                In(mOutDiffuse).xyz(), Out(mOutDiffuse).xyz()});
        }
        break;
    }

    return true;
}


//-----------------------------------------------------------------------
void FFPLighting::copyFrom(const SubRenderState& rhs)
{
	const FFPLighting& rhsLighting = static_cast<const FFPLighting&>(rhs);

	int lightCount[3];

	rhsLighting.getLightCount(lightCount);
	setLightCount(lightCount);

	mNormalisedEnable = rhsLighting.mNormalisedEnable;
}

//-----------------------------------------------------------------------
bool FFPLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    //! [disable]
	if (!srcPass->getLightingEnabled())
		return false;
	//! [disable]

	int lightCount[3];

	renderState->getLightCount(lightCount);
	
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

	setLightCount(lightCount);

	return true;
}

//-----------------------------------------------------------------------
void FFPLighting::setLightCount(const int lightCount[3])
{
	for (int type=0; type < 3; ++type)
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
void FFPLighting::getLightCount(int lightCount[3]) const
{
	lightCount[0] = 0;
	lightCount[1] = 0;
	lightCount[2] = 0;

	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{
		const LightParams curParams = mLightParamsList[i];

		if (curParams.mType == Light::LT_POINT)
			lightCount[0]++;
		else if (curParams.mType == Light::LT_DIRECTIONAL)
			lightCount[1]++;
		else if (curParams.mType == Light::LT_SPOTLIGHT)
			lightCount[2]++;
	}
}

//-----------------------------------------------------------------------
const String& FFPLightingFactory::getType() const
{
	return FFPLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	FFPLightingFactory::createInstance(ScriptCompiler* compiler, 
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
    if (val == "ffp")
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

        static_cast<FFPLighting*>(ret)->setNormaliseEnabled(val == "normalised");
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
