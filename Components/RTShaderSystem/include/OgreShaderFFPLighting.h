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
#ifndef _ShaderFFPLighting_
#define _ShaderFFPLighting_

#include "OgreShaderPrerequisites.h"
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

/** Lighting sub render state implementation of the Fixed Function Pipeline.
@see http://msdn.microsoft.com/en-us/library/bb147178(VS.85).aspx
Derives from SubRenderState class.
*/
class FFPLighting : public SubRenderState
{

// Interface.
public:
	
	/** Class default constructor */
	FFPLighting();

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
	void					setSpecularEnable		(bool enable) { mSpeuclarEnable = enable; }

	/** 
	Get the specular component state. 
	*/
	bool					getSpecularEnable		() const	  { return mSpeuclarEnable; }
		
	static String Type;

// Protected types:
protected:

	// Per light parameters.
	struct LightParams
	{
		Light::LightTypes	mType;				// Light type.		
		Parameter*			mPosition;			// Light position.
		Parameter*			mDirection;			// Light direction.
		Parameter*			mAttenuatParams;	// Attenuation parameters.
		Parameter*			mSpotParams;		// Spot light parameters.
		Parameter*			mDiffuseColour;		// Diffuse colour.
		Parameter*			mSpecularColour;	// Specular colour.

	};

	typedef std::vector<LightParams>				LightParamsList;
	typedef LightParamsList::iterator				LightParamsIterator;
	typedef LightParamsList::const_iterator			LightParamsConstIterator;

// Protected methods
protected:
	/** 
	@see SubRenderState::resolveParameters.
	*/
	virtual bool			resolveParameters		(ProgramSet* programSet);

	/** 
	@see SubRenderState::resolveDependencies.
	*/
	virtual bool			resolveDependencies		(ProgramSet* programSet);

	/** 
	@see SubRenderState::addFunctionInvocations.
	*/
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);


	/** 
	Internal method that adds global illumination component functions invocations.
	*/
	bool			addGlobalIlluminationInvocation	(Function* vsMain, const int groupOrder, int& internalCounter);
			
	/** 
	Internal method that adds per light illumination component functions invocations.
	*/
	bool			addIlluminationInvocation		(LightParams* curLightParams, Function* vsMain, const int groupOrder, int& internalCounter);


// Attributes.
protected:	
	TrackVertexColourType	mTrackVertexColourType;			// Track per vertex colour type.
	bool					mSpeuclarEnable;				// Specular component enabled/disabled.
	LightParamsList			mLightParamsList;				// Light list.
	Parameter*				mWorldViewMatrix;				// World view matrix parameter.
	Parameter*				mWorldViewITMatrix;				// World view matrix inverse transpose parameter.
	Parameter*				mVSInPosition;					// Vertex shader input position parameter.
	Parameter*				mVSInNormal;					// Vertex shader input normal.
	Parameter*				mVSDiffuse;						// Vertex shader diffuse.
	Parameter*				mVSOutDiffuse;					// Vertex shader output diffuse colour parameter.
	Parameter*				mVSOutSpecular;					// Vertex shader output specular colour parameter.
	Parameter*				mDerivedSceneColour;			// Derived scene colour parameter.
	Parameter*				mLightAmbientColour;			// Ambient light colour parameter.
	Parameter*				mDerivedAmbientLightColour;		// Derived ambient light colour parameter.
	Parameter*				mSurfaceAmbientColour;			// Surface ambient colour parameter.
	Parameter*				mSurfaceDiffuseColour;			// Surface diffuse colour parameter.
	Parameter*				mSurfaceSpecularColour;			// Surface specular colour parameter.
	Parameter*				mSurfaceEmissiveColour;			// Surface emissive colour parameter.
	Parameter*				mSurfaceShininess;				// Surface shininess parameter.
	static Light			msBlankLight;					// Shared blank light.

};


/** 
A factory that enables creation of FFPLighting instances.
@remarks Sub class of SubRenderStateFactory
*/
class FFPLightingFactory : public SubRenderStateFactory
{
public:

	/** 
	@see SubRenderStateFactory::getType.
	*/
	virtual const String&	getType				() const;
	
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

