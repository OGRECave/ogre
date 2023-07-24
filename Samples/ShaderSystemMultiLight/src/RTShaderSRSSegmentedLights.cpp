#include "RTShaderSRSSegmentedLights.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreGpuProgram.h"
#include "OgrePass.h"
#include "OgreShaderGenerator.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreMaterialSerializer.h"
#include "SegmentedDynamicLightManager.h"

#define SL_LIB_PERPIXELLIGHTING                 "SegmentedPerPixelLighting"
#define SL_FUNC_TRANSFORMNORMAL                 "SL_TransformNormal"
#define SL_FUNC_TRANSFORMPOSITION                   "SL_TransformPosition"
#define SL_FUNC_LIGHT_DIRECTIONAL_DIFFUSE           "SL_Light_Directional_Diffuse"
#define SL_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR   "SL_Light_Directional_DiffuseSpecular"
#define SL_FUNC_LIGHT_AMBIENT_DIFFUSE               "SL_Light_Ambient_Diffuse"
#define SL_FUNC_LIGHT_SEGMENT_TEXTURE_AMBIENT_DIFFUSE   "SL_Light_Segment_Texture_Ambient_Diffuse"
#define SL_FUNC_LIGHT_SEGMENT_DEBUG             "SL_Light_Segment_Debug"

using namespace Ogre;
using namespace Ogre::RTShader;

String RTShaderSRSSegmentedLights::Type = "Segmented_PerPixelLighting";
Light RTShaderSRSSegmentedLights::msBlankLight;


//-----------------------------------------------------------------------
RTShaderSRSSegmentedLights::RTShaderSRSSegmentedLights()
{
    mTrackVertexColourType          = TVC_NONE; 
    mSpecularEnable                 = false;
    mUseSegmentedLightTexture       = false;
    mLightSamplerIndex = 0;

    msBlankLight.setDiffuseColour(ColourValue::Black);
    msBlankLight.setSpecularColour(ColourValue::Black);
    msBlankLight.setAttenuation(0,1,0,0);
}

//-----------------------------------------------------------------------
const String& RTShaderSRSSegmentedLights::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int RTShaderSRSSegmentedLights::getExecutionOrder() const
{
    return FFP_LIGHTING;
}

//-----------------------------------------------------------------------
void RTShaderSRSSegmentedLights::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source,
    const LightList* pLightList)
{
    if ((mLightParamsList.empty()) && (!mUseSegmentedLightTexture))
        return;

    Light::LightTypes curLightType = Light::LT_DIRECTIONAL; 
    unsigned int curSearchLightIndex = 0;

    //update spot strength
    float spotIntensity = 1;
    
    // Update per light parameters.
    for (auto & curParams : mLightParamsList)
    {
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

            // Update light direction.
            vParameter = srcLight->getAs4DVector(true);
            curParams.mDirection->setGpuParameter(vParameter.ptr(),3,1);
            break;

        case Light::LT_POINT:

            // Update light position.
            vParameter = srcLight->getAs4DVector(true);
            curParams.mPosition->setGpuParameter(vParameter.ptr(),3,1);

            // Update light attenuation parameters.
            curParams.mSpotParams->setGpuParameter(Ogre::Vector3(1 / srcLight->getAttenuationRange(),0,0));
            break;
        case Light::LT_RECTLIGHT:
        case Light::LT_SPOTLIGHT:
            {                       
                Ogre::Vector3 vec3;

                // Update light position.
                vParameter = srcLight->getAs4DVector(true);
                curParams.mPosition->setGpuParameter(vParameter.ptr(),3,1);


                // Update light direction.
                vec3 = source->getInverseTransposeWorldMatrix().linear() * srcLight->getDerivedDirection();
                vec3.normalise();

                vParameter.x = -vec3.x;
                vParameter.y = -vec3.y;
                vParameter.z = -vec3.z;
                vParameter.w = 0.0;
                curParams.mDirection->setGpuParameter(vParameter.ptr(),3,1);

                // Update spotlight parameters.
                Real phi   = Math::Cos(srcLight->getSpotlightOuterAngle().valueRadians() * 0.5f);
                Real theta = Math::Cos(srcLight->getSpotlightInnerAngle().valueRadians() * 0.5f);
                
                vec3.x = 1 / srcLight->getAttenuationRange();
                vec3.y = phi;
                vec3.z = 1 / (theta - phi);

                curParams.mSpotParams->setGpuParameter(vec3);
            }
            break;
        }

        float lightIntensity = 1;
        if (curParams.mType == Light::LT_SPOTLIGHT)
        {
            lightIntensity = spotIntensity;
        }

        // Update diffuse colour.
        colour = srcLight->getDiffuseColour() * lightIntensity;
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            colour = colour * pass->getDiffuse();
        }
        curParams.mDiffuseColour->setGpuParameter(colour.ptr(),3,1);    

        // Update specular colour if need to.
        if  ((mSpecularEnable) && (curParams.mType == Light::LT_DIRECTIONAL))
        {
            // Update diffuse colour.
            colour = srcLight->getSpecularColour() * lightIntensity;
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                colour = colour * pass->getSpecular();
            }
            curParams.mSpecularColour->setGpuParameter(colour.ptr(),3,1);       
        }                                                                           
    }

    if (mUseSegmentedLightTexture)
    {
        unsigned int indexStart = 0, indexEnd = 0;
        Ogre::Vector4 lightBounds; 
        SegmentedDynamicLightManager::getSingleton().getLightListRange(rend, lightBounds, indexStart, indexEnd);
        mPSLightTextureIndexLimit->setGpuParameter(Ogre::Vector2((Ogre::Real)indexStart, (Ogre::Real)indexEnd));
        mPSLightTextureLightBounds->setGpuParameter(lightBounds);

        Ogre::TextureUnitState* pLightTexture = pass->getTextureUnitState(mLightSamplerIndex);
        const Ogre::String& textureName = SegmentedDynamicLightManager::getSingleton().getSDLTextureName();
        if (textureName != pLightTexture->getTextureName())
        {
            pLightTexture->setTextureName(textureName, Ogre::TEX_TYPE_2D);
        }
    }
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::resolveParameters(ProgramSet* programSet)
{
    if (false == resolveGlobalParameters(programSet))
        return false;

    if (false == resolvePerLightParameters(programSet))
        return false;

    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::resolveGlobalParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();


    // Resolve world IT matrix.
    mWorldITMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);

    // Get surface ambient colour if need to.
    if ((mTrackVertexColourType & TVC_AMBIENT) == 0)
    {       
        mDerivedAmbientLightColour = psProgram->resolveParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR);
    }
    else
    {
        mLightAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
        mSurfaceAmbientColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
    }

    // Get surface diffuse colour if need to.
    if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
    {
        mSurfaceDiffuseColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
    }

    // Get surface specular colour if need to.
    if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
    {
        mSurfaceSpecularColour = psProgram->resolveParameter(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
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

    
    //Check if another SRS already defined a normal in world space to be used
    mPSLocalNormal = psMain->getLocalParameter(Parameter::SPC_NORMAL_WORLD_SPACE);
    if (mPSLocalNormal.get() == NULL)
    {
        //create parameters to fetch the normal from the vertex shader
        
        // Resolve input vertex shader normal.
        mVSInNormal = vsMain->resolveInputParameter(Parameter::SPC_NORMAL_OBJECT_SPACE);
        // Resolve output vertex shader normal.
        mVSOutNormal = vsMain->resolveOutputParameter(Parameter::SPC_NORMAL_WORLD_SPACE);
        // Resolve input pixel shader normal.
        mPSInNormal = psMain->resolveInputParameter(mVSOutNormal);
        mPSLocalNormal = psMain->resolveLocalParameter(Parameter::SPC_NORMAL_WORLD_SPACE);
    }
    
    mPSDiffuse = psMain->getInputParameter(Parameter::SPC_COLOR_DIFFUSE);
    if (mPSDiffuse.get() == NULL)
    {
        mPSDiffuse = psMain->getLocalParameter(Parameter::SPC_COLOR_DIFFUSE);
        if (mPSDiffuse.get() == NULL)
            return false;
    }

    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    mPSTempDiffuseColour = psMain->resolveLocalParameter(GCT_FLOAT4, "lPerPixelDiffuse");
    mVSOutWorldPos = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_WORLD_SPACE);
    mPSInWorldPos = psMain->resolveInputParameter(mVSOutWorldPos);
    mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
    mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);


    if (mSpecularEnable)
    {
        mPSSpecular = psMain->getInputParameter(Parameter::SPC_COLOR_SPECULAR);
        if (mPSSpecular.get() == NULL)
        {
            mPSSpecular = psMain->getLocalParameter(Parameter::SPC_COLOR_SPECULAR);
            if (mPSSpecular.get() == NULL)
                return false;
        }

        mPSTempSpecularColour = psMain->resolveLocalParameter(GCT_FLOAT4, "lPerPixelSpecular");
        mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);
        mWorldMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLD_MATRIX);
    }

    
    if (mUseSegmentedLightTexture)
    {
        mPSLightTextureIndexLimit = psProgram->resolveParameter(GCT_FLOAT2, -1, (uint16)GPV_PER_OBJECT, "LightTextureIndexLimits");
        mPSLightTextureLightBounds = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_PER_OBJECT, "LightTextureBounds");
        mPSSegmentedLightTexture = psProgram->resolveParameter(Ogre::GCT_SAMPLER2D, mLightSamplerIndex, (Ogre::uint16)Ogre::GPV_GLOBAL, "segmentedLightTexture");
    }

    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::resolvePerLightParameters(ProgramSet* programSet)
{
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    // Resolve per light parameters.
    for (auto & i : mLightParamsList)
    {       
        switch (i.mType)
        {
        case Light::LT_RECTLIGHT:
        case Light::LT_DIRECTIONAL:
            i.mDirection = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "light_direction_space");
            break;

        case Light::LT_POINT:
        case Light::LT_SPOTLIGHT:
            i.mPosition = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "light_position_space");
            i.mDirection = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "light_direction_space");
            i.mSpotParams = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "spotlight_params");
            break;
        }

        // Resolve diffuse colour.
        if ((mTrackVertexColourType & TVC_DIFFUSE) == 0)
        {
            i.mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_diffuse");
        }
        else
        {
            i.mDiffuseColour = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "light_diffuse");
        }   

        if ((mSpecularEnable) && (i.mType == Light::LT_DIRECTIONAL))
        {
            // Resolve specular colour.
            if ((mTrackVertexColourType & TVC_SPECULAR) == 0)
            {
                i.mSpecularColour = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS | (uint16)GPV_GLOBAL, "derived_light_specular");
            }
            else
            {
                i.mSpecularColour = psProgram->resolveParameter(GCT_FLOAT3, -1, (uint16)GPV_LIGHTS, "light_specular");
            }                       
        }       

    }

    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(SL_LIB_PERPIXELLIGHTING);

    psProgram->addDependency(SL_LIB_PERPIXELLIGHTING);

    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();  

    // Add the global illumination functions.
    if (false == addVSInvocation(vsMain, FFP_VS_LIGHTING))
        return false;

    // Add the global illumination functions.
    if (false == addPSGlobalIlluminationInvocationBegin(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    // Add per light functions.
    for (auto & i : mLightParamsList)
    {       
        if (false == addPSIlluminationInvocation(&i, psMain, FFP_PS_COLOUR_BEGIN + 1))
            return false;
    }

    if (mUseSegmentedLightTexture)
    {
        addPSSegmentedTextureLightInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1);
    }


    // Add the global illumination functions.
    if (false == addPSGlobalIlluminationInvocationEnd(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    // Assign back temporary variables to the ps diffuse and specular components.
    if (false == addPSFinalAssignmentInvocation(psMain, FFP_PS_COLOUR_BEGIN + 1))
        return false;


    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::addVSInvocation(Function* vsMain, const int groupOrder)
{
    FunctionInvocation* curFuncInvocation = NULL;

    if (mVSInNormal.get() != NULL)
    {
        // Transform normal in world space.
        curFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_TRANSFORMNORMAL, groupOrder);
        curFuncInvocation->pushOperand(mWorldITMatrix, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSInNormal, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSOutNormal, Operand::OPS_OUT); 
        vsMain->addAtomInstance(curFuncInvocation);
    }

    // Transform world space position if need to.
    if (mVSOutWorldPos.get() != NULL)
    {
        curFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_TRANSFORMPOSITION, groupOrder);
        curFuncInvocation->pushOperand(mWorldMatrix, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mVSOutWorldPos, Operand::OPS_OUT);   
        vsMain->addAtomInstance(curFuncInvocation);
    }


    return true;
}


//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::addPSGlobalIlluminationInvocationBegin(Function* psMain, const int groupOrder)
{
    FunctionAtom* curFuncInvocation = NULL;

    if (mPSInNormal.get())
    {
        curFuncInvocation = OGRE_NEW AssignmentAtom(FFP_PS_PRE_PROCESS + 1);
        curFuncInvocation->pushOperand(mPSInNormal, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mPSLocalNormal, Operand::OPS_OUT);   
        psMain->addAtomInstance(curFuncInvocation);
    }

    //alpha channel is controlled by the diffuse value
    if (mTrackVertexColourType & TVC_DIFFUSE)
    {
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_W);
        curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_W); 
        psMain->addAtomInstance(curFuncInvocation);
    }
    else
    {
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(mDerivedSceneColour, Operand::OPS_IN, Operand::OPM_W);
        curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_W); 
        psMain->addAtomInstance(curFuncInvocation);     
    }

    ParameterPtr pZeroParam = ParameterFactory::createConstParam(Ogre::Vector3::ZERO);
    
    curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
    curFuncInvocation->pushOperand(pZeroParam, Operand::OPS_IN);    
    curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);
    psMain->addAtomInstance(curFuncInvocation); 

    if (mSpecularEnable)
    {
        curFuncInvocation = OGRE_NEW AssignmentAtom(groupOrder);
        curFuncInvocation->pushOperand(pZeroParam, Operand::OPS_IN);    
        curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);
        psMain->addAtomInstance(curFuncInvocation); 
    }
    
    return true;
}



//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::addPSGlobalIlluminationInvocationEnd(Function* psMain, const int groupOrder)
{
    FunctionAtom* curFuncInvocation = NULL;

    // Merge diffuse colour with vertex colour if need to.
    if (mTrackVertexColourType & TVC_DIFFUSE)           
    {
        curFuncInvocation = OGRE_NEW BinaryOpAtom('*', groupOrder);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);  
        curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);
        psMain->addAtomInstance(curFuncInvocation);
    }

    // Merge specular colour with vertex colour if need to.
    if ((mSpecularEnable == true) && (mTrackVertexColourType & TVC_SPECULAR))
    {                           
        curFuncInvocation = OGRE_NEW BinaryOpAtom('*', groupOrder);
        curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);  
        curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);
        curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);
        psMain->addAtomInstance(curFuncInvocation);
    }


    if ((mTrackVertexColourType & TVC_AMBIENT) == 0 && 
        (mTrackVertexColourType & TVC_EMISSIVE) == 0)
    {
        curFuncInvocation = OGRE_NEW BinaryOpAtom('+', groupOrder);
        curFuncInvocation->pushOperand(mDerivedSceneColour, Operand::OPS_IN, (Operand::OPM_XYZ));
        curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, (Operand::OPM_XYZ));  
        curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
        psMain->addAtomInstance(curFuncInvocation);     
    }
    else
    {
        if (mTrackVertexColourType & TVC_AMBIENT)
        {
            curFuncInvocation = OGRE_NEW BinaryOpAtom('*', groupOrder);
            curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);          
            curFuncInvocation->pushOperand(mLightAmbientColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mLightAmbientColour, Operand::OPS_OUT, Operand::OPM_XYZ);    
            psMain->addAtomInstance(curFuncInvocation);
        
            curFuncInvocation = OGRE_NEW BinaryOpAtom('+', groupOrder);
            curFuncInvocation->pushOperand(mLightAmbientColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);            
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            psMain->addAtomInstance(curFuncInvocation);
        }
        else
        {
            curFuncInvocation = OGRE_NEW BinaryOpAtom('+', groupOrder);
            curFuncInvocation->pushOperand(mDerivedAmbientLightColour, Operand::OPS_IN, Operand::OPM_XYZ);  
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            psMain->addAtomInstance(curFuncInvocation);
        }

        if (mTrackVertexColourType & TVC_EMISSIVE)
        {
            curFuncInvocation = OGRE_NEW BinaryOpAtom('+', groupOrder);
            curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            psMain->addAtomInstance(curFuncInvocation);
        }
        else
        {
            curFuncInvocation = OGRE_NEW BinaryOpAtom('+', groupOrder);
            curFuncInvocation->pushOperand(mSurfaceEmissiveColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            psMain->addAtomInstance(curFuncInvocation);
        }       
    }

    if (mSpecularEnable)
    {
        curFuncInvocation = OGRE_NEW BinaryOpAtom('+', groupOrder);
        curFuncInvocation->pushOperand(mPSSpecular, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN); 
        curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT);    
        psMain->addAtomInstance(curFuncInvocation); 
    }

    return true;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder)
{   
    FunctionInvocation* curFuncInvocation = NULL;   

    
    switch (curLightParams->mType)
    {
    case Light::LT_RECTLIGHT:
    case Light::LT_DIRECTIONAL:         
        if (mSpecularEnable)
        {               
            curFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR, groupOrder);
            curFuncInvocation->pushOperand(mPSLocalNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mPSInWorldPos, Operand::OPS_IN);         
            curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN);            
            curFuncInvocation->pushOperand(curLightParams->mSpecularColour, Operand::OPS_IN);           
            curFuncInvocation->pushOperand(mSurfaceShininess, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN, Operand::OPM_XYZ);
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_OUT, Operand::OPM_XYZ);  
            psMain->addAtomInstance(curFuncInvocation);
        }

        else
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSLocalNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN);                    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN, Operand::OPM_XYZ);    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_OUT, Operand::OPM_XYZ);   
            psMain->addAtomInstance(curFuncInvocation); 
        }   
        break;

    case Light::LT_POINT:   
    case Light::LT_SPOTLIGHT:
        {
            curFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_LIGHT_AMBIENT_DIFFUSE, groupOrder);
            curFuncInvocation->pushOperand(mPSLocalNormal, Operand::OPS_IN);
            curFuncInvocation->pushOperand(mPSInWorldPos, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mPosition, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDirection, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mSpotParams, Operand::OPS_IN);
            curFuncInvocation->pushOperand(curLightParams->mDiffuseColour, Operand::OPS_IN);                    
            curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_INOUT, Operand::OPM_XYZ); 
            psMain->addAtomInstance(curFuncInvocation); 
        }
        break;
    }

    return true;
}

bool RTShaderSRSSegmentedLights::addPSSegmentedTextureLightInvocation(Function* psMain, const int groupOrder)
{
    float invWidth = 1.0f / (float)SegmentedDynamicLightManager::getSingleton().getTextureWidth();
    float invHeight = 1.0f / (float)SegmentedDynamicLightManager::getSingleton().getTextureHeight();
    ParameterPtr paramInvWidth = ParameterFactory::createConstParam(invWidth);
    ParameterPtr paramInvHeight = ParameterFactory::createConstParam(invHeight);

    FunctionInvocation* curFuncInvocation = NULL;   
    curFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_LIGHT_SEGMENT_TEXTURE_AMBIENT_DIFFUSE, groupOrder);
    curFuncInvocation->pushOperand(mPSLocalNormal, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mPSInWorldPos, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mPSSegmentedLightTexture, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mPSLightTextureIndexLimit, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mPSLightTextureLightBounds, Operand::OPS_IN);
    curFuncInvocation->pushOperand(paramInvWidth, Operand::OPS_IN);
    curFuncInvocation->pushOperand(paramInvHeight, Operand::OPS_IN);                    
    curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_INOUT, Operand::OPM_XYZ); 
    psMain->addAtomInstance(curFuncInvocation); 

    if (SegmentedDynamicLightManager::getSingleton().isDebugMode())
    {
        ParameterPtr psOutColor = psMain->resolveOutputParameter(Parameter::SPS_COLOR, -1, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);

        FunctionInvocation* curDebugFuncInvocation = NULL;
        curDebugFuncInvocation = OGRE_NEW FunctionInvocation(SL_FUNC_LIGHT_SEGMENT_DEBUG, FFP_PS_COLOUR_END + 1);
        curDebugFuncInvocation->pushOperand(mPSLocalNormal, Operand::OPS_IN);
        curDebugFuncInvocation->pushOperand(mPSInWorldPos, Operand::OPS_IN);
        curDebugFuncInvocation->pushOperand(mPSSegmentedLightTexture, Operand::OPS_IN);
        curDebugFuncInvocation->pushOperand(mPSLightTextureIndexLimit, Operand::OPS_IN);
        curDebugFuncInvocation->pushOperand(mPSLightTextureLightBounds, Operand::OPS_IN);
        curDebugFuncInvocation->pushOperand(paramInvWidth, Operand::OPS_IN);
        curDebugFuncInvocation->pushOperand(paramInvHeight, Operand::OPS_IN);   

        curDebugFuncInvocation->pushOperand(psOutColor, Operand::OPS_INOUT, Operand::OPM_XYZ);  
        psMain->addAtomInstance(curDebugFuncInvocation);    
    }

    return true;
}


//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::addPSFinalAssignmentInvocation( Function* psMain, const int groupOrder)
{
    FunctionAtom* curFuncInvocation;

    curFuncInvocation = OGRE_NEW AssignmentAtom(FFP_PS_COLOUR_BEGIN + 1);
    curFuncInvocation->pushOperand(mPSTempDiffuseColour, Operand::OPS_IN);  
    curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_OUT);   
    psMain->addAtomInstance(curFuncInvocation); 

    curFuncInvocation = OGRE_NEW AssignmentAtom(FFP_PS_COLOUR_BEGIN + 1);
    curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);    
    curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT);    
    psMain->addAtomInstance(curFuncInvocation);

    if (mSpecularEnable)
    {
        curFuncInvocation = OGRE_NEW AssignmentAtom(FFP_PS_COLOUR_BEGIN + 1);
        curFuncInvocation->pushOperand(mPSTempSpecularColour, Operand::OPS_IN);
        curFuncInvocation->pushOperand(mPSSpecular, Operand::OPS_OUT);          
        psMain->addAtomInstance(curFuncInvocation); 
    }

    return true;
}


//-----------------------------------------------------------------------
void RTShaderSRSSegmentedLights::copyFrom(const SubRenderState& rhs)
{
    const RTShaderSRSSegmentedLights& rhsLighting = static_cast<const RTShaderSRSSegmentedLights&>(rhs);

    mUseSegmentedLightTexture = rhsLighting.mUseSegmentedLightTexture;
    mLightParamsList = rhsLighting.mLightParamsList;
}

//-----------------------------------------------------------------------
bool RTShaderSRSSegmentedLights::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    if (srcPass->getLightingEnabled() == false)
        return false;

    mUseSegmentedLightTexture = SegmentedDynamicLightManager::getSingleton().isActive();
    setTrackVertexColourType(srcPass->getVertexColourTracking());           

    if (srcPass->getShininess() > 0.0 &&
        srcPass->getSpecular() != ColourValue::Black)
    {
        setSpecularEnable(true);
    }
    else
    {
        setSpecularEnable(false);   
    }

    setLightCount(renderState->getLightCount());

    if (mUseSegmentedLightTexture)
    {
        const_cast<RenderState*>(renderState)->setLightCountAutoUpdate(false);

        Ogre::TextureUnitState* pLightTexture = dstPass->createTextureUnitState();
        pLightTexture->setTextureName(SegmentedDynamicLightManager::getSingleton().getSDLTextureName(), Ogre::TEX_TYPE_2D);
        pLightTexture->setTextureFiltering(Ogre::TFO_NONE);
        mLightSamplerIndex = dstPass->getNumTextureUnitStates() - 1;
    }


    return true;
}

//-----------------------------------------------------------------------
void RTShaderSRSSegmentedLights::setLightCount(int lightCount)
{
    mLightParamsList.clear();
    //Set always to have one single directional lights
    LightParams curParams;
    curParams.mType = Light::LT_DIRECTIONAL;
    mLightParamsList.push_back(curParams);

    curParams.mType = Light::LT_POINT;
    for (int i=0; i < lightCount; ++i)
    {
        if ((!mUseSegmentedLightTexture) || (curParams.mType == Light::LT_DIRECTIONAL))
        {
            mLightParamsList.push_back(curParams);
        }
    }
}

//-----------------------------------------------------------------------
const String& RTShaderSRSSegmentedLightsFactory::getType() const
{
    return RTShaderSRSSegmentedLights::Type;
}

//-----------------------------------------------------------------------
SubRenderState* RTShaderSRSSegmentedLightsFactory::createInstance(ScriptCompiler* compiler, 
        PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "lighting_stage")
    {
        if(prop->values.size() == 1)
        {
            if (prop->values.front()->getString() == "per_pixel")
            {
                return createOrRetrieveInstance(translator);
            }
        }       
    }

    return NULL;
}

//-----------------------------------------------------------------------
void RTShaderSRSSegmentedLightsFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
    Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "lighting_stage");
    ser->writeValue("per_pixel");
}

//-----------------------------------------------------------------------
SubRenderState* RTShaderSRSSegmentedLightsFactory::createInstanceImpl()
{
    return OGRE_NEW RTShaderSRSSegmentedLights;
}

