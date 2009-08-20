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
#ifndef _ShaderFFPTextureStage_
#define _ShaderFFPTextureStage_

#include "OgrePrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreShaderParameter.h"
#include "OgreRenderSystem.h"

namespace Ogre {

class Frustum;

namespace CRTShader {

class Program;
class Function;

/************************************************************************/
/*                                                                      */
/************************************************************************/
class FFPTextureStage : public SubRenderState
{

// Interface.
public:
	FFPTextureStage();
	virtual ~FFPTextureStage();

	virtual const String&	getType					() const;
	virtual int				getExecutionOrder		() const;
	virtual void			copyFrom				(const SubRenderState& rhs);
	virtual uint32			getHashCode				();
	virtual void			updateGpuProgramsParams	(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);

	void					setTextureUnitState		(TextureUnitState* textureUnitState);
	void					setTextureSamplerIndex	(unsigned short index);

	static String Type;

// Protected methods
protected:
	virtual bool			resolveParameters		(ProgramSet* programSet);
	virtual bool			resolveUniformParams	(ProgramSet* programSet);
	virtual bool			resolveFunctionsParams	(ProgramSet* programSet);
	virtual bool			resolveDependencies		(ProgramSet* programSet);
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);


	bool					addVSFunctionInvocations(Function* vsMain);
	bool					addPSFunctionInvocations(Function* psMain);

	void					addPSArgumentInvocations(Function* psMain, 
													 Parameter* arg,
													 Parameter* texel,
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
												const LayerBlendModeEx& blendMode,
												const int groupOrder, 
												int& internalCounter);
	
	TexCoordCalcMethod		getTexCalcMethod		();
	bool					needsTextureMatrix		();

// Attributes.
protected:
	TextureUnitState*		mTextureUnitState;
	const Frustum*			mTextureProjector;
	unsigned short			mTextureSamplerIndex;
	GpuConstantType			mTextureSamplerType;
	GpuConstantType			mVSInTextureCoordinateType;	
	GpuConstantType			mVSOutTextureCoordinateType;		
	Parameter*				mWorldMatrix;
	Parameter*				mWorldITMatrix;
	Parameter*				mViewMatrix;
	Parameter*				mViewTMatrix;
	Parameter*				mTextureMatrix;
	Parameter*  			mTextureViewProjImageMatrix;
	Parameter*				mTextureSampler;
	Parameter*				mVSInputTexCoord;
	Parameter*				mVSInputNormal;
	Parameter*				mVSOutputTexCoord;
	Parameter*				mPSInputTexCoord;
	Parameter*				mPSOutColor;
	Parameter*				mPSDiffuseColor;
	Parameter*				mPSSpecularColor;
	Parameter*  			mVSInputPos;
};


/************************************************************************/
/*                                                                      */
/************************************************************************/
class FFPTextureStageFactory : public SubRenderStateFactory
{
public:
	virtual const String&	getType				() const;
	
protected:
	virtual SubRenderState*	createInstanceImpl	();


};

}
}

#endif

