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

namespace Ogre {
namespace RTShader {
/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

enum SkinningType
{
    ST_LINEAR,
    ST_DUAL_QUATERNION
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
    const String& getType() const override;

    /** 
    @see SubRenderStateFactory::createInstance.
    */
    SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator) override;

    /** 
    @see SubRenderStateFactory::writeInstance.
    */
    void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass) override;

    /** 
    Sets the list of custom shadow caster materials
    */
    static void setCustomShadowCasterMaterials(const SkinningType skinningType, const MaterialPtr& caster1Weight,
                                               const MaterialPtr& caster2Weight, const MaterialPtr& caster3Weight,
                                               const MaterialPtr& caster4Weight);

    /** 
    Sets the list of custom shadow receiver materials
    */
    static void setCustomShadowReceiverMaterials(const SkinningType skinningType, const MaterialPtr& receiver1Weight,
                                                 const MaterialPtr& receiver2Weight, const MaterialPtr& receiver3Weight,
                                                 const MaterialPtr& receiver4Weight);

    /** 
    Returns the name of a custom shadow caster material for a given number of weights
    */
    static const MaterialPtr& getCustomShadowCasterMaterial(const SkinningType skinningType, ushort index);

    /** 
    Returns the name of a custom shadow receiver material for a given number of weights
    */
    static const MaterialPtr& getCustomShadowReceiverMaterial(const SkinningType skinningType, ushort index);

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
    static void prepareEntityForSkinning(const Entity* pEntity, SkinningType skinningType = ST_LINEAR,
                                         bool correctAntidpodalityHandling = false, bool shearScale = false);

    /** 
        @brief
            The maximum number of bones for which hardware skinning is performed.

        This number should be limited to avoid problems of using to many parameters
        in a shader. For example, in pixel shader 3 this should be around 70-90 
        dependent on other sub-render states in the shader.

        The default value for this property is 70 which correspond to pixel shader model 3 limitations
    */
    static ushort getMaxCalculableBoneCount() { return mMaxCalculableBoneCount; }
    /** 
        Sets the maximum number of bones for which hardware skinning is performed.
        @see getMaxCalculableBoneCount()
    */
    static void setMaxCalculableBoneCount(ushort count) { mMaxCalculableBoneCount = count; }

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

private:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    SubRenderState* createInstanceImpl() override;

    ///The maximum number of bones for which hardware skinning is performed.
    ///@see getMaxCalculableBoneCount()
    static ushort mMaxCalculableBoneCount;
};

/** @} */
/** @} */


}
}

#endif
#endif

