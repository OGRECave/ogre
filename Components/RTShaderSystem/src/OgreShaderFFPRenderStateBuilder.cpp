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

	curFactory = new FFPTransformFactory;	
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = new FFPColourFactory;	
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = new FFPLightingFactory;
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = new FFPTexturingFactory;
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoyList.push_back(curFactory);

	curFactory = new FFPFogFactory;	
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
		delete *it;		
	}
	mFFPSubRenderStateFactoyList.clear();
}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildRenderState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	renderState->reset();

	// Build transformation sub state.
	buildTransformSubState(sgPass, renderState);	

	// Build colour sub state.
	buildColourSubState(sgPass, renderState);

	// Build lighting sub state.
	buildLightingSubState(sgPass, renderState);

	// Build texturing sub state.
	buildTexturingSubState(sgPass, renderState);	
	
	// Build fog sub state.
	buildFogSubState(sgPass, renderState);
	
	// Resolve colour stage flags.
	resolveColourStageFlags(sgPass, renderState);

}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildTransformSubState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	SubRenderState* subRenderState;
	
	subRenderState = sgPass->getCustomFFPSubState(FFP_TRANSFORM);
	
	if (subRenderState == NULL)	
		subRenderState = ShaderGenerator::getSingleton().createSubRenderState(FFPTransform::Type);	
		
	renderState->addSubRenderState(subRenderState);
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildColourSubState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	SubRenderState* subRenderState;

	subRenderState = sgPass->getCustomFFPSubState(FFP_COLOUR);

	if (subRenderState == NULL)	
	{
		subRenderState = ShaderGenerator::getSingleton().createSubRenderState(FFPColour::Type);

		TrackVertexColourType trackColour = sgPass->getSrcPass()->getVertexColourTracking();

		if (trackColour != 0)
		{
			FFPColour* colourSubState = static_cast<FFPColour*>(subRenderState);

			colourSubState->addResolveStageMask(FFPColour::SF_VS_INPUT_DIFFUSE);
		}
	}
	

	renderState->addSubRenderState(subRenderState);
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildLightingSubState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	Pass* srcPass = sgPass->getSrcPass();

	if (srcPass->getLightingEnabled() == false)
		return;

	int lightCount[3];

	renderState->getLightCount(lightCount);

	// No lights allowed.
	if (lightCount[0] + lightCount[1] + lightCount[2] == 0)
		return;

	if (srcPass->getIteratePerLight())
	{
		LogManager::getSingleton().stream() << "FFPRenderStateBuilder::buildLightingSubState: Can not build light sub state for pass of material " << srcPass->getParent()->getParent()->getName() <<
			".since per light iteration not implemented !!. No light will be applied to this pass !!!";
		return;
	}

	SubRenderState* subRenderState;

	subRenderState = sgPass->getCustomFFPSubState(FFP_LIGHTING);
	if (subRenderState == NULL)
	{			
		subRenderState = ShaderGenerator::getSingleton().createSubRenderState(FFPLighting::Type);
		FFPLighting*     subRenderStateLighting = static_cast<FFPLighting*>(subRenderState);
		
		subRenderStateLighting->setTrackVertexColourType(srcPass->getVertexColourTracking());			

		if (srcPass->getShininess() > 0.0 &&
			srcPass->getSpecular() != ColourValue::Black)
		{
			subRenderStateLighting->setSpecularEnable(true);
		}
		else
		{
			subRenderStateLighting->setSpecularEnable(false);	
		}

		// Case this pass should run once per light -> override the light policy.
		if (srcPass->getIteratePerLight())
		{		
 			if (srcPass->getRunOnlyForOneLightType())
 			{
				if (srcPass->getOnlyLightType() == Light::LT_POINT)
				{
					lightCount[0] = 1;
					lightCount[1] = 0;
					lightCount[2] = 0;
				}
				else if (srcPass->getOnlyLightType() == Light::LT_DIRECTIONAL)
				{
					lightCount[0] = 0;
					lightCount[1] = 1;
					lightCount[2] = 0;
				}
				else if (srcPass->getOnlyLightType() == Light::LT_SPOTLIGHT)
				{
					lightCount[0] = 0;
					lightCount[1] = 0;
					lightCount[2] = 1;
				}
 			}
			else
			{
				lightCount[0] = 1;
				lightCount[1] = 1;
				lightCount[2] = 1;
			}			
		}

		subRenderStateLighting->setLightCount(lightCount);
		
	}
	
	renderState->addSubRenderState(subRenderState);
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildTexturingSubState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	Pass* srcPass = sgPass->getSrcPass();
	SubRenderState* subRenderState;

	subRenderState = sgPass->getCustomFFPSubState(FFP_TEXTURING);
	if (subRenderState == NULL)
	{
		subRenderState = ShaderGenerator::getSingleton().createSubRenderState(FFPTexturing::Type);
		FFPTexturing* subRenderStateTexture = static_cast<FFPTexturing*>(subRenderState);

		subRenderStateTexture->setTextureUnitCount(srcPass->getNumTextureUnitStates());

		// Build texture stage sub states.
		for (unsigned short i=0; i < srcPass->getNumTextureUnitStates(); ++i)
		{		
			TextureUnitState* texUnitState = srcPass->getTextureUnitState(i);								

			subRenderStateTexture->setTextureUnit(i, texUnitState);			
		}						
	}	

	renderState->addSubRenderState(subRenderState);
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildFogSubState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	Pass* srcPass = sgPass->getSrcPass();
	Pass* dstPass = sgPass->getDstPass();
	FogMode fogMode;
	ColourValue newFogColour;
	Real newFogStart, newFogEnd, newFogDensity;

	if (srcPass->getFogOverride())
	{
		fogMode			= srcPass->getFogMode();
		newFogColour	= srcPass->getFogColour();
		newFogStart		= srcPass->getFogStart();
		newFogEnd		= srcPass->getFogEnd();
		newFogDensity	= srcPass->getFogDensity();
	}
	else
	{
		SceneManager* sceneMgr = ShaderGenerator::getSingleton().getSceneManager();

		fogMode			= sceneMgr->getFogMode();
		newFogColour	= sceneMgr->getFogColour();
		newFogStart		= sceneMgr->getFogStart();
		newFogEnd		= sceneMgr->getFogEnd();
		newFogDensity	= sceneMgr->getFogDensity();
	}

	
	SubRenderState* subRenderState;
	
	
	subRenderState = sgPass->getCustomFFPSubState(FFP_FOG);

	if (subRenderState == NULL)
	{
		subRenderState = ShaderGenerator::getSingleton().createSubRenderState(FFPFog::Type);
		FFPFog* subRenderStateFog = static_cast<FFPFog*>(subRenderState);

		subRenderStateFog->setFogProperties(fogMode, newFogColour, newFogStart, newFogEnd, newFogDensity);
	}
	
	renderState->addSubRenderState(subRenderState);

	// Override scene fog since it will happen in shader.
	dstPass->setFog(true, FOG_NONE);			
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