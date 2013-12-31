/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreShaderExLayeredBlending.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderProgram.h"

namespace Ogre {
namespace RTShader {


String LayeredBlending::Type = "LayeredBlendRTSSEx";

 struct BlendModeDescription {
        /* Type of the blend mode */
        LayeredBlending::BlendMode type;
        /* name of the blend mode. */
        const char* name;
        /* shader function name . */
        const char* funcName;
    };

const BlendModeDescription _blendModes[(int)LayeredBlending::LB_MaxBlendModes] = {
 	{ LayeredBlending::LB_FFPBlend ,"default", ""},
	{ LayeredBlending::LB_BlendNormal ,"normal", "SGX_blend_normal"},
	{ LayeredBlending::LB_BlendLighten,"lighten", "SGX_blend_lighten"},
	{ LayeredBlending::LB_BlendDarken ,"darken", "SGX_blend_darken"},
	{ LayeredBlending::LB_BlendMultiply ,"multiply", "SGX_blend_multiply"},
	{ LayeredBlending::LB_BlendAverage ,"average", "SGX_blend_average"},
	{ LayeredBlending::LB_BlendAdd ,"add", "SGX_blend_add"},
	{ LayeredBlending::LB_BlendSubtract ,"subtract", "SGX_blend_subtract"},
	{ LayeredBlending::LB_BlendDifference ,"difference", "SGX_blend_difference"},
	{ LayeredBlending::LB_BlendNegation ,"negation", "SGX_blend_negation"},
	{ LayeredBlending::LB_BlendExclusion ,"exclusion", "SGX_blend_exclusion"},
	{ LayeredBlending::LB_BlendScreen ,"screen", "SGX_blend_screen"},
	{ LayeredBlending::LB_BlendOverlay ,"overlay", "SGX_blend_overlay"},
	{ LayeredBlending::LB_BlendHardLight ,"hard_light", "SGX_blend_hardLight"},
	{ LayeredBlending::LB_BlendSoftLight ,"soft_light", "SGX_blend_softLight"},
	{ LayeredBlending::LB_BlendColorDodge ,"color_dodge", "SGX_blend_colorDodge"},
	{ LayeredBlending::LB_BlendColorBurn ,"color_burn", "SGX_blend_colorBurn"},
	{ LayeredBlending::LB_BlendLinearDodge ,"linear_dodge", "SGX_blend_linearDodge"},
	{ LayeredBlending::LB_BlendLinearBurn ,"linear_burn", "SGX_blend_linearBurn"},
	{ LayeredBlending::LB_BlendLinearLight ,"linear_light", "SGX_blend_linearLight"},
	{ LayeredBlending::LB_BlendVividLight ,"vivid_light", "SGX_blend_vividLight"},
	{ LayeredBlending::LB_BlendPinLight ,"pin_light", "SGX_blend_pinLight"},
	{ LayeredBlending::LB_BlendHardMix ,"hard_mix", "SGX_blend_hardMix"},
	{ LayeredBlending::LB_BlendReflect ,"reflect", "SGX_blend_reflect"},
	{ LayeredBlending::LB_BlendGlow ,"glow", "SGX_blend_glow"},
	{ LayeredBlending::LB_BlendPhoenix ,"phoenix", "SGX_blend_phoenix"},
	{ LayeredBlending::LB_BlendSaturation ,"saturation", "SGX_blend_saturation"},
	{ LayeredBlending::LB_BlendColor ,"color", "SGX_blend_color"},
	{ LayeredBlending::LB_BlendLuminosity, "luminosity", "SGX_blend_luminosity"}
	};
		

 struct SourceModifierDescription {
        /* Type of the source modifier*/
        LayeredBlending::SourceModifier type;
        /* name of the source modifier. */
        const char* name;
        /* shader function name . */
        const char* funcName;
    };

const SourceModifierDescription _sourceModifiers[(int)LayeredBlending::SM_MaxSourceModifiers] = {
 	{ LayeredBlending::SM_None ,""},
	{ LayeredBlending::SM_Source1Modulate ,"src1_modulate"},
	{ LayeredBlending::SM_Source2Modulate ,"src2_modulate"},
	{ LayeredBlending::SM_Source1InvModulate ,"src1_inverse_modulate"},
	{ LayeredBlending::SM_Source2InvModulate ,"src2_inverse_modulate"}
	};
//-----------------------------------------------------------------------
LayeredBlending::LayeredBlending()
{

}

//-----------------------------------------------------------------------
const Ogre::String& LayeredBlending::getType() const
{
	return Type;
}


//-----------------------------------------------------------------------
bool LayeredBlending::resolveParameters(ProgramSet* programSet)
{

	//resolve peremeter for normal texturing procedures
	bool isSuccess = FFPTexturing::resolveParameters(programSet);
	
	if (isSuccess)
	{
		//resolve source modification parameters
		Program* psProgram = programSet->getCpuFragmentProgram();

		for(size_t i = mTextureBlends.size() - 1; i != (size_t)-1 ; --i)
		{
			TextureBlend& texBlend = mTextureBlends[i];
			if ((texBlend.sourceModifier != SM_Invalid) && 
				(texBlend.sourceModifier != SM_None))
			{
				
				texBlend.modControlParam = psProgram->resolveAutoParameterInt(
					GpuProgramParameters::ACT_CUSTOM, texBlend.customNum);
				if (texBlend.modControlParam.get() == NULL)
				{	
					isSuccess = false;
					break;
				}
			}
		}
	}
	return isSuccess;
}



//-----------------------------------------------------------------------
bool LayeredBlending::resolveDependencies(ProgramSet* programSet)
{
	FFPTexturing::resolveDependencies(programSet);
	Program* psProgram = programSet->getCpuFragmentProgram();

	psProgram->addDependency(SGX_LIB_LAYEREDBLENDING);

	return true;
}

//-----------------------------------------------------------------------
void LayeredBlending::copyFrom(const SubRenderState& rhs)
{
	FFPTexturing::copyFrom(rhs);
	
	const LayeredBlending& rhsTexture = static_cast<const LayeredBlending&>(rhs);
	mTextureBlends = rhsTexture.mTextureBlends;	
}

//-----------------------------------------------------------------------
void LayeredBlending::addPSBlendInvocations(Function* psMain, 
										 ParameterPtr arg1,
										 ParameterPtr arg2,
										 ParameterPtr texel,
										 int samplerIndex,
										 const LayerBlendModeEx& blendMode,
										 const int groupOrder, 
										 int& internalCounter,
										 int targetChannels)
{
	//
	// Add the modifier invocation
	//

	addPSModifierInvocation(psMain, samplerIndex, arg1, arg2, groupOrder, internalCounter, targetChannels);

	//
	// Add the blending function invocations
	//

	BlendMode mode = getBlendMode(samplerIndex);
	
	if ((LB_FFPBlend == mode) || (LB_Invalid == mode))
	{
		FFPTexturing::addPSBlendInvocations(psMain, arg1, arg2, texel, samplerIndex, blendMode, groupOrder, internalCounter, targetChannels);
	}
	else 
	{
		//find the function name for the blend mode
		String funcName;
		for(int i = 0 ; i < (int)LayeredBlending::LB_MaxBlendModes ; ++i)
		{
			if (_blendModes[i].type == mode)
			{
				funcName = _blendModes[i].funcName;
				break;
			}
		}

		//add the function of the blend mode
		if (funcName.empty() == false)
		{
			FunctionInvocation* curFuncInvocation = OGRE_NEW FunctionInvocation(funcName, groupOrder, internalCounter++);
			curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
			curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
			curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
			psMain->addAtomInstance(curFuncInvocation);	
		}
	}
}

//-----------------------------------------------------------------------
void LayeredBlending::addPSModifierInvocation(Function* psMain, 
										 int samplerIndex, 
										 ParameterPtr arg1,
										 ParameterPtr arg2,
										 const int groupOrder, 
										 int& internalCounter,
										 int targetChannels)
{
	SourceModifier modType;
	int customNum;
	if (getSourceModifier(samplerIndex, modType, customNum) == true)
	{
		ParameterPtr modifiedParam;
		String funcName;
		switch (modType)
		{
		case SM_Source1Modulate: 
			funcName = "SGX_src_mod_modulate";
			modifiedParam = arg1;
			break;
		case SM_Source2Modulate:
			funcName = "SGX_src_mod_modulate";
			modifiedParam = arg2;
			break;
		case SM_Source1InvModulate:
			funcName = "SGX_src_mod_inv_modulate";
			modifiedParam = arg1;
			break;
		case SM_Source2InvModulate:
			funcName = "SGX_src_mod_inv_modulate";
			modifiedParam = arg2;
			break;
		default:
			break;
		}

		//add the function of the blend mode
		if (funcName.empty() == false)
		{
			ParameterPtr& controlParam = mTextureBlends[samplerIndex].modControlParam;

			FunctionInvocation* curFuncInvocation = OGRE_NEW FunctionInvocation(funcName, groupOrder, internalCounter++);
			curFuncInvocation->pushOperand(modifiedParam, Operand::OPS_IN, targetChannels);
			curFuncInvocation->pushOperand(controlParam, Operand::OPS_IN, targetChannels);
			curFuncInvocation->pushOperand(modifiedParam, Operand::OPS_OUT, targetChannels);		
			psMain->addAtomInstance(curFuncInvocation);	
		}
	}
}


//-----------------------------------------------------------------------
void LayeredBlending::setBlendMode(unsigned short index, BlendMode mode)
{
	if(mTextureBlends.size() < (size_t)index + 1)
	{
		mTextureBlends.resize(index + 1);
	}
	mTextureBlends[index].blendMode = mode;
}

//-----------------------------------------------------------------------
LayeredBlending::BlendMode LayeredBlending::getBlendMode(unsigned short index) const
{
	if(index < mTextureBlends.size())
	{
		return mTextureBlends[index].blendMode;
	}
	return LB_Invalid;
}


//-----------------------------------------------------------------------
void LayeredBlending::setSourceModifier(unsigned short index, SourceModifier modType, int customNum)
{
	if(mTextureBlends.size() < (size_t)index + 1)
	{
		mTextureBlends.resize(index + 1);
	}
	mTextureBlends[index].sourceModifier = modType;
	mTextureBlends[index].customNum = customNum;
}

//-----------------------------------------------------------------------
bool LayeredBlending::getSourceModifier(unsigned short index, SourceModifier& modType, int& customNum) const
{
	modType = SM_Invalid;
	customNum = 0;
	if(index < mTextureBlends.size())
	{
		modType = mTextureBlends[index].sourceModifier;
		customNum = mTextureBlends[index].customNum;
	}
	return (modType != SM_Invalid);
}


//----------------------Factory Implementation---------------------------
//-----------------------------------------------------------------------
const String& LayeredBlendingFactory::getType() const
{
	return LayeredBlending::Type;
}

//-----------------------------------------------------------------------
SubRenderState*	LayeredBlendingFactory::createInstance(ScriptCompiler* compiler, 
									PropertyAbstractNode* prop, TextureUnitState* texState, SGScriptTranslator* translator)
{
	if (prop->name == "layered_blend")
	{
		String blendType;
		if(false == SGScriptTranslator::getString(prop->values.front(), &blendType))
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
			return NULL;
		}

		LayeredBlending::BlendMode blendMode = stringToBlendMode(blendType);
		if (blendMode == LayeredBlending::LB_Invalid)
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
				"Expected one of the following blend modes: default, normal, " \
				"lighten, darken, multiply, average, add, " \
				"subtract, difference, negation, exclusion, " \
				"screen, overlay, hard_light, soft_light, " \
				"color_dodge, color_burn, linear_dodge, linear_burn, " \
				"linear_light, vivid_light, pin_light, hard_mix, " \
				"reflect, glow, phoenix, saturation, color and luminosity");
			return NULL;
		}

		
		//get the layer blend sub-render state to work on
		LayeredBlending* layeredBlendState =
			createOrRetrieveSubRenderState(translator);
		
		//update the layer sub render state
		unsigned short texIndex = texState->getParent()->getTextureUnitStateIndex(texState);
		layeredBlendState->setBlendMode(texIndex, blendMode);

		return layeredBlendState;
	}
	if (prop->name == "source_modifier")
	{
		if(prop->values.size() < 3)
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
				"Expected three or more parameters.");
			return NULL;
		}

		// Read light model type.
		bool isParseSuccess;
		String modifierString;
		String paramType;
		int customNum;
		
		AbstractNodeList::const_iterator itValue = prop->values.begin();
		isParseSuccess = SGScriptTranslator::getString(*itValue, &modifierString); 
		LayeredBlending::SourceModifier modType = stringToSourceModifier(modifierString);
		isParseSuccess &= modType != LayeredBlending::SM_Invalid;
		if(isParseSuccess == false)
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
				"Expected one of the following modifier type as first parameter: " \
				"src1_modulate, src2_modulate, src1_inverse_modulate, src2_inverse_modulate.");
			return NULL;
		}

		++itValue;
		isParseSuccess = SGScriptTranslator::getString(*itValue, &paramType); 
		isParseSuccess &= (paramType == "custom");
		if(isParseSuccess == false)
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
				"Expected reserved word custom as second parameter.");
			return NULL;
		}
		++itValue;
		isParseSuccess = SGScriptTranslator::getInt(*itValue, &customNum); 
		if(isParseSuccess == false)
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
				"Expected number of custom parameter as third parameter.");
			return NULL;
		}

		//get the layer blend sub-render state to work on
		LayeredBlending* layeredBlendState =
			createOrRetrieveSubRenderState(translator);
		
		//update the layer sub render state
		unsigned short texIndex = texState->getParent()->getTextureUnitStateIndex(texState);
		layeredBlendState->setSourceModifier(texIndex, modType, customNum);

		return layeredBlendState;
			
	}
	
	return NULL;
		
}

//-----------------------------------------------------------------------
void LayeredBlendingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
										const TextureUnitState* srcTextureState, const TextureUnitState* dstTextureState)
{
	unsigned short texIndex = srcTextureState->getParent()->
		getTextureUnitStateIndex(srcTextureState);
	
	//get blend mode for current texture unit
	LayeredBlending* layeredBlendingSubRenderState = static_cast<LayeredBlending*>(subRenderState);
	
	//write the blend mode
	LayeredBlending::BlendMode blendMode = layeredBlendingSubRenderState->getBlendMode(texIndex);
	if (blendMode != LayeredBlending::LB_Invalid)
	{
		ser->writeAttribute(5, "layered_blend");	
		ser->writeValue(blendModeToString(blendMode));
	}

	//write the source modifier
	LayeredBlending::SourceModifier modType;
	int customNum;
	if (layeredBlendingSubRenderState->getSourceModifier(texIndex, modType, customNum) == true)
	{
		ser->writeAttribute(5, "source_modifier");	
		ser->writeValue(sourceModifierToString(modType));
		ser->writeValue("custom");
		ser->writeValue(StringConverter::toString(customNum));
	}

}

//-----------------------------------------------------------------------
SubRenderState*	LayeredBlendingFactory::createInstanceImpl()
{
	return OGRE_NEW LayeredBlending;
}

//-----------------------------------------------------------------------
LayeredBlending::BlendMode LayeredBlendingFactory::stringToBlendMode(const String &strValue)
{
	for(int i = 0 ; i < (int)LayeredBlending::LB_MaxBlendModes ; ++i)
	{
		if (_blendModes[i].name == strValue)
		{
			return _blendModes[i].type;
		}
	}
	return LayeredBlending::LB_Invalid;
}

//-----------------------------------------------------------------------
String LayeredBlendingFactory::blendModeToString(LayeredBlending::BlendMode blendMode)
{
	for(int i = 0 ; i < (int)LayeredBlending::LB_MaxBlendModes ; ++i)
	{
		if (_blendModes[i].type == blendMode)
		{
			return _blendModes[i].name;
		}
	}
	return "";
}

//-----------------------------------------------------------------------
LayeredBlending::SourceModifier LayeredBlendingFactory::stringToSourceModifier(const String &strValue)
{
	for(int i = 0 ; i < (int)LayeredBlending::SM_MaxSourceModifiers ; ++i)
	{
		if (_sourceModifiers[i].name == strValue)
		{
			return _sourceModifiers[i].type;
		}
	}
	return LayeredBlending::SM_Invalid;
}

//-----------------------------------------------------------------------
String LayeredBlendingFactory::sourceModifierToString(LayeredBlending::SourceModifier modifier)
{
	for(int i = 0 ; i < (int)LayeredBlending::SM_MaxSourceModifiers ; ++i)
	{
		if (_sourceModifiers[i].type == modifier)
		{
			return _sourceModifiers[i].name;
		}
	}
	return "";
}

//-----------------------------------------------------------------------
LayeredBlending* LayeredBlendingFactory::createOrRetrieveSubRenderState(SGScriptTranslator* translator)
{
	LayeredBlending* layeredBlendState;
	//check if we already create a blend srs
	SubRenderState*	subState = translator->getGeneratedSubRenderState(getType());
	if (subState != NULL)
	{
		layeredBlendState = static_cast<LayeredBlending*>(subState);
	}
	else
	{
		SubRenderState* subRenderState = createOrRetrieveInstance(translator);
		layeredBlendState = static_cast<LayeredBlending*>(subRenderState);
	}
	return layeredBlendState;
}


}
}
#endif


