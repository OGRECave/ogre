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
#include "PassController.h"

#include "OgrePass.h"

#include "PassEventArgs.h"
#include "TechniqueController.h"

PassController::PassController(Pass* pass)
: mParentController(NULL), mPass(pass)
{
	registerEvents();
}

PassController::PassController(TechniqueController* parent, Pass* pass)
: mParentController(parent), mPass(pass)
{
	registerEvents();
}

PassController::~PassController()
{
}

void PassController::registerEvents()
{
	registerEvent(NameChanged);
	registerEvent(AmbientChanged);
	registerEvent(DiffuseChanged);
	registerEvent(SpecularChanged);
	registerEvent(ShininessChanged);
	registerEvent(SelfIllumChanged);
	registerEvent(VertexColourTrackingChanged);
	registerEvent(PointSizeChanged);
	registerEvent(PointSpritesChanged);
	registerEvent(PointAttenuationChanged);
	registerEvent(PointMinSizeChanged);
	registerEvent(PointMaxSizeChanged);
	registerEvent(SceneBlendingTypeChanged);
	registerEvent(SceneBlendSrcFactorChanged);
	registerEvent(SceneBlendDestFactorChanged);
	registerEvent(DepthCheckChanged);
	registerEvent(DepthWriteChanged);
	registerEvent(DepthFunctionChanged);
	registerEvent(ColourWriteChanged);
	registerEvent(CullingModeChanged);
	registerEvent(ManualCullingModeChanged);
	registerEvent(LightingChanged);
	registerEvent(MaxLightsChanged);
	registerEvent(StartLightChanged);
	registerEvent(ShadingModeChanged);
	registerEvent(PolygonModeChanged);
	registerEvent(FogChanged);
	registerEvent(DepthBiasChanged);
	registerEvent(AlphaRejectionChanged);
	registerEvent(IteratePerLightChanged);
	registerEvent(LightCountPerIterationChanged);
}


TechniqueController* PassController::getParentController() const
{
	return mParentController;
}

Pass* PassController::getPass() const
{
	return mPass;
}

void PassController::setName(const String& name)
{
	mPass->setName(name);

	fireEvent(NameChanged, PassEventArgs(this));
}

void PassController::setAmbient(Real red, Real green, Real blue)
{
	mPass->setAmbient(red, green, blue);

	fireEvent(AmbientChanged, PassEventArgs(this));
}

void PassController::setAmbient(const ColourValue& ambient)
{
	mPass->setAmbient(ambient);

	fireEvent(AmbientChanged, PassEventArgs(this));
}

void PassController::setDiffuse(Real red, Real green, Real blue, Real alpha)
{
	mPass->setDiffuse(red, green, blue, alpha);

	fireEvent(DiffuseChanged, PassEventArgs(this));
}

void PassController::setDiffuse(const ColourValue &diffuse)
{
	mPass->setDiffuse(diffuse);

	fireEvent(DiffuseChanged, PassEventArgs(this));
}

void PassController::setSpecular(Real red, Real green, Real blue, Real alpha)
{
	mPass->setSpecular(red, green, blue, alpha);

	fireEvent(SpecularChanged, PassEventArgs(this));
}

void PassController::setSpecular(const ColourValue &specular)
{
	mPass->setSpecular(specular);

	fireEvent(SpecularChanged, PassEventArgs(this));
}

void PassController::setShininess(Real val)
{
	mPass->setShininess(val);

	fireEvent(ShininessChanged, PassEventArgs(this));
}

void PassController::setSelfIllumination(Real red, Real green, Real blue)
{
	mPass->setSelfIllumination(red, green, blue);

	fireEvent(SelfIllumChanged, PassEventArgs(this));
}

void PassController::setSelfIllumination(const ColourValue& selfIllum)
{
	mPass->setSelfIllumination(selfIllum);

	fireEvent(SelfIllumChanged, PassEventArgs(this));
}

void PassController::setVertexColourTracking(TrackVertexColourType tracking)
{
	mPass->setVertexColourTracking(tracking);

	fireEvent(VertexColourTrackingChanged, PassEventArgs(this));
}

void PassController::setPointSize(Real ps)
{
	mPass->setPointSize(ps);

	fireEvent(PointSizeChanged, PassEventArgs(this));
}

void PassController::setPointSpritesEnabled(bool enabled)
{
	mPass->setPointSpritesEnabled(enabled);

	fireEvent(PointSpritesChanged, PassEventArgs(this));
}

void PassController::setPointAttenuation(bool enabled, Real constant /* =0.0f */, Real linear /* =1.0f */, Real quadratic /* =0.0f */)
{
	mPass->setPointAttenuation(enabled, constant, linear, quadratic);

	fireEvent(PointAttenuationChanged, PassEventArgs(this));
}

void PassController::setPointMinSize(Real min)
{
	mPass->setPointMinSize(min);

	fireEvent(PointMinSizeChanged, PassEventArgs(this));
}

void PassController::setPointMaxSize(Real max)
{
	mPass->setPointMaxSize(max);

	fireEvent(PointMaxSizeChanged, PassEventArgs(this));
}

void PassController::setSceneBlending(const SceneBlendType sbt)
{
	mPass->setSceneBlending(sbt);

	fireEvent(SceneBlendingTypeChanged, PassEventArgs(this));
}

void PassController::setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor)
{
	mPass->setSceneBlending(sourceFactor, destFactor);

	fireEvent(SceneBlendSrcFactorChanged, PassEventArgs(this));
}

void PassController::setDepthCheckEnabled(bool enabled)
{
	mPass->setDepthCheckEnabled(enabled);

	fireEvent(DepthCheckChanged, PassEventArgs(this));
}

void PassController::setDepthWriteEnabled(bool enabled)
{
	mPass->setDepthWriteEnabled(enabled);

	fireEvent(DepthWriteChanged, PassEventArgs(this));
}

void PassController::setDepthFunction(CompareFunction func)
{
	mPass->setDepthFunction(func);

	fireEvent(DepthFunctionChanged, PassEventArgs(this));
}

void PassController::setColourWriteEnabled(bool enabled)
{
	mPass->setColourWriteEnabled(enabled);

	fireEvent(ColourWriteChanged, PassEventArgs(this));
}

void PassController::setCullingMode(CullingMode mode)
{
	mPass->setCullingMode(mode);

	fireEvent(CullingModeChanged, PassEventArgs(this));
}

void PassController::setManualCullingMode(ManualCullingMode mode)
{
	mPass->setManualCullingMode(mode);

	fireEvent(ManualCullingModeChanged, PassEventArgs(this));
}

void PassController::setLightingEnabled(bool enabled)
{
	mPass->setLightingEnabled(enabled);

	fireEvent(LightingChanged, PassEventArgs(this));
}

void PassController::setMaxSimultaneousLights(unsigned short maxLights)
{
	mPass->setMaxSimultaneousLights(maxLights);

	fireEvent(MaxLightsChanged, PassEventArgs(this));
}

void PassController::setStartLight(unsigned short startLight)
{
	mPass->setStartLight(startLight);

	fireEvent(StartLightChanged, PassEventArgs(this));
}

void PassController::setShadingMode(ShadeOptions mode)
{
	mPass->setShadingMode(mode);

	fireEvent(ShadingModeChanged, PassEventArgs(this));
}

void PassController::setPolygonMode(PolygonMode mode)
{
	mPass->setPolygonMode(mode);

	fireEvent(PolygonModeChanged, PassEventArgs(this));
}

void PassController::setFog(bool overrideScene, FogMode mode /* =FOG_NONE */, const ColourValue& colour /* =ColourValue::White */, Real expDensity /* =0.001 */, Real linearStart /* =0.0 */, Real linearEnd /* =1.0 */)
{
	mPass->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);

	fireEvent(FogChanged, PassEventArgs(this));
}

void PassController::setDepthBias(float constantBias, float slopeScaleBias /* =0.0f */)
{
	mPass->setDepthBias(constantBias, slopeScaleBias);

	fireEvent(DepthBiasChanged, PassEventArgs(this));
}

void PassController::setAlphaRejectSettings(CompareFunction func, unsigned char value)
{
	mPass->setAlphaRejectSettings(func, value);

	fireEvent(AlphaRejectionChanged, PassEventArgs(this));
}

void PassController::setAlphaRejectFunction(CompareFunction func)
{
	mPass->setAlphaRejectFunction(func);

	fireEvent(AlphaRejectionChanged, PassEventArgs(this));
}

void PassController::setAlphaRejectValue(unsigned char val)
{
	mPass->setAlphaRejectValue(val);

	fireEvent(AlphaRejectionChanged, PassEventArgs(this));
}

void PassController::setIteratePerLight(bool enabled, bool onlyForOneLightType /* =true */, Light::LightTypes lightType /*=Light::LT_POINT */)
{
	mPass->setIteratePerLight(enabled, onlyForOneLightType, lightType);

	fireEvent(IteratePerLightChanged, PassEventArgs(this));
}

void PassController::setLightCountPerIteration(unsigned short c)
{
	mPass->setLightCountPerIteration(c);

	fireEvent(LightCountPerIterationChanged, PassEventArgs(this));
}

void PassController::setVertexProgram(const String& name, bool resetParams /* =true */)
{
	mPass->setVertexProgram(name, resetParams);

	// TODO: Fire event
}

void PassController::setVertexProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setVertexProgramParameters(params);

	// TODO: Fire event
}

void PassController::setShadowCasterVertexProgram(const String& name)
{
	mPass->setShadowCasterVertexProgram(name);

	// TODO: Fire event
}

void PassController::setShadowCasterVertexProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setShadowCasterVertexProgramParameters(params);

	// TODO: Fire event
}

void PassController::setShadowReceiverVertexProgram(const String& name)
{
	mPass->setShadowReceiverVertexProgram(name);

	// TODO: Fire event
}

void PassController::setShadowReceiverVertexProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setShadowReceiverVertexProgramParameters(params);

	// TODO: Fire event
}

void PassController::setShadowReceiverFragmentProgram(const String& name)
{
	mPass->setShadowReceiverFragmentProgram(name);

	// TODO: Fire event
}

void PassController::setShadowReceiverFragmentProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setShadowReceiverFragmentProgramParameters(params);

	// TODO: Fire event
}

void PassController::setFragmentProgram(const String& name, bool resetParams /* =true */)
{
	mPass->setFragmentProgram(name, resetParams);

	// TODO: Fire event
}

void PassController::setFragmentProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setFragmentProgramParameters(params);

	// TODO: Fire event
}

void PassController::setTextureFiltering(TextureFilterOptions filterType)
{
	mPass->setTextureFiltering(filterType);

	// TODO: Fire event
}

void PassController::setTextureAnisotropy(unsigned int maxAniso)
{
	mPass->setTextureAnisotropy(maxAniso);

	// TODO: Fire event
}