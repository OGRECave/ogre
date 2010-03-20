/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreShaderExLayeredBlending.h"
#include "OgreShaderProgram.h"

namespace Ogre {
namespace RTShader {

#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

String LayeredBlending::Type = "LayeredBlendRTSSEx";

 struct BlendModeDescription {
        /* Type of the blend mode */
        LayeredBlending::BlendMode type;
        /* name of the blend mode. */
        const char* name;
        /* shader function name . */
        const char* funcName;
    };

BlendModeDescription _blendModes[(int)LayeredBlending::LB_MaxBlendModes] = {
 	{ LayeredBlending::LB_FFPBlend ,"blend_default", ""},
	{ LayeredBlending::LB_BlendNormal ,"blend_normal", "SGX_blend_normal"},
	{ LayeredBlending::LB_BlendLighten,"blend_lighten", "SGX_blend_lighten"},
	{ LayeredBlending::LB_BlendDarken ,"blend_darken", "SGX_blend_darken"},
	{ LayeredBlending::LB_BlendMultiply ,"blend_multiply", "SGX_blend_multiply"},
	{ LayeredBlending::LB_BlendAverage ,"blend_average", "SGX_blend_average"},
	{ LayeredBlending::LB_BlendAdd ,"blend_add", "SGX_blend_add"},
	{ LayeredBlending::LB_BlendSubtract ,"blend_subtract", "SGX_blend_subtract"},
	{ LayeredBlending::LB_BlendDifference ,"blend_difference", "SGX_blend_difference"},
	{ LayeredBlending::LB_BlendNegation ,"blend_negation", "SGX_blend_negation"},
	{ LayeredBlending::LB_BlendExclusion ,"blend_exclusion", "SGX_blend_exclusion"},
	{ LayeredBlending::LB_BlendScreen ,"blend_screen", "SGX_blend_screen"},
	{ LayeredBlending::LB_BlendOverlay ,"blend_overlay", "SGX_blend_overlay"},
	{ LayeredBlending::LB_BlendHardLight ,"blend_hardLight", "SGX_blend_hardLight"},
	{ LayeredBlending::LB_BlendSoftLight ,"blend_softLight", "SGX_blend_softLight"},
	{ LayeredBlending::LB_BlendColorDodge ,"blend_colorDodge", "SGX_blend_colorDodge"},
	{ LayeredBlending::LB_BlendColorBurn ,"blend_colorBurn", "SGX_blend_colorBurn"},
	{ LayeredBlending::LB_BlendLinearDodge ,"blend_linearDodge", "SGX_blend_linearDodge"},
	{ LayeredBlending::LB_BlendLinearBurn ,"blend_linearBurn", "SGX_blend_linearBurn"},
	{ LayeredBlending::LB_BlendLinearLight ,"blend_linearLight", "SGX_blend_linearLight"},
	{ LayeredBlending::LB_BlendVividLight ,"blend_vividLight", "SGX_blend_vividLight"},
	{ LayeredBlending::LB_BlendPinLight ,"blend_pinLight", "SGX_blend_pinLight"},
	{ LayeredBlending::LB_BlendHardMix ,"blend_hardMix", "SGX_blend_hardMix"},
	{ LayeredBlending::LB_BlendReflect ,"blend_reflect", "SGX_blend_reflect"},
	{ LayeredBlending::LB_BlendGlow ,"blend_glow", "SGX_blend_glow"},
	{ LayeredBlending::LB_BlendPhoenix ,"blend_phoenix", "SGX_blend_phoenix"},
	{ LayeredBlending::LB_BlendSaturation ,"blend_saturation", "SGX_blend_saturation"},
	{ LayeredBlending::LB_BlendColor ,"blend_color", "SGX_blend_color"},
	{ LayeredBlending::LB_BlendLuminosity, "blend_luminosity", "SGX_blend_luminosity"}
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
bool LayeredBlending::resolveDependencies(ProgramSet* programSet)
{
	FFPTexturing::resolveDependencies(programSet);
	Program* psProgram = programSet->getCpuFragmentProgram();

	psProgram->addDependency("SGXLib_LayeredBlending");

	return true;
}

//-----------------------------------------------------------------------
void LayeredBlending::copyFrom(const SubRenderState& rhs)
{
	FFPTexturing::copyFrom(rhs);
	
	const LayeredBlending& rhsTexture = static_cast<const LayeredBlending&>(rhs);
	mBlendModes = rhsTexture.mBlendModes;	
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
	FunctionInvocation* curFuncInvocation = NULL;

	BlendMode mode = LB_FFPBlend;
	
	if (((size_t)samplerIndex < mBlendModes.size()) && (samplerIndex >= 0))
	{
		mode = mBlendModes[samplerIndex];
	}

	if (LB_FFPBlend == mode)
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
			curFuncInvocation = OGRE_NEW FunctionInvocation(funcName, groupOrder, internalCounter++);
			curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
			curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
			curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
			psMain->addAtomInstace(curFuncInvocation);	
		}
	}
}

//-----------------------------------------------------------------------
void LayeredBlending::setBlendMode(unsigned short index, BlendMode mode)
{
	if(mBlendModes.size() < (size_t)index + 1)
	{
		mBlendModes.resize(index + 1);
	}
	mBlendModes[index] = mode;
}

//-----------------------------------------------------------------------
LayeredBlending::BlendMode LayeredBlending::getBlendMode(unsigned short index) const
{
	if(index < mBlendModes.size())
	{
		return mBlendModes[index];
	}
	return LB_Invalid;
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
				"Expected one of the following blend modes: blend_default, blend_normal, " \
				"blend_lighten, blend_darken, blend_multiply, blend_average, blend_add, " \
				"blend_subtract, blend_difference, blend_negation, blend_exclusion, " \
				"blend_screen, blend_overlay, blend_hardLight, blend_softLight, " \
				"blend_colorDodge, blend_colorBurn, blend_linearDodge, blend_linearBurn, " \
				"blend_linearLight, blend_vividLight, blend_pinLight, blend_hardMix, " \
				"blend_reflect, blend_glow, blend_phoenix, blend_saturation, blend_color, blend_luminosity");
			return NULL;
		}

		
		//get the layer blend sub-render state to work on
		LayeredBlending* layeredBlendState;
		
		//check if we already create a blend srs
		SubRenderState*	subState = translator->getGeneratedSubRenderState(LayeredBlending::Type);
		if (subState != NULL)
		{
			layeredBlendState = static_cast<LayeredBlending*>(subState);
		}
		else
		{
			SubRenderState* subRenderState = SubRenderStateFactory::createInstance();
			layeredBlendState = static_cast<LayeredBlending*>(subRenderState);
		}

		//update the layer sub render state
		unsigned short texIndex = texState->getParent()->getTextureUnitStateIndex(texState);
		layeredBlendState->setBlendMode(texIndex, blendMode);

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
	LayeredBlending::BlendMode blendMode = layeredBlendingSubRenderState->getBlendMode(texIndex);

	//write blend mode
	if (blendMode != LayeredBlending::LB_Invalid)
	{
		ser->writeAttribute(5, "layered_blend");	
		ser->writeValue(blendModeToString(blendMode));
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

}
}
#endif


