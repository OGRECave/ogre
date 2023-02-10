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


/** \addtogroup Optional
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
    
    static String Type;

// Protected types:
protected:
    
    // Per texture unit parameters.
    struct _OgreRTSSExport TextureUnitParams
    {
        // Texture unit state.
        TextureUnitState* mTextureUnitState;
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
        // Vertex shader input texture coordinates parameter.
        ParameterPtr mVSInputTexCoord;
        // Vertex shader output texture coordinates parameter.
        ParameterPtr mVSOutputTexCoord;
        // Pixel shader input texture coordinates parameter.
        ParameterPtr mPSInputTexCoord;
    };

    typedef std::vector<TextureUnitParams>         TextureUnitParamsList;
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
    bool resolveParameters(ProgramSet* programSet) override;

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
    bool resolveDependencies(ProgramSet* programSet) override;

    /** 
    @see SubRenderState::addFunctionInvocations.
    */
    bool addFunctionInvocations(ProgramSet* programSet) override;


    /** 
    Internal method that adds vertex shader functions invocations.
    */
    bool addVSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* vsMain);

    /** 
    Internal method that adds pixel shader functions invocations.
    */
    bool addPSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* psMain);

    /** 
    Adds the fragment shader code which samples the texel color in the texture
    */
    virtual void addPSSampleTexelInvocation(TextureUnitParams* textureUnitParams, Function* psMain, 
        const ParameterPtr& texel, int groupOrder);

    ParameterPtr getPSArgument(ParameterPtr texel, LayerBlendSource blendSrc, const ColourValue& colourValue,
                               Real alphaValue, bool isAlphaArgument) const;

    virtual void addPSBlendInvocations(Function* psMain, ParameterPtr arg1, ParameterPtr arg2,
                ParameterPtr texel,int samplerIndex, const LayerBlendModeEx& blendMode,
                const int groupOrder, Operand::OpMask targetChannels);
    
    /** 
    Determines if the given texture unit state need to use texture transformation matrix.
    */
    bool needsTextureMatrix(TextureUnitState* textureUnitState);

    bool setParameter(const String& name, const String& value) override;

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

    bool mIsPointSprite;
    bool mLateAddBlend;
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

