/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _ShaderExIntegratedPSSM3_
#define _ShaderExIntegratedPSSM3_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreVector4.h"
#include "OgreLight.h"
#include "OgreCommon.h"
#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"

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
class _OgreRTSSExport IntegratedPSSM3 : public SubRenderState
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
	struct _OgreRTSSExport ShadowTextureParams
	{					
		Real				mMaxRange;				// The max range of this shadow texture in terms of PSSM (far plane of viewing camera).
		unsigned int		mTextureSamplerIndex;	// The shadow map sampler index.
		UniformParameterPtr	mTextureSampler;		// The shadow map sampler.			
		UniformParameterPtr	mInvTextureSize;		// The inverse texture 
		UniformParameterPtr	mWorldViewProjMatrix;	// The source light view projection matrix combined with world matrix.		
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
	UniformParameterPtr			mPSSplitPoints;					// Split points parameter.
	ParameterPtr				mVSInPos;						// Vertex shader input position parameter.	
	ParameterPtr				mVSOutPos;						// Vertex shader output position (clip space) parameter.
	ParameterPtr				mVSOutDepth;					// Vertex shader output depth (clip space) parameter.
	ParameterPtr				mPSInDepth;						// Pixel shader input depth (clip space) parameter.
	ParameterPtr				mPSLocalShadowFactor;			// Pixel shader local computed shadow colour parameter.
	ParameterPtr				mPSDiffuse;						// Pixel shader in/local diffuse colour parameter.
	ParameterPtr				mPSOutDiffuse;					// Pixel shader output diffuse colour parameter.
	ParameterPtr				mPSSpecualr;					// Pixel shader in/local specular colour parameter.
	UniformParameterPtr			mPSDerivedSceneColour;			// Derived scene colour (ambient term).

};


/** 
A factory that enables creation of IntegratedPSSM3 instances.
@remarks Sub class of SubRenderStateFactory
*/
class _OgreRTSSExport IntegratedPSSM3Factory : public SubRenderStateFactory
{
public:

	/** 
	@see SubRenderStateFactory::getType.
	*/
	virtual const String&	getType				() const;

	/** 
	@see SubRenderStateFactory::createInstance.
	*/
	virtual SubRenderState*	createInstance		(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);


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

