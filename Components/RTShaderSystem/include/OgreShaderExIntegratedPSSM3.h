/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _ShaderExIntegratedPSSM3_
#define _ShaderExIntegratedPSSM3_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreCommon.h"
#include "OgreShaderSubRenderState.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

#define SGX_LIB_INTEGRATEDPSSM                      "SGXLib_IntegratedPSSM"
#define SGX_FUNC_COMPUTE_SHADOW_COLOUR3             "SGX_ComputeShadowFactor_PSSM3"
#define SGX_FUNC_APPLYSHADOWFACTOR_DIFFUSE          "SGX_ApplyShadowFactor_Diffuse"

/** Integrated PSSM shadow receiver with 3 splits sub render state implementation.
Derives from SubRenderState class.
*/
class _OgreRTSSExport IntegratedPSSM3 : public SubRenderState
{

    // Interface.
public:
    typedef std::vector<Real> SplitPointList;

    /** Class default constructor */    
    IntegratedPSSM3();

    /** 
    @see SubRenderState::getType.
    */
    virtual const String& getType() const;

    /** 
    @see SubRenderState::getType.
    */
    virtual int getExecutionOrder() const;

    /** 
    @see SubRenderState::updateGpuProgramsParams.
    */
    virtual void updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

    /** 
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const SubRenderState& rhs);


    /** 
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass);


    
    /** Manually configure a new splitting scheme.
    @param newSplitPoints A list which is splitCount + 1 entries long, containing the
    split points. The first value is the near point, the last value is the
    far point, and each value in between is both a far point of the previous
    split, and a near point for the next one.
    */
    void setSplitPoints(const SplitPointList& newSplitPoints);

    void setDebug(bool enable) { mDebug = enable; }

    static String Type;

    // Protected types:
protected:

    // Shadow texture parameters.
    struct _OgreRTSSExport ShadowTextureParams
    {                   
        // The max range of this shadow texture in terms of PSSM (far plane of viewing camera).
        Real mMaxRange;
        // The shadow map sampler index.
        unsigned int mTextureSamplerIndex;
        // The shadow map sampler.          
        UniformParameterPtr mTextureSampler;
        // The inverse texture 
        UniformParameterPtr mInvTextureSize;
        // The source light view projection matrix combined with world matrix.      
        UniformParameterPtr mWorldViewProjMatrix;
        // The vertex shader output position in light space.
        ParameterPtr mVSOutLightPosition;
        // The pixel shader input position in light space.
        ParameterPtr mPSInLightPosition;

    };

    typedef std::vector<ShadowTextureParams>           ShadowTextureParamsList;
    typedef ShadowTextureParamsList::iterator           ShadowTextureParamsIterator;
    typedef ShadowTextureParamsList::const_iterator     ShadowTextureParamsConstIterator;

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

    /** 
    Internal method that adds related vertex shader functions invocations.
    */
    bool addVSInvocation(Function* vsMain, const int groupOrder);

    /** 
    Internal method that adds related pixel shader functions invocations.
    */
    bool addPSInvocation(Program* psProgram, const int groupOrder);





    // Attributes.
protected:      
    // Shadow texture parameter list.   
    ShadowTextureParamsList mShadowTextureParamsList;
    // Split points parameter.
    UniformParameterPtr mPSSplitPoints;
    // Vertex shader input position parameter.  
    ParameterPtr mVSInPos;
    // Vertex shader output position (clip space) parameter.
    ParameterPtr mVSOutPos;
    // Vertex shader output depth (clip space) parameter.
    ParameterPtr mVSOutDepth;
    // Pixel shader input depth (clip space) parameter.
    ParameterPtr mPSInDepth;
    // Pixel shader local computed shadow colour parameter.
    ParameterPtr mPSLocalShadowFactor;
    // Pixel shader in/local diffuse colour parameter.
    ParameterPtr mPSDiffuse;
    // Pixel shader output diffuse colour parameter.
    ParameterPtr mPSOutDiffuse;
    // Pixel shader in/local specular colour parameter.
    ParameterPtr mPSSpecualr;
    // Derived scene colour (ambient term).
    UniformParameterPtr mPSDerivedSceneColour;

    bool mUseTextureCompare;
    bool mDebug;
};


/** 
A factory that enables creation of IntegratedPSSM3 instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport IntegratedPSSM3Factory : public SubRenderStateFactory
{
public:

    /** 
    @see SubRenderStateFactory::getType.
    */
    virtual const String& getType() const;

    /** 
    @see SubRenderStateFactory::createInstance.
    */
    virtual SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);


protected:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    virtual SubRenderState* createInstanceImpl();


};

/** @} */
/** @} */

}
}

#endif
#endif

