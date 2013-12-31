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
#ifndef _RTShaderSRSSegmentedLights_
#define _RTShaderSRSSegmentedLights_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderParameter.h"
#include "OgreShaderSubRenderState.h"
#include "OgreVector4.h"
#include "OgreLight.h"
#include "OgreCommon.h"

/** Segmented lighting sub render state
* The following is sub render state handles lighting in the scene.
* This sub render state is heavily based on PerPixelLighting
*/
class RTShaderSRSSegmentedLights : public Ogre::RTShader::SubRenderState
{

	// Interface.
public:
	/** Class default constructor */	
	RTShaderSRSSegmentedLights();

	/** 
	@see SubRenderState::getType.
	*/
	virtual const Ogre::String& getType() const;

	/** 
	@see SubRenderState::getType.
	*/
	virtual int getExecutionOrder() const;

	/** 
	@see SubRenderState::updateGpuProgramsParams.
	*/
	virtual void updateGpuProgramsParams(Ogre::Renderable* rend, Ogre::Pass* pass, const Ogre::AutoParamDataSource* source, const Ogre::LightList* pLightList);

	/** 
	@see SubRenderState::copyFrom.
	*/
	virtual void copyFrom(const Ogre::RTShader::SubRenderState& rhs);


	/** 
	@see SubRenderState::preAddToRenderState.
	*/
	virtual bool preAddToRenderState(const Ogre::RTShader::RenderState* renderState, Ogre::Pass* srcPass, Ogre::Pass* dstPass);



	static Ogre::String Type;

	// Protected types:
protected:

	// Per light parameters.
	struct LightParams
	{
		Ogre::Light::LightTypes mType;				// Light type.		
		Ogre::RTShader::UniformParameterPtr mPosition;			// Light position.
		Ogre::RTShader::UniformParameterPtr mDirection;			// Light direction.
		Ogre::RTShader::UniformParameterPtr mSpotParams;		// Spot light parameters.
		Ogre::RTShader::UniformParameterPtr mDiffuseColour;		// Diffuse colour.
		Ogre::RTShader::UniformParameterPtr mSpecularColour;	// Specular colour.

	};

	typedef Ogre::vector<LightParams>::type LightParamsList;
	typedef LightParamsList::iterator LightParamsIterator;
	typedef LightParamsList::const_iterator LightParamsConstIterator;

	// Protected methods
protected:

	/** 
	Set the track per vertex colour type. Ambient, Diffuse, Specular and Emissive lighting components source
	can be the vertex colour component. To establish such a link one should provide the matching flags to this
	sub render state.
	*/
	void setTrackVertexColourType(Ogre::TrackVertexColourType type) { mTrackVertexColourType = type; }

	/** 
	Return the current track per vertex type.
	*/
	Ogre::TrackVertexColourType getTrackVertexColourType() const { return mTrackVertexColourType; }


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
	bool getSpecularEnable() const	  { return mSpecularEnable; }


	/** 
	@see SubRenderState::resolveParameters.
	*/
	virtual bool resolveParameters(Ogre::RTShader::ProgramSet* programSet);

	/** Resolve global lighting parameters */
	bool resolveGlobalParameters(Ogre::RTShader::ProgramSet* programSet);

	/** Resolve per light parameters */
	bool resolvePerLightParameters(Ogre::RTShader::ProgramSet* programSet);

	/** 
	@see SubRenderState::resolveDependencies.
	*/
	virtual bool resolveDependencies(Ogre::RTShader::ProgramSet* programSet);

	/** 
	@see SubRenderState::addFunctionInvocations.
	*/
	virtual bool addFunctionInvocations(Ogre::RTShader::ProgramSet* programSet);


	/** 
	Internal method that adds related vertex shader functions invocations.
	*/
	bool addVSInvocation(Ogre::RTShader::Function* vsMain, const int groupOrder, int& internalCounter);


	/** 
	Internal method that adds global illumination component functions invocations.
	*/
	bool addPSGlobalIlluminationInvocationBegin(Ogre::RTShader::Function* psMain, const int groupOrder, int& internalCounter);
	bool addPSGlobalIlluminationInvocationEnd(Ogre::RTShader::Function* psMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds per light illumination component functions invocations.
	*/
	bool addPSIlluminationInvocation(LightParams* curLightParams, Ogre::RTShader::Function* psMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds light illumination component calculated from the segmented texture.
	*/
	bool addPSSegmentedTextureLightInvocation(Ogre::RTShader::Function* psMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds the final colour assignments.
	*/
	bool addPSFinalAssignmentInvocation(Ogre::RTShader::Function* psMain, const int groupOrder, int& internalCounter);


	// Attributes.
protected:	
	Ogre::TrackVertexColourType mTrackVertexColourType;			// Track per vertex colour type.
	bool mSpecularEnable;				// Specular component enabled/disabled.
	LightParamsList mLightParamsList;				// Light list.
	Ogre::RTShader::UniformParameterPtr mWorldMatrix;				// World view matrix parameter.
	Ogre::RTShader::UniformParameterPtr mWorldITMatrix;				// World view matrix inverse transpose parameter.
	Ogre::RTShader::ParameterPtr mVSInPosition;					// Vertex shader input position parameter.
	Ogre::RTShader::ParameterPtr mVSOutWorldPos;					// Vertex shader output view position (position in camera space) parameter.
	Ogre::RTShader::ParameterPtr mPSInWorldPos;					// Pixel shader input view position (position in camera space) parameter.
	Ogre::RTShader::ParameterPtr mVSInNormal;					// Vertex shader input normal.
	Ogre::RTShader::ParameterPtr mVSOutNormal;					// Vertex shader output normal.
	Ogre::RTShader::ParameterPtr mPSInNormal;					// Pixel shader input normal.
	Ogre::RTShader::ParameterPtr mPSLocalNormal;
	Ogre::RTShader::ParameterPtr mPSTempDiffuseColour;			// Pixel shader temporary diffuse calculation parameter.
	Ogre::RTShader::ParameterPtr mPSTempSpecularColour;			// Pixel shader temporary specular calculation parameter.
	Ogre::RTShader::ParameterPtr mPSDiffuse;						// Pixel shader input/local diffuse parameter.	
	Ogre::RTShader::ParameterPtr mPSSpecular;					// Pixel shader input/local specular parameter.	
	Ogre::RTShader::ParameterPtr mPSOutDiffuse;					// Pixel shader output diffuse parameter.	
	Ogre::RTShader::ParameterPtr mPSOutSpecular;					// Pixel shader output specular parameter.	
	Ogre::RTShader::UniformParameterPtr mDerivedSceneColour;			// Derived scene colour parameter.
	Ogre::RTShader::UniformParameterPtr mLightAmbientColour;			// Ambient light colour parameter.
	Ogre::RTShader::UniformParameterPtr mDerivedAmbientLightColour;		// Derived ambient light colour parameter.
	Ogre::RTShader::UniformParameterPtr mSurfaceAmbientColour;			// Surface ambient colour parameter.
	Ogre::RTShader::UniformParameterPtr mSurfaceDiffuseColour;			// Surface diffuse colour parameter.
	Ogre::RTShader::UniformParameterPtr mSurfaceSpecularColour;			// Surface specular colour parameter.
	Ogre::RTShader::UniformParameterPtr mSurfaceEmissiveColour;			// Surface emissive colour parameter.
	Ogre::RTShader::UniformParameterPtr mSurfaceShininess;				// Surface shininess parameter.
	
	//Segmented texture 
	bool mUseSegmentedLightTexture;
	bool mIsDebugMode;
	unsigned short mLightSamplerIndex;
	Ogre::RTShader::UniformParameterPtr mPSLightTextureIndexLimit;
	Ogre::RTShader::UniformParameterPtr mPSLightTextureLightBounds;
	Ogre::RTShader::UniformParameterPtr mPSSegmentedLightTexture;
	//Ogre::RTShader::UniformParameterPtr mPSLightAreaBounds;

	static Ogre::Light msBlankLight;					// Shared blank light.

};


/** 
A factory that enables creation of PerPixelLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class RTShaderSRSSegmentedLightsFactory : public Ogre::RTShader::SubRenderStateFactory
{
public:

	/** 
	@see SubRenderStateFactory::getType.
	*/
	virtual const Ogre::String&	getType() const;

	/** 
	@see SubRenderStateFactory::createInstance.
	*/
	virtual Ogre::RTShader::SubRenderState*	createInstance(Ogre::ScriptCompiler* compiler, Ogre::PropertyAbstractNode* prop, Ogre::Pass* pass, Ogre::RTShader::SGScriptTranslator* translator);

	/** 
	@see SubRenderStateFactory::writeInstance.
	*/
	virtual void writeInstance(Ogre::MaterialSerializer* ser, Ogre::RTShader::SubRenderState* subRenderState, Ogre::Pass* srcPass, Ogre::Pass* dstPass);


protected:

	/** 
	@see SubRenderStateFactory::createInstanceImpl.
	*/
	virtual Ogre::RTShader::SubRenderState* createInstanceImpl();


};

#endif

