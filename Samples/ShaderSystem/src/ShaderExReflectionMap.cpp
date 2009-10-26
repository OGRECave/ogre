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
#include "ShaderExReflectionMap.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreScriptCompiler.h"



/************************************************************************/
/*                                                                      */
/************************************************************************/
String ShaderExReflectionMap::Type							= "SGX_ReflectionMap";
String ShaderExReflectionMap::MaskMapTextureNameKey			= "SGX_ReflectionMaskMapTextureName";
String ShaderExReflectionMap::ReflectionMapTextureNameKey	= "SGX_ReflectionMapTextureName";

//-----------------------------------------------------------------------
#define SGX_LIB_REFLECTIONMAP					"SampleLib_ReflectionMap"
#define SGX_FUNC_APPLY_REFLECTION_MAP			"SGX_ApplyReflectionMap"

//-----------------------------------------------------------------------
ShaderExReflectionMap::ShaderExReflectionMap()
{
	mMaskMapSamplerIndex				= 0;			
	mReflectionMapSamplerIndex			= 0;
	mReflectionMapType					= TEX_TYPE_2D;
}

//-----------------------------------------------------------------------
const String& ShaderExReflectionMap::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
int	ShaderExReflectionMap::getExecutionOrder() const
{
	// We place this effect after texturing stage and before fog stage.
	return FFP_TEXTURING + 1;
}

//-----------------------------------------------------------------------
uint32 ShaderExReflectionMap::getHashCode()
{
	uint32 hashCode = 0;

	// Start the hash code with parent code.
	sh_hash_combine(hashCode, SubRenderState::getHashCode());

	// Mix in the mask map sampler index.
	sh_hash_combine(hashCode, mMaskMapSamplerIndex);

	// Mix in the reflection map sampler index.
	sh_hash_combine(hashCode, mReflectionMapSamplerIndex);

	// Mix in the reflection map type.
	sh_hash_combine(hashCode, mReflectionMapType);
	
	return hashCode;
}

//-----------------------------------------------------------------------
void ShaderExReflectionMap::copyFrom(const SubRenderState& rhs)
{
	const ShaderExReflectionMap& rhsReflectionMap = static_cast<const ShaderExReflectionMap&>(rhs);
	
	// Copy all settings that affect this sub render state output code.
	mMaskMapSamplerIndex = rhsReflectionMap.mMaskMapSamplerIndex;
	mReflectionMapSamplerIndex = rhsReflectionMap.mReflectionMapSamplerIndex;
	mReflectionMapType = rhsReflectionMap.mReflectionMapType;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::preAddToRenderState( RenderState* renderState, Pass* srcPass, Pass* dstPass )
{
	// Validate mask texture name.
	const Any& maskUser = srcPass->getUserObjectBindings().getUserAny(ShaderExReflectionMap::MaskMapTextureNameKey);
	if (maskUser.isEmpty())	
		return false;	

	// Validate reflection map texture name.
	const Any& reflectionUser = srcPass->getUserObjectBindings().getUserAny(ShaderExReflectionMap::ReflectionMapTextureNameKey);
	if (reflectionUser.isEmpty())	
		return false;

	const String maskMapTextureName			= any_cast<const String>(maskUser);
	const String reflectionMapTextureName	= any_cast<const String>(reflectionUser);

	
	TextureUnitState* textureUnit;
	
	// Create the mask texture unit.
	textureUnit = dstPass->createTextureUnitState();
	textureUnit->setTextureName(maskMapTextureName);		
	mMaskMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

	// Create the reflection texture unit.
	textureUnit = dstPass->createTextureUnitState();

	if (mReflectionMapType == TEX_TYPE_2D)
	{
		textureUnit->setTextureName(reflectionMapTextureName);	
	}
	else
	{
		textureUnit->setCubicTextureName(reflectionMapTextureName, true);	
	}
		
	mReflectionMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

	return true;
}

//-----------------------------------------------------------------------
void ShaderExReflectionMap::preRemoveFromRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass) 
{	
	// Erase the normal map texture association.
	srcPass->getUserObjectBindings().eraseUserAny(ShaderExReflectionMap::MaskMapTextureNameKey);	
	srcPass->getUserObjectBindings().eraseUserAny(ShaderExReflectionMap::ReflectionMapTextureNameKey);	
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::resolveParameters(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* vsMain = vsProgram->getEntryPointFunction();
	Function* psMain = psProgram->getEntryPointFunction();

	// Resolve vs input mask texture coordinates.
	// NOTE: We use the first texture coordinate hard coded here
	// You may want to parametrize this as well - just remember to add it to hash and copy methods. 
	mVSInMaskTexcoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 0, Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
	if (mVSInMaskTexcoord.get() == 0)
		return false;

	// Resolve vs output mask texture coordinates.
	mVSOutMaskTexcoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, mVSInMaskTexcoord->getContent(), GCT_FLOAT2);
	if (mVSOutMaskTexcoord.get() == 0)
		return false;

	// Resolve ps input mask texture coordinates.
	mPSInMaskTexcoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutMaskTexcoord->getIndex(), 
		mVSOutMaskTexcoord->getContent(),
		GCT_FLOAT2);

	// Resolve vs output reflection texture coordinates.
	mVSOutReflectionTexcoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
		Parameter::SPC_UNKNOWN,
		mReflectionMapType == TEX_TYPE_2D ? GCT_FLOAT2 : GCT_FLOAT3);
	if (mVSOutReflectionTexcoord.get() == 0)
		return false;

	// Resolve ps input reflection texture coordinates.
	mPSInReflectionTexcoord= psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
		mVSOutReflectionTexcoord->getIndex(), 
		mVSOutReflectionTexcoord->getContent(),
		mVSOutReflectionTexcoord->getType());


	// Resolve world matrix.	
	mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
	if (mWorldMatrix.get() == NULL)
		return false;	

	// Resolve world inverse transpose matrix.	
	mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 0);
	if (mWorldITMatrix.get() == NULL)
		return false;	


	// Resolve view matrix.
	mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
	if (mViewMatrix.get() == NULL)
		return false;	

	// Resolve vertex position.
	mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
	if (mVSInputPos.get() == NULL)
		return false;		

	// Resolve vertex normal.
	mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
	if (mVSInputNormal.get() == NULL)
		return false;		

	// Resolve mask texture sampler parameter.		
	mMaskMapSampler = psProgram->resolveParameter(GCT_SAMPLER2D, mMaskMapSamplerIndex, (uint16)GPV_GLOBAL, "mask_sampler");
	if (mMaskMapSampler.get() == NULL)
		return false;

	// Resolve reflection texture sampler parameter.		
	mReflectionMapSampler = psProgram->resolveParameter(mReflectionMapType == TEX_TYPE_2D ? GCT_SAMPLER2D : GCT_SAMPLERCUBE, 
		mReflectionMapSamplerIndex, (uint16)GPV_GLOBAL, "reflection_texture");
	if (mReflectionMapSampler.get() == NULL)
		return false;

	// Resolve ps output diffuse colour.
	mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
	if (mPSOutDiffuse.get() == NULL)
		return false;

	return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::resolveDependencies(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();
	Program* psProgram = programSet->getCpuFragmentProgram();

	vsProgram->addDependency(FFP_LIB_COMMON);
	vsProgram->addDependency(FFP_LIB_TEXTURING);
	
	psProgram->addDependency(FFP_LIB_COMMON);
	psProgram->addDependency(FFP_LIB_TEXTURING);
	psProgram->addDependency(SGX_LIB_REFLECTIONMAP);
	
	return true;
}


//-----------------------------------------------------------------------
bool ShaderExReflectionMap::addFunctionInvocations(ProgramSet* programSet)
{
	Program* vsProgram = programSet->getCpuVertexProgram();	
	Function* vsMain = vsProgram->getEntryPointFunction();	
	Program* psProgram = programSet->getCpuFragmentProgram();
	Function* psMain = psProgram->getEntryPointFunction();	
	

	// Add vertex shader invocations.
	if (false == addVSInvocations(vsMain, FFP_VS_TEXTURING + 1))
		return false;


	// Add pixel shader invocations.
	if (false == addPSInvocations(psMain, FFP_PS_TEXTURING + 1))
		return false;
	
	return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::addVSInvocations( Function* vsMain, const int groupOrder )
{
	FunctionInvocation* funcInvoaction = NULL;
	int internalCounter = 0;

	// Output mask texture coordinates.
	funcInvoaction = new FunctionInvocation(FFP_FUNC_ASSIGN,  groupOrder, internalCounter++); 
	funcInvoaction->getParameterList().push_back(mVSInMaskTexcoord->getName());
	funcInvoaction->getParameterList().push_back(mVSOutMaskTexcoord->getName());
	vsMain->addAtomInstace(funcInvoaction);

	// Output reflection texture coordinates.
	if (mReflectionMapType == TEX_TYPE_2D)
	{
		funcInvoaction = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,  groupOrder, internalCounter++); 
		funcInvoaction->getParameterList().push_back(mWorldITMatrix->getName());
		funcInvoaction->getParameterList().push_back(mViewMatrix->getName());	
		funcInvoaction->getParameterList().push_back(mVSInputNormal->getName());	
		funcInvoaction->getParameterList().push_back(mVSOutReflectionTexcoord->getName());
		vsMain->addAtomInstace(funcInvoaction);
	}
	else
	{
		funcInvoaction = new FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT, groupOrder, internalCounter++); 
		funcInvoaction->getParameterList().push_back(mWorldMatrix->getName());
		funcInvoaction->getParameterList().push_back(mWorldITMatrix->getName());
		funcInvoaction->getParameterList().push_back(mViewMatrix->getName());					
		funcInvoaction->getParameterList().push_back(mVSInputNormal->getName());	
		funcInvoaction->getParameterList().push_back(mVSInputPos->getName());				
		funcInvoaction->getParameterList().push_back(mVSOutReflectionTexcoord->getName());
		vsMain->addAtomInstace(funcInvoaction);
	}
	


	return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::addPSInvocations( Function* psMain, const int groupOrder )
{
	FunctionInvocation* funcInvoaction = NULL;
	int internalCounter = 0;

	funcInvoaction = new FunctionInvocation(SGX_FUNC_APPLY_REFLECTION_MAP, groupOrder, internalCounter++);
	funcInvoaction->getParameterList().push_back(mMaskMapSampler->getName());
	funcInvoaction->getParameterList().push_back(mPSInMaskTexcoord->getName());
	funcInvoaction->getParameterList().push_back(mReflectionMapSampler->getName());
	funcInvoaction->getParameterList().push_back(mPSInReflectionTexcoord->getName());	
	funcInvoaction->getParameterList().push_back(mPSOutDiffuse->getName() + ".xyz");
	funcInvoaction->getParameterList().push_back(mPSOutDiffuse->getName() + ".xyz");
	
	psMain->addAtomInstace(funcInvoaction);

	return true;
}

//-----------------------------------------------------------------------
void ShaderExReflectionMap::setReflectionMapType( TextureType type )
{
	if (type != TEX_TYPE_2D && type != TEX_TYPE_CUBE_MAP)
	{
		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
			"Invalid texture type set - only 2D or Cube supported",
			"ShaderExReflectionMap::setReflectionMapType");
	}
	mReflectionMapType = type;
}

//-----------------------------------------------------------------------
SubRenderState*	ShaderExReflectionMapFactory::createInstance(ScriptCompiler* compiler, 
														 PropertyAbstractNode* prop, Pass* pass)
{
	if (prop->name == "rtss_ext_reflection_map")
	{
		if(prop->values.size() >= 2)
		{
			String strValue;
			AbstractNodeList::const_iterator it = prop->values.begin();

			// Read reflection map type.
			if(false == SGScriptTranslator::getString(*it, &strValue))
			{
				compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				return NULL;
			}
			++it;

			SubRenderState* subRenderState = SubRenderStateFactory::createInstance();
			ShaderExReflectionMap* reflectionMapSubRenderState = static_cast<ShaderExReflectionMap*>(subRenderState);


			// Reflection map is cubic texture.
			if (strValue == "cube_map")
			{
				reflectionMapSubRenderState->setReflectionMapType(TEX_TYPE_CUBE_MAP);
			}

			// Reflection map is 2d texture.
			else if (strValue == "2d_map")
			{
				reflectionMapSubRenderState->setReflectionMapType(TEX_TYPE_2D);
			}
	
			// Read mask texture.
			if (false == SGScriptTranslator::getString(*it, &strValue))
			{
				compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
				return NULL;
			}
			++it;
			pass->getUserObjectBindings().setUserAny(ShaderExReflectionMap::MaskMapTextureNameKey, Any(strValue));

			// Read reflection texture.
			if (false == SGScriptTranslator::getString(*it, &strValue))
			{
				compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
				return NULL;
			}
			pass->getUserObjectBindings().setUserAny(ShaderExReflectionMap::ReflectionMapTextureNameKey, Any(strValue));
				
			return subRenderState;								
			
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------
void ShaderExReflectionMapFactory::writeInstance(MaterialSerializer* ser, 
											 SubRenderState* subRenderState, 
											 Pass* srcPass, Pass* dstPass)
{
	// Validate mask texture name.
	const Any& maskUser = srcPass->getUserObjectBindings().getUserAny(ShaderExReflectionMap::MaskMapTextureNameKey);
	if (maskUser.isEmpty())	
		return;	

	// Validate reflection map texture name.
	const Any& reflectionUser = srcPass->getUserObjectBindings().getUserAny(ShaderExReflectionMap::ReflectionMapTextureNameKey);
	if (reflectionUser.isEmpty())	
		return;

	const String maskMapTextureName			= any_cast<const String>(maskUser);
	const String reflectionMapTextureName	= any_cast<const String>(reflectionUser);


	ser->writeAttribute(4, "rtss_ext_reflection_map");
	

	ShaderExReflectionMap* reflectionMapSubRenderState = static_cast<ShaderExReflectionMap*>(subRenderState);

	if (reflectionMapSubRenderState->getReflectionMapType() == TEX_TYPE_CUBE_MAP)
	{
		ser->writeValue("cube_map");
	}
	else if (reflectionMapSubRenderState->getReflectionMapType() == TEX_TYPE_2D)
	{
		ser->writeValue("2d_map");
	}	

	ser->writeValue(maskMapTextureName);
	ser->writeValue(reflectionMapTextureName);

}



//-----------------------------------------------------------------------
const String& ShaderExReflectionMapFactory::getType() const
{
	return ShaderExReflectionMap::Type;
}


//-----------------------------------------------------------------------
SubRenderState*	ShaderExReflectionMapFactory::createInstanceImpl()
{
	return new ShaderExReflectionMap;
}
