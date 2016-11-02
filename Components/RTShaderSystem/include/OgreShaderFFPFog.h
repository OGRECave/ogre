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
#ifndef _ShaderFFPFog_
#define _ShaderFFPFog_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderSubRenderState.h"
#include "OgreVector4.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Fog sub render state implementation of the Fixed Function Pipeline.
@see http://msdn.microsoft.com/en-us/library/bb173398.aspx
Derives from SubRenderState class.
*/
class _OgreRTSSExport FFPFog : public SubRenderState
{
public:

    // Fog calculation mode enum.
    enum CalcMode
    {
        CM_PER_VERTEX   = 1,        // Per vertex fog calculations. (Default).
        CM_PER_PIXEL    = 2         // Per pixel fog calculations.
    };

// Interface.
public:

    /** Class default constructor */
    FFPFog();

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
    virtual void updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

    /** 
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const SubRenderState& rhs);

    /** 
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass);

    /** 
    Set the fog properties this fog sub render state should emulate.
    @param fogMode The fog mode to emulate (FOG_NONE, FOG_EXP, FOG_EXP2, FOG_LINEAR).
    @param fogColour The colour of the fog.
    @param fogStart Start distance of fog, used for linear mode only.
    @param fogEnd End distance of fog, used for linear mode only.
    @param fogDensity Fog density used in exponential modes only.
    @see http://msdn.microsoft.com/en-us/library/bb173401.aspx
    */
    void setFogProperties(FogMode fogMode, 
        const ColourValue& fogColour, 
        float fogStart, 
        float fogEnd, 
        float fogDensity);

    /** 
    Set the fog calculation mode. Either per vertex or per pixel.
    @param calcMode The calculation mode to set.
    */
    void setCalcMode(CalcMode calcMode) { mCalcMode = calcMode; }

    /** 
    Return the current calculation mode.
    */
    CalcMode getCalcMode() const { return mCalcMode; }

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

// Attributes.
protected:  
    // Fog calculation mode.
    CalcMode mCalcMode;
    // Fog formula. 
    FogMode mFogMode;
    // Fog colour value.
    ColourValue mFogColourValue;
    // Fog parameters (density, start, end, 1/end-start).
    Vector4 mFogParamsValue;
    // True if the fog parameters should be taken from the pass.
    bool mPassOverrideParams;

    // World view projection parameter.     
    UniformParameterPtr mWorldViewProjMatrix;
    // Fog colour parameter.    
    UniformParameterPtr mFogColour;
    // Fog parameters program parameter.    
    UniformParameterPtr mFogParams;
    // Vertex shader input position parameter.
    ParameterPtr mVSInPos;
    // Vertex shader output fog colour parameter.
    ParameterPtr mVSOutFogFactor;
    // Pixel shader input fog factor.
    ParameterPtr mPSInFogFactor;
    // Vertex shader output depth.
    ParameterPtr mVSOutDepth;
    // Pixel shader input depth.
    ParameterPtr mPSInDepth;
    // Pixel shader output diffuse colour.
    ParameterPtr mPSOutDiffuse;
};


/** 
A factory that enables creation of FFPFog instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport FFPFogFactory : public SubRenderStateFactory
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

    /** 
    @see SubRenderStateFactory::writeInstance.
    */
    virtual void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass);

    
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

