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

#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS


Ogre::String LayeredBlending::mSubRenderType = "LayeredBlendRTSSEx";

LayeredBlending::LayeredBlending(void)
{
}

//-----------------------------------------------------------------------
const Ogre::String& LayeredBlending::getType() const
{
	return mSubRenderType;
}

//-----------------------------------------------------------------------
bool LayeredBlending::resolveDependencies(ProgramSet* programSet)
{
	FFPTexturing::resolveDependencies(programSet);
	Program* psProgram = programSet->getCpuFragmentProgram();

	psProgram->addDependency("FFPLib_LayeredBlending");

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

	String funcName;
	PLayeredBlendingMode mode = LB_FFPBlend;
	
	if (((size_t)samplerIndex < mBlendModes.size()) && (samplerIndex >= 0))
	{
		mode = mBlendModes[samplerIndex];
	}

	switch(mode)
	{
	case LB_FFPBlend:
		FFPTexturing::addPSBlendInvocations(psMain, arg1, arg2, texel, samplerIndex, blendMode, groupOrder, internalCounter, targetChannels);
		break;
	case LB_BlendNormal:
		funcName = "blend_normal";
		break;
	case LB_BlendLighten:
		funcName = "blend_lighten";						
		break;
	case LB_BlendDarken:
		funcName = "blend_darken";
		break;
	case LB_BlendMultiply:
		funcName = "blend_multiply";
		break;
	case LB_BlendAverage:
		funcName = "blend_average";						
		break;
	case LB_BlendAdd:
		funcName = "blend_add";						
		break;
	case LB_BlendSubtract:
		funcName = "blend_subtract";						
		break;
	case LB_BlendDifference:
		funcName = "blend_difference";						
		break;
	case LB_BlendNegation:
		funcName = "blend_negation";						
		break;
	case LB_BlendExclusion:
		funcName = "blend_exclusion";						
		break;
	case LB_BlendScreen:
		funcName = "blend_screen";						
		break;
	case LB_BlendOverlay:
		funcName = "blend_overlay";						
		break;
	case LB_BlendHardLight:
		funcName = "blend_hardLight";						
		break;
	case LB_BlendSoftLight:
		funcName = "blend_softLight";						
		break;
	case LB_BlendColorDodge:
		funcName = "blend_colorDodge";						
		break;
	case LB_BlendColorBurn:
		funcName = "blend_colorBurn";						
		break;	
	case LB_BlendLinearDodge:
		funcName = "blend_linearDodge";						
		break;
	case LB_BlendLinearBurn:
		funcName = "blend_linearBurn";						
		break;
	case LB_BlendLinearLight:
		funcName = "blend_linearLight";						
		break;
	case LB_BlendVividLight:
		funcName = "blend_vividLight";						
		break;
	case LB_BlendPinLight:
		funcName = "blend_pinLight";						
		break;
	case LB_BlendHardMix:
		funcName = "blend_hardMix";						
		break;
	case LB_BlendReflect:
		funcName = "blend_reflect";						
		break;
	case LB_BlendGlow:
		funcName = "blend_glow";						
		break;
	case LB_BlendPhoenix:
		funcName = "blend_phoenix";						
		break;
	case LB_BlendSaturation:
		funcName = "blend_saturation";						
		break;
	case LB_BlendColor:
		funcName = "blend_color";						
		break;
	case LB_BlendLuminosity:
		funcName = "blend_luminosity";						
		break;
	}

	if(mode != LB_FFPBlend)
	{
		curFuncInvocation = OGRE_NEW FunctionInvocation(funcName, groupOrder, internalCounter++);
		curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
		curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);		
		psMain->addAtomInstace(curFuncInvocation);	
	}
}

//-----------------------------------------------------------------------
void LayeredBlending::addBlendType(int index, LayeredBlending::PLayeredBlendingMode mode)
{
	if(mBlendModes.size() < index + 1)
	{
		mBlendModes.resize(index + 1);
	}
	mBlendModes[index]= mode;
}


//----------------------Factory Implementation---------------------------
//-----------------------------------------------------------------------
const String& LayeredBlendingFactory::getType() const
{
	return LayeredBlending::mSubRenderType;
}

//-----------------------------------------------------------------------
SubRenderState*	LayeredBlendingFactory::createInstance(ScriptCompiler* compiler, 
													PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
	if (prop->name == "layered_blend")
	{
		SGScriptTranslator::TexturesParamCollection paramCollection = translator->getParamCollection();

		SubRenderState* subRenderState = SubRenderStateFactory::createInstance();
		LayeredBlending* LayeredBlendState = static_cast<LayeredBlending*>(subRenderState);
		
		String strValue = "blend_default";

		SGScriptTranslator::TexturesParamCollection::const_iterator it = paramCollection.begin();
		for( ;it != paramCollection.end(); ++it)
		{
			int index = 0;
			TextureUnitState * textureUnitState = it->first;

			Pass* pass = textureUnitState->getParent();
			Pass::TextureUnitStateIterator texIter =  pass->getTextureUnitStateIterator();
			while(texIter.hasMoreElements())
			{
				TextureUnitState* pTex = texIter.getNext();
				if(pTex == textureUnitState)
				{
					break;
				}
				++index;
			}
			
			SGScriptTranslator::Properties properties = it->second;

			SGScriptTranslator::Properties::const_iterator iter = properties.find("layered_blend");
			if(iter != properties.end())
			{
				SGScriptTranslator::PropertyValues propValues = iter->second;
				if(propValues.size() > 0)
				{
					strValue = propValues[0];
				}
			}
			LayeredBlendState->addBlendType(index, stringToPBMEnum(strValue));
		}
		translator->clearParamCollection();
		return subRenderState;								
	}		


	return NULL;
}

//-----------------------------------------------------------------------
void LayeredBlendingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
										Pass* srcPass, Pass* dstPass)
{
	ser->writeAttribute(4, "layered_blend");
	//ser->writeValue("layered_blend");
}

//-----------------------------------------------------------------------
SubRenderState*	LayeredBlendingFactory::createInstanceImpl()
{
	return OGRE_NEW LayeredBlending;
}

//-----------------------------------------------------------------------
LayeredBlending::PLayeredBlendingMode LayeredBlendingFactory::stringToPBMEnum(const String &strValue)
{
	LayeredBlending::PLayeredBlendingMode mode;

	if (strValue == "blend_default")
	{
		mode = LayeredBlending::LB_FFPBlend;
	}
	else if (strValue == "blend_normal")
	{
		mode = LayeredBlending::LB_BlendNormal;
	}
	else if (strValue == "blend_lighten")
	{
		mode = LayeredBlending::LB_BlendLighten;
	}
	else if (strValue == "blend_darken")
	{
		mode = LayeredBlending::LB_BlendDarken;
	}
	else if (strValue == "blend_multiply")
	{
		mode = LayeredBlending::LB_BlendMultiply;
	}
	else if (strValue == "blend_average")
	{
		mode = LayeredBlending::LB_BlendAverage;
	}
	else if (strValue == "blend_add")
	{
		mode = LayeredBlending::LB_BlendAdd;
	}
	else if (strValue == "blend_subtract")
	{
		mode = LayeredBlending::LB_BlendSubtract;
	}
	else if (strValue == "blend_difference")
	{
		mode = LayeredBlending::LB_BlendDifference;
	}				
	else if (strValue == "blend_negation")
	{
		mode = LayeredBlending::LB_BlendNegation;
	}				
	else if (strValue == "blend_exclusion")
	{
		mode = LayeredBlending::LB_BlendExclusion;
	}
	else if (strValue == "blend_screen")
	{
		mode = LayeredBlending::LB_BlendScreen;
	}
	else if (strValue == "blend_overlay")
	{
		mode = LayeredBlending::LB_BlendOverlay;
	}
	else if (strValue == "blend_hardLight")
	{
		mode = LayeredBlending::LB_BlendHardLight;
	}
	else if (strValue == "blend_softLight")
	{
		mode = LayeredBlending::LB_BlendSoftLight;
	}
	else if (strValue == "blend_colorDodge")
	{
		mode = LayeredBlending::LB_BlendColorDodge;
	}
	else if (strValue == "blend_colorBurn")
	{
		mode = LayeredBlending::LB_BlendColorBurn;
	}
	else if (strValue == "blend_linearDodge")
	{
		mode = LayeredBlending::LB_BlendLinearDodge;
	}
	else if (strValue == "blend_linearBurn")
	{
		mode = LayeredBlending::LB_BlendLinearBurn;
	}
	else if (strValue == "blend_linearLight")
	{
		mode = LayeredBlending::LB_BlendLinearLight;
	}
	else if (strValue == "blend_vividLight")
	{
		mode = LayeredBlending::LB_BlendVividLight;
	}
	else if (strValue == "blend_pinLight")
	{
		mode = LayeredBlending::LB_BlendPinLight;
	}
	else if (strValue == "blend_hardMix")
	{
		mode = LayeredBlending::LB_BlendHardMix;
	}
	else if (strValue == "blend_reflect")
	{
		mode = LayeredBlending::LB_BlendReflect;
	}
	else if (strValue == "blend_glow")
	{
		mode = LayeredBlending::LB_BlendGlow;
	}
	else if (strValue == "blend_phoenix")
	{
		mode = LayeredBlending::LB_BlendPhoenix;
	}
	else if (strValue == "blend_saturation")
	{
		mode = LayeredBlending::LB_BlendSaturation;
	}
	else if (strValue == "blend_color")
	{
		mode = LayeredBlending::LB_BlendColor;
	}
	else if (strValue == "blend_luminosity")
	{
		mode = LayeredBlending::LB_BlendLuminosity;
	}
	return mode;
}

#endif
