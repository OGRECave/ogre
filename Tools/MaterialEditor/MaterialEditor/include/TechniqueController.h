/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
