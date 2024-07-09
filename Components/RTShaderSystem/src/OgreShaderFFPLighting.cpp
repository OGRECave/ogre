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
	mLightCount						= 0;
	mLtcLUT1SamplerIndex            = -1;
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

	// Resolve input vertex shader normal.
    mVSInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
	
	if (mTrackVertexColourType != 0)
	{
		mInDiffuse = vsMain->resolveInputParameter(Parameter::SPC_COLOR_DIFFUSE);
	}
	

	// Resolve output vertex shader diffuse colour.
	mOutDiffuse = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
	
	// Resolve per light parameters.
	mPositions = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY, mLightCount);
	mAttenuatParams = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION_ARRAY, mLightCount);
	mDirections = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY, mLightCount);
	mSpotParams = vsProgram->resolveParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS_ARRAY, mLightCount);

	// Resolve diffuse colour.
	if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
	{
		mDiffuseColours = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY, mLightCount);
	}
	else
	{
		mDiffuseColours = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY, mLightCount);
	}

	if (mSpecularEnable)
	{
		// Get surface shininess.
		mSurfaceShininess = vsProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SHININESS);

		// Resolve specular colour.
		if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
		{
			mSpecularColours = vsProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY, mLightCount);
		}
		else
		{
			mSpecularColours = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY, mLightCount);
		}

		if (mOutSpecular.get() == NULL)
		{
			mOutSpecular = vsMain->resolveOutputParameter(Parameter::SPC_COLOR_SPECULAR);
		}
	}

	//if(needViewPos)
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

	addDefines(vsProgram);

	if(mSpecularEnable)
		programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM)->addPreprocessorDefines("USE_SPECULAR");

	return true;
}

void FFPLighting::addDefines(Program* program)
{
	if(mNormalisedEnable)
	    program->addPreprocessorDefines("NORMALISED");

	if(mSpecularEnable)
		program->addPreprocessorDefines("USE_SPECULAR");

	if(mTrackVertexColourType & TVC_DIFFUSE)
		program->addPreprocessorDefines("TVC_DIFFUSE");

	if(mTrackVertexColourType & TVC_SPECULAR)
		program->addPreprocessorDefines("TVC_SPECULAR");

	program->addPreprocessorDefines("LIGHT_COUNT=" + StringConverter::toString(mLightCount));
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
	for (int i = 0; i < mLightCount; i++)
	{
		addIlluminationInvocation(i, stage);
	}

    auto psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    auto psMain = psProgram->getMain();
    if (auto shadowFactor = psMain->getLocalParameter("lShadowFactor"))
    {
        auto ambient = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);
        auto psOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

        auto fstage = psMain->getStage(FFP_PS_COLOUR_BEGIN);

		std::vector<Operand> args = {In(ambient), In(shadowFactor),InOut(psOutDiffuse)};

        if (mSpecularEnable)
        {
            auto psSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
            if (!psSpecular)
                psSpecular = psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
			args.push_back(InOut(psSpecular));
        }

        fstage.callFunction("SGX_ApplyShadowFactor_Modulative", args);
    }

    return true;
}

//-----------------------------------------------------------------------
void FFPLighting::addGlobalIlluminationInvocation(const FunctionStageRef& stage)
{
    // Transform normal to view space
	if(mLightCount)
	    stage.callBuiltin("mul", mWorldViewITMatrix, mVSInNormal, mViewNormal);

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
void FFPLighting::addIlluminationInvocation(int i, const FunctionStageRef& stage)
{
    std::vector<Operand> args = {In(mViewNormal),         In(mViewPos), In(mPositions),      At(i),
                                 In(mAttenuatParams),     At(i),        In(mDirections),     At(i),
                                 In(mSpotParams),         At(i),        In(mDiffuseColours), At(i),
                                 InOut(mOutDiffuse).xyz()};

    if (mTrackVertexColourType & (TVC_DIFFUSE | TVC_SPECULAR))
    {
		args.push_back(In(mInDiffuse));
    }

    if (mSpecularEnable)
    {
		args.insert(args.end(), {In(mSpecularColours), At(i), In(mSurfaceShininess), InOut(mOutSpecular).xyz()});
	}

	if (mShadowFactor)
	{
		if(i < int(mShadowFactor->getSize()))
			args.insert(args.end(), {In(mShadowFactor), At(i)});
		else
			args.push_back(In(1));
	}

	if(mLTCLUT1)
	{
		args.insert(args.end(), {In(mLTCLUT1), In(mLTCLUT2)});
	}

	stage.callFunction("evaluateLight", args);
}


//-----------------------------------------------------------------------
void FFPLighting::copyFrom(const SubRenderState& rhs)
{
	const FFPLighting& rhsLighting = static_cast<const FFPLighting&>(rhs);

	mLightCount 	  = rhsLighting.mLightCount;
	mNormalisedEnable = rhsLighting.mNormalisedEnable;
	mTwoSidedLighting = rhsLighting.mTwoSidedLighting;
}

uint16 ensureLtcLUTPresent(Pass* dstPass)
{
	auto tus = dstPass->getTextureUnitState("ltc_1.dds");
	// return idx of existing texture unit
	if(tus)
		return dstPass->getTextureUnitStateIndex(tus);

	auto ltcSampler = TextureManager::getSingleton().getSampler("Ogre/LtcLUTSampler");
	tus = dstPass->createTextureUnitState("ltc_1.dds");
	tus->setNumMipmaps(0);
	tus->setName("ltc_1.dds");
	tus->setSampler(ltcSampler);
	tus = dstPass->createTextureUnitState("ltc_2.dds");
	tus->setNumMipmaps(0);
	tus->setSampler(ltcSampler);

	return dstPass->getNumTextureUnitStates() - 2; // idx of first LUT
}

//-----------------------------------------------------------------------
bool FFPLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    //! [disable]
	if (!srcPass->getLightingEnabled())
		return false;
	//! [disable]

	mLightCount = renderState->getLightCount();
	
	setTrackVertexColourType(srcPass->getVertexColourTracking());

	mSpecularEnable = srcPass->getShininess() > 0.0 &&
		(srcPass->getSpecular() != ColourValue::Black || (srcPass->getVertexColourTracking() & TVC_SPECULAR) != 0);

	// Case this pass should run once per light(s) -> override the light policy.
	if (srcPass->getIteratePerLight())
	{		
		mLightCount = srcPass->getLightCountPerIteration();
	}

	if(srcPass->getMaxSimultaneousLights() == 0)
	{
		mLightCount = 0;
	}

	if (renderState->haveAreaLights())
		mLtcLUT1SamplerIndex = ensureLtcLUTPresent(dstPass);

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
