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
#include "OgreShaderFFPLighting.h"
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
String FFPLighting::Type = "FFP_Lighting";

Light FFPLighting::msBlankLight;

//-----------------------------------------------------------------------
FFPLighting::FFPLighting()
{
	mTrackVertexColourType			= TVC_NONE;
	mWorldViewMatrix				= NULL;
	mWorldViewITMatrix				= NULL;
	mVSInPosition					= NULL;
	mVSInNormal						= NULL;
	mVSDiffuse						= NULL;
	mVSOutDiffuse					= NULL;
	mVSOutSpecular  				= NULL;	
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
FFPLighting::~FFPLighting()
{

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
uint32 FFPLighting::getHashCode()
{
	uint32 hashCode = 0;
	
	boost::hash_combine(hashCode, __super::getHashCode());

	LightParamsIterator it = mLightParamsList.begin();


	boost::hash_combine(hashCode, mSpeuclarEnable);	

	while(it != mLightParamsList.end())
	{
		boost::hash_combine(hashCode, it->mType);		
		++it;
	}
	
	boost::hash_combine(hashCode, mTrackVertexColourType);
	
	
	return hashCode;
}

//-----------------------------------------------------------------------
void FFPLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
										  const LightList* pLightList)
{		
	GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
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
			vsGpuParams->setNamedConstant(curParams.mDirection->getName(), vParameter);
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
			Matrix3 matViewIT;

			source->getInverseTransposeViewMatrix().extract3x3Matrix(matViewIT);

			
			// Update light position.
			vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
			vsGpuParams->setNamedConstant(curParams.mPosition->getName(), vParameter);
			
							
			vec3 = matViewIT * srcLight->getDirection();
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
			vsGpuParams->setNamedConstant(curParams.mDiffuseColour->getName(), colour);					
		}
		else
		{					
			colour = srcLight->getDiffuseColour();
			vsGpuParams->setNamedConstant(curParams.mDiffuseColour->getName(), colour);	
		}

		// Update specular colour if need to.
		if (mSpeuclarEnable)
		{
			// Update diffuse colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				colour = srcLight->getSpecularColour() * pass->getSpecular();
				vsGpuParams->setNamedConstant(curParams.mSpecularColour->getName(), colour);					
			}
			else
			{					
				colour = srcLight->getSpecularColour();
				vsGpuParams->setNamedConstant(curParams.mSpecularColour->getName(), colour);	
			}
		}																			
	}
}

//-----------------------------------------------------------------------
bool FFPLighting::resolveParameters(ProgramSet* programSet)
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
		mSurfaceDiffuseColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR, 0);
		if (mSurfaceDiffuseColour == NULL)		
			return false;	 
	}

	// Get surface specular colour if need to.
	if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
	{
		mSurfaceSpecularColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR, 0);
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
	mSurfaceShininess = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SHININESS, 0);
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


	

	// Resolve per light parameters.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		switch (mLightParamsList[i].mType)
		{
		case Light::LT_DIRECTIONAL:
			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_position_view_space");
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
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, "derived_light_diffuse");
			if (mLightParamsList[i].mDiffuseColour == NULL)
				return false;
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_diffuse");
			if (mLightParamsList[i].mDiffuseColour == NULL)
				return false;
		}		

		if (mSpeuclarEnable)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, "derived_light_specular");
				if (mLightParamsList[i].mSpecularColour == NULL)
					return false;
			}
			else
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, "light_specular");
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
bool FFPLighting::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_LIGHTING);

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();	
	Function* vsMain = vsProgram->getEntryPointFunction();	
	
	int internalCounter = 0;
	
	// Add the global illumination functions.
	if (false == addGlobalIlluminationInvocation(vsMain, FFP_VS_LIGHTING, internalCounter))
		return false;


	// Add per light functions.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		if (false == addIlluminationInvocation(&mLightParamsList[i], vsMain, FFP_VS_LIGHTING, internalCounter))
			return false;
	}

	

	return true;
}

//-----------------------------------------------------------------------
bool FFPLighting::addGlobalIlluminationInvocation(Function* vsMain, const int groupOrder, int& internalCounter)
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
bool FFPLighting::addIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder, int& internalCounter)
{	
	FunctionInvocation* curFuncInvocation = NULL;	

	// Merge diffuse colour with vertex colour if need to.
	if (mTrackVertexColourType & TVC_DIFFUSE)			
	{
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mVSDiffuse->getName() + ".xyz");	
		curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(curLightParams->mDiffuseColour->getName() + ".xyz");
		vsMain->addAtomInstace(curFuncInvocation);
	}

	// Merge specular colour with vertex colour if need to.
	if (mSpeuclarEnable && mTrackVertexColourType & TVC_SPECULAR)
	{							
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->getParameterList().push_back(mVSDiffuse->getName() + ".xyz");	
		curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");
		curFuncInvocation->getParameterList().push_back(curLightParams->mSpecularColour->getName() + ".xyz");
		vsMain->addAtomInstace(curFuncInvocation);
	}

	switch (curLightParams->mType)
	{
		
	case Light::LT_DIRECTIONAL:			
		if (mSpeuclarEnable)
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
			vsMain->addAtomInstace(curFuncInvocation);
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
			vsMain->addAtomInstace(curFuncInvocation);	
		}				
		break;

	case Light::LT_POINT:	
		if (mSpeuclarEnable)
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
			vsMain->addAtomInstace(curFuncInvocation);			
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
			vsMain->addAtomInstace(curFuncInvocation);	
		}
				
		break;

	case Light::LT_SPOTLIGHT:
		if (mSpeuclarEnable)
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
			vsMain->addAtomInstace(curFuncInvocation);			
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
			vsMain->addAtomInstace(curFuncInvocation);	
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------
void FFPLighting::copyFrom(const SubRenderState& rhs)
{
	const FFPLighting& rhsLighting = static_cast<const FFPLighting&>(rhs);

	int maxLightCount[3];

	rhsLighting.getMaxLightCount(maxLightCount);
	setMaxLightCount(maxLightCount);
}

//-----------------------------------------------------------------------
void FFPLighting::setMaxLightCount(const int maxLightCount[3])
{

	for (int type=0; type < 3; ++type)
	{
		for (int i=0; i < maxLightCount[type]; ++i)
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
void FFPLighting::getMaxLightCount(int maxLightCount[3]) const
{
	maxLightCount[0] = 0;
	maxLightCount[1] = 0;
	maxLightCount[2] = 0;

	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{
		const LightParams curParams = mLightParamsList[i];

		if (curParams.mType == Light::LT_POINT)
			maxLightCount[0]++;
		else if (curParams.mType == Light::LT_DIRECTIONAL)
			maxLightCount[1]++;
		else if (curParams.mType == Light::LT_SPOTLIGHT)
			maxLightCount[2]++;
	}
}

//-----------------------------------------------------------------------
const String& FFPLightingFactory::getType() const
{
	return FFPLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	FFPLightingFactory::createInstanceImpl()
{
	return new FFPLighting;
}

}
}

