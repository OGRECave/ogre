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
    
    // Resolve texture sampler parameter.       
    textureUnitParams->mTextureSampler = psProgram->resolveParameter(textureUnitParams->mTextureSamplerType, "gTextureSampler", textureUnitParams->mTextureSamplerIndex);

    // Resolve texture matrix parameter.
    if (needsTextureMatrix(textureUnitParams->mTextureUnitState))
    {               
        textureUnitParams->mTextureMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_TEXTURE_MATRIX, textureUnitParams->mTextureSamplerIndex);
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
        mWorldITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX);
        mViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_VIEW_MATRIX);
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
        break;

    case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
        mWorldITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
        mViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_VIEW_MATRIX);
        break;

    case TEXCALC_PROJECTIVE_TEXTURE:
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
        textureUnitParams->mTextureViewProjImageMatrix = vsProgram->resolveParameter(
            GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, textureUnitParams->mTextureSamplerIndex);
        break;
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

    switch (textureUnitParams->mTexCoordCalcMethod)
    {
        case TEXCALC_NONE:                  
            // Resolve explicit vs input texture coordinates.
            
            if(mIsPointSprite)
                break;

            if (textureUnitParams->mTextureMatrix.get() == NULL)
                texCoordContent = Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + textureUnitParams->mTextureUnitState->getTextureCoordSet());

            textureUnitParams->mVSInputTexCoord = vsMain->resolveInputParameter(
                Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 +
                                   textureUnitParams->mTextureUnitState->getTextureCoordSet()),
                textureUnitParams->mVSInTextureCoordinateType);
            break;

        case TEXCALC_ENVIRONMENT_MAP:
        case TEXCALC_ENVIRONMENT_MAP_PLANAR:        
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            // Resolve vertex normal.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
            break;  

        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:

            // Resolve vertex normal.
            mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
            // Resolve vertex position.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            break;

        case TEXCALC_PROJECTIVE_TEXTURE:
            // Resolve vertex position.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            break;
    }

    if(mIsPointSprite)
    {
        textureUnitParams->mPSInputTexCoord = psMain->resolveInputParameter(Parameter::SPC_POINTSPRITE_COORDINATE);
    }
    else
    {
        // Resolve vs output texture coordinates.
        textureUnitParams->mVSOutputTexCoord =
            vsMain->resolveOutputParameter(texCoordContent, textureUnitParams->mVSOutTextureCoordinateType);

        // Resolve ps input texture coordinates.
        textureUnitParams->mPSInputTexCoord = psMain->resolveInputParameter(textureUnitParams->mVSOutputTexCoord);
    }

    mPSDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mPSDiffuse.get() == NULL)
    {
        mPSDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    }

    mPSSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
    if (mPSSpecular.get() == NULL)
    {
        mPSSpecular = psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
    }

    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (!mPSDiffuse || !mPSSpecular)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Not all parameters could be constructed for the sub-render state.");
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
    
    auto stage = vsMain->getStage(FFP_VS_TEXTURING);

    switch (textureUnitParams->mTexCoordCalcMethod)
    {
    case TEXCALC_NONE:
        stage.assign(textureUnitParams->mVSInputTexCoord, textureUnitParams->mVSOutputTexCoord);
        break;
    case TEXCALC_ENVIRONMENT_MAP:
    case TEXCALC_ENVIRONMENT_MAP_PLANAR:
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,
                           {In(mWorldMatrix), In(mViewMatrix), In(mWorldITMatrix), In(mVSInputPos), In(mVSInputNormal),
                            Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,
                           {In(mWorldMatrix), In(mWorldITMatrix), In(mViewMatrix), In(mVSInputNormal), In(mVSInputPos),
                            Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    case TEXCALC_ENVIRONMENT_MAP_NORMAL:
        stage.callFunction(
            FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,
            {In(mWorldITMatrix), In(mViewMatrix), In(mVSInputNormal), Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    case TEXCALC_PROJECTIVE_TEXTURE:
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_PROJECTION,
                           {In(mWorldMatrix), In(textureUnitParams->mTextureViewProjImageMatrix), In(mVSInputPos),
                            Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    default:
        return false;
    }

    if (textureUnitParams->mTextureMatrix)
    {
        stage.callFunction(FFP_FUNC_TRANSFORM_TEXCOORD, textureUnitParams->mTextureMatrix,
                           textureUnitParams->mVSOutputTexCoord, textureUnitParams->mVSOutputTexCoord);
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
    ParameterPtr texel = psMain->resolveLocalParameter(GCT_FLOAT4, c_ParamTexelEx + StringConverter::toString(textureUnitParams->mTextureSamplerIndex));
    addPSSampleTexelInvocation(textureUnitParams, psMain, texel, FFP_PS_SAMPLING);

    // Build colour argument for source1.
    source1 = getPSArgument(texel, colourBlend.source1, colourBlend.colourArg1, colourBlend.alphaArg1, false);

    // Build colour argument for source2.
    source2 = getPSArgument(texel, colourBlend.source2, colourBlend.colourArg2, colourBlend.alphaArg2, false);

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
        source1 = getPSArgument(texel, alphaBlend.source1, alphaBlend.colourArg1, alphaBlend.alphaArg1, true);

        // Build alpha argument for source2.
        source2 = getPSArgument(texel, alphaBlend.source2, alphaBlend.colourArg2, alphaBlend.alphaArg2, true);

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
    auto stage = psMain->getStage(groupOrder);

    if (textureUnitParams->mTexCoordCalcMethod != TEXCALC_PROJECTIVE_TEXTURE)
    {
        stage.sampleTexture(textureUnitParams->mTextureSampler, textureUnitParams->mPSInputTexCoord, texel);
        return;
    }

    stage.callFunction(FFP_FUNC_SAMPLE_TEXTURE_PROJ, textureUnitParams->mTextureSampler,
                       textureUnitParams->mPSInputTexCoord, texel);
}

//-----------------------------------------------------------------------
ParameterPtr FFPTexturing::getPSArgument(ParameterPtr texel, LayerBlendSource blendSrc, const ColourValue& colourValue,
                                         Real alphaValue, bool isAlphaArgument) const
{
    switch(blendSrc)
    {
    case LBS_CURRENT:
        return mPSOutDiffuse;
    case LBS_TEXTURE:
        return texel;
    case LBS_DIFFUSE:
        return mPSDiffuse;
    case LBS_SPECULAR:
        return mPSSpecular;
    case LBS_MANUAL:
        if (isAlphaArgument)
        {
            return ParameterFactory::createConstParam(Vector4(alphaValue));
        }

        return ParameterFactory::createConstParam(Vector4((Real)colourValue.r, (Real)colourValue.g,
                                                         (Real)colourValue.b, (Real)colourValue.a));
    }

    return ParameterPtr();
}

//-----------------------------------------------------------------------
void FFPTexturing::addPSBlendInvocations(Function* psMain, 
                                          ParameterPtr arg1,
                                          ParameterPtr arg2,
                                          ParameterPtr texel,
                                          int samplerIndex,
                                          const LayerBlendModeEx& blendMode,
                                          const int groupOrder, 
                                          Operand::OpMask mask)
{
    auto stage = psMain->getStage(groupOrder);
    switch(blendMode.operation)
    {
    case LBX_SOURCE1:
        stage.assign(In(arg1).mask(mask), Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_SOURCE2:
        stage.assign(In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_MODULATE:
        stage.mul(In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_MODULATE_X2:
        stage.callFunction(FFP_FUNC_MODULATEX2, In(arg1).mask(mask), In(arg2).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_MODULATE_X4:
        stage.callFunction(FFP_FUNC_MODULATEX4, In(arg1).mask(mask), In(arg2).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_ADD:
        stage.add(In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_ADD_SIGNED:
        stage.callFunction(FFP_FUNC_ADDSIGNED, In(arg1).mask(mask), In(arg2).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_ADD_SMOOTH:
        stage.callFunction(FFP_FUNC_ADDSMOOTH, In(arg1).mask(mask), In(arg2).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_SUBTRACT:
        stage.sub(In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_BLEND_DIFFUSE_ALPHA:
        stage.callFunction(FFP_FUNC_LERP, {In(arg2).mask(mask), In(arg1).mask(mask), In(mPSDiffuse).w(),
                                           Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_BLEND_TEXTURE_ALPHA:
        stage.callFunction(FFP_FUNC_LERP, {In(arg2).mask(mask), In(arg1).mask(mask), In(texel).w(),
                                           Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_BLEND_CURRENT_ALPHA:
        stage.callFunction(FFP_FUNC_LERP, {In(arg2).mask(mask), In(arg1).mask(mask),
                                           In(mPSOutDiffuse).w(),
                                           Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_BLEND_MANUAL:
        stage.callFunction(FFP_FUNC_LERP, {In(arg2).mask(mask), In(arg1).mask(mask),
                                           In(ParameterFactory::createConstParam(blendMode.factor)),
                                           Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_DOTPRODUCT:
        stage.callFunction(FFP_FUNC_DOTPRODUCT, In(arg2).mask(mask), In(arg1).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_BLEND_DIFFUSE_COLOUR:
        stage.callFunction(FFP_FUNC_LERP, {In(arg2).mask(mask), In(arg1).mask(mask),
                                           In(mPSDiffuse).mask(mask), Out(mPSOutDiffuse).mask(mask)});
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

    if (auto rs = Root::getSingleton().getRenderSystem())
    {
        if (mIsPointSprite && !rs->getCapabilities()->hasCapability(RSC_POINT_SPRITES))
            return false;
    }

    //count the number of texture units we need to process
    size_t validTexUnits = 0;
    for (const auto tu : srcPass->getTextureUnitStates())
    {
        validTexUnits += int(isProcessingNeeded(tu));
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
void FFPTexturing::setTextureUnitCount(size_t count)
{
    mTextureUnitParamsList.resize(count);

    for (unsigned int i=0; i < count; ++i)
    {
        TextureUnitParams& curParams = mTextureUnitParamsList[i];

        curParams.mTextureUnitState             = NULL;
        curParams.mTextureSamplerIndex          = 0;              
        curParams.mTextureSamplerType           = GCT_SAMPLER2D;        
        curParams.mVSInTextureCoordinateType    = GCT_FLOAT2;   
        curParams.mVSOutTextureCoordinateType   = GCT_FLOAT2;       
    }
}

//-----------------------------------------------------------------------
void FFPTexturing::setTextureUnit(unsigned short index, TextureUnitState* textureUnitState)
{
    OgreAssert(index < mTextureUnitParamsList.size(), "FFPTexturing unit index out of bounds");
    OgreAssert(textureUnitState->getBindingType() == TextureUnitState::BT_FRAGMENT, "only fragment shaders supported");

    TextureUnitParams& curParams = mTextureUnitParamsList[index];


    curParams.mTextureSamplerIndex = index;
    curParams.mTextureUnitState    = textureUnitState;

    bool isGLES2 = Root::getSingletonPtr()->getRenderSystem()->getName().find("OpenGL ES 2") != String::npos;

    switch (curParams.mTextureUnitState->getTextureType())
    {
    case TEX_TYPE_1D:
        curParams.mTextureSamplerType = GCT_SAMPLER1D;
        curParams.mVSInTextureCoordinateType = GCT_FLOAT1;
        if(!isGLES2) // no 1D texture support
            break;
        OGRE_FALLTHROUGH;
    case TEX_TYPE_2D:
        curParams.mTextureSamplerType = GCT_SAMPLER2D;
        curParams.mVSInTextureCoordinateType = GCT_FLOAT2;
        break;
    case TEX_TYPE_2D_RECT:
        curParams.mTextureSamplerType = GCT_SAMPLERRECT;
        curParams.mVSInTextureCoordinateType = GCT_FLOAT2;
        break;
    case TEX_TYPE_EXTERNAL_OES:
        curParams.mTextureSamplerType = GCT_SAMPLER_EXTERNAL_OES;
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
