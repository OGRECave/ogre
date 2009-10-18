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
#include "OgreMaterialSerializer.h"

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
	mWorldViewMatrix				= NULL;
	mWorldViewITMatrix				= NULL;
	mVSInPosition					= NULL;
	mVSOutViewPos					= NULL;
	mPSInViewPos					= NULL;
	mVSInNormal						= NULL;
	mVSOutNormal					= NULL;
	mPSInNormal						= NULL;
	mPSDiffuse						= NULL;
	mPSSpecular						= NULL;
	mPSTempDiffuseColour			= NULL;
	mPSTempSpecularColour			= NULL;
	mPSOutDiffuse					= NULL;
	mPSOutSpecular					= NULL;
	mDerivedSceneColour				= NULL;
	mLightAmbientColour				= NULL;
	mDerivedAmbientLightColour		= NULL;
	mSurfaceAmbientColour			= NULL;
	mSurfaceDiffuseColour			= NULL;
	mSurfaceSpecularColour			= NULL;	
	mSurfaceEmissiveColour			= NULL;
	mSurfaceShininess				= NULL;		
	mSpeuclarEnable					= false;

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
uint32 PerPixelLighting::getHashCode()
{
	uint32 hashCode = 0;

	sh_hash_combine(hashCode, SubRenderState::getHashCode());

	LightParamsIterator it = mLightParamsList.begin();


	sh_hash_combine(hashCode, mSpeuclarEnable);	

	while(it != mLightParamsList.end())
	{
		sh_hash_combine(hashCode, it->mType);		
		++it;
	}

	sh_hash_combine(hashCode, mTrackVertexColourType);


	return hashCode;
}

//-----------------------------------------------------------------------
void PerPixelLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
	const LightList* pLightList)
{
	if (mLightParamsList.size() == 0)
		return;

	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();
	SceneManager* sceneMgr = ShaderGenerator::getSingleton().getSceneManager();

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
				Real phi   = Math::Cos(srcLight->getSpotlightOuterAngle().valueRadians() * 0.5);
				Real theta = Math::Cos(srcLight->getSpotlightInnerAngle().valueRadians() * 0.5);

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
		if (mSpeuclarEnable)
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
	if (mWorldViewITMatrix == NULL)		
		return false;	

	// Get surface ambient colour if need to.
	if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
	{		
		mDerivedAmbientLightColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR, 0);
		if (mDerivedAmbientLightColour == NULL)		
			return false;
	}
	else
	{
		mLightAmbientColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR, 0);
		if (mLightAmbientColour == NULL)		
			return false;	

		mSurfaceAmbientColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR, 0);
		if (mSurfaceAmbientColour == NULL)		
			return false;	

	}

	// Get surface diffuse colour if need to.
	if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
	{
		mSurfaceDiffuseColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR, 0);
		if (mSurfaceDiffuseColour == NULL)		
			return false;	 
	}

	// Get surface specular colour if need to.
	if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
	{
		mSurfaceSpecularColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR, 0);
		if (mSurfaceSpecularColour == NULL)		
			return false;	 
	}


	// Get surface emissive colour if need to.
	if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		mSurfaceEmissiveColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR, 0);
		if (mSurfaceEmissiveColour == NULL)		
			return false;	 
	}

	// Get derived scene colour.
	mDerivedSceneColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR, 0);
	if (mDerivedSceneColour == NULL)		
		return false;

	// Get surface shininess.
	mSurfaceShininess = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SHININESS, 0);
	if (mSurfaceShininess == NULL)		
		return false;

	// Resolve input vertex shader normal.
	mVSInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, GCT_FLOAT3);
	if (mVSInNormal == NULL)
		return false;

	// Resolve output vertex shader normal.
	mVSOutNormal = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT3);
	if (mVSOutNormal == NULL)
		return false;

	// Resolve input pixel shader normal.
	mPSInNormal = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutNormal->getIndex(), GCT_FLOAT3);
	if (mPSInNormal == NULL)
		return false;

	const ShaderParameterList& inputParams = psMain->getInputParameters();
	const ShaderParameterList& localParams = psMain->getLocalParameters();

	mPSDiffuse = psMain->getParameterBySemantic(inputParams, Parameter::SPS_COLOR, 0);
	if (mPSDiffuse == NULL)
	{
		mPSDiffuse = psMain->getParameterBySemantic(localParams, Parameter::SPS_COLOR, 0);
		if (mPSDiffuse == NULL)
			return false;
	}

	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	if (mPSOutDiffuse == NULL)
		return false;

	mPSTempDiffuseColour = psMain->resolveLocalParameter(Parameter::SPS_COLOR, -1, GCT_FLOAT4, "lDiffusePerPixelColour");
	if (mPSTempDiffuseColour == NULL)
		return false;

	if (mSpeuclarEnable)
	{
		mPSSpecular = psMain->getParameterBySemantic(inputParams, Parameter::SPS_COLOR, 1);
		if (mPSSpecular == NULL)
		{
			mPSSpecular = psMain->getParameterBySemantic(localParams, Parameter::SPS_COLOR, 1);
			if (mPSSpecular == NULL)
				return false;
		}

		mPSTempSpecularColour = psMain->resolveLocalParameter(Parameter::SPS_COLOR, -1, GCT_FLOAT4, "lSpecularPerPixelColour");
		if (mPSTempSpecularColour == NULL)
			return false;


		mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
		if (mVSInPosition == NULL)
			return false;

		mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT3);
		if (mVSOutViewPos == NULL)
			return false;	

		mPSInViewPos = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
			mVSOutViewPos->getIndex(), GCT_FLOAT3);
		if (mPSInViewPos == NULL)
			return false;

		mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
		if (mWorldViewMatrix == NULL)		
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
			if (mLightParamsList[i].mDirection == NULL)
				return false;
			break;

		case Light::LT_POINT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix == NULL)		
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
			if (mVSInPosition == NULL)
				return false;

			mLightParamsList[i].mPosition = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mPosition == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams == NULL)
				return false;	

			if (mVSOutViewPos == NULL)
			{
				mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT3);
				if (mVSOutViewPos == NULL)
					return false;	

				mPSInViewPos = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
					mVSOutViewPos->getIndex(), GCT_FLOAT3);
				if (mPSInViewPos == NULL)
					return false;
			}			
			break;

		case Light::LT_SPOTLIGHT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix == NULL)		
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
			if (mVSInPosition == NULL)
				return false;

			mLightParamsList[i].mPosition = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mPosition == NULL)
				return false;

			mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_view_space");
			if (mLightParamsList[i].mDirection == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams == NULL)
				return false;	

			mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
			if (mLightParamsList[i].mSpotParams == NULL)
				return false;

			if (mVSOutViewPos == NULL)
			{
				mVSOutViewPos = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT3);
				if (mVSOutViewPos == NULL)
					return false;	

				mPSInViewPos = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
					mVSOutViewPos->getIndex(), GCT_FLOAT3);
				if (mPSInViewPos == NULL)
					return false;
			}		
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_diffuse");
			if (mLightParamsList[i].mDiffuseColour == NULL)
				return false;
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_diffuse");
			if (mLightParamsList[i].mDiffuseColour == NULL)
				return false;
		}		

		if (mSpeuclarEnable)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_specular");
				if (mLightParamsList[i].mSpecularColour == NULL)
					return false;
			}
			else
			{
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_specular");
				if (mLightParamsList[i].mSpecularColour == NULL)
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
	curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
	curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
	curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
	curFuncInvocation->getParameterList().push_back(mVSOutNormal->getName());	
	vsMain->addAtomInstace(curFuncInvocation);

	// Transform view space position if need to.
	if (mVSOutViewPos != NULL)
	{
		curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMPOSITION, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mWorldViewMatrix->getName());
		curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());
		curFuncInvocation->getParameterList().push_back(mVSOutViewPos->getName());	
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
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mDerivedSceneColour->getName());
		curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());	
		psMain->addAtomInstace(curFuncInvocation);		
	}
	else
	{
		if (mTrackVertexColourType & TVC_AMBIENT)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mLightAmbientColour->getName());
			curFuncInvocation->getParameterList().push_back(mPSDiffuse->getName());			
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());	
			psMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mDerivedAmbientLightColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);
		}

		if (mTrackVertexColourType & TVC_EMISSIVE)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mPSDiffuse->getName() );
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());	
			psMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mSurfaceEmissiveColour->getName());
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());	
			psMain->addAtomInstace(curFuncInvocation);
		}		
	}

	if (mSpeuclarEnable)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mPSSpecular->getName());
		curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName());	
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
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mPSDiffuse->getName() + ".xyz");	
		curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Merge specular colour with vertex colour if need to.
	if (mSpeuclarEnable && mTrackVertexColourType & TVC_SPECULAR)
	{							
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mPSDiffuse->getName() + ".xyz");	
		curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");
		psMain->addAtomInstace(curFuncInvocation);
	}

	switch (curLightParams->mType)
	{

	case Light::LT_DIRECTIONAL:			
		if (mSpeuclarEnable)
		{				
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mPSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(mPSInViewPos->getName());			
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(mSurfaceShininess->getName());
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);
		}

		else
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mPSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");					
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);	
		}	
		break;

	case Light::LT_POINT:	
		if (mSpeuclarEnable)
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mPSInNormal->getName());			
			curFuncInvocation->getParameterList().push_back(mPSInViewPos->getName());	
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(mSurfaceShininess->getName());
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);		
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mPSInNormal->getName());			
			curFuncInvocation->getParameterList().push_back(mPSInViewPos->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");					
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);	
		}

		break;

	case Light::LT_SPOTLIGHT:
		if (mSpeuclarEnable)
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mPSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(mPSInViewPos->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpotParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(mSurfaceShininess->getName());
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);			
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_LIGHT_SPOT_DIFFUSE, groupOrder, internalCounter++); 						
			curFuncInvocation->getParameterList().push_back(mPSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(mPSInViewPos->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpotParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");					
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName() + ".xyz");	
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

	curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN + 1, internalCounter++); 								
	curFuncInvocation->getParameterList().push_back(mPSTempDiffuseColour->getName());	
	curFuncInvocation->getParameterList().push_back(mPSDiffuse->getName());	
	psMain->addAtomInstace(curFuncInvocation);	

	curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN + 1, internalCounter++); 								
	curFuncInvocation->getParameterList().push_back(mPSDiffuse->getName());	
	curFuncInvocation->getParameterList().push_back(mPSOutDiffuse->getName());	
	psMain->addAtomInstace(curFuncInvocation);

	if (mSpeuclarEnable)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, FFP_PS_COLOUR_BEGIN + 1, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mPSTempSpecularColour->getName());
		curFuncInvocation->getParameterList().push_back(mPSSpecular->getName());			
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

	// Case this pass should run once per light -> override the light policy.
	if (srcPass->getIteratePerLight())
	{		
		if (srcPass->getRunOnlyForOneLightType())
		{
			if (srcPass->getOnlyLightType() == Light::LT_POINT)
			{
				lightCount[0] = 1;
				lightCount[1] = 0;
				lightCount[2] = 0;
			}
			else if (srcPass->getOnlyLightType() == Light::LT_DIRECTIONAL)
			{
				lightCount[0] = 0;
				lightCount[1] = 1;
				lightCount[2] = 0;
			}
			else if (srcPass->getOnlyLightType() == Light::LT_SPOTLIGHT)
			{
				lightCount[0] = 0;
				lightCount[1] = 0;
				lightCount[2] = 1;
			}
		}
		else
		{
			lightCount[0] = 1;
			lightCount[1] = 1;
			lightCount[2] = 1;
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

			curParams.mPosition			= NULL;
			curParams.mDirection		= NULL;
			curParams.mAttenuatParams	= NULL;
			curParams.mSpotParams		= NULL;
			curParams.mDiffuseColour	= NULL;
			curParams.mSpecularColour	= NULL;		

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
	if (prop->name == "light_model")
	{
		if(prop->values.size() == 1)
		{
			String modelType;
			
			if(false == SGScriptTranslator::getString(prop->values.front(), &modelType))
			{
				compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				return NULL;
			}
			
			if (modelType == "sgx_per_pixel")
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
	ser->writeAttribute(4, "light_model");
	ser->writeValue("sgx_per_pixel");
}

//-----------------------------------------------------------------------
SubRenderState*	PerPixelLightingFactory::createInstanceImpl()
{
	return new PerPixelLighting;
}

}
}

