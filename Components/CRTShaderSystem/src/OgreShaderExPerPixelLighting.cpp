/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include <boost/functional/hash/hash.hpp>
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
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String PerPixelLighting::Type = "SGX_PerPixelLighting";

//-----------------------------------------------------------------------
PerPixelLighting::PerPixelLighting()
{
	mTrackVertexColourType			= TVC_NONE;
	mWorldViewMatrix				= NULL;
	mWorldViewITMatrix				= NULL;
	mVSInPosition					= NULL;
	mVSInNormal						= NULL;
	mVSDiffuse						= NULL;
	mVSOutDiffuse					= NULL;
	mVSOutNormal					= NULL;
	mVSOutSpecular  				= NULL;	
	mPSInDiffuse					= NULL;
	mPSInSpecular					= NULL;
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
	mCurCamera						= NULL;
}

//-----------------------------------------------------------------------
PerPixelLighting::~PerPixelLighting()
{

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
	
	boost::hash_combine(hashCode, __super::getHashCode());

	LightParamsIterator it = mLightParamsList.begin();

	while(it != mLightParamsList.end())
	{
		boost::hash_combine(hashCode, it->mProperties.mType);
		boost::hash_combine(hashCode, it->mProperties.mEnableSpecular);
		++it;
	}
	
	boost::hash_combine(hashCode, mTrackVertexColourType);
	
	
	return hashCode;
}

//-----------------------------------------------------------------------
void PerPixelLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass)
{		
	GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
	GpuProgramParametersSharedPtr psGpuParams = pass->getFragmentProgramParameters();
	SceneManager* sceneMgr = ShaderGenerator::getSingleton().getSceneManager();

	Viewport* curViewport = sceneMgr->getCurrentViewport();
	Camera* curCamera     = curViewport->getCamera();
	const Matrix4& matView = curCamera->getViewMatrix(true);
	

	// Update per light parameters.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{
		const LightParams& curParams = mLightParamsList[i];

		// Case this is specified light in the scene, we have to update
		// it's properties manually since the scene manager sorts lights for each renderable
		// on per frame basis and this may cause to uniform parameters to be set wrongly and give undefined results.
		// I.E - this sub state may build a shader code that treat light 0 as point light and light 1 as diretional light.
		// The scene manager may cull out the point light since it is out of range and will create a list of one light only and apply
		// the directional light parameters to the point light parameters slots that the shader expects.
		if (curParams.mLight != NULL)
		{
			Light*		srcLight = curParams.mLight;
			Vector4		vParameter;
			ColourValue colour;
						
			
			switch (curParams.mProperties.mType)
			{
			case Light::LT_DIRECTIONAL:

				// Update light direction.
				vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
				psGpuParams->setNamedConstant(curParams.mDirection->getName(), vParameter);
				break;

			case Light::LT_POINT:

				// Update light position.
				vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
				vsGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);

				// Update light attenuation parameters.
				vParameter.x = srcLight->getAttenuationRange();
				vParameter.y = srcLight->getAttenuationConstant();
				vParameter.z = srcLight->getAttenuationLinear();
				vParameter.w = srcLight->getAttenuationQuadric();
				vsGpuParams->setNamedConstant(curParams.mAttenuatParams->getName(), vParameter);
				break;

			case Light::LT_SPOTLIGHT:
			{
								
				Vector3 vec3;

				
				// Update light position.
				vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
				vsGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);

				// Update light direction.
				if (mCurCamera != curCamera)
				{
					mCurCamera = curCamera;

					Matrix4 matViewIT;	

					matViewIT = matView.inverseAffine();
					matViewIT = matViewIT.transpose();
					matViewIT.extract3x3Matrix(mMatViewIT3);	
				}					
								
				vec3 = mMatViewIT3 * srcLight->getDirection();
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
				vsGpuParams->setNamedConstant(curParams.mAttenuatParams->getName(), vParameter);

				// Update spotlight parameters.
				Real phi   = Math::Cos(srcLight->getSpotlightOuterAngle().valueRadians() * 0.5);
				Real theta = Math::Cos(srcLight->getSpotlightInnerAngle().valueRadians() * 0.5);

				vec3.x = theta;
				vec3.y = phi;
				vec3.z = srcLight->getSpotlightFalloff();
				
				vsGpuParams->setNamedConstant(curParams.mSpotParams->getName(), vec3);
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
			if (curParams.mProperties.mEnableSpecular)
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
}

//-----------------------------------------------------------------------
void PerPixelLighting::preFindVisibleObjects(SceneManager* source, 
										SceneManager::IlluminationRenderStage irs, 
										Viewport* v)
{
	mCurCamera = NULL;
}


//-----------------------------------------------------------------------
bool PerPixelLighting::resolveParameters(ProgramSet* programSet)
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
		mDerivedAmbientLightColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR, 0);
		if (mDerivedAmbientLightColour == NULL)		
			return false;
	}
	else
	{
		mLightAmbientColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR, 0);
		if (mLightAmbientColour == NULL)		
			return false;	
		
		mSurfaceAmbientColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR, 0);
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
		mSurfaceEmissiveColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR, 0);
		if (mSurfaceEmissiveColour == NULL)		
			return false;	 
	}

	// Get derived scene colour.
	mDerivedSceneColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR, 0);
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

	// Resolve output vertex shader diffuse colour.
	mVSOutDiffuse = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	if (mVSOutDiffuse == NULL)
		return false;

	// Resolve output vertex shader normal in view space colour.
	mVSOutNormal = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, GCT_FLOAT3);
	if (mVSOutNormal == NULL)
		return false;

	// Resolve input pixel shader diffuse colour.
	mPSInDiffuse = psMain->resolveInputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	if (mPSInDiffuse == NULL)
		return false;

	// Resolve output pixel shader diffuse colour.
	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	if (mPSOutDiffuse == NULL)
		return false;

	// Resolve per light parameters.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{
		const LightProperties& curLightProp = mLightParamsList[i].mProperties;

		switch (curLightProp.mType)
		{
		case Light::LT_DIRECTIONAL:
			mLightParamsList[i].mDirection = psProgram->resolveParameter(GCT_FLOAT4, -1, "light_position_view_space");
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

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_position_view_space");
			if (mLightParamsList[i].mPosition == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams == NULL)
				return false;			
			break;
		
		case Light::LT_SPOTLIGHT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix == NULL)		
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
			if (mVSInPosition == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_position_view_space");
			if (mLightParamsList[i].mPosition == NULL)
				return false;

			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_position_view_space");
			if (mLightParamsList[i].mDirection == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams == NULL)
				return false;	

			mLightParamsList[i].mSpotParams = vsProgram->resolveParameter(GCT_FLOAT3, -1, "spotlight_params");
			if (mLightParamsList[i].mSpotParams == NULL)
				return false;		
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, "derived_light_diffuse");
			if (mLightParamsList[i].mDiffuseColour == NULL)
				return false;
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, "light_diffuse");
			if (mLightParamsList[i].mDiffuseColour == NULL)
				return false;
		}		

		if (curLightProp.mEnableSpecular)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, "derived_light_specular");
				if (mLightParamsList[i].mSpecularColour == NULL)
					return false;
			}
			else
			{
				mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, "light_specular");
				if (mLightParamsList[i].mSpecularColour == NULL)
					return false;
			}

			if (mVSOutSpecular == NULL)
			{
				mVSOutSpecular = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, GCT_FLOAT4);
				if (mVSOutSpecular == NULL)
					return false;
			}
			
			if (mVSInPosition == NULL)
			{
				mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
				if (mVSInPosition == NULL)
					return false;
			}

			if (mWorldViewMatrix == NULL)
			{
				mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
				if (mWorldViewMatrix == NULL)		
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

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_LIGHTING);

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();	
	Function* vsMain = vsProgram->getEntryPointFunction();	
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* psMain = psProgram->getEntryPointFunction();	
	
	int funcInvocationCounter = 0;
	
	// Add the global illumination functions.
	if (false == addGlobalIlluminationInvocation(vsMain, FFP_VS_LIGHTING, funcInvocationCounter))
		return false;

	funcInvocationCounter = 0;

	// Add per light functions.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		if (false == addIlluminationInvocation(&mLightParamsList[i], psMain, FFP_PS_COLOUR_BEGIN + 1, funcInvocationCounter))
			return false;
	}

	

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addGlobalIlluminationInvocation(Function* vsMain, const int groupOrder, int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;	

	if ((mTrackVertexColourType & TVC_AMBIENT) == 0 && 
		(mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mDerivedSceneColour->getName());
		curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName());	
		vsMain->addAtomInstace(curFuncInvocation);		
	}
	else
	{
		if (mTrackVertexColourType & TVC_AMBIENT)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mLightAmbientColour->getName());
			curFuncInvocation->getParameterList().push_back(mSurfaceAmbientColour->getName());			
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName());	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mDerivedAmbientLightColour->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			vsMain->addAtomInstace(curFuncInvocation);
		}

		if (mTrackVertexColourType & TVC_EMISSIVE)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mVSDiffuse->getName() );
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName());	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mSurfaceEmissiveColour->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName());	
			vsMain->addAtomInstace(curFuncInvocation);
		}		
	}

	return true;
}

//-----------------------------------------------------------------------
bool PerPixelLighting::addIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter)
{
	const LightProperties& curLightProp = curLightParams->mProperties;
	FunctionInvocation* curFuncInvocation = NULL;	

	// Merge diffuse colour with vertex colour if need to.
	if (mTrackVertexColourType & TVC_DIFFUSE)			
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mVSDiffuse->getName() + ".xyz");	
		curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
		psMain->addAtomInstace(curFuncInvocation);
	}

	// Merge specular colour with vertex colour if need to.
	if (curLightProp.mEnableSpecular && 
		mTrackVertexColourType & TVC_SPECULAR)
	{							
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mVSDiffuse->getName() + ".xyz");	
		curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");
		psMain->addAtomInstace(curFuncInvocation);
	}

	switch (curLightProp.mType)
	{
		
	case Light::LT_DIRECTIONAL:			
		if (curLightProp.mEnableSpecular)
		{				
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mWorldViewMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());			
			curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(mSurfaceShininess->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutSpecular->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutSpecular->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);
		}

		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder, internalCounter++); 
			curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");					
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);	
		}				
		break;

	case Light::LT_POINT:	
		if (curLightProp.mEnableSpecular)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_LIGHT_POINT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mWorldViewMatrix->getName());			
			curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());
			curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(mSurfaceShininess->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutSpecular->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutSpecular->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);			
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_LIGHT_POINT_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mWorldViewMatrix->getName());			
			curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());
			curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");					
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);	
		}
				
		break;

	case Light::LT_SPOTLIGHT:
		if (curLightProp.mEnableSpecular)
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_LIGHT_SPOT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mWorldViewMatrix->getName());			
			curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());
			curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpotParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");			
			curFuncInvocation->getParameterList().push_back(mSurfaceShininess->getName());
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutSpecular->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutSpecular->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);			
		}
		else
		{
			curFuncInvocation = new FunctionInvocation(FFP_FUNC_LIGHT_SPOT_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->getParameterList().push_back(mWorldViewMatrix->getName());			
			curFuncInvocation->getParameterList().push_back(mVSInPosition->getName());
			curFuncInvocation->getParameterList().push_back(mWorldViewITMatrix->getName());
			curFuncInvocation->getParameterList().push_back(mVSInNormal->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mPosition->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mDirection->getName() + ".xyz");
			curFuncInvocation->getParameterList().push_back(curLightParams->mAttenuatParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mSpotParams->getName());
			curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");					
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			curFuncInvocation->getParameterList().push_back(mVSOutDiffuse->getName() + ".xyz");	
			psMain->addAtomInstace(curFuncInvocation);	
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------
void PerPixelLighting::copyFrom(const SubRenderState& rhs)
{
	const PerPixelLighting& rhsLighting = static_cast<const PerPixelLighting&>(rhs);

	setLightCount(rhsLighting.getLightCount());

	for (unsigned int i=0; i < rhsLighting.getLightCount(); ++i)
	{
		setLightProperties(i, rhsLighting.getLightProperties(i));
	}
}

//-----------------------------------------------------------------------
void PerPixelLighting::setLightCount(size_t count)
{
	mLightParamsList.resize(count);
	for (unsigned int i=0; i < count; ++i)
	{
		LightParams* curParams = &mLightParamsList[i];

		curParams->mPosition		= NULL;
		curParams->mDirection		= NULL;
		curParams->mAttenuatParams	= NULL;
		curParams->mSpotParams		= NULL;
		curParams->mDiffuseColour	= NULL;
		curParams->mSpecularColour  = NULL;		
	}
}

//-----------------------------------------------------------------------
size_t PerPixelLighting::getLightCount() const
{
	return mLightParamsList.size();
}

//-----------------------------------------------------------------------
void PerPixelLighting::setLightProperties(unsigned int index, const LightProperties& properties)
{
	mLightParamsList[index].mProperties = properties;
	if (properties.mName.empty() == false)
	{
		mLightParamsList[index].mLight = ShaderGenerator::getSingleton().getSceneManager()->getLight(properties.mName);
	}
}

//-----------------------------------------------------------------------
const PerPixelLighting::LightProperties&	PerPixelLighting::getLightProperties(unsigned int index) const
{
	return mLightParamsList[index].mProperties;
}


//-----------------------------------------------------------------------
const String& PerPixelLightingFactory::getType() const
{
	return PerPixelLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	PerPixelLightingFactory::createInstanceImpl()
{
	return new PerPixelLighting;
}

}
}

