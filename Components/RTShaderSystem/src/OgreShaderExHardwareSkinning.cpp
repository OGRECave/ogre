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
#include "OgreShaderExHardwareSkinning.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderExDualQuaternionSkinning.h"
#include "OgreShaderExLinearSkinning.h"
#include "OgreMesh.h"
#include "OgreShaderGenerator.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreMaterial.h"
#include "OgreSubMesh.h"
#include "OgreTechnique.h"
#include "OgreMaterialSerializer.h"

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
    return Type;
}

//-----------------------------------------------------------------------
int HardwareSkinning::getExecutionOrder() const
{
    return FFP_TRANSFORM;
}

//-----------------------------------------------------------------------
void HardwareSkinning::setHardwareSkinningParam(ushort boneCount, ushort weightCount, SkinningType skinningType, bool correctAntipodalityHandling, bool scalingShearingSupport)
{
    mSkinningType = skinningType;
    
    if(skinningType == ST_DUAL_QUATERNION)
    {
        if(mDualQuat.isNull())
        {
            mDualQuat.bind(OGRE_NEW DualQuaternionSkinning);
        }

        mActiveTechnique = mDualQuat;
    }
    else //if(skinningType == ST_LINEAR)
    {
        if(mLinear.isNull())
        {
            mLinear.bind(OGRE_NEW LinearSkinning);
        }

        mActiveTechnique = mLinear;
    }
    
    mActiveTechnique->setHardwareSkinningParam(boneCount, weightCount, correctAntipodalityHandling, scalingShearingSupport);
}

//-----------------------------------------------------------------------
ushort HardwareSkinning::getBoneCount()
{
    assert(!mActiveTechnique.isNull());
    return mActiveTechnique->getBoneCount();
}

//-----------------------------------------------------------------------
ushort HardwareSkinning::getWeightCount()
{
    assert(!mActiveTechnique.isNull());
    return mActiveTechnique->getWeightCount();
}

//-----------------------------------------------------------------------
SkinningType HardwareSkinning::getSkinningType()
{
    assert(!mActiveTechnique.isNull());
    return mSkinningType;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::hasCorrectAntipodalityHandling()
{
    assert(!mActiveTechnique.isNull());
    return mActiveTechnique->hasCorrectAntipodalityHandling();
}

//-----------------------------------------------------------------------
bool HardwareSkinning::hasScalingShearingSupport()
{
    assert(!mActiveTechnique.isNull());
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
void operator<<(std::ostream& o, const HardwareSkinning::SkinningData& data)
{
    o << data.isValid;
    o << data.maxBoneCount;
    o << data.maxWeightCount;
    o << data.skinningType;
    o << data.correctAntipodalityHandling;
    o << data.scalingShearingSupport;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass)
{
    bool isValid = true;
    Technique* pFirstTech = srcPass->getParent()->getParent()->getTechnique(0);
    const Any& hsAny = pFirstTech->getUserObjectBindings().getUserAny(HS_DATA_BIND_NAME);

    if (hsAny.isEmpty() == false)
    {
        HardwareSkinning::SkinningData pData =
            (any_cast<HardwareSkinning::SkinningData>(hsAny));
        isValid = pData.isValid;
        
        //If the skinning data is being passed through the material, we need to create an instance of the appropriate
        //skinning type and set its parameters here
        setHardwareSkinningParam(pData.maxBoneCount, pData.maxWeightCount, pData.skinningType, 
                     pData.correctAntipodalityHandling, pData.scalingShearingSupport);
    }

    //If there is no associated technique, default to linear skinning as a pass-through
    if(mActiveTechnique.isNull())
    {
        setHardwareSkinningParam(0, 0, ST_LINEAR, false, false);
    }

    int boneCount = mActiveTechnique->getBoneCount();
    int weightCount = mActiveTechnique->getWeightCount();

    bool doBoneCalculations =  isValid &&
        (boneCount != 0) && (boneCount <= 256) &&
        (weightCount != 0) && (weightCount <= 4) &&
        ((mCreator == NULL) || (boneCount <= mCreator->getMaxCalculableBoneCount()));

    mActiveTechnique->setDoBoneCalculations(doBoneCalculations);

    if ((doBoneCalculations) && (mCreator))
    {
        //update the receiver and caster materials
        if (dstPass->getParent()->getShadowCasterMaterial().isNull())
        {
            dstPass->getParent()->setShadowCasterMaterial(
                mCreator->getCustomShadowCasterMaterial(mSkinningType, weightCount - 1));
        }

        // TODO fix it 
        //if (dstPass->getParent()->getShadowReceiverMaterial().isNull())
        //{
        //  dstPass->getParent()->setShadowReceiverMaterial(
        //      mCreator->getCustomShadowReceiverMaterial(mSkinningType, weightCount - 1));
        //}
    }

    return true;
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveParameters(ProgramSet* programSet)
{
    assert(!mActiveTechnique.isNull());
    return mActiveTechnique->resolveParameters(programSet);
}

//-----------------------------------------------------------------------
bool HardwareSkinning::resolveDependencies(ProgramSet* programSet)
{
    assert(!mActiveTechnique.isNull());
    return mActiveTechnique->resolveDependencies(programSet);
}

//-----------------------------------------------------------------------
bool HardwareSkinning::addFunctionInvocations(ProgramSet* programSet)
{
    assert(!mActiveTechnique.isNull());
    return mActiveTechnique->addFunctionInvocations(programSet);
}

//-----------------------------------------------------------------------
HardwareSkinningFactory::HardwareSkinningFactory() :
    mMaxCalculableBoneCount(70)
{

}

//-----------------------------------------------------------------------
const String& HardwareSkinningFactory::getType() const
{
    return HardwareSkinning::Type;
}

//-----------------------------------------------------------------------
SubRenderState* HardwareSkinningFactory::createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "hardware_skinning")
    {
        bool hasError = false;
        uint32 boneCount = 0;
        uint32 weightCount = 0;
        String skinningType = "";
        SkinningType skinType = ST_LINEAR;
        bool correctAntipodalityHandling = false;
        bool scalingShearingSupport = false;
        
        if(prop->values.size() >= 2)
        {
            AbstractNodeList::iterator it = prop->values.begin();
            if(false == SGScriptTranslator::getUInt(*it, &boneCount))
                hasError = true;

            ++it;
            if(false == SGScriptTranslator::getUInt(*it, &weightCount))
                hasError = true;

            if(prop->values.size() >= 5)
            {
                ++it;
                SGScriptTranslator::getString(*it, &skinningType);

                ++it;
                SGScriptTranslator::getBoolean(*it, &correctAntipodalityHandling);

                ++it;
                SGScriptTranslator::getBoolean(*it, &scalingShearingSupport);
            }

            //If the skinningType is not specified or is specified incorrectly, default to linear skinning.
            if(skinningType == "dual_quaternion")
            {
                skinType = ST_DUAL_QUATERNION;
            }
            else
            {
                skinType = ST_LINEAR;
            }
        }

        if (hasError == true)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, "Expected the format: hardware_skinning <bone count> <weight count> [skinning type] [correct antipodality handling] [scaling/shearing support]");
            return NULL;
        }
        else
        {
            //create and update the hardware skinning sub render state
            SubRenderState* subRenderState = createOrRetrieveInstance(translator);
            HardwareSkinning* hardSkinSrs = static_cast<HardwareSkinning*>(subRenderState);
            hardSkinSrs->setHardwareSkinningParam(boneCount, weightCount, skinType, correctAntipodalityHandling, scalingShearingSupport);
            
            return subRenderState;
        }

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
const MaterialPtr& HardwareSkinningFactory::getCustomShadowCasterMaterial(const SkinningType skinningType, ushort index) const
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
const MaterialPtr& HardwareSkinningFactory::getCustomShadowReceiverMaterial(const SkinningType skinningType, ushort index) const
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

//-----------------------------------------------------------------------
void HardwareSkinningFactory::prepareEntityForSkinning(const Entity* pEntity, SkinningType skinningType, 
                               bool correctAntidpodalityHandling, bool shearScale)
{
    if (pEntity != NULL) 
    {
        size_t lodLevels = pEntity->getNumManualLodLevels() + 1;
        for(size_t indexLod = 0 ; indexLod < lodLevels ; ++indexLod)
        {
            const Entity* pCurEntity = pEntity;
            if (indexLod > 0) pCurEntity = pEntity->getManualLodLevel(indexLod - 1);

            unsigned int numSubEntities = pCurEntity->getNumSubEntities();
            for(unsigned int indexSub = 0 ; indexSub < numSubEntities ; ++indexSub)
            {
                ushort boneCount = 0,weightCount = 0;
                bool isValid = extractSkeletonData(pCurEntity, indexSub, boneCount, weightCount);

                const SubEntity* pSubEntity = pCurEntity->getSubEntity(indexSub);
                const MaterialPtr& pMat = pSubEntity->getMaterial();
                imprintSkeletonData(pMat, isValid, boneCount, weightCount, skinningType, correctAntidpodalityHandling, shearScale);
            }
        }
    }
}

//-----------------------------------------------------------------------
bool HardwareSkinningFactory::extractSkeletonData(const Entity* pEntity, unsigned int subEntityIndex, ushort& boneCount, ushort& weightCount)
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
        MeshPtr pMesh = pEntity->getMesh();

        RenderOperation ro;
        SubMesh* pSubMesh = pMesh->getSubMesh(subEntityIndex);
        pSubMesh->_getRenderOperation(ro,0);

        //get the largest bone assignment
        boneCount = std::max<ushort>(pMesh->sharedBlendIndexToBoneIndexMap.size(), pSubMesh->blendIndexToBoneIndexMap.size());
            
        //go over vertex deceleration 
        //check that they have blend indices and blend weights
        const VertexElement* pDeclWeights = ro.vertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS,0);
        const VertexElement* pDeclIndexes = ro.vertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES,0);
        if ((pDeclWeights != NULL) && (pDeclIndexes != NULL))
        {
            isValidData = true;
            switch (pDeclWeights->getType())
            {
            case VET_FLOAT1: weightCount = 1; break;
            case VET_FLOAT2: weightCount = 2; break;
            case VET_FLOAT3: weightCount = 3; break;
            case VET_FLOAT4: weightCount = 4; break;
            default: isValidData = false; 
            }
        }
    }
    return isValidData;
}

//-----------------------------------------------------------------------
bool HardwareSkinningFactory::imprintSkeletonData(const MaterialPtr& pMaterial, bool isVaild, 
                ushort boneCount, ushort weightCount, SkinningType skinningType, bool correctAntidpodalityHandling, bool scalingShearingSupport)
{
    bool isUpdated = false;
    if (pMaterial->getNumTechniques() > 0) 
    {
        HardwareSkinning::SkinningData data;

        //get the previous skinning data if available
        UserObjectBindings& binding = pMaterial->getTechnique(0)->getUserObjectBindings();
        const Any& hsAny = binding.getUserAny(HS_DATA_BIND_NAME);
        if (hsAny.isEmpty() == false)
        {
            data = (any_cast<HardwareSkinning::SkinningData>(hsAny));
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
            binding.setUserAny(HS_DATA_BIND_NAME, Any(data));

            size_t schemeCount = ShaderGenerator::getSingleton().getRTShaderSchemeCount();
            for(size_t i = 0 ; i < schemeCount ; ++i)
            {
                //invalidate the material so it will be recreated with the correct
                //amount of bones and weights
                const String& schemeName = ShaderGenerator::getSingleton().getRTShaderScheme(i);
                ShaderGenerator::getSingleton().invalidateMaterial(
                    schemeName, pMaterial->getName(), pMaterial->getGroup());

            }

        }
    }
    return isUpdated;

}


}
}

#endif


