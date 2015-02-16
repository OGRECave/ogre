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
#ifndef _ShaderFFPTextureStage_
#define _ShaderFFPTextureStage_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderSubRenderState.h"
#include "OgreRenderSystem.h"

namespace Ogre {
namespace RTShader {


/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Texturing sub render state implementation of the Fixed Function Pipeline.
Implements texture coordinate processing:
@see http://msdn.microsoft.com/en-us/library/bb206247.aspx
Implements texture blending operation:
@see http://msdn.microsoft.com/en-us/library/bb206241.aspx
Derives from SubRenderState class.
*/
class _OgreRTSSExport FFPTexturing : public SubRenderState
{

// Interface.
public:

    /** Class default constructor */
    FFPTexturing();

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
    
    static void AddTextureSampleWrapperInvocation(UniformParameterPtr textureSampler,UniformParameterPtr textureSamplerState,
        GpuConstantType samplerType, Function* function, int groupOrder, int& internalCounter);

    
	static ParameterPtr GetSamplerWrapperParam(UniformParameterPtr sampler, Function* function);

    static String Type;

// Protected types:
protected:
    
    // Per texture unit parameters.
    struct _OgreRTSSExport TextureUnitParams
    {
        // Texture unit state.
        TextureUnitState* mTextureUnitState;
        // Texture projector.
        const Frustum* mTextureProjector;
        // Texture sampler index.
        unsigned short mTextureSamplerIndex;
        // Texture sampler index.
        GpuConstantType mTextureSamplerType;
        // Vertex shader input texture coordinate type.
        GpuConstantType mVSInTextureCoordinateType;
        // Vertex shader output texture coordinates type.
        GpuConstantType mVSOutTextureCoordinateType;
        // Texture coordinates calculation method.
        TexCoordCalcMethod mTexCoordCalcMethod;
        // Texture matrix parameter.
        UniformParameterPtr mTextureMatrix;
        // Texture View Projection Image space matrix parameter.
        UniformParameterPtr mTextureViewProjImageMatrix;
        // Texture sampler parameter.
        UniformParameterPtr mTextureSampler;
    // Texture sampler state parameter.
        UniformParameterPtr mTextureSamplerState;
        // Vertex shader input texture coordinates parameter.
        ParameterPtr mVSInputTexCoord;
        // Vertex shader output texture coordinates parameter.
        ParameterPtr mVSOutputTexCoord;
        // Pixel shader input texture coordinates parameter.
        ParameterPtr mPSInputTexCoord;
    };

    typedef vector<TextureUnitParams>::type         TextureUnitParamsList;
    typedef TextureUnitParamsList::iterator         TextureUnitParamsIterator;
    typedef TextureUnitParamsList::const_iterator   TextureUnitParamsConstIterator;

// Protected methods
protected:

    /** 
    Set the number of texture units this texturing sub state has to handle.
    @param count The number of texture unit states.
    */
    void setTextureUnitCount(size_t count);

    /** 
    Return the number of texture units this sub state handle. 
    */
    size_t getTextureUnitCount() const { return mTextureUnitParamsList.size(); }

    /** 
    Set texture unit of a given stage index.
    @param index The stage index of the given texture unit state.
    @param textureUnitState The texture unit state to bound the the stage index.
    */
    void setTextureUnit(unsigned short index, TextureUnitState* textureUnitState);

    /** 
    @see SubRenderState::resolveParameters.
    */
    virtual bool resolveParameters(ProgramSet* programSet);

    /** 
    Internal method that resolves uniform parameters of the given texture unit parameters.
    */
    bool resolveUniformParams(TextureUnitParams* textureUnitParams, ProgramSet* programSet);

    /** 
    Internal method that resolves functions parameters of the given texture unit parameters.
    */
    bool resolveFunctionsParams(TextureUnitParams* textureUnitParams, ProgramSet* programSet);

    /** 
    @see SubRenderState::resolveDependencies.
    */
    virtual bool resolveDependencies(ProgramSet* programSet);

    /** 
    @see SubRenderState::addFunctionInvocations.
    */
    virtual bool addFunctionInvocations(ProgramSet* programSet);


    /** 
    Internal method that adds vertex shader functions invocations.
    */
    bool addVSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* vsMain);

    /** 
    Internal method that adds pixel shader functions invocations.
    */
    bool addPSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* psMain, int& internalCounter);

    /** 
    Adds the fragment shader code which samples the texel color in the texture
    */
    virtual void addPSSampleTexelInvocation(TextureUnitParams* textureUnitParams, Function* psMain, 
        const ParameterPtr& texel, int groupOrder, int& internalCounter);

    virtual void addPSArgumentInvocations(Function* psMain, ParameterPtr arg, ParameterPtr texel,
                int samplerIndex, LayerBlendSource blendSrc, const ColourValue& colourValue, Real alphaValue,
                 bool isAlphaArgument, const int groupOrder, int& internalCounter);

    virtual void addPSBlendInvocations(Function* psMain, ParameterPtr arg1, ParameterPtr arg2,
                ParameterPtr texel,int samplerIndex, const LayerBlendModeEx& blendMode,
                const int groupOrder, int& internalCounter, int targetChannels);
    
    /** 
    Determines the texture coordinates calculation method of the given texture unit state.
    */
    TexCoordCalcMethod getTexCalcMethod(TextureUnitState* textureUnitState);

    /** 
    Determines if the given texture unit state need to use texture transformation matrix.
    */
    bool needsTextureMatrix(TextureUnitState* textureUnitState);

    /** 
    Determines whether a given texture unit needs to be processed by this srs
    */
    virtual bool isProcessingNeeded(TextureUnitState* texUnitState);


// Attributes.
protected:
    // Texture units list.      
    TextureUnitParamsList mTextureUnitParamsList;
    // World matrix parameter.
    UniformParameterPtr mWorldMatrix;
    // World inverse transpose matrix parameter.
    UniformParameterPtr mWorldITMatrix;
    // View matrix parameter.           
    UniformParameterPtr mViewMatrix;
    // Vertex shader input normal parameter.
    ParameterPtr mVSInputNormal;
    // Vertex shader input position parameter.      
    ParameterPtr mVSInputPos;
    // Pixel shader output colour.
    ParameterPtr mPSOutDiffuse;
    // Pixel shader diffuse colour.
    ParameterPtr mPSDiffuse;
    // Pixel shader specular colour.
    ParameterPtr mPSSpecular;
};


/** 
A factory that enables creation of FFPTexturing instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport FFPTexturingFactory : public SubRenderStateFactory
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

