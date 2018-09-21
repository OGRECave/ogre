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

        Light*      srcLight = NULL;
        Vector4     vParameter;
        ColourValue colour;

        // Search a matching light from the current sorted lights of the given renderable.
        for (unsigned int j = curSearchLightIndex; j < pLightList->size(); ++j)
        {
            if (pLightList->at(j)->getType() == curLightType)
            {               
                srcLight = pLightList->at(j);
                curSearchLightIndex = j + 1;
                break;
            }           
        }

        // No matching light found -> use a blank dummy light for parameter update.
        if (srcLight == NULL)
        {                       
            srcLight = &msBlankLight;
        }


        switch (curParams.mType)
        {
        case Light::LT_DIRECTIONAL:                                 
            {                       
                Vector3 vec3;                           

                // Update light direction. (Object space).
                vec3 = matWorldInvRotation * srcLight->getDerivedDirection();               
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
            vParameter = srcLight->getAs4DVector(true);
            curParams.mPosition->setGpuParameter(vParameter);

            // Update light attenuation parameters.
            vParameter.x = srcLight->getAttenuationRange();
            vParameter.y = srcLight->getAttenuationConstant();
            vParameter.z = srcLight->getAttenuationLinear();
            vParameter.w = srcLight->getAttenuationQuadric();
            curParams.mAttenuatParams->setGpuParameter(vParameter);
            break;

        case Light::LT_SPOTLIGHT:
            {                       
                Vector3 vec3;               
                                            
                // Update light position. (World space).                
                vParameter = srcLight->getAs4DVector(true);
                curParams.mPosition->setGpuParameter(vParameter);

                            
                // Update light direction. (Object space).
                vec3 = matWorldInvRotation * srcLight->getDerivedDirection();               
                vec3.normalise();
            
                vParameter.x = -vec3.x;
                vParameter.y = -vec3.y;
                vParameter.z = -vec3.z;
                vParameter.w = 0.0;
                curParams.mDirection->setGpuParameter(vParameter);                          
                
                // Update light attenuation parameters.
                vParameter.x = srcLight->getAttenuationRange();
                vParameter.y = srcLight->getAttenuationConstant();
                vParameter.z = srcLight->getAttenuationLinear();
                vParameter.w = srcLight->getAttenuationQuadric();
                curParams.mAttenuatParams->setGpuParameter(vParameter);

                // Update spotlight parameters.
                Real phi   = Math::Cos(srcLight->getSpotlightOuterAngle().valueRadians() * 0.5f);
                Real theta = Math::Cos(srcLight->getSpotlightInnerAngle().valueRadians() * 0.5f);

                vec3.x = theta;
                vec3.y = phi;
                vec3.z = srcLight->getSpotlightFalloff();

                curParams.mSpotParams->setGpuParameter(vec3);
            }
            break;
        }


        // Update diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            colour = srcLight->getDiffuseColour() * pass->getDiffuse() * srcLight->getPowerScale();
            curParams.mDiffuseColour->setGpuParameter(colour);                  
        }
        else
        {                   
            colour = srcLight->getDiffuseColour() * srcLight->getPowerScale();
            curParams.mDiffuseColour->setGpuParameter(colour);  
        }

        // Update specular colour if need to.
        if (mSpecularEnable)
        {
            // Update diffuse colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                colour = srcLight->getSpecularColour() * pass->getSpecular() * srcLight->getPowerScale();
                curParams.mSpecularColour->setGpuParameter(colour);                 
            }
            else
            {                   
                colour = srcLight->getSpecularColour() * srcLight->getPowerScale();
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
        mDerivedAmbientLightColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR, 0);
        hasError |= !(mDerivedAmbientLightColour.get());
    }
    else
    {
        mLightAmbientColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR, 0);
        mSurfaceAmbientColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR, 0);
    
        hasError |= !(mDerivedAmbientLightColour.get()) || !(mLightAmbientColour.get());
    }

    // Get surface diffuse colour if need to.
    if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
    {
        mSurfaceDiffuseColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR, 0);
        hasError |= !(mSurfaceDiffuseColour.get());
    }

    // Get surface specular colour if need to.
    if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
    {
        mSurfaceSpecularColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR, 0);
        hasError |= !(mSurfaceSpecularColour.get());
    }


    // Get surface emissive colour if need to.
    if ((mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        mSurfaceEmissiveColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR, 0);
        hasError |= !(mSurfaceEmissiveColour.get());
    }

    // Get derived scene colour.
    mDerivedSceneColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR, 0);

    // Get surface shininess.
    mSurfaceShininess = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_SURFACE_SHININESS, 0);

    // Resolve input vertex shader normal.
    mVSInNormal = vsMain->resolveInputParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT3);

    // Resolve input vertex shader tangent.
    if (mNormalMapSpace == NMS_TANGENT)
    {
        mVSInTangent = vsMain->resolveInputParameter(Parameter::SPS_TANGENT, 0, Parameter::SPC_TANGENT_OBJECT_SPACE, GCT_FLOAT3);
        
        // Resolve local vertex shader TNB matrix.
        mVSTBNMatrix = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lMatTBN", GCT_MATRIX_3X3);
        
        hasError |= !(mVSTBNMatrix.get()) || !(mVSInTangent.get());
    }
    
    // Resolve input vertex shader texture coordinates.
    mVSInTexcoord = vsMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSTexCoordSetIndex, 
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex),
        GCT_FLOAT2);

    // Resolve output vertex shader texture coordinates.
    mVSOutTexcoord = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
        Parameter::Content(Parameter::SPC_TEXTURE_COORDINATE0 + mVSTexCoordSetIndex),
        GCT_FLOAT2);
    
    // Resolve pixel input texture coordinates normal.
    mPSInTexcoord = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
        mVSOutTexcoord->getIndex(), 
        mVSOutTexcoord->getContent(),
        mVSOutTexcoord->getType());

    // Resolve pixel shader normal.
    if (mNormalMapSpace == NMS_OBJECT)
    {
        mPSNormal = psMain->resolveLocalParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_OBJECT_SPACE, GCT_FLOAT4);
        hasError |= !(mPSNormal.get());
    }
    else if (mNormalMapSpace == NMS_TANGENT)
    {
        mPSNormal = psMain->resolveLocalParameter(Parameter::SPS_NORMAL, 0, Parameter::SPC_NORMAL_TANGENT_SPACE, GCT_FLOAT4);
        hasError |= !(mPSNormal.get());
    }
    

    const ShaderParameterList& inputParams = psMain->getInputParameters();
    const ShaderParameterList& localParams = psMain->getLocalParameters();

    mPSDiffuse = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    if (mPSDiffuse.get() == NULL)
    {
        mPSDiffuse = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    }

    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    mPSTempDiffuseColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapDiffuse", GCT_FLOAT4);

    if (mSpecularEnable)
    {
        mPSSpecular = psMain->getParameterByContent(inputParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
        if (mPSSpecular.get() == NULL)
        {
            mPSSpecular = psMain->getParameterByContent(localParams, Parameter::SPC_COLOR_SPECULAR, GCT_FLOAT4);
        }

        mPSTempSpecularColour = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapSpecular", GCT_FLOAT4);
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);

        if (mNormalMapSpace == NMS_TANGENT)
        {
            mVSOutView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
                Parameter::SPC_POSTOCAMERA_TANGENT_SPACE, GCT_FLOAT3);
            hasError |= !(mVSOutView.get());
        }
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            mVSOutView = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
                Parameter::SPC_POSTOCAMERA_OBJECT_SPACE, GCT_FLOAT3);
            hasError |= !(mVSOutView.get());
        }
        

        mPSInView = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
            mVSOutView->getIndex(), 
            mVSOutView->getContent(),
            mVSOutView->getType());

        // Resolve camera position world space.
        mCamPosWorldSpace = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_CAMERA_POSITION, 0);
        mVSLocalDir = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapTempDir", GCT_FLOAT3);
        mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
        // Resolve world matrix.                
        mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);

        // Resolve inverse world rotation matrix.
        mWorldInvRotMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "inv_world_rotation_matrix");

        hasError |= !(mPSSpecular.get()) || !(mPSTempSpecularColour.get()) || !(mVSInPosition.get()) || !(mPSInView.get()) || !(mCamPosWorldSpace.get()) || 
            !(mVSLocalDir.get()) || !(mVSWorldPosition.get()) || !(mWorldMatrix.get()) || !(mWorldInvRotMatrix.get());
    }

    hasError |= !(mPSNormalMapSampler.get()) || !(mDerivedSceneColour.get()) || !(mSurfaceShininess.get()) || !(mVSInNormal.get()) || 
        !(mVSInTexcoord.get()) || !(mVSOutTexcoord.get()) || !(mPSInTexcoord.get()) || !(mPSDiffuse.get()) || !(mPSOutDiffuse.get()) ||
        !(mPSTempDiffuseColour.get());

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
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0 + i),
                    GCT_FLOAT3);
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0 + i),
                    GCT_FLOAT3);
            }

            mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
                mLightParamsList[i].mVSOutDirection->getIndex(), 
                mLightParamsList[i].mVSOutDirection->getContent(), 
                mLightParamsList[i].mVSOutDirection->getType());

            hasError = !(mLightParamsList[i].mDirection.get()) || !(mLightParamsList[i].mVSOutDirection.get()) || !(mLightParamsList[i].mPSInDirection.get());
            break;

        case Light::LT_POINT:       
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);

            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_position_world_space");

            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0 + i),
                    GCT_FLOAT3);
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0 + i),
                    GCT_FLOAT3);
            }
            
            mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
                mLightParamsList[i].mVSOutToLightDir->getIndex(), 
                mLightParamsList[i].mVSOutToLightDir->getContent(), 
                mLightParamsList[i].mVSOutToLightDir->getType());

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");

            // Resolve local dir.
            if (mVSLocalDir.get() == NULL)
            {
                mVSLocalDir = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapTempDir", GCT_FLOAT3);
            }   

            // Resolve world position.
            if (mVSWorldPosition.get() == NULL)
            {
                mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
            }   

            // Resolve world matrix.
            if (mWorldMatrix.get() == NULL)
            {               
                mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
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
            mVSInPosition = vsMain->resolveInputParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_OBJECT_SPACE, GCT_FLOAT4);
            mLightParamsList[i].mPosition = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_position_world_space");
            
            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_TANGENT_SPACE0 + i),
                    GCT_FLOAT3);
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutToLightDir = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
                    Parameter::Content(Parameter::SPC_POSTOLIGHT_OBJECT_SPACE0 + i),
                    GCT_FLOAT3);
            }
            
            mLightParamsList[i].mPSInToLightDir = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
                mLightParamsList[i].mVSOutToLightDir->getIndex(), 
                mLightParamsList[i].mVSOutToLightDir->getContent(), 
                mLightParamsList[i].mVSOutToLightDir->getType());

            mLightParamsList[i].mDirection = vsProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS|GPV_PER_OBJECT, "light_direction_obj_space");

            if (mNormalMapSpace == NMS_TANGENT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_TANGENT_SPACE0 + i),
                    GCT_FLOAT3);
            }
            else if (mNormalMapSpace == NMS_OBJECT)
            {
                mLightParamsList[i].mVSOutDirection = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
                    Parameter::Content(Parameter::SPC_LIGHTDIRECTION_OBJECT_SPACE0 + i),
                    GCT_FLOAT3);
            }
                        
            mLightParamsList[i].mPSInDirection = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
                mLightParamsList[i].mVSOutDirection->getIndex(), 
                mLightParamsList[i].mVSOutDirection->getContent(), 
                mLightParamsList[i].mVSOutDirection->getType());

            mLightParamsList[i].mAttenuatParams = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_LIGHTS, "light_attenuation");

            mLightParamsList[i].mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
            
            // Resolve local dir.
            if (mVSLocalDir.get() == NULL)
            {
                mVSLocalDir = vsMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lNormalMapTempDir", GCT_FLOAT3);
            }   

            // Resolve world position.
            if (mVSWorldPosition.get() == NULL)
            {
                mVSWorldPosition = vsMain->resolveLocalParameter(Parameter::SPS_POSITION, 0, Parameter::SPC_POSITION_WORLD_SPACE, GCT_FLOAT3);
            }   

            // Resolve world matrix.
            if (mWorldMatrix.get() == NULL)
            {               
                mWorldMatrix = vsProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_WORLD_MATRIX, 0);
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
    FunctionInvocation* curFuncInvocation = NULL;

    // Construct TNB matrix.
    if (mNormalMapSpace == NMS_TANGENT)
    {
        curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_CONSTRUCT_TBNMATRIX, groupOrder);
        curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSInTangent, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_OUT); 
        vsMain->addAtomInstance(curFuncInvocation);
    }
    

    // Output texture coordinates.
    curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
    curFuncInvocation->pushOperand(mVSInTexcoord, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mVSOutTexcoord, Operand::OPS_OUT);   
    vsMain->addAtomInstance(curFuncInvocation);

    // Compute world space position.
    if (mVSWorldPosition.get() != NULL)
    {
        curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMPOSITION, groupOrder);
        curFuncInvocation->pushOperand(mWorldMatrix, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN); 
        curFuncInvocation->pushOperand(mVSWorldPosition, Operand::OPS_OUT); 
        vsMain->addAtomInstance(curFuncInvocation);
    }
    

    // Compute view vector.
    if (mVSInPosition.get() != NULL && 
        mVSOutView.get() != NULL)
    {   
        // View vector in world space.
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder);
        curFuncInvocation->pushOperand(mCamPosWorldSpace, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(mVSWorldPosition, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);  
        vsMain->addAtomInstance(curFuncInvocation);

        // Transform to object space.
        curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder);
        curFuncInvocation->pushOperand(mWorldInvRotMatrix, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);   
        curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);  
        vsMain->addAtomInstance(curFuncInvocation);

        // Transform to tangent space.
        if (mNormalMapSpace == NMS_TANGENT)
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder);
            curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mVSOutView, Operand::OPS_OUT);   
            vsMain->addAtomInstance(curFuncInvocation);
        }

        // Output object space.
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
            curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mVSOutView, Operand::OPS_OUT);                   
            vsMain->addAtomInstance(curFuncInvocation);
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
    FunctionInvocation* curFuncInvocation = NULL;

    // Compute light direction in texture space.
    if (curLightParams->mDirection.get() != NULL &&
        curLightParams->mVSOutDirection.get() != NULL)
    {
        // Transform to texture space.
        if (mNormalMapSpace == NMS_TANGENT)
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder);
            curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mVSOutDirection, Operand::OPS_OUT);  
            vsMain->addAtomInstance(curFuncInvocation);
        }
        // Output object space.
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
            curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mVSOutDirection, Operand::OPS_OUT);                  
            vsMain->addAtomInstance(curFuncInvocation);
        }
    }
    
    // Transform light vector to target space..
    if (curLightParams->mPosition.get() != NULL &&
        curLightParams->mVSOutToLightDir.get() != NULL)
    {
        // Compute light vector.
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_SUBTRACT, groupOrder);
        curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(mVSWorldPosition, Operand::OPS_IN);  
        curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);  
        vsMain->addAtomInstance(curFuncInvocation);

        // Transform to object space.
        curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder);
        curFuncInvocation->pushOperand(mWorldInvRotMatrix, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);   
        curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_OUT);  
        vsMain->addAtomInstance(curFuncInvocation);

        // Transform to tangent space.      
        if (mNormalMapSpace == NMS_TANGENT)
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_TRANSFORMNORMAL, groupOrder);
            curFuncInvocation->pushOperand(mVSTBNMatrix, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mVSOutToLightDir, Operand::OPS_OUT); 
            vsMain->addAtomInstance(curFuncInvocation);
        }
        
        // Output object space.
        else if (mNormalMapSpace == NMS_OBJECT)
        {
            curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
            curFuncInvocation->pushOperand(mVSLocalDir, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mVSOutToLightDir, Operand::OPS_OUT);                 
            vsMain->addAtomInstance(curFuncInvocation);
        }
    }


    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addPSNormalFetchInvocation(Function* psMain, const int groupOrder)
{
    FunctionInvocation* curFuncInvocation = NULL;
    curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_FETCHNORMAL, groupOrder);
	curFuncInvocation->pushOperand(mPSNormalMapSampler, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mPSInTexcoord, Operand::OPS_IN);
	curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_OUT);	
    psMain->addAtomInstance(curFuncInvocation);     

    return true;
}

//-----------------------------------------------------------------------
bool NormalMapLighting::addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder)
{   
    FunctionInvocation* curFuncInvocation = NULL;   

    // Merge diffuse colour with vertex colour if need to.
    if (mTrackVertexColourType & TVC_DIFFUSE)           
    {
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);  
        curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);
        psMain->addAtomInstance(curFuncInvocation);
    }

    // Merge specular colour with vertex colour if need to.
    if (mSpecularEnable && mTrackVertexColourType & TVC_SPECULAR)
    {                           
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_MODULATE, groupOrder);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);  
        curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);
        psMain->addAtomInstance(curFuncInvocation);
    }

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
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);  
            psMain->addAtomInstance(curFuncInvocation);
        }

        else
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mPSInDirection, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);                  
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
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
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);  
            psMain->addAtomInstance(curFuncInvocation);     
        }
        else
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_LIGHT_POINT_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSNormal, Operand::OPS_IN);                     
            curFuncInvocation->pushOperand(curLightParams->mPSInToLightDir, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(curLightParams->mAttenuatParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);                  
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
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
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);  
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
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
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
