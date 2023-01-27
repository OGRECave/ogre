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
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

namespace Ogre {
namespace RTShader {


String LayeredBlending::Type = "LayeredBlendRTSSEx";
const String SRS_LAYERED_BLENDING = "LayeredBlendRTSSEx";

enum BlendMode : int
{
    LB_Invalid = -1,
    LB_FFPBlend,
    LB_BlendNormal,
    LB_BlendLighten,
    LB_BlendDarken,
    LB_BlendMultiply,
    LB_BlendAverage,
    LB_BlendAdd,
    LB_BlendSubtract,
    LB_BlendDifference,
    LB_BlendNegation,
    LB_BlendExclusion,
    LB_BlendScreen,
    LB_BlendOverlay,
    LB_BlendSoftLight,
    LB_BlendHardLight,
    LB_BlendColorDodge,
    LB_BlendColorBurn,
    LB_BlendLinearDodge,
    LB_BlendLinearBurn,
    LB_BlendLinearLight,
    LB_BlendVividLight,
    LB_BlendPinLight,
    LB_BlendHardMix,
    LB_BlendReflect,
    LB_BlendGlow,
    LB_BlendPhoenix,
    LB_BlendSaturation,
    LB_BlendColor,
    LB_BlendLuminosity,
    LB_MaxBlendModes
};

enum SourceModifier : int
{
    SM_Invalid = -1,
    SM_None,
    SM_Source1Modulate,
    SM_Source2Modulate,
    SM_Source1InvModulate,
    SM_Source2InvModulate,
    SM_MaxSourceModifiers
};

namespace {

 struct BlendModeDescription {
        /* Type of the blend mode */
        BlendMode type;
        /* name of the blend mode. */
        const char* name;
        /* shader function name . */
        const char* funcName;
    };

const BlendModeDescription _blendModes[(int)LB_MaxBlendModes] = {
    { LB_FFPBlend ,"default", ""},
    { LB_BlendNormal ,"normal", "SGX_blend_normal"},
    { LB_BlendLighten,"lighten", "SGX_blend_lighten"},
    { LB_BlendDarken ,"darken", "SGX_blend_darken"},
    { LB_BlendMultiply ,"multiply", "SGX_blend_multiply"},
    { LB_BlendAverage ,"average", "SGX_blend_average"},
    { LB_BlendAdd ,"add", "SGX_blend_add"},
    { LB_BlendSubtract ,"subtract", "SGX_blend_subtract"},
    { LB_BlendDifference ,"difference", "SGX_blend_difference"},
    { LB_BlendNegation ,"negation", "SGX_blend_negation"},
    { LB_BlendExclusion ,"exclusion", "SGX_blend_exclusion"},
    { LB_BlendScreen ,"screen", "SGX_blend_screen"},
    { LB_BlendOverlay ,"overlay", "SGX_blend_overlay"},
    { LB_BlendHardLight ,"hard_light", "SGX_blend_hardLight"},
    { LB_BlendSoftLight ,"soft_light", "SGX_blend_softLight"},
    { LB_BlendColorDodge ,"color_dodge", "SGX_blend_colorDodge"},
    { LB_BlendColorBurn ,"color_burn", "SGX_blend_colorBurn"},
    { LB_BlendLinearDodge ,"linear_dodge", "SGX_blend_linearDodge"},
    { LB_BlendLinearBurn ,"linear_burn", "SGX_blend_linearBurn"},
    { LB_BlendLinearLight ,"linear_light", "SGX_blend_linearLight"},
    { LB_BlendVividLight ,"vivid_light", "SGX_blend_vividLight"},
    { LB_BlendPinLight ,"pin_light", "SGX_blend_pinLight"},
    { LB_BlendHardMix ,"hard_mix", "SGX_blend_hardMix"},
    { LB_BlendReflect ,"reflect", "SGX_blend_reflect"},
    { LB_BlendGlow ,"glow", "SGX_blend_glow"},
    { LB_BlendPhoenix ,"phoenix", "SGX_blend_phoenix"},
    { LB_BlendSaturation ,"saturation", "SGX_blend_saturation"},
    { LB_BlendColor ,"color", "SGX_blend_color"},
    { LB_BlendLuminosity, "luminosity", "SGX_blend_luminosity"}
};
        

 struct SourceModifierDescription {
        /* Type of the source modifier*/
        SourceModifier type;
        /* name of the source modifier. */
        const char* name;
    };

const SourceModifierDescription _sourceModifiers[(int)SM_MaxSourceModifiers] = {
    { SM_None ,""},
    { SM_Source1Modulate ,"src1_modulate"},
    { SM_Source2Modulate ,"src2_modulate"},
    { SM_Source1InvModulate ,"src1_inverse_modulate"},
    { SM_Source2InvModulate ,"src2_inverse_modulate"}
    };

//-----------------------------------------------------------------------
BlendMode stringToBlendMode(const String &strValue)
{
    for(const auto & _blendMode : _blendModes)
    {
        if (_blendMode.name == strValue)
        {
            return _blendMode.type;
        }
    }
    return LB_Invalid;
}

//-----------------------------------------------------------------------
String blendModeToString(BlendMode blendMode)
{
    for(const auto & _blendMode : _blendModes)
    {
        if (_blendMode.type == blendMode)
        {
            return _blendMode.name;
        }
    }
    return "";
}

//-----------------------------------------------------------------------
SourceModifier stringToSourceModifier(const String &strValue)
{
    for(const auto & _sourceModifier : _sourceModifiers)
    {
        if (_sourceModifier.name == strValue)
        {
            return _sourceModifier.type;
        }
    }
    return SM_Invalid;
}

//-----------------------------------------------------------------------
String sourceModifierToString(SourceModifier modifier)
{
    for(const auto & _sourceModifier : _sourceModifiers)
    {
        if (_sourceModifier.type == modifier)
        {
            return _sourceModifier.name;
        }
    }
    return "";
}
}

LayeredBlending::TextureBlend::TextureBlend() : blendMode(LB_Invalid), sourceModifier(SM_Invalid), customNum(0) {}

//-----------------------------------------------------------------------
LayeredBlending::LayeredBlending()
{

}

//-----------------------------------------------------------------------
const Ogre::String& LayeredBlending::getType() const
{
    return SRS_LAYERED_BLENDING;
}


//-----------------------------------------------------------------------
bool LayeredBlending::resolveParameters(ProgramSet* programSet)
{

    //resolve peremeter for normal texturing procedures
    bool isSuccess = FFPTexturing::resolveParameters(programSet);
    
    if (isSuccess)
    {
        //resolve source modification parameters
        Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

        for(size_t i = mTextureBlends.size() - 1; i != (size_t)-1 ; --i)
        {
            TextureBlend& texBlend = mTextureBlends[i];
            if ((texBlend.sourceModifier != SM_Invalid) && 
                (texBlend.sourceModifier != SM_None))
            {
                
                texBlend.modControlParam = psProgram->resolveParameter(
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
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    psProgram->addDependency("SGXLib_LayeredBlending");

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
                                         Operand::OpMask mask)
{
    //
    // Add the modifier invocation
    //

    addPSModifierInvocation(psMain, samplerIndex, arg1, arg2, groupOrder, mask);

    //
    // Add the blending function invocations
    //

    BlendMode mode = getBlendMode(samplerIndex);
    
    if ((LB_FFPBlend == mode) || (LB_Invalid == mode))
    {
        FFPTexturing::addPSBlendInvocations(psMain, arg1, arg2, texel, samplerIndex, blendMode, groupOrder, mask);
    }
    else 
    {
        //find the function name for the blend mode
        const char* funcName = NULL;
        for(const auto & _blendMode : _blendModes)
        {
            if (_blendMode.type == mode)
            {
                funcName = _blendMode.funcName;
                break;
            }
        }

        //add the function of the blend mode
        if (funcName)
        {
            psMain->getStage(groupOrder)
                .callFunction(funcName, In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        }
    }
}

//-----------------------------------------------------------------------
void LayeredBlending::addPSModifierInvocation(Function* psMain, 
                                         int samplerIndex, 
                                         ParameterPtr arg1,
                                         ParameterPtr arg2,
                                         const int groupOrder, 
                                         Operand::OpMask mask)
{
    SourceModifier modType;
    int customNum;
    if (getSourceModifier(samplerIndex, modType, customNum) == true)
    {
        ParameterPtr modifiedParam;
        const char* funcName = NULL;
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
        if (funcName)
        {
            ParameterPtr& controlParam = mTextureBlends[samplerIndex].modControlParam;
            psMain->getStage(groupOrder)
                .callFunction(funcName, In(modifiedParam).mask(mask), In(controlParam).mask(mask),
                              Out(modifiedParam).mask(mask));
        }
    }
}

bool LayeredBlending::setBlendMode(uint16 index, const String& mode)
{
    auto blendMode = stringToBlendMode(mode);
    if (blendMode == LB_Invalid)
        return false;

    if(mTextureBlends.size() < (size_t)index + 1)
    {
        mTextureBlends.resize(index + 1);
    }
    mTextureBlends[index].blendMode = blendMode;
    return true;
}

//-----------------------------------------------------------------------
BlendMode LayeredBlending::getBlendMode(unsigned short index) const
{
    if(index < mTextureBlends.size())
    {
        return mTextureBlends[index].blendMode;
    }
    return LB_Invalid;
}


//-----------------------------------------------------------------------
bool LayeredBlending::setSourceModifier(unsigned short index, const String& modType, int customNum)
{
    SourceModifier mod = stringToSourceModifier(modType);
    if (mod == SM_Invalid)
        return false;

    if(mTextureBlends.size() < (size_t)index + 1)
    {
        mTextureBlends.resize(index + 1);
    }
    mTextureBlends[index].sourceModifier = mod;
    mTextureBlends[index].customNum = customNum;
    return true;
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
    return SRS_LAYERED_BLENDING;
}

//-----------------------------------------------------------------------
SubRenderState* LayeredBlendingFactory::createInstance(ScriptCompiler* compiler, 
                                    PropertyAbstractNode* prop, TextureUnitState* texState, SGScriptTranslator* translator)
{
    if (prop->name == "layered_blend")
    {
        if (stringToBlendMode(prop->values.front()->getString()) == LB_Invalid)
        {
            StringVector vec;
            for (const auto& m : _blendModes)
                vec.push_back(m.name);

            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                "Expected one of the following blend modes: " + StringConverter::toString(vec));
            return NULL;
        }

        
        //get the layer blend sub-render state to work on
        LayeredBlending* layeredBlendState =
            createOrRetrieveSubRenderState(translator);
        
        //update the layer sub render state
        unsigned short texIndex = texState->getParent()->getTextureUnitStateIndex(texState);
        layeredBlendState->setBlendMode(texIndex, prop->values.front()->getString());

        return layeredBlendState;
    }
    if (prop->name == "source_modifier")
    {
        if(prop->values.size() < 3)
            return NULL;

        // Read light model type.
        bool isParseSuccess;
        String modifierString;
        String paramType;
        int customNum;
        
        AbstractNodeList::const_iterator itValue = prop->values.begin();
        modifierString = (*itValue)->getString();
        isParseSuccess = stringToSourceModifier(modifierString) != SM_Invalid;
        if (isParseSuccess == false)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
                "Expected one of the following modifier type as first parameter: " \
                "src1_modulate, src2_modulate, src1_inverse_modulate, src2_inverse_modulate.");
            return NULL;
        }

        ++itValue;
        isParseSuccess &= ((*itValue)->getString() == "custom");
        if(isParseSuccess == false)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
                "Expected reserved word custom as second parameter.");
            return NULL;
        }
        ++itValue;
        if(!SGScriptTranslator::getInt(*itValue, &customNum))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
                "Expected number of custom parameter as third parameter.");
            return NULL;
        }

        //get the layer blend sub-render state to work on
        LayeredBlending* layeredBlendState = createOrRetrieveSubRenderState(translator);

        //update the layer sub render state
        unsigned short texIndex = texState->getParent()->getTextureUnitStateIndex(texState);
        layeredBlendState->setSourceModifier(texIndex, modifierString, customNum);

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
    BlendMode blendMode = layeredBlendingSubRenderState->getBlendMode(texIndex);
    if (blendMode != LB_Invalid)
    {
        ser->writeAttribute(5, "layered_blend");    
        ser->writeValue(blendModeToString(blendMode));
    }

    //write the source modifier
    SourceModifier modType;
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
SubRenderState* LayeredBlendingFactory::createInstanceImpl()
{
    return OGRE_NEW LayeredBlending;
}

//-----------------------------------------------------------------------
LayeredBlending* LayeredBlendingFactory::createOrRetrieveSubRenderState(SGScriptTranslator* translator)
{
    LayeredBlending* layeredBlendState;
    //check if we already create a blend srs
    SubRenderState* subState = translator->getGeneratedSubRenderState(getType());
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


