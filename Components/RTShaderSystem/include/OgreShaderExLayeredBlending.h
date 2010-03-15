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
#ifndef _OgreShaderExLayeredBlending_
#define _OgreShaderExLayeredBlending_

#include "OgreShaderFFPTexturing.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunction.h"
#include "OgreShaderSubRenderState.h"

using namespace Ogre;
using namespace Ogre::RTShader;

/** Texturing sub render state implementation of layered blending.
Derives from FFPTexturing class which derives from SubRenderState class.
*/
class LayeredBlending : public Ogre::RTShader::FFPTexturing
{
public:
	enum PLayeredBlendingMode
	{
		LB_FFPBlend,
		LB_BlendNormal,
		LB_BlendLighten,			
		LB_BlendDarken,		
		LB_BlendMultiply,
		LB_BlendAverage,	
		LB_BlendAdd,
		LB_BlendSubtract,
		LB_BlendDifference,
		LB_BlendNegation,
		LB_BlendExclusion,
		LB_BlendScreen,
		LB_BlendOverlay,
		LB_BlendSoftLight,
		LB_BlendHardLight,
		LB_BlendColorDodge,
		LB_BlendColorBurn, 
		LB_BlendLinearDodge,
		LB_BlendLinearBurn,
		LB_BlendLinearLight,
		LB_BlendVividLight,
		LB_BlendPinLight,
		LB_BlendHardMix,
		LB_BlendReflect,
		LB_BlendGlow,
		LB_BlendPhoenix,
		LB_BlendSaturation,
		LB_BlendColor,
		LB_BlendLuminosity
	};

	static Ogre::String mSubRenderType;

	/** Class default constructor */
	LayeredBlending();

	/** 
	@see SubRenderState::getType.
	*/
	virtual const Ogre::String&	getType					() const;


	/** 
	@Adds a layered blending mode to the list of texture parameters (mTextureUnitParamsList).
	*/
	void addBlendType(int index, LayeredBlending::PLayeredBlendingMode mode);

	/** 
	@see SubRenderState::copyFrom.
	*/
	virtual void			copyFrom				(const SubRenderState& rhs);


	vector<PLayeredBlendingMode>::type mBlendModes;

// Protected methods
protected:
	
	/** 
	@see SubRenderState::resolveDependencies.
	*/
	virtual bool			resolveDependencies		(Ogre::RTShader::ProgramSet* programSet);


	virtual void					addPSBlendInvocations(Function* psMain, 
													ParameterPtr arg1,
													ParameterPtr arg2,
													ParameterPtr texel,
													int samplerIndex,
													const LayerBlendModeEx& blendMode,
													const int groupOrder, 
													int& internalCounter,
													int targetChannels);
	
};



/** 
A factory that enables creation of LayeredBlending instances.
@remarks Sub class of SubRenderStateFactory
*/
class LayeredBlendingFactory : public SubRenderStateFactory
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

	/** 
	@see SubRenderStateFactory::writeInstance.
	*/
	virtual void			writeInstance		(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass);

	
protected:

	/** 
	@see SubRenderStateFactory::createInstanceImpl.
	*/
	virtual SubRenderState*	createInstanceImpl	();

	/** 
	@Converts string to Enum
	*/
	LayeredBlending::PLayeredBlendingMode stringToPBMEnum(const String &strValue);
};


#endif
