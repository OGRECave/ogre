/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExPerPixelLighting.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"
#include "OgreShaderGenerator.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"

namespace Ogre {
namespace RTShader {

#define SGX_LIB_PERPIXELLIGHTING					"SGXLib_PerPixelLighting"
#define SGX_FUNC_TRANSFORMNORMAL					"SGX_TransformNormal"
#define SGX_FUNC_TRANSFORMPOSITION					"SGX_TransformPosition"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE			"SGX_Light_Directional_Diffuse"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR	"SGX_Light_Directional_DiffuseSpecular"
#define SGX_FUNC_LIGHT_POINT_DIFFUSE				"SGX_Light_Point_Diffuse"
#define SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR		"SGX_Light_Point_DiffuseSpecular"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSE					"SGX_Light_Spot_Diffuse"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR			"SGX_Light_Spot_DiffuseSpecular"
	

/************************************************************************/
/*                                                                      */
/************************************************************************/
String PerPixelLighting::Type = "SGX_PerPixelLighting";
Light PerPixelLighting::msBlankLight;

//-----------------------------------------------------------------------
PerPixelLighting::PerPixelLighting()
{
	mTrackVertexColourType			= TVC_NONE;	
	mSpecularEnable					= false;

	msBlankLight.setDiffuseColour(ColourValue::Black);
	msBlankLight.setSpecularColour(ColourValue::Black);
	msBlankLight.setAttenuation(0,1,0,0);
}

//-----------------------------------------------------------------------
const String& PerPixelLighting::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	PerPixelLighting::getExecutionOrder() const
{
	return FFP_LIGHTING;
}

//-----------------------------------------------------------------------
void PerPixelLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
	const LightList* pLightList)
{
	if (mLightParamsList.size() == 0)
		return;

	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();
	SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();

	Viewport* curViewport = sceneMgr->getCurrentViewport();
	Camera* curCamera     = curViewport->getCamera();
	const Matrix4& matView = curCamera->getViewMatrix(true);
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

		Light*		srcLight = NULL;
		Vector4		vParameter;
		ColourValue colour;

		// Search a matching light from the current sorted lights of the given renderable.
		for (unsigned int j = curSearchLightIndex; j < pLightList->size(); ++j)
		{
			if (pLightList->at(j)->getType() == curLightType)
			{				
				srcLight = pLightList->at(j);
				curSearchLightIndex = j + 1;
				break;
			}			
		}

		// No matching light found -> use a blank dummy light for parameter update.
		if (srcLight == NULL)
		{						
			srcLight = &msBlankLight;
		}


		switch (curParams.mType)
		{
		case Light::LT_DIRECTIONAL:

			// Update light direction.
			vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
			psGpuParams->setNamedConstant(curParams.mDirection->getName(), vParameter);
			break;

		case Light::LT_POINT:

			// Update light position.
			vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
			psGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);

			// Update light attenuation parameters.
			vParameter.x = srcLight->getAttenuationRange();
			vParameter.y = srcLight->getAttenuationConstant();
			vParameter.z = srcLight->getAttenuationLinear();
			vParameter.w = srcLight->getAttenuationQuadric();
			psGpuParams->setNamedConstant(curParams.mAttenuatParams->getName(), vParameter);
			break;

		case Light::LT_SPOTLIGHT:
			{						
				Vector3 vec3;
				Matrix3 matViewIT;

				
				// Update light position.
				vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
				psGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);


				// Update light direction.
				source->getInverseTransposeViewMatrix().extract3x3Matrix(matViewIT);
				vec3 = matViewIT * srcLight->getDerivedDirection();
				vec3.normalise();

				vParameter.x = -vec3.x;
				vParameter.y = -vec3.y;
				vParameter.z = -vec3.z;
				vParameter.w = 0.0;
				psGpuParams->setNamedConstant(curParams.mDirection->getName(), vParameter);

				// Update light attenuation parameters.
				vParameter.x = srcLight->getAttenuationRange();
				vParameter.y = srcLight->getAttenuationConstant();
				vParameter.z = srcLight->getAttenuationLinear();
				vParameter.w = srcLight->getAttenuationQuadric();
				psGpuParams->setNamedConstant(curParams.mAttenuatParams->getName(), vParameter);

				// Update spotlight parameters.
				Real phi   = Math::Cos(srcLight->getSpotlightOuterAngle().valueRadians() * 0.5f);
				Real theta = Math::Cos(srcLight->getSpotlightInnerAngle().valueRadians() * 0.5f);

				vec3.x = theta;
				vec3.y = phi;
				vec3.z = srcLight->getSpotlightFalloff();

				psGpuParams->setNamedConstant(curParams.mSpotParams->getName(), vec3);
			}
			break;
		}


		// Update diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			colour = srcLight->getDiffuseColour() * pass->getDiffuse();
			psGpuParams->setNamedConstant(curParams.mDiffuseColour->getName(), colour);					
		}
		else
		{					
			colour = srcLight->getDiffuseColour();
			psGpuParams->setNamedConstant(curParams.mDiffuseColour->getName(), colour);	
		}

		// Update specular colour if need to.
		if (mSpecularEnable)
		{
			// Update diffuse colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				colour = srcLight->getSpecularColour() * pass->getSpecular();
				psGpuParams->setNamedConstant(curParams.mSpecularColour->getName(), colour);					
			}
			else
			{					
				colour = srcLight->getSpecularColour();
				psGpuParams->setNamedConstant(curParams.mSpecularColour->getName(), colour);	
			}
		}																			
	}
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
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();


	// Resolve world view IT matrix.
	mWorldViewITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX, 0);
	if (mWorldViewITMatrix.get() == NULL)		
		return false;	

	// Get surface ambient colour if need to.
	if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
	{		
		mDerivedAmbientLightColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR, 0);
		if (mDerivedAmbientLightColour.get() == NULL)		
			return false;
	}
	else
	{
		mLightAmbientColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR, 0);
		if (mLightAmbientColour.get() == NULL)		
			return false;	

		mSurfaceAmbientColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR, 0);
		if (mSurfaceAmbientColour.get() == NULL)		
			return false;	

	}

	// Get surface diffuse colour if need to.
	if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
	{
		mSurfaceDiffuseColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR, 0);
		if (mSurfaceDiffuseColour.get() == NULL)		
			return false;	 
	}

	// Get surface specular colour if need to.
	if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
	{
		mSurfaceSpecularColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR, 0);
		if (mSurfaceSpecularColour.get() == NULL)		
			return false;	 
	}


	// Get surface emissive colour if need to.
	if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		mSurfaceEmissiveColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR, 0);
		if (mSurfaceEmissiveColour.get() == NULL)		
			return false;	 
	}

	// Get derived scene colour.
	mDerivedSceneColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR, 0);
	if (mDerivedSceneColour.get() == NULL)		
		return false;

	// Get surface shininess.
	mSurfaceShininess = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SHININESS, 0);
	if (mSurfaceShininess.get() == NULL)		
		return false;

	// Resolve input vertex shader normal.
	mVSInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
	if (mVSInNormal.get() == NULL)
		return false;

	// Resolve output vertex shader normal.
	mVSOutNormal = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, Parameter::SPC_NORMAL_VIEW_SPACE, GCT_FLOAT3);
	if (mVSOutNormal.get() == NULL)
		return false;

	// Resolve input pixel shader normal.
	mPSInNormal = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutNormal->getIndex(), 
		mVSOutNormal->getContent(),
		GCT_FLOAT3);
	if (mPSInNormal.get() == NULL)
		return false;

	const ShaderParameterList& inputParams = psMain->getInputParameters();
	const ShaderParameterList& localParams = psMain->getLocalParameters();

	mPSDiffuse = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSDiffuse.get() == NULL)
	{
		mPSDiffuse = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
		if (mPSDiffuse.get() == NULL)
			return false;
	}

	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSOutDiffuse.get() == NULL)
		return false;

	mPSTempDiffuseColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lPerPixelDiffuse", GCT_FLOAT4);
	if (mPSTempDiffuseColour.get() == NULL)
		return false;

	if (mSpecularEnable)
	{
		mPSSpecular = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
		if (mPSSpecular.get() == NULL)
		{
			mPSSpecular = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
			if (mPSSpecular.get() == NULL)
				return false;
		}

		mPSTempSpecularColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lPerPixelSpecular", GCT_FLOAT4);
		if (mPSTempSpecularColour.get() == NULL)
			return false;


		mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
		if (mVSInPosition.get() == NULL)
			return false;

		mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, Parameter::SPC_POSITION_VIEW_SPACE, GCT_FLOAT3);
		if (mVSOutViewPos.get() == NULL)
			return false;	

		mPSInViewPos = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
			mVSOutViewPos->getIndex(), 
			mVSOutViewPos->getContent(),
			GCT_FLOAT3);
		if (mPSInViewPos.get() == NULL)
			return false;

		mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
		if (mWorldViewMatrix.get() == NULL)		
			return false;									
	}

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolvePerLightParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();


	// Resolve per light parameters.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		switch (mLightParamsList[i].mType)
		{
		case Light::LT_DIRECTIONAL:
			mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
			if (mLightParamsList[i].mDirection.get() == NULL)
				return false;
			break;

		case Light::LT_POINT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix.get() == NULL)		
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0,  Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition.get() == NULL)
				return false;

			mLightParamsList[i].mPosition = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mPosition.get() == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams.get() == NULL)
				return false;	

			if (mVSOutViewPos.get() == NULL)
			{
				mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, Parameter::SPC_POSITION_VIEW_SPACE, GCT_FLOAT3);
				if (mVSOutViewPos.get() == NULL)
					return false;	

				mPSInViewPos = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
					mVSOutViewPos->getIndex(),
					mVSOutViewPos->getContent(),
					GCT_FLOAT3);
				if (mPSInViewPos.get() == NULL)
					return false;
			}			
			break;

		case Light::LT_SPOTLIGHT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix.get() == NULL)		
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition.get() == NULL)
				return false;

			mLightParamsList[i].mPosition = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mPosition.get() == NULL)
				return false;

			mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
			if (mLightParamsList[i].mDirection.get() == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams.get() == NULL)
				return false;	

			mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
			if (mLightParamsList[i].mSpotParams.get() == NULL)
				return false;

			if (mVSOutViewPos.get() == NULL)
			{
				mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, Parameter::SPC_POSITION_VIEW_SPACE, GCT_FLOAT3);
				if (mVSOutViewPos.get() == NULL)
					return false;	

				mPSInViewPos = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
					mVSOutViewPos->getIndex(), 
					mVSOutViewPos->getContent(),
					GCT_FLOAT3);
				if (mPSInViewPos.get() == NULL)
					return false;
			}		
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_diffuse");
			if (mLightParamsList[i].mDiffuseColour.get() == NULL)
				return false;
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_diffuse");
			if (mLightParamsList[i].mDiffuseColour.get() == NULL)
				return false;
		}		

		if (mSpecularEnable)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_specular");
				if (mLightParamsList[i].mSpecularColour.get() == NULL)
					return false;
			}
			else
			{
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_specular");
				if (mLightParamsList[i].mSpecularColour.get() == NULL)
					return false;
			}						
		}		
	}

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

	psProgram->addDependency(FFP_LIB_COMMON);
	psProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();	
	Function* vsMain = vsProgram->getEntryPointFunction();	
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* psMain = psProgram->getEntryPointFunction();	

	int internalCounter = 0;


	// Add the global illumination functions.
	if (false == addVSInvocation(vsMain, FFP_VS_LIGHTING, internalCounter))
		return false;


	internalCounter = 0;

	// Add the global illumination functions.
	if (false == addPSGlobalIlluminationInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1, internalCounter))
		return false;


	// Add per light functions.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		if (false == addPSIlluminationInvocation(&mLightParamsList[i], psMain, FFP_PS_COLOUR_BEGIN + 1, internalCounter))
			return false;
	}

	// Assign back temporary variables to the ps diffuse and specular components.
	if (false == addPSFinalAssignmentInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1, internalCounter))
		return false;


	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addVSInvocation(Function* vsMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;

	// Transform normal in view space.
	curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
	curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mVSOutNormal, Operand::OPS_OUT);	
	vsMain->addAtomInstace(curFuncInvocation);

	// Transform view space position if need to.
	if (mVSOutViewPos.get() != NULL)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMPOSITION, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSOutViewPos, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);
	}
	

	return true;
}


//-----------------------------------------------------------------------
bool PerPixelLighting::addPSGlobalIlluminationInvocation(Function* psMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;	

	if ((mTrackVertexColourType & TVC_AMBIENT) == 0 && 
		(mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mDerivedSceneColour, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT);	
		psMain->addAtomInstace(curFuncInvocation);		
	}
	else
	{
		if (mTrackVertexColourType & TVC_AMBIENT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mLightAmbientColour, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT);	
			psMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mDerivedAmbientLightColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);
		}

		if (mTrackVertexColourType & TVC_EMISSIVE)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT);	
			psMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mSurfaceEmissiveColour, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT);	
			psMain->addAtomInstace(curFuncInvocation);
		}		
	}

	if (mSpecularEnable)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mPSSpecular, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT);	
		psMain->addAtomInstace(curFuncInvocation);	
	}
	
	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter)
{	
	FunctionInvocation* curFuncInvocation = NULL;	

	// Merge diffuse colour with vertex colour if need to.
	if (mTrackVertexColourType & TVC_DIFFUSE)			
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
		curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Merge specular colour with vertex colour if need to.
	if (mSpecularEnable && mTrackVertexColourType & TVC_SPECULAR)
	{							
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
		curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		psMain->addAtomInstace(curFuncInvocation);
	}

	switch (curLightParams->mType)
	{

	case Light::LT_DIRECTIONAL:			
		if (mSpecularEnable)
		{				
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSInViewPos, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);
		}

		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));					
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);	
		}	
		break;

	case Light::LT_POINT:	
		if (mSpecularEnable)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mPSInViewPos, Operand::OPS_IN);	
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);		
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mPSInViewPos, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));					
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);	
		}

		break;

	case Light::LT_SPOTLIGHT:
		if (mSpecularEnable)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSInViewPos, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);			
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_SPOT_DIFFUSE, groupOrder, internalCounter++); 						
			curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSInViewPos, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));					
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			psMain->addAtomInstace(curFuncInvocation);	
		}
		break;
	}

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addPSFinalAssignmentInvocation( Function* psMain, const int groupOrder, int& internalCounter )
{
	FunctionInvocation* curFuncInvocation;

	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN + 1, internalCounter++); 								
	curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_OUT);	
	psMain->addAtomInstace(curFuncInvocation);	

	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN + 1, internalCounter++); 								
	curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);	
	curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT);	
	psMain->addAtomInstace(curFuncInvocation);

	if (mSpecularEnable)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN + 1, internalCounter++); 
		curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSSpecular, Operand::OPS_OUT);			
		psMain->addAtomInstace(curFuncInvocation);	
	}

	return true;
}


//-----------------------------------------------------------------------
void PerPixelLighting::copyFrom(const SubRenderState& rhs)
{
	const PerPixelLighting& rhsLighting = static_cast<const PerPixelLighting&>(rhs);

	int lightCount[3];

	rhsLighting.getLightCount(lightCount);
	setLightCount(lightCount);
}

//-----------------------------------------------------------------------
bool PerPixelLighting::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
	if (srcPass->getLightingEnabled() == false)
		return false;

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
				"Using iterative lighting method with RT Shader System requires specifieng explicit light type.",
				"PerPixelLighting::preAddToRenderState");			
		}
	}

	setLightCount(lightCount);

	return true;
}

//-----------------------------------------------------------------------
void PerPixelLighting::setLightCount(const int lightCount[3])
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
void PerPixelLighting::getLightCount(int lightCount[3]) const
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
const String& PerPixelLightingFactory::getType() const
{
	return PerPixelLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	PerPixelLightingFactory::createInstance(ScriptCompiler* compiler, 
														PropertyAbstractNode* prop, Pass* pass)
{
	if (prop->name == "lighting_stage")
	{
		if(prop->values.size() == 1)
		{
			String modelType;
			
			if(false == SGScriptTranslator::getString(prop->values.front(), &modelType))
			{
				compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				return NULL;
			}
			
			if (modelType == "per_pixel")
			{
				return SubRenderStateFactory::createInstance();
			}
		}		
	}

	return NULL;
}

//-----------------------------------------------------------------------
void PerPixelLightingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
											Pass* srcPass, Pass* dstPass)
{
	ser->writeAttribute(4, "lighting_stage");
	ser->writeValue("per_pixel");
}

//-----------------------------------------------------------------------
SubRenderState*	PerPixelLightingFactory::createInstanceImpl()
{
	return OGRE_NEW PerPixelLighting;
}

}
}

#endif