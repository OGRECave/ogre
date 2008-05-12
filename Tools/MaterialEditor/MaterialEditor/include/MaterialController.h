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
#ifndef _MATERIALCONTROLLER_H_
#define _MATERIALCONTROLLER_H_

#include <list>

#include <boost/signal.hpp>

#include "OgreMaterial.h"

#include "EventContainer.h"

namespace Ogre
{
	class Technique;
}

class TechniqueController;

using namespace Ogre;

typedef std::list<TechniqueController*> TechniqueControllerList;

class MaterialController : public EventContainer
{
public:
	enum MaterialEvent
	{
		NameChanged,
		TechniqueAdded,
		TechniqueRemoved
	};

	MaterialController();
	MaterialController(MaterialPtr material);
	virtual ~MaterialController();

	MaterialPtr getMaterial() const;
	void setMaterial(MaterialPtr mp);
	
	TechniqueController* getTechniqueController(const String& name);
	const TechniqueControllerList* getTechniqueControllers() const;		

	void setName(const String& name);
	void setReceiveShadows(bool enabled);
	void setTransparencyCastsShadows(bool enabled);
	TechniqueController* createTechnique(void);
	TechniqueController* createTechnique(const String& name);
	void removeTechnique(unsigned short index);
	void removeAllTechniques(void);

	void setAmbient(const ColourValue&  ambient);
	void setAmbient(Real red, Real green, Real blue);
	void setColourWriteEnabled(bool enabled);   
	void setCullingMode(CullingMode mode);   
	void setDepthBias(float constantBias, float slopeScaleBias);   
	void setDepthCheckEnabled(bool enabled);  
	void setDepthFunction(CompareFunction func);
	void setDepthWriteEnabled(bool enabled);   
	void setDiffuse(const ColourValue&  diffuse);
	void setDiffuse(Real red, Real green, Real blue, Real alpha);   
	void setFog(bool overrideScene, FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White, Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0);   
	void setLightingEnabled(bool enabled);
	void setManualCullingMode(ManualCullingMode mode);
	void setPointSize(Real ps);   
	void setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor  destFactor);   
	void setSceneBlending(const SceneBlendType sbt);   
	void setSelfIllumination(const ColourValue& selfIllum);   
	void setSelfIllumination(Real red, Real green, Real blue);   
	void setShadingMode(ShadeOptions mode);   
	void setShininess(Real val);   
	void setSpecular(const ColourValue& specular);   
	void setSpecular(Real red, Real green, Real blue, Real alpha);   
	void setTextureAnisotropy(unsigned int maxAniso);   
	void setTextureFiltering(TextureFilterOptions filterType);   

protected:
	void registerEvents();

	MaterialPtr mMaterialPtr;

	TechniqueControllerList mTechniqueControllers;
};

#endif // _MATERIALCONTROLLER_H_

