#include "SamplePlugin.h"
#include "ShaderSystemTexturedFog.h"

using namespace Ogre;
using namespace OgreBites;

const String Sample_ShaderSystemTexturedFog::FOG_DISTANCE_SLIDER = "FogDistance";
const String Sample_ShaderSystemTexturedFog::ACTIVATE_FOG_BUTTON = "ActivateFog";
const String Sample_ShaderSystemTexturedFog::FOG_BACKGROUND_SLIDER = "FogBackground";
const String Sample_ShaderSystemTexturedFog::ACTIVATE_SKY_BUTTON = "ActivateSkyBox";

#ifndef OGRE_STATIC_LIB

static SamplePlugin* sp;
static Sample* s;

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_ShaderSystemTexturedFog;
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
