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
#ifndef _ShaderExReflectionMap_
#define _ShaderExReflectionMap_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderSubRenderState.h"

using namespace Ogre;
using namespace Ogre::RTShader;

/** Reflection map sub render state implementation.
It will use as extension for the RT Shader System.
This extension is based on reflection map + mask effect that exists in
3D studio max material system.
The effect uses 2 input textures - a mask texture and reflection texture.
The vertex shader responsible to output two texture coordinates sets:
1. 2D texcoord set that will use to sample to mask texture.
2. 2D/3D texcoord set that will use to sample the reflection map.
The pixel shader will sample both textures, multiply the results 
and add to the output diffuse color.
*/
class ShaderExReflectionMap : public SubRenderState
{

// Interface.
public:
	/** Class default constructor */	
	ShaderExReflectionMap();
	
	/** 
	@see SubRenderState::getType.
	*/
	virtual const String&	getType				() const;

	/** 
	@see SubRenderState::getExecutionOrder.
	*/
	virtual int				getExecutionOrder		() const;

	/** 
	@see SubRenderState::getHashCode.
	*/
	virtual uint32			getHashCode				();

	/** 
	@see SubRenderState::copyFrom.
	*/
	virtual void			copyFrom				(const SubRenderState& rhs);

	/** 
	@see SubRenderState::preAddToRenderState.
	*/
	virtual bool			preAddToRenderState		(RenderState* renderState, Pass* srcPass, Pass* dstPass);

	/** 
	@see SubRenderState::preRemoveFromRenderState.
	*/
	virtual void			preRemoveFromRenderState(RenderState* renderState, Pass* srcPass, Pass* dstPass);

	/** Sets the reflection map type. */
	void					setReflectionMapType	(TextureType type);

	/** Get the reflection map type. */
	TextureType				getReflectionMapType	() const { return mReflectionMapType; }

	static String Type;

	/// The mask map texture name key.	
	static String MaskMapTextureNameKey;

	/// The reflection map texture name key.	
	static String ReflectionMapTextureNameKey;


// Protected methods.
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
	bool			addVSInvocations				(Function* vsMain, const int groupOrder);


	/** 
	Internal method that adds related pixel shader functions invocations.
	*/
	bool			addPSInvocations				(Function* psMain, const int groupOrder);


// Attributes.
protected:
	unsigned short			mMaskMapSamplerIndex;				// Mask map texture sampler index.
	unsigned short			mReflectionMapSamplerIndex;			// Reflection map texture sampler index.
	TextureType				mReflectionMapType;					// Reflection map type. Valid are 2D or Cube.
	Parameter*				mMaskMapSampler;					// Normal map texture sampler parameter.
	Parameter*				mReflectionMapSampler;				// Reflection map texture sampler parameter.
	Parameter*				mVSInMaskTexcoord;					// Vertex shader input mask texture coordinates.
	Parameter*				mVSOutMaskTexcoord;					// Vertex shader output mask texture coordinates.
	Parameter*				mVSOutReflectionTexcoord;			// Vertex shader output reflection texture coordinates.
	Parameter*				mPSInMaskTexcoord;					// Pixel shader input mask texture coordinates.
	Parameter*				mPSInReflectionTexcoord;			// Pixel shader input reflection texture coordinates.
	Parameter*				mWorldMatrix;						// World matrix parameter.
	Parameter*				mWorldITMatrix;						// World inverse transpose matrix parameter.
	Parameter*				mViewMatrix;						// View matrix parameter.			
	Parameter*				mVSInputNormal;						// Vertex shader input normal parameter.
	Parameter*  			mVSInputPos;						// Vertex shader input position parameter.		
	Parameter*				mPSOutDiffuse;						// Pixel shader output colour.
};


/** 
A factory that enables creation of ShaderExReflectionMap instances.
@remarks Sub class of SubRenderStateFactory
*/
class ShaderExReflectionMapFactory : public SubRenderStateFactory
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



#endif

