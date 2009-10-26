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
#ifndef _ShaderFFPColur_
#define _ShaderFFPColur_

#include "OgreShaderPrerequisites.h"
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

/** Colour sub render state implementation of the Fixed Function Pipeline.
Derives from SubRenderState class.
*/
class OGRE_RTSHADERSYSTEM_API FFPColour : public SubRenderState
{
public:

	// Parameter stage flags of the colour component.
	enum StageFlags
	{
		SF_VS_INPUT_DIFFUSE		= 1 << 1,
		SF_VS_INPUT_SPECULAR	= 1 << 2,
		SF_VS_OUTPUT_DIFFUSE	= 1 << 3,
		SF_VS_OUTPUT_SPECULAR	= 1 << 4,
		SF_PS_INPUT_DIFFUSE		= 1 << 5,
		SF_PS_INPUT_SPECULAR	= 1 << 6,
		SF_PS_OUTPUT_DIFFUSE	= 1 << 7,
		SF_PS_OUTPUT_SPECULAR	= 1 << 8,
	};

// Interface.
public:

	/** Class default constructor */
	FFPColour();


	/** 
	@see SubRenderState::getType.
	*/
	virtual const String&	getType					() const;

	/** 
	@see SubRenderState::getType.
	*/
	virtual int				getExecutionOrder		() const;

	/** 
	@see SubRenderState::copyFrom.
	*/
	virtual void			copyFrom				(const SubRenderState& rhs);

	/** 
	@see SubRenderState::getHashCode.
	*/
	virtual uint32			getHashCode				();

	/** 
	@see SubRenderState::preAddToRenderState.
	*/
	virtual bool			preAddToRenderState		(RenderState* renderState, Pass* srcPass, Pass* dstPass);

	/** 
	Set the resolve stage flags that this sub render state will produce.
	I.E - If one want to specify that the vertex shader program needs to get a diffuse component
	and the pixel shader should output diffuse component he should pass SF_VS_INPUT_DIFFUSE | SF_PS_OUTPUT_DIFFUSE.
	@param flags The stage flag to set.
	*/
	void				setResolveStageFlags		(unsigned int flags) { mResolveStageFlags = flags; }

	/** 
	Get the current resolve stage flags.		
	*/
	unsigned int		getResolveStageFlags		() const			 { return mResolveStageFlags; }

	/** 
	Add the given mask to resolve stage flags that this sub render state will produce.	
	@param mask The mask to add to current flag set.
	*/
	void				addResolveStageMask 		(unsigned int mask)  { mResolveStageFlags |= mask; }

	/** 
	Remove the given mask from the resolve stage flags that this sub render state will produce.	
	@param mask The mask to remove from current flag set.
	*/
	void				removeResolveStageMask 		(unsigned int mask)  { mResolveStageFlags &= ~mask; }

	static String Type;

// Protected methods
protected:	
	virtual bool			resolveParameters		(ProgramSet* programSet);	
	virtual bool			resolveDependencies		(ProgramSet* programSet);
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);

// Attributes.
protected:
	ParameterPtr	mVSInputDiffuse;			// Vertex shader input diffuse component.
	ParameterPtr	mVSInputSpecular;			// Vertex shader input specular component.
	ParameterPtr	mVSOutputDiffuse;			// Vertex shader output diffuse component.
	ParameterPtr	mVSOutputSpecular;			// Vertex shader input specular component.
	ParameterPtr	mPSInputDiffuse;			// Pixel shader input diffuse component.
	ParameterPtr	mPSInputSpecular;			// Pixel shader input specular component.
	ParameterPtr	mPSOutputDiffuse;			// Pixel shader output diffuse component.
	ParameterPtr	mPSOutputSpecular;			// Pixel shader input specular component.
	unsigned int	mResolveStageFlags;			// Stage flags that defines resolve parameters definitions.
};


/** 
A factory that enables creation of FFPColour instances.
@remarks Sub class of SubRenderStateFactory
*/
class FFPColourFactory : public SubRenderStateFactory
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

