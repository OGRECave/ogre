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
#ifndef _TECHNIQUECONTROLLER_H_
#define _TECHNIQUECONTROLLER_H_

#include <list>

#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgrePrerequisites.h"

#include "EventContainer.h"

namespace Ogre
{
	class ColourValue;
	class Technique;
}

class MaterialController;
class PassController;

using namespace Ogre;

typedef std::list<PassController*> PassControllerList;

class TechniqueController : public EventContainer
{
public:
	enum TechniqueEvent 
	{
		NameChanged,
		SchemeChanged,
		LodIndexChanged,
		PassAdded,
		PassRemoved
	};

	TechniqueController(Technique* technique);
	TechniqueController(MaterialController* parent, Technique* technique);
	virtual ~TechniqueController();

	MaterialController* getParentController() const;
	const Technique* getTechnique() const;
	const PassControllerList* getPassControllers() const;

	PassController* createPass(void);
	PassController* createPass(const String& name);
	void removeAllPasses(void);
	void removePass(unsigned short index);

	void movePass(const unsigned short sourceIndex, const unsigned short destIndex);
	void setName(const String& name);
	void setSchemeName(const String& schemeName);
	void setLodIndex(unsigned short index);

	void setAmbient(const ColourValue& ambient);
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

	MaterialController* mParentController;
	Technique* mTechnique;
	PassControllerList mPassControllers;
};
#endif // _TECHNIQUECONTROLLER_H_
