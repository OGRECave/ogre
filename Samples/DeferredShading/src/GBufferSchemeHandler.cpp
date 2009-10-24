/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "GBufferSchemeHandler.h"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

using namespace Ogre;

const String GBufferSchemeHandler::NORMAL_MAP_PATTERN = "normal";

Technique* GBufferSchemeHandler::handleSchemeNotFound(unsigned short schemeIndex, 
		const String& schemeName, Material* originalMaterial, unsigned short lodIndex, 
		const Renderable* rend)
{
	Ogre::MaterialManager& matMgr = Ogre::MaterialManager::getSingleton();
	String curSchemeName = matMgr.getActiveScheme();
	matMgr.setActiveScheme(MaterialManager::DEFAULT_SCHEME_NAME);
	Technique* originalTechnique = originalMaterial->getBestTechnique(lodIndex, rend);
	matMgr.setActiveScheme(curSchemeName);

	Technique* gBufferTech = originalMaterial->createTechnique();
	gBufferTech->removeAllPasses();
	gBufferTech->setSchemeName(schemeName);

	Technique* noGBufferTech = originalMaterial->createTechnique();
	noGBufferTech->removeAllPasses();
	noGBufferTech->setSchemeName("NoGBuffer");

	for (unsigned short i=0; i<originalTechnique->getNumPasses(); i++)
	{
		Pass* originalPass = originalTechnique->getPass(i);
		PassProperties props = inspectPass(originalPass, lodIndex, rend);
		
		if (!props.isDeferred)
		{
			//Just copy the technique so it gets rendered regularly
			Pass* clonePass = noGBufferTech->createPass();
			*clonePass = *originalPass;
			continue;
		}

		Pass* newPass = gBufferTech->createPass();
		MaterialGenerator::Perm perm = getPermutation(props);

		const Ogre::MaterialPtr& templateMat = mMaterialGenerator.getMaterial(perm);
    	
		//We assume that the GBuffer technique contains only one pass. But its true.
		*newPass = *(templateMat->getTechnique(0)->getPass(0));
		fillPass(newPass, originalPass, props);    
	}
    
	return gBufferTech;
}

bool GBufferSchemeHandler::checkNormalMap(
	TextureUnitState* tus, GBufferSchemeHandler::PassProperties& props)
{
	bool isNormal = false;
	Ogre::String lowerCaseAlias = tus->getTextureNameAlias();
	Ogre::StringUtil::toLowerCase(lowerCaseAlias);
	if (lowerCaseAlias.find(NORMAL_MAP_PATTERN) != Ogre::String::npos)
	{
		isNormal = true;
	}
	else 
	{
		Ogre::String lowerCaseName = tus->getTextureName();
		Ogre::StringUtil::toLowerCase(lowerCaseName);
		if (lowerCaseName.find(NORMAL_MAP_PATTERN) != Ogre::String::npos)
		{
			isNormal = true;
		}
	}

	if (isNormal)
	{
		if (props.normalMap == 0)
		{
			props.normalMap = tus;
		}
		else
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
				"Multiple normal map patterns matches",
				"GBufferSchemeHandler::inspectPass");
		}
	}
	return isNormal;
}

GBufferSchemeHandler::PassProperties GBufferSchemeHandler::inspectPass(
	Pass* pass, unsigned short lodIndex, const Renderable* rend)
{
	PassProperties props;
	
	//TODO : Use renderable to indicate wether this has skinning.
	//Probably use same const cast that renderSingleObject uses.
	if (pass->hasVertexProgram())
	{
		props.isSkinned = pass->getVertexProgram()->isSkeletalAnimationIncluded();
	}
	else 
	{
		props.isSkinned = false;
	}

	for (unsigned short i=0; i<pass->getNumTextureUnitStates(); i++) 
	{
		TextureUnitState* tus = pass->getTextureUnitState(i);
		if (!checkNormalMap(tus, props))
		{
			props.regularTextures.push_back(tus);
		}
		if (tus->getEffects().size() > 0)
		{
			props.isDeferred = false;
		}
		
	}

    if (pass->getDiffuse() != ColourValue::White)
    {
        props.hasDiffuseColour = true;
    }

    //Check transparency
    if (pass->getDestBlendFactor() != Ogre::SBF_ZERO)
    {
        //TODO : Better ways to do this
        props.isDeferred = false;
    }
	return props;
}

MaterialGenerator::Perm GBufferSchemeHandler::getPermutation(const PassProperties& props)
{
	MaterialGenerator::Perm perm = 0;
	switch (props.regularTextures.size())
	{
	case 0:
		perm |= GBufferMaterialGenerator::GBP_NO_TEXTURES;
		
		if (props.normalMap != 0)
		{
			perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		}
		else
		{
			perm |= GBufferMaterialGenerator::GBP_NO_TEXCOORDS;
		}
		break;
	case 1:
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXTURE;
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		break;
	case 2:
		perm |= GBufferMaterialGenerator::GBP_TWO_TEXTURES;
		//TODO : When do we use two texcoords?
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		break;
	case 3:
		perm |= GBufferMaterialGenerator::GBP_THREE_TEXTURES;
		perm |= GBufferMaterialGenerator::GBP_ONE_TEXCOORD;
		break;
	default:
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
			"Can not generate G-Buffer materials for '>3 regular-texture' objects",
			"GBufferSchemeHandler::inspectPass");
	}

	if (props.isSkinned)
	{
		perm |= GBufferMaterialGenerator::GBP_SKINNED;
	}

	if (props.normalMap != 0)
	{
		perm |= GBufferMaterialGenerator::GBP_NORMAL_MAP;
	}

    if (props.hasDiffuseColour)
    {
        perm |= GBufferMaterialGenerator::GBP_HAS_DIFFUSE_COLOUR;
    }
	return perm;
}

void GBufferSchemeHandler::fillPass(
	Pass* gBufferPass, Pass* originalPass, const PassProperties& props)
{
	//Reference the correct textures. Normal map first!
	int texUnitIndex = 0;
	if (props.normalMap != 0)
	{
		*(gBufferPass->getTextureUnitState(texUnitIndex)) = *(props.normalMap);
		texUnitIndex++;
	}
	for (size_t i=0; i<props.regularTextures.size(); i++)
	{
		*(gBufferPass->getTextureUnitState(texUnitIndex)) = *(props.regularTextures[i]);
		texUnitIndex++;
	}
	gBufferPass->setAmbient(originalPass->getAmbient());
	gBufferPass->setDiffuse(originalPass->getDiffuse());
	gBufferPass->setSpecular(originalPass->getSpecular());
	gBufferPass->setShininess(originalPass->getShininess());
    gBufferPass->setCullingMode(originalPass->getCullingMode());
    gBufferPass->setLightingEnabled(false);
}
