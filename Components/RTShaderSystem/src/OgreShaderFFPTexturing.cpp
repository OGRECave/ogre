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
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreTextureUnitState.h"
#include "OgreFrustum.h"
#include "OgrePass.h"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPTexturing::Type = "FFP_Texturing";
#define _INT_VALUE(f) (*(int*)(&(f)))

//-----------------------------------------------------------------------
FFPTexturing::FFPTexturing()
{	
}

//-----------------------------------------------------------------------
const String& FFPTexturing::getType() const
{
	return Type;
}

//-----------------------------------------------------------------------
int	FFPTexturing::getExecutionOrder() const
{		
	return FFP_TEXTURING;
}

//-----------------------------------------------------------------------
bool FFPTexturing::resolveParameters(ProgramSet* programSet)
{
	for (unsigned int i=0; i < mTextureUnitParamsList.size(); ++i)
	{
		TextureUnitParams* curParams = &mTextureUnitParamsList[i];

		if (false == resolveUniformParams(curParams, programSet))
			return false;


		if (false == resolveFunctionsParams(curParams, programSet))
			return false;
	}
	

	return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::resolveUniformParams(TextureUnitParams* textureUnitParams, ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	
	
	// Resolve texture sampler parameter.		
	textureUnitParams->mTextureSampler = psProgram->resolveParameter(textureUnitParams->mTextureSamplerType, textureUnitParams->mTextureSamplerIndex, (uint16)GPV_GLOBAL, "gTextureSampler");
	if (textureUnitParams->mTextureSampler.get() == NULL)
		return false;
	
		
	// Resolve texture matrix parameter.
	if (needsTextureMatrix(textureUnitParams->mTextureUnitState))
	{				
		textureUnitParams->mTextureMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_TEXTURE_MATRIX, textureUnitParams->mTextureSamplerIndex);
		if (textureUnitParams->mTextureMatrix.get() == NULL)
			return false;
	}

	switch (textureUnitParams->mTexCoordCalcMethod)
	{
	case TEXCALC_NONE:								
		break;

	// Resolve World + View matrices.
	case TEXCALC_ENVIRONMENT_MAP:
	case TEXCALC_ENVIRONMENT_MAP_PLANAR:	
	case TEXCALC_ENVIRONMENT_MAP_NORMAL:
		
		mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 0);
		if (mWorldITMatrix.get() == NULL)		
			return false;	
		
		mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
		if (mViewMatrix.get() == NULL)		
			return false;				
		
		break;

	case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
		mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		if (mWorldMatrix.get() == NULL)		
			return false;	

		mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 0);
		if (mWorldITMatrix.get() == NULL)		
			return false;	

		mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
		if (mViewMatrix.get() == NULL)		
			return false;	

		break;


	case TEXCALC_PROJECTIVE_TEXTURE:

		mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
		if (mWorldMatrix.get() == NULL)		
			return false;	

		textureUnitParams->mTextureViewProjImageMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_LIGHTS, "gTexViewProjImageMatrix");
		if (textureUnitParams->mTextureViewProjImageMatrix.get() == NULL)		
			return false;	

		const TextureUnitState::EffectMap&		effectMap = textureUnitParams->mTextureUnitState->getEffects();	
		TextureUnitState::EffectMap::const_iterator	effi;

		for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
		{
			if (effi->second.type == TextureUnitState::ET_PROJECTIVE_TEXTURE)
			{
				textureUnitParams->mTextureProjector = effi->second.frustum;
				break;
			}
		}

		

		if (textureUnitParams->mTextureProjector == NULL)		
			return false;	

		break;
	}

	return true;
}



//-----------------------------------------------------------------------
bool FFPTexturing::resolveFunctionsParams(TextureUnitParams* textureUnitParams, ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain   = vsProgram->getEntryPointFunction();
	Function* psMain   = psProgram->getEntryPointFunction();
	Parameter::Content texCoordContent = Parameter::SPC_UNKNOWN;
	
	switch (textureUnitParams->mTexCoordCalcMethod)
	{
		case TEXCALC_NONE:					
			// Resolve explicit vs input texture coordinates.
			
			if (textureUnitParams->mTextureMatrix.get() == NULL)
				texCoordContent = Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + textureUnitParams->mTextureUnitState->getTextureCoordSet());

			textureUnitParams->mVSInputTexCoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
				textureUnitParams->mTextureUnitState->getTextureCoordSet(), 
				Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + textureUnitParams->mTextureUnitState->getTextureCoordSet()),
				textureUnitParams->mVSInTextureCoordinateType);	
			if (textureUnitParams->mVSInputTexCoord.get() == NULL)			
				return false;		
			break;

		case TEXCALC_ENVIRONMENT_MAP:
		case TEXCALC_ENVIRONMENT_MAP_PLANAR:		
		case TEXCALC_ENVIRONMENT_MAP_NORMAL:
			// Resolve vertex normal.
			mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
			if (mVSInputNormal.get() == NULL)			
				return false;									
			break;	

		case TEXCALC_ENVIRONMENT_MAP_REFLECTION:

			// Resolve vertex normal.
			mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
			if (mVSInputNormal.get() == NULL)			
				return false;		

			// Resolve vertex position.
			mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInputPos.get() == NULL)			
				return false;		
			break;

		case TEXCALC_PROJECTIVE_TEXTURE:
			// Resolve vertex position.
			mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
			if (mVSInputPos.get() == NULL)			
				return false;		
			break;
	}

	// Resolve vs output texture coordinates.
	textureUnitParams->mVSOutputTexCoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		-1,
		texCoordContent,
		textureUnitParams->mVSOutTextureCoordinateType);

	if (textureUnitParams->mVSOutputTexCoord.get() == NULL)
		return false;
		

	// Resolve ps input texture coordinates.
	textureUnitParams->mPSInputTexCoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		textureUnitParams->mVSOutputTexCoord->getIndex(),
		textureUnitParams->mVSOutputTexCoord->getContent(),
		textureUnitParams->mVSOutTextureCoordinateType);

	if (textureUnitParams->mPSInputTexCoord.get() == NULL)
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

	mPSSpecular = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
	if (mPSSpecular.get() == NULL)
	{
		mPSSpecular = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
		if (mPSSpecular.get() == NULL)
			return false;
	}

	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSOutDiffuse.get() == NULL)	
		return false;

	
	return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_TEXTURING);	
	psProgram->addDependency(FFP_LIB_COMMON);
	psProgram->addDependency(FFP_LIB_TEXTURING);

	return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain   = vsProgram->getEntryPointFunction();
	Function* psMain   = psProgram->getEntryPointFunction();
	int internalCounter = 0;

	for (unsigned int i=0; i < mTextureUnitParamsList.size(); ++i)
	{
		TextureUnitParams* curParams = &mTextureUnitParamsList[i];

		if (false == addVSFunctionInvocations(curParams, vsMain))
			return false;

		if (false == addPSFunctionInvocations(curParams, psMain, internalCounter))
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::addVSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* vsMain)
{
	FunctionInvocation* texCoordCalcFunc = NULL;

	
	switch (textureUnitParams->mTexCoordCalcMethod)
	{
	case TEXCALC_NONE:
		if (textureUnitParams->mTextureMatrix.get() == NULL)
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(textureUnitParams->mVSInputTexCoord, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);					
		}
		else
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM_TEXCOORD,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(textureUnitParams->mTextureMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSInputTexCoord, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}						
		break;

	case TEXCALC_ENVIRONMENT_MAP:
	case TEXCALC_ENVIRONMENT_MAP_PLANAR:
		if (textureUnitParams->mTextureMatrix.get() == NULL)
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}
		else
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(textureUnitParams->mTextureMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}			
		break;

			
	case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
		if (textureUnitParams->mTextureMatrix.get() == NULL)
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(mWorldMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);					
			texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(mVSInputPos, Operand::OPS_IN);				
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}
		else
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(mWorldMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);					
			texCoordCalcFunc->pushOperand(textureUnitParams->mTextureMatrix, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(mVSInputPos, Operand::OPS_IN);				
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}			
		break;

	case TEXCALC_ENVIRONMENT_MAP_NORMAL:
		if (textureUnitParams->mTextureMatrix.get() == NULL)
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}
		else
		{
			texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

			texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(textureUnitParams->mTextureMatrix, Operand::OPS_IN);
			texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);	
			texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
		}			
		break;
		break;

	case TEXCALC_PROJECTIVE_TEXTURE:

		texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_PROJECTION,  FFP_VS_TEXTURING, textureUnitParams->mTextureSamplerIndex); 

		texCoordCalcFunc->pushOperand(mWorldMatrix, Operand::OPS_IN);
		texCoordCalcFunc->pushOperand(textureUnitParams->mTextureViewProjImageMatrix, Operand::OPS_IN);	
		texCoordCalcFunc->pushOperand(mVSInputPos, Operand::OPS_IN);		
		texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);

		break;
	}

	if (texCoordCalcFunc != NULL)
		vsMain->addAtomInstace(texCoordCalcFunc);

	return true;
}
//-----------------------------------------------------------------------
bool FFPTexturing::addPSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* psMain, int& internalCounter)
{
	const LayerBlendModeEx& colourBlend = textureUnitParams->mTextureUnitState->getColourBlendMode();
	const LayerBlendModeEx& alphaBlend  = textureUnitParams->mTextureUnitState->getAlphaBlendMode();
	ParameterPtr source1;
	ParameterPtr source2;
	int groupOrder = FFP_PS_TEXTURING;
	
			
	// Add texture sampling code.
	ParameterPtr texel = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "texel", GCT_FLOAT4);
	FunctionInvocation* curFuncInvocation = NULL;
	
	if (textureUnitParams->mTexCoordCalcMethod == TEXCALC_PROJECTIVE_TEXTURE)
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE_PROJ, groupOrder, internalCounter++);
	else	
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE, groupOrder, internalCounter++);

	curFuncInvocation->pushOperand(textureUnitParams->mTextureSampler, Operand::OPS_IN);
	curFuncInvocation->pushOperand(textureUnitParams->mPSInputTexCoord, Operand::OPS_IN);
	curFuncInvocation->pushOperand(texel, Operand::OPS_OUT);
	psMain->addAtomInstace(curFuncInvocation);

	// Build colour argument for source1.
	source1 = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "source1", GCT_FLOAT4);
		
	addPSArgumentInvocations(psMain, source1, texel, 
		textureUnitParams->mTextureSamplerIndex,
		colourBlend.source1, colourBlend.colourArg1, 
		colourBlend.alphaArg1, false, groupOrder, internalCounter);

	// Build colour argument for source2.
	source2 = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "source2", GCT_FLOAT4);

	addPSArgumentInvocations(psMain, source2, texel, 
		textureUnitParams->mTextureSamplerIndex,
		colourBlend.source2, colourBlend.colourArg2, 
		colourBlend.alphaArg2, false, groupOrder, internalCounter);

	bool needDifferentAlphaBlend = false;
	if (alphaBlend.operation != colourBlend.operation ||
		alphaBlend.source1 != colourBlend.source1 ||
		alphaBlend.source2 != colourBlend.source2)
		needDifferentAlphaBlend = true;

	// Build colours blend
	addPSBlendInvocations(psMain, source1, source2, texel, 
		textureUnitParams->mTextureSamplerIndex,
		colourBlend, groupOrder, internalCounter, 
		needDifferentAlphaBlend ? (Operand::OPM_X | Operand::OPM_Y | Operand::OPM_Z )  : Operand::OPM_ALL);

	// Case we need different alpha channel code.
	if (needDifferentAlphaBlend)
	{
		// Build alpha argument for source1.
		addPSArgumentInvocations(psMain, source1, texel,
			textureUnitParams->mTextureSamplerIndex, 
			alphaBlend.source1, alphaBlend.colourArg1, 
			alphaBlend.alphaArg1, true, groupOrder, internalCounter);

		// Build alpha argument for source2.
		addPSArgumentInvocations(psMain, source2, texel, 
			textureUnitParams->mTextureSamplerIndex,
			alphaBlend.source2, alphaBlend.colourArg2, 
			alphaBlend.alphaArg2, true, groupOrder, internalCounter);

		// Build alpha blend
		addPSBlendInvocations(psMain, source1, source2, texel, 
			textureUnitParams->mTextureSamplerIndex, 
			alphaBlend, groupOrder, internalCounter,
			Operand::OPM_W);
	}
	
	

	return true;
}

//-----------------------------------------------------------------------
void FFPTexturing::addPSArgumentInvocations(Function* psMain, 
											 ParameterPtr arg,
											 ParameterPtr texel,
											 int samplerIndex,
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
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		if (samplerIndex == 0)
			curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);
		else
			curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN);
		curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBS_TEXTURE:		
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(texel, Operand::OPS_IN);
		curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBS_DIFFUSE:		
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);	
		curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);		
		curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBS_SPECULAR:		
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);	
		curFuncInvocation->pushOperand(mPSSpecular, Operand::OPS_IN);		
		curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);		
		psMain->addAtomInstace(curFuncInvocation);	
		break;

	case LBS_MANUAL:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_CONSTRUCT, groupOrder, internalCounter++);

		if (isAlphaArgument == false)
		{
			curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(alphaValue), Operand::OPS_IN);						
		}
		else
		{				
			curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(colourValue.r), Operand::OPS_IN);		
			curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(colourValue.g), Operand::OPS_IN);		
			curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(colourValue.b), Operand::OPS_IN);		
			curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(colourValue.a), Operand::OPS_IN);		
		}
		
		curFuncInvocation->pushOperand(arg, Operand::OPS_IN);	
		psMain->addAtomInstace(curFuncInvocation);	
		break;
	}
}

//-----------------------------------------------------------------------
void FFPTexturing::addPSBlendInvocations(Function* psMain, 
										  ParameterPtr arg1,
										  ParameterPtr arg2,
										  ParameterPtr texel,
										  int samplerIndex,
										  const LayerBlendModeEx& blendMode,
										  const int groupOrder, 
										  int& internalCounter,
										  int targetChannels)
{
	FunctionInvocation* curFuncInvocation = NULL;

	switch(blendMode.operation)
	{
	case LBX_SOURCE1:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);						
		break;
	case LBX_SOURCE2:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);							
		break;
	case LBX_MODULATE:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_MODULATE_X2:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATEX2, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_MODULATE_X4:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATEX4, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);	
		break;
	case LBX_ADD:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_ADD_SIGNED:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADDSIGNED, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);				
		break;
	case LBX_ADD_SMOOTH:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADDMOOTH, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);			
		break;
	case LBX_SUBTRACT:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);	
		break;
	case LBX_BLEND_DIFFUSE_ALPHA:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_W);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_TEXTURE_ALPHA:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(texel, Operand::OPS_IN, Operand::OPM_W);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_CURRENT_ALPHA:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_OUT, targetChannels);

		if (samplerIndex == 0)
			curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_W);
		else
			curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN, Operand::OPM_W);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_MANUAL:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(ParameterFactory::createConstParamFloat(blendMode.factor), Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);
		break;
	case LBX_DOTPRODUCT:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_DOTPRODUCT, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);		
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	case LBX_BLEND_DIFFUSE_COLOUR:
		curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);		
		break;
	}
}

//-----------------------------------------------------------------------
TexCoordCalcMethod FFPTexturing::getTexCalcMethod(TextureUnitState* textureUnitState)
{
	TexCoordCalcMethod						texCoordCalcMethod = TEXCALC_NONE;	
	const TextureUnitState::EffectMap&		effectMap = textureUnitState->getEffects();	
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
bool FFPTexturing::needsTextureMatrix(TextureUnitState* textureUnitState)
{
	TexCoordCalcMethod						texCoordCalcMethod = TEXCALC_NONE;	
	const TextureUnitState::EffectMap&		effectMap = textureUnitState->getEffects();	
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

	const Ogre::Matrix4 matTexture = textureUnitState->getTextureTransform();

	// Resolve texture matrix parameter.
	if (matTexture != Matrix4::IDENTITY)
		return true;

	return false;
}


//-----------------------------------------------------------------------
void FFPTexturing::copyFrom(const SubRenderState& rhs)
{
	const FFPTexturing& rhsTexture = static_cast<const FFPTexturing&>(rhs);

	setTextureUnitCount(rhsTexture.getTextureUnitCount());

	for (unsigned int i=0; i < rhsTexture.getTextureUnitCount(); ++i)
	{
		setTextureUnit(i, rhsTexture.mTextureUnitParamsList[i].mTextureUnitState);
	}		
}

//-----------------------------------------------------------------------
bool FFPTexturing::preAddToRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
	setTextureUnitCount(srcPass->getNumTextureUnitStates());

	// Build texture stage sub states.
	for (unsigned short i=0; i < srcPass->getNumTextureUnitStates(); ++i)
	{		
		TextureUnitState* texUnitState = srcPass->getTextureUnitState(i);								

		setTextureUnit(i, texUnitState);			
	}	

	return true;
}

//-----------------------------------------------------------------------
uint32 FFPTexturing::getHashCode()
{
	uint32 hashCode = 0;
	
	sh_hash_combine(hashCode, SubRenderState::getHashCode());

	for (unsigned int i=0; i < mTextureUnitParamsList.size(); ++i)
	{
		TextureUnitParams* curParams = &mTextureUnitParamsList[i];

		sh_hash_combine(hashCode, curParams->mTextureSamplerIndex);
		sh_hash_combine(hashCode, curParams->mTextureUnitState->getTextureType());

		if (needsTextureMatrix(curParams->mTextureUnitState))
			sh_hash_combine(hashCode, curParams->mTextureSamplerIndex);	

		const LayerBlendModeEx& colourBlend = curParams->mTextureUnitState->getColourBlendMode();
		const LayerBlendModeEx& alphaBlend  = curParams->mTextureUnitState->getAlphaBlendMode();

		sh_hash_combine(hashCode, colourBlend.operation);	
		sh_hash_combine(hashCode, colourBlend.source1);	
		sh_hash_combine(hashCode, colourBlend.source2);	

		if (colourBlend.source1 == LBS_MANUAL)
		{	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.r));	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.g));	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.b));	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg1.a));	
		}

		if (colourBlend.source2 == LBS_MANUAL)
		{	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.r));	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.g));	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.b));	
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.colourArg2.a));		
		}

		if (colourBlend.operation == LBX_BLEND_MANUAL)
		{		
			sh_hash_combine(hashCode, _INT_VALUE(colourBlend.factor));				
		}

		sh_hash_combine(hashCode, alphaBlend.operation);	
		sh_hash_combine(hashCode, alphaBlend.source1);	
		sh_hash_combine(hashCode, alphaBlend.source2);	


		if (alphaBlend.source1 == LBS_MANUAL)
		{		
			sh_hash_combine(hashCode, _INT_VALUE(alphaBlend.alphaArg1));				
		}

		if (alphaBlend.source2 == LBS_MANUAL)
		{		
			sh_hash_combine(hashCode, _INT_VALUE(alphaBlend.alphaArg2));	
		}

		if (alphaBlend.operation == LBX_BLEND_MANUAL)
		{		
			sh_hash_combine(hashCode, _INT_VALUE(alphaBlend.factor));			
		}

		const TextureUnitState::EffectMap&		effectMap = curParams->mTextureUnitState->getEffects();	
		TextureUnitState::EffectMap::const_iterator	effi;
		bool hasEnvEffect = false;
		bool hasProjEffect = false;
		bool hasTransformEffect = false;

		// Check which effects applied on the current texture unit.
		for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
		{
			if (effi->second.type == TextureUnitState::ET_ENVIRONMENT_MAP)
			{
				hasEnvEffect = true;
			}
			else if (effi->second.type == TextureUnitState::ET_PROJECTIVE_TEXTURE)
			{
				hasProjEffect = true;
			}
			else if (effi->second.type == TextureUnitState::ET_UVSCROLL ||
				effi->second.type == TextureUnitState::ET_USCROLL ||
				effi->second.type == TextureUnitState::ET_VSCROLL ||
				effi->second.type == TextureUnitState::ET_ROTATE ||
				effi->second.type == TextureUnitState::ET_TRANSFORM)
			{
				hasTransformEffect = true;
			}			
		}

		if (hasEnvEffect)
		{
			sh_hash_combine(hashCode, TextureUnitState::ET_ENVIRONMENT_MAP);		
		}

		if (hasProjEffect)
		{
			sh_hash_combine(hashCode, TextureUnitState::ET_PROJECTIVE_TEXTURE);		
		}

		if (hasTransformEffect)
		{
			sh_hash_combine(hashCode, TextureUnitState::ET_TRANSFORM);		
		}
	}
			
	
	return hashCode;
}

//-----------------------------------------------------------------------
void FFPTexturing::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
											  const LightList* pLightList)
{
	for (unsigned int i=0; i < mTextureUnitParamsList.size(); ++i)
	{
		TextureUnitParams* curParams = &mTextureUnitParamsList[i];

		if (curParams->mTextureProjector != NULL && curParams->mTextureViewProjImageMatrix.get() != NULL)
		{		
			GpuProgramParametersSharedPtr vsGpuParams = pass->getVertexProgramParameters();
			Matrix4 matTexViewProjImage;

			matTexViewProjImage = 
				Matrix4::CLIPSPACE2DTOIMAGESPACE * 
				curParams->mTextureProjector->getProjectionMatrixWithRSDepth() * 
				curParams->mTextureProjector->getViewMatrix();

			vsGpuParams->setNamedConstant(curParams->mTextureViewProjImageMatrix->getName(), matTexViewProjImage);
		}
	}
}

//-----------------------------------------------------------------------
void FFPTexturing::setTextureUnitCount(size_t count)
{
	mTextureUnitParamsList.resize(count);

	for (unsigned int i=0; i < count; ++i)
	{
		TextureUnitParams& curParams = mTextureUnitParamsList[i];

		curParams.mTextureUnitState				= NULL;			
		curParams.mTextureProjector				= NULL;				  
		curParams.mTextureSamplerIndex			= NULL;			  
		curParams.mTextureSamplerType			= GCT_SAMPLER2D;		
		curParams.mVSInTextureCoordinateType	= GCT_FLOAT2;	
		curParams.mVSOutTextureCoordinateType	= GCT_FLOAT2;		
	}
}

//-----------------------------------------------------------------------
void FFPTexturing::setTextureUnit(unsigned short index, TextureUnitState* textureUnitState)
{
	if (index >= mTextureUnitParamsList.size())
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
			"FFPTexturing unit index out of bounds !!!",
			"FFPTexturing::setTextureState");
	}

	if (textureUnitState->getBindingType() == TextureUnitState::BT_VERTEX)
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
			"FFP Texture unit does not support vertex texture fetch !!!",
			"FFPTexturing::setTextureState");
	}
	

	TextureUnitParams& curParams = mTextureUnitParamsList[index];


	curParams.mTextureSamplerIndex = index;
	curParams.mTextureUnitState    = textureUnitState;

	switch (curParams.mTextureUnitState->getTextureType())
	{
	case TEX_TYPE_1D:
		curParams.mTextureSamplerType = GCT_SAMPLER1D;
		curParams.mVSInTextureCoordinateType = GCT_FLOAT1;
		break;
	case TEX_TYPE_2D:
		curParams.mTextureSamplerType = GCT_SAMPLER2D;
		curParams.mVSInTextureCoordinateType = GCT_FLOAT2;
		break;
	case TEX_TYPE_3D:
		curParams.mTextureSamplerType = GCT_SAMPLER3D;
		curParams.mVSInTextureCoordinateType = GCT_FLOAT3;
		break;
	case TEX_TYPE_CUBE_MAP:
		curParams.mTextureSamplerType = GCT_SAMPLERCUBE;
		curParams.mVSInTextureCoordinateType = GCT_FLOAT3;
		break;
	}	

	 curParams.mVSOutTextureCoordinateType = curParams.mVSInTextureCoordinateType;
	 curParams.mTexCoordCalcMethod = getTexCalcMethod(curParams.mTextureUnitState);

	 if (curParams.mTexCoordCalcMethod == TEXCALC_PROJECTIVE_TEXTURE)
		 curParams.mVSOutTextureCoordinateType = GCT_FLOAT3;	
}


//-----------------------------------------------------------------------
const String& FFPTexturingFactory::getType() const
{
	return FFPTexturing::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	FFPTexturingFactory::createInstance(ScriptCompiler* compiler, 
												 PropertyAbstractNode* prop, Pass* pass)
{
	if (prop->name == "texturing_stage")
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
void FFPTexturingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
									 Pass* srcPass, Pass* dstPass)
{
	ser->writeAttribute(4, "texturing_stage");
	ser->writeValue("ffp");
}

//-----------------------------------------------------------------------
SubRenderState*	FFPTexturingFactory::createInstanceImpl()
{
	return OGRE_NEW FFPTexturing;
}


}
}

