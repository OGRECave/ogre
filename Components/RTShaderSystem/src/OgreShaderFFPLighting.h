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
#ifndef _ShaderFFPLighting_
#define _ShaderFFPLighting_

#include "OgreShaderPrerequisites.h"
#if defined(RTSHADER_SYSTEM_BUILD_CORE_SHADERS) || defined(RTSHADER_SYSTEM_BUILD_EXT_SHADERS)
#include "OgreShaderSubRenderState.h"
#include "OgreLight.h"
#include "OgreCommon.h"

namespace Ogre {
namespace RTShader {


/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Lighting sub render state implementation of the Fixed Function Pipeline.
@see http://msdn.microsoft.com/en-us/library/bb147178.aspx
Derives from SubRenderState class.
*/
class FFPLighting : public SubRenderState
{

// Interface.
public:
    
    /** Class default constructor */
    FFPLighting();

    /** 
    @see SubRenderState::getType.
    */
    const String& getType() const override;

    /** 
    @see SubRenderState::getType.
    */
    int getExecutionOrder() const override;

    /** 
    @see SubRenderState::copyFrom.
    */
    void copyFrom(const SubRenderState& rhs) override;

    /** 
    @see SubRenderState::preAddToRenderState.
    */
    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;

    /** normalise the blinn-phong reflection model to make it energy conserving
     *
     * see [this for details](http://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/)
     */
    void setNormaliseEnabled(bool enable) { mNormalisedEnable = enable; }

    bool setParameter(const String& name, const String& value) override;

    /**
    Get the specular component state.
    */
    bool getSpecularEnable() const    { return mSpecularEnable; }

// Protected types:
protected:
    int mLightCount;
    // Per light parameters.
    // Light position.
    UniformParameterPtr mPositions;
    // Light direction.
    UniformParameterPtr mDirections;
    // Attenuation parameters.
    UniformParameterPtr mAttenuatParams;
    // Spot light parameters.
    UniformParameterPtr mSpotParams;
    // Diffuse colour.
    UniformParameterPtr mDiffuseColours;
    // Specular colour.
    UniformParameterPtr mSpecularColours;

    void addDefines(Program* program);

    /** 
    Set the track per vertex colour type. Ambient, Diffuse, Specular and Emissive lighting components source
    can be the vertex colour component. To establish such a link one should provide the matching flags to this
    sub render state.
    */
    void setTrackVertexColourType(TrackVertexColourType type) { mTrackVertexColourType = type; }

    /** 
    Return the current track per vertex type.
    */
    TrackVertexColourType getTrackVertexColourType() const { return mTrackVertexColourType; }

    /** 
    @see SubRenderState::resolveParameters.
    */
    bool resolveParameters(ProgramSet* programSet) override;

    /** 
    @see SubRenderState::resolveDependencies.
    */
    bool resolveDependencies(ProgramSet* programSet) override;

    /** 
    @see SubRenderState::addFunctionInvocations.
    */
    bool addFunctionInvocations(ProgramSet* programSet) override;


    /** 
    Internal method that adds global illumination component functions invocations.
    */
    void addGlobalIlluminationInvocation(const FunctionStageRef& stage);
            
    /** 
    Internal method that adds per light illumination component functions invocations.
    */
    void addIlluminationInvocation(int i, const FunctionStageRef& stage);


    // Track per vertex colour type.
    TrackVertexColourType mTrackVertexColourType;
    // Specular component enabled/disabled.
    bool mSpecularEnable;
    bool mNormalisedEnable;
    bool mTwoSidedLighting;
    int8 mLtcLUT1SamplerIndex;

    // World view matrix parameter.
    UniformParameterPtr mWorldViewMatrix;
    // World view matrix inverse transpose parameter.
    UniformParameterPtr mWorldViewITMatrix;
    // Transformed view normal
    ParameterPtr mViewNormal;
    // Transformed view position
    ParameterPtr mViewPos;
    // Vertex shader input position parameter.
    ParameterPtr mVSInPosition;
    // Vertex shader input normal.
    ParameterPtr mVSInNormal;
    // Vertex shader diffuse.
    ParameterPtr mInDiffuse;
    // Vertex shader output diffuse colour parameter.
    ParameterPtr mOutDiffuse;
    // Vertex shader output specular colour parameter.
    ParameterPtr mOutSpecular;
    // Derived scene colour parameter.
    UniformParameterPtr mDerivedSceneColour;
    // Ambient light colour parameter.
    UniformParameterPtr mLightAmbientColour;
    // Derived ambient light colour parameter.
    UniformParameterPtr mDerivedAmbientLightColour;
    // Surface emissive colour parameter.
    UniformParameterPtr mSurfaceEmissiveColour;
    // Surface shininess parameter.
    UniformParameterPtr mSurfaceShininess;

    UniformParameterPtr mLTCLUT1;
    UniformParameterPtr mLTCLUT2;

    ParameterPtr mShadowFactor;
};


/** 
A factory that enables creation of FFPLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class FFPLightingFactory : public SubRenderStateFactory
{
public:

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

    
protected:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    SubRenderState* createInstanceImpl() override;


};

/** @} */
/** @} */

}
}

#endif
#endif
