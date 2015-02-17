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

#include "VTestPlugin.h"
#include "ParticleTest.h"
#include "TransparencyTest.h"
#include "TextureEffectsTest.h"
#include "CubeMappingTest.h"
#include "TextureBlitTest.h"
#include "OgreResourceGroupManager.h"

VTestPlugin::VTestPlugin()
    :SamplePlugin("VTestPlugin")
{
    // add the playpen tests
    addSample(new TextureBlitTest());
    addSample(new CubeMappingTest()); // no bg on Win
    addSample(new ParticleTest());
    addSample(new TransparencyTest());
   addSample(new TextureEffectsTest());
}
//---------------------------------------------------------------------

VTestPlugin::~VTestPlugin()
{
    for (OgreBites::SampleSet::iterator i = mSamples.begin(); i != mSamples.end(); ++i)
    {
        delete *i;
    }
    mSamples.clear();
}
//---------------------------------------------------------------------

#ifndef OGRE_STATIC_LIB

VTestPlugin* testPlugin = 0;

extern "C" _OgreSampleExport void dllStartPlugin()
{
    testPlugin = OGRE_NEW VTestPlugin();
    Ogre::Root::getSingleton().installPlugin(testPlugin);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Ogre::Root::getSingleton().uninstallPlugin(testPlugin); 
    OGRE_DELETE testPlugin;
}

#endif
