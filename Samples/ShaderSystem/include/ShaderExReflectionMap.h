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
#ifndef _ShaderExReflectionMap_
#define _ShaderExReflectionMap_

#include "OgreRTShaderSystem.h"


using namespace Ogre;
using namespace Ogre::RTShader;


/** Reflection map sub render state implementation.
It will use as extension for the RT Shader System.
This extension is based on reflection map + mask effect that exists in
3D studio max material system.
The effect uses 2 input textures - a mask texture and reflection texture.
The vertex shader responsible to output two texture coordinates sets:
1. 2D texcoord set that will use to sample to mask texture.
2. 2D/3D texcoord set that will use to sample the reflection map.
The pixel shader will sample both textures, multiply the results 
and add to the output diffuse color.
*/
class ShaderExReflectionMap : public SubRenderState
{

// Interface.
public:
    /** Class default constructor */    
    ShaderExReflectionMap();
    
    /** 
    @see SubRenderState::getType.
    */
    const String&   getType             () const override;

    /** 
    @see SubRenderState::getExecutionOrder.
    */
    int             getExecutionOrder       () const override;

    /** 
    @see SubRenderState::copyFrom.
    */
    void            copyFrom                (const SubRenderState& rhs) override;

    /** 
    @see SubRenderState::preAddToRenderState.
    */
    bool            preAddToRenderState     (const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;

    /** 
    @see SubRenderState::updateGpuProgramsParams.
    */
    void            updateGpuProgramsParams (Renderable* rend, const Pass* pass,  const AutoParamDataSource* source,  const LightList* pLightList) override;

    /** Sets the reflection map type. */
    void                    setReflectionMapType    (TextureType type);

    /** Get the reflection map type. */
    TextureType             getReflectionMapType    () const { return mReflectionMapType; }

    /** Set the reflection map power. */
    void                    setReflectionPower      (const Real reflectionPower);
    
    /** Return the reflection map power. */
    Real                    getReflectionPower      () const { return mReflectionPowerValue; }

    /** 
    Set the reflection map texture name.
    */
    void                    setReflectionMapTextureName     (const String& textureName) { mReflectionMapTextureName = textureName; }

    /** 
    Return the reflection map texture name.
    */
    const String&           getReflectionMapTextureName     () const { return mReflectionMapTextureName; }


    /** 
    Set the mask map texture name.
    */
    void                    setMaskMapTextureName       (const String& textureName) { mMaskMapTextureName = textureName; }

    /** 
    Return the mask map texture name.
    */
    const String&           getMaskMapTextureName       () const { return mMaskMapTextureName; }


    static String Type;
    
// Protected methods.
protected:
    

    /** 
    @see SubRenderState::resolveParameters.
    */
    bool            resolveParameters       (ProgramSet* programSet) override;

    /** 
    @see SubRenderState::resolveDependencies.
    */
    bool            resolveDependencies     (ProgramSet* programSet) override;

    /** 
    @see SubRenderState::addFunctionInvocations.
    */
    bool            addFunctionInvocations  (ProgramSet* programSet) override;

    /** 
    Internal method that adds related vertex shader functions invocations.
    */
    bool            addVSInvocations                (const FunctionStageRef& stage);


    /** 
    Internal method that adds related pixel shader functions invocations.
    */
    bool            addPSInvocations                (const FunctionStageRef& stage);


// Attributes.
protected:  
    String                  mReflectionMapTextureName;          // The reflection map texture name.
    String                  mMaskMapTextureName;                // The mask map texture name.
    unsigned short          mMaskMapSamplerIndex;               // Mask map texture sampler index.
    unsigned short          mReflectionMapSamplerIndex;         // Reflection map texture sampler index.
    Real                    mReflectionPowerValue;              // The reflection power value.
    bool                    mReflectionPowerChanged;            // Indicate if reflection power changed.
    TextureType             mReflectionMapType;                 // Reflection map type. Valid are 2D or Cube.
    UniformParameterPtr     mMaskMapSampler;                    // Normal map texture sampler parameter.
    UniformParameterPtr     mReflectionMapSampler;              // Reflection map texture sampler parameter.
    UniformParameterPtr     mReflectionPower;                   // Reflection map power.
    ParameterPtr            mVSInMaskTexcoord;                  // Vertex shader input mask texture coordinates.
    ParameterPtr            mVSOutMaskTexcoord;                 // Vertex shader output mask texture coordinates.
    ParameterPtr            mVSOutReflectionTexcoord;           // Vertex shader output reflection texture coordinates.
    ParameterPtr            mPSInMaskTexcoord;                  // Pixel shader input mask texture coordinates.
    ParameterPtr            mPSInReflectionTexcoord;            // Pixel shader input reflection texture coordinates.
    UniformParameterPtr     mWorldMatrix;                       // World matrix parameter.
    UniformParameterPtr     mWorldITMatrix;                     // World inverse transpose matrix parameter.
    UniformParameterPtr     mViewMatrix;                        // View matrix parameter.           
    ParameterPtr            mVSInputNormal;                     // Vertex shader input normal parameter.
    ParameterPtr            mVSInputPos;                        // Vertex shader input position parameter.      
    ParameterPtr            mPSOutDiffuse;                      // Pixel shader output colour.
};


/** 
A factory that enables creation of ShaderExReflectionMap instances.
@remarks Sub class of SubRenderStateFactory
*/
class ShaderExReflectionMapFactory : public SubRenderStateFactory
{
public:

    /** 
    @see SubRenderStateFactory::getType.
    */
    const String&   getType             () const override;

    /** 
    @see SubRenderStateFactory::createInstance.
    */
    SubRenderState* createInstance      (ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator) override;

    /** 
    @see SubRenderStateFactory::writeInstance.
    */
    void            writeInstance       (MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass) override;


protected:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    SubRenderState* createInstanceImpl  () override;



};



#endif

