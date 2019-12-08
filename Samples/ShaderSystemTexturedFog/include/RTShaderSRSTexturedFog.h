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
#ifndef _ShaderSRSTexturedFog_
#define _ShaderSRSTexturedFog_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreVector.h"

using namespace Ogre;
using namespace RTShader;

class RTShaderSRSTexturedFogFactory;
/** Textured Fog sub render state implementation.
This class implements a sub render state which is viewed as fog like effect. However, unlike a regular for
that as objects get further they become a single color, in this implementation they take the texture of a background image. 

Derives from SubRenderState class.
*/
class RTShaderSRSTexturedFog : public SubRenderState
{
public:
// Interface.
public:

    /** Class default constructor */
    RTShaderSRSTexturedFog(RTShaderSRSTexturedFogFactory* factory = NULL);

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

    /** 
    Set the fog properties this fog sub render state should emulate.
    @param fogMode The fog mode to emulate (FOG_NONE, FOG_EXP, FOG_EXP2, FOG_LINEAR).
    @param fogStart Start distance of fog, used for linear mode only.
    @param fogEnd End distance of fog, used for linear mode only.
    @param fogDensity Fog density used in exponential modes only.
    */
    void setFogProperties(FogMode fogMode, float fogStart, float fogEnd, float fogDensity);

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
    /// The factory which created the texture fog instance
    RTShaderSRSTexturedFogFactory* mFactory;    
    /// Fog formula. 
    FogMode mFogMode;               
    /// Fog parameters (density, start, end, 1/end-start).
    Vector4 mFogParamsValue;        
    /// True if the fog parameters should be taken from the pass.
    bool mPassOverrideParams;   
    /// The index of the background texture unit state in the pass 
    unsigned int mBackgroundSamplerIndex;

    // World matrix parameter.  
    UniformParameterPtr mWorldMatrix;               
    // camera position parameter.       
    UniformParameterPtr mCameraPos;             
    // Fog parameters program parameter.    
    UniformParameterPtr mFogParams;             
    // Vertex shader input position parameter.
    ParameterPtr mVSInPos;              
    // Fog colour parameter.    
    ParameterPtr mFogColour;                
    // Vertex shader output fog colour parameter.
    ParameterPtr mVSOutFogFactor;       
    // Pixel shader input fog factor.
    ParameterPtr mPSInFogFactor;            
    // Vertex shader output depth.
    ParameterPtr mVSOutDepth;           
    // Pixel shader input depth.
    ParameterPtr mPSInDepth;                
    // Vertex shader world position relative to camera.
    ParameterPtr mVSOutPosView;         
    // Pixel shader world position relative to camera.
    ParameterPtr mPSInPosView;           
    // Pixel shader output diffuse colour.
    ParameterPtr mPSOutDiffuse;         
    //Background color texture parameter
    ParameterPtr mBackgroundTextureSampler; 
    
};


/** 
A factory that enables creation of RTShaderSRSTexturedFog instances.
@remarks Sub class of SubRenderStateFactory
*/
class RTShaderSRSTexturedFogFactory : public SubRenderStateFactory
{
public:

    /** 
    @see SubRenderStateFactory::getType.
    */
    virtual const String&   getType() const;

    /** Set the name of the texture to use as a background for the fog */
    const String& getBackgroundTextureName() const { return mBackgroundTextureName; }
    /** Return the name of the texture used as a background for the fog */
    void setBackgroundTextureName(const String& name) { mBackgroundTextureName = name; }
    
protected:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    virtual SubRenderState* createInstanceImpl();

private:
    String mBackgroundTextureName;
};

#endif

