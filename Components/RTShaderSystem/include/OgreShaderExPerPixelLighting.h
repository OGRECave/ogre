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
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#ifndef _ShaderExPerPixelLighting_
#define _ShaderExPerPixelLighting_

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

/** Per pixel Lighting extension sub render state implementation.
Derives from SubRenderState class.
*/
class _OgreRTSSExport PerPixelLighting : public SubRenderState
{

// Interface.
public:
	/** Class default constructor */	
	PerPixelLighting();

	/** 
	@see SubRenderState::getType.
	*/
	virtual const String&	getType					() const;

	/** 
	@see SubRenderState::getType.
	*/
	virtual int				getExecutionOrder		() const;

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


	
	static String Type;

// Protected types:
protected:
	
	// Per light parameters.
	struct _OgreRTSSExport LightParams
	{
		Light::LightTypes		mType;				// Light type.		
		UniformParameterPtr		mPosition;			// Light position.
		UniformParameterPtr		mDirection;			// Light direction.
		UniformParameterPtr		mAttenuatParams;	// Attenuation parameters.
		UniformParameterPtr		mSpotParams;		// Spot light parameters.
		UniformParameterPtr		mDiffuseColour;		// Diffuse colour.
		UniformParameterPtr		mSpecularColour;	// Specular colour.

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
	UniformParameterPtr		mWorldViewMatrix;				// World view matrix parameter.
	UniformParameterPtr		mWorldViewITMatrix;				// World view matrix inverse transpose parameter.
	ParameterPtr			mVSInPosition;					// Vertex shader input position parameter.
	ParameterPtr			mVSOutViewPos;					// Vertex shader output view position (position in camera space) parameter.
	ParameterPtr			mPSInViewPos;					// Pixel shader input view position (position in camera space) parameter.
	ParameterPtr			mVSInNormal;					// Vertex shader input normal.
	ParameterPtr			mVSOutNormal;					// Vertex shader output normal.
	ParameterPtr			mPSInNormal;					// Pixel shader input normal.
	ParameterPtr			mPSTempDiffuseColour;			// Pixel shader temporary diffuse calculation parameter.
	ParameterPtr			mPSTempSpecularColour;			// Pixel shader temporary specular calculation parameter.
	ParameterPtr			mPSDiffuse;						// Pixel shader input/local diffuse parameter.	
	ParameterPtr			mPSSpecular;					// Pixel shader input/local specular parameter.	
	ParameterPtr			mPSOutDiffuse;					// Pixel shader output diffuse parameter.	
	ParameterPtr			mPSOutSpecular;					// Pixel shader output specular parameter.	
	UniformParameterPtr		mDerivedSceneColour;			// Derived scene colour parameter.
	UniformParameterPtr		mLightAmbientColour;			// Ambient light colour parameter.
	UniformParameterPtr		mDerivedAmbientLightColour;		// Derived ambient light colour parameter.
	UniformParameterPtr		mSurfaceAmbientColour;			// Surface ambient colour parameter.
	UniformParameterPtr		mSurfaceDiffuseColour;			// Surface diffuse colour parameter.
	UniformParameterPtr		mSurfaceSpecularColour;			// Surface specular colour parameter.
	UniformParameterPtr		mSurfaceEmissiveColour;			// Surface emissive colour parameter.
	UniformParameterPtr		mSurfaceShininess;				// Surface shininess parameter.
	static Light			msBlankLight;					// Shared blank light.
};


/** 
A factory that enables creation of PerPixelLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport PerPixelLightingFactory : public SubRenderStateFactory
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
#endif

