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
#include "TechniqueController.h"

#include "OgreTechnique.h"

#include "MaterialController.h"
#include "PassController.h"
#include "TechniqueEventArgs.h"

TechniqueController::TechniqueController(Technique* technique)
: mParentController(NULL), mTechnique(technique)
{
	registerEvents();
}

TechniqueController::TechniqueController(MaterialController* parent, Technique* technique)
: mParentController(parent), mTechnique(technique)
{
	registerEvents();
}

TechniqueController::~TechniqueController()
{
}

void TechniqueController::registerEvents()
{
	registerEvent(NameChanged);
	registerEvent(SchemeChanged);
	registerEvent(LodIndexChanged);
	registerEvent(PassAdded);
	registerEvent(PassRemoved);
}

const Technique* TechniqueController::getTechnique() const
{
	return  mTechnique;
}

MaterialController* TechniqueController::getParentController() const
{
	return mParentController;
}

const PassControllerList* TechniqueController::getPassControllers() const
{
	return &mPassControllers;
}

PassController* TechniqueController::createPass(void)
{
	Pass* pass = mTechnique->createPass();

	// Create controller
	PassController* pc = new PassController(this, pass);
	mPassControllers.push_back(pc);

	fireEvent(PassAdded, TechniqueEventArgs(this, pc));

	return pc;
}

PassController* TechniqueController::createPass(const String& name)
{
	Pass* pass = mTechnique->createPass();
	pass->setName(name);

	// Create controller
	PassController* pc = new PassController(this, pass);
	mPassControllers.push_back(pc);

	fireEvent(PassAdded, TechniqueEventArgs(this, pc));

	return pc;
}

void TechniqueController::removeAllPasses(void)
{
}

void TechniqueController::removePass(unsigned short index)
{
}

void TechniqueController::movePass(const unsigned short sourceIndex, const unsigned short destIndex)
{
}

void TechniqueController::setName(const String& name)
{
	mTechnique->setName(name);

	fireEvent(NameChanged, TechniqueEventArgs(this));
}

void TechniqueController::setSchemeName(const String& schemeName)
{
	mTechnique->setSchemeName(schemeName);

	fireEvent(SchemeChanged, TechniqueEventArgs(this));
}

void TechniqueController::setLodIndex(unsigned short index)
{
	mTechnique->setLodIndex(index);

	fireEvent(LodIndexChanged, TechniqueEventArgs(this));
}

void TechniqueController::setAmbient(const ColourValue& ambient)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setAmbient(ambient);
}

void TechniqueController::setAmbient(Real red, Real green, Real blue)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setAmbient(red, green, blue);
}

void TechniqueController::setColourWriteEnabled(bool enabled)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setColourWriteEnabled(enabled);
}

void TechniqueController::setCullingMode(CullingMode mode)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setCullingMode(mode);
}

void TechniqueController::setDepthBias(float constantBias, float slopeScaleBias)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setDepthBias(constantBias, slopeScaleBias);
}

void TechniqueController::setDepthCheckEnabled(bool enabled)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setDepthCheckEnabled(enabled);
}

void TechniqueController::setDepthFunction(CompareFunction func)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setDepthFunction(func);
}

void TechniqueController::setDepthWriteEnabled(bool enabled)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setDepthWriteEnabled(enabled);
}

void TechniqueController::setDiffuse(const ColourValue&  diffuse)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setDiffuse(diffuse);
}


void TechniqueController::setDiffuse(Real red, Real green, Real blue, Real alpha)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setDiffuse(red, green, blue, alpha);
}

void TechniqueController::setFog(bool overrideScene, FogMode mode /* = FOG_NONE */, const ColourValue& colour /* = ColourValue::White */, Real expDensity /* = 0.001 */, Real linearStart /* = 0.0 */, Real linearEnd /* = 1.0 */)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);
}

void TechniqueController::setLightingEnabled(bool enabled)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setLightingEnabled(true);
}

void TechniqueController::setManualCullingMode(ManualCullingMode mode)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setManualCullingMode(mode);
}

void TechniqueController::setPointSize(Real ps)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setPointSize(ps);
}

void TechniqueController::setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor  destFactor)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setSceneBlending(sourceFactor, destFactor);
}

void TechniqueController::setSceneBlending(const SceneBlendType sbt)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setSceneBlending(sbt);
}

void TechniqueController::setSelfIllumination(const ColourValue& selfIllum)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setSelfIllumination(selfIllum);
}

void TechniqueController::setSelfIllumination(Real red, Real green, Real blue)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setSelfIllumination(red, green, blue);
}

void TechniqueController::setShadingMode(ShadeOptions mode)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setShadingMode(mode);
}

void TechniqueController::setShininess(Real val)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setShininess(val);
}

void TechniqueController::setSpecular(const ColourValue& specular)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setSpecular(specular);
}

void TechniqueController::setSpecular(Real red, Real green, Real blue, Real alpha)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setSpecular(red, green, blue, alpha);
}

void TechniqueController::setTextureAnisotropy(unsigned int maxAniso)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setTextureAnisotropy(maxAniso);
}

void TechniqueController::setTextureFiltering(TextureFilterOptions filterType)
{
	PassControllerList::iterator it;
	for(it = mPassControllers.begin(); it != mPassControllers.end(); ++it)
		(*it)->setTextureFiltering(filterType);
}

