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
#ifndef _ShaderExNormalMapLighting_
#define _ShaderExNormalMapLighting_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderExPerPixelLighting.h"
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

#define SGX_LIB_NORMALMAPLIGHTING                   "SGXLib_NormalMapLighting"
#define SGX_FUNC_CONSTRUCT_TBNMATRIX                "SGX_ConstructTBNMatrix"
#define SGX_FUNC_FETCHNORMAL                        "SGX_FetchNormal"

/** Normal Map Lighting extension sub render state implementation.
Derives from SubRenderState class.
*/
class _OgreRTSSExport NormalMapLighting : public PerPixelLighting
{

// Interface.
public:
    /** Class default constructor */    
    NormalMapLighting();

    /** 
    @see SubRenderState::getType.
    */
    virtual const String& getType() const;

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
    Set the index of the input vertex shader texture coordinate set 
    */
    void setTexCoordIndex(unsigned int index) { mVSTexCoordSetIndex = index;}

    /** 
    Return the index of the input vertex shader texture coordinate set.
    */
    unsigned int getTexCoordIndex() const { return mVSTexCoordSetIndex; }

    // Type of this render state.
    static String Type;

    /// Normal map space definition.
    enum NormalMapSpace
    {
        /**
        Normal map contains normal data in tangent space.
        This is the default normal mapping behavior and it requires that the
        target mesh will have valid tangents within its vertex data.
         */
        NMS_TANGENT,
        /**
        Normal map contains normal data in object local space.
        This normal mapping technique has the advantages of better visualization results,
        lack of artifacts that comes from texture mirroring usage, it doesn't requires tangent
        and it also saves some instruction in the vertex shader stage.
        The main drawback of using this kind of normal map is that the target object must be static
        in terms of local space rotations and translations.
         */
        NMS_OBJECT
    };

    /** 
    Set the normal map space.
    @param normalMapSpace The normal map space.
    */
    void setNormalMapSpace(NormalMapSpace normalMapSpace) { mNormalMapSpace = normalMapSpace; }

    /** Return the normal map space. */
    NormalMapSpace getNormalMapSpace() const { return mNormalMapSpace; }

    /** 
    Set the normal map texture name.
    */
    void setNormalMapTextureName(const String& textureName) { mNormalMapTextureName = textureName; }

    /** 
    Return the normal map texture name.
    */
    const String& getNormalMapTextureName() const { return mNormalMapTextureName; }

    /** 
    Set the normal map filtering attributes.
    @param minFilter The desired min filter.
    @param magFilter The desired mag filter.
    @param mipFilter The desired mip filter.
    */
    void setNormalMapFiltering(const FilterOptions minFilter, const FilterOptions magFilter, const FilterOptions mipFilter) 
    { mNormalMapMinFilter = minFilter; mNormalMapMagFilter = magFilter; mNormalMapMipFilter = mipFilter; }

    /** 
    Return the normal map filtering attributes.
    @param minFilter The desired min filter.
    @param magFilter The desired mag filter.
    @param mipFilter The desired mip filter.
    */
    void getNormalMapFiltering(FilterOptions& minFilter, FilterOptions& magFilter, FilterOptions& mipFilter) const
    { minFilter = mNormalMapMinFilter; magFilter = mNormalMapMagFilter ; mipFilter = mNormalMapMipFilter; }

    /** Setup the normal map anisotropy value. 
    @param anisotropy The anisotropy value.
    */
    void setNormalMapAnisotropy(unsigned int anisotropy) { mNormalMapAnisotropy = anisotropy; }


    /** Return the normal map anisotropy value. */
    unsigned int getNormalMapAnisotropy() const { return mNormalMapAnisotropy; }

    
    /** Setup the normal map map mip bias value. 
    @param mipBias The map mip bias value.
    */
    void setNormalMapMipBias(Real mipBias) { mNormalMapMipBias = mipBias; }


    /** Return the normal map mip bias value. */
    Real getNormalMapMipBias() const { return mNormalMapMipBias; }

// Protected methods
protected:
    /** Resolve global lighting parameters */
    bool resolveGlobalParameters(ProgramSet* programSet);

    /** Resolve per light parameters */
    bool resolvePerLightParameters(ProgramSet* programSet);

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
    Internal method that adds per light illumination component functions invocations.
    */
    bool addVSIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder);

    /** 
    Internal method that perform normal fetch invocation.
    */
    bool addPSNormalFetchInvocation(Function* psMain, const int groupOrder);

    /** 
    Internal method that adds per light illumination component functions invocations.
    */
    bool addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder);

// Attributes.
protected:  
    // The normal map texture name.
    String mNormalMapTextureName;
    // Normal map texture sampler index.
    unsigned short mNormalMapSamplerIndex;
    // Vertex shader input texture coordinate set index.
    unsigned int mVSTexCoordSetIndex;
    // The normal map min filter.
    FilterOptions mNormalMapMinFilter;
    // The normal map mag filter.
    FilterOptions mNormalMapMagFilter;
    // The normal map mip filter.
    FilterOptions mNormalMapMipFilter;
    // The normal map max anisotropy value.
    unsigned int mNormalMapAnisotropy;
    // The normal map mip map bias.
    Real mNormalMapMipBias;
    // The normal map space.
    NormalMapSpace mNormalMapSpace;
    // World matrix parameter.
    UniformParameterPtr mWorldMatrix;
    // World matrix inverse rotation matrix parameter.
    UniformParameterPtr mWorldInvRotMatrix;
    // Camera position in world space parameter.    
    UniformParameterPtr mCamPosWorldSpace;
    // Vertex shader world position parameter.
    ParameterPtr mVSWorldPosition;
    // Vertex shader output view vector (position in camera space) parameter.
    ParameterPtr mVSOutView;
    // Pixel shader input view position (position in camera space) parameter.
    ParameterPtr mPSInView;
    // Vertex shader input tangent.
    ParameterPtr mVSInTangent;
    // Vertex shader local TNB matrix.
    ParameterPtr mVSTBNMatrix;
    // Vertex shader local light direction.
    ParameterPtr mVSLocalDir;
    // Normal map texture sampler parameter.
    UniformParameterPtr mNormalMapSampler;
    // Pixel shader normal parameter.
    ParameterPtr mPSNormal;
    // Vertex shader input texture coordinates.
    ParameterPtr mVSInTexcoord;
    // Vertex shader output texture coordinates.
    ParameterPtr mVSOutTexcoord;
    // Pixel shader input texture coordinates.
    ParameterPtr mPSInTexcoord;
};


/** 
A factory that enables creation of NormalMapLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport NormalMapLightingFactory : public SubRenderStateFactory
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

