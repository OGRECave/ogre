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
#ifndef _ShaderFFPTextureStage_
#define _ShaderFFPTextureStage_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"
#include "OgreRenderSystem.h"

namespace Ogre {
namespace RTShader {


/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Texturing sub render state implementation of the Fixed Function Pipeline.
Implements texture coordinate processing:
@see http://msdn.microsoft.com/en-us/library/bb206247(VS.85).aspx
Implements texture blending operation:
@see http://msdn.microsoft.com/en-us/library/bb206241(VS.85).aspx
Derives from SubRenderState class.
*/
class OGRE_RTSHADERSYSTEM_API FFPTexturing : public SubRenderState
{

// Interface.
public:

	/** Class default constructor */
	FFPTexturing();

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

	static String Type;

// Protected types:
protected:

	// Per texture unit parameters.
	struct TextureUnitParams
	{
		TextureUnitState*		mTextureUnitState;				// Texture unit state.
		const Frustum*			mTextureProjector;				// Texture projector.
		unsigned short			mTextureSamplerIndex;			// Texture sampler index.
		GpuConstantType			mTextureSamplerType;			// Texture sampler index.
		GpuConstantType			mVSInTextureCoordinateType;		// Vertex shader input texture coordinate type.
		GpuConstantType			mVSOutTextureCoordinateType;	// Vertex shader output texture coordinates type.		
		TexCoordCalcMethod		mTexCoordCalcMethod;			// Texture coordinates calculation method.
		Parameter*				mTextureMatrix;					// Texture matrix parameter.
		Parameter*  			mTextureViewProjImageMatrix;	// Texture View Projection Image space matrix parameter.
		Parameter*				mTextureSampler;				// Texture sampler parameter.
		Parameter*				mVSInputTexCoord;				// Vertex shader input texture coordinates parameter.
		Parameter*				mVSOutputTexCoord;				// Vertex shader output texture coordinates parameter.
		Parameter*				mPSInputTexCoord;				// Pixel shader input texture coordinates parameter.
	};

	typedef std::vector<TextureUnitParams>			TextureUnitParamsList;
	typedef TextureUnitParamsList::iterator			TextureUnitParamsIterator;
	typedef TextureUnitParamsList::const_iterator	TextureUnitParamsConstIterator;

// Protected methods
protected:

	/** 
	Set the number of texture units this texturing sub state has to handle.
	@param count The number of texture unit states.
	*/
	void					setTextureUnitCount		(size_t count);

	/** 
	Return the number of texture units this sub state handle. 
	*/
	size_t					getTextureUnitCount		() const { return mTextureUnitParamsList.size(); }

	/** 
	Set texture unit of a given stage index.
	@param index The stage index of the given texture unit state.
	@param textureUnitState The texture unit state to bound the the stage index.
	*/
	void					setTextureUnit			(unsigned short index, TextureUnitState* textureUnitState);

	/** 
	@see SubRenderState::resolveParameters.
	*/
	virtual bool			resolveParameters		(ProgramSet* programSet);

	/** 
	Internal method that resolves uniform parameters of the given texture unit parameters.
	*/
			bool			resolveUniformParams	(TextureUnitParams* textureUnitParams, ProgramSet* programSet);

	/** 
	Internal method that resolves functions parameters of the given texture unit parameters.
	*/
			bool			resolveFunctionsParams	(TextureUnitParams* textureUnitParams, ProgramSet* programSet);

	/** 
	@see SubRenderState::resolveDependencies.
	*/
	virtual bool			resolveDependencies		(ProgramSet* programSet);

	/** 
	@see SubRenderState::addFunctionInvocations.
	*/
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);


	/** 
	Internal method that adds vertex shader functions invocations.
	*/
	bool					addVSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* vsMain);

	/** 
	Internal method that adds pixel shader functions invocations.
	*/
	bool					addPSFunctionInvocations(TextureUnitParams* textureUnitParams, Function* psMain, int& internalCounter);

	void					addPSArgumentInvocations(Function* psMain, 
													 Parameter* arg,
													 Parameter* texel,
													 int samplerIndex,
													 LayerBlendSource blendSrc,
													 const ColourValue& colourValue,
													 Real alphaValue,
													 bool isAlphaArgument,
													 const int groupOrder, 
													 int& internalCounter);

	void					addPSBlendInvocations(Function* psMain, 
												Parameter* arg1,
												Parameter* arg2,
												Parameter* texel,
												int samplerIndex,
												const LayerBlendModeEx& blendMode,
												const int groupOrder, 
												int& internalCounter,
												const char* targetChannels);
	
	/** 
	Determines the texture coordinates calculation method of the given texture unit state.
	*/
	TexCoordCalcMethod		getTexCalcMethod		(TextureUnitState* textureUnitState);

	/** 
	Determines if the given texture unit state need to use texture transformation matrix..
	*/
	bool					needsTextureMatrix		(TextureUnitState* textureUnitState);

// Attributes.
protected:
	TextureUnitParamsList	mTextureUnitParamsList;		// Texture units list. 		
	Parameter*				mWorldMatrix;				// World matrix parameter.
	Parameter*				mWorldITMatrix;				// World inverse transpose matrix parameter.
	Parameter*				mViewMatrix;				// View matrix parameter.			
	Parameter*				mVSInputNormal;				// Vertex shader input normal parameter.
	Parameter*  			mVSInputPos;				// Vertex shader input position parameter.		
	Parameter*				mPSOutDiffuse;				// Pixel shader output colour.
	Parameter*				mPSDiffuse;					// Pixel shader diffuse colour.
	Parameter*				mPSSpecular;				// Pixel shader specular colour.
};


/** 
A factory that enables creation of FFPTexturing instances.
@remarks Sub class of SubRenderStateFactory
*/
class FFPTexturingFactory : public SubRenderStateFactory
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

