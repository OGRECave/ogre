#include "SamplePlugin.h"
#include "ShaderSystemMultiLight.h"

using namespace Ogre;
using namespace OgreBites;

const String Sample_ShaderSystemMultiLight::DEBUG_MODE_CHECKBOX = "DebugMode";
const String Sample_ShaderSystemMultiLight::NUM_OF_LIGHTS_SLIDER = "NumOfLights";
const String Sample_ShaderSystemMultiLight::TWIRL_LIGHTS_CHECKBOX = "TwirlLights";

#ifndef OGRE_STATIC_LIB

static SamplePlugin* sp;
static Sample* s;

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_ShaderSystemMultiLight;
    sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
    sp->addSample(s);
    Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Root::getSingleton().uninstallPlugin(sp); 
    OGRE_DELETE sp;
    delete s;
}

#endif
