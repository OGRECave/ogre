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

    // Resolve input vertex shader tangent.
    if (mNormalMapSpace == NMS_TANGENT)
    {
        mVSInTangent = vsMain->resolveInputParameter(Parameter::SPC_TANGENT_OBJECT_SPACE);
        
        // Resolve local vertex shader TNB matrix.
        mVSTBNMatrix = vsMain->resolveLocalParameter("lMatTBN", GCT_MATRIX_3X3);
    }
    
    // Resolve input vertex shader texture coordinates.
    mVSInTexcoord = vsMain->resolveInputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);

    // Resolve output vertex shader texture coordinates.
    mVSOutTexcoord = vsMain->resolveOutputParameter(
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex), GCT_FLOAT2);

    // Resolve pixel input texture coordinates normal.
    mPSInTexcoord = psMain->resolveInputParameter(mVSOutTexcoord);

    // Resolve pixel shader normal.
    if (mNormalMapSpace == NMS_OBJECT)
    {
        mViewNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);
    }
    else if (mNormalMapSpace == NMS_TANGENT)
    {
        mViewNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_TANGENT_SPACE, GCT_FLOAT3);
    }

    mInDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mInDiffuse.get() == NULL)
    {
        mInDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    }
    OgreAssert(mInDiffuse, "mInDiffuse is NULL");

    mOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (mSpecularEnable)
    {
        mPSSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
        if (mPSSpecular.get() == NULL)
        {
            mPSSpecular = psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
        }

        OgreAssert(mPSSpecular, "mPSSpecular is NULL");

        mOutSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

        if (mNormalMapSpace == NMS_TANGENT)
        {
            mVSOutView = vsMain->resolveOutputParameter(Parameter::SPC_POSTOCAMERA_TANGENT_SPACE);
        }
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            mVSOutView = vsMain->resolveOutputParameter(Parameter::SPC_POSTOCAMERA_OBJECT_SPACE);
        }

        mToView = psMain->resolveInputParameter(mVSOutView);

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

    // Resolve per light parameters.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        switch (mLightParamsList[i].mType)
        {
        case Light::LT_DIRECTIONAL:
            mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_direction_obj_space");

            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0 + i));
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0 + i));
            }

            mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(mLightParamsList[i].mVSOutDirection);
            break;

        case Light::LT_POINT:       
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION, i);

            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0 + i));
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0 + i));
            }
            
            mLightParamsList[i].mToLight = psMain->resolveInputParameter(mLightParamsList[i].mVSOutToLightDir);

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);

            // Resolve local dir.
            if (mVSLocalDir.get() == NULL)
            {
                mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
            }   

            // Resolve world position.
            if (mVSWorldPosition.get() == NULL)
            {
                mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE);
            }   

            // Resolve world matrix.
            if (mWorldMatrix.get() == NULL)
            {               
                mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
            }

            // Resolve inverse world rotation matrix.
            if (mWorldInvRotMatrix.get() == NULL)
            {               
                mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_3X3, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");
            }
            break;

        case Light::LT_SPOTLIGHT:       
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_POSITION, i);
            
            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0 + i));
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0 + i));
            }
            
            mLightParamsList[i].mToLight = psMain->resolveInputParameter(mLightParamsList[i].mVSOutToLightDir);

            mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_direction_obj_space");

            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0 + i));
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0 + i));
            }
                        
            mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(mLightParamsList[i].mVSOutDirection);

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION, i);
            mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS, i);
            
            // Resolve local dir.
            if (mVSLocalDir.get() == NULL)
            {
                mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
            }   

            // Resolve world position.
            if (mVSWorldPosition.get() == NULL)
            {
                mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE);
            }   

            // Resolve world matrix.
            if (mWorldMatrix.get() == NULL)
            {               
                mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
            }
            
            // Resolve inverse world rotation matrix.
            if (mWorldInvRotMatrix.get() == NULL)
            {               
                mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_3X3, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");
            }
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

    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_TRANSFORM);
    vsProgram->addDependency(FFP_LIB_TEXTURING);
    psProgram->addDependency(FFP_LIB_TEXTURING);

    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(SGX_LIB_NORMALMAP);

    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_PERPIXELLIGHTING);

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
    if (false == addVSInvocation(vsMain, FFP_VS_LIGHTING))
        return false;

    auto stage = psMain->getStage(FFP_PS_COLOUR_BEGIN + 1);

    // Add the normal fetch function invocation.
    stage.callFunction(SGX_FUNC_FETCHNORMAL, mPSNormalMapSampler, mPSInTexcoord, mViewNormal);
    
    // Add the global illumination functions.
    if (false == addPSGlobalIlluminationInvocation(stage))
        return false;


    // Add per light functions.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        if (false == addIlluminationInvocation(&mLightParamsList[i], stage))
            return false;
    }

    // Assign back temporary variables to the ps diffuse and specular components.
    if (false == addPSFinalAssignmentInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addVSInvocation(Function* vsMain, const int groupOrder)
{
    auto stage = vsMain->getStage(groupOrder);

    // Construct TNB matrix.
    if (mNormalMapSpace == NMS_TANGENT)
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
    if (mVSInPosition.get() != NULL && 
        mVSOutView.get() != NULL)
    {   
        // View vector in world space.
        stage.callFunction(FFP_FUNC_SUBTRACT, In(mCamPosWorldSpace).xyz(), In(mVSWorldPosition).xyz(), mVSLocalDir);

        // Transform to object space.
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldInvRotMatrix, mVSLocalDir, mVSLocalDir);

        // Transform to tangent space.
        if (mNormalMapSpace == NMS_TANGENT)
        {
            stage.callFunction(FFP_FUNC_TRANSFORM, mVSTBNMatrix, mVSLocalDir, mVSOutView);
        }

        // Output object space.
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            stage.assign(mVSLocalDir, mVSOutView);
        }
    }

    // Add per light functions.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        if (false == addVSIlluminationInvocation(&mLightParamsList[i], vsMain, groupOrder))
            return false;
    }


    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addVSIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder)
{
    auto stage = vsMain->getStage(groupOrder);

    // Compute light direction in texture space.
    if (curLightParams->mDirection.get() != NULL &&
        curLightParams->mVSOutDirection.get() != NULL)
    {
        // Transform to texture space.
        if (mNormalMapSpace == NMS_TANGENT)
        {
            stage.callFunction(FFP_FUNC_TRANSFORM, mVSTBNMatrix, In(curLightParams->mDirection).xyz(),
                               curLightParams->mVSOutDirection);
        }
        // Output object space.
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            stage.assign(In(curLightParams->mDirection).xyz(), curLightParams->mVSOutDirection);
        }
    }
    
    // Transform light vector to target space..
    if (curLightParams->mPosition.get() != NULL &&
        curLightParams->mVSOutToLightDir.get() != NULL)
    {
        // Compute light vector.
        stage.callFunction(FFP_FUNC_SUBTRACT, In(curLightParams->mPosition).xyz(), mVSWorldPosition,
                           mVSLocalDir);

        // Transform to object space.
        stage.callFunction(FFP_FUNC_TRANSFORM, mWorldInvRotMatrix, mVSLocalDir, mVSLocalDir);

        // Transform to tangent space.      
        if (mNormalMapSpace == NMS_TANGENT)
        {
            stage.callFunction(FFP_FUNC_TRANSFORM, mVSTBNMatrix, mVSLocalDir,
                               curLightParams->mVSOutToLightDir);
        }

        // Output object space.
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            stage.assign(mVSLocalDir, curLightParams->mVSOutToLightDir);
        }
    }


    return true;
}

//-----------------------------------------------------------------------
void NormalMapLighting::copyFrom(const SubRenderState& rhs)
{
    const NormalMapLighting& rhsLighting = static_cast<const NormalMapLighting&>(rhs);

    int lightCount[3];

    rhsLighting.getLightCount(lightCount);
    setLightCount(lightCount);

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
