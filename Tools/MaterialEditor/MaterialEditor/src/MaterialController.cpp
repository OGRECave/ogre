/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------
*/
#include "MaterialController.h"

#include "OgreColourValue.h"
#include "OgreTechnique.h"

#include "MaterialEventArgs.h"
#include "TechniqueController.h"

MaterialController::MaterialController() 
: mMaterialPtr(NULL)
{
	registerEvents();
}

MaterialController::MaterialController(MaterialPtr materialPtr) 
: mMaterialPtr(materialPtr)
{
	registerEvents();
}

MaterialController::~MaterialController()
{
}

void MaterialController::registerEvents()
{
	registerEvent(NameChanged);
	registerEvent(TechniqueAdded);
	registerEvent(TechniqueRemoved);
}

MaterialPtr MaterialController::getMaterial() const
{
	return mMaterialPtr;
}

void MaterialController::setMaterial(MaterialPtr mp)
{
	mMaterialPtr = mp;
}

TechniqueController* MaterialController::getTechniqueController(const String& name)
{
	TechniqueController* tc;
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
	{
		tc = (*it);
		if(tc->getTechnique()->getName() == name) return tc;
	}

	return NULL;
}

const TechniqueControllerList* MaterialController::getTechniqueControllers() const
{
	return &mTechniqueControllers;
}

void MaterialController::setName(const String& name)
{
	// ???
}

void MaterialController::setReceiveShadows(bool enabled)
{
	mMaterialPtr->setReceiveShadows(enabled);
}

void MaterialController::setTransparencyCastsShadows(bool enabled)
{
	mMaterialPtr->setTransparencyCastsShadows(enabled);
}

TechniqueController* MaterialController::createTechnique(void)
{
	Technique* t = mMaterialPtr->createTechnique();

	// Create controller
	TechniqueController* tc = new TechniqueController(t);
	mTechniqueControllers.push_back(tc);

	fireEvent(TechniqueAdded, MaterialEventArgs(this, tc));

	return tc;
}

TechniqueController* MaterialController::createTechnique(const String& name)
{
	Technique* t = mMaterialPtr->createTechnique();
	t->setName(name);

	// Create controller
	TechniqueController* tc = new TechniqueController(t);
	mTechniqueControllers.push_back(tc);

	fireEvent(TechniqueAdded, MaterialEventArgs(this, tc));

	return tc;
}

void MaterialController::removeTechnique(unsigned short index)
{
	mMaterialPtr->removeTechnique(index);

	fireEvent(TechniqueRemoved, MaterialEventArgs(this));
}

void MaterialController::removeAllTechniques(void)
{
	mMaterialPtr->removeAllTechniques();

	fireEvent(TechniqueRemoved, MaterialEventArgs(this));
}

void MaterialController::setAmbient(const ColourValue& ambient)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setAmbient(ambient);
}

void MaterialController::setAmbient(Real red, Real green, Real blue)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setAmbient(red, green, blue);
}

void MaterialController::setColourWriteEnabled(bool enabled)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setColourWriteEnabled(enabled);
}

void MaterialController::setCullingMode(CullingMode mode)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setCullingMode(mode);
}

void MaterialController::setDepthBias(float constantBias, float slopeScaleBias)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setDepthBias(constantBias, slopeScaleBias);
}

void MaterialController::setDepthCheckEnabled(bool enabled)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setDepthCheckEnabled(enabled);
}

void MaterialController::setDepthFunction(CompareFunction func)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setDepthFunction(func);
}

void MaterialController::setDepthWriteEnabled(bool enabled)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setDepthWriteEnabled(enabled);
}

void MaterialController::setDiffuse(const ColourValue&  diffuse)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setDiffuse(diffuse);
}


void MaterialController::setDiffuse(Real red, Real green, Real blue, Real alpha)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setDiffuse(red, green, blue, alpha);
}

void MaterialController::setFog(bool overrideScene, FogMode mode /* = FOG_NONE */, const ColourValue& colour /* = ColourValue::White */, Real expDensity /* = 0.001 */, Real linearStart /* = 0.0 */, Real linearEnd /* = 1.0 */)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);
}

void MaterialController::setLightingEnabled(bool enabled)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setLightingEnabled(true);
}

void MaterialController::setManualCullingMode(ManualCullingMode mode)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setManualCullingMode(mode);
}

void MaterialController::setPointSize(Real ps)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setPointSize(ps);
}

void MaterialController::setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor  destFactor)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setSceneBlending(sourceFactor, destFactor);
}

void MaterialController::setSceneBlending(const SceneBlendType sbt)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setSceneBlending(sbt);
}

void MaterialController::setSelfIllumination(const ColourValue& selfIllum)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setSelfIllumination(selfIllum);
}

void MaterialController::setSelfIllumination(Real red, Real green, Real blue)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setSelfIllumination(red, green, blue);
}

void MaterialController::setShadingMode(ShadeOptions mode)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setShadingMode(mode);
}

void MaterialController::setShininess(Real val)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setShininess(val);
}

void MaterialController::setSpecular(const ColourValue& specular)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setSpecular(specular);
}

void MaterialController::setSpecular(Real red, Real green, Real blue, Real alpha)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setSpecular(red, green, blue, alpha);
}

void MaterialController::setTextureAnisotropy(unsigned int maxAniso)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setTextureAnisotropy(maxAniso);
}

void MaterialController::setTextureFiltering(TextureFilterOptions filterType)
{
	TechniqueControllerList::iterator it;
	for(it = mTechniqueControllers.begin(); it != mTechniqueControllers.end(); ++it)
		(*it)->setTextureFiltering(filterType);
}

