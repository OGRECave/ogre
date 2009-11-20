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
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderGenerator.h"
#include "OgreShaderFFPRenderStateBuilder.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPLighting.h"
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderFFPFog.h"
#include "OgrePass.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "OgreShaderFFPRenderState.h"


namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::FFPRenderStateBuilder* Singleton<RTShader::FFPRenderStateBuilder>::ms_Singleton = 0;

namespace RTShader {


//-----------------------------------------------------------------------
FFPRenderStateBuilder* FFPRenderStateBuilder::getSingletonPtr()
{
	assert( ms_Singleton );  
	return ms_Singleton;
}

//-----------------------------------------------------------------------
FFPRenderStateBuilder& FFPRenderStateBuilder::getSingleton()
{
	assert( ms_Singleton );  
	return ( *ms_Singleton );
}

//-----------------------------------------------------------------------------
FFPRenderStateBuilder::FFPRenderStateBuilder()
{
	

}

//-----------------------------------------------------------------------------
FFPRenderStateBuilder::~FFPRenderStateBuilder()
{
	

}

//-----------------------------------------------------------------------------
bool FFPRenderStateBuilder::initialize()
{
	SubRenderStateFactory* curFactory;

	curFactory = OGRE_NEW FFPTransformFactory;	
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = OGRE_NEW FFPColourFactory;	
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = OGRE_NEW FFPLightingFactory;
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = OGRE_NEW FFPTexturingFactory;
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = OGRE_NEW FFPFogFactory;	
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	return true;
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::finalize()
{
	SubRenderStateFactoryIterator it;

	for (it = mFFPSubRenderStateFactoyList.begin(); it != mFFPSubRenderStateFactoyList.end(); ++it)
	{
		ShaderGenerator::getSingleton().removeSubRenderStateFactory(*it);		
		OGRE_DELETE *it;		
	}
	mFFPSubRenderStateFactoyList.clear();
}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildRenderState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	renderState->reset();

	// Build transformation sub state.
	buildFFPSubRenderState(FFP_TRANSFORM, FFPTransform::Type, sgPass, renderState);	

	// Build colour sub state.
	buildFFPSubRenderState(FFP_COLOUR, FFPColour::Type, sgPass, renderState);

	// Build lighting sub state.
	buildFFPSubRenderState(FFP_LIGHTING, FFPLighting::Type, sgPass, renderState);

	// Build texturing sub state.
	buildFFPSubRenderState(FFP_TEXTURING, FFPTexturing::Type, sgPass, renderState);	
	
	// Build fog sub state.
	buildFFPSubRenderState(FFP_FOG, FFPFog::Type, sgPass, renderState);
	
	// Resolve colour stage flags.
	resolveColourStageFlags(sgPass, renderState);

}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildFFPSubRenderState(int subRenderStateOrder, const String& subRenderStateType,
												ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	SubRenderState* subRenderState;

	subRenderState = sgPass->getCustomFFPSubState(subRenderStateOrder);

	if (subRenderState == NULL)	
	{
		subRenderState = ShaderGenerator::getSingleton().createSubRenderState(subRenderStateType);		
	}

	if (subRenderState->preAddToRenderState(renderState, sgPass->getSrcPass(), sgPass->getDstPass()))
	{
		renderState->addSubRenderState(subRenderState);
	}
	else
	{		
		ShaderGenerator::getSingleton().destroySubRenderState(subRenderState);				
	}
}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::resolveColourStageFlags( ShaderGenerator::SGPass* sgPass, RenderState* renderState )
{
	const SubRenderStateList& subRenderStateList = renderState->getSubStateList();
	FFPColour* colourSubState = NULL;

	// Find the colour sub state.
	for (SubRenderStateConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
	{
		SubRenderState* curSubRenderState = *it;

		if (curSubRenderState->getType() == FFPColour::Type)
		{
			colourSubState = static_cast<FFPColour*>(curSubRenderState);
			break;
		}
	}
	
	for (SubRenderStateConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
	{
		SubRenderState* curSubRenderState = *it;

		// Add vertex shader specular lighting output in case of specular enabled.
		if (curSubRenderState->getType() == FFPLighting::Type)
		{
			FFPLighting* lightingSubState = static_cast<FFPLighting*>(curSubRenderState);

			colourSubState->addResolveStageMask(FFPColour::SF_VS_OUTPUT_DIFFUSE);

			Pass* srcPass = sgPass->getSrcPass();

			if (srcPass->getShininess() > 0.0 &&
				srcPass->getSpecular() != ColourValue::Black)
			{
				colourSubState->addResolveStageMask(FFPColour::SF_VS_OUTPUT_SPECULAR);				
			}	
			break;
		}
	}
}



}
}

#endif