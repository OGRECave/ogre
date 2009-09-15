/*
-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
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

