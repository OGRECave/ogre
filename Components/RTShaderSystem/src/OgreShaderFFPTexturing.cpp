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
const String SRS_TEXTURING = "FFP_Texturing";

#define _INT_VALUE(f) (*(int*)(&(f)))

const String c_ParamTexelEx("texel_");

//-----------------------------------------------------------------------
FFPTexturing::FFPTexturing() : mIsPointSprite(false), mLateAddBlend(false)
{
}

//-----------------------------------------------------------------------
const String& FFPTexturing::getType() const
{
    return SRS_TEXTURING;
}

//-----------------------------------------------------------------------
int FFPTexturing::getExecutionOrder() const
{
    return FFP_TEXTURING;
}

//-----------------------------------------------------------------------
bool FFPTexturing::resolveParameters(ProgramSet* programSet)
{
    for (auto & i : mTextureUnitParamsList)
    {
        TextureUnitParams* curParams = &i;
        if(!curParams->mTextureUnitState)
            continue;

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
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
        OGRE_FALLTHROUGH;
    case TEXCALC_ENVIRONMENT_MAP_NORMAL:
        //TODO: change the following 'mWorldITMatrix' member to 'mWorldViewITMatrix'
        mWorldITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_NORMAL_MATRIX);
        break;

    case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
        mWorldITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
        mViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_CAMERA_POSITION);
        break;

    case TEXCALC_PROJECTIVE_TEXTURE:
        textureUnitParams->mTextureViewProjImageMatrix = vsProgram->resolveParameter(
            GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX, textureUnitParams->mTextureSamplerIndex);
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

            // assume already resolved
            if(vsMain->getOutputParameter(texCoordContent, textureUnitParams->mVSInTextureCoordinateType))
                break;

            textureUnitParams->mVSInputTexCoord = vsMain->resolveInputParameter(
                Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 +
                                   textureUnitParams->mTextureUnitState->getTextureCoordSet()),
                textureUnitParams->mVSInTextureCoordinateType);
            break;

        case TEXCALC_ENVIRONMENT_MAP:
        case TEXCALC_ENVIRONMENT_MAP_PLANAR:
        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
            // Resolve vertex normal.
            mVSInputPos = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            OGRE_FALLTHROUGH;
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            mVSInputNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
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

    vsProgram->addDependency(FFP_LIB_TEXTURING);
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

    for (auto & i : mTextureUnitParamsList)
    {
        TextureUnitParams* curParams = &i;
        if(!curParams->mTextureUnitState)
            continue;

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
        if(textureUnitParams->mVSInputTexCoord)
        stage.assign(textureUnitParams->mVSInputTexCoord, textureUnitParams->mVSOutputTexCoord);
        break;
    case TEXCALC_ENVIRONMENT_MAP:
    case TEXCALC_ENVIRONMENT_MAP_PLANAR:
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_ENV_SPHERE,
                           {In(mWorldMatrix), In(mWorldITMatrix), In(mVSInputPos), In(mVSInputNormal),
                            Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_ENV_REFLECT,
                           {In(mWorldMatrix), In(mWorldITMatrix), In(mViewMatrix), In(mVSInputNormal), In(mVSInputPos),
                            Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    case TEXCALC_ENVIRONMENT_MAP_NORMAL:
        stage.callFunction(FFP_FUNC_GENERATE_TEXCOORD_ENV_NORMAL,
                           {In(mWorldITMatrix), In(mVSInputNormal), Out(textureUnitParams->mVSOutputTexCoord)});
        break;
    case TEXCALC_PROJECTIVE_TEXTURE:
        stage.callBuiltin("mul", textureUnitParams->mTextureViewProjImageMatrix, mVSInputPos,
                          textureUnitParams->mVSOutputTexCoord);
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

    // Build colour argument for source1.
    source1 = getPSArgument(texel, colourBlend.source1, colourBlend.colourArg1, colourBlend.alphaArg1, false);

    // Build colour argument for source2.
    source2 = getPSArgument(texel, colourBlend.source2, colourBlend.colourArg2, colourBlend.alphaArg2, false);

    if(source1 == texel || source2 == texel || colourBlend.operation == LBX_BLEND_TEXTURE_ALPHA)
        addPSSampleTexelInvocation(textureUnitParams, psMain, texel, FFP_PS_SAMPLING);

    bool needDifferentAlphaBlend = false;
    if (alphaBlend.operation != colourBlend.operation ||
        alphaBlend.source1 != colourBlend.source1 ||
        alphaBlend.source2 != colourBlend.source2 ||
        colourBlend.source1 == LBS_MANUAL ||
        colourBlend.source2 == LBS_MANUAL ||
        alphaBlend.source1 == LBS_MANUAL ||
        alphaBlend.source2 == LBS_MANUAL)
        needDifferentAlphaBlend = true;

    if(mLateAddBlend && colourBlend.operation == LBX_ADD)
    {
        groupOrder = FFP_PS_COLOUR_END + 50 + 20; // after PBR lighting
    }

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

        if(source1 == texel || source2 == texel || alphaBlend.operation == LBX_BLEND_TEXTURE_ALPHA)
            addPSSampleTexelInvocation(textureUnitParams, psMain, texel, FFP_PS_SAMPLING);

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

    stage.callBuiltin("texture2DProj", textureUnitParams->mTextureSampler, textureUnitParams->mPSInputTexCoord, texel);
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
    case LBX_MODULATE_X2:
    case LBX_MODULATE_X4:
        stage.mul(In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        if (blendMode.operation == LBX_MODULATE_X2)
            stage.mul(In(mPSOutDiffuse).mask(mask), 2.0, Out(mPSOutDiffuse).mask(mask));
        if (blendMode.operation == LBX_MODULATE_X4)
            stage.mul(In(mPSOutDiffuse).mask(mask), 4.0, Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_ADD:
    case LBX_ADD_SIGNED:
        stage.add(In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        if(blendMode.operation == LBX_ADD_SIGNED)
            stage.sub(In(mPSOutDiffuse).mask(mask), 0.5, Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_ADD_SMOOTH:
        stage.callFunction(FFP_FUNC_ADDSMOOTH, In(arg1).mask(mask), In(arg2).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_SUBTRACT:
        stage.sub(In(arg1).mask(mask), In(arg2).mask(mask), Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_BLEND_DIFFUSE_ALPHA:
        stage.callBuiltin(
            "mix", {In(arg2).mask(mask), In(arg1).mask(mask), In(mPSDiffuse).w(), Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_BLEND_TEXTURE_ALPHA:
        stage.callBuiltin("mix",
                          {In(arg2).mask(mask), In(arg1).mask(mask), In(texel).w(), Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_BLEND_CURRENT_ALPHA:
        stage.callBuiltin(
            "mix", {In(arg2).mask(mask), In(arg1).mask(mask), In(mPSOutDiffuse).w(), Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_BLEND_MANUAL:
        stage.callBuiltin(
            "mix", {In(arg2).mask(mask), In(arg1).mask(mask), In(blendMode.factor), Out(mPSOutDiffuse).mask(mask)});
        break;
    case LBX_DOTPRODUCT:
        stage.callFunction(FFP_FUNC_DOTPRODUCT, In(arg2).mask(mask), In(arg1).mask(mask),
                           Out(mPSOutDiffuse).mask(mask));
        break;
    case LBX_BLEND_DIFFUSE_COLOUR:
        stage.callBuiltin("mix", {In(arg2).mask(mask), In(arg1).mask(mask), In(mPSDiffuse).mask(mask),
                                  Out(mPSOutDiffuse).mask(mask)});
        break;
    }
}

//-----------------------------------------------------------------------
bool FFPTexturing::needsTextureMatrix(TextureUnitState* textureUnitState)
{
    for (auto& m : textureUnitState->getEffects())
    {
        switch (m.second.type)
        {

        case TextureUnitState::ET_UVSCROLL:
        case TextureUnitState::ET_USCROLL:
        case TextureUnitState::ET_VSCROLL:
        case TextureUnitState::ET_ROTATE:
        case TextureUnitState::ET_TRANSFORM:
            return true;
        case TextureUnitState::ET_ENVIRONMENT_MAP:
        case TextureUnitState::ET_PROJECTIVE_TEXTURE:
            break;
        }
    }

    const Ogre::Matrix4 matTexture = textureUnitState->getTextureTransform();

    // Resolve texture matrix parameter.
    if (matTexture != Matrix4::IDENTITY)
        return true;

    return false;
}


bool FFPTexturing::setParameter(const String& name, const String& value)
{
    if(name == "late_add_blend")
    {
        StringConverter::parse(value, mLateAddBlend);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------
void FFPTexturing::copyFrom(const SubRenderState& rhs)
{
    const FFPTexturing& rhsTexture = static_cast<const FFPTexturing&>(rhs);

    mLateAddBlend = rhsTexture.mLateAddBlend;
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

    setTextureUnitCount(srcPass->getTextureUnitStates().size());

    std::set<uint16> nonFFP_TUS;
    auto nonFFPany = srcPass->getUserObjectBindings().getUserAny("_RTSS_nonFFP_TUS");
    if(nonFFPany.has_value())
    {
        nonFFP_TUS = any_cast<std::set<uint16>>(nonFFPany);
    }

    // Build texture stage sub states.
    for (unsigned short i=0; i < srcPass->getNumTextureUnitStates(); ++i)
    {
        if(nonFFP_TUS.find(i) != nonFFP_TUS.end())
            continue;
        TextureUnitState* texUnitState = srcPass->getTextureUnitState(i);
        setTextureUnit(i, texUnitState);
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

    TextureUnitParams& curParams = mTextureUnitParamsList[index];


    curParams.mTextureSamplerIndex = index;
    curParams.mTextureUnitState    = textureUnitState;

    if(textureUnitState->isTextureLoadFailing()) // -> will be set to a 2D texture
        return;

    bool isGLES2 = ShaderGenerator::getSingleton().getTargetLanguage() == "glsles";

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
     curParams.mTexCoordCalcMethod = textureUnitState->_deriveTexCoordCalcMethod();

    if (curParams.mTexCoordCalcMethod == TEXCALC_PROJECTIVE_TEXTURE)
        curParams.mVSOutTextureCoordinateType = GCT_FLOAT4;

    // let TexCalcMethod override texture type, as it might be wrong for
    // content_type shadow & content_type compositor
    if (curParams.mTexCoordCalcMethod == TEXCALC_ENVIRONMENT_MAP_REFLECTION ||
        curParams.mTexCoordCalcMethod == TEXCALC_ENVIRONMENT_MAP_NORMAL)
    {
        curParams.mVSOutTextureCoordinateType = GCT_FLOAT3;
        curParams.mTextureSamplerType = GCT_SAMPLERCUBE;
    }

    if (curParams.mTexCoordCalcMethod == TEXCALC_ENVIRONMENT_MAP_PLANAR ||
        curParams.mTexCoordCalcMethod == TEXCALC_ENVIRONMENT_MAP)
    {
        curParams.mVSOutTextureCoordinateType = GCT_FLOAT2;
        curParams.mTextureSamplerType = GCT_SAMPLER2D;
    }
}

//-----------------------------------------------------------------------
const String& FFPTexturingFactory::getType() const
{
    return SRS_TEXTURING;
}

//-----------------------------------------------------------------------
SubRenderState* FFPTexturingFactory::createInstance(ScriptCompiler* compiler,
                                                 PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "texturing_stage")
    {
        if(prop->values.size() == 1)
        {
            const String& value = prop->values.front()->getString();

            auto inst = createOrRetrieveInstance(translator);

            if (value == "ffp")
                return inst;

            if (value == "late_add_blend")
                inst->setParameter(value, "true");

            return inst;
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------
void FFPTexturingFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState,
                                     Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "texturing_stage");
    ser->writeValue("late_add_blend"); // this is the only case where somebody would add this as a custom SRS
}

//-----------------------------------------------------------------------
SubRenderState* FFPTexturingFactory::createInstanceImpl()
{
    return OGRE_NEW FFPTexturing;
}


}
}

#endif
