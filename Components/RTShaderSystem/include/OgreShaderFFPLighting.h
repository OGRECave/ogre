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
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
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
class _OgreRTSSExport FFPLighting : public SubRenderState
{

// Interface.
public:
    
    /** Class default constructor */
    FFPLighting();

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


    static String Type;

// Protected types:
protected:

    // Per light parameters.
    struct _OgreRTSSExport LightParams
    {
        Light::LightTypes       mType;              // Light type.      
        // Light position.
        UniformParameterPtr mPosition;
        // Light direction.
        UniformParameterPtr mDirection;
        // Attenuation parameters.
        UniformParameterPtr mAttenuatParams;
        // Spot light parameters.
        UniformParameterPtr mSpotParams;
        // Diffuse colour.
        UniformParameterPtr mDiffuseColour;
        // Specular colour.
        UniformParameterPtr mSpecularColour;

    };

    typedef std::vector<LightParams>               LightParamsList;
    typedef LightParamsList::iterator               LightParamsIterator;
    typedef LightParamsList::const_iterator         LightParamsConstIterator;

// Protected methods
protected:
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
    Set the light count per light type that this sub render state will generate.
    @see ShaderGenerator::setLightCount.
    */
    void setLightCount(const int lightCount[3]);

    /** 
    Get the light count per light type that this sub render state will generate.
    @see ShaderGenerator::getLightCount.
    */
    void getLightCount(int lightCount[3]) const;

    /** 
    Set the specular component state. If set to true this sub render state will compute a specular
    lighting component in addition to the diffuse component.
    @param enable Pass true to enable specular component computation.
    */
    void setSpecularEnable(bool enable) { mSpecularEnable = enable; }

    /** 
    Get the specular component state. 
    */
    bool getSpecularEnable() const    { return mSpecularEnable; }

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
    Internal method that adds global illumination component functions invocations.
    */
    bool addGlobalIlluminationInvocation(Function* vsMain, const int groupOrder);
            
    /** 
    Internal method that adds per light illumination component functions invocations.
    */
    bool addIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder);


// Attributes.
protected:  
    // Track per vertex colour type.
    TrackVertexColourType mTrackVertexColourType;
    // Specular component enabled/disabled.
    bool mSpecularEnable;
    // Light list.
    LightParamsList mLightParamsList;
    // World view matrix parameter.
    UniformParameterPtr mWorldViewMatrix;
    // World view matrix inverse transpose parameter.
    UniformParameterPtr mWorldViewITMatrix;
    // Vertex shader input position parameter.
    ParameterPtr mVSInPosition;
    // Vertex shader input normal.
    ParameterPtr mVSInNormal;
    // Vertex shader diffuse.
    ParameterPtr mVSDiffuse;
    // Vertex shader output diffuse colour parameter.
    ParameterPtr mVSOutDiffuse;
    // Vertex shader output specular colour parameter.
    ParameterPtr mVSOutSpecular;
    // Derived scene colour parameter.
    UniformParameterPtr mDerivedSceneColour;
    // Ambient light colour parameter.
    UniformParameterPtr mLightAmbientColour;
    // Derived ambient light colour parameter.
    UniformParameterPtr mDerivedAmbientLightColour;
    // Surface ambient colour parameter.
    UniformParameterPtr mSurfaceAmbientColour;
    // Surface diffuse colour parameter.
    UniformParameterPtr mSurfaceDiffuseColour;
    // Surface specular colour parameter.
    UniformParameterPtr mSurfaceSpecularColour;
    // Surface emissive colour parameter.
    UniformParameterPtr mSurfaceEmissiveColour;
    // Surface shininess parameter.
    UniformParameterPtr mSurfaceShininess;
    // Shared blank light.
    static Light msBlankLight;

};


/** 
A factory that enables creation of FFPLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport FFPLightingFactory : public SubRenderStateFactory
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
