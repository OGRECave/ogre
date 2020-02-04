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
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String NormalMapLighting::Type                      = "SGX_NormalMapLighting";

//-----------------------------------------------------------------------
NormalMapLighting::NormalMapLighting() : PerPixelLighting()
{
    mNormalMapSamplerIndex          = 0;
    mVSTexCoordSetIndex             = 0;
    mNormalMapSpace                 = NMS_TANGENT;
    mNormalMapSampler = TextureManager::getSingleton().createSampler();
    mNormalMapSampler->setMipmapBias(-1.0);
}

//-----------------------------------------------------------------------
const String& NormalMapLighting::getType() const
{
    return Type;
}

//-----------------------------------------------------------------------
void NormalMapLighting::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
    const LightList* pLightList)
{       
    if (mLightParamsList.empty())
        return;

    Light::LightTypes curLightType = Light::LT_DIRECTIONAL; 
    unsigned int curSearchLightIndex = 0;

    // We need the inverse of the inverse transpose
    Matrix3 matWorldInvRotation = source->getTransposeWorldMatrix().linear();

    // Update inverse rotation parameter.
    if (mWorldInvRotMatrix.get() != NULL)   
        mWorldInvRotMatrix->setGpuParameter(matWorldInvRotation);   
        
    // Update per light parameters.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {
        const LightParams& curParams = mLightParamsList[i];

        if (curLightType != curParams.mType)
        {
            curLightType = curParams.mType;
            curSearchLightIndex = 0;
        }

        // Search a matching light from the current sorted lights of the given renderable.
        size_t j;
        for (j = curSearchLightIndex; j < pLightList->size(); ++j)
        {
            if (pLightList->at(j)->getType() == curLightType)
            {               
                curSearchLightIndex = j + 1;
                break;
            }           
        }

        switch (curParams.mType)
        {
        case Light::LT_DIRECTIONAL:                                 
            {                       
                Vector3 vec3;                           

                // Update light direction. (Object space).
                vec3 = matWorldInvRotation * source->getLightDirection(j);
                curParams.mDirection->setGpuParameter(Vector4(-vec3.normalisedCopy(), 0.0f));
            }
            break;

        case Light::LT_POINT:
            // update light index. data will be set by scene manager
            curParams.mPosition->updateExtraInfo(j);
            curParams.mAttenuatParams->updateExtraInfo(j);
            break;

        case Light::LT_SPOTLIGHT:
            {                       
                Vector3 vec3;               
                                            
                // Update light position. (World space).                
                curParams.mPosition->updateExtraInfo(j);
                curParams.mAttenuatParams->updateExtraInfo(j);
                curParams.mSpotParams->updateExtraInfo(j);

                // Update light direction. (Object space).
                vec3 = matWorldInvRotation * source->getLightDirection(j);
                curParams.mDirection->setGpuParameter(Vector4(-vec3.normalisedCopy(), 0.0f));
            }
            break;
        }


        // Update diffuse colour.
        curParams.mDiffuseColour->updateExtraInfo(j);

        // Update specular colour if need to.
        if (mSpecularEnable)
        {
            curParams.mSpecularColour->updateExtraInfo(j);
        }                                                                           
    }
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolveGlobalParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();
    
    // Resolve normal map texture sampler parameter.
    if(!mLightParamsList.empty())
        mPSNormalMapSampler = psProgram->resolveParameter(GCT_SAMPLER2D, mNormalMapSamplerIndex, (uint16)GPV_PER_OBJECT, "gNormalMapSampler");

    // Get surface ambient colour if need to.
    if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
    {       
        mDerivedAmbientLightColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
    }
    else
    {
        mLightAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
    }

    // Get surface emissive colour if need to.
    if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        mSurfaceEmissiveColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
    }

    // Get derived scene colour.
    mDerivedSceneColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR);

    // Get surface shininess.
    mSurfaceShininess = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SHININESS);

    // Resolve input vertex shader normal.
    mVSInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);

    auto normalContent = Parameter::SPC_NORMAL_OBJECT_SPACE;
    auto posContent = Parameter::SPC_POSTOCAMERA_OBJECT_SPACE;

    // Resolve input vertex shader tangent.
    if (mNormalMapSpace & NMS_TANGENT)
    {
        mVSInTangent = vsMain->resolveInputParameter(Parameter::SPC_TANGENT_OBJECT_SPACE);
        
        // Resolve local vertex shader TNB matrix.
        mVSTBNMatrix = vsMain->resolveLocalParameter("lMatTBN", GCT_MATRIX_3X3);

        normalContent = Parameter::SPC_NORMAL_TANGENT_SPACE;
        posContent = Parameter::SPC_POSTOCAMERA_TANGENT_SPACE;
    }
    
    // Resolve input vertex shader texture coordinates.
    mVSInTexcoord = vsMain->resolveInputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);
    mVSOutTexcoord = vsMain->resolveOutputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);
    mPSInTexcoord = psMain->resolveInputParameter(mVSOutTexcoord);

    // Resolve pixel shader normal.
    mViewNormal = psMain->resolveLocalParameter(normalContent, GCT_FLOAT3);

    mInDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mInDiffuse.get() == NULL)
    {
        mInDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    }
    OgreAssert(mInDiffuse, "mInDiffuse is NULL");

    mOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (mSpecularEnable)
    {
        mOutSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);

        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
        mVSOutViewPos = vsMain->resolveOutputParameter(posContent);
        mToView = psMain->resolveInputParameter(mVSOutViewPos);

        // Resolve camera position world space.
        mCamPosWorldSpace = vsProgram->resolveParameter(GpuProgramParameters::ACT_CAMERA_POSITION);
        mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
        mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE);
        // Resolve world matrix.                
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);

        // Resolve inverse world rotation matrix.
        mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_3X3, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");
    }

    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolvePerLightParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();

    // at most 8 lights are supported
    if(mLightParamsList.size() > 8)
        mLightParamsList.resize(8);

    bool needViewPos = mNormalMapSpace == NMS_PARALLAX;

    auto lightDirContent = mNormalMapSpace & NMS_TANGENT ? Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0
                                                          : Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0;
    auto lightPosContent = mNormalMapSpace & NMS_TANGENT ? Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0
                                                          : Parameter::SPC_POSITION_OBJECT_SPACE;

    // Resolve per light parameters.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        switch (mLightParamsList[i].mType)
        {
        case Light::LT_DIRECTIONAL:
            mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_direction_obj_space");
            mLightParamsList[i].mVSOutDirection =
                vsMain->resolveOutputParameter(Parameter::Content(lightDirContent + i));
            mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(mLightParamsList[i].mVSOutDirection);
            break;

        case Light::LT_POINT:
            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION, i);
            mLightParamsList[i].mVSOutToLightDir =
                vsMain->resolveOutputParameter(Parameter::Content(lightPosContent + i), GCT_FLOAT3);
            mLightParamsList[i].mToLight = psMain->resolveInputParameter(mLightParamsList[i].mVSOutToLightDir);

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);

            needViewPos = true;
            break;

        case Light::LT_SPOTLIGHT:

            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION, i);
            mLightParamsList[i].mVSOutToLightDir =
                vsMain->resolveOutputParameter(Parameter::Content(lightPosContent + i), GCT_FLOAT3);
            mLightParamsList[i].mToLight = psMain->resolveInputParameter(mLightParamsList[i].mVSOutToLightDir);

            mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_direction_obj_space");
            mLightParamsList[i].mVSOutDirection =
                vsMain->resolveOutputParameter(Parameter::Content(lightDirContent + i));
            mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(mLightParamsList[i].mVSOutDirection);

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);
            mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS, i);

            needViewPos = true;
            break;
        }

        // Resolve diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR, i);
        }
        else
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED, i);
        }       
            
        if (mSpecularEnable)
        {
            // Resolve specular colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR, i);
            }
            else
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED, i);
            }   
        }   

    }

    if(needViewPos)
    {
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
        // Resolve local dir.
        mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
        mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE);
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
        // Resolve inverse world rotation matrix.
        mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_3X3, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");
    }

    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_TRANSFORM);
    vsProgram->addDependency(FFP_LIB_TEXTURING);
    vsProgram->addDependency(SGX_LIB_NORMALMAP);

    psProgram->addDependency(FFP_LIB_TEXTURING);
    psProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);
    psProgram->addDependency(SGX_LIB_NORMALMAP);

    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();  

    // Add the global illumination functions.
    addVSInvocation(vsMain->getStage(FFP_VS_LIGHTING));

    auto stage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1);

    if (!mLightParamsList.empty() && mNormalMapSpace == NMS_PARALLAX)
    {
        // TODO: user specificed scale and bias
        stage.callFunction("SGX_Generate_Parallax_Texcoord", {In(mPSNormalMapSampler), In(mPSInTexcoord), In(mToView),
                                                              In(Vector2(0.04, -0.02)), Out(mPSInTexcoord)});

        // overwrite texcoord0 unconditionally, only one texcoord set is supported with parallax mapping
        // we are before FFP_PS_TEXTURING, so the new value will be used
        auto texcoord0 = psMain->resolveInputParameter(Parameter::SPC_TEXTURE_COORDINATE0, GCT_FLOAT2);
        stage.assign(mPSInTexcoord, texcoord0);
    }

    // Add the normal fetch function invocation
    if(!mLightParamsList.empty())
    {
        stage.callFunction(SGX_FUNC_FETCHNORMAL, mPSNormalMapSampler, mPSInTexcoord, mViewNormal);
    }

    // Add the global illumination functions.
    addPSGlobalIlluminationInvocation(stage);

    // Add per light functions.
    for (const auto& lp : mLightParamsList)
    {
        addIlluminationInvocation(&lp, stage);
    }

    // Assign back temporary variables
    stage.assign(mOutDiffuse, mInDiffuse);

    return true;
}

//-----------------------------------------------------------------------
void NormalMapLighting::addVSInvocation(const FunctionStageRef& stage)
{
    // Construct TNB matrix.
    if (mNormalMapSpace & NMS_TANGENT)
    {
        stage.callFunction(SGX_FUNC_CONSTRUCT_TBNMATRIX, mVSInNormal, mVSInTangent, mVSTBNMatrix);
    }
    
    // Output texture coordinates.
    stage.assign(mVSInTexcoord, mVSOutTexcoord);

    // Compute world space position.
    if (mVSWorldPosition.get() != NULL)
    {
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldMatrix, mVSInPosition, mVSWorldPosition);
    }
    


    // Compute view vector.
    if (mVSInPosition && mVSOutViewPos)
    {
        // View vector in world space.
        stage.sub(In(mCamPosWorldSpace).xyz(), In(mVSWorldPosition).xyz(), mVSLocalDir);

        // Transform to object space.
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldInvRotMatrix, mVSLocalDir, mVSLocalDir);

        // Transform to tangent space.
        if (mNormalMapSpace & NMS_TANGENT)
        {
            stage.callFunction(FFP_FUNC_TRANSFORM, mVSTBNMatrix, mVSLocalDir, mVSOutViewPos);
        }

        // Output object space.
        else if (mNormalMapSpace & NMS_OBJECT)
        {
            stage.assign(mVSLocalDir, mVSOutViewPos);
        }
    }

    // Add per light functions.
    for (const auto& lp : mLightParamsList)
    {
        addVSIlluminationInvocation(&lp, stage);
    }
}

//-----------------------------------------------------------------------
void NormalMapLighting::addVSIlluminationInvocation(const LightParams* curLightParams, const FunctionStageRef& stage)
{
    // Compute light direction in texture space.
    if (curLightParams->mDirection.get() != NULL &&
        curLightParams->mVSOutDirection.get() != NULL)
    {
        // Transform to texture space.
        if (mNormalMapSpace & NMS_TANGENT)
        {
            stage.callFunction(FFP_FUNC_TRANSFORM, mVSTBNMatrix, In(curLightParams->mDirection).xyz(),
                               curLightParams->mVSOutDirection);
        }
        // Output object space.
        else if (mNormalMapSpace & NMS_OBJECT)
        {
            stage.assign(In(curLightParams->mDirection).xyz(), curLightParams->mVSOutDirection);
        }
    }
    
    // Transform light vector to target space..
    if (curLightParams->mPosition.get() != NULL &&
        curLightParams->mVSOutToLightDir.get() != NULL)
    {
        // Compute light vector.
        stage.sub(In(curLightParams->mPosition).xyz(), mVSWorldPosition, mVSLocalDir);

        // Transform to object space.
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldInvRotMatrix, mVSLocalDir, mVSLocalDir);

        // Transform to tangent space.      
        if (mNormalMapSpace & NMS_TANGENT)
        {
            stage.callFunction(FFP_FUNC_TRANSFORM, mVSTBNMatrix, mVSLocalDir,
                               curLightParams->mVSOutToLightDir);
        }

        // Output object space.
        else if (mNormalMapSpace & NMS_OBJECT)
        {
            stage.assign(mVSLocalDir, curLightParams->mVSOutToLightDir);
        }
    }
}

//-----------------------------------------------------------------------
void NormalMapLighting::copyFrom(const SubRenderState& rhs)
{
    const NormalMapLighting& rhsLighting = static_cast<const NormalMapLighting&>(rhs);

    setLightCount(rhsLighting.getLightCount());

    mTrackVertexColourType = rhsLighting.mTrackVertexColourType;
    mSpecularEnable = rhsLighting.mSpecularEnable;
    mNormalMapSpace = rhsLighting.mNormalMapSpace;
    mNormalMapTextureName = rhsLighting.mNormalMapTextureName;
    mNormalMapSampler = rhsLighting.mNormalMapSampler;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (!PerPixelLighting::preAddToRenderState(renderState, srcPass, dstPass))
        return false;

    TextureUnitState* normalMapTexture = dstPass->createTextureUnitState();

    normalMapTexture->setTextureName(mNormalMapTextureName);
    normalMapTexture->setSampler(mNormalMapSampler);
    mNormalMapSamplerIndex = dstPass->getNumTextureUnitStates() - 1;

    if(mNormalMapSpace == NMS_PARALLAX)
        mSpecularEnable = true;

    return true;
}

//-----------------------------------------------------------------------
const String& NormalMapLightingFactory::getType() const
{
    return NormalMapLighting::Type;
}

//-----------------------------------------------------------------------
SubRenderState* NormalMapLightingFactory::createInstance(ScriptCompiler* compiler, 
                                                        PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "lighting_stage")
    {
        if(prop->values.size() >= 2)
        {
            String strValue;
            AbstractNodeList::const_iterator it = prop->values.begin();
            
            // Read light model type.
            if(false == SGScriptTranslator::getString(*it, &strValue))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                return NULL;
            }

            // Case light model type is normal map
            if (strValue == "normal_map")
            {
                ++it;
                if (false == SGScriptTranslator::getString(*it, &strValue))
                {
                    compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    return NULL;
                }

                
                SubRenderState* subRenderState = createOrRetrieveInstance(translator);
                NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);
                
                normalMapSubRenderState->setNormalMapTextureName(strValue);

                
                // Read normal map space type.
                if (prop->values.size() >= 3)
                {                   
                    ++it;
                    if (false == SGScriptTranslator::getString(*it, &strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return NULL;
                    }

                    // Normal map defines normals in tangent space.
                    if (strValue == "tangent_space")
                    {
                        normalMapSubRenderState->setNormalMapSpace(NormalMapLighting::NMS_TANGENT);
                    }

                    // Normal map defines normals in object space.
                    if (strValue == "object_space")
                    {
                        normalMapSubRenderState->setNormalMapSpace(NormalMapLighting::NMS_OBJECT);
                    }

                    if (strValue == "parallax")
                    {
                        normalMapSubRenderState->setNormalMapSpace(NormalMapLighting::NMS_PARALLAX);
                    }
                }

                // Read texture coordinate index.
                if (prop->values.size() >= 4)
                {   
                    unsigned int textureCoordinateIndex = 0;

                    ++it;
                    if (SGScriptTranslator::getUInt(*it, &textureCoordinateIndex))
                    {
                        normalMapSubRenderState->setTexCoordIndex(textureCoordinateIndex);
                    }
                }

                // Read texture filtering format.
                if (prop->values.size() >= 5)
                {
                    ++it;
                    if (false == SGScriptTranslator::getString(*it, &strValue))
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return NULL;
                    }

                    if (strValue == "none")
                    {
                        normalMapSubRenderState->setNormalMapFiltering(FO_POINT, FO_POINT, FO_NONE);
                    }

                    else if (strValue == "bilinear")
                    {
                        normalMapSubRenderState->setNormalMapFiltering(FO_LINEAR, FO_LINEAR, FO_POINT);
                    }

                    else if (strValue == "trilinear")
                    {
                        normalMapSubRenderState->setNormalMapFiltering(FO_LINEAR, FO_LINEAR, FO_LINEAR);
                    }

                    else if (strValue == "anisotropic")
                    {
                        normalMapSubRenderState->setNormalMapFiltering(FO_ANISOTROPIC, FO_ANISOTROPIC, FO_LINEAR);
                    }
                    else
                    {
                        // sampler reference
                        normalMapSubRenderState->setNormalMapSampler(TextureManager::getSingleton().getSampler(strValue));
                        return subRenderState;
                    }

                    compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line, "use sampler reference");
                }

                // Read max anisotropy value.
                if (prop->values.size() >= 6)
                {   
                    unsigned int maxAnisotropy = 0;

                    ++it;
                    if (SGScriptTranslator::getUInt(*it, &maxAnisotropy))
                    {
                        normalMapSubRenderState->setNormalMapAnisotropy(maxAnisotropy);
                    }
                }

                // Read mip bias value.
                if (prop->values.size() >= 7)
                {   
                    Real mipBias = 0;

                    ++it;
                    if (SGScriptTranslator::getReal(*it, &mipBias))
                    {
                        normalMapSubRenderState->setNormalMapMipBias(mipBias);
                    }
                }
                                
                return subRenderState;                              
            }
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------
void NormalMapLightingFactory::writeInstance(MaterialSerializer* ser, 
                                             SubRenderState* subRenderState, 
                                             Pass* srcPass, Pass* dstPass)
{
    NormalMapLighting* normalMapSubRenderState = static_cast<NormalMapLighting*>(subRenderState);

    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("normal_map");
    ser->writeValue(normalMapSubRenderState->getNormalMapTextureName());    
    
    if (normalMapSubRenderState->getNormalMapSpace() == NormalMapLighting::NMS_TANGENT)
    {
        ser->writeValue("tangent_space");
    }
    else if (normalMapSubRenderState->getNormalMapSpace() == NormalMapLighting::NMS_OBJECT)
    {
        ser->writeValue("object_space");
    }

    ser->writeValue(StringConverter::toString(normalMapSubRenderState->getTexCoordIndex()));
}

//-----------------------------------------------------------------------
SubRenderState* NormalMapLightingFactory::createInstanceImpl()
{
    return OGRE_NEW NormalMapLighting;
}

}
}

#endif
