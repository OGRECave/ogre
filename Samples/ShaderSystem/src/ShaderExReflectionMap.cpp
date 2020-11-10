/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "ShaderExReflectionMap.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreScriptCompiler.h"



/************************************************************************/
/*                                                                      */
/************************************************************************/
String ShaderExReflectionMap::Type                          = "SGX_ReflectionMap";

//-----------------------------------------------------------------------
#define SGX_LIB_REFLECTIONMAP                   "SampleLib_ReflectionMap"
#define SGX_FUNC_APPLY_REFLECTION_MAP           "SGX_ApplyReflectionMap"

//-----------------------------------------------------------------------
ShaderExReflectionMap::ShaderExReflectionMap()
{
    mMaskMapSamplerIndex                = 0;            
    mReflectionMapSamplerIndex          = 0;
    mReflectionMapType                  = TEX_TYPE_2D;
    mReflectionPowerChanged             = true;
    mReflectionPowerValue               = 0.5;
}

//-----------------------------------------------------------------------
const String& ShaderExReflectionMap::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int ShaderExReflectionMap::getExecutionOrder() const
{
    // We place this effect after texturing stage and before fog stage.
    return FFP_TEXTURING + 1;
}

//-----------------------------------------------------------------------
void ShaderExReflectionMap::copyFrom(const SubRenderState& rhs)
{
    const ShaderExReflectionMap& rhsReflectionMap = static_cast<const ShaderExReflectionMap&>(rhs);
    
    // Copy all settings that affect this sub render state output code.
    mMaskMapSamplerIndex = rhsReflectionMap.mMaskMapSamplerIndex;
    mReflectionMapSamplerIndex = rhsReflectionMap.mReflectionMapSamplerIndex;
    mReflectionMapType = rhsReflectionMap.mReflectionMapType;
    mReflectionPowerChanged = rhsReflectionMap.mReflectionPowerChanged;
    mReflectionPowerValue = rhsReflectionMap.mReflectionPowerValue;
    mReflectionMapTextureName = rhsReflectionMap.mReflectionMapTextureName;
    mMaskMapTextureName = rhsReflectionMap.mMaskMapTextureName;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass )
{
    TextureUnitState* textureUnit;
    
    // Create the mask texture unit.
    textureUnit = dstPass->createTextureUnitState();
    textureUnit->setTextureName(mMaskMapTextureName);       
    mMaskMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    // Create the reflection texture unit.
    textureUnit = dstPass->createTextureUnitState();

    textureUnit->setTextureName(mReflectionMapTextureName, mReflectionMapType);
    mReflectionMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    // Resolve vs input mask texture coordinates.
    // NOTE: We use the first texture coordinate hard coded here
    // You may want to parametrize this as well - just remember to add it to hash and copy methods. 
    mVSInMaskTexcoord = vsMain->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
    // Resolve vs output mask texture coordinates.
    mVSOutMaskTexcoord = vsMain->resolveOutputParameter(mVSInMaskTexcoord->getContent(), GCT_FLOAT2);
    // Resolve ps input mask texture coordinates.
    mPSInMaskTexcoord = psMain->resolveInputParameter(mVSOutMaskTexcoord);

    // Resolve vs output reflection texture coordinates.
    mVSOutReflectionTexcoord = vsMain->resolveOutputParameter(
        Parameter::SPC_UNKNOWN, mReflectionMapType == TEX_TYPE_2D ? GCT_FLOAT2 : GCT_FLOAT3);

    // Resolve ps input reflection texture coordinates.
    mPSInReflectionTexcoord= psMain->resolveInputParameter(mVSOutReflectionTexcoord);


    // Resolve world matrix.    
    mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
    // Resolve world inverse transpose matrix.  
    mWorldITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
    // Resolve view matrix.
    mViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_VIEW_MATRIX);
    // Resolve vertex position.
    mVSInputPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    // Resolve vertex normal.
    mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
    // Resolve mask texture sampler parameter.      
    mMaskMapSampler = psProgram->resolveParameter(GCT_SAMPLER2D, "mask_sampler", mMaskMapSamplerIndex);

    // Resolve reflection texture sampler parameter.        
    mReflectionMapSampler = psProgram->resolveParameter(mReflectionMapType == TEX_TYPE_2D ? GCT_SAMPLER2D : GCT_SAMPLERCUBE, 
        "reflection_texture", mReflectionMapSamplerIndex);

    // Resolve reflection power parameter.      
    mReflectionPower = psProgram->resolveParameter(GCT_FLOAT1, "reflection_power");
    // Resolve ps output diffuse colour.
    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

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
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();  
    

    // Add vertex shader invocations.
    if (false == addVSInvocations(vsMain->getStage(FFP_VS_TEXTURING + 1)))
        return false;


    // Add pixel shader invocations.
    if (false == addPSInvocations(psMain->getStage(FFP_PS_TEXTURING + 1)))
        return false;
    
    return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::addVSInvocations( const FunctionStageRef& stage )
{
    // Output mask texture coordinates.
    stage.assign(mVSInMaskTexcoord, mVSOutMaskTexcoord);


    // Output reflection texture coordinates.
    if (mReflectionMapType == TEX_TYPE_2D)
    {
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,
                           {In(mWorldITMatrix), In(mViewMatrix), In(mVSInputNormal), Out(mVSOutReflectionTexcoord)});
    }
    else
    {
        stage.callFunction(
            FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,
            {In(mWorldMatrix), In(mWorldITMatrix), In(mViewMatrix), In(mVSInputNormal), In(mVSInputPos), Out(mVSOutReflectionTexcoord)});
    }
    


    return true;
}

//-----------------------------------------------------------------------
bool ShaderExReflectionMap::addPSInvocations( const FunctionStageRef& stage )
{
    stage.callFunction(SGX_FUNC_APPLY_REFLECTION_MAP,
                       {In(mMaskMapSampler), In(mPSInMaskTexcoord), In(mReflectionMapSampler),
                        In(mPSInReflectionTexcoord), In(mPSOutDiffuse).xyz(), In(mReflectionPower),
                        Out(mPSOutDiffuse).xyz()});
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
void ShaderExReflectionMap::setReflectionPower(const Real reflectionPower)
{
    mReflectionPowerValue = reflectionPower;
    mReflectionPowerChanged = true;
}

//-----------------------------------------------------------------------
void ShaderExReflectionMap::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList)
{
    if (mReflectionPowerChanged)
    {
        GpuProgramParametersSharedPtr fsParams = pass->getFragmentProgramParameters();

        mReflectionPower->setGpuParameter(mReflectionPowerValue);

        mReflectionPowerChanged = false;
    }   
}

//-----------------------------------------------------------------------
SubRenderState* ShaderExReflectionMapFactory::createInstance(ScriptCompiler* compiler, 
                                                         PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
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
            reflectionMapSubRenderState->setMaskMapTextureName(strValue);
            ++it;
            
        
            // Read reflection texture.
            if (false == SGScriptTranslator::getString(*it, &strValue))
            {
                compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                return NULL;
            }           
            reflectionMapSubRenderState->setReflectionMapTextureName(strValue);
            ++it;

            // Read reflection power value.
            Real reflectionPower = 0.5;
            if (false == SGScriptTranslator::getReal(*it, &reflectionPower))
            {
                compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                return NULL;
            }           
            reflectionMapSubRenderState->setReflectionPower(reflectionPower);
                
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

    ser->writeValue(reflectionMapSubRenderState->getMaskMapTextureName());
    ser->writeValue(reflectionMapSubRenderState->getReflectionMapTextureName());
    ser->writeValue(StringConverter::toString(reflectionMapSubRenderState->getReflectionPower()));
}

//-----------------------------------------------------------------------
const String& ShaderExReflectionMapFactory::getType() const
{
    return ShaderExReflectionMap::Type;
}


//-----------------------------------------------------------------------
SubRenderState* ShaderExReflectionMapFactory::createInstanceImpl()
{
    return OGRE_NEW ShaderExReflectionMap;
}
