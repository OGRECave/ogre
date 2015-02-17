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
#include "OgreShaderSubRenderState.h"
#include "OgreLight.h"
#include "OgreCommon.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

#define SGX_LIB_NORMALMAPLIGHTING                   "SGXLib_NormalMapLighting"
#define SGX_FUNC_CONSTRUCT_TBNMATRIX                "SGX_ConstructTBNMatrix"
#define SGX_FUNC_TRANSFORMNORMAL                    "SGX_TransformNormal"
#define SGX_FUNC_TRANSFORMPOSITION                  "SGX_TransformPosition"
#define SGX_FUNC_FETCHNORMAL                        "SGX_FetchNormal"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSE          "SGX_Light_Directional_Diffuse"
#define SGX_FUNC_LIGHT_DIRECTIONAL_DIFFUSESPECULAR  "SGX_Light_Directional_DiffuseSpecular"
#define SGX_FUNC_LIGHT_POINT_DIFFUSE                "SGX_Light_Point_Diffuse"
#define SGX_FUNC_LIGHT_POINT_DIFFUSESPECULAR        "SGX_Light_Point_DiffuseSpecular"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSE                 "SGX_Light_Spot_Diffuse"
#define SGX_FUNC_LIGHT_SPOT_DIFFUSESPECULAR         "SGX_Light_Spot_DiffuseSpecular"

/** Normal Map Lighting extension sub render state implementation.
Derives from SubRenderState class.
*/
class _OgreRTSSExport NormalMapLighting : public SubRenderState
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
    Set the index of the input vertex shader texture coordinate set 
    */
    void setTexCoordIndex(unsigned int index) { mVSTexCoordSetIndex = index;}

    /** 
    Return the index of the input vertex shader texture coordinate set.
    */
    unsigned int getTexCoordIndex() const { return mVSTexCoordSetIndex; }

    // Type of this render state.
    static String Type;

    // Normal map space definition.
    enum NormalMapSpace
    {
        NMS_TANGENT,        // Normal map contains normal data in tangent space.
                            // This is the default normal mapping behavior and it requires that the
                            // target mesh will have valid tangents within its vertex data.
        
        NMS_OBJECT          // Normal map contains normal data in object local space.
                            // This normal mapping technique has the advantages of better visualization results,
                            // lack of artifacts that comes from texture mirroring usage, it doesn't requires tangent
                            // and it also saves some instruction in the vertex shader stage.
                            // The main drawback of using this kind of normal map is that the target object must be static
                            // in terms of local space rotations and translations.
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



// Protected types:
protected:
    
    // Per light parameters.
    struct _OgreRTSSExport LightParams
    {
        // Light type.
        Light::LightTypes mType;
        // Light position.
        UniformParameterPtr mPosition;
        // Vertex shader output vertex position to light position direction (texture space).
        ParameterPtr mVSOutToLightDir;
        // Pixel shader input vertex position to light position direction (texture space).
        ParameterPtr mPSInToLightDir;
        // Light direction.
        UniformParameterPtr mDirection;
        // Vertex shader output light direction (texture space).
        ParameterPtr mVSOutDirection;
        // Pixel shader input light direction (texture space).      
        ParameterPtr mPSInDirection;
        // Attenuation parameters.
        UniformParameterPtr mAttenuatParams;
        // Spot light parameters.
        UniformParameterPtr mSpotParams;
        // Diffuse colour.
        UniformParameterPtr mDiffuseColour;
        // Specular colour.
        UniformParameterPtr mSpecularColour;

    };

    typedef vector<LightParams>::type LightParamsList;
    typedef LightParamsList::iterator LightParamsIterator;
    typedef LightParamsList::const_iterator LightParamsConstIterator;

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
    bool addVSInvocation(Function* vsMain, const int groupOrder, int& internalCounter);

    /** 
    Internal method that adds per light illumination component functions invocations.
    */
    bool addVSIlluminationInvocation(LightParams* curLightParams, Function* vsMain, const int groupOrder, int& internalCounter);

    /** 
    Internal method that perform normal fetch invocation.
    */
    bool addPSNormalFetchInvocation(Function* psMain, const int groupOrder, int& internalCounter);


    /** 
    Internal method that adds global illumination component functions invocations.
    */
    bool addPSGlobalIlluminationInvocation(Function* psMain, const int groupOrder, int& internalCounter);

    /** 
    Internal method that adds per light illumination component functions invocations.
    */
    bool addPSIlluminationInvocation(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter);

    /** 
    Internal method that adds the final colour assignments.
    */
    bool addPSFinalAssignmentInvocation(Function* psMain, const int groupOrder, int& internalCounter);


// Attributes.
protected:
    // The normal map texture name.
    String mNormalMapTextureName;
    // Track per vertex colour type.
    TrackVertexColourType mTrackVertexColourType;
    // Specular component enabled/disabled.
    bool mSpecularEnable;
    // Light list.
    LightParamsList mLightParamsList;
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
    // Vertex shader input position parameter.
    ParameterPtr mVSInPosition;
    // Vertex shader world position parameter.
    ParameterPtr mVSWorldPosition;
    // Vertex shader output view vector (position in camera space) parameter.
    ParameterPtr mVSOutView;
    // Pixel shader input view position (position in camera space) parameter.
    ParameterPtr mPSInView;
    // Vertex shader input normal.
    ParameterPtr mVSInNormal;
    // Vertex shader input tangent.
    ParameterPtr mVSInTangent;
    // Vertex shader local TNB matrix.
    ParameterPtr mVSTBNMatrix;
    // Vertex shader local light direction.
    ParameterPtr mVSLocalDir;
    // Normal map texture sampler parameter.
    UniformParameterPtr mNormalMapSampler;
	// Normal map texture sampler state parameter.
	UniformParameterPtr mNormalMapSamplerState;
    // Pixel shader normal parameter.
    ParameterPtr mPSNormal;
    // Vertex shader input texture coordinates.
    ParameterPtr mVSInTexcoord;
    // Vertex shader output texture coordinates.
    ParameterPtr mVSOutTexcoord;
    // Pixel shader input texture coordinates.
    ParameterPtr mPSInTexcoord;
    // Pixel shader temporary diffuse calculation parameter.
    ParameterPtr mPSTempDiffuseColour;
    // Pixel shader temporary specular calculation parameter.
    ParameterPtr mPSTempSpecularColour;
    // Pixel shader input/local diffuse parameter.  
    ParameterPtr mPSDiffuse;
    // Pixel shader input/local specular parameter. 
    ParameterPtr mPSSpecular;
    // Pixel shader output diffuse parameter.   
    ParameterPtr mPSOutDiffuse;
    // Pixel shader output specular parameter.  
    ParameterPtr mPSOutSpecular;
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

