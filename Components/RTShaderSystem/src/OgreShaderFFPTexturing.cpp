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
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String FFPTexturing::Type = "FFP_Texturing";
#define _INT_VALUE(f) (*(int*)(&(f)))

const String c_ParamTexelEx("texel_");

//-----------------------------------------------------------------------
FFPTexturing::FFPTexturing() : mIsPointSprite(false)
{   
}

//-----------------------------------------------------------------------
const String& FFPTexturing::getType() const
{
    return Type;
}

//-----------------------------------------------------------------------
int FFPTexturing::getExecutionOrder() const
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
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    bool hasError = false;
    
    // Resolve texture sampler parameter.       
    textureUnitParams->mTextureSampler = psProgram->resolveParameter(textureUnitParams->mTextureSamplerType, textureUnitParams->mTextureSamplerIndex, (uint16)GPV_GLOBAL, "gTextureSampler");
    hasError |= !textureUnitParams->mTextureSampler;

    // Resolve texture matrix parameter.
    if (needsTextureMatrix(textureUnitParams->mTextureUnitState))
    {               
        textureUnitParams->mTextureMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_TEXTURE_MATRIX, textureUnitParams->mTextureSamplerIndex);
        hasError |= !(textureUnitParams->mTextureMatrix.get());
    }

    switch (textureUnitParams->mTexCoordCalcMethod)
    {
    case TEXCALC_NONE:                              
        break;

    // Resolve World + View matrices.
    case TEXCALC_ENVIRONMENT_MAP:
    case TEXCALC_ENVIRONMENT_MAP_PLANAR:    
    case TEXCALC_ENVIRONMENT_MAP_NORMAL:
        //TODO: change the following 'mWorldITMatrix' member to 'mWorldViewITMatrix'
        mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX, 0);
        mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
        mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
        
        hasError |= !(mWorldITMatrix.get())  || !(mViewMatrix.get()) || !(mWorldMatrix.get());
        break;

    case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
        mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
        mWorldITMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 0);
        mViewMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_VIEW_MATRIX, 0);
        
        hasError |= !(mWorldMatrix.get()) || !(mWorldITMatrix.get()) || !(mViewMatrix.get());
        break;

    case TEXCALC_PROJECTIVE_TEXTURE:

        mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
        textureUnitParams->mTextureViewProjImageMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_LIGHTS, "gTexViewProjImageMatrix");
        
        hasError |= !(mWorldMatrix.get()) || !(textureUnitParams->mTextureViewProjImageMatrix.get());
        
        const TextureUnitState::EffectMap&      effectMap = textureUnitParams->mTextureUnitState->getEffects(); 
        TextureUnitState::EffectMap::const_iterator effi;

        for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
        {
            if (effi->second.type == TextureUnitState::ET_PROJECTIVE_TEXTURE)
            {
                textureUnitParams->mTextureProjector = effi->second.frustum;
                break;
            }
        }

        hasError |= !(textureUnitParams->mTextureProjector);
        break;
    }

    
    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "FFPTexturing::resolveUniformParams" );
    }
    return true;
}



//-----------------------------------------------------------------------
bool FFPTexturing::resolveFunctionsParams(TextureUnitParams* textureUnitParams, ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* psMain   = psProgram->getEntryPointFunction();
    Parameter::Content texCoordContent = Parameter::SPC_UNKNOWN;
    bool hasError = false;

    switch (textureUnitParams->mTexCoordCalcMethod)
    {
        case TEXCALC_NONE:                  
            // Resolve explicit vs input texture coordinates.
            
            if(mIsPointSprite)
                break;

            if (textureUnitParams->mTextureMatrix.get() == NULL)
                texCoordContent = Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + textureUnitParams->mTextureUnitState->getTextureCoordSet());

            textureUnitParams->mVSInputTexCoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
                textureUnitParams->mTextureUnitState->getTextureCoordSet(), 
                Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + textureUnitParams->mTextureUnitState->getTextureCoordSet()),
                textureUnitParams->mVSInTextureCoordinateType); 
            hasError |= !(textureUnitParams->mVSInputTexCoord.get());
            break;

        case TEXCALC_ENVIRONMENT_MAP:
        case TEXCALC_ENVIRONMENT_MAP_PLANAR:        
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            // Resolve vertex normal.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
            mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
            hasError |= !(mVSInputNormal.get()) || !(mVSInputPos.get());
            break;  

        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:

            // Resolve vertex normal.
            mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
            // Resolve vertex position.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
            
            hasError |= !(mVSInputNormal.get()) || !(mVSInputPos.get());
            break;

        case TEXCALC_PROJECTIVE_TEXTURE:
            // Resolve vertex position.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
            hasError |= !(mVSInputPos.get());
            break;
    }

    if(mIsPointSprite)
    {
        textureUnitParams->mPSInputTexCoord =
            psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 0,
                                          Parameter::SPC_POINTSPRITE_COORDINATE, GCT_FLOAT2);
    }
    else
    {
        // Resolve vs output texture coordinates.
        textureUnitParams->mVSOutputTexCoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES,
            -1,
            texCoordContent,
            textureUnitParams->mVSOutTextureCoordinateType);

        // Resolve ps input texture coordinates.
        textureUnitParams->mPSInputTexCoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES,
            textureUnitParams->mVSOutputTexCoord->getIndex(),
            textureUnitParams->mVSOutputTexCoord->getContent(),
            textureUnitParams->mVSOutTextureCoordinateType);
    }

    const ShaderParameterList& inputParams = psMain->getInputParameters();
    const ShaderParameterList& localParams = psMain->getLocalParameters();

    mPSDiffuse = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    if (mPSDiffuse.get() == NULL)
    {
        mPSDiffuse = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    }

    mPSSpecular = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
    if (mPSSpecular.get() == NULL)
    {
        mPSSpecular = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
    }

    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);

    hasError |= (!textureUnitParams->mVSOutputTexCoord && !mIsPointSprite) ||
                !textureUnitParams->mPSInputTexCoord || !mPSDiffuse || !mPSSpecular ||
                !mPSOutDiffuse;

    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "FFPTexturing::resolveFunctionsParams" );
    }
    return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::resolveDependencies(ProgramSet* programSet)
{
    //! [deps_resolve]
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(FFP_LIB_TEXTURING);    
    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(FFP_LIB_TEXTURING);
    //! [deps_resolve]
    return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* psMain   = psProgram->getEntryPointFunction();

    for (unsigned int i=0; i < mTextureUnitParamsList.size(); ++i)
    {
        TextureUnitParams* curParams = &mTextureUnitParamsList[i];

        if (false == addVSFunctionInvocations(curParams, vsMain))
            return false;

        if (false == addPSFunctionInvocations(curParams, psMain))
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------
bool FFPTexturing::addVSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* vsMain)
{
    if(mIsPointSprite)
        return true;

    FunctionInvocation* texCoordCalcFunc = NULL;

    
    switch (textureUnitParams->mTexCoordCalcMethod)
    {
    case TEXCALC_NONE:
        texCoordCalcFunc =
            OGRE_NEW AssignmentAtom(textureUnitParams->mVSOutputTexCoord,
                                    textureUnitParams->mVSInputTexCoord, FFP_VS_TEXTURING);                    
        break;

    case TEXCALC_ENVIRONMENT_MAP:
    case TEXCALC_ENVIRONMENT_MAP_PLANAR:
        texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,  FFP_VS_TEXTURING);

        //TODO: Add field member mWorldViewITMatrix 
        texCoordCalcFunc->pushOperand(mWorldMatrix, Operand::OPS_IN);   
        texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);    
        texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(mVSInputPos, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);         
        break;

            
    case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
        texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,  FFP_VS_TEXTURING);

        texCoordCalcFunc->pushOperand(mWorldMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);            
        texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN); 
        texCoordCalcFunc->pushOperand(mVSInputPos, Operand::OPS_IN);                
        texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);          
        break;

    case TEXCALC_ENVIRONMENT_MAP_NORMAL:
        texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,  FFP_VS_TEXTURING);

        texCoordCalcFunc->pushOperand(mWorldITMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(mViewMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(mVSInputNormal, Operand::OPS_IN); 
        texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);          
        break;

    case TEXCALC_PROJECTIVE_TEXTURE:

        texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_GENERATE_TEXCOORD_PROJECTION,  FFP_VS_TEXTURING);

        texCoordCalcFunc->pushOperand(mWorldMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(textureUnitParams->mTextureViewProjImageMatrix, Operand::OPS_IN); 
        texCoordCalcFunc->pushOperand(mVSInputPos, Operand::OPS_IN);        
        texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
        break;
    default:
        return false;
    }

    vsMain->addAtomInstance(texCoordCalcFunc);

    if (textureUnitParams->mTextureMatrix)
    {
        texCoordCalcFunc = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM_TEXCOORD,  FFP_VS_TEXTURING);
        texCoordCalcFunc->pushOperand(textureUnitParams->mTextureMatrix, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_IN);
        texCoordCalcFunc->pushOperand(textureUnitParams->mVSOutputTexCoord, Operand::OPS_OUT);
        vsMain->addAtomInstance(texCoordCalcFunc);
    }

    return true;
}
//-----------------------------------------------------------------------
bool FFPTexturing::addPSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* psMain)
{
    const LayerBlendModeEx& colourBlend = textureUnitParams->mTextureUnitState->getColourBlendMode();
    const LayerBlendModeEx& alphaBlend  = textureUnitParams->mTextureUnitState->getAlphaBlendMode();
    ParameterPtr source1;
    ParameterPtr source2;
    int groupOrder = FFP_PS_TEXTURING;
    
            
    // Add texture sampling code.
    ParameterPtr texel = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, c_ParamTexelEx + StringConverter::toString(textureUnitParams->mTextureSamplerIndex), GCT_FLOAT4);
    addPSSampleTexelInvocation(textureUnitParams, psMain, texel, FFP_PS_SAMPLING);

    // Build colour argument for source1.
    source1 = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "source1", GCT_FLOAT4);
        
    addPSArgumentInvocations(psMain, source1, texel, 
        textureUnitParams->mTextureSamplerIndex,
        colourBlend.source1, colourBlend.colourArg1, 
        colourBlend.alphaArg1, false, groupOrder);

    // Build colour argument for source2.
    source2 = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "source2", GCT_FLOAT4);

    addPSArgumentInvocations(psMain, source2, texel, 
        textureUnitParams->mTextureSamplerIndex,
        colourBlend.source2, colourBlend.colourArg2, 
        colourBlend.alphaArg2, false, groupOrder);

    bool needDifferentAlphaBlend = false;
    if (alphaBlend.operation != colourBlend.operation ||
        alphaBlend.source1 != colourBlend.source1 ||
        alphaBlend.source2 != colourBlend.source2 ||
        colourBlend.source1 == LBS_MANUAL ||
        colourBlend.source2 == LBS_MANUAL ||
        alphaBlend.source1 == LBS_MANUAL ||
        alphaBlend.source2 == LBS_MANUAL)
        needDifferentAlphaBlend = true;

    // Build colours blend
    addPSBlendInvocations(psMain, source1, source2, texel, 
        textureUnitParams->mTextureSamplerIndex,
        colourBlend, groupOrder,
        needDifferentAlphaBlend ? Operand::OPM_XYZ : Operand::OPM_ALL);

    // Case we need different alpha channel code.
    if (needDifferentAlphaBlend)
    {
        // Build alpha argument for source1.
        addPSArgumentInvocations(psMain, source1, texel,
            textureUnitParams->mTextureSamplerIndex, 
            alphaBlend.source1, alphaBlend.colourArg1, 
            alphaBlend.alphaArg1, true, groupOrder);

        // Build alpha argument for source2.
        addPSArgumentInvocations(psMain, source2, texel, 
            textureUnitParams->mTextureSamplerIndex,
            alphaBlend.source2, alphaBlend.colourArg2, 
            alphaBlend.alphaArg2, true, groupOrder);

        // Build alpha blend
        addPSBlendInvocations(psMain, source1, source2, texel,
                              textureUnitParams->mTextureSamplerIndex, alphaBlend, groupOrder,
                              Operand::OPM_W);
    }
    
    

    return true;
}

//-----------------------------------------------------------------------
void FFPTexturing::addPSSampleTexelInvocation(TextureUnitParams* textureUnitParams, Function* psMain, 
                                              const ParameterPtr& texel, int groupOrder)
{
    FunctionInvocation* curFuncInvocation = NULL;

    if (textureUnitParams->mTexCoordCalcMethod == TEXCALC_PROJECTIVE_TEXTURE)
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE_PROJ, groupOrder);
    else
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SAMPLE_TEXTURE, groupOrder);

    curFuncInvocation->pushOperand(textureUnitParams->mTextureSampler, Operand::OPS_IN);
    curFuncInvocation->pushOperand(textureUnitParams->mPSInputTexCoord, Operand::OPS_IN);
    curFuncInvocation->pushOperand(texel, Operand::OPS_OUT);

    psMain->addAtomInstance(curFuncInvocation);
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
                                             const int groupOrder)
{
    FunctionInvocation* curFuncInvocation = NULL;

    switch(blendSrc)
    {
    case LBS_CURRENT:
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        if (samplerIndex == 0)
            curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);
        else
            curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN);
        curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);      
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBS_TEXTURE:       
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(texel, Operand::OPS_IN);
        curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);      
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBS_DIFFUSE:       
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);        
        curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);      
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBS_SPECULAR:      
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(mPSSpecular, Operand::OPS_IN);       
        curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);      
        psMain->addAtomInstance(curFuncInvocation); 
        break;

    case LBS_MANUAL:
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);

        if (isAlphaArgument)
        {
            curFuncInvocation->pushOperand(ParameterFactory::createConstParam(Vector4(alphaValue)), Operand::OPS_IN);
        }
        else
        {
            curFuncInvocation->pushOperand(ParameterFactory::createConstParam(Vector4((Real)colourValue.r,
                                                                                      (Real)colourValue.g,
                                                                                      (Real)colourValue.b,
                                                                                      (Real)colourValue.a)),
                                           Operand::OPS_IN);
        }
        
        curFuncInvocation->pushOperand(arg, Operand::OPS_OUT);
        psMain->addAtomInstance(curFuncInvocation); 
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
                                          int targetChannels)
{
    FunctionInvocation* curFuncInvocation = NULL;

    switch(blendMode.operation)
    {
    case LBX_SOURCE1:
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);                     
        break;
    case LBX_SOURCE2:
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);                         
        break;
    case LBX_MODULATE:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);         
        break;
    case LBX_MODULATE_X2:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATEX2, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);         
        break;
    case LBX_MODULATE_X4:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATEX4, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation); 
        break;
    case LBX_ADD:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADD, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);         
        break;
    case LBX_ADD_SIGNED:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADDSIGNED, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);             
        break;
    case LBX_ADD_SMOOTH:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ADDSMOOTH, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);         
        break;
    case LBX_SUBTRACT:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation); 
        break;
    case LBX_BLEND_DIFFUSE_ALPHA:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_W);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBX_BLEND_TEXTURE_ALPHA:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(texel, Operand::OPS_IN, Operand::OPM_W);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBX_BLEND_CURRENT_ALPHA:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);

        if (samplerIndex == 0)
            curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_W);
        else
            curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_IN, Operand::OPM_W);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBX_BLEND_MANUAL:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(ParameterFactory::createConstParam(blendMode.factor), Operand::OPS_IN);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);
        break;
    case LBX_DOTPRODUCT:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_DOTPRODUCT, groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);      
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    case LBX_BLEND_DIFFUSE_COLOUR:
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_LERP, groupOrder);
        curFuncInvocation->pushOperand(arg2, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(arg1, Operand::OPS_IN, targetChannels);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT, targetChannels);        
        psMain->addAtomInstance(curFuncInvocation);     
        break;
    }
}

//-----------------------------------------------------------------------
TexCoordCalcMethod FFPTexturing::getTexCalcMethod(TextureUnitState* textureUnitState)
{
    TexCoordCalcMethod                      texCoordCalcMethod = TEXCALC_NONE;  
    const TextureUnitState::EffectMap&      effectMap = textureUnitState->getEffects(); 
    TextureUnitState::EffectMap::const_iterator effi;
    
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
    const TextureUnitState::EffectMap&      effectMap = textureUnitState->getEffects(); 
    TextureUnitState::EffectMap::const_iterator effi;

    for (effi = effectMap.begin(); effi != effectMap.end(); ++effi)
    {
        switch (effi->second.type)
        {
    
        case TextureUnitState::ET_UVSCROLL:
        case TextureUnitState::ET_USCROLL:
        case TextureUnitState::ET_VSCROLL:
        case TextureUnitState::ET_ROTATE:
        case TextureUnitState::ET_TRANSFORM:
        case TextureUnitState::ET_ENVIRONMENT_MAP:
        case TextureUnitState::ET_PROJECTIVE_TEXTURE:
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
bool FFPTexturing::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    mIsPointSprite = srcPass->getPointSpritesEnabled();

    //count the number of texture units we need to process
    size_t validTexUnits = 0;
    for (unsigned short i=0; i < srcPass->getNumTextureUnitStates(); ++i)
    {       
        if (isProcessingNeeded(srcPass->getTextureUnitState(i)))
        {
            ++validTexUnits;
        }
    }

    setTextureUnitCount(validTexUnits);

    // Build texture stage sub states.
    for (unsigned short i=0; i < srcPass->getNumTextureUnitStates(); ++i)
    {       
        TextureUnitState* texUnitState = srcPass->getTextureUnitState(i);                               

        if (isProcessingNeeded(texUnitState))
        {
            setTextureUnit(i, texUnitState);    
        }
    }   

    return true;
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
            Matrix4 matTexViewProjImage;

            matTexViewProjImage = 
                Matrix4::CLIPSPACE2DTOIMAGESPACE * 
                curParams->mTextureProjector->getProjectionMatrixWithRSDepth() * 
                curParams->mTextureProjector->getViewMatrix();

            curParams->mTextureViewProjImageMatrix->setGpuParameter(matTexViewProjImage);
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

        curParams.mTextureUnitState             = NULL;         
        curParams.mTextureProjector             = NULL;               
        curParams.mTextureSamplerIndex          = 0;              
        curParams.mTextureSamplerType           = GCT_SAMPLER2D;        
        curParams.mVSInTextureCoordinateType    = GCT_FLOAT2;   
        curParams.mVSOutTextureCoordinateType   = GCT_FLOAT2;       
    }
}

//-----------------------------------------------------------------------
void FFPTexturing::setTextureUnit(unsigned short index, TextureUnitState* textureUnitState)
{
    if (index >= mTextureUnitParamsList.size())
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "FFPTexturing unit index out of bounds !!!",
            "FFPTexturing::setTextureUnit");
    }

    if (textureUnitState->getBindingType() == TextureUnitState::BT_VERTEX)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "FFP Texture unit does not support vertex texture fetch !!!",
            "FFPTexturing::setTextureUnit");
    }
    
    if (textureUnitState->getBindingType() == TextureUnitState::BT_GEOMETRY)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "FFP Texture unit does not support geometry texture fetch !!!",
            "FFPTexturing::setTextureUnit");
    }

    if (textureUnitState->getBindingType() == TextureUnitState::BT_COMPUTE)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "FFP Texture unit does not support comput texture fetch !!!",
            "FFPTexturing::setTextureUnit");
    }

    if (textureUnitState->getBindingType() == TextureUnitState::BT_TESSELLATION_DOMAIN)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "FFP Texture unit does not support domain texture fetch !!!",
            "FFPTexturing::setTextureUnit");
    }

    if (textureUnitState->getBindingType() == TextureUnitState::BT_TESSELLATION_HULL)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "FFP Texture unit does not support hull texture fetch !!!",
            "FFPTexturing::setTextureUnit");
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
    case TEX_TYPE_2D_RECT:
        curParams.mTextureSamplerType = GCT_SAMPLERRECT;
        curParams.mVSInTextureCoordinateType = GCT_FLOAT2;
        break;
    case TEX_TYPE_2D_ARRAY:
        curParams.mTextureSamplerType = GCT_SAMPLER2DARRAY;
        curParams.mVSInTextureCoordinateType = GCT_FLOAT3;
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
bool FFPTexturing::isProcessingNeeded(TextureUnitState* texUnitState)
{
    return texUnitState->getBindingType() == TextureUnitState::BT_FRAGMENT;
}


//-----------------------------------------------------------------------
const String& FFPTexturingFactory::getType() const
{
    return FFPTexturing::Type;
}

//-----------------------------------------------------------------------
SubRenderState* FFPTexturingFactory::createInstance(ScriptCompiler* compiler, 
                                                 PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
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
                return createOrRetrieveInstance(translator);
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
SubRenderState* FFPTexturingFactory::createInstanceImpl()
{
    return OGRE_NEW FFPTexturing;
}


}
}

#endif
