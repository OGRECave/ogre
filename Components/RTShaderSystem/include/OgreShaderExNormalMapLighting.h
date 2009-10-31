/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreShaderParameter.h"
#include "OgreShaderSubRenderState.h"
#include "OgreVector4.h"
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

/** Normal Map Lighting extension sub render state implementation.
Derives from SubRenderState class.
*/
class NormalMapLighting : public SubRenderState
{

// Interface.
public:
	/** Class default constructor */	
	NormalMapLighting();

	/** 
	@see SubRenderState::getType.
	*/
	virtual const String&	getType					() const;

	/** 
	@see SubRenderState::getType.
	*/
	virtual int				getExecutionOrder		() const;

	/** 
	@see SubRenderState::getHashCode.
	*/
	virtual uint32			getHashCode				();

	/** 
	@see SubRenderState::updateGpuProgramsParams.
	*/
	virtual void			updateGpuProgramsParams	(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

	/** 
	@see SubRenderState::copyFrom.
	*/
	virtual void			copyFrom				(const SubRenderState& rhs);


	/** 
	@see SubRenderState::preAddToRenderState.
	*/
	virtual bool			preAddToRenderState		(RenderState* renderState, Pass* srcPass, Pass* dstPass);

	/** 
	Set the index of the input vertex shader texture coordinate set 
	*/
	void					setTexCoordIndex		(unsigned int index) { mVSTexCoordSetIndex = index;}

	/** 
	Return the index of the input vertex shader texture coordinate set.
	*/
	unsigned int			getTexCoordIndex		() const { return mVSTexCoordSetIndex; }

	// Type of this render state.
	static String Type;

	/// The normal map texture name key.	
	static String NormalMapTextureNameKey;

	// Normal map space definition.
	enum NormalMapSpace
	{
		NMS_TANGENT,		// Normal map contains normal data in tangent space.
							// This is the default normal mapping behavior and it requires that the
							// target mesh will have valid tangents within its vertex data.
		
		NMS_OBJECT			// Normal map contains normal data in object local space.
							// This normal mapping technique has the advantages of better visualization results,
							// lack of artifacts that comes from texture mirroring usage, it doesn't requires tangent
							// and it also saves some instruction in the vertex shader stage.
							// The main drawback of using this kind of normal map is that the target object must be static
							// in terms of local space rotations and translations.
	};

	/** Set the normal map space.
	@param
	*/
	void					setNormalMapSpace			(NormalMapSpace normalMapSpace) { mNormalMapSpace = normalMapSpace; }

	/** Return the normal map space. */
	NormalMapSpace			getNormalMapSpace			() const { return mNormalMapSpace; }


// Protected types:
protected:
	
	// Per light parameters.
	struct LightParams
	{
		Light::LightTypes	mType;				// Light type.		
		ParameterPtr		mPosition;			// Light position.
		ParameterPtr		mVSOutToLightDir;	// Vertex shader output vertex position to light position direction (texture space).
		ParameterPtr		mPSInToLightDir;	// Pixel shader input vertex position to light position direction (texture space).
		ParameterPtr		mDirection;			// Light direction.
		ParameterPtr		mVSOutDirection;	// Vertex shader output light direction (texture space).
		ParameterPtr		mPSInDirection;		// Pixel shader input light direction (texture space).		
		ParameterPtr		mAttenuatParams;	// Attenuation parameters.
		ParameterPtr		mSpotParams;		// Spot light parameters.
		ParameterPtr		mDiffuseColour;		// Diffuse colour.
		ParameterPtr		mSpecularColour;	// Specular colour.

	};

	typedef vector<LightParams>::type				LightParamsList;
	typedef LightParamsList::iterator				LightParamsIterator;
	typedef LightParamsList::const_iterator			LightParamsConstIterator;

// Protected methods
protected:

	/** 
	Set the track per vertex colour type. Ambient, Diffuse, Specular and Emissive lighting components source
	can be the vertex colour component. To establish such a link one should provide the matching flags to this
	sub render state.
	*/
	void					setTrackVertexColourType(TrackVertexColourType type) { mTrackVertexColourType = type; }

	/** 
	Return the current track per vertex type.
	*/
	TrackVertexColourType	getTrackVertexColourType() const { return mTrackVertexColourType; }


	/** 
	Set the light count per light type that this sub render state will generate.
	@see ShaderGenerator::setLightCount.
	*/
	void					setLightCount			(const int lightCount[3]);

	/** 
	Get the light count per light type that this sub render state will generate.
	@see ShaderGenerator::getLightCount.
	*/
	void					getLightCount			(int lightCount[3]) const;
	/** 
	Set the specular component state. If set to true this sub render state will compute a specular
	lighting component in addition to the diffuse component.
	@param enable Pass true to enable specular component computation.
	*/
	void					setSpecularEnable		(bool enable) { mSpecularEnable = enable; }

	/** 
	Get the specular component state. 
	*/
	bool					getSpecularEnable		() const	  { return mSpecularEnable; }


	/** 
	@see SubRenderState::resolveParameters.
	*/
	virtual bool			resolveParameters			(ProgramSet* programSet);

	/** Resolve global lighting parameters */
	bool					resolveGlobalParameters		(ProgramSet* programSet);

	/** Resolve per light parameters */
	bool					resolvePerLightParameters	(ProgramSet* programSet);

	/** 
	@see SubRenderState::resolveDependencies.
	*/
	virtual bool			resolveDependencies		(ProgramSet* programSet);

	/** 
	@see SubRenderState::addFunctionInvocations.
	*/
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);
	

	/** 
	Internal method that adds related vertex shader functions invocations.
	*/
	bool			addVSInvocation						(Function* vsMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds per light illumination component functions invocations.
	*/
	bool			addVSIlluminationInvocation			(LightParams* curLightParams, Function* vsMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that perform normal fetch invocation.
	*/
	bool			addPSNormalFetchInvocation		(Function* psMain, const int groupOrder, int& internalCounter);


	/** 
	Internal method that adds global illumination component functions invocations.
	*/
	bool			addPSGlobalIlluminationInvocation	(Function* psMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds per light illumination component functions invocations.
	*/
	bool			addPSIlluminationInvocation		(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds the final colour assignments.
	*/
	bool			addPSFinalAssignmentInvocation	(Function* psMain, const int groupOrder, int& internalCounter);


// Attributes.
protected:	
	TrackVertexColourType	mTrackVertexColourType;			// Track per vertex colour type.
	bool					mSpecularEnable;				// Specular component enabled/disabled.
	LightParamsList			mLightParamsList;				// Light list.
	unsigned short			mNormalMapSamplerIndex;			// Normal map texture sampler index.
	unsigned int			mVSTexCoordSetIndex;			// Vertex shader input texture coordinate set index.
	NormalMapSpace			mNormalMapSpace;				// The normal map space.
	ParameterPtr			mWorldMatrix;					// World matrix parameter.
	ParameterPtr			mWorldInvRotMatrix;				// World matrix inverse rotation matrix parameter.
	ParameterPtr			mCamPosWorldSpace;				// Camera position in world space parameter.	
	ParameterPtr			mVSInPosition;					// Vertex shader input position parameter.
	ParameterPtr			mVSWorldPosition;				// Vertex shader world position parameter.
	ParameterPtr			mVSOutView;						// Vertex shader output view vector (position in camera space) parameter.
	ParameterPtr			mPSInView;						// Pixel shader input view position (position in camera space) parameter.
	ParameterPtr			mVSInNormal;					// Vertex shader input normal.
	ParameterPtr			mVSInTangent;					// Vertex shader input tangent.
	ParameterPtr			mVSTBNMatrix;					// Vertex shader local TNB matrix.
	ParameterPtr			mVSLocalDir;					// Vertex shader local light direction.
	ParameterPtr			mNormalMapSampler;				// Normal map texture sampler parameter.
	ParameterPtr			mPSNormal;						// Pixel shader normal parameter.
	ParameterPtr			mVSInTexcoord;					// Vertex shader input texture coordinates.
	ParameterPtr			mVSOutTexcoord;					// Vertex shader output texture coordinates.
	ParameterPtr			mPSInTexcoord;					// Pixel shader input texture coordinates.
	ParameterPtr			mPSTempDiffuseColour;			// Pixel shader temporary diffuse calculation parameter.
	ParameterPtr			mPSTempSpecularColour;			// Pixel shader temporary specular calculation parameter.
	ParameterPtr			mPSDiffuse;						// Pixel shader input/local diffuse parameter.	
	ParameterPtr			mPSSpecular;					// Pixel shader input/local specular parameter.	
	ParameterPtr			mPSOutDiffuse;					// Pixel shader output diffuse parameter.	
	ParameterPtr			mPSOutSpecular;					// Pixel shader output specular parameter.	
	ParameterPtr			mDerivedSceneColour;			// Derived scene colour parameter.
	ParameterPtr			mLightAmbientColour;			// Ambient light colour parameter.
	ParameterPtr			mDerivedAmbientLightColour;		// Derived ambient light colour parameter.
	ParameterPtr			mSurfaceAmbientColour;			// Surface ambient colour parameter.
	ParameterPtr			mSurfaceDiffuseColour;			// Surface diffuse colour parameter.
	ParameterPtr			mSurfaceSpecularColour;			// Surface specular colour parameter.
	ParameterPtr			mSurfaceEmissiveColour;			// Surface emissive colour parameter.
	ParameterPtr			mSurfaceShininess;				// Surface shininess parameter.
	static Light			msBlankLight;					// Shared blank light.
};


/** 
A factory that enables creation of NormalMapLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class NormalMapLightingFactory : public SubRenderStateFactory
{
public:

	/** 
	@see SubRenderStateFactory::getType.
	*/
	virtual const String&	getType				() const;

	/** 
	@see SubRenderStateFactory::createInstance.
	*/
	virtual SubRenderState*	createInstance		(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass);

	/** 
	@see SubRenderStateFactory::writeInstance.
	*/
	virtual void			writeInstance		(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass);

	
protected:

	/** 
	@see SubRenderStateFactory::createInstanceImpl.
	*/
	virtual SubRenderState*	createInstanceImpl	();


};

/** @} */
/** @} */

}
}

#endif

