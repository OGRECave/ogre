/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#include "SamplePlugin.h"
#include "VolumeTerrain.h"

#include "OgreVolumeCSGSource.h"
#include "OgreVolumeCacheSource.h"
#include "OgreVolumeTextureSource.h"
#include "OgreVolumeMeshBuilder.h"

using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::Volume;

void Sample_VolumeTerrain::setupContent(void)
{
    setupControls();
        
    Real size = (Real)256.0;
    Vector3 to(size);

    // Skydome
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    // Light
    Light* directionalLight0 = mSceneMgr->createLight("directionalLight0");
    directionalLight0->setType(Light::LT_DIRECTIONAL);
    directionalLight0->setDirection(Vector3((Real)1, (Real)-1, (Real)1));
    directionalLight0->setDiffuseColour((Real)1, (Real)0.98, (Real)0.73);
    directionalLight0->setSpecularColour((Real)0.1, (Real)0.1, (Real)0.1);
   
    // Volume
    mVolumeRoot = OGRE_NEW Chunk();
    SceneNode *volumeRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VolumeParent");
    mVolumeRoot->load(volumeRootNode, mSceneMgr, "volumeTerrain.cfg");
    mVolumeRoot->setMaterial("triplanarReference");

    // Camera
    mCamera->setPosition(to * mVolumeRoot->getScale());
    mCamera->lookAt((Real)5.5, (Real)5.5, (Real)0.0);
    mCamera->setNearClipDistance((Real)0.5);

}
    
//-----------------------------------------------------------------------

void Sample_VolumeTerrain::setupControls(void)
{
    mTrayMgr->showCursor();
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        setDragLook(true);
#endif
    mCameraMan->setTopSpeed((Real)100.0);
    // make room for the volume
    mTrayMgr->showLogo(TL_TOPRIGHT);
    mTrayMgr->showFrameStats(TL_TOPRIGHT);
    mTrayMgr->toggleAdvancedFrameStats();
}
    
//-----------------------------------------------------------------------

void Sample_VolumeTerrain::cleanupContent(void)
{   
    OGRE_DELETE mVolumeRoot;
    mVolumeRoot = 0;
}
    
//-----------------------------------------------------------------------

Sample_VolumeTerrain::Sample_VolumeTerrain(void) : mVolumeRoot(0), mHideAll(false)
{
    mInfo["Title"] = "Volume Terrain";
    mInfo["Description"] = "Demonstrates a volumetric terrain defined by an 3D texture.";
    mInfo["Thumbnail"] = "thumb_volumeterrain.png";
    mInfo["Category"] = "Geometry";
}
    
//-----------------------------------------------------------------------

bool Sample_VolumeTerrain::keyPressed(const OIS::KeyEvent& evt)
{
    if (evt.key == OIS::KC_F10)
    {
        mVolumeRoot->setVolumeVisible(!mVolumeRoot->getVolumeVisible());
    }
    if (evt.key == OIS::KC_F11)
    {
        mVolumeRoot->setOctreeVisible(!mVolumeRoot->getOctreeVisible());
    }
    if (evt.key == OIS::KC_F12)
    {
        mVolumeRoot->setDualGridVisible(!mVolumeRoot->getDualGridVisible());
    }
    if (evt.key == OIS::KC_H)
    {
        if (mHideAll)
        {
            mTrayMgr->showAll();
        }
        else
        {
            mTrayMgr->hideAll();
        }
        mHideAll = !mHideAll;
    }
    return SdkSample::keyPressed(evt);
}
    
//-----------------------------------------------------------------------

#ifndef OGRE_STATIC_LIB

SamplePlugin* sp;
Sample* s;
    
//-----------------------------------------------------------------------

extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_VolumeTerrain();
    sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
    sp->addSample(s);
    Root::getSingleton().installPlugin(sp);
}
    
//-----------------------------------------------------------------------

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Root::getSingleton().uninstallPlugin(sp); 
    OGRE_DELETE sp;
    delete s;
}

#endif
