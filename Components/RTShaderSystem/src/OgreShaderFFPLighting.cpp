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
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
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
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPLighting::Type = "FFP_Lighting";
Light FFPLighting::msBlankLight;

//-----------------------------------------------------------------------
FFPLighting::FFPLighting()
{
	mTrackVertexColourType			= TVC_NONE;
	mSpecularEnable					= false;

	msBlankLight.setDiffuseColour(ColourValue::Black);
	msBlankLight.setSpecularColour(ColourValue::Black);
	msBlankLight.setAttenuation(0,1,0,0);
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
	if (mLightParamsList.size() == 0)
		return;

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
			curParams.mDirection->setGpuParameter(vParameter);
			break;

		case Light::LT_POINT:

			// Update light position.
			vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
			curParams.mPosition->setGpuParameter(vParameter);

			// Update light attenuation parameters.
			vParameter.x = srcLight->getAttenuationRange();
			vParameter.y = srcLight->getAttenuationConstant();
			vParameter.z = srcLight->getAttenuationLinear();
			vParameter.w = srcLight->getAttenuationQuadric();
			curParams.mAttenuatParams->setGpuParameter(vParameter);
			break;

		case Light::LT_SPOTLIGHT:
		{						
			Vector3 vec3;
			Matrix3 matViewIT;

			source->getInverseTransposeViewMatrix().extract3x3Matrix(matViewIT);

			
			// Update light position.
			vParameter = matView.transformAffine(srcLight->getAs4DVector(true));
			curParams.mPosition->setGpuParameter(vParameter);
			
							
			vec3 = matViewIT * srcLight->getDerivedDirection();
			vec3.normalise();

			vParameter.x = -vec3.x;
			vParameter.y = -vec3.y;
			vParameter.z = -vec3.z;
			vParameter.w = 0.0;
			curParams.mDirection->setGpuParameter(vParameter);

			// Update light attenuation parameters.
			vParameter.x = srcLight->getAttenuationRange();
			vParameter.y = srcLight->getAttenuationConstant();
			vParameter.z = srcLight->getAttenuationLinear();
			vParameter.w = srcLight->getAttenuationQuadric();
			curParams.mAttenuatParams->setGpuParameter(vParameter);

			// Update spotlight parameters.
			Real phi   = Math::Cos(srcLight->getSpotlightOuterAngle().valueRadians() * 0.5f);
			Real theta = Math::Cos(srcLight->getSpotlightInnerAngle().valueRadians() * 0.5f);

			vec3.x = theta;
			vec3.y = phi;
			vec3.z = srcLight->getSpotlightFalloff();
			
			curParams.mSpotParams->setGpuParameter(vec3);
		}
			break;
		}

		
		// Update diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			colour = srcLight->getDiffuseColour() * pass->getDiffuse();
			curParams.mDiffuseColour->setGpuParameter(colour);					
		}
		else
		{					
			colour = srcLight->getDiffuseColour();
			curParams.mDiffuseColour->setGpuParameter(colour);	
		}

		// Update specular colour if need to.
		if (mSpecularEnable)
		{
			// Update diffuse colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				colour = srcLight->getSpecularColour() * pass->getSpecular();
				curParams.mSpecularColour->setGpuParameter(colour);					
			}
			else
			{					
				colour = srcLight->getSpecularColour();
				curParams.mSpecularColour->setGpuParameter(colour);	
			}
		}																			
	}
}

//-----------------------------------------------------------------------
bool FFPLighting::resolveParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();

	// Resolve world view IT matrix.
	mWorldViewITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX, 0);
	if (mWorldViewITMatrix.get() == NULL)
		return false;	

	// Get surface ambient colour if need to.
	if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
	{		
		mDerivedAmbientLightColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR, 0);
		if (mDerivedAmbientLightColour.get() == NULL)
			return false;
	}
	else
	{
		mLightAmbientColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR, 0);
		if (mLightAmbientColour.get() == NULL)
			return false;	
		
		mSurfaceAmbientColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR, 0);
		if (mSurfaceAmbientColour.get() == NULL)
			return false;	

	}

	// Get surface diffuse colour if need to.
	if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
	{
		mSurfaceDiffuseColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR, 0);
		if (mSurfaceDiffuseColour.get() == NULL)
			return false;	 
	}

	// Get surface specular colour if need to.
	if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
	{
		mSurfaceSpecularColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR, 0);
		if (mSurfaceSpecularColour.get() == NULL)
			return false;	 
	}
		 
	
	// Get surface emissive colour if need to.
	if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
	{
		mSurfaceEmissiveColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR, 0);
		if (mSurfaceEmissiveColour.get() == NULL)
			return false;	 
	}

	// Get derived scene colour.
	mDerivedSceneColour = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR, 0);
	if (mDerivedSceneColour.get() == NULL)
		return false;

	// Get surface shininess.
	mSurfaceShininess = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SHININESS, 0);
	if (mSurfaceShininess.get() == NULL)
		return false;

	// Resolve input vertex shader normal.
	mVSInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
	if (mVSInNormal.get() == NULL)
		return false;

	if (mTrackVertexColourType != 0)
	{
		mVSDiffuse = vsMain->resolveInputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
		if (mVSDiffuse.get() == NULL)
			return false;
	}
	

	// Resolve output vertex shader diffuse colour.
	mVSOutDiffuse = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mVSOutDiffuse.get() == NULL)
		return false;


	

	// Resolve per light parameters.
	for (unsigned int i=0; i < mLightParamsList.size(); ++i)
	{		
		switch (mLightParamsList[i].mType)
		{
		case Light::LT_DIRECTIONAL:
			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mDirection.get() == NULL)
				return false;
			break;
		
		case Light::LT_POINT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix.get() == NULL)
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition.get() == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mPosition.get() == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams.get() == NULL)
				return false;			
			break;
		
		case Light::LT_SPOTLIGHT:
			mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
			if (mWorldViewMatrix.get() == NULL)
				return false;	

			mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInPosition.get() == NULL)
				return false;

			mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mPosition.get() == NULL)
				return false;

			mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_position_view_space");
			if (mLightParamsList[i].mDirection.get() == NULL)
				return false;

			mLightParamsList[i].mAttenuatParams = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");
			if (mLightParamsList[i].mAttenuatParams.get() == NULL)
				return false;	

			mLightParamsList[i].mSpotParams = vsProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
			if (mLightParamsList[i].mSpotParams.get() == NULL)
				return false;		
			break;
		}

		// Resolve diffuse colour.
		if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL | (uint16)GPV_LIGHTS, "derived_light_diffuse");
			if (mLightParamsList[i].mDiffuseColour.get() == NULL)
				return false;
		}
		else
		{
			mLightParamsList[i].mDiffuseColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_diffuse");
			if (mLightParamsList[i].mDiffuseColour.get() == NULL)
				return false;
		}		

		if (mSpecularEnable)
		{
			// Resolve specular colour.
			if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL | (uint16)GPV_LIGHTS, "derived_light_specular");
				if (mLightParamsList[i].mSpecularColour.get() == NULL)
					return false;
			}
			else
			{
				mLightParamsList[i].mSpecularColour = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_specular");
				if (mLightParamsList[i].mSpecularColour.get() == NULL)
					return false;
			}

			if (mVSOutSpecular.get() == NULL)
			{
				mVSOutSpecular = vsMain->resolveOutputParameter(Parameter::SPS_COLOR, 1, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
				if (mVSOutSpecular.get() == NULL)
					return false;
			}
			
			if (mVSInPosition.get() == NULL)
			{
				mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
				if (mVSInPosition.get() == NULL)
					return false;
			}

			if (mWorldViewMatrix.get() == NULL)
			{
				mWorldViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLDVIEW_MATRIX, 0);
				if (mWorldViewMatrix.get() == NULL)
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
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mDerivedSceneColour, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT);	
		vsMain->addAtomInstace(curFuncInvocation);		
	}
	else
	{
		if (mTrackVertexColourType & TVC_AMBIENT)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mLightAmbientColour, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSDiffuse, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT);	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mDerivedAmbientLightColour, Operand::OPS_IN, (Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z));	
			vsMain->addAtomInstace(curFuncInvocation);
		}

		if (mTrackVertexColourType & TVC_EMISSIVE)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mVSDiffuse, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT);	
			vsMain->addAtomInstace(curFuncInvocation);
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mSurfaceEmissiveColour, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT);	
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
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mVSDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
		curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		vsMain->addAtomInstace(curFuncInvocation);
	}

	// Merge specular colour with vertex colour if need to.
	if (mSpecularEnable && mTrackVertexColourType & TVC_SPECULAR)
	{							
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++); 
		curFuncInvocation->pushOperand(mVSDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
		curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
		vsMain->addAtomInstace(curFuncInvocation);
	}

	switch (curLightParams->mType)
	{
		
	case Light::LT_DIRECTIONAL:			
		if (mSpecularEnable)
		{				
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutSpecular, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutSpecular, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			vsMain->addAtomInstace(curFuncInvocation);
		}

		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder, internalCounter++); 
			curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));					
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			vsMain->addAtomInstace(curFuncInvocation);	
		}				
		break;

	case Light::LT_POINT:	
		if (mSpecularEnable)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LIGHT_POINT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutSpecular, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutSpecular, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			vsMain->addAtomInstace(curFuncInvocation);			
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LIGHT_POINT_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));					
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			vsMain->addAtomInstace(curFuncInvocation);	
		}
				
		break;

	case Light::LT_SPOTLIGHT:
		if (mSpecularEnable)
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LIGHT_SPOT_DIFFUSESPECULAR, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));			
			curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutSpecular, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutSpecular, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			vsMain->addAtomInstace(curFuncInvocation);			
		}
		else
		{
			curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LIGHT_SPOT_DIFFUSE, groupOrder, internalCounter++); 			
			curFuncInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);			
			curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mWorldViewITMatrix, Operand::OPS_IN);
			curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));
			curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
			curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));					
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_IN, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
			curFuncInvocation->pushOperand(mVSOutDiffuse, Operand::OPS_OUT, (Operand::OPM_X |  Operand::OPM_Y | Operand::OPM_Z));	
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

	int lightCount[3];

	rhsLighting.getLightCount(lightCount);
	setLightCount(lightCount);
}

//-----------------------------------------------------------------------
bool FFPLighting::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
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

			if (modelType == "ffp")
			{
				return SubRenderStateFactory::createInstance();
			}
		}		
	}

	return NULL;
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
