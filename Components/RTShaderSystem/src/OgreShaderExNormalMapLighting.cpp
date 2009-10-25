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
#include "OgreShaderExNormalMapLighting.h"
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

#define SGX_LIB_NORMALMAPLIGHTING					"SGXLib_NormalMapLighting"
#define SGX_FUNC_CONSTRUCT_TBNMATRIX				"SGX_ConstructTBNMatrix"
#define SGX_FUNC_TRANSFORMNORMAL					"SGX_TransformNormal"
#define SGX_FUNC_TRANSFORMPOSITION					"SGX_TransformPosition"
#define SGX_FUNC_FETCHNORMAL						"SGX_FetchNormal"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE			"SGX_Light_Directional_Diffuse"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR	"SGX_Light_Directional_DiffuseSpecular"
#define SGX_FUNC_LIGHT_POINT_DIFFUSE				"SGX_Light_Point_Diffuse"
#define SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR		"SGX_Light_Point_DiffuseSpecular"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSE					"SGX_Light_Spot_Diffuse"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR			"SGX_Light_Spot_DiffuseSpecular"
	

/************************************************************************/
/*                                                                      */
/************************************************************************/
String NormalMapLighting::Type						= "SGX_NormalMapLighting";
String NormalMapLighting::NormalMapTextureNameKey   = "SGNormalMapTextureName";

Light NormalMapLighting::msBlankLight;

//-----------------------------------------------------------------------
NormalMapLighting::NormalMapLighting()
{
	mTrackVertexColourType			= TVC_NONE;
	mCamPosWorldSpace				= NULL;
	mVSInPosition					= NULL;
	mVSWorldPosition				= NULL;
	mVSOutView						= NULL;
	mPSInView						= NULL;
	mVSInNormal						= NULL;
	mVSInTangent					= NULL;
	mVSLocalDir						= NULL;
	mPSNormal						= NULL;
	mPSDiffuse						= NULL;
	mPSSpecular						= NULL;
	mPSTempDiffuseColour			= NULL;
	mPSTempSpecularColour			= NULL;
	mPSOutDiffuse					= NULL;
	mPSOutSpecular					= NULL;
	mVSInTexcoord					= NULL;
	mVSOutTexcoord					= NULL;
	mPSInTexcoord					= NULL;
	mDerivedSceneColour				= NULL;
	mLightAmbientColour				= NULL;
	mDerivedAmbientLightColour		= NULL;
	mSurfaceAmbientColour			= NULL;
	mSurfaceDiffuseColour			= NULL;
	mSurfaceSpecularColour			= NULL;	
	mSurfaceEmissiveColour			= NULL;
	mSurfaceShininess				= NULL;	
	mVSTBNMatrix					= NULL;		
	mNormalMapSampler				= NULL;
	mWorldMatrix					= NULL;
	mWorldInvRotMatrix				= NULL;
	mNormalMapSamplerIndex			= 0;
	mVSTexCoordSetIndex				= 0;
	mSpeuclarEnable					= false;
	mNormalMapSpace					= NMS_TANGENT;

	msBlankLight.setDiffuseColour(ColourValue::Black);
	msBlankLight.setSpecularColour(ColourValue::Black);
	msBlankLight.setAttenuation(0,1,0,0);
}

//-----------------------------------------------------------------------
const String& NormalMapLighting::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	NormalMapLighting::getExecutionOrder() const
{
	return FFP_LIGHTING;
}

//-----------------------------------------------------------------------
uint32 NormalMapLighting::getHashCode()
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
	sh_hash_combine(hashCode, mNormalMapSamplerIndex);	
	sh_hash_combine(hashCode, mVSTexCoordSetIndex);
	sh_hash_combine(hashCode, mNormalMapSpace);

	return hashCode;
}

//-----------------------------------------------------------------------
void NormalMapLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
	const LightList* pLightList)
{		
	if (mLightParamsList.size() == 0)
		return;

	GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();
	SceneManager* sceneMgr = ShaderGenerator::getSingleton().getSceneManager();	
	const Matrix4& matWorldInv	= source->getInverseWorldMatrix();	
	Light::LightTypes curLightType = Light::LT_DIRECTIONAL; 
	unsigned int curSearchLightIndex = 0;
	const Matrix4& matWorld = source->getWorldMatrix();
	Matrix3 matWorldInvRotation;
	Vector3 vRow0(matWorld[0][0], matWorld[0][1], matWorld[0][2]); 
	Vector3 vRow1(matWorld[1][0], matWorld[1][1], matWorld[1][2]); 
	Vector3 vRow2(matWorld[2][0], matWorld[2][1], matWorld[2][2]); 

	vRow0.normalise();
	vRow1.normalise();
	vRow2.normalise();

	matWorldInvRotation.SetColumn(0, vRow0);
	matWorldInvRotation.SetColumn(1, vRow1);
	matWorldInvRotation.SetColumn(2, vRow2);

	// Update inverse rotation parameter.
	if (mWorldInvRotMatrix != NULL)	
		vsGpuParams->setNamedConstant(mWorldInvRotMatrix->getName(), matWorldInvRotation);	
		
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
			{						
				Vector3 vec3;							

				// Update light direction. (Object space).
				vec3 = matWorldInvRotation * srcLight->getDerivedDirection();				
				vec3.normalise();

				vParameter.x = -vec3.x;
				vParameter.y = -vec3.y;
				vParameter.z = -vec3.z;
				vParameter.w = 0.0;
				vsGpuParams->setNamedConstant(curParams.mDirection->getName(), vParameter);
			}
			break;

		case Light::LT_POINT:

			// Update light position. (World space).				
			vParameter = srcLight->getAs4DVector(true);
			vsGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);

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
											
				// Update light position. (World space).				
				vParameter = srcLight->getAs4DVector(true);
				vsGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);

							
				// Update light direction. (Object space).
				vec3 = matWorldInvRotation * srcLight->getDerivedDirection();				
				vec3.normalise();
			
				vParameter.x = -vec3.x;
				vParameter.y = -vec3.y;
				vParameter.z = -vec3.z;
				vParameter.w = 0.0;
				vsGpuParams->setNamedConstant(curParams.mDirection->getName(), vParameter);							
				
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
bool NormalMapLighting::resolveParameters(ProgramSet* programSet)
{
	if (false == resolveGlobalParameters(programSet))
		return false;
	
	if (false == resolvePerLightParameters(programSet))
		return false;
	
	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolveGlobalParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();
	
	// Resolve normal map texture sampler parameter.		
	mNormalMapSampler = psProgram->resolveParameter(GCT_SAMPLER2D, mNormalMapSamplerIndex, (uint16)GPV_PER_OBJECT, "gNormalMapSampler");
	if (mNormalMapSampler == NULL)
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
	mVSInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
	if (mVSInNormal == NULL)
		return false;

	// Resolve input vertex shader tangent.
	if (mNormalMapSpace == NMS_TANGENT)
	{
		mVSInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT, GCT_FLOAT3);
		if (mVSInTangent == NULL)
			return false;

		// Resolve local vertex shader TNB matrix.
		mVSTBNMatrix = vsMain->resolveLocalParameter("lMatTBN", GCT_MATRIX_3X3);
		if (mVSTBNMatrix == NULL)
			return false;
	}
	
	// Resolve input vertex shader texture coordinates.
	mVSInTexcoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSTexCoordSetIndex, 
		Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex),
		GCT_FLOAT2);
	if (mVSInTexcoord == NULL)
		return false;

	// Resolve output vertex shader texture coordinates.
	mVSOutTexcoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
		Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex),
		GCT_FLOAT2);
	if (mVSOutTexcoord == NULL)
		return false;

	
	// Resolve pixel input texture coordinates normal.
	mPSInTexcoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutTexcoord->getIndex(), 
		mVSOutTexcoord->getContent(),
		mVSOutTexcoord->getType());
	if (mPSInTexcoord == NULL)
		return false;

	// Resolve pixel shader normal.
	if (mNormalMapSpace == NMS_OBJECT)
	{
		mPSNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
		if (mPSNormal == NULL)
			return false;
	}
	else if (mNormalMapSpace == NMS_TANGENT)
	{
		mPSNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_TANGENT_SPACE, GCT_FLOAT3);
		if (mPSNormal == NULL)
			return false;
	}
	

	const ShaderParameterList& inputParams = psMain->getInputParameters();
	const ShaderParameterList& localParams = psMain->getLocalParameters();

	mPSDiffuse = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSDiffuse == NULL)
	{
		mPSDiffuse = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
		if (mPSDiffuse == NULL)
			return false;
	}

	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSOutDiffuse == NULL)
		return false;

	mPSTempDiffuseColour = psMain->resolveLocalParameter("lNormalMapDiffuse", GCT_FLOAT4);
	if (mPSTempDiffuseColour == NULL)
		return false;

	if (mSpeuclarEnable)
	{
		mPSSpecular = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
		if (mPSSpecular == NULL)
		{
			mPSSpecular = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
			if (mPSSpecular == NULL)
				return false;
		}

		mPSTempSpecularColour = psMain->resolveLocalParameter("lNormalMapSpecular", GCT_FLOAT4);
		if (mPSTempSpecularColour == NULL)
			return false;


		mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
		if (mVSInPosition == NULL)
			return false;

		if (mNormalMapSpace == NMS_TANGENT)
		{
			mVSOutView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
				Parameter::SPC_POSTOCAMERA_TANGENT_SPACE, GCT_FLOAT3);
			if (mVSOutView == NULL)
				return false;	
		}
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			mVSOutView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
				Parameter::SPC_POSTOCAMERA_OBJECT_SPACE, GCT_FLOAT3);
			if (mVSOutView == NULL)
				return false;
		}
		

		mPSInView = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
			mVSOutView->getIndex(), 
			mVSOutView->getContent(),
			mVSOutView->getType());
		if (mPSInView == NULL)
			return false;	

		// Resolve camera position world space.
		mCamPosWorldSpace = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_CAMERA_POSITION, 0);
		if (mCamPosWorldSpace == NULL)		
			return false;	
		
		mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
		if (mVSLocalDir == NULL)
			return false;	

		mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
		if (mVSWorldPosition == NULL)
			return false;

		// Resolve world matrix.				
		mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		if (mWorldMatrix == NULL)		
			return false;	

		// Resolve inverse world rotation matrix.
		mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "gMatInvRotation");
		if (mWorldInvRotMatrix == NULL)		
			return false;	
		
	}
	
	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolvePerLightParameters(ProgramSet* programSet)
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
			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_obj_space");
			if (mLightParamsList[i].mDirection == NULL)
				return false;

			if (mNormalMapSpace == NMS_TANGENT)
			{
				mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
					Parameter::Content(Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0 + i),
					GCT_FLOAT3);
			}
			else if (mNormalMapSpace == NMS_OBJECT)
			{
				mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
					Parameter::Content(Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0 + i),
					GCT_FLOAT3);
			}
			if (mLightParamsList[i].mVSOutDirection == NULL)
				return false;

			mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutDirection->getIndex(), 
				mLightParamsList[i].mVSOutDirection->getContent(), 
				mLightParamsList[i].mVSOutDirection->getType());
			if (mLightParamsList[i].mPSInDirection == NULL)
				return false;

			break;

		case Light::LT_POINT:		
			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_world_space");
			if (mLightParamsList[i].mPosition == NULL)
				return false;

			if (mNormalMapSpace == NMS_TANGENT)
			{
				mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
					Parameter::Content(Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0 + i),
					GCT_FLOAT3);
			}
			else if (mNormalMapSpace == NMS_OBJECT)
			{
				mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
					Parameter::Content(Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0 + i),
					GCT_FLOAT3);
			}
			
			if (mLightParamsList[i].mVSOutToLightDir == NULL)
				return false;

			mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutToLightDir->getIndex(), 
				mLightParamsList[i].mVSOutToLightDir->getContent(), 
				mLightParamsList[i].mVSOutToLightDir->getType());
			if (mLightParamsList[i].mPSInToLightDir == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams == NULL)
				return false;	

			// Resolve local dir.
			if (mVSLocalDir == NULL)
			{
				mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
				if (mVSLocalDir == NULL)
					return false;	
			}	

			// Resolve world position.
			if (mVSWorldPosition == NULL)
			{
				mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
				if (mVSWorldPosition == NULL)
					return false;	
			}	

			// Resolve world matrix.
			if (mWorldMatrix == NULL)
			{				
				mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
				if (mWorldMatrix == NULL)		
					return false;	
			}

			// Resolve inverse world rotation matrix.
			if (mWorldInvRotMatrix == NULL)
			{				
				mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_GLOBAL, "inv_world_rotation_matrix");
				if (mWorldInvRotMatrix == NULL)		
					return false;	
			}				
			break;

		case Light::LT_SPOTLIGHT:		
			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_world_space");
			if (mLightParamsList[i].mPosition == NULL)
				return false;

			if (mNormalMapSpace == NMS_TANGENT)
			{
				mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
					Parameter::Content(Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0 + i),
					GCT_FLOAT3);
			}
			else if (mNormalMapSpace == NMS_OBJECT)
			{
				mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
					Parameter::Content(Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0 + i),
					GCT_FLOAT3);
			}
			if (mLightParamsList[i].mVSOutToLightDir == NULL)
				return false;

			mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutToLightDir->getIndex(), 
				mLightParamsList[i].mVSOutToLightDir->getContent(), 
				mLightParamsList[i].mVSOutToLightDir->getType());
			if (mLightParamsList[i].mPSInToLightDir == NULL)
				return false;

			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_obj_space");
			if (mLightParamsList[i].mDirection == NULL)
				return false;

			if (mNormalMapSpace == NMS_TANGENT)
			{
				mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
					Parameter::Content(Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0 + i),
					GCT_FLOAT3);
			}
			else if (mNormalMapSpace == NMS_OBJECT)
			{
				mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
					Parameter::Content(Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0 + i),
					GCT_FLOAT3);
			}
			if (mLightParamsList[i].mVSOutDirection == NULL)
				return false;


			mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutDirection->getIndex(), 
				mLightParamsList[i].mVSOutDirection->getContent(), 
				mLightParamsList[i].mVSOutDirection->getType());
			if (mLightParamsList[i].mPSInDirection == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams == NULL)
				return false;	

			mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
			if (mLightParamsList[i].mSpotParams == NULL)
				return false;

			// Resolve local dir.
			if (mVSLocalDir == NULL)
			{
				mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
				if (mVSLocalDir == NULL)
					return false;	
			}	

			// Resolve world position.
			if (mVSWorldPosition == NULL)
			{
				mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
				if (mVSWorldPosition == NULL)
					return false;	
			}	

			// Resolve world matrix.
			if (mWorldMatrix == NULL)
			{				
				mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
				if (mWorldMatrix == NULL)		
					return false;	
			}

			// Resolve inverse world rotation matrix.
			if (mWorldInvRotMatrix == NULL)
			{				
				mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "gMatInvRotation");
				if (mWorldInvRotMatrix == NULL)		
					return false;	
			}						
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "derived_light_diffuse");
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
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "derived_light_specular");
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
bool NormalMapLighting::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(SGX_LIB_NORMALMAPLIGHTING);

	psProgram->addDependency(FFP_LIB_COMMON);
	psProgram->addDependency(SGX_LIB_NORMALMAPLIGHTING);

	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addFunctionInvocations(ProgramSet* programSet)
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


	// Add the normal fetch function invocation.
	if (false == addPSNormalFetchInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1, internalCounter))
		return false;

	
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
bool NormalMapLighting::addVSInvocation(Function* vsMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;

	// Construct TNB matrix.
	if (mNormalMapSpace == NMS_TANGENT)
	{
		curFuncInvocation = new FunctionInvocation(SGX_FUNC_CONSTRUCT_TBNMATRIX, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
		curFuncInvocation->getParameterList().push_back(mVSInTangent->getName());
		curFuncInvocation->getParameterList().push_back(mVSTBNMatrix->getName());	
		vsMain->addAtomInstace(curFuncInvocation);
	}
	

	// Output texture coordinates.
	curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
	curFuncInvocation->getParameterList().push_back(mVSInTexcoord->getName());
	curFuncInvocation->getParameterList().push_back(mVSOutTexcoord->getName());	
	vsMain->addAtomInstace(curFuncInvocation);

	// Compute world space position.
	if (mVSWorldPosition != NULL)
	{
		curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMPOSITION, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mWorldMatrix->getName());
		curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());	
		curFuncInvocation->getParameterList().push_back(mVSWorldPosition->getName());	
		vsMain->addAtomInstace(curFuncInvocation);
	}
	

	// Compute view vector.
	if (mVSInPosition != NULL && 
		mVSOutView != NULL)
	{	
		// View vector in world space.
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mCamPosWorldSpace->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(mVSWorldPosition->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to object space.
		curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mWorldInvRotMatrix->getName());
		curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());	
		curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to tangent space.
		if (mNormalMapSpace == NMS_TANGENT)
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mVSTBNMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutView->getName());	
			vsMain->addAtomInstace(curFuncInvocation);
		}

		// Output object space.
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutView->getName());					
			vsMain->addAtomInstace(curFuncInvocation);
		}
	}

	// Add per light functions.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		if (false == addVSIlluminationInvocation(&mLightParamsList[i], vsMain, groupOrder, internalCounter))
			return false;
	}


	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addVSIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;

	// Compute light direction in texture space.
	if (curLightParams->mDirection != NULL &&
		curLightParams->mVSOutDirection != NULL)
	{
		// Transform to texture space.
		if (mNormalMapSpace == NMS_TANGENT)
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mVSTBNMatrix->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mVSOutDirection->getName());	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		// Output object space.
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mVSOutDirection->getName());					
			vsMain->addAtomInstace(curFuncInvocation);
		}
	}
	
	// Transform light vector to target space..
	if (curLightParams->mPosition != NULL &&
		curLightParams->mVSOutToLightDir != NULL)
	{
		// Compute light vector.
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(mVSWorldPosition->getName());	
		curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to object space.
		curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mWorldInvRotMatrix->getName());
		curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());	
		curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to tangent space.		
		if (mNormalMapSpace == NMS_TANGENT)
		{
			curFuncInvocation = new FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mVSTBNMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mVSOutToLightDir->getName());	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		
		// Output object space.
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mVSLocalDir->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mVSOutToLightDir->getName());					
			vsMain->addAtomInstace(curFuncInvocation);
		}
	}


	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addPSNormalFetchInvocation(Function* psMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;	

	curFuncInvocation = new FunctionInvocation(SGX_FUNC_FETCHNORMAL, groupOrder, internalCounter++); 
	curFuncInvocation->getParameterList().push_back(mNormalMapSampler->getName());
	curFuncInvocation->getParameterList().push_back(mPSInTexcoord->getName());
	curFuncInvocation->getParameterList().push_back(mPSNormal->getName());	
	psMain->addAtomInstace(curFuncInvocation);		

	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addPSGlobalIlluminationInvocation(Function* psMain, const int groupOrder, int& internalCounter)
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
bool NormalMapLighting::addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter)
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
			curFuncInvocation->getParameterList().push_back(mPSNormal->getName());
			curFuncInvocation->getParameterList().push_back(mPSInView->getName());			
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInDirection->getName() + ".xyz");
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
			curFuncInvocation->getParameterList().push_back(mPSNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInDirection->getName() + ".xyz");
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
			curFuncInvocation->getParameterList().push_back(mPSNormal->getName());			
			curFuncInvocation->getParameterList().push_back(mPSInView->getName());	
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInToLightDir->getName() + ".xyz");
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
			curFuncInvocation->getParameterList().push_back(mPSNormal->getName());						
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInToLightDir->getName() + ".xyz");
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
			curFuncInvocation->getParameterList().push_back(mPSNormal->getName());
			curFuncInvocation->getParameterList().push_back(mPSInView->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInToLightDir->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInDirection->getName() + ".xyz");
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
			curFuncInvocation->getParameterList().push_back(mPSNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInToLightDir->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mPSInDirection->getName() + ".xyz");
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
bool NormalMapLighting::addPSFinalAssignmentInvocation( Function* psMain, const int groupOrder, int& internalCounter )
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
void NormalMapLighting::copyFrom(const SubRenderState& rhs)
{
	const NormalMapLighting& rhsLighting = static_cast<const NormalMapLighting&>(rhs);

	int lightCount[3];

	rhsLighting.getLightCount(lightCount);
	setLightCount(lightCount);

	mTrackVertexColourType = rhsLighting.mTrackVertexColourType;
	mSpeuclarEnable = rhsLighting.mSpeuclarEnable;
	mNormalMapSpace = rhsLighting.mNormalMapSpace;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
	if (srcPass->getLightingEnabled() == false)
		return false;

	int lightCount[3];

	renderState->getLightCount(lightCount);

	const Any& passUserData = srcPass->getUserObjectBindings().getUserAny(NormalMapLighting::NormalMapTextureNameKey);

	if (passUserData.isEmpty())	
		return false;	

	const String normalMapTextureName = any_cast<const String>(passUserData);

	TextureUnitState* normalMapTexture = dstPass->createTextureUnitState();

	normalMapTexture->setTextureName(normalMapTextureName);	
	normalMapTexture->setTextureMipmapBias(-1.0);
	mNormalMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

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
void NormalMapLighting::preRemoveFromRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass) 
{	
	// Erase the normal map texture association.
	srcPass->getUserObjectBindings().eraseUserAny(NormalMapLighting::NormalMapTextureNameKey);	
}

//-----------------------------------------------------------------------
void NormalMapLighting::setLightCount(const int lightCount[3])
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
			curParams.mVSOutDirection	= NULL;
			curParams.mPSInDirection	= NULL;
			curParams.mVSOutToLightDir	= NULL;
			curParams.mPSInToLightDir	= NULL;

			mLightParamsList.push_back(curParams);
		}
	}			
}

//-----------------------------------------------------------------------
void NormalMapLighting::getLightCount(int lightCount[3]) const
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
const String& NormalMapLightingFactory::getType() const
{
	return NormalMapLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	NormalMapLightingFactory::createInstance(ScriptCompiler* compiler, 
														PropertyAbstractNode* prop, Pass* pass)
{
	if (prop->name == "light_model")
	{
		if(prop->values.size() >= 2)
		{
			String strValue;
			AbstractNodeList::const_iterator it = prop->values.begin();
			
			// Read light model type.
			if(false == SGScriptTranslator::getString(*it, &strValue))
			{
				compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				return NULL;
			}

			// Case light model type us normal map
			if (strValue == "sgx_normal_map")
			{
				++it;
				if (false == SGScriptTranslator::getString(*it, &strValue))
				{
					compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					return NULL;
				}

				unsigned int textureCoordinateIndex = 0;
				SubRenderState* subRenderState = SubRenderStateFactory::createInstance();
				
				pass->getUserObjectBindings().setUserAny(NormalMapLighting::NormalMapTextureNameKey, Any(strValue));

				// Read normal map space type.
				if (prop->values.size() >= 3)
				{
					NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);

					++it;
					if (false == SGScriptTranslator::getString(*it, &strValue))
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return NULL;
					}

					// Normal map defines normals in tangent space.
					if (strValue == "tangent_space")
					{
						normalMapSubRenderState->setNormalMapSpace(NormalMapLighting::NMS_TANGENT);
					}

					// Normal map defines normals in object space.
					if (strValue == "object_space")
					{
						normalMapSubRenderState->setNormalMapSpace(NormalMapLighting::NMS_OBJECT);
					}
				}

				// Read texture coordinate index.
				if (prop->values.size() >= 4)
				{
					NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);

					++it;
					if (SGScriptTranslator::getUInt(*it, &textureCoordinateIndex))
					{
						normalMapSubRenderState->setTexCoordIndex(textureCoordinateIndex);
					}
				}
								
				return subRenderState;								
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------
void NormalMapLightingFactory::writeInstance(MaterialSerializer* ser, 
											 SubRenderState* subRenderState, 
											 Pass* srcPass, Pass* dstPass)
{
	const Any& passUserData = srcPass->getUserObjectBindings().getUserAny(NormalMapLighting::NormalMapTextureNameKey);
	if (passUserData.isEmpty())	
		return;	

	const String normalMapTextureName = any_cast<const String>(passUserData);

	ser->writeAttribute(4, "light_model");
	ser->writeValue("sgx_normal_map");
	ser->writeValue(normalMapTextureName);	

	NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);

	if (normalMapSubRenderState->getNormalMapSpace() == NormalMapLighting::NMS_TANGENT)
	{
		ser->writeValue("tangent_space");
	}
	else if (normalMapSubRenderState->getNormalMapSpace() == NormalMapLighting::NMS_OBJECT)
	{
		ser->writeValue("object_space");
	}

	ser->writeValue(StringConverter::toString(normalMapSubRenderState->getTexCoordIndex()));
}

//-----------------------------------------------------------------------
SubRenderState*	NormalMapLightingFactory::createInstanceImpl()
{
	return new NormalMapLighting;
}

}
}

