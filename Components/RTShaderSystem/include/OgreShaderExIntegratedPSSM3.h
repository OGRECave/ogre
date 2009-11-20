/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#ifndef _ShaderExIntegratedPSSM3_
#define _ShaderExIntegratedPSSM3_

#include "OgreRTShaderSystem.h"
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

/** Integrated PSSM shadow receiver with 3 splits sub render state implementation.
Derives from SubRenderState class.
*/
class IntegratedPSSM3 : public SubRenderState
{

	// Interface.
public:
	typedef std::vector<Real> SplitPointList;

	/** Class default constructor */	
	IntegratedPSSM3();

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


	
	/** Manually configure a new splitting scheme.
	@param newSplitPoints A list which is splitCount + 1 entries long, containing the
	split points. The first value is the near point, the last value is the
	far point, and each value in between is both a far point of the previous
	split, and a near point for the next one.
	*/
	void			setSplitPoints					(const SplitPointList& newSplitPoints);

	static String Type;

	// Protected types:
protected:

	// Shadow texture parameters.
	struct ShadowTextureParams
	{					
		Real				mMaxRange;				// The max range of this shadow texture in terms of PSSM (far plane of viewing camera).
		unsigned int		mTextureSamplerIndex;	// The shadow map sampler index.
		ParameterPtr		mTextureSampler;		// The shadow map sampler.			
		ParameterPtr		mInvTextureSize;		// The inverse texture 
		ParameterPtr		mWorldViewProjMatrix;	// The source light view projection matrix combined with world matrix.		
		ParameterPtr		mVSOutLightPosition;	// The vertex shader output position in light space.
		ParameterPtr		mPSInLightPosition;		// The pixel shader input position in light space.

	};

	typedef std::vector<ShadowTextureParams>			ShadowTextureParamsList;
	typedef ShadowTextureParamsList::iterator			ShadowTextureParamsIterator;
	typedef ShadowTextureParamsList::const_iterator		ShadowTextureParamsConstIterator;

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
	Internal method that adds related vertex shader functions invocations.
	*/
	bool			addVSInvocation						(Function* vsMain, const int groupOrder, int& internalCounter);

	/** 
	Internal method that adds related pixel shader functions invocations.
	*/
	bool			addPSInvocation						(Program* psProgram, const int groupOrder, int& internalCounter);





	// Attributes.
protected:		
	ShadowTextureParamsList		mShadowTextureParamsList;		// Shadow texture parameter list.	
	ParameterPtr				mPSSplitPoints;					// Split points parameter.
	ParameterPtr				mVSInPos;						// Vertex shader input position parameter.	
	ParameterPtr				mVSOutPos;						// Vertex shader output position (clip space) parameter.
	ParameterPtr				mVSOutDepth;					// Vertex shader output depth (clip space) parameter.
	ParameterPtr				mPSInDepth;						// Pixel shader input depth (clip space) parameter.
	ParameterPtr				mPSLocalShadowFactor;			// Pixel shader local computed shadow colour parameter.
	ParameterPtr				mPSDiffuse;						// Pixel shader in/local diffuse colour parameter.
	ParameterPtr				mPSOutDiffuse;					// Pixel shader output diffuse colour parameter.
	ParameterPtr				mPSSpecualr;					// Pixel shader in/local specular colour parameter.
	ParameterPtr				mPSDerivedSceneColour;			// Derived scene colour (ambient term).

};


/** 
A factory that enables creation of IntegratedPSSM3 instances.
@remarks Sub class of SubRenderStateFactory
*/
class IntegratedPSSM3Factory : public SubRenderStateFactory
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

