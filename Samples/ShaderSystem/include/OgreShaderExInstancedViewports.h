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
#ifndef _ShaderExInstancedViewports_
#define _ShaderExInstancedViewports_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreRTShaderSystem.h"
#include "OgreShaderSubRenderState.h"
namespace Ogre {
namespace RTShader {

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/


/**todo
*/
class ShaderExInstancedViewports : public SubRenderState
{
    friend class ShaderExInstancedViewportsFactory;
    bool mOwnsGlobalData; // only true for template sub render state
// Interface.
public:
    /** Class default constructor */    
    ShaderExInstancedViewports();
    ~ShaderExInstancedViewports();
    
    /** 
    @see SubRenderState::getType.
    */
    virtual const String&   getType             () const;

    /** 
    @see SubRenderState::getExecutionOrder.
    */
    virtual int             getExecutionOrder       () const;

    /** 
    @see SubRenderState::copyFrom.
    */
    virtual void            copyFrom                (const SubRenderState& rhs);

    /** 
    @see SubRenderState::preAddToRenderState.
    */
    virtual bool            preAddToRenderState     (const RenderState* renderState, Pass* srcPass, Pass* dstPass);

    /** 
    @see SubRenderState::updateGpuProgramsParams.
    */
    virtual void            updateGpuProgramsParams (Renderable* rend, const Pass* pass,  const AutoParamDataSource* source,  const LightList* pLightList);


    /** Set the monitors count. */
    void                    setMonitorsCount        (const Vector2 monitorsCount);
    
    /** Return the monitors count. */
    Vector2                 getMonitorsCount        () const { return mMonitorsCount; }

    static String Type;

// Protected methods.
protected:
    

    /** 
    @see SubRenderState::resolveParameters.
    */
    virtual bool            resolveParameters       (ProgramSet* programSet);

    /** 
    @see SubRenderState::resolveDependencies.
    */
    virtual bool            resolveDependencies     (ProgramSet* programSet);

    /** 
    @see SubRenderState::addFunctionInvocations.
    */
    virtual bool            addFunctionInvocations  (ProgramSet* programSet);

    /** 
    Internal method that adds related vertex shader functions invocations.
    */
    bool            addVSInvocations                (Function* vsMain, const int groupOrder);


    /** 
    Internal method that adds related pixel shader functions invocations.
    */
    bool            addPSInvocations                (Function* psMain, const int groupOrder);


// Attributes.
protected:  
    ParameterPtr            mVSInPosition;      // Vertex shader original input position in projective space.
    ParameterPtr            mVSOriginalOutPositionProjectiveSpace;      // Vertex shader original output position in projective space.
    ParameterPtr            mVSOutPositionProjectiveSpace;      // Vertex shader output texcord position in projective space.
    ParameterPtr            mPSInPositionProjectiveSpace;       // Pixel shader input position in projective space.
    UniformParameterPtr     mVSInMonitorsCount;                 // Vertex shader uniform monitors count.        
    UniformParameterPtr     mPSInMonitorsCount;                 // Pixel shader uniform monitors count.     
    ParameterPtr            mVSInMonitorIndex;                  // Vertex shader uniform monitor index.     
    ParameterPtr            mVSOutMonitorIndex;                 // Vertex shader output monitor index.      
    ParameterPtr            mPSInMonitorIndex;                  // Pixel shader input monitor index.    

    ParameterPtr            mVSInViewportOffsetMatrixR0;    
    ParameterPtr            mVSInViewportOffsetMatrixR1;    
    ParameterPtr            mVSInViewportOffsetMatrixR2;    
    ParameterPtr            mVSInViewportOffsetMatrixR3;    

    UniformParameterPtr     mWorldViewMatrix;                       // world & view parameter.
    UniformParameterPtr     mProjectionMatrix;                      // projection parameter.

    Vector2                 mMonitorsCount;
    bool                    mMonitorsCountChanged;

};


/** 
A factory that enables creation of ShaderExInstancedViewports instances.
@remarks Sub class of SubRenderStateFactory
*/
class ShaderExInstancedViewportsFactory : public SubRenderStateFactory
{
public:

    /** 
    @see SubRenderStateFactory::getType.
    */
    virtual const String&   getType             () const;

    /** 
    @see SubRenderStateFactory::createInstance.
    */
    virtual SubRenderState* createInstance      (ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);

    /** 
    @see SubRenderStateFactory::writeInstance.
    */
    virtual void            writeInstance       (MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass);

protected:

    /** 
    @see SubRenderStateFactory::createInstanceImpl.
    */
    virtual SubRenderState* createInstanceImpl  ();



};


}
}

#endif
#endif

