#ifndef _PASSCONTROLLER_H_
#define _PASSCONTROLLER_H_

#include <list>

#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgreLight.h"
#include "OgrePrerequisites.h"

#include "EventContainer.h"

namespace Ogre
{
    class ColourValue;
    class Pass;
}

class TechniqueController;

using namespace Ogre;

class PassController : public EventContainer
{
public:
    enum PassEvent
    {
        NameChanged,
        AmbientChanged,
        DiffuseChanged,
        SpecularChanged,
        ShininessChanged,
        SelfIllumChanged,
        VertexColourTrackingChanged,
        PointSizeChanged,
        PointSpritesChanged,
        PointAttenuationChanged,
        PointMinSizeChanged,
        PointMaxSizeChanged,
        SceneBlendingTypeChanged,
        SceneBlendSrcFactorChanged,
        SceneBlendDestFactorChanged,
        DepthCheckChanged,
        DepthWriteChanged,
        DepthFunctionChanged,
        ColourWriteChanged,
        CullingModeChanged,
        ManualCullingModeChanged,
        LightingChanged,
        MaxLightsChanged,
        StartLightChanged,
        ShadingModeChanged,
        PolygonModeChanged,
        FogChanged,
        DepthBiasChanged,
        AlphaRejectionChanged,
        IteratePerLightChanged,
        LightCountPerIterationChanged,
    };

    PassController(Pass* pass);
    PassController(TechniqueController* parent, Pass* pass);
    virtual ~PassController();
    
    TechniqueController* getParentController() const;
    Pass* getPass() const;
    
    void registerEvents();

    void  setName(const String& name);
    void  setAmbient(Real red, Real green, Real blue);
    void  setAmbient(const ColourValue& ambient); 
    void  setDiffuse(Real red, Real green, Real blue, Real alpha);
    void  setDiffuse(const ColourValue &diffuse); 
    void  setSpecular(Real red, Real green, Real blue, Real alpha);
    void  setSpecular(const ColourValue &specular);
    void  setShininess(Real val);
    void  setSelfIllumination(Real red, Real green, Real blue);
    void  setSelfIllumination(const ColourValue &selfIllum);
    void  setVertexColourTracking(TrackVertexColourType tracking);
    void  setPointSize(Real ps);
    void  setPointSpritesEnabled(bool enabled);
    void  setPointAttenuation(bool enabled, Real constant=0.0f, Real linear=1.0f, Real quadratic=0.0f);
    void  setPointMinSize(Real min);
    void  setPointMaxSize(Real max);
    void  setSceneBlending(const SceneBlendType sbt);
    void  setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);
    void  setDepthCheckEnabled(bool enabled);
    void  setDepthWriteEnabled(bool enabled);
    void  setDepthFunction(CompareFunction func);
    void  setColourWriteEnabled(bool enabled);
    void  setCullingMode(CullingMode mode);
    void  setManualCullingMode(ManualCullingMode mode);
    void  setLightingEnabled(bool enabled);
    void  setMaxSimultaneousLights(unsigned short maxLights);
    void  setStartLight(unsigned short startLight);
    void  setShadingMode(ShadeOptions mode);
    void  setPolygonMode(PolygonMode mode);
    void  setFog(bool overrideScene, FogMode mode=FOG_NONE, const ColourValue &colour=ColourValue::White, Real expDensity=0.001, Real linearStart=0.0, Real linearEnd=1.0);
    void  setDepthBias(float constantBias, float slopeScaleBias=0.0f);
    void  setAlphaRejectSettings(CompareFunction func, unsigned char value);
    void  setAlphaRejectFunction(CompareFunction func);
    void  setAlphaRejectValue(unsigned char val);
    void  setIteratePerLight(bool enabled, bool onlyForOneLightType=true, Light::LightTypes lightType=Light::LT_POINT);
    void  setLightCountPerIteration(unsigned short c);
    void  setVertexProgram(const String &name, bool resetParams=true);
    void  setVertexProgramParameters(GpuProgramParametersSharedPtr params);
    void  setShadowCasterVertexProgram(const String &name);
    void  setShadowCasterVertexProgramParameters(GpuProgramParametersSharedPtr params);
    void  setShadowReceiverVertexProgram(const String &name);
    void  setShadowReceiverVertexProgramParameters(GpuProgramParametersSharedPtr params);
    void  setShadowReceiverFragmentProgram(const String &name);
    void  setShadowReceiverFragmentProgramParameters(GpuProgramParametersSharedPtr params);
    void  setFragmentProgram(const String &name, bool resetParams=true);
    void  setFragmentProgramParameters(GpuProgramParametersSharedPtr params);
    void  setTextureFiltering(TextureFilterOptions filterType);
    void  setTextureAnisotropy(unsigned int maxAniso);
    
protected:
    Pass* mPass;
    TechniqueController* mParentController;
};

#endif // _PASSCONTROLLER_H_