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
void NormalMapLighting::updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, 
    const LightList* pLightList)
{       
    if (mLightParamsList.empty())
        return;

    Light::LightTypes curLightType = Light::LT_DIRECTIONAL; 
    unsigned int curSearchLightIndex = 0;
    const Affine3& matWorld = source->getWorldMatrix();
    Matrix3 matWorldInvRotation;
    Vector3 vRow0(matWorld[0][0], matWorld[0][1], matWorld[0][2]); 
    Vector3 vRow1(matWorld[1][0], matWorld[1][1], matWorld[1][2]); 
    Vector3 vRow2(matWorld[2][0], matWorld[2][1], matWorld[2][2]); 

    vRow0.normalise();
    vRow1.normalise();
    vRow2.normalise();

    matWorldInvRotation.SetColumn(0, vRow0);
    matWorldInvRotation.SetColumn(1, vRow1);
    matWorldInvRotation.SetColumn(2, vRow2);

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

        Vector4     vParameter;
        ColourValue colour;

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
                vec3.normalise();

                vParameter.x = -vec3.x;
                vParameter.y = -vec3.y;
                vParameter.z = -vec3.z;
                vParameter.w = 0.0;
                curParams.mDirection->setGpuParameter(vParameter);
            }
            break;

        case Light::LT_POINT:

            // Update light position. (World space).                
            vParameter = source->getLightAs4DVector(j);
            curParams.mPosition->setGpuParameter(vParameter);

            // Update light attenuation parameters.
            vParameter = source->getLightAttenuation(j);
            curParams.mAttenuatParams->setGpuParameter(vParameter);
            break;

        case Light::LT_SPOTLIGHT:
            {                       
                Vector3 vec3;               
                                            
                // Update light position. (World space).                
                vParameter = source->getLightAs4DVector(j);
                curParams.mPosition->setGpuParameter(vParameter);

                            
                // Update light direction. (Object space).
                vec3 = matWorldInvRotation * source->getLightDirection(j);
                vec3.normalise();
            
                vParameter.x = -vec3.x;
                vParameter.y = -vec3.y;
                vParameter.z = -vec3.z;
                vParameter.w = 0.0;
                curParams.mDirection->setGpuParameter(vParameter);                          
                
                // Update light attenuation parameters.
                vParameter = source->getLightAttenuation(j);
                curParams.mAttenuatParams->setGpuParameter(vParameter);

                // Update spotlight parameters.
                vec3 = source->getSpotlightParams(j).xyz();
                curParams.mSpotParams->setGpuParameter(vec3);
            }
            break;
        }


        // Update diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            colour = pass->getDiffuse() * source->getLightDiffuseColourWithPower(j);
            curParams.mDiffuseColour->setGpuParameter(colour);                  
        }
        else
        {                   
            colour = source->getLightDiffuseColourWithPower(j);
            curParams.mDiffuseColour->setGpuParameter(colour);  
        }

        // Update specular colour if need to.
        if (mSpecularEnable)
        {
            // Update diffuse colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                colour = pass->getSpecular() * source->getLightSpecularColourWithPower(j);
                curParams.mSpecularColour->setGpuParameter(colour);                 
            }
            else
            {                   
                colour = source->getLightSpecularColourWithPower(j);
                curParams.mSpecularColour->setGpuParameter(colour); 
            }
        }                                                                           
    }
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolveGlobalParameters(ProgramSet* programSet)
{
    bool hasError = false;
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
        hasError |= !(mDerivedAmbientLightColour.get());
    }
    else
    {
        mLightAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
        mSurfaceAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
    
        hasError |= !(mDerivedAmbientLightColour.get()) || !(mLightAmbientColour.get());
    }

    // Get surface diffuse colour if need to.
    if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
    {
        mSurfaceDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
        hasError |= !(mSurfaceDiffuseColour.get());
    }

    // Get surface specular colour if need to.
    if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
    {
        mSurfaceSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
        hasError |= !(mSurfaceSpecularColour.get());
    }


    // Get surface emissive colour if need to.
    if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        mSurfaceEmissiveColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR);
        hasError |= !(mSurfaceEmissiveColour.get());
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
        
        hasError |= !(mVSTBNMatrix.get()) || !(mVSInTangent.get());
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
        mPSNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT4);
        hasError |= !(mPSNormal.get());
    }
    else if (mNormalMapSpace == NMS_TANGENT)
    {
        mPSNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_TANGENT_SPACE, GCT_FLOAT4);
        hasError |= !(mPSNormal.get());
    }

    mInDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mInDiffuse.get() == NULL)
    {
        mInDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
    }

    mOutDiffuse = psMain->resolveOutputParameter(Parameter::SPC_COLOR_DIFFUSE);

    if (mSpecularEnable)
    {
        mPSSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
        if (mPSSpecular.get() == NULL)
        {
            mPSSpecular = psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
        }

        mOutSpecular = psMain->resolveLocalParameter(Parameter::SPC_COLOR_SPECULAR);
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

        if (mNormalMapSpace == NMS_TANGENT)
        {
            mVSOutView = vsMain->resolveOutputParameter(Parameter::SPC_POSTOCAMERA_TANGENT_SPACE);
            hasError |= !(mVSOutView.get());
        }
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            mVSOutView = vsMain->resolveOutputParameter(Parameter::SPC_POSTOCAMERA_OBJECT_SPACE);
            hasError |= !(mVSOutView.get());
        }
        

        mPSInView = psMain->resolveInputParameter(mVSOutView);

        // Resolve camera position world space.
        mCamPosWorldSpace = vsProgram->resolveParameter(GpuProgramParameters::ACT_CAMERA_POSITION);
        mVSLocalDir = vsMain->resolveLocalParameter("lNormalMapTempDir", GCT_FLOAT3);
        mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPC_POSITION_WORLD_SPACE);
        // Resolve world matrix.                
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);

        // Resolve inverse world rotation matrix.
        mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");

        hasError |= !(mPSSpecular.get()) || !(mOutSpecular.get()) || !(mVSInPosition.get()) || !(mPSInView.get()) || !(mCamPosWorldSpace.get()) ||
            !(mVSLocalDir.get()) || !(mVSWorldPosition.get()) || !(mWorldMatrix.get()) || !(mWorldInvRotMatrix.get());
    }

    hasError |= !(mPSNormalMapSampler.get()) || !(mDerivedSceneColour.get()) || !(mSurfaceShininess.get()) || !(mVSInNormal.get()) || 
        !(mVSInTexcoord.get()) || !(mVSOutTexcoord.get()) || !(mPSInTexcoord.get()) || !(mInDiffuse.get()) || !(mOutDiffuse.get()) ||
        !(mOutDiffuse.get());

    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "NormalMapLighting::resolveGlobalParameters" );
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

    bool hasError = false;
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

            hasError = !(mLightParamsList[i].mDirection.get()) || !(mLightParamsList[i].mVSOutDirection.get()) || !(mLightParamsList[i].mPSInDirection.get());
            break;

        case Light::LT_POINT:       
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_position_world_space");

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
            
            mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(mLightParamsList[i].mVSOutToLightDir);

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");

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
                mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");
            }       

            hasError |= !(mVSInPosition.get()) || !(mLightParamsList[i].mPosition.get()) || !(mLightParamsList[i].mVSOutToLightDir.get()) ||
                 !(mLightParamsList[i].mPSInToLightDir.get()) || !(mLightParamsList[i].mAttenuatParams.get()) || !(mVSLocalDir.get()) ||
                 !(mVSWorldPosition.get()) || !(mWorldMatrix.get()) || !(mWorldInvRotMatrix.get());
            
            break;

        case Light::LT_SPOTLIGHT:       
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_position_world_space");
            
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
            
            mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(mLightParamsList[i].mVSOutToLightDir);

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

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");

            mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
            
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
                mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");
            }
                  
            hasError |= !(mVSInPosition.get()) || !(mLightParamsList[i].mPosition.get()) || !(mLightParamsList[i].mVSOutToLightDir.get()) || 
                !(mLightParamsList[i].mPSInToLightDir.get()) || !(mLightParamsList[i].mDirection.get()) || !(mLightParamsList[i].mVSOutDirection.get()) || 
                !(mLightParamsList[i].mPSInDirection.get()) || !(mLightParamsList[i].mAttenuatParams.get()) || !(mLightParamsList[i].mSpotParams.get()) ||
                !(mVSLocalDir.get()) || !(mVSWorldPosition.get()) || !(mWorldMatrix.get()) || !(mWorldInvRotMatrix.get());
                        
            break;
        }

        // Resolve diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "derived_light_diffuse");
        }
        else
        {
            mLightParamsList[i].mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_diffuse");
        }       

        hasError |= !(mLightParamsList[i].mDiffuseColour.get());
            
        if (mSpecularEnable)
        {
            // Resolve specular colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "derived_light_specular");
            }
            else
            {
                mLightParamsList[i].mSpecularColour = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_specular");
            }   
            hasError |= !(mLightParamsList[i].mSpecularColour.get());
        }   

    }

    
    if (hasError)
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "NormalMapLighting::resolvePerLightParameters" );
    }
    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_TEXTURING);
    psProgram->addDependency(FFP_LIB_TEXTURING);

    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(SGX_LIB_NORMALMAPLIGHTING);

    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_NORMALMAPLIGHTING);

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

    // Add the normal fetch function invocation.
    if (false == addPSNormalFetchInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;

    
    // Add the global illumination functions.
    if (false == addPSGlobalIlluminationInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    // Add per light functions.
    for (unsigned int i=0; i < mLightParamsList.size(); ++i)
    {       
        if (false == addPSIlluminationInvocation(&mLightParamsList[i], psMain, FFP_PS_COLOUR_BEGIN + 1))
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
        stage.callFunction(SGX_FUNC_TRANSFORMPOSITION, mWorldMatrix, mVSInPosition, mVSWorldPosition);
    }
    

    // Compute view vector.
    if (mVSInPosition.get() != NULL && 
        mVSOutView.get() != NULL)
    {   
        // View vector in world space.
        stage.callFunction(FFP_FUNC_SUBTRACT, In(mCamPosWorldSpace).xyz(), In(mVSWorldPosition).xyz(),
                           mVSLocalDir);

        // Transform to object space.
        stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mWorldInvRotMatrix, mVSLocalDir, mVSLocalDir);

        // Transform to tangent space.
        if (mNormalMapSpace == NMS_TANGENT)
        {
            stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mVSTBNMatrix, mVSLocalDir, mVSOutView);
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
            stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mVSTBNMatrix, In(curLightParams->mDirection).xyz(),
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
        stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mWorldInvRotMatrix, mVSLocalDir, mVSLocalDir);

        // Transform to tangent space.      
        if (mNormalMapSpace == NMS_TANGENT)
        {
            stage.callFunction(SGX_FUNC_TRANSFORMNORMAL, mVSTBNMatrix, mVSLocalDir,
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
bool NormalMapLighting::addPSNormalFetchInvocation(Function* psMain, const int groupOrder)
{
    psMain->getStage(groupOrder)
        .callFunction(SGX_FUNC_FETCHNORMAL, mPSNormalMapSampler, mPSInTexcoord, mPSNormal);

    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder)
{
    auto stage = psMain->getStage(groupOrder);

    // Merge diffuse colour with vertex colour if need to.
    if (mTrackVertexColourType & TVC_DIFFUSE)           
    {
        stage.callFunction(FFP_FUNC_MODULATE, In(mInDiffuse).xyz(), In(curLightParams->mDiffuseColour).xyz(),
                           Out(curLightParams->mDiffuseColour).xyz());
    }

    // Merge specular colour with vertex colour if need to.
    if (mSpecularEnable && mTrackVertexColourType & TVC_SPECULAR)
    {
        stage.callFunction(FFP_FUNC_MODULATE, In(mInDiffuse).xyz(), In(curLightParams->mSpecularColour).xyz(),
                           Out(curLightParams->mSpecularColour).xyz());
    }

    FunctionInvocation* curFuncInvocation = NULL;
    switch (curLightParams->mType)
    {

    case Light::LT_DIRECTIONAL:         
        if (mSpecularEnable)
        {               
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mPSInView, Operand::OPS_IN);         
            curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);          
            curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);         
            curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutSpecular, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutSpecular, Operand::OPS_OUT, Operand::OPM_XYZ);
            psMain->addAtomInstance(curFuncInvocation);
        }

        else
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);                  
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
            psMain->addAtomInstance(curFuncInvocation); 
        }   
        break;

    case Light::LT_POINT:   
        if (mSpecularEnable)
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);         
            curFuncInvocation->pushOperand(mPSInView, Operand::OPS_IN); 
            curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);         
            curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutSpecular, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutSpecular, Operand::OPS_OUT, Operand::OPM_XYZ);
            psMain->addAtomInstance(curFuncInvocation);     
        }
        else
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);                     
            curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);                  
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
            psMain->addAtomInstance(curFuncInvocation); 
        }

        break;

    case Light::LT_SPOTLIGHT:
        if (mSpecularEnable)
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mPSInView, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);         
            curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutSpecular, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutSpecular, Operand::OPS_OUT, Operand::OPM_XYZ);
            psMain->addAtomInstance(curFuncInvocation);         
        }
        else
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_SPOT_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);                  
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mOutDiffuse, Operand::OPS_OUT, Operand::OPM_XYZ);
            psMain->addAtomInstance(curFuncInvocation); 
        }
        break;
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
