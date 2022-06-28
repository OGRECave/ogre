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
#ifndef _ShaderExHardwareSkinning_
#define _ShaderExHardwareSkinning_

#include "OgreShaderPrerequisites.h"

#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderSubRenderState.h"

#define HS_MAX_WEIGHT_COUNT 4

namespace Ogre {
namespace RTShader {

    class HardwareSkinningFactory;
    class DualQuaternionSkinning;
    class HardwareSkinningTechnique;
    class LinearSkinning;

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Implement a sub render state which performs hardware skinning.
Meaning, this sub render states adds calculations which multiply
the points and normals by their assigned bone matricies.
*/
class _OgreRTSSExport HardwareSkinning : public SubRenderState
{
public:
    struct SkinningData
    {
        SkinningData() :
            isValid(true), maxBoneCount(0), maxWeightCount(0), skinningType(ST_LINEAR), correctAntipodalityHandling(false), scalingShearingSupport(false)
        {}

        bool isValid;
        ushort maxBoneCount;
        ushort maxWeightCount;
        SkinningType skinningType;
        bool correctAntipodalityHandling;
        bool scalingShearingSupport;
    };

// Interface.
public:
    /** Class default constructor */
    HardwareSkinning();

    /**
    @see SubRenderState::getType.
    */
    virtual const String& getType() const;

    /**
    @see SubRenderState::getType.
    */
    virtual int getExecutionOrder() const;

    /**
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const SubRenderState& rhs);

    /**
    Set the hardware skinning parameters.
    @param boneCount The maximum number of bones in the model this material
         is assigned to. Note that this parameter can be higher but not
         lower than the actual number of bones.
    @param weightCount The maximum number of weights/bones affecting a vertex.
    @param skinningType The type of skinning desired.
    @param correctAntipodalityHandling If correct antipodality handling should be utilized (Only applicable for dual quaternion skinning).
    @param scalingShearingSupport If scaling and shearing support should be enabled (Only applicable for dual quaternion skinning).
    */
    void setHardwareSkinningParam(ushort boneCount, ushort weightCount, SkinningType skinningType = ST_LINEAR,  bool correctAntipodalityHandling = false, bool scalingShearingSupport = false);

    /**
    Returns the number of bones in the model assigned to the material.
    @see setHardwareSkinningParam()
    */
    ushort getBoneCount();

    /**
    Returns the number of weights/bones affecting a vertex.
    @see setHardwareSkinningParam()
    */
    ushort getWeightCount();

    /**
    Returns the current skinning type in use.
    @see setHardwareSkinningParam()
     */
    SkinningType getSkinningType();

    /**
    Only applicable for dual quaternion skinning.
    @see setHardwareSkinningParam()
    */
    bool hasCorrectAntipodalityHandling();

    /**
    Only applicable for dual quaternion skinning.
    @see setHardwareSkinningParam()
    */
    bool hasScalingShearingSupport();

    /**
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass);

    /**
    Set the factory which created this sub render state
    */
    void _setCreator(const HardwareSkinningFactory* pCreator) { mCreator = pCreator; }

    static String Type;

// Protected methods
protected:
    /**
    @see SubRenderState::resolveParameters.
    */
    virtual bool resolveParameters(ProgramSet* programSet);

    /**
    @see SubRenderState::resolveDependencies.
    */
    virtual bool resolveDependencies(ProgramSet* programSet);

    /**
    @see SubRenderState::addFunctionInvocations.
    */
    virtual bool addFunctionInvocations(ProgramSet* programSet);

    SharedPtr<LinearSkinning> mLinear;
    SharedPtr<DualQuaternionSkinning> mDualQuat;
    SharedPtr<HardwareSkinningTechnique> mActiveTechnique;
    
    ///The factory which created this sub render state
    const HardwareSkinningFactory* mCreator;
    SkinningType mSkinningType;
};

/** 
A factory that enables creation of HardwareSkinning instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport HardwareSkinningFactory : public SubRenderStateFactory, 
    public Singleton<HardwareSkinningFactory>
{
public:
    HardwareSkinningFactory();
    ~HardwareSkinningFactory();
    
    /** 
    @see SubRenderStateFactory::getType.
    */
    virtual const String& getType() const;

    /** 
    @see SubRenderStateFactory::createInstance.
    */
    virtual SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);

    /** 
    @see SubRenderStateFactory::writeInstance.
    */
    virtual void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass);

    /** 
    Sets the list of custom shadow caster materials
    */
    virtual void setCustomShadowCasterMaterials(const SkinningType skinningType, const MaterialPtr& caster1Weight, const MaterialPtr& caster2Weight,
        const MaterialPtr& caster3Weight, const MaterialPtr& caster4Weight);
    
    /** 
    Sets the list of custom shadow receiver materials
    */
    virtual void setCustomShadowReceiverMaterials(const SkinningType skinningType, const MaterialPtr& receiver1Weight, const MaterialPtr& receiver2Weight,
        const MaterialPtr& receiver3Weight, const MaterialPtr& receiver4Weight);

    /** 
    Returns the name of a custom shadow caster material for a given number of weights
    */
    const MaterialPtr& getCustomShadowCasterMaterial(const SkinningType skinningType, ushort index) const;

    /** 
    Returns the name of a custom shadow receiver material for a given number of weights
    */
    const MaterialPtr& getCustomShadowReceiverMaterial(const SkinningType skinningType, ushort index) const;

    /**
        @brief 
            Prepares an entity's material for use in the hardware skinning (HS).
        
        This function prepares an entity's material for use by the HS sub-render
        state. This function scans the entity and extracts the information of the amount
        of bones and weights in the entity. This function replaces the need specify in 
        the material script the  amount of bones and weights needed to make the HS work.
        
        Note that this class does not save the the bone and weight count information 
        internally. Rather this information is stored in the entity's materials as a 
        user binded object.
        
        @par pEntity A pointer to an entity who's materials need preparing.
    */
    void prepareEntityForSkinning(const Entity* pEntity, SkinningType skinningType = ST_LINEAR, bool correctAntidpodalityHandling = false, bool shearScale = false);

    /** 
        @brief
            The maximum number of bones for which hardware skinning is performed.

        This number should be limited to avoid problems of using to many parameters
        in a shader. For example, in pixel shader 3 this should be around 70-90 
        dependent on other sub-render states in the shader.

        The default value for this property is 70 which correspond to pixel shader model 3 limitations
    */
    ushort getMaxCalculableBoneCount() const {
        return mMaxCalculableBoneCount; }
    /** 
        Sets the maximum number of bones for which hardware skinning is performed.
        @see getMaxCalculableBoneCount()
    */
    void setMaxCalculableBoneCount(ushort count) {
        mMaxCalculableBoneCount = count; }

    /** 
    Override standard Singleton retrieval.
    
    @remarks Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        
    @par 
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
    */
    static HardwareSkinningFactory& getSingleton(void);
    
    /// @copydoc Singleton::getSingleton()
    static HardwareSkinningFactory* getSingletonPtr(void);

protected:
    /** 
        @brief
            Extracts the maximum amount of bones and weights used in an specific subentity of given entity.
        
        @param pEntity The entity from which the information needs to be extracted.
        @param subEntityIndex The index of subentity from which the information needs to be extracted.
        @param boneCount The maximum number of bones used by the entity.
        @param weightCount The maximum number of weights used by the entity.
        @return Returns true if the entity can use HS. False if not. 
    */
    bool extractSkeletonData(const Entity* pEntity, size_t subEntityIndex,
        ushort& boneCount, ushort& weightCount);

    /** 
        @brief
            Updates an entity's the skeleton data onto one of it's materials.
        
        @param pMaterial The material to update with the information.
        @param isValid Tells if the material can be used with HS.
        @param boneCount The maximum number of bones used by the entity.
        @param weightCount The maximum number of weights used by the entity.
        @return Returns true if the data was updated on the material. False if not.
    */
    bool imprintSkeletonData(const MaterialPtr& pMaterial, bool isValid, 
        ushort boneCount, ushort weightCount, SkinningType skinningType, bool correctAntidpodalityHandling, bool scalingShearingSupport);

protected:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    virtual SubRenderState* createInstanceImpl();

    /// A set of custom shadow caster materials
    MaterialPtr mCustomShadowCasterMaterialsLinear[HS_MAX_WEIGHT_COUNT];
    MaterialPtr mCustomShadowCasterMaterialsDualQuaternion[HS_MAX_WEIGHT_COUNT];

    /// A set of custom shadow receiver materials
    MaterialPtr mCustomShadowReceiverMaterialsLinear[HS_MAX_WEIGHT_COUNT];
    MaterialPtr mCustomShadowReceiverMaterialsDualQuaternion[HS_MAX_WEIGHT_COUNT];

    ///The maximum number of bones for which hardware skinning is performed.
    ///@see getMaxCalculableBoneCount()
    ushort mMaxCalculableBoneCount;
};

/** @} */
/** @} */


}
}

#endif
#endif

