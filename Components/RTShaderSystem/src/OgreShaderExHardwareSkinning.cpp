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

#define HS_DATA_BIND_NAME "HS_SRS_DATA"


namespace Ogre {
template<> RTShader::HardwareSkinningFactory* Singleton<RTShader::HardwareSkinningFactory>::msSingleton = 0;

namespace RTShader {

HardwareSkinningFactory* HardwareSkinningFactory::getSingletonPtr(void)
{
    return msSingleton;
}
HardwareSkinningFactory& HardwareSkinningFactory::getSingleton(void)
{  
    assert( msSingleton );  return ( *msSingleton );
}

String HardwareSkinning::Type = "SGX_HardwareSkinning";
const String SRS_HARDWARE_SKINNING = "SGX_HardwareSkinning";

ushort HardwareSkinningFactory::mMaxCalculableBoneCount = 70;

#define HS_MAX_WEIGHT_COUNT 4

/// A set of custom shadow caster materials
static MaterialPtr mCustomShadowCasterMaterialsLinear[HS_MAX_WEIGHT_COUNT];
static MaterialPtr mCustomShadowCasterMaterialsDualQuaternion[HS_MAX_WEIGHT_COUNT];

/// A set of custom shadow receiver materials
static MaterialPtr mCustomShadowReceiverMaterialsLinear[HS_MAX_WEIGHT_COUNT];
static MaterialPtr mCustomShadowReceiverMaterialsDualQuaternion[HS_MAX_WEIGHT_COUNT];

/************************************************************************/
/*                                                                      */
/************************************************************************/
HardwareSkinning::HardwareSkinning() :
    mCreator(NULL),
    mSkinningType(ST_LINEAR)
{
}

//-----------------------------------------------------------------------
const String& HardwareSkinning::getType() const
{
    return SRS_HARDWARE_SKINNING;
}

//-----------------------------------------------------------------------
int HardwareSkinning::getExecutionOrder() const
{
    return FFP_TRANSFORM;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::setParameter(const String& name, const String& value)
{
    if (name == "type")
    {
        if (value == "dual_quaternion")
        {
            mSkinningType = ST_DUAL_QUATERNION;
            if (!mDualQuat)
            {
                mDualQuat.reset(OGRE_NEW DualQuaternionSkinning);
            }

            mActiveTechnique = mDualQuat;
            return true;
        }
        else if(value == "linear")
        {
            mSkinningType = ST_LINEAR;
            if (!mLinear)
            {
                mLinear.reset(OGRE_NEW LinearSkinning);
            }

            mActiveTechnique = mLinear;
            return true;
        }
    }
    else if(mActiveTechnique)
    {
        return mActiveTechnique->setParameter(name, value);
    }
    return false;
}
//-----------------------------------------------------------------------
ushort HardwareSkinning::getBoneCount()
{
    assert(mActiveTechnique);
    return mActiveTechnique->getBoneCount();
}

//-----------------------------------------------------------------------
ushort HardwareSkinning::getWeightCount()
{
    assert(mActiveTechnique);
    return mActiveTechnique->getWeightCount();
}

//-----------------------------------------------------------------------
SkinningType HardwareSkinning::getSkinningType()
{
    assert(mActiveTechnique);
    return mSkinningType;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::hasCorrectAntipodalityHandling()
{
    assert(mActiveTechnique);
    return mActiveTechnique->hasCorrectAntipodalityHandling();
}

//-----------------------------------------------------------------------
bool HardwareSkinning::hasScalingShearingSupport()
{
    assert(mActiveTechnique);
    return mActiveTechnique->hasScalingShearingSupport();
}

//-----------------------------------------------------------------------
void HardwareSkinning::copyFrom(const SubRenderState& rhs)
{
    const HardwareSkinning& hardSkin = static_cast<const HardwareSkinning&>(rhs);

    mDualQuat = hardSkin.mDualQuat;
    mLinear = hardSkin.mLinear;
    mActiveTechnique = hardSkin.mActiveTechnique;
    
    mCreator = hardSkin.mCreator;
    mSkinningType = hardSkin.mSkinningType;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    bool isValid = true;
    Technique* pFirstTech = srcPass->getParent()->getParent()->getTechnique(0);
    const Any& hsAny = pFirstTech->getUserObjectBindings().getUserAny(HS_DATA_BIND_NAME);

    if (hsAny.has_value())
    {
        HardwareSkinning::SkinningData pData = any_cast<HardwareSkinning::SkinningData>(hsAny);
        isValid = pData.isValid;
        
        //If the skinning data is being passed through the material, we need to create an instance of the appropriate
        //skinning type and set its parameters here
        setParameter("type", pData.skinningType == ST_LINEAR ? "linear" : "dual_quaternion");
        mActiveTechnique->setHardwareSkinningParam(pData.maxBoneCount, pData.maxWeightCount,
                                                   pData.correctAntipodalityHandling, pData.scalingShearingSupport);
    }

    //If there is no associated technique, default to linear skinning as a pass-through
    if(!mActiveTechnique)
    {
        setParameter("type", "linear");
    }

    int boneCount = mActiveTechnique->getBoneCount();
    int weightCount = mActiveTechnique->getWeightCount();

    bool doBoneCalculations =  isValid &&
        (boneCount != 0) && (boneCount <= 256) &&
        (weightCount != 0) && (weightCount <= 4) &&
        ((mCreator == NULL) || (boneCount <= mCreator->getMaxCalculableBoneCount()));

    // This requires GLES3.0
    if (ShaderGenerator::getSingleton().getTargetLanguage() == "glsles" &&
        !GpuProgramManager::getSingleton().isSyntaxSupported("glsl300es"))
        doBoneCalculations = false;

    mActiveTechnique->setDoBoneCalculations(doBoneCalculations);
    mActiveTechnique->setDoLightCalculations(srcPass->getLightingEnabled());

    if ((doBoneCalculations) && (mCreator))
    {
        //update the receiver and caster materials
        if (!dstPass->getParent()->getShadowCasterMaterial())
        {
            auto casterMat = mCreator->getCustomShadowCasterMaterial(mSkinningType, weightCount - 1);

            // if the caster material itsefl uses RTSS hardware skinning
            if(casterMat.get() != dstPass->getParent()->getParent())
                dstPass->getParent()->setShadowCasterMaterial(casterMat);
        }

        if (!dstPass->getParent()->getShadowReceiverMaterial())
        {
            dstPass->getParent()->setShadowReceiverMaterial(
                mCreator->getCustomShadowReceiverMaterial(mSkinningType, weightCount - 1));
        }
    }

    return true;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveParameters(ProgramSet* programSet)
{
    assert(mActiveTechnique);
    return mActiveTechnique->resolveParameters(programSet);
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveDependencies(ProgramSet* programSet)
{
    assert(mActiveTechnique);
    return mActiveTechnique->resolveDependencies(programSet);
}

//-----------------------------------------------------------------------
bool HardwareSkinning::addFunctionInvocations(ProgramSet* programSet)
{
    assert(mActiveTechnique);
    return mActiveTechnique->addFunctionInvocations(programSet);
}

//-----------------------------------------------------------------------
HardwareSkinningFactory::HardwareSkinningFactory()
{

}

HardwareSkinningFactory::~HardwareSkinningFactory() {}

//-----------------------------------------------------------------------
const String& HardwareSkinningFactory::getType() const
{
    return SRS_HARDWARE_SKINNING;
}

//-----------------------------------------------------------------------
SubRenderState* HardwareSkinningFactory::createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "hardware_skinning")
    {
        String skinningType = "linear";
        
        if(prop->values.size() < 2)
            return NULL;

        std::map<String, String> params;
        AbstractNodeList::iterator it = prop->values.begin();
        params["max_bone_count"] = (*it)->getString();
        ++it;
        params["weight_count"] = (*it)->getString();

        if(prop->values.size() >= 3)
        {
            ++it;
            skinningType = (*it)->getString();
        }

        if(skinningType != "dual_quaternion" && skinningType != "linear")
            return NULL;

        if(prop->values.size() >= 5)
        {
            ++it;
            params["correct_antipodality"] = (*it)->getString();
            ++it;
            params["scale_shearing"] = (*it)->getString();
        }

        //create and update the hardware skinning sub render state
        SubRenderState* subRenderState = createOrRetrieveInstance(translator);
        subRenderState->setParameter("type", skinningType);

        for(const auto& p : params)
        {
            if(!subRenderState->setParameter(p.first, p.second))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, p.second);
            }
        }

        return subRenderState;
    }

    return NULL;
}

//-----------------------------------------------------------------------
void HardwareSkinningFactory::writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, 
                                       Pass* srcPass, Pass* dstPass)
{
    ser->writeAttribute(4, "hardware_skinning");
    
    HardwareSkinning* hardSkinSrs = static_cast<HardwareSkinning*>(subRenderState);
    ser->writeValue(StringConverter::toString(hardSkinSrs->getBoneCount()));
    ser->writeValue(StringConverter::toString(hardSkinSrs->getWeightCount()));

    //Correct antipodality handling and scaling and shearing support are only really valid for dual quaternion skinning
    if(hardSkinSrs->getSkinningType() == ST_DUAL_QUATERNION)
    {
        ser->writeValue("dual_quaternion");
        ser->writeValue(StringConverter::toString(hardSkinSrs->hasCorrectAntipodalityHandling()));
        ser->writeValue(StringConverter::toString(hardSkinSrs->hasScalingShearingSupport()));
    }
}

//-----------------------------------------------------------------------
SubRenderState* HardwareSkinningFactory::createInstanceImpl()
{
    HardwareSkinning* pSkin = OGRE_NEW HardwareSkinning;
    
    pSkin->_setCreator(this);
    return pSkin;
}

//-----------------------------------------------------------------------
void HardwareSkinningFactory::setCustomShadowCasterMaterials(const SkinningType skinningType, const MaterialPtr& caster1Weight, const MaterialPtr& caster2Weight,
                                            const MaterialPtr& caster3Weight, const MaterialPtr& caster4Weight)
{
    if(skinningType == ST_DUAL_QUATERNION)
    {
        mCustomShadowCasterMaterialsDualQuaternion[0] = caster1Weight;
        mCustomShadowCasterMaterialsDualQuaternion[1] = caster2Weight;
        mCustomShadowCasterMaterialsDualQuaternion[2] = caster3Weight;
        mCustomShadowCasterMaterialsDualQuaternion[3] = caster4Weight;
    }
    else //if(skinningType == ST_LINEAR)
    {
        mCustomShadowCasterMaterialsLinear[0] = caster1Weight;
        mCustomShadowCasterMaterialsLinear[1] = caster2Weight;
        mCustomShadowCasterMaterialsLinear[2] = caster3Weight;
        mCustomShadowCasterMaterialsLinear[3] = caster4Weight;
    }
}

//-----------------------------------------------------------------------
void HardwareSkinningFactory::setCustomShadowReceiverMaterials(const SkinningType skinningType, const MaterialPtr& receiver1Weight, const MaterialPtr& receiver2Weight,
                                              const MaterialPtr& receiver3Weight, const MaterialPtr& receiver4Weight)
{
    if(skinningType == ST_DUAL_QUATERNION)
    {
        mCustomShadowReceiverMaterialsDualQuaternion[0] = receiver1Weight;
        mCustomShadowReceiverMaterialsDualQuaternion[1] = receiver2Weight;
        mCustomShadowReceiverMaterialsDualQuaternion[2] = receiver3Weight;
        mCustomShadowReceiverMaterialsDualQuaternion[3] = receiver4Weight;
    }
    else //if(skinningType == ST_LINEAR)
    {
        mCustomShadowReceiverMaterialsLinear[0] = receiver1Weight;
        mCustomShadowReceiverMaterialsLinear[1] = receiver2Weight;
        mCustomShadowReceiverMaterialsLinear[2] = receiver3Weight;
        mCustomShadowReceiverMaterialsLinear[3] = receiver4Weight;
    }
}

//-----------------------------------------------------------------------
const MaterialPtr& HardwareSkinningFactory::getCustomShadowCasterMaterial(const SkinningType skinningType, ushort index)
{
    assert(index < HS_MAX_WEIGHT_COUNT);

    if(skinningType == ST_DUAL_QUATERNION)
    {
        return mCustomShadowCasterMaterialsDualQuaternion[index];
    }
    else //if(skinningType = ST_LINEAR)
    {
        return mCustomShadowCasterMaterialsLinear[index];
    }
}

//-----------------------------------------------------------------------
const MaterialPtr& HardwareSkinningFactory::getCustomShadowReceiverMaterial(const SkinningType skinningType, ushort index)
{
    assert(index < HS_MAX_WEIGHT_COUNT);

    if(skinningType == ST_DUAL_QUATERNION)
    {
        return mCustomShadowReceiverMaterialsDualQuaternion[index];
    }
    else //if(skinningType == ST_LINEAR)
    {
        return mCustomShadowReceiverMaterialsLinear[index];
    }
}

//----------------------------------------------------------------------
/**
    @brief
        Extracts the maximum amount of bones and weights used in an specific subentity of given entity.

    @param pEntity The entity from which the information needs to be extracted.
    @param subEntityIndex The index of subentity from which the information needs to be extracted.
    @param boneCount The maximum number of bones used by the entity.
    @param weightCount The maximum number of weights used by the entity.
    @return Returns true if the entity can use HS. False if not.
*/
static bool extractSkeletonData(const Entity* pEntity, size_t subEntityIndex, ushort& boneCount, ushort& weightCount)
{
    bool isValidData = false;
    boneCount = 0;
    weightCount = 0;

    //Check if we have pose animation which the HS sub render state does not 
    //know how to handle
    bool hasVertexAnim = pEntity->getMesh()->hasVertexAnimation();

    //gather data on the skeleton
    if (!hasVertexAnim && pEntity->hasSkeleton())
    {
        //get weights count
        const MeshPtr& pMesh = pEntity->getMesh();

        RenderOperation ro;
        SubMesh* pSubMesh = pMesh->getSubMesh(subEntityIndex);
        pSubMesh->_getRenderOperation(ro,0);

        //get the largest bone assignment
        boneCount = ushort(std::max(pMesh->sharedBlendIndexToBoneIndexMap.size(), pSubMesh->blendIndexToBoneIndexMap.size()));
            
        //go over vertex deceleration 
        //check that they have blend indices and blend weights
        const VertexElement* pDeclWeights = ro.vertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS,0);
        const VertexElement* pDeclIndexes = ro.vertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES,0);
        if ((pDeclWeights != NULL) && (pDeclIndexes != NULL))
        {
            isValidData = true;
            switch (pDeclWeights->getType())
            {
            case VET_FLOAT1:
                weightCount = 1;
                break;
            case VET_USHORT2_NORM:
            case VET_FLOAT2:
                weightCount = 2;
                break;
            case VET_FLOAT3:
                weightCount = 3;
                break;
            case VET_USHORT4_NORM:
            case VET_UBYTE4_NORM:
            case VET_FLOAT4:
                weightCount = 4;
                break;
            default:
                isValidData = false;
                break;
            }
        }
    }
    return isValidData;
}

/**
    @brief
        Updates an entity's the skeleton data onto one of it's materials.

    @param pMaterial The material to update with the information.
    @param isValid Tells if the material can be used with HS.
    @param boneCount The maximum number of bones used by the entity.
    @param weightCount The maximum number of weights used by the entity.
    @return Returns true if the data was updated on the material. False if not.
*/
static bool imprintSkeletonData(const MaterialPtr& pMaterial, bool isVaild,
                ushort boneCount, ushort weightCount, SkinningType skinningType, bool correctAntidpodalityHandling, bool scalingShearingSupport)
{
    bool isUpdated = false;
    if (pMaterial->getNumTechniques() > 0) 
    {
        HardwareSkinning::SkinningData data;

        //get the previous skinning data if available
        UserObjectBindings& binding = pMaterial->getTechnique(0)->getUserObjectBindings();
        const Any& hsAny = binding.getUserAny(HS_DATA_BIND_NAME);
        if (hsAny.has_value())
        {
            data = any_cast<HardwareSkinning::SkinningData>(hsAny);
        }

        //check if we need to update the data
        if (((data.isValid == true) && (isVaild == false)) ||
            (data.maxBoneCount < boneCount) ||
            (data.maxWeightCount < weightCount))
        {
            //update the data
            isUpdated = true;
            data.isValid &= isVaild;
            data.maxBoneCount = std::max<ushort>(data.maxBoneCount, boneCount);
            data.maxWeightCount = std::max<ushort>(data.maxWeightCount, weightCount);
            data.skinningType = skinningType;
            data.correctAntipodalityHandling = correctAntidpodalityHandling;
            data.scalingShearingSupport = scalingShearingSupport;

            //update the data in the material and invalidate it in the RTShader system
            //do it will be regenerated
            binding.setUserAny(HS_DATA_BIND_NAME, data);

            size_t schemeCount = ShaderGenerator::getSingleton().getRTShaderSchemeCount();
            for(size_t i = 0 ; i < schemeCount ; ++i)
            {
                //invalidate the material so it will be recreated with the correct
                //amount of bones and weights
                const String& schemeName = ShaderGenerator::getSingleton().getRTShaderScheme(i);
                ShaderGenerator::getSingleton().invalidateMaterial(schemeName, *pMaterial);
            }

        }
    }
    return isUpdated;

}

void HardwareSkinningFactory::prepareEntityForSkinning(const Entity* pEntity, SkinningType skinningType,
                               bool correctAntidpodalityHandling, bool shearScale)
{
    // This requires GLES3.0
    if (ShaderGenerator::getSingleton().getTargetLanguage() == "glsles" &&
        !GpuProgramManager::getSingleton().isSyntaxSupported("glsl300es"))
        return;

    if (pEntity != NULL)
    {
        size_t lodLevels = pEntity->getNumManualLodLevels() + 1;
        for(size_t indexLod = 0 ; indexLod < lodLevels ; ++indexLod)
        {
            const Entity* pCurEntity = pEntity;
            if (indexLod > 0) pCurEntity = pEntity->getManualLodLevel(indexLod - 1);

            size_t numSubEntities = pCurEntity->getNumSubEntities();
            for(size_t indexSub = 0 ; indexSub < numSubEntities ; ++indexSub)
            {
                ushort boneCount = 0,weightCount = 0;
                bool isValid = extractSkeletonData(pCurEntity, indexSub, boneCount, weightCount);

                SubEntity* pSubEntity = pCurEntity->getSubEntity(indexSub);
                const MaterialPtr& pMat = pSubEntity->getMaterial();
                imprintSkeletonData(pMat, isValid, boneCount, weightCount, skinningType, correctAntidpodalityHandling, shearScale);
            }
        }
    }
}

}
}

#endif


