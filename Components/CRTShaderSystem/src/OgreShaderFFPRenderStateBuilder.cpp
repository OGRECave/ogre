/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreShaderGenerator.h"
#include "OgreShaderFFPRenderStateBuilder.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPLighting.h"
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPTextureStage.h"
#include "OgreShaderFFPFog.h"
#include "OgrePass.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "OgreShaderFFPRenderState.h"


namespace Ogre {

//-----------------------------------------------------------------------
template<> 
CRTShader::FFPRenderStateBuilder* Singleton<CRTShader::FFPRenderStateBuilder>::ms_Singleton = 0;

namespace CRTShader {


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

	curFactory = new FFPTextureStageFactory;	
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

	buildTextureStageSubState(sgPass, renderState);	
	
	// Build fog sub state.
	buildFogSubState(sgPass, renderState);
	
	const SubRenderStateList& subRenderStateList = renderState->getSubStateList();
	FFPColour* colourSubState = NULL;

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

		if (trackColour != NULL)
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

	int maxLightCount[3];

	renderState->getMaxLightCount(maxLightCount);

	// No lights allowed.
	if (maxLightCount[0] + maxLightCount[1] + maxLightCount[2] == 0)
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
					maxLightCount[0] = 1;
					maxLightCount[1] = 0;
					maxLightCount[2] = 0;
				}
				else if (srcPass->getOnlyLightType() == Light::LT_DIRECTIONAL)
				{
					maxLightCount[0] = 0;
					maxLightCount[1] = 1;
					maxLightCount[2] = 0;
				}
				else if (srcPass->getOnlyLightType() == Light::LT_SPOTLIGHT)
				{
					maxLightCount[0] = 0;
					maxLightCount[1] = 0;
					maxLightCount[2] = 1;
				}
 			}
			else
			{
				maxLightCount[0] = 1;
				maxLightCount[1] = 1;
				maxLightCount[2] = 1;
			}			
		}

		subRenderStateLighting->setMaxLightCount(maxLightCount);
		
	}
	
	renderState->addSubRenderState(subRenderState);
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildTextureStageSubState(ShaderGenerator::SGPass* sgPass, RenderState* renderState)
{
	Pass* dstPass = sgPass->getDstPass();

	// Build texture stage sub states.
	for (unsigned short i=0; i < dstPass->getNumTextureUnitStates(); ++i)
	{		
		TextureUnitState* texUnitState = dstPass->getTextureUnitState(i);
		SubRenderState* subRenderState;
		
		subRenderState = sgPass->getCustomFFPSubState(FFP_TEXTURE_STAGE0 + i*100);
		if (subRenderState == NULL)
		{
			subRenderState = ShaderGenerator::getSingleton().createSubRenderState(FFPTextureStage::Type);
			FFPTextureStage* subRenderStateTexture = static_cast<FFPTextureStage*>(subRenderState);

			subRenderStateTexture->setTextureUnitState(texUnitState);
			subRenderStateTexture->setTextureSamplerIndex(i);
		}
				
		renderState->addSubRenderState(subRenderState);
	}	
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



}
}
