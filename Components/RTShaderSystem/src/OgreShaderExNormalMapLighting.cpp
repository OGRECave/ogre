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
	mNormalMapSamplerIndex			= 0;
	mVSTexCoordSetIndex				= 0;
	mSpecularEnable					= false;
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
void NormalMapLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
	const LightList* pLightList)
{		
	if (mLightParamsList.size() == 0)
		return;

	GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();
	SceneManager* sceneMgr = ShaderGenerator::getSingleton().getActiveSceneManager();	
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
	if (mWorldInvRotMatrix.get() != NULL)	
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
	if (mNormalMapSampler.get() == NULL)
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

	// Resolve input vertex shader tangent.
	if (mNormalMapSpace == NMS_TANGENT)
	{
		mVSInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT, GCT_FLOAT3);
		if (mVSInTangent.get() == NULL)
			return false;

		// Resolve local vertex shader TNB matrix.
		mVSTBNMatrix = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lMatTBN", GCT_MATRIX_3X3);
		if (mVSTBNMatrix.get() == NULL)
			return false;
	}
	
	// Resolve input vertex shader texture coordinates.
	mVSInTexcoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSTexCoordSetIndex, 
		Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex),
		GCT_FLOAT2);
	if (mVSInTexcoord.get() == NULL)
		return false;

	// Resolve output vertex shader texture coordinates.
	mVSOutTexcoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
		Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex),
		GCT_FLOAT2);
	if (mVSOutTexcoord.get() == NULL)
		return false;

	
	// Resolve pixel input texture coordinates normal.
	mPSInTexcoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutTexcoord->getIndex(), 
		mVSOutTexcoord->getContent(),
		mVSOutTexcoord->getType());
	if (mPSInTexcoord.get() == NULL)
		return false;

	// Resolve pixel shader normal.
	if (mNormalMapSpace == NMS_OBJECT)
	{
		mPSNormal = psMain->resolveLocalParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
		if (mPSNormal.get() == NULL)
			return false;
	}
	else if (mNormalMapSpace == NMS_TANGENT)
	{
		mPSNormal = psMain->resolveLocalParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_TANGENT_SPACE, GCT_FLOAT3);
		if (mPSNormal.get() == NULL)
			return false;
	}
	

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

	mPSTempDiffuseColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapDiffuse", GCT_FLOAT4);
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

		mPSTempSpecularColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapSpecular", GCT_FLOAT4);
		if (mPSTempSpecularColour.get() == NULL)
			return false;


		mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
		if (mVSInPosition.get() == NULL)
			return false;

		if (mNormalMapSpace == NMS_TANGENT)
		{
			mVSOutView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
				Parameter::SPC_POSTOCAMERA_TANGENT_SPACE, GCT_FLOAT3);
			if (mVSOutView.get() == NULL)
				return false;	
		}
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			mVSOutView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
				Parameter::SPC_POSTOCAMERA_OBJECT_SPACE, GCT_FLOAT3);
			if (mVSOutView.get() == NULL)
				return false;
		}
		

		mPSInView = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
			mVSOutView->getIndex(), 
			mVSOutView->getContent(),
			mVSOutView->getType());
		if (mPSInView.get() == NULL)
			return false;	

		// Resolve camera position world space.
		mCamPosWorldSpace = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_CAMERA_POSITION, 0);
		if (mCamPosWorldSpace.get() == NULL)		
			return false;	
		
		mVSLocalDir = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapTempDir", GCT_FLOAT3);
		if (mVSLocalDir.get() == NULL)
			return false;	

		mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
		if (mVSWorldPosition.get() == NULL)
			return false;

		// Resolve world matrix.				
		mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		if (mWorldMatrix.get() == NULL)		
			return false;	

		// Resolve inverse world rotation matrix.
		mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "gMatInvRotation");
		if (mWorldInvRotMatrix.get() == NULL)		
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
			if (mLightParamsList[i].mDirection.get() == NULL)
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
			if (mLightParamsList[i].mVSOutDirection.get() == NULL)
				return false;

			mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutDirection->getIndex(), 
				mLightParamsList[i].mVSOutDirection->getContent(), 
				mLightParamsList[i].mVSOutDirection->getType());
			if (mLightParamsList[i].mPSInDirection.get() == NULL)
				return false;

			break;

		case Light::LT_POINT:		
			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition.get() == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_world_space");
			if (mLightParamsList[i].mPosition.get() == NULL)
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
			
			if (mLightParamsList[i].mVSOutToLightDir.get() == NULL)
				return false;

			mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutToLightDir->getIndex(), 
				mLightParamsList[i].mVSOutToLightDir->getContent(), 
				mLightParamsList[i].mVSOutToLightDir->getType());
			if (mLightParamsList[i].mPSInToLightDir.get() == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams.get() == NULL)
				return false;	

			// Resolve local dir.
			if (mVSLocalDir.get() == NULL)
			{
				mVSLocalDir = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapTempDir", GCT_FLOAT3);
				if (mVSLocalDir.get() == NULL)
					return false;	
			}	

			// Resolve world position.
			if (mVSWorldPosition.get() == NULL)
			{
				mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
				if (mVSWorldPosition.get() == NULL)
					return false;	
			}	

			// Resolve world matrix.
			if (mWorldMatrix.get() == NULL)
			{				
				mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
				if (mWorldMatrix.get() == NULL)		
					return false;	
			}

			// Resolve inverse world rotation matrix.
			if (mWorldInvRotMatrix.get() == NULL)
			{				
				mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_GLOBAL, "inv_world_rotation_matrix");
				if (mWorldInvRotMatrix.get() == NULL)		
					return false;	
			}				
			break;

		case Light::LT_SPOTLIGHT:		
			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition.get() == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_world_space");
			if (mLightParamsList[i].mPosition.get() == NULL)
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
			if (mLightParamsList[i].mVSOutToLightDir.get() == NULL)
				return false;

			mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutToLightDir->getIndex(), 
				mLightParamsList[i].mVSOutToLightDir->getContent(), 
				mLightParamsList[i].mVSOutToLightDir->getType());
			if (mLightParamsList[i].mPSInToLightDir.get() == NULL)
				return false;

			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_direction_obj_space");
			if (mLightParamsList[i].mDirection.get() == NULL)
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
			if (mLightParamsList[i].mVSOutDirection.get() == NULL)
				return false;


			mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				mLightParamsList[i].mVSOutDirection->getIndex(), 
				mLightParamsList[i].mVSOutDirection->getContent(), 
				mLightParamsList[i].mVSOutDirection->getType());
			if (mLightParamsList[i].mPSInDirection.get() == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams.get() == NULL)
				return false;	

			mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
			if (mLightParamsList[i].mSpotParams.get() == NULL)
				return false;

			// Resolve local dir.
			if (mVSLocalDir.get() == NULL)
			{
				mVSLocalDir = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapTempDir", GCT_FLOAT3);
				if (mVSLocalDir.get() == NULL)
					return false;	
			}	

			// Resolve world position.
			if (mVSWorldPosition.get() == NULL)
			{
				mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
				if (mVSWorldPosition.get() == NULL)
					return false;	
			}	

			// Resolve world matrix.
			if (mWorldMatrix.get() == NULL)
			{				
				mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
				if (mWorldMatrix.get() == NULL)		
					return false;	
			}

			// Resolve inverse world rotation matrix.
			if (mWorldInvRotMatrix.get() == NULL)
			{				
				mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "gMatInvRotation");
				if (mWorldInvRotMatrix.get() == NULL)		
					return false;	
			}						
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "derived_light_diffuse");
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
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "derived_light_specular");
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
		curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_CONSTRUCT_TBNMATRIX, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSInTangent, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);
	}
	

	// Output texture coordinates.
	curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
	curFuncInvocation->pushOperand(mVSInTexcoord, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mVSOutTexcoord, Operand::OPS_OUT);	
	vsMain->addAtomInstace(curFuncInvocation);

	// Compute world space position.
	if (mVSWorldPosition.get() != NULL)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMPOSITION, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mWorldMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mVSWorldPosition, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);
	}
	

	// Compute view vector.
	if (mVSInPosition.get() != NULL && 
		mVSOutView.get() != NULL)
	{	
		// View vector in world space.
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mCamPosWorldSpace, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(mVSWorldPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to object space.
		curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mWorldInvRotMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to tangent space.
		if (mNormalMapSpace == NMS_TANGENT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutView, Operand::OPS_OUT);	
			vsMain->addAtomInstace(curFuncInvocation);
		}

		// Output object space.
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutView, Operand::OPS_OUT);					
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
	if (curLightParams->mDirection.get() != NULL &&
		curLightParams->mVSOutDirection.get() != NULL)
	{
		// Transform to texture space.
		if (mNormalMapSpace == NMS_TANGENT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mVSOutDirection, Operand::OPS_OUT);	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		// Output object space.
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mVSOutDirection, Operand::OPS_OUT);					
			vsMain->addAtomInstace(curFuncInvocation);
		}
	}
	
	// Transform light vector to target space..
	if (curLightParams->mPosition.get() != NULL &&
		curLightParams->mVSOutToLightDir.get() != NULL)
	{
		// Compute light vector.
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(mVSWorldPosition, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to object space.
		curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mWorldInvRotMatrix, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);	
		curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);

		// Transform to tangent space.		
		if (mNormalMapSpace == NMS_TANGENT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mVSOutToLightDir, Operand::OPS_OUT);	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		
		// Output object space.
		else if (mNormalMapSpace == NMS_OBJECT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mVSOutToLightDir, Operand::OPS_OUT);					
			vsMain->addAtomInstace(curFuncInvocation);
		}
	}


	return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addPSNormalFetchInvocation(Function* psMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;	

	curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_FETCHNORMAL, groupOrder, internalCounter++); 
	curFuncInvocation->pushOperand(mNormalMapSampler, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mPSInTexcoord, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_OUT);	
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
bool NormalMapLighting::addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter)
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
			curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSInView, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
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
			curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
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
			curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mPSInView, Operand::OPS_IN);	
			curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
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
			curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);						
			curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
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
			curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mPSInView, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
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
			curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
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
bool NormalMapLighting::addPSFinalAssignmentInvocation( Function* psMain, const int groupOrder, int& internalCounter )
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
void NormalMapLighting::copyFrom(const SubRenderState& rhs)
{
	const NormalMapLighting& rhsLighting = static_cast<const NormalMapLighting&>(rhs);

	int lightCount[3];

	rhsLighting.getLightCount(lightCount);
	setLightCount(lightCount);

	mTrackVertexColourType = rhsLighting.mTrackVertexColourType;
	mSpecularEnable = rhsLighting.mSpecularEnable;
	mNormalMapSpace = rhsLighting.mNormalMapSpace;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
	if (srcPass->getLightingEnabled() == false)
		return false;

	int lightCount[3];

	renderState->getLightCount(lightCount);

	UserObjectBindings* rtssBindings = any_cast<UserObjectBindings*>(srcPass->getUserObjectBindings().getUserAny(ShaderGenerator::BINDING_OBJECT_KEY));
	const Any& passUserData = rtssBindings->getUserAny(NormalMapLighting::NormalMapTextureNameKey);

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
				"NormalMapLighting::preAddToRenderState");			
		}
	}

	setLightCount(lightCount);

	return true;
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
	if (prop->name == "lighting_stage")
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

			// Case light model type is normal map
			if (strValue == "normal_map")
			{
				++it;
				if (false == SGScriptTranslator::getString(*it, &strValue))
				{
					compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					return NULL;
				}

				unsigned int textureCoordinateIndex = 0;
				SubRenderState* subRenderState = SubRenderStateFactory::createInstance();
				UserObjectBindings* rtssBindings = any_cast<UserObjectBindings*>(pass->getUserObjectBindings().getUserAny(ShaderGenerator::BINDING_OBJECT_KEY));
				
				rtssBindings->setUserAny(NormalMapLighting::NormalMapTextureNameKey, Any(strValue));

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
	UserObjectBindings* rtssBindings = any_cast<UserObjectBindings*>(srcPass->getUserObjectBindings().getUserAny(ShaderGenerator::BINDING_OBJECT_KEY));
	const Any& passUserData = rtssBindings->getUserAny(NormalMapLighting::NormalMapTextureNameKey);
	if (passUserData.isEmpty())	
		return;	

	const String normalMapTextureName = any_cast<const String>(passUserData);

	ser->writeAttribute(4, "lighting_stage");
	ser->writeValue("normal_map");
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
	return OGRE_NEW NormalMapLighting;
}

}
}

#endif
