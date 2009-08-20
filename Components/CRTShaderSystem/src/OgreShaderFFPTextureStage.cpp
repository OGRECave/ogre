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
#include "OgreShaderFFPTextureStage.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreTextureUnitState.h"
#include "OgreFrustum.h"
#include "OgrePass.h"

namespace Ogre {
namespace CRTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPTextureStage::Type = "FFP_TextureUnitState";
#define _INT_VALUE(f) (*(int*)(&(f)))

//-----------------------------------------------------------------------
FFPTextureStage::FFPTextureStage()
{
	mTextureUnitState			= NULL;
	mTextureSamplerIndex		= 0;
	mTextureSampler				= NULL;
	mTextureMatrix				= NULL;
	mWorldMatrix				= NULL;
	mWorldITMatrix				= NULL;
	mViewMatrix					= NULL;	
	mViewTMatrix				= NULL;
	mVSInputNormal				= NULL;
	mVSInputTexCoord			= NULL;
	mVSOutputTexCoord			= NULL;
	mPSInputTexCoord			= NULL;	
	mPSOutColor					= NULL;
	mPSDiffuseColor				= NULL;
	mPSSpecularColor			= NULL;
	mVSInputPos					= NULL;
	mTextureViewProjImageMatrix	= NULL;
	mTextureProjector			= NULL;	
}

//-----------------------------------------------------------------------
FFPTextureStage::~FFPTextureStage()
{

}

//-----------------------------------------------------------------------
const String& FFPTextureStage::getType() const
{
	return Type;
}

//-----------------------------------------------------------------------
int	FFPTextureStage::getExecutionOrder() const
{		
	return FFP_TEXTURE_STAGE0 + mTextureSamplerIndex*100;
}

//-----------------------------------------------------------------------
bool FFPTextureStage::resolveParameters(ProgramSet* programSet)
{
	if (false == resolveUniformParams(programSet))
		return false;


	if (false == resolveFunctionsParams(programSet))
		return false;

	return true;
}

//-----------------------------------------------------------------------
bool FFPTextureStage::resolveUniformParams(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	
	
	// Resolve texture sampler parameter.		
	mTextureSampler = psProgram->resolveParameter(mTextureSamplerType, mTextureSamplerIndex, "gTextureSampler");
	if (mTextureSampler == NULL)
		return false;
	
		
	// Resolve texture matrix parameter.
	if (needsTextureMatrix())
	{				
		mTextureMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_TEXTURE_MATRIX, mTextureSamplerIndex);
		if (mTextureMatrix == NULL)
			return false;
	}

	// Resolve auto texture coordinates generation parameters.
	TexCoordCalcMethod texCoordCalcMethod = getTexCalcMethod();


	switch (texCoordCalcMethod)
	{
	case TEXCALC_NONE:								
		break;

	// Resolve World + View matrices.
	case TEXCALC_ENVIRONMENT_MAP:
	case TEXCALC_ENVIRONMENT_MAP_PLANAR:	
	case TEXCALC_ENVIRONMENT_MAP_NORMAL:
		
		mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 0);
		if (mWorldITMatrix == NULL)		
			return false;	
		
		mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
		if (mViewMatrix == NULL)		
			return false;				
		
		break;

	case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
		mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		if (mWorldMatrix == NULL)		
			return false;	

		mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 0);
		if (mWorldITMatrix == NULL)		
			return false;	

		mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
		if (mViewMatrix == NULL)		
			return false;	

		mViewTMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEW_MATRIX, 0);
		if (mViewTMatrix == NULL)		
			return false;	
	
		break;


	case TEXCALC_PROJECTIVE_TEXTURE:

		mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		if (mWorldMatrix == NULL)		
			return false;	

		mTextureViewProjImageMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, "gTexViewProjImageMatrix");
		if (mTextureViewProjImageMatrix == NULL)		
			return false;	

		const TextureUnitState::EffectMap&		effectMap = mTextureUnitState->getEffects();	
		TextureUnitState::EffectMap::const_iterator	effi;

		for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
		{
			if (effi->second.type == TextureUnitState::ET_PROJECTIVE_TEXTURE)
			{
				mTextureProjector = effi->second.frustum;
				break;
			}
		}

		

		if (mTextureProjector == NULL)		
			return false;	

		break;
	}

	return true;
}



//-----------------------------------------------------------------------
bool FFPTextureStage::resolveFunctionsParams(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain   = vsProgram->getEntryPointFunction();
	Function* psMain   = psProgram->getEntryPointFunction();	
	TexCoordCalcMethod texCoordCalcMethod = getTexCalcMethod();


	switch (texCoordCalcMethod)
	{
		case TEXCALC_NONE:					
			// Resolve explicit vs input texture coordinates.
			mVSInputTexCoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mTextureUnitState->getTextureCoordSet(), mVSInTextureCoordinateType);	
			if (mVSInputTexCoord == NULL)			
				return false;		
			break;

		case TEXCALC_ENVIRONMENT_MAP:
		case TEXCALC_ENVIRONMENT_MAP_PLANAR:		
		case TEXCALC_ENVIRONMENT_MAP_NORMAL:
			// Resolve vertex normal.
			mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, GCT_FLOAT3);
			if (mVSInputNormal == NULL)			
				return false;									
			break;	

		case TEXCALC_ENVIRONMENT_MAP_REFLECTION:

			// Resolve vertex normal.
			mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, GCT_FLOAT3);
			if (mVSInputNormal == NULL)			
				return false;		

			// Resolve vertex position.
			mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
			if (mVSInputPos == NULL)			
				return false;		
			break;

		case TEXCALC_PROJECTIVE_TEXTURE:
			// Resolve vertex position.
			mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, GCT_FLOAT4);
			if (mVSInputPos == NULL)			
				return false;		
			break;
	}

	// Resolve vs output texture coordinates.
	mVSOutputTexCoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		-1,
		mVSOutTextureCoordinateType);

	if (mVSOutputTexCoord == NULL)
		return false;
		

	// Resolve ps input texture coordinates.
	mPSInputTexCoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutputTexCoord->getIndex(),
		mVSOutTextureCoordinateType);

	if (mPSInputTexCoord == NULL)
		return false;

	const ShaderParameterList& inputParams = psMain->getInputParameters();
	const ShaderParameterList& localParams = psMain->getLocalParameters();

	mPSDiffuseColor = psMain->getParameterBySemantic(inputParams, Parameter::SPS_COLOR, 0);
	if (mPSDiffuseColor == NULL)
	{
		mPSDiffuseColor = psMain->getParameterBySemantic(localParams, Parameter::SPS_COLOR, 0);
		if (mPSDiffuseColor == NULL)
			return false;
	}

	mPSSpecularColor = psMain->getParameterBySemantic(inputParams, Parameter::SPS_COLOR, 1);
	if (mPSSpecularColor == NULL)
	{
		mPSSpecularColor = psMain->getParameterBySemantic(localParams, Parameter::SPS_COLOR, 1);
		if (mPSSpecularColor == NULL)
			return false;
	}

	mPSOutColor = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, GCT_FLOAT4);
	if (mPSOutColor == NULL)	
		return false;

	
	return true;
}

//-----------------------------------------------------------------------
bool FFPTextureStage::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_TEXTURESTAGE);	
	psProgram->addDependency(FFP_LIB_COMMON);
	psProgram->addDependency(FFP_LIB_TEXTURESTAGE);

	return true;
}

//-----------------------------------------------------------------------
bool FFPTextureStage::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain   = vsProgram->getEntryPointFunction();
	Function* psMain   = psProgram->getEntryPointFunction();


	if (false == addVSFunctionInvocations(vsMain))
		return false;

	if (false == addPSFunctionInvocations(psMain))
		return false;

	return true;
}

//-----------------------------------------------------------------------
bool FFPTextureStage::addVSFunctionInvocations(Function* vsMain)
{
	TexCoordCalcMethod texCoordCalcMethod = getTexCalcMethod();
	FunctionInvocation* texCoordCalcFunc = NULL;

	
	switch (texCoordCalcMethod)
	{
	case TEXCALC_NONE:
		if (mTextureMatrix == NULL)
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_ASSIGN,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mVSInputTexCoord->getName());
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());					
		}
		else
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_TRANSFORM_TEXCOORD,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mTextureMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mVSInputTexCoord->getName());
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}						
		break;

	case TEXCALC_ENVIRONMENT_MAP:
	case TEXCALC_ENVIRONMENT_MAP_PLANAR:
		if (mTextureMatrix == NULL)
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mWorldITMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mViewMatrix->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSInputNormal->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}
		else
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mWorldITMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mViewMatrix->getName());	
			texCoordCalcFunc->getParameterList().push_back(mTextureMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mVSInputNormal->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}			
		break;

			
	case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
		if (mTextureMatrix == NULL)
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mWorldMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mWorldITMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mViewMatrix->getName());					
			texCoordCalcFunc->getParameterList().push_back(mVSInputNormal->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSInputPos->getName());				
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}
		else
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mWorldMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mWorldITMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mViewMatrix->getName());					
			texCoordCalcFunc->getParameterList().push_back(mTextureMatrix->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSInputNormal->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSInputPos->getName());				
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}			
		break;

	case TEXCALC_ENVIRONMENT_MAP_NORMAL:
		if (mTextureMatrix == NULL)
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mWorldITMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mViewMatrix->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSInputNormal->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}
		else
		{
			texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

			texCoordCalcFunc->getParameterList().push_back(mWorldITMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mViewMatrix->getName());	
			texCoordCalcFunc->getParameterList().push_back(mTextureMatrix->getName());
			texCoordCalcFunc->getParameterList().push_back(mVSInputNormal->getName());	
			texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());
		}			
		break;
		break;

	case TEXCALC_PROJECTIVE_TEXTURE:

		texCoordCalcFunc = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_PROJECTION,  FFP_VS_TEXTURE_COORD, mTextureSamplerIndex); 

		texCoordCalcFunc->getParameterList().push_back(mWorldMatrix->getName());
		texCoordCalcFunc->getParameterList().push_back(mTextureViewProjImageMatrix->getName());	
		texCoordCalcFunc->getParameterList().push_back(mVSInputPos->getName());		
		texCoordCalcFunc->getParameterList().push_back(mVSOutputTexCoord->getName());

		break;
	}

	if (texCoordCalcFunc != NULL)
		vsMain->addAtomInstace(texCoordCalcFunc);

	return true;
}
//-----------------------------------------------------------------------
bool FFPTextureStage::addPSFunctionInvocations(Function* psMain)
{
	const LayerBlendModeEx& colourBlend = mTextureUnitState->getColourBlendMode();
	const LayerBlendModeEx& alphaBlend  = mTextureUnitState->getAlphaBlendMode();
	Parameter* source1 = NULL;
	Parameter* source2 = NULL;
	int groupOrder = FFP_PS_TEXTURE_STAGE0 + 100 * mTextureSamplerIndex;
	int internalCounter = 0;
	
			
	// Add texture sampling code.
	Parameter* texel = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, GCT_FLOAT4, "texel");
	FunctionInvocation* curFuncInvocation = NULL;
	
	if (getTexCalcMethod() == TEXCALC_PROJECTIVE_TEXTURE)
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE_PROJ, groupOrder, internalCounter++);
	else	
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE, groupOrder, internalCounter++);

	curFuncInvocation->getParameterList().push_back(mTextureSampler->getName());
	curFuncInvocation->getParameterList().push_back(mPSInputTexCoord->getName());
	curFuncInvocation->getParameterList().push_back(texel->getName());
	psMain->addAtomInstace(curFuncInvocation);

	// Build colour argument for source1.
	source1 = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, GCT_FLOAT4, "source1");
		
	addPSArgumentInvocations(psMain, source1, texel, 
		colourBlend.source1, colourBlend.colourArg1, 
		colourBlend.alphaArg1, false, groupOrder, internalCounter);

	// Build colour argument for source2.
	source2 = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, GCT_FLOAT4, "source2");

	addPSArgumentInvocations(psMain, source2, texel, 
		colourBlend.source2, colourBlend.colourArg2, 
		colourBlend.alphaArg2, false, groupOrder, internalCounter);

	// Build colours blend
	addPSBlendInvocations(psMain, source1, source2, texel, colourBlend, groupOrder, internalCounter);

	// Case we need different alpha channel code.
	if (alphaBlend.operation != colourBlend.operation ||
		alphaBlend.source1 != colourBlend.source1 ||
		alphaBlend.source2 != colourBlend.source2)
	{
		// Build alpha argument for source1.
		addPSArgumentInvocations(psMain, source1, texel, 
			alphaBlend.source1, alphaBlend.colourArg1, 
			alphaBlend.alphaArg1, true, groupOrder, internalCounter);

		// Build alpha argument for source2.
		addPSArgumentInvocations(psMain, source2, texel, 
			alphaBlend.source2, alphaBlend.colourArg2, 
			alphaBlend.alphaArg2, true, groupOrder, internalCounter);

		// Build alpha blend
		addPSBlendInvocations(psMain, source1, source2, texel, alphaBlend, groupOrder, internalCounter);
	}
	
	

	return true;
}

//-----------------------------------------------------------------------
void FFPTextureStage::addPSArgumentInvocations(Function* psMain, 
											 Parameter* arg,
											 Parameter* texel,
											 LayerBlendSource blendSrc,
											 const ColourValue& colourValue,
											 Real alphaValue,
											 bool isAlphaArgument,
											 const int groupOrder, 
											 int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;

	switch(blendSrc)
	{
	case LBS_CURRENT:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		if (mTextureSamplerIndex == 0)
			curFuncInvocation->getParameterList().push_back(mPSDiffuseColor->getName());
		else
			curFuncInvocation->getParameterList().push_back(mPSOutColor->getName());
		curFuncInvocation->getParameterList().push_back(arg->getName());		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBS_TEXTURE:		
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(texel->getName());
		curFuncInvocation->getParameterList().push_back(arg->getName());		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBS_DIFFUSE:		
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);	
		curFuncInvocation->getParameterList().push_back(mPSDiffuseColor->getName());		
		curFuncInvocation->getParameterList().push_back(arg->getName());		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBS_SPECULAR:		
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);	
		curFuncInvocation->getParameterList().push_back(mPSSpecularColor->getName());		
		curFuncInvocation->getParameterList().push_back(arg->getName());		
		psMain->addAtomInstace(curFuncInvocation);	
		break;

	case LBS_MANUAL:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_CONSTRUCT, groupOrder, internalCounter++);

		if (isAlphaArgument == false)
		{
			curFuncInvocation->getParameterList().push_back(StringConverter::toString(alphaValue));						
		}
		else
		{
			curFuncInvocation->getParameterList().push_back(StringConverter::toString(colourValue.r));		
			curFuncInvocation->getParameterList().push_back(StringConverter::toString(colourValue.g));		
			curFuncInvocation->getParameterList().push_back(StringConverter::toString(colourValue.b));		
			curFuncInvocation->getParameterList().push_back(StringConverter::toString(colourValue.a));		
		}
		
		curFuncInvocation->getParameterList().push_back(arg->getName());	
		psMain->addAtomInstace(curFuncInvocation);	
		break;
	}
}

//-----------------------------------------------------------------------
void FFPTextureStage::addPSBlendInvocations(Function* psMain, 
										  Parameter* arg1,
										  Parameter* arg2,
										  Parameter* texel,
										  const LayerBlendModeEx& blendMode,
										  const int groupOrder, 
										  int& internalCounter)
{
	FunctionInvocation* curFuncInvocation = NULL;
	String arg1ParamName = arg1->getName();
	String arg2ParamName = arg2->getName();
	String outParamName = mPSOutColor->getName();

	if (blendMode.blendType == LBT_ALPHA)	
	{
		arg1ParamName += ".a";
		arg2ParamName += ".a";
		outParamName += ".a";	
	}
	

	switch(blendMode.operation)
	{
	case LBX_SOURCE1:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);						
		break;
	case LBX_SOURCE2:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);							
		break;
	case LBX_MODULATE:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_MODULATE_X2:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATEX2, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_MODULATE_X4:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_MODULATEX4, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);	
		break;
	case LBX_ADD:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_ADD_SIGNED:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADDSIGNED, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);				
		break;
	case LBX_ADD_SMOOTH:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_ADDMOOTH, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_SUBTRACT:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);	
		break;
	case LBX_BLEND_DIFFUSE_ALPHA:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(mPSDiffuseColor->getName() + ".a");
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_TEXTURE_ALPHA:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(texel->getName() + ".a");
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_CURRENT_ALPHA:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);

		if (mTextureSamplerIndex == 0)
			curFuncInvocation->getParameterList().push_back(mPSDiffuseColor->getName() + ".a");
		else
			curFuncInvocation->getParameterList().push_back(mPSOutColor->getName() + ".a");
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_MANUAL:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(StringConverter::toString(blendMode.factor));
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);
		break;
	case LBX_DOTPRODUCT:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_DOTPRODUCT, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);		
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_DIFFUSE_COLOUR:
		curFuncInvocation = new FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->getParameterList().push_back(arg2ParamName);
		curFuncInvocation->getParameterList().push_back(arg1ParamName);
		curFuncInvocation->getParameterList().push_back(mPSDiffuseColor->getName());
		curFuncInvocation->getParameterList().push_back(outParamName);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	}
}

//-----------------------------------------------------------------------
TexCoordCalcMethod FFPTextureStage::getTexCalcMethod()
{
	TexCoordCalcMethod						texCoordCalcMethod = TEXCALC_NONE;	
	const TextureUnitState::EffectMap&		effectMap = mTextureUnitState->getEffects();	
	TextureUnitState::EffectMap::const_iterator	effi;
	
	for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
	{
		switch (effi->second.type)
		{
		case TextureUnitState::ET_ENVIRONMENT_MAP:
			if (effi->second.subtype == TextureUnitState::ENV_CURVED)
			{
				texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP;				
			}
			else if (effi->second.subtype == TextureUnitState::ENV_PLANAR)
			{
				texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP_PLANAR;				
			}
			else if (effi->second.subtype == TextureUnitState::ENV_REFLECTION)
			{
				texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP_REFLECTION;				
			}
			else if (effi->second.subtype == TextureUnitState::ENV_NORMAL)
			{
				texCoordCalcMethod = TEXCALC_ENVIRONMENT_MAP_NORMAL;				
			}
			break;
		case TextureUnitState::ET_UVSCROLL:
		case TextureUnitState::ET_USCROLL:
		case TextureUnitState::ET_VSCROLL:
		case TextureUnitState::ET_ROTATE:
		case TextureUnitState::ET_TRANSFORM:
			break;
		case TextureUnitState::ET_PROJECTIVE_TEXTURE:
			texCoordCalcMethod = TEXCALC_PROJECTIVE_TEXTURE;
			break;
		}
	}

	return texCoordCalcMethod;
}

//-----------------------------------------------------------------------
bool FFPTextureStage::needsTextureMatrix()
{
	TexCoordCalcMethod						texCoordCalcMethod = TEXCALC_NONE;	
	const TextureUnitState::EffectMap&		effectMap = mTextureUnitState->getEffects();	
	TextureUnitState::EffectMap::const_iterator	effi;

	for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
	{
		switch (effi->second.type)
		{
	
		case TextureUnitState::ET_UVSCROLL:
		case TextureUnitState::ET_USCROLL:
		case TextureUnitState::ET_VSCROLL:
		case TextureUnitState::ET_ROTATE:
		case TextureUnitState::ET_TRANSFORM:
			return true;		
		}
	}

	const Ogre::Matrix4 matTexture = mTextureUnitState->getTextureTransform();

	// Resolve texture matrix parameter.
	if (matTexture != Matrix4::IDENTITY)
		return true;

	return false;
}


//-----------------------------------------------------------------------
void FFPTextureStage::copyFrom(const SubRenderState& rhs)
{
	const FFPTextureStage& rhsTexture = static_cast<const FFPTextureStage&>(rhs);

	setTextureSamplerIndex(rhsTexture.mTextureSamplerIndex);
	setTextureUnitState(rhsTexture.mTextureUnitState);	
}

//-----------------------------------------------------------------------
uint32 FFPTextureStage::getHashCode()
{
	uint32 hashCode = 0;
	
	boost::hash_combine(hashCode, SubRenderState::getHashCode());
	boost::hash_combine(hashCode, mTextureSamplerIndex);
	boost::hash_combine(hashCode, mTextureUnitState->getTextureType());

	if (needsTextureMatrix())
		boost::hash_combine(hashCode, mTextureSamplerIndex);	

	const LayerBlendModeEx& colourBlend = mTextureUnitState->getColourBlendMode();
	const LayerBlendModeEx& alphaBlend  = mTextureUnitState->getAlphaBlendMode();

	boost::hash_combine(hashCode, colourBlend.operation);	
	boost::hash_combine(hashCode, colourBlend.source1);	
	boost::hash_combine(hashCode, colourBlend.source2);	

	if (colourBlend.source1 == LBS_MANUAL)
	{	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.r));	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.g));	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.b));	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.a));	
	}

	if (colourBlend.source2 == LBS_MANUAL)
	{	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.r));	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.g));	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.b));	
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.a));		
	}
	
	if (colourBlend.operation == LBX_BLEND_MANUAL)
	{		
		boost::hash_combine(hashCode, _INT_VALUE(colourBlend.factor));				
	}

	boost::hash_combine(hashCode, alphaBlend.operation);	
	boost::hash_combine(hashCode, alphaBlend.source1);	
	boost::hash_combine(hashCode, alphaBlend.source2);	


	if (alphaBlend.source1 == LBS_MANUAL)
	{		
		boost::hash_combine(hashCode, _INT_VALUE(alphaBlend.alphaArg1));				
	}

	if (alphaBlend.source2 == LBS_MANUAL)
	{		
		boost::hash_combine(hashCode, _INT_VALUE(alphaBlend.alphaArg2));	
	}

	if (alphaBlend.operation == LBX_BLEND_MANUAL)
	{		
		boost::hash_combine(hashCode, _INT_VALUE(alphaBlend.factor));			
	}

	const TextureUnitState::EffectMap&		effectMap = mTextureUnitState->getEffects();	
	TextureUnitState::EffectMap::const_iterator	effi;

	for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
	{
		boost::hash_combine(hashCode, _INT_VALUE(effi->second.type));		
	}
	
	return hashCode;
}

//-----------------------------------------------------------------------
void FFPTextureStage::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
											  const LightList* pLightList)
{
	if (mTextureProjector != NULL && mTextureViewProjImageMatrix != NULL)
	{		
		GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
		Matrix4 matTexViewProjImage;

		matTexViewProjImage = 
			Matrix4::CLIPSPACE2DTOIMAGESPACE * 
			mTextureProjector->getProjectionMatrixWithRSDepth() * 
			mTextureProjector->getViewMatrix();

		vsGpuParams->setNamedConstant(mTextureViewProjImageMatrix->getName(), matTexViewProjImage);
	}
}

//-----------------------------------------------------------------------
void FFPTextureStage::setTextureUnitState(TextureUnitState* textureUnitState)
{
	mTextureUnitState = textureUnitState;

	switch (mTextureUnitState->getTextureType())
	{
	case TEX_TYPE_1D:
		mTextureSamplerType = GCT_SAMPLER1D;
		mVSInTextureCoordinateType = GCT_FLOAT1;
		break;
	case TEX_TYPE_2D:
		mTextureSamplerType = GCT_SAMPLER2D;
		mVSInTextureCoordinateType = GCT_FLOAT2;
		break;
	case TEX_TYPE_3D:
		mTextureSamplerType = GCT_SAMPLER3D;
		mVSInTextureCoordinateType = GCT_FLOAT3;
		break;
	case TEX_TYPE_CUBE_MAP:
		mTextureSamplerType = GCT_SAMPLERCUBE;
		mVSInTextureCoordinateType = GCT_FLOAT3;
		break;
	}	

	 mVSOutTextureCoordinateType = mVSInTextureCoordinateType;

	 if (getTexCalcMethod() == TEXCALC_PROJECTIVE_TEXTURE)
		 mVSOutTextureCoordinateType = GCT_FLOAT3;

	if (mTextureUnitState->getBindingType() == TextureUnitState::BT_VERTEX)
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
			"FFP Texture unit does not support vertex texture fetch !!!",
			"FFPTextureUnit::setTextureUnitState");
	}
}

//-----------------------------------------------------------------------
void FFPTextureStage::setTextureSamplerIndex(unsigned short index)
{
	mTextureSamplerIndex = index;	
}

//-----------------------------------------------------------------------
const String& FFPTextureStageFactory::getType() const
{
	return FFPTextureStage::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	FFPTextureStageFactory::createInstanceImpl()
{
	return new FFPTextureStage;
}


}
}

