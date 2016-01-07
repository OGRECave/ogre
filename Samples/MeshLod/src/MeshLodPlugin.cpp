#include "MeshLod.h"

#ifndef OGRE_STATIC_LIB

static OgreBites::SamplePlugin* sp;
static OgreBites::Sample* s;

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_MeshLod;
    sp = OGRE_NEW OgreBites::SamplePlugin(s->getInfo()["Title"] + " Sample");
    sp->addSample(s);
    Ogre::Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Ogre::Root::getSingleton().uninstallPlugin(sp); 
    OGRE_DELETE sp;
    delete s;
}

#endif
