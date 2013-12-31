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
#ifndef _ShaderExTriplanarTexturing_
#define _ShaderExTriplanarTexturing_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"

#define SGX_FUNC_TRIPLANAR_TEXTURING "SGX_TriplanarTexturing"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Triplanar Texturing.
Derives from SubRenderState class.
*/
class _OgreRTSSExport TriplanarTexturing : public SubRenderState
{

protected:
    
    /// Parameters: Texturescale, Plateau-size (0 to 0.57, not bigger to avoid division by zero!), transition-speed
    Vector3 mParameters;
    
    /// Parameters uniform.
    UniformParameterPtr mPSTPParams;
    
    /// Normal vertex shader in.
    ParameterPtr mVSInNormal;
    
    /// Normal vertex shader out.
    ParameterPtr mVSOutNormal;

    /// Normal pixel shader in.
    ParameterPtr mPSInNormal;
    
    /// Position vertex shader in.
    ParameterPtr mVSInPosition;
    
    /// Position vertex shader out.
    ParameterPtr mVSOutPosition;

    /// Position pixel shader in.
    ParameterPtr mPSInPosition;

    /// Texture sampler for the x-direction planar mapping part.
    UniformParameterPtr mSamplerFromX;

    /// Texture sampler for the y-direction planar mapping part.
    UniformParameterPtr mSamplerFromY;

    /// Texture sampler for the z-direction planar mapping part.
    UniformParameterPtr mSamplerFromZ;
    
    /// Pixel shader input diffuse colour.
    ParameterPtr mPSInDiffuse;

    /// Pixel shader output diffuse colour.
    ParameterPtr mPSOutDiffuse;

    /// Texturename mapping from x.
    String mTextureNameFromX;
    
    /// Texture sampler id mapping from x.
    ushort mTextureSamplerIndexFromX;

    /// Texturename mapping from y.
    String mTextureNameFromY;
    
    /// Texture sampler id mapping from y.
    ushort mTextureSamplerIndexFromY;
    
    /// Texturename mapping from z.
    String mTextureNameFromZ;
    
    /// Texture sampler id mapping from z.
    ushort mTextureSamplerIndexFromZ;

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

public:
    
    /// The type.
    static String type;

    /** Sets the parameters.
    @param parameters
        Parameters: Texturescale, Plateau-size (0 to 0.57, not bigger to avoid division by zero!), transition-speed.
    */
    virtual void setParameters(const Vector3 &parameters);

    /** Sets the texture names for the mapping.
    @param textureNameFromX
        The texture mapping from x.
    @param textureNameFromY
        The texture mapping from y.
    @param textureNameFromZ
        The texture mapping from z.
    */
    virtual void setTextureNames(const String &textureNameFromX, const String &textureNameFromY, const String &textureNameFromZ);
    
    /** 
    @see SubRenderState::getType.
    */
    virtual const String& getType() const;

    /** 
    @see SubRenderState::getExecutionOrder.
    */
    virtual int getExecutionOrder() const;

    /** 
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool preAddToRenderState (const RenderState* renderState, Pass* srcPass, Pass* dstPass);

    /** 
    @see SubRenderState::copyFrom.
    */
    virtual void copyFrom(const SubRenderState& rhs);
    
    /** 
    @see SubRenderState::updateGpuProgramsParams.
    */
    virtual void updateGpuProgramsParams(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

};


/** 
A factory that enables creation of TriplanarMapping instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport TriplanarTexturingFactory : public SubRenderStateFactory
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
