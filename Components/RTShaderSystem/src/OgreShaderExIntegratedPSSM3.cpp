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

/************************************************************************/
/*                                                                      */
/************************************************************************/
String IntegratedPSSM3::Type = "SGX_IntegratedPSSM3";

//-----------------------------------------------------------------------
IntegratedPSSM3::IntegratedPSSM3()
{   
    
}

//-----------------------------------------------------------------------
const String& IntegratedPSSM3::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int IntegratedPSSM3::getExecutionOrder() const
{
    return FFP_TEXTURING + 1;
}

//-----------------------------------------------------------------------
void IntegratedPSSM3::updateGpuProgramsParams(Renderable* rend, Pass* pass, 
                                             const AutoParamDataSource* source, 
                                             const LightList* pLightList)
{
    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();
    size_t shadowIndex = 0;

    while(it != mShadowTextureParamsList.end())
    {                       
        it->mWorldViewProjMatrix->setGpuParameter(source->getTextureWorldViewProjMatrix(shadowIndex));              
        it->mInvTextureSize->setGpuParameter(source->getInverseTextureSize(it->mTextureSamplerIndex));
        
        ++it;
        ++shadowIndex;
    }

    Vector4 vSplitPoints;

    vSplitPoints.x = mShadowTextureParamsList[0].mMaxRange;
    vSplitPoints.y = mShadowTextureParamsList[1].mMaxRange;
    vSplitPoints.z = 0.0;
    vSplitPoints.w = 0.0;

    mPSSplitPoints->setGpuParameter(vSplitPoints);

}

//-----------------------------------------------------------------------
void IntegratedPSSM3::copyFrom(const SubRenderState& rhs)
{
    const IntegratedPSSM3& rhsPssm= static_cast<const IntegratedPSSM3&>(rhs);

    mShadowTextureParamsList.resize(rhsPssm.mShadowTextureParamsList.size());

    ShadowTextureParamsConstIterator itSrc = rhsPssm.mShadowTextureParamsList.begin();
    ShadowTextureParamsIterator itDst = mShadowTextureParamsList.begin();

    while(itDst != mShadowTextureParamsList.end())
    {
        itDst->mMaxRange = itSrc->mMaxRange;        
        ++itSrc;
        ++itDst;
    }
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::preAddToRenderState(const RenderState* renderState, 
                                         Pass* srcPass, Pass* dstPass)
{
    if (srcPass->getLightingEnabled() == false ||
        srcPass->getParent()->getParent()->getReceiveShadows() == false)
        return false;

    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();

    while(it != mShadowTextureParamsList.end())
    {
        TextureUnitState* curShadowTexture = dstPass->createTextureUnitState();
            
        curShadowTexture->setContentType(TextureUnitState::CONTENT_SHADOW);
        curShadowTexture->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
        curShadowTexture->setTextureBorderColour(ColourValue::White);
        it->mTextureSamplerIndex = dstPass->getNumTextureUnitStates() - 1;
        ++it;
    }

    

    return true;
}

//-----------------------------------------------------------------------
void IntegratedPSSM3::setSplitPoints(const SplitPointList& newSplitPoints)
{
    if (newSplitPoints.size() != 4)
    {
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
            "IntegratedPSSM3 sub render state supports only 4 split points",
            "IntegratedPSSM3::setSplitPoints");
    }

    mShadowTextureParamsList.resize(newSplitPoints.size() - 1);

    for (unsigned int i=1; i < newSplitPoints.size(); ++i)
    {
        ShadowTextureParams& curParams = mShadowTextureParamsList[i-1];

        curParams.mMaxRange             = newSplitPoints[i];        
    }
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();
    
    // Get input position parameter.
    mVSInPos = vsMain->getInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
    
    // Get output position parameter.
    mVSOutPos = vsMain->getOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);
    
    // Resolve vertex shader output depth.      
    mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPC_DEPTH_VIEW_SPACE);
    
    // Resolve input depth parameter.
    mPSInDepth = psMain->resolveInputParameter(mVSOutDepth);
    
    // Get in/local diffuse parameter.
    mPSDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mPSDiffuse.get() == NULL)   
    {
        mPSDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    }
    
    // Resolve output diffuse parameter.
    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);
    
    // Get in/local specular parameter.
    mPSSpecualr = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
    if (mPSSpecualr.get() == NULL)  
    {
        mPSSpecualr = psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
    }
    
    // Resolve computed local shadow colour parameter.
    mPSLocalShadowFactor = psMain->resolveLocalParameter("lShadowFactor", GCT_FLOAT1);

    // Resolve computed local shadow colour parameter.
    mPSSplitPoints = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "pssm_split_points");

    // Get derived scene colour.
    mPSDerivedSceneColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);
    
    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();
    int lightIndex = 0;

    while(it != mShadowTextureParamsList.end())
    {
        it->mWorldViewProjMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "world_texture_view_proj"); 

        it->mVSOutLightPosition = vsMain->resolveOutputParameter(Parameter::Content(Parameter::SPC_POSITION_LIGHT_SPACE0 + lightIndex));        
        it->mPSInLightPosition = psMain->resolveInputParameter(it->mVSOutLightPosition);    
        it->mTextureSampler = psProgram->resolveParameter(GCT_SAMPLER2D, it->mTextureSamplerIndex, (uint16)GPV_GLOBAL, "shadow_map");       
        
        it->mInvTextureSize = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "inv_shadow_texture_size");       

        if (!(it->mInvTextureSize.get()) || !(it->mTextureSampler.get()) || !(it->mPSInLightPosition.get()) ||
            !(it->mVSOutLightPosition.get()) || !(it->mWorldViewProjMatrix.get()))
        {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "IntegratedPSSM3::resolveParameters" );
        }

        ++lightIndex;
        ++it;
    }

    if (!(mVSInPos.get()) || !(mVSOutPos.get()) || !(mVSOutDepth.get()) || !(mPSInDepth.get()) || !(mPSDiffuse.get()) || !(mPSOutDiffuse.get()) || 
        !(mPSSpecualr.get()) || !(mPSLocalShadowFactor.get()) || !(mPSSplitPoints.get()) || !(mPSDerivedSceneColour.get()))
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "IntegratedPSSM3::resolveParameters" );
    }

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_COMMON);
    
    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_INTEGRATEDPSSM);

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    // Add vertex shader invocations.
    if (false == addVSInvocation(vsMain, FFP_VS_TEXTURING + 1))
        return false;

    // Add pixel shader invocations.
    if (false == addPSInvocation(psProgram, FFP_PS_COLOUR_BEGIN + 2))
        return false;

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addVSInvocation(Function* vsMain, const int groupOrder)
{
    auto stage = vsMain->getStage(groupOrder);

    // Output the vertex depth in camera space.
    stage.assign(In(mVSOutPos).z(), mVSOutDepth);

    // Compute world space position.    
    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();

    while(it != mShadowTextureParamsList.end())
    {
        stage.callFunction(FFP_FUNC_TRANSFORM, it->mWorldViewProjMatrix, mVSInPos, it->mVSOutLightPosition);
        ++it;
    }

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addPSInvocation(Program* psProgram, const int groupOrder)
{
    Function* psMain = psProgram->getEntryPointFunction();
    auto stage = psMain->getStage(groupOrder);

    ShadowTextureParams& splitParams0 = mShadowTextureParamsList[0];
    ShadowTextureParams& splitParams1 = mShadowTextureParamsList[1];
    ShadowTextureParams& splitParams2 = mShadowTextureParamsList[2];

    // Compute shadow factor.
    stage.callFunction(SGX_FUNC_COMPUTE_SHADOW_COLOUR3,
                       {In(mPSInDepth), In(mPSSplitPoints), In(splitParams0.mPSInLightPosition),
                        In(splitParams1.mPSInLightPosition), In(splitParams2.mPSInLightPosition),
                        In(splitParams0.mTextureSampler), In(splitParams1.mTextureSampler),
                        In(splitParams2.mTextureSampler), In(splitParams0.mInvTextureSize),
                        In(splitParams1.mInvTextureSize), In(splitParams2.mInvTextureSize), Out(mPSLocalShadowFactor)});

    // Apply shadow factor on diffuse colour.
    stage.callFunction(SGX_FUNC_APPLYSHADOWFACTOR_DIFFUSE,
                       {In(mPSDerivedSceneColour), In(mPSDiffuse), In(mPSLocalShadowFactor), Out(mPSDiffuse)});

    // Apply shadow factor on specular colour.
    stage.callFunction(SGX_FUNC_MODULATE_SCALAR, mPSLocalShadowFactor, mPSSpecualr, mPSSpecualr);

    // Assign the local diffuse to output diffuse.
    stage.assign(mPSDiffuse, mPSOutDiffuse);

    return true;
}



//-----------------------------------------------------------------------
const String& IntegratedPSSM3Factory::getType() const
{
    return IntegratedPSSM3::Type;
}

//-----------------------------------------------------------------------
SubRenderState* IntegratedPSSM3Factory::createInstance(ScriptCompiler* compiler, 
                                                      PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "integrated_pssm4")
    {       
        if (prop->values.size() != 4)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        }
        else
        {
            IntegratedPSSM3::SplitPointList splitPointList; 

            AbstractNodeList::const_iterator it = prop->values.begin();
            AbstractNodeList::const_iterator itEnd = prop->values.end();

            while(it != itEnd)
            {
                Real curSplitValue;
                
                if (false == SGScriptTranslator::getReal(*it, &curSplitValue))
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                }

                splitPointList.push_back(curSplitValue);

                ++it;
            }

            if (splitPointList.size() == 4)
            {
                SubRenderState* subRenderState = createOrRetrieveInstance(translator);
                IntegratedPSSM3* pssmSubRenderState = static_cast<IntegratedPSSM3*>(subRenderState);

                pssmSubRenderState->setSplitPoints(splitPointList);

                return pssmSubRenderState;
            }
        }       
    }

    return NULL;
}

//-----------------------------------------------------------------------
SubRenderState* IntegratedPSSM3Factory::createInstanceImpl()
{
    return OGRE_NEW IntegratedPSSM3;
}

}
}

#endif
