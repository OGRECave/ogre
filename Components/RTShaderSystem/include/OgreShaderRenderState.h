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
#ifndef _ShaderRenderState_
#define _ShaderRenderState_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreShaderProgramSet.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** This is a container class for sub render state class.
A render state is defined by the sub render states that compose it.
The user should use this interface to define global or per material custom behavior.
I.E In order to apply per pixel to a specific material one should implement a sub class of SubRenderState that
perform a per pixel lighting model, get the render state of the target material and add the custom sub class to it.
*/
class _OgreRTSSExport RenderState : public RTShaderSystemAlloc
{

    // Interface.
public:

    /** Class default constructor. */
    RenderState();

    /** Class destructor */
    virtual ~RenderState();

    /** Reset this render state */
    void reset();

    /** Add a template sub render state to this render state.
    @param subRenderState The sub render state template to add.
    */
    void addTemplateSubRenderState(SubRenderState* subRenderState);

    /// Add multiple template sub render states by SRS type
    void addTemplateSubRenderStates(const StringVector& srsTypes);

    /** Remove a sub render state from this render state.
    @param subRenderState The sub render state to remove.
    */
    void removeSubRenderState(SubRenderState* subRenderState);

    /** Get the list of the sub render states composing this render state. */
    const SubRenderStateList& getSubRenderStates() const { return mSubRenderStateList; }

    /// get sub render state by type (uniquely identified) or NULL if not found
    SubRenderState* getSubRenderState(const String& type) const;

    /** 
    Set the light count per light type.
    @param 
    lightCount The light count per type.
    lightCount[0] defines the point light count.
    lightCount[1] defines the directional light count.
    lightCount[2] defines the spot light count.
    */
    void setLightCount(const Vector3i& lightCount);

    /** 
    Get the light count per light type.

    lightCount[0] defines the point light count.
    lightCount[1] defines the directional light count.
    lightCount[2] defines the spot light count.
    */
    const Vector3i& getLightCount() const;

    /** 
    Set the light count auto update state.
    If the value is false the light count will remain static for the values that were set by the user.
    If the value is true the light count will be updated from the owner shader generator scheme based on current scene lights.
    The default is true.
    */
    void setLightCountAutoUpdate(bool autoUpdate) { mLightCountAutoUpdate = autoUpdate; }

    /** 
    Return true if this render state override the light count. 
    If light count is not overridden it will be updated from the shader generator based on current scene lights.
    */
    bool getLightCountAutoUpdate() const { return mLightCountAutoUpdate; }

    


    // Attributes.
protected:
    // The sub render states list.  
    SubRenderStateList mSubRenderStateList;
    // The light count per light type definition.
    Vector3i mLightCount;
    // True if light count was explicitly set.
    bool mLightCountAutoUpdate;

private:
    friend class ProgramManager;
};

/** This is the target render state. This class will hold the actual generated CPU/GPU programs.
It will be initially build from the FFP state of a given Pass by the FFP builder and then will be linked
with the custom pass render state and the global scheme render state. See ShaderGenerator::SGPass::buildTargetRenderState().
*/
class _OgreRTSSExport TargetRenderState : public RenderState
{

// Interface.
public:
    
    /** Class default constructor. */
    TargetRenderState();
    ~TargetRenderState();

    /** Add the SubRenderStates of the given render state as templates to this render state.
    @note Only sub render states with non FFP execution order will be added.
    @param templateRS The other render state to use as a template.
    @param srcPass The source pass that this render state is constructed from.
    @param dstPass The destination pass that constructed from this render state.
    */
    void link(const RenderState& templateRS, Pass* srcPass, Pass* dstPass);

    /** Add the SubRenderStates to this render state.
     */
    void link(const StringVector& srsTypes, Pass* srcPass, Pass* dstPass);

    /** Update the GPU programs constant parameters before a renderable is rendered.
    @param rend The renderable object that is going to be rendered.
    @param pass The pass that is used to do the rendering operation.
    @param source The auto parameter auto source instance.
    @param pLightList The light list used for the current rendering operation.
    */
    void updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

    /** Add sub render state to this render state.
    @param subRenderState The sub render state to add.
    */
    void addSubRenderStateInstance(SubRenderState* subRenderState);

    /** Acquire CPU/GPU programs set associated with the given render state and bind them to the pass.
    @param pass The pass to bind the programs to.
    */
    void acquirePrograms(Pass* pass);

    /** Release CPU/GPU programs set associated with the given render state and pass.
    @param pass The pass to release the programs from.
    */
    void releasePrograms(Pass* pass);

    /// Key name for associating with a Pass instance.
    static const char* UserKey;
private:
    /** Bind the uniform parameters of a given CPU and GPU program set. */
    static void bindUniformParameters(Program* pCpuProgram, const GpuProgramParametersSharedPtr& passParams);

    /** Sort the sub render states composing this render state. */
    void sortSubRenderStates();
    
    /** Create CPU programs that represent this render state.   
    */
    void createCpuPrograms();

    /** Create the program set of this render state.
    */
    ProgramSet* createProgramSet();

    /** Return the program set of this render state.
    */
    ProgramSet* getProgramSet() { return mProgramSet.get(); }
    
    // Tells if the list of the sub render states is sorted.
    bool mSubRenderStateSortValid;
    // The program set of this RenderState.
    std::unique_ptr<ProgramSet> mProgramSet;
    Pass* mParent;

private:
    friend class ProgramManager;
};

typedef std::shared_ptr<TargetRenderState> TargetRenderStatePtr;

/** @} */
/** @} */

}
}

#endif

